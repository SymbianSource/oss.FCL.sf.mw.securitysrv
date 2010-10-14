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
    RDEBUG("aMessage.Handle()", aMessage.Handle());
    CActiveScheduler::Add( this );
    }

// ---------------------------------------------------------------------------
// CSecuiDialogOperation::~CSecuiDialogOperation()
// ---------------------------------------------------------------------------
//
CSecuiDialogOperation::~CSecuiDialogOperation()
    {
    RDEBUG("0", 0);
    Cancel();
    RDEBUG("0x99", 0x99);
    }

// ---------------------------------------------------------------------------
// CSecuiDialogOperation::RunError()
// ---------------------------------------------------------------------------
//
TInt CSecuiDialogOperation::RunError( TInt aError )
    {
    RDEBUG("aError", aError);
    if( !iMessage.IsNull() )
        {
        RDEBUG("completing message iMessage.Handle()", iMessage.Handle());
        iMessage.Complete( aError );
        }
    RDEBUG("0x99", 0x99);
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// CSecuiDialogOperation::ShowWarningNoteL()
// ---------------------------------------------------------------------------
//
void CSecuiDialogOperation::ShowWarningNoteL( const TDesC& /* aMessage */ )
    {
    RDEBUG("not used 0", 0);
    }

