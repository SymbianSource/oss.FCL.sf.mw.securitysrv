/*
* Copyright (c) 2003 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  WIM Security Dialog Handler. Calls WimSecDlgs to query PIN(s)
*               from user.
*
*/



// INCLUDE FILES
#include    "WimSecurityDlgHandler.h"
#include    "WimTrace.h"


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWimSecurityDlgHandler::CWimSecurityDlgHandler
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CWimSecurityDlgHandler::CWimSecurityDlgHandler()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSecurityDlgHandler::CWimSecurityDlgHandler | Begin"));
    }

// -----------------------------------------------------------------------------
// CWimSecurityDlgHandler::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CWimSecurityDlgHandler::ConstructL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSecurityDlgHandler::ConstructL | Begin"));
    }

// -----------------------------------------------------------------------------
// CWimSecurityDlgHandler::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CWimSecurityDlgHandler* CWimSecurityDlgHandler::NewL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSecurityDlgHandler::NewL | Begin"));
    CWimSecurityDlgHandler* self = new( ELeave ) CWimSecurityDlgHandler;
    
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    return self;
    }

    
// Destructor
CWimSecurityDlgHandler::~CWimSecurityDlgHandler()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSecurityDlgHandler::~CWimSecurityDlgHandler | Begin"));
    if ( iSecurityDlg )
        {
        _WIMTRACE(_L("WIM | WIMServer | CWimSecurityDlgHandler::~CWimSecurityDlgHandler | Release dialog"));
        iSecurityDlg->Release();
        _WIMTRACE(_L("WIM | WIMServer | CWimSecurityDlgHandler::~CWimSecurityDlgHandler | Dialog released"));
        }
    /*if ( iPKIDlg )
        {    
        iPKIDlg->Release();
        }*/
    iLibrary.Close();
    _WIMTRACE(_L("WIM | WIMServer | CWimSecurityDlgHandler::~CWimSecurityDlgHandler | Library closed"));
    }

// -----------------------------------------------------------------------------
// CWimSecurityDlgHandler::EnablePIN
// Shows a security dialog for fetching PIN value from the user for enabling or 
// disabling PIN query.
// -----------------------------------------------------------------------------
//
void CWimSecurityDlgHandler::EnablePIN(
    TBool aEnable,
    TBool aRetry, 
    const TPINParams& aPINParams, 
    TPINValue& aPINValue,
    TRequestStatus& aStatus )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSecurityDlgHandler::GetPINValue | Begin"));

    // First try to load security dialog
    TInt loadError = LoadSecurityDialog( aStatus );

    if ( loadError == KErrNone ) // Dialog loaded succesfully
        {
        if ( aEnable )
            {
            iSecurityDlg->EnablePIN( aPINParams, aRetry, aPINValue, aStatus );
            }
        else
            {
            iSecurityDlg->DisablePIN( aPINParams, aRetry, aPINValue, aStatus );
            }
        }
    }

// -----------------------------------------------------------------------------
// CWimSecurityDlgHandler::ChangePINValue
// Shows a security dialog for changing PIN value.
// -----------------------------------------------------------------------------
//
void CWimSecurityDlgHandler::ChangePINValue(
    TBool aRetry,
    const TPINParams& aPINParams, 
    TPINValue& aOldPINValue,
    TPINValue& aNewPINValue,
    TRequestStatus& aStatus )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSecurityDlgHandler::ChangePINValue | Begin"));

    // First try to load security dialog
    TInt loadError = LoadSecurityDialog( aStatus );

    if ( loadError == KErrNone ) // Dialog loaded succesfully
        {
        iSecurityDlg->ChangePIN( aPINParams, 
                                 aRetry, 
                                 aOldPINValue, 
                                 aNewPINValue, 
                                 aStatus );
        }
    }

// -----------------------------------------------------------------------------
// CWimSecurityDlgHandler::ShowPINBlocked
// Shows the note about blocked PIN to the user.
// -----------------------------------------------------------------------------
//
void CWimSecurityDlgHandler::ShowPINBlocked( 
    const TPINParams& aPINParams, 
    TRequestStatus& aStatus )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSecurityDlgHandler::ShowPINBlocked | Begin"));
    // First try to load security dialog
    TInt loadError = LoadSecurityDialog( aStatus );

    if ( loadError == KErrNone ) // Dialog loaded succesfully
        {
        iSecurityDlg->PINBlocked( aPINParams, aStatus );
        }
    }
    
// -----------------------------------------------------------------------------
// CWimSecurityDlgHandler::ShowPINTotalBlocked
// Shows the note about total blocked PIN to the user.
// -----------------------------------------------------------------------------
//
void CWimSecurityDlgHandler::ShowPINTotalBlocked( 
    const TPINParams& aPINParams, 
    TRequestStatus& aStatus )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSecurityDlgHandler::ShowPINTotalBlocked | Begin"));
    // First try to load security dialog
    TInt loadError = LoadSecurityDialog( aStatus );

    if ( loadError == KErrNone ) // Dialog loaded succesfully
        {
        iSecurityDlg->TotalBlocked( aPINParams, aStatus );
        }
    }    

// -----------------------------------------------------------------------------
// CWimSecurityDlgHandler::ShowCardRemoved
// Shows the note about card removed to the user.
// -----------------------------------------------------------------------------
//
void CWimSecurityDlgHandler::ShowCardRemoved( 
    TRequestStatus& /*aStatus*/ )
    {
    /*_WIMTRACE(_L("WIM | WIMServer | CWimSecurityDlgHandler::ShowCardRemoved | Begin"));
    // First try to load security dialog
    TInt loadError = LoadPKIDialog( aStatus );

    if ( loadError == KErrNone ) // Dialog loaded succesfully
        {
        iPKIDlg->Note( MPKIDialog::ENoSecurityModule, aStatus );
        }*/
    return;
    }



// -----------------------------------------------------------------------------
// CWimSecurityDlgHandler::UnblockPIN
// Shows a security dialog for fetching PIN and PUK values 
// for unblocking blocked PIN.
// -----------------------------------------------------------------------------
//
void CWimSecurityDlgHandler::UnblockPIN(
    TBool aRetry,
    const TPINParams& aBlockedPINParams,
    const TPINParams& aUnblockingPINParams, 
    TPINValue& aUnblockingPINValue, 
    TPINValue& aNewPINValue,
    TRequestStatus& aStatus )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSecurityDlgHandler::UnblockPIN | Begin"));

    // First try to load security dialog
    TInt loadError = LoadSecurityDialog( aStatus );

    if ( loadError == KErrNone ) // Dialog loaded succesfully
        {
        iSecurityDlg->UnblockPIN( aBlockedPINParams,
                                  aUnblockingPINParams, 
                                  aRetry,
                                  aUnblockingPINValue, 
                                  aNewPINValue,
                                  aStatus );
        }
    }

// -----------------------------------------------------------------------------
// CWimSecurityDlgHandler::EnterPIN
// Shows a security dialog for asking user to enter PIN code.
// -----------------------------------------------------------------------------
//
void CWimSecurityDlgHandler::EnterPIN( 
    TBool aRetry, 
    const TPINParams& aPINParams, 
    TPINValue& aPINValue,
    TRequestStatus& aStatus )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSecurityDlgHandler::EnterPIN | Begin"));

    // First try to load security dialog
    TInt loadError = LoadSecurityDialog( aStatus );

    if ( loadError == KErrNone ) // Dialog loaded succesfully
        {
        _WIMTRACE(_L("WIM | WIMServer | CWimSecurityDlgHandler::EnterPIN | Call EnterPIN"));
        iSecurityDlg->EnterPIN( aPINParams, aRetry, aPINValue, aStatus );
        _WIMTRACE(_L("WIM | WIMServer | CWimSecurityDlgHandler::EnterPIN | EnterPIN called"));
        }
    }

// -----------------------------------------------------------------------------
// CWimSecurityDlgHandler::Close()
// -----------------------------------------------------------------------------
//
void CWimSecurityDlgHandler::Cancel()
    {
	TRAPD( err, DoLoadSecurityDialogL() );
	if ( err == KErrNone ) // Dialog loaded succesfully
        {
        _WIMTRACE(_L("WIM | WIMServer | CWimSecurityDlgHandler::EnterPIN | Call EnterPIN"));
        iSecurityDlg->Cancel();
        _WIMTRACE(_L("WIM | WIMServer | CWimSecurityDlgHandler::EnterPIN | EnterPIN called"));
        }
    }

// -----------------------------------------------------------------------------
// CWimSecurityDlgHandler::LoadSecurityDialog
// Load Security Dialog if not yet done. Completes message with error if load
// fails.
// -----------------------------------------------------------------------------
//
TInt CWimSecurityDlgHandler::LoadSecurityDialog( TRequestStatus& aStatus )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSecurityDlgHandler::LoadSecurityDialog | Begin"));

    // Load CTSecDlgs.dll, trap if leave occurred
    TRAPD( err, DoLoadSecurityDialogL() );
    
    if ( err ) // Dll load failed, complete message with error
        {
        _WIMTRACE2(_L("WIM | WIMServer | CWimSecurityDlgHandler::LoadSecurityDialog | DoLoadSecurityDialogL()=%d"), err);
        TRequestStatus* status = &aStatus;
        User::RequestComplete( status, err );
        }
    return err;
    }
    
// -----------------------------------------------------------------------------
// CWimSecurityDlgHandler::LoadPKIDialog
// Load PKI Dialog if not yet done. Completes message with error if load
// fails.
// -----------------------------------------------------------------------------
//
TInt CWimSecurityDlgHandler::LoadPKIDialog( TRequestStatus& /*aStatus*/ )
    {
    //Keep it for future usage 
    
    /*_WIMTRACE(_L("WIM | WIMServer | CWimPKIDlgHandler::LoadPKIDialog | Begin"));

    // Load CTSecDlgs.dll, trap if leave occurred
    TRAPD( err, DoLoadPKIDialogL() );
    
    if ( err ) // Dll load failed, complete message with error
        {
        _WIMTRACE2(_L("WIM | WIMServer | CWimSecurityDlgHandler::LoadPKIDialog | DoLoadPKIDialogL()=%d"), err);
        TRequestStatus* status = &aStatus;
        User::RequestComplete( status, err );
        }
    return err;*/
    return 0;
    }


// -----------------------------------------------------------------------------
// CWimSecurityDlgHandler::DoLoadSecurityDialogL
// Load security dialog if not yet done
// -----------------------------------------------------------------------------
//
void CWimSecurityDlgHandler::DoLoadSecurityDialogL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSecurityDlgHandler::DoLoadSecurityDialogL | Begin"));

    if ( !iSecurityDlg ) // Dialog not loaded yet
        {
        // Load security dialog dll

        iSecurityDlg = SecurityDialogFactory::CreateL();

        _WIMTRACE2(_L("WIM | WIMServer | CWimSecurityDlgHandler::LoadSecurityDialog | iSecurityDlg=%d"), iSecurityDlg );
        }
    }

// -----------------------------------------------------------------------------
// CWimSecurityDlgHandler::DoLoadPKIDialogL
// Load PKI dialog if not yet done
// -----------------------------------------------------------------------------
//
void CWimSecurityDlgHandler::DoLoadPKIDialogL()
    {
    //iPKIDlg = PKIDialogFactory::CreateNoteL();
    return;
    }



//  End of File
