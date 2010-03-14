/*
* ============================================================================
*  Name         : SimLockUIAppUI.h
*  Part of      : Sim Lock UI Application
*  Description  : SimLock UI Application UI Header
*  Version      : 
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

#ifndef __SIMLOCKUI_APPUI_H__
#define __SIMLOCKUI_APPUI_H__

// System includes
#include <aknappui.h>
#include <aknnotedialog.h>  // CAknNoteDialog::TTone
#include <e32property.h>    // RProperty.


// FORWARD DECLARATIONS
class RProperty;
// User includes
#include "simlockuibackgroundcontrol.h" // CSimLockUIBackgroundControl::TBackgroundText

// Forward reference
class CSimLockUIBackgroundControl;
class CSimLockDataHandlingDelegate;
class CAknInformationNote;
class CSimLockTelephonyProxy;

//const TUid KSimLockProperty={0x2000B0FC};
/**
 *  CSimLockUIAppUi
 *
 *  An instance of class CSimLockUIAppUi is the UserInterface part of the AVKON
 *  application framework for the SimLockUI example application
 *
 *  @lib avkon.lib
 *  @lib eikcore.lib
 *  @lib eiksrv.lib
 */
class CSimLockUIAppUi : public CAknAppUi
    {
public: // Constructor/Destructor

    /**
     * CSimLockUIAppUi
     * Perform the first phase of two phase construction.
     * This needs to be public due to the way the framework constructs the AppUi
     *
     * @param aSimLockDelegate Sim Lock UI Delegate (ownership not transferred)
     */
    CSimLockUIAppUi( CSimLockDataHandlingDelegate& aSimLockDelegate );

    /**
     * ConstructL
     * Perform the second phase construction of a CSimLockUIAppUi object
     * this needs to be public due to the way the framework constructs the AppUi
     */
    void ConstructL();

    /**
     * ~CSimLockUIAppUi
     * Destroy the object and release all memory objects
     */
    virtual ~CSimLockUIAppUi();

private: // Private Data Type(s)

    /**
     * Indicate which background text to use
     */
    enum TBackgroundText
        {
        EBackgroundTextEmpty,
        EBackgroundTextMain,
        EBackgroundTextUnlockComplete,
        EBackgroundTextUnlockFailed,
        EBackgroundTextUnlockIncorrect
        };

private: // from CAknAppUi
    /**
     * @see CAknAppUi::HandleCommandL
     */
    virtual void HandleCommandL( TInt aCommand );

    /**
     * @see CAknAppUi::HandleForegroundEventL
     */
    virtual void HandleForegroundEventL( TBool aForeground );

private: // Private Function Declarations

    /**
     * ProcessSimUnlockDialogsL
     * Process the series of dialogs that control the Sim Unlock process
     *
     * @return  Indicates whether or not application flow should continue
     */
    TBool ProcessSimUnlockDialogsL();

    /**
     * CreateBackgroundControlL
     * Create the background control for SimLock UI.  Background contains the default
     * background image for the current skin and text indicating to the user the
     * current stage in the unlock process.
     *
     */
    void CreateBackgroundControlL();

    /**
     * CheckLockStateL
     * Check the state of the lock and display a dialog if it is either
     * unlocked or there are no lock attempts remaining.
     *
     * @return  Indicates whether or not application flow should continue
     */
    TBool CheckLockStateL() const;

    /**
     * DisplayIntroductionDialogL
     * Display introduction dialog
     *
     * @return  Indicates whether or not application flow should continue
     */
    TBool DisplayIntroductionDialogL();

    /**
     * PromptForPasswordL
     * Handle scenario where unlock attempt failed.  Prompt user about whether or
     * not to continue
     *
     * @param   aPassword Descriptor to receive password
     * @return  Indicates whether or not application flow should continue
     */
    TBool PromptForPasswordL( TDes& aPassword );

    /**
     * CompletePromptForPasswordL
     * Ask for second password and check for call in progress
     *
     * @param   aPassword original password entered
     * @param   aNewPassword Descriptor where 2nd password is to be inserted
     * @return  Indicates whether or not application flow should continue
     */
    TBool CompletePromptForPasswordL( const TDesC& aPassword, TDes& aNewPassword );

    /**
     * AttemptToUnlockSimL
     * Ask for second password and check for call in progress
     *
     * @param   aPassword the Sim Lock password
     * @return  Indicates whether or not application flow should continue
     */
    TBool AttemptToUnlockSimL( const TDesC& aPassword );

    /**
     * HandleUnlockFailedL
     * Handle scenario where unlock attempt failed.  Prompt user about whether or
     * not to continue
     *
     * @return  Indicates whether or not application flow should continue
     */
    TBool HandleUnlockFailedL();

    /**
    /**
     * ShowInformationNoteL
     * Show an information note
     *
     * @param   aResourceId indicates what resource is displayed
     * @return  Indicates whether or not application flow should continue 
     */
    TInt ShowInformationNoteL( TInt aResourceId ) const;

    /**
     * SetBackgroundText
     * Change text in background control
     *
     * @param aText indicates type of text
     */
    void SetBackgroundTextL( TBackgroundText aText );

private: // Private Member Data

    /**
     * Control that contains SimLock UI background
     * owns
     */
    CSimLockUIBackgroundControl* iBackgroundControl;

    /**
     * Pointer to "delegate" class that is responsible for non-UI functions related
     * to the phone and SimLock.     
     */
    CSimLockDataHandlingDelegate& iSimLockDelegate;

    /**
     * For foreground event handling, indicate first time run
     */
    TBool iFirstRun;

    /**
     * Proxy used to talk to CTelephony in a synchronous fashion
     * owns
     */
    CSimLockTelephonyProxy* iTelephonyProxy;  
    
    //RProperty for publish key
    RProperty iProperty;

    };


#endif // __SIMLOCKUI_APPUI_H__

// End of file.
