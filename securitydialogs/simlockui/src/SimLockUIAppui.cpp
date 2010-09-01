/*
 * ============================================================================
 *  Name        : SimLockUIAppUi.cpp
 *  Part of     : Sim Lock UI Application
 *  Description : Implementation of Sim Lock UI Application UI Methods
 *  Version     : 
 *  
* Copyright (c) 2005-2010 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:   Build info file for Ado domain appinstall 
* ============================================================================
*/

// Platform Includes
#include <avkon.hrh>
#include <aknnotewrappers.h>    // CAknNoteDialog
#include <StringLoader.h>       // StringLoader
#include <exterror.h>           // KErrGsm0707OperationNotAllowed
#include <eikspane.h>     // CEikStatusPane
#include <aknmessagequerydialog.h> // CAknMessageQueryDialog
#include <e32base.h>
#include <e32property.h> //Rproperty
#include <rmmcustomapi.h>       // TSimLockPassword

// Local Includes
#include "simlockisaserverdefinitions.h"
#include "simlockui.pan"
#include "simlockuiappui.h"
#include "simlockuibackgroundcontrol.h"
#include "simlockui.hrh"
#include "simlockdatahandlingdelegate.h"
#include "simlocktelephonyproxy.h"
#include "simlockuikeys.h"

// Resource Includes
#include <simlockui.rsg>

// ---------------------------------------------------------------------------
// CSimLockUIAppUi::CSimLockUIAppUi
// ---------------------------------------------------------------------------
CSimLockUIAppUi::CSimLockUIAppUi(
        CSimLockDataHandlingDelegate& aSimLockDelegate) :
    iSimLockDelegate(aSimLockDelegate), iFirstRun(ETrue)
    {
    // no implementation required
    }

// ---------------------------------------------------------------------------
// CSimLockUIAppUi::ConstructL
// ---------------------------------------------------------------------------
void CSimLockUIAppUi::ConstructL()
    {
    BaseConstructL(EAknEnableSkin | EAknEnableMSK);

    // Used to get status of phone call
    iTelephonyProxy = CSimLockTelephonyProxy::NewL();
    //Define RProperty to Publish Keys for SimLockUi
    TInt ret = RProperty::Define(KSimLockProperty, ESimLockActiveStatus,
            RProperty::EInt);
    }

// ---------------------------------------------------------------------------
// CSimLockUIAppUi::~CSimLockUIAppUi
// ---------------------------------------------------------------------------
CSimLockUIAppUi::~CSimLockUIAppUi()
    {

    delete iTelephonyProxy;

    if (iBackgroundControl)
        {
        RemoveFromStack(iBackgroundControl);
        delete iBackgroundControl;
        }
    //Sets SimLock Ui P&S keys to show that SimLock Ui is exiting
    RProperty::Set(KSimLockProperty, ESimLockActiveStatus, KSimLockStatusDone);

    }

// ---------------------------------------------------------------------------
// CSimLockUIAppUi::HandleCommandL
// ---------------------------------------------------------------------------
void CSimLockUIAppUi::HandleCommandL(TInt aCommand)
    {
    switch (aCommand)
        {
        case EEikCmdExit:
        case EAknSoftkeyExit:
            Exit();
            break;

        default:
            Panic(ESimLockUIBasicUi);
            break;
        }
    }

// ---------------------------------------------------------------------------
// CSimLockUIAppUi::HandleForegroundEventL
// ---------------------------------------------------------------------------
void CSimLockUIAppUi::HandleForegroundEventL(TBool aForeground)
    {
    CAknAppUi::HandleForegroundEventL(aForeground);

    // When the app switches to the foreground for the first time, display
    // series of dialogs.
    if (aForeground && iFirstRun)
        {
        iFirstRun = EFalse;   
        ProcessSimUnlockDialogsL();
        PrepareToExit();
        Exit();
        }
    }

// ---------------------------------------------------------------------------
// CSimLockUIAppUi::ProcessSimUnlockDialogsL
// ---------------------------------------------------------------------------
TBool CSimLockUIAppUi::ProcessSimUnlockDialogsL()
    {
    TBool keepLooping = ETrue;

    RMmCustomAPI::TSimLockPassword password;


    // Loop until flag is set
    do
        {
        // Create background if not already there
        if (!iBackgroundControl)
            {
            CreateBackgroundControlL();
            }
        // Display Introduction Dialog/Prompt
        if (!DisplayIntroductionDialogL())
            {
            return EFalse;
            }
        // Prompt for password and attempt to unlock Sim
        if (!PromptForPasswordL(password))
            {
            // skip next iteration
            continue;
            }        
        // Set only if no dialogs are ever dismissed
         keepLooping = AttemptToUnlockSimL(password);

        // Continue to loop while indicated to start over and dialog
        // has not been dismissed
        }
    while (keepLooping);
    
  return keepLooping;
    }

// ---------------------------------------------------------------------------
// CSimLockUIAppUi::CreateBackgroundControlL
// ---------------------------------------------------------------------------
void CSimLockUIAppUi::CreateBackgroundControlL()
    {
    iBackgroundControl = CSimLockUIBackgroundControl::NewL();
    AddToStackL(iBackgroundControl);
    }

// ---------------------------------------------------------------------------
// CSimLockUIAppUi::DisplayIntroductionDialogL
// ---------------------------------------------------------------------------
TBool CSimLockUIAppUi::DisplayIntroductionDialogL()
    {
    SetBackgroundTextL(EBackgroundTextEmpty);

    HBufC* displayText = StringLoader::LoadLC(R_SIM_INFO_WELCOME_NOTE_TEXT);
    CAknMessageQueryDialog* query = new (ELeave) CAknMessageQueryDialog(
            CAknMessageQueryDialog::ENoTone);
    CleanupStack::PushL(query);
    query->SetMessageTextL(*displayText);
    TInt executeResult = query->ExecuteLD(R_SIM_INFO_WELCOME_NOTE);
    CleanupStack::Pop(query);
    CleanupStack::PopAndDestroy(displayText);
    return executeResult;

    }

// ---------------------------------------------------------------------------
// CSimLockUIAppUi::PromptForPasswordL
// ---------------------------------------------------------------------------
TBool CSimLockUIAppUi::PromptForPasswordL(TDes& aPassword)
    {
    RMmCustomAPI::TSimLockPassword newPassword;

    // Prompt for password twice.  Loop until passwords match.
    do
        {
        aPassword.Zero();

        // Ask for password
        CAknTextQueryDialog* dialog = CAknTextQueryDialog::NewL(aPassword);
        if (!dialog->ExecuteLD(R_SIM_INFO_ENTER_PASSWORD))
            {
            return EFalse;
            }

        // Ask to re-enter password and verify there is no call in progress
        if (!CompletePromptForPasswordL(aPassword, newPassword))
            {
            return EFalse;
            }
        }
    while (newPassword.Compare(aPassword));

    return ETrue;
    }

// ---------------------------------------------------------------------------
// CSimLockUIAppUi::CompletePromptForPasswordL
// ---------------------------------------------------------------------------
TBool CSimLockUIAppUi::CompletePromptForPasswordL(const TDesC& aPassword,
        TDes& aNewPassword)
    {
    TBool callInProgress = EFalse;

    aNewPassword.Zero();

    do // Loop until no call in progress
        {
        // Display dialog to ask to re-enter password
        CAknTextQueryDialog* dialog = CAknTextQueryDialog::NewL(aNewPassword);
        if (!dialog->ExecuteLD(R_SIM_INFO_REENTER_PASSWORD))
            {
             return EFalse;
            }

        // Check to see if passwords match
        if (aNewPassword.Compare(aPassword))
            {
            // Create message query dialog to indicate that codes do not match
            CAknMessageQueryDialog* query =
                    new (ELeave) CAknMessageQueryDialog(
                            CAknMessageQueryDialog::ENoTone);
            return query->ExecuteLD(R_SIM_INFO_CODES_NOT_MATCH);            
            }

        // If there is a call in progress, show note and return to "re-enter password" prompt
        if (!iTelephonyProxy->IsCallInProgress())
            {
            return ETrue;
            }

        // Do not allow Sim unlocking, return to password prompt
        ShowInformationNoteL(R_SIM_UNLOCK_MESSAGE_END_CALL);
        callInProgress = ETrue;
        }
    while (callInProgress);

    return ETrue;
    }

// ---------------------------------------------------------------------------
// CSimLockUIAppUi::AttemptToUnlockSimL
// ---------------------------------------------------------------------------
TBool CSimLockUIAppUi::AttemptToUnlockSimL(const TDesC& aPassword)
    {
    
const TDesC& password =  aPassword;

#ifdef __WINS__
    // In WINS (Emulator) builds, call to OpenSimLock will just time out
    TInt unlockResult = KErrGsm0707IncorrectPassword;
#else
    // Attempt to unlock SIM
    TInt unlockResult = iSimLockDelegate.OpenSimLock( password );
#endif

    CAknMessageQueryDialog* query = new (ELeave) CAknMessageQueryDialog(
            CAknMessageQueryDialog::ENoTone);

    switch (unlockResult)
        {
        case KErrAlreadyExists:
            // Should not happen!  This indicates that the phone was already unlocked,
            // which it shouldn't be if we got this far.
            ASSERT( 0 );

            // Fall Through

        case KErrNone:
            SetBackgroundTextL(EBackgroundTextUnlockComplete);
            ShowInformationNoteL(R_SIM_PHONE_UNLOCKED);
            return EFalse;

        default:
            // Oops, we missed one.
            ASSERT( 0 );
        case KErrGsm0707OperationNotAllowed:
        case KErrLocked:
            // Permanently locked
            SetBackgroundTextL(EBackgroundTextEmpty);
            query->ExecuteLD(R_SIM_UNLOCK_FINAL);
            return EFalse;
        case KErrTimedOut:
        case KErrGeneral:        
        case KErrArgument:
        case KErrGsm0707IncorrectPassword:
            // Not permanently locked
            return HandleUnlockFailedL();


        }//switch
    }
// ---------------------------------------------------------------------------
// CSimLockUIAppUi::HandleUnlockFailedL
// ---------------------------------------------------------------------------
TBool CSimLockUIAppUi::HandleUnlockFailedL()
    {
    RMmCustomAPI::TSimLockPassword password;

    // Otherwise, prompt user to try again
    SetBackgroundTextL(EBackgroundTextEmpty);

    // Show "Code Incorrect" and prompt user to continue or not
    CAknMessageQueryDialog* query = new (ELeave) CAknMessageQueryDialog(
            CAknMessageQueryDialog::ENoTone);
    if (query->ExecuteLD(R_SIM_INFO_CODE_INCORRECT))
        {
        if ( PromptForPasswordL(password) )
            {
            return AttemptToUnlockSimL(password);    
            }
        else
            {
            // Set flag indicating to keep looping            
            return ETrue;
            }
        }
    else
        {
        // display intro popup
        return ETrue;
        }
    }

// ---------------------------------------------------------------------------
// CSimLockUIAppUi::ShowInformationNoteL
// ---------------------------------------------------------------------------
TInt CSimLockUIAppUi::ShowInformationNoteL(TInt aResourceId) const
    {
    TInt executeResult;

    // Show note with the provided resource id
    HBufC* displayText = StringLoader::LoadLC(aResourceId);

    CAknInformationNote* dialog = new (ELeave) CAknInformationNote(ETrue);
    executeResult = dialog->ExecuteLD(*displayText);
    CleanupStack::PopAndDestroy(displayText);
    return executeResult;
    }

// ---------------------------------------------------------------------------
// CSimLockUIAppUi::SetBackgroundTextL
// ---------------------------------------------------------------------------
void CSimLockUIAppUi::SetBackgroundTextL(TBackgroundText aText)
    {
    HBufC* displayText = NULL;

    // Load resource and set text in background control based on provided info
    switch (aText)
        {
        case EBackgroundTextEmpty:
            displayText = KNullDesC().AllocL();
            break;

        case EBackgroundTextMain:
            displayText = StringLoader::LoadL(
                    R_SIMLOCK_UI_BACKGROUND_TEXT_MAIN);
            break;

        case EBackgroundTextUnlockComplete:
            displayText = StringLoader::LoadL(
                    R_SIMLOCK_UI_BACKGROUND_TEXT_UNLOCK_COMPLETE);
            break;

        case EBackgroundTextUnlockFailed:
            displayText = StringLoader::LoadL(
                    R_SIMLOCK_UI_BACKGROUND_TEXT_UNLOCK_FAILED);
            break;

        case EBackgroundTextUnlockIncorrect:
            displayText = StringLoader::LoadL(
                    R_SIMLOCK_UI_BACKGROUND_TEXT_UNLOCK_INCORRECT);
            break;

        default:
            ASSERT( 0 );
            break;
        }

    // Transfer ownership to iBackgroundControl
    iBackgroundControl->SetBackgroundText(displayText);
    }

// End of file.
