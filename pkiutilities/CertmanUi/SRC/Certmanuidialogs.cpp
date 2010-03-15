/*
* Copyright (c) 2003-2010 Nokia Corporation and/or its subsidiary(-ies).
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
// ---------------------------------------------------------
//
CCertManUIWaitDialog::CCertManUIWaitDialog()
    {
    }

// ---------------------------------------------------------
// CCertManUiDialog::~CCertManUiDialogs()
// ---------------------------------------------------------
//
CCertManUIWaitDialog::~CCertManUIWaitDialog()
    {
    if( iDialog )
        {
        delete iDialog;
        iDialog = NULL;
        }
    }

//---------------------------------------------------------------
// CCertManUiDialog::StartWaitDialogL()
//---------------------------------------------------------------
//
void CCertManUIWaitDialog::StartWaitDialogL( TInt aDialogSelector )
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUIWaitDialog::StartWaitDialogL" );

    if( !iDialog )
        {
        iDialog = new( ELeave ) CAknWaitDialog(
                reinterpret_cast< CEikDialog** >( &iDialog ), ETrue );

        TInt dialog = 0;
        switch( aDialogSelector )
            {
            case ECertmanUiDeleteDialog:
                LOG_WRITE( "Show delete note" );
                dialog = R_CERTMANUI_DELETE_WAIT_NOTE;
                break;

            case ECertmanUiWaitDialog:
                LOG_WRITE( "Show wait note" );
                dialog = R_CERTMANUI_WAIT_NOTE;
                break;

            default:
                __ASSERT_DEBUG( EFalse, User::Panic( KDesCertmanUiFault, EBadDialogSelector ) );
                break;
            }

        iDialog->ExecuteLD( dialog );
        }

    ++iOpenCount;

    CERTMANUILOGGER_LEAVEFN( "CCertManUIWaitDialog::StartWaitDialogL" );
    }

//---------------------------------------------------------------
// CCertManUiDialog::CloseWaitDialogL
//---------------------------------------------------------------
//
void CCertManUIWaitDialog::CloseWaitDialogL()
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUIWaitDialog::CloseWaitDialogL" );

    --iOpenCount;

    if( iDialog && !iOpenCount )
        {
        TRAPD( err, iDialog->ProcessFinishedL() );
        if( !err )
            {
            delete iDialog;
            iDialog = NULL;
            }
        }

    CERTMANUILOGGER_LEAVEFN( "CCertManUIWaitDialog::CloseWaitDialogL" );
    }

