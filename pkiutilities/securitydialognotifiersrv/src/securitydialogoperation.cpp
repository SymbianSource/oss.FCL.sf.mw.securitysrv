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
* Description:  Base class for CSecurityDialog operations
*
*/

#include "securitydialogoperation.h"    // CSecurityDialogOperation
#include "securitydialogoperationobserver.h" // MSecurityDialogOperationObserver
#include "securitydialogstrace.h"       // TRACE macro


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CSecurityDialogOperation::CSecurityDialogOperation()
// ---------------------------------------------------------------------------
//
CSecurityDialogOperation::CSecurityDialogOperation(
        MSecurityDialogOperationObserver& aObserver, const RMessage2& aMessage,
        TInt aReplySlot ) : CActive( CActive::EPriorityStandard ), iObserver( aObserver ),
        iMessage( aMessage ), iReplySlot( aReplySlot )
    {
    TRACE( "CSecurityDialogOperation::CSecurityDialogOperation, iMessage 0x%08x",
            iMessage.Handle() );
    CActiveScheduler::Add( this );
    }

// ---------------------------------------------------------------------------
// CSecurityDialogOperation::~CSecurityDialogOperation()
// ---------------------------------------------------------------------------
//
CSecurityDialogOperation::~CSecurityDialogOperation()
    {
    TRACE( "CSecurityDialogOperation::~CSecurityDialogOperation, begin" );
    Cancel();
    TRACE( "CSecurityDialogOperation::~CSecurityDialogOperation, end" );
    }

// ---------------------------------------------------------------------------
// CSecurityDialogOperation::RunError()
// ---------------------------------------------------------------------------
//
TInt CSecurityDialogOperation::RunError( TInt aError )
    {
    TRACE( "CSecurityDialogOperation::RunError, aError=%d", aError );
    if( !iMessage.IsNull() )
        {
        TRACE( "CSecurityDialogOperation::RunError, completing message 0x%08x",
                iMessage.Handle() );
        iMessage.Complete( aError );
        }
    return KErrNone;
    }

