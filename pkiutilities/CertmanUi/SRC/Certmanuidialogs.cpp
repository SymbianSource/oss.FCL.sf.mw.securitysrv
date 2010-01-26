/*
* Copyright (c) 2003-2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Implementation of class CertManUIDialogs
*
*/


// INCLUDE FILES

#include <AknWaitDialog.h>
#include <certmanui.rsg>
#include "Certmanuidialogs.h"
#include "CertManUILogger.h"
#include "CertmanuiCommon.h"


#ifdef _DEBUG
_LIT( KDesCertmanUiFault, "CertmanUi" );
#endif

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CCertManUiDialog::CCertManUiDialogs()
// Default c++ constructor
// ---------------------------------------------------------
CCertManUIWaitDialog::CCertManUIWaitDialog()
    {
    iDialog = NULL;
    }

// ---------------------------------------------------------
// CCertManUiDialog::~CCertManUiDialogs()
// Destructor
// ---------------------------------------------------------
CCertManUIWaitDialog::~CCertManUIWaitDialog()
    {
    if ( iDialog )
        {
        delete iDialog;
        }
    }

//---------------------------------------------------------------
// CCertManUiDialog::StartWaitDialogL
// Displays wait dialog
//---------------------------------------------------------------
void CCertManUIWaitDialog::StartWaitDialogL( TInt aDialogSelector )
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUIWaitDialog::StartWaitDialogL" );
    TInt dialog = 0;

    // If iDisplayed flag is true, but iDialog is NULL we have had leave.
    // Reset iDisplayed to normalize situation
    if(iDisplayed && iDialog == NULL)
        {
        iDisplayed = EFalse;
        }

    if(!iDisplayed)
        {
        if( !iDialog )
           {
            iDialog = new ( ELeave )CAknWaitDialog( REINTERPRET_CAST(CEikDialog**,
                &iDialog), ETrue );	
           } 


        //CleanupStack::PushL(TCleanupItem(CleanupWaitDialog, (TAny**)&iDialog));

        switch(aDialogSelector)
            {
            case ECertmanUiDeleteDialog:
                LOG_WRITE("Show delete note");
                dialog = R_CERTMANUI_DELETE_WAIT_NOTE;
                break;

            case ECertmanUiWaitDialog:
                LOG_WRITE("Show wait note");
                dialog = R_CERTMANUI_WAIT_NOTE;
                break;
            default:
                __ASSERT_DEBUG( EFalse, User::Panic(KDesCertmanUiFault,
                    EBadDialogSelector));
                break;
            }

        iDialog->ExecuteLD(dialog);
        iDisplayed = ETrue;
        }

    CERTMANUILOGGER_LEAVEFN( "CCertManUIWaitDialog::StartWaitDialogL" );
    }

//---------------------------------------------------------------
// CCertManUiDialog::CloseWaitDialogL
// Close wait dialog
//---------------------------------------------------------------
void CCertManUIWaitDialog::CloseWaitDialogL()
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUIWaitDialog::CloseWaitDialogL" );

    if( iDialog )
        {
        iDialog->ProcessFinishedL();
        //CleanupStack::PopAndDestroy();
        }

    iDisplayed = EFalse;
    CERTMANUILOGGER_LEAVEFN( "CCertManUIWaitDialog::CloseWaitDialogL" );
    }

// ---------------------------------------------------------
// CCertManUIWaitDialog::CleanupWaitDialog
//
// ---------------------------------------------------------
void CCertManUIWaitDialog::CleanupWaitDialog(TAny* aAny)
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUIWaitDialog::CleanupWaitDialog" );

    CAknWaitDialog** dialog = (CAknWaitDialog**) aAny;
    if ( dialog && *dialog )
        {
        delete *dialog;
        }
    CERTMANUILOGGER_LEAVEFN( "CCertManUIWaitDialog::CleanupWaitDialog" );
    }


// End of file
