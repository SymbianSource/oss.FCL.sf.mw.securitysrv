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
    RDEBUG("0", 0);
    Cancel();
    iServer.Close();
    delete iInputBuffer;
    iInputBuffer = NULL;
    delete iOutputBuffer;
    iOutputBuffer = NULL;
    iIsDeleted = ETrue;
    RDEBUG("0x99", 0x99);
    }

// ---------------------------------------------------------------------------
// CSecuiDialogs::StartLD()
// ---------------------------------------------------------------------------
//
void CSecuiDialogs::StartLD( const TDesC8& aBuffer, TInt aReplySlot,
        const RMessagePtr2& aMessage )
    {
    RDEBUG("0", 0);
    User::LeaveIfError( iServer.Connect() );

    const TInt* ptr = reinterpret_cast< const TInt* >( aBuffer.Ptr() );
    iOperation = static_cast< TSecurityDialogOperation >( *ptr & KSecurityDialogOperationMask );
    iReplySlot = aReplySlot;
    RDEBUG("iReplySlot", iReplySlot);

    RDEBUG("iMessagePtr.Handle()", iMessagePtr.Handle());
    iMessagePtr = aMessage;

    RDEBUG("iOperation", iOperation);

    TSecurityNotificationPckg pckg;
    pckg.Copy( aBuffer );
    RDEBUG("0", 0);
    TInt iStartup = pckg().iStartup;
    RDEBUG("iStartup", iStartup);
    TInt iEvent = pckg().iEvent;
    RDEBUG("iEvent", iEvent);
		TInt lOperation = 0x0000;
    if(iStartup)
			lOperation = 0x1000;
		lOperation += iEvent;
    iOperation = static_cast< TSecurityDialogOperation >( lOperation );
    RDEBUG("new iOperation", iOperation);

    __ASSERT_DEBUG( iOutputBuffer == NULL, User::Invariant() );
    TInt outputBufLen = 0;
    if( iEvent < 0x100 || iEvent == 0x106 /* from Autolock*/)	// a simple test to prevent unknown codes. Nevertheless they will also be stopped later in case that no dialog can answer the request
        {
            iOutputBuffer = new( ELeave ) TPINValueBuf;
            outputBufLen = sizeof( TPINValueBuf );
        }
    else
    		{
        		RDEBUG("not allowed iOperation", iOperation);
            User::Leave( KErrNotSupported );
        }

    __ASSERT_DEBUG( iInputBuffer == NULL, User::Invariant() );
    iInputBuffer = aBuffer.AllocL();

    if( iOutputBuffer )
        {
        RDEBUG("outputBufLen", outputBufLen);
        iOutputPtr.Set( static_cast< TUint8* >( iOutputBuffer ), outputBufLen, outputBufLen );
        iServer.SecuiDialogOperation( iOperation, *iInputBuffer, iOutputPtr, iStatus );
        }
    else
        {
        RDEBUG("KErrNotSupported", KErrNotSupported);
        User::Leave( KErrNotSupported );
        }
    SetActive();
    RDEBUG("0x99", 0x99);
    }

// ---------------------------------------------------------------------------
// CSecuiDialogs::RunL()
// ---------------------------------------------------------------------------
//
void CSecuiDialogs::RunL()
    {
    RDEBUG("0", 0);
    TInt error = iStatus.Int();
    RDEBUG("error", error);
    User::LeaveIfError( error );
    __ASSERT_DEBUG( iOutputPtr.Ptr(), User::Invariant() );
    RDEBUG("iReplySlot", iReplySlot);
    TInt maxx = iMessagePtr.GetDesMaxLength(iReplySlot);
    RDEBUG("maxx", maxx);
    TInt curr = iMessagePtr.GetDesLength(iReplySlot);
    RDEBUG("curr", curr);
    // no need to copy. Besides, it seems to crash because it's too long
    // iMessagePtr.WriteL( iReplySlot, iOutputPtr );
    RDEBUG("not called WriteL", 0);

    RDEBUG("completing iMessagePtr.Handle()", iMessagePtr.Handle());
    iMessagePtr.Complete( error );

    RDEBUG("0", 0);
    delete this;
    RDEBUG("0x99", 0x99);
    }

// ---------------------------------------------------------------------------
// CSecuiDialogs::DoCancel()
// ---------------------------------------------------------------------------
//
void CSecuiDialogs::DoCancel()
    {
    RDEBUG("0", 0);
    iServer.CancelOperation();
    if( !iMessagePtr.IsNull() )
        {
        RDEBUG("completing iMessagePtr.Handle()", iMessagePtr.Handle());
        iMessagePtr.Complete( KErrCancel );
        }
    RDEBUG("0x99", 0x99);
    }

// ---------------------------------------------------------------------------
// CSecuiDialogs::RunError()
// ---------------------------------------------------------------------------
//
TInt CSecuiDialogs::RunError( TInt aError )
    {
    RDEBUG("aError", aError);
    if( !iMessagePtr.IsNull() )
        {
        RDEBUG("completing iMessagePtr.Handle()", iMessagePtr.Handle());
        iMessagePtr.Complete( aError );
        }

    RDEBUG("0", 0);
    delete this;

    RDEBUG("0x99", 0x99);
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// CSecuiDialogs::CSecuiDialogs()
// ---------------------------------------------------------------------------
//
CSecuiDialogs::CSecuiDialogs( TBool& aIsDeleted ) : CActive( CActive::EPriorityLow ),
        iIsDeleted( aIsDeleted ), iOutputPtr( NULL, 0, 0 )
    {
    RDEBUG("0", 0);
    CActiveScheduler::Add( this );
    iIsDeleted = EFalse;
    }

