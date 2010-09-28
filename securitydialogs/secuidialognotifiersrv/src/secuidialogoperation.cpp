/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Base class for CSecuiDialog operations
*
*/

#include "secuidialogoperation.h"    // CSecuiDialogOperation
#include "secuidialogoperationobserver.h" // MSecuiDialogOperationObserver
#include <hb/hbwidgets/hbdevicemessageboxsymbian.h> // CHbDeviceMessageBoxSymbian
#include "secuidialogstrace.h"       // TRACE macro


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CSecuiDialogOperation::CSecuiDialogOperation()
// ---------------------------------------------------------------------------
//
CSecuiDialogOperation::CSecuiDialogOperation(
        MSecuiDialogOperationObserver& aObserver, const RMessage2& aMessage,
        TInt aReplySlot ) : CActive( CActive::EPriorityStandard ), iObserver( aObserver ),
        iMessage( aMessage ), iReplySlot( aReplySlot )
    {
    TRACE( "CSecuiDialogOperation::CSecuiDialogOperation, iMessage 0x%08x",
            iMessage.Handle() );
    CActiveScheduler::Add( this );
    }

// ---------------------------------------------------------------------------
// CSecuiDialogOperation::~CSecuiDialogOperation()
// ---------------------------------------------------------------------------
//
CSecuiDialogOperation::~CSecuiDialogOperation()
    {
    TRACE( "CSecuiDialogOperation::~CSecuiDialogOperation, begin" );
    Cancel();
    TRACE( "CSecuiDialogOperation::~CSecuiDialogOperation, end" );
    }

// ---------------------------------------------------------------------------
// CSecuiDialogOperation::RunError()
// ---------------------------------------------------------------------------
//
TInt CSecuiDialogOperation::RunError( TInt aError )
    {
    TRACE( "CSecuiDialogOperation::RunError, aError=%d", aError );
    if( !iMessage.IsNull() )
        {
        TRACE( "CSecuiDialogOperation::RunError, completing message 0x%08x",
                iMessage.Handle() );
        iMessage.Complete( aError );
        }
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// CSecuiDialogOperation::ShowWarningNoteL()
// ---------------------------------------------------------------------------
//
void CSecuiDialogOperation::ShowWarningNoteL( const TDesC& aMessage )
    {
    CHbDeviceMessageBoxSymbian *note =
            CHbDeviceMessageBoxSymbian::NewL( CHbDeviceMessageBoxSymbian::EWarning );
    CleanupStack::PushL( note );
    note->SetTextL( aMessage );
    (void)note->ExecL();    // waits for response or timeout
    CleanupStack::PopAndDestroy( note );
    }

