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
* Description:  CSecurityDialogs active object
*
*/

#include "securitydialogs.h"            // CSecurityDialogs
#include "securitydialogstrace.h"       // TRACE macro


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CSecurityDialogs::NewL()
// ---------------------------------------------------------------------------
//
CSecurityDialogs* CSecurityDialogs::NewL(  TBool& aIsDeleted )
    {
    return new( ELeave ) CSecurityDialogs( aIsDeleted );
    }

// ---------------------------------------------------------------------------
// CSecurityDialogs::~CSecurityDialogs()
// ---------------------------------------------------------------------------
//
CSecurityDialogs::~CSecurityDialogs()
    {
    TRACE( "CSecurityDialogs::~CSecurityDialogs, begin" );
    Cancel();
    iServer.Close();
    delete iInputBuffer;
    iInputBuffer = NULL;
    delete iOutputBuffer;
    iOutputBuffer = NULL;
    iIsDeleted = ETrue;
    TRACE( "CSecurityDialogs::~CSecurityDialogs, end" );
    }

// ---------------------------------------------------------------------------
// CSecurityDialogs::StartLD()
// ---------------------------------------------------------------------------
//
void CSecurityDialogs::StartLD( const TDesC8& aBuffer, TInt aReplySlot,
        const RMessagePtr2& aMessage )
    {
    TRACE( "CSecurityDialogs::StartLD, begin" );
    User::LeaveIfError( iServer.Connect() );

    const TInt* ptr = reinterpret_cast< const TInt* >( aBuffer.Ptr() );
    iOperation = static_cast< TSecurityDialogOperation >( *ptr & KSecurityDialogOperationMask );
    iReplySlot = aReplySlot;

    TRACE( "CSecurityDialogs::StartLD, message 0x%08x", iMessagePtr.Handle() );
    iMessagePtr = aMessage;

    TRACE( "CSecurityDialogs::StartLD, iOperation=%d", iOperation );
    __ASSERT_DEBUG( iOutputBuffer == NULL, User::Invariant() );
    TInt outputBufLen = 0;
    switch( iOperation )
        {
        case ESecureConnection:
        case ESignText:
            iOutputBuffer = new( ELeave ) TCTTokenObjectHandleBuf;
            outputBufLen = sizeof( TCTTokenObjectHandleBuf );
            break;
        case EEnterPIN:
        case EEnablePIN:
        case EDisablePIN:
            iOutputBuffer = new( ELeave ) TPINValueBuf;
            outputBufLen = sizeof( TPINValueBuf );
            break;
        case EChangePIN:
        case EUnblockPIN:
            iOutputBuffer = new( ELeave ) TTwoPINOutputBuf;
            outputBufLen = sizeof( TTwoPINOutputBuf );
            break;
        case EPINBlocked:
        case ETotalBlocked:
            break;
        case EUnblockPINInClear:
            iOutputBuffer = new( ELeave ) TUnblockPINInClearOutputBuf;
            outputBufLen = sizeof( TUnblockPINInClearOutputBuf );
            break;
        case EServerAuthenticationFailure:
            iOutputBuffer = new( ELeave ) TServerAuthenticationFailureOutputBuf;
            outputBufLen = sizeof( TServerAuthenticationFailureOutputBuf );
            break;
        default:
            User::Leave( KErrNotSupported );
            break;
        }

    __ASSERT_DEBUG( iInputBuffer == NULL, User::Invariant() );
    iInputBuffer = aBuffer.AllocL();

    if( iOutputBuffer )
        {
        iOutputPtr.Set( static_cast< TUint8* >( iOutputBuffer ), outputBufLen, outputBufLen );
        iServer.SecurityDialogOperation( iOperation, *iInputBuffer, iOutputPtr, iStatus );
        }
    else
        {
        User::Leave( KErrNotSupported );
        }
    SetActive();
    TRACE( "CSecurityDialogs::StartLD, end" );
    }

// ---------------------------------------------------------------------------
// CSecurityDialogs::RunL()
// ---------------------------------------------------------------------------
//
void CSecurityDialogs::RunL()
    {
    TRACE( "CSecurityDialogs::RunL, iStatus.Int()=%d", iStatus.Int() );
    TInt error = iStatus.Int();
    User::LeaveIfError( error );
    __ASSERT_DEBUG( iOutputPtr.Ptr(), User::Invariant() );
    iMessagePtr.WriteL( iReplySlot, iOutputPtr );

    TRACE( "CSecurityDialogs::RunL, completing message 0x%08x", iMessagePtr.Handle() );
    iMessagePtr.Complete( error );

    TRACE( "CSecurityDialogs::RunL, deleting this" );
    delete this;
    TRACE( "CSecurityDialogs::RunL, end" );
    }

// ---------------------------------------------------------------------------
// CSecurityDialogs::DoCancel()
// ---------------------------------------------------------------------------
//
void CSecurityDialogs::DoCancel()
    {
    TRACE( "CSecurityDialogs::DoCancel, begin" );
    iServer.CancelOperation();
    if( !iMessagePtr.IsNull() )
        {
        TRACE( "CSecurityDialogs::DoCancel, completing message 0x%08x", iMessagePtr.Handle() );
        iMessagePtr.Complete( KErrCancel );
        }
    TRACE( "CSecurityDialogs::DoCancel(), end" );
    }

// ---------------------------------------------------------------------------
// CSecurityDialogs::RunError()
// ---------------------------------------------------------------------------
//
TInt CSecurityDialogs::RunError( TInt aError )
    {
    TRACE( "CSecurityDialogs::RunError, aError=%d", aError );
    if( !iMessagePtr.IsNull() )
        {
        TRACE( "CSecurityDialogs::RunError, completing message 0x%08x", iMessagePtr.Handle() );
        iMessagePtr.Complete( aError );
        }

    TRACE( "CSecurityDialogs::RunError, deleting this" );
    delete this;

    TRACE( "CSecurityDialogs::RunError, end" );
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// CSecurityDialogs::CSecurityDialogs()
// ---------------------------------------------------------------------------
//
CSecurityDialogs::CSecurityDialogs( TBool& aIsDeleted ) : CActive( CActive::EPriorityLow ),
        iIsDeleted( aIsDeleted ), iOutputPtr( NULL, 0, 0 )
    {
    TRACE( "CSecurityDialogs::CSecurityDialogs" );
    CActiveScheduler::Add( this );
    iIsDeleted = EFalse;
    }

