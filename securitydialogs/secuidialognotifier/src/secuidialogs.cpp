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
* Description:  CSecuiDialogs active object
*
*/

#include "secuidialogs.h"            // CSecuiDialogs
#include "secuidialogstrace.h"       // TRACE macro

#include <securitynotification.h>

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CSecuiDialogs::NewL()
// ---------------------------------------------------------------------------
//
CSecuiDialogs* CSecuiDialogs::NewL(  TBool& aIsDeleted )
    {
    return new( ELeave ) CSecuiDialogs( aIsDeleted );
    }

// ---------------------------------------------------------------------------
// CSecuiDialogs::~CSecuiDialogs()
// ---------------------------------------------------------------------------
//
CSecuiDialogs::~CSecuiDialogs()
    {
    TRACE( "CSecuiDialogs::~CSecuiDialogs, begin" );
    Cancel();
    iServer.Close();
    delete iInputBuffer;
    iInputBuffer = NULL;
    delete iOutputBuffer;
    iOutputBuffer = NULL;
    iIsDeleted = ETrue;
    TRACE( "CSecuiDialogs::~CSecuiDialogs, end" );
    }

// ---------------------------------------------------------------------------
// CSecuiDialogs::StartLD()
// ---------------------------------------------------------------------------
//
void CSecuiDialogs::StartLD( const TDesC8& aBuffer, TInt aReplySlot,
        const RMessagePtr2& aMessage )
    {
    TRACE( "CSecuiDialogs::StartLD, begin" );
    User::LeaveIfError( iServer.Connect() );

    const TInt* ptr = reinterpret_cast< const TInt* >( aBuffer.Ptr() );
    iOperation = static_cast< TSecurityDialogOperation >( *ptr & KSecurityDialogOperationMask );
    iReplySlot = aReplySlot;
    TRACE( "CSecuiDialogs::StartLD, iReplySlot 0x%08x", iReplySlot );

    TRACE( "CSecuiDialogs::StartLD, message 0x%08x", iMessagePtr.Handle() );
    iMessagePtr = aMessage;

    TRACE( "CSecuiDialogs::StartLD, iOperation=%d", iOperation );

    TSecurityNotificationPckg pckg;
    pckg.Copy( aBuffer );
    TRACE( "CSecuiDialogs::StartLD, Copy=%d", 1 );
    TInt iStartup = pckg().iStartup;
    TRACE( "CSecuiDialogs::StartLD, iStartup=%d", iStartup );
    TInt iEvent = pckg().iEvent;
    TRACE( "CSecuiDialogs::StartLD, iEvent=%d", iEvent );
		TInt lOperation = 0x0000;
    if(iStartup)
			lOperation = 0x1000;
		lOperation += iEvent;
    iOperation = static_cast< TSecurityDialogOperation >( lOperation );
    TRACE( "CSecuiDialogs::StartLD, new iOperation=%d", iOperation );

    __ASSERT_DEBUG( iOutputBuffer == NULL, User::Invariant() );
    TInt outputBufLen = 0;
    if( iEvent < 0x100 || iEvent == 0x106 /* from Autolock*/)	// a simple test to prevent unknown codes. Nevertheless they will also be stopped later in case that no dialog can answer the request
        {
            iOutputBuffer = new( ELeave ) TPINValueBuf;
            outputBufLen = sizeof( TPINValueBuf );
        }
    else
    		{
        		TRACE( "CSecuiDialogs::StartLD, not allowed iOperation =%d", iOperation );
            User::Leave( KErrNotSupported );
        }

    __ASSERT_DEBUG( iInputBuffer == NULL, User::Invariant() );
    iInputBuffer = aBuffer.AllocL();

    if( iOutputBuffer )
        {
        TRACE( "CSecuiDialogs::StartLD, iOutputPtr.Set outputBufLen=%d", outputBufLen );
        iOutputPtr.Set( static_cast< TUint8* >( iOutputBuffer ), outputBufLen, outputBufLen );
        iServer.SecuiDialogOperation( iOperation, *iInputBuffer, iOutputPtr, iStatus );
        }
    else
        {
        TRACE( "CSecuiDialogs::StartLD, Leave KErrNotSupported=%d", KErrNotSupported );
        User::Leave( KErrNotSupported );
        }
    SetActive();
    TRACE( "CSecuiDialogs::StartLD, end" );
    }

// ---------------------------------------------------------------------------
// CSecuiDialogs::RunL()
// ---------------------------------------------------------------------------
//
void CSecuiDialogs::RunL()
    {
    TRACE( "CSecuiDialogs::RunL, iStatus.Int()=%d", iStatus.Int() );
    TInt error = iStatus.Int();
    User::LeaveIfError( error );
    __ASSERT_DEBUG( iOutputPtr.Ptr(), User::Invariant() );
    TRACE( "CSecuiDialogs::RunL, calling iMessagePtr.WriteL" );
    TRACE( "CSecuiDialogs::RunL, iReplySlot 0x%08x", iReplySlot );
    TInt maxx = iMessagePtr.GetDesMaxLength(iReplySlot);
    TRACE( "CSecuiDialogs::RunL, maxx 0x%08x", maxx );
    TInt curr = iMessagePtr.GetDesLength(iReplySlot);
    TRACE( "CSecuiDialogs::RunL, curr 2 0x%08x", curr );
    // no need to copy. Besides, it seems to crash because it's too long
    // iMessagePtr.WriteL( iReplySlot, iOutputPtr );
    TRACE( "CSecuiDialogs::RunL, called iMessagePtr.WriteL" );

    TRACE( "CSecuiDialogs::RunL, completing message 0x%08x", iMessagePtr.Handle() );
    iMessagePtr.Complete( error );

    TRACE( "CSecuiDialogs::RunL, deleting this" );
    delete this;
    TRACE( "CSecuiDialogs::RunL, end" );
    }

// ---------------------------------------------------------------------------
// CSecuiDialogs::DoCancel()
// ---------------------------------------------------------------------------
//
void CSecuiDialogs::DoCancel()
    {
    TRACE( "CSecuiDialogs::DoCancel, begin" );
    iServer.CancelOperation();
    if( !iMessagePtr.IsNull() )
        {
        TRACE( "CSecuiDialogs::DoCancel, completing message 0x%08x", iMessagePtr.Handle() );
        iMessagePtr.Complete( KErrCancel );
        }
    TRACE( "CSecuiDialogs::DoCancel(), end" );
    }

// ---------------------------------------------------------------------------
// CSecuiDialogs::RunError()
// ---------------------------------------------------------------------------
//
TInt CSecuiDialogs::RunError( TInt aError )
    {
    TRACE( "CSecuiDialogs::RunError, aError=%d", aError );
    if( !iMessagePtr.IsNull() )
        {
        TRACE( "CSecuiDialogs::RunError, completing message 0x%08x", iMessagePtr.Handle() );
        iMessagePtr.Complete( aError );
        }

    TRACE( "CSecuiDialogs::RunError, deleting this" );
    delete this;

    TRACE( "CSecuiDialogs::RunError, end" );
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// CSecuiDialogs::CSecuiDialogs()
// ---------------------------------------------------------------------------
//
CSecuiDialogs::CSecuiDialogs( TBool& aIsDeleted ) : CActive( CActive::EPriorityLow ),
        iIsDeleted( aIsDeleted ), iOutputPtr( NULL, 0, 0 )
    {
    TRACE( "CSecuiDialogs::CSecuiDialogs" );
    CActiveScheduler::Add( this );
    iIsDeleted = EFalse;
    }

