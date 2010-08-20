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
* Description:  Security dialog notifier server session
*
*/

#include "securitydialognotifiersession.h"  // CSecurityDialogNotifierSession
#include "securitydialognotifierserver.h"   // CSecurityDialogNotifierServer
#include "securitydialognotifierservername.h" // KSecurityDialogsCancelOperation
#include "securitydialogoperserverauthfail.h" // CServerAuthFailOperation
#include "securitydialogstrace.h"           // TRACE macro
#include <secdlgimpldefs.h>                 // TSecurityDialogOperation

const TInt KInputParam = 0;
const TInt KOutputParam = 1;


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CSecurityDialogNotifierSession::NewL()
// ---------------------------------------------------------------------------
//
CSecurityDialogNotifierSession* CSecurityDialogNotifierSession::NewL()
    {
    CSecurityDialogNotifierSession* self = new( ELeave ) CSecurityDialogNotifierSession;
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// CSecurityDialogNotifierSession::~CSecurityDialogNotifierSession()
// ---------------------------------------------------------------------------
//
CSecurityDialogNotifierSession::~CSecurityDialogNotifierSession()
    {
    TRACE( "CSecurityDialogNotifierSession::~CSecurityDialogNotifierSession, begin" );
    Server().RemoveSession();
    delete iOperationHandler;
    iOperationHandler = NULL;
    delete iInputBuffer;
    iInputBuffer = NULL;
    TRACE( "CSecurityDialogNotifierSession::~CSecurityDialogNotifierSession, end" );
    }

// ---------------------------------------------------------------------------
// CSecurityDialogNotifierSession::CreateL()
// ---------------------------------------------------------------------------
//
void CSecurityDialogNotifierSession::CreateL()
    {
    TRACE( "CSecurityDialogNotifierSession::~CSecurityDialogNotifierSession" );
    Server().AddSession();
    }

// ---------------------------------------------------------------------------
// CSecurityDialogNotifierSession::ServiceL()
// ---------------------------------------------------------------------------
//
void CSecurityDialogNotifierSession::ServiceL( const RMessage2& aMessage )
    {
    TRACE( "CSecurityDialogNotifierSession::ServiceL, message 0x%08x", aMessage.Handle() );
    TRAPD( error, DispatchMessageL( aMessage ) );
    TRACE( "CSecurityDialogNotifierSession::ServiceL, dispatched, error %d", error );
    if( error && !aMessage.IsNull() )
        {
        TRACE( "CSecurityDialogNotifierSession::ServiceL, completing msg 0x%08x",
                aMessage.Handle() );
        aMessage.Complete( error );
        }
    }

// ---------------------------------------------------------------------------
// CSecurityDialogNotifierSession::OperationComplete()
// ---------------------------------------------------------------------------
//
void CSecurityDialogNotifierSession::OperationComplete()
    {
    TRACE( "CSecurityDialogNotifierSession::OperationComplete, begin" );
    delete iOperationHandler;
    iOperationHandler = NULL;
    TRACE( "CSecurityDialogNotifierSession::OperationComplete, end" );
    }

// ---------------------------------------------------------------------------
// CSecurityDialogNotifierSession::CSecurityDialogNotifierSession()
// ---------------------------------------------------------------------------
//
CSecurityDialogNotifierSession::CSecurityDialogNotifierSession()
    {
    TRACE( "CSecurityDialogNotifierSession::CSecurityDialogNotifierSession" );
    }

// ---------------------------------------------------------------------------
// CSecurityDialogNotifierSession::ConstructL()
// ---------------------------------------------------------------------------
//
void CSecurityDialogNotifierSession::ConstructL()
    {
    TRACE( "CSecurityDialogNotifierSession::ConstructL" );
    }

// ---------------------------------------------------------------------------
// CSecurityDialogNotifierSession::Server()
// ---------------------------------------------------------------------------
//
CSecurityDialogNotifierServer& CSecurityDialogNotifierSession::Server()
    {
    return *static_cast< CSecurityDialogNotifierServer* >(
            const_cast< CServer2* >( CSession2::Server() ) );
    }

// ---------------------------------------------------------------------------
// CSecurityDialogNotifierSession::DispatchMessageL()
// ---------------------------------------------------------------------------
//
void CSecurityDialogNotifierSession::DispatchMessageL( const RMessage2& aMessage )
    {
    TRACE( "CSecurityDialogNotifierSession::DispatchMessageL, begin" );
    if( !IsOperationCancelled( aMessage ) )
        {
        TSecurityDialogOperation operation =
                static_cast< TSecurityDialogOperation >( aMessage.Function() );
        TRACE( "CSecurityDialogNotifierSession::DispatchMessageL, operation=%d", operation );
        switch( operation )
            {
            case ESecureConnection:
            case ESignText:
            case EEnterPIN:
            case EEnablePIN:
            case EDisablePIN:
            case EChangePIN:
            case EUnblockPIN:
            case EPINBlocked:
            case ETotalBlocked:
            case EUnblockPINInClear:
                User::Leave( KErrNotSupported );
                break;
            case EServerAuthenticationFailure:
                ServerAuthenticationFailureL( aMessage );
                break;
            default:
                User::Leave( KErrNotSupported );
                break;
            }
        }
    TRACE( "CSecurityDialogNotifierSession::DispatchMessageL, end" );
    }

// ---------------------------------------------------------------------------
// CSecurityDialogNotifierSession::IsOperationCancelled()
// ---------------------------------------------------------------------------
//
TBool CSecurityDialogNotifierSession::IsOperationCancelled( const RMessage2& aMessage )
    {
    TBool isCancelled = EFalse;
    if( aMessage.Function() == KSecurityDialogCancelOperation )
        {
        if( iOperationHandler )
            {
            TRACE( "CSecurityDialogNotifierSession::CheckIfOperationCancelledL, cancelling" );
            iOperationHandler->CancelOperation();
            }
        TRACE( "CSecurityDialogNotifierSession::CheckIfOperationCancelledL, completing msg 0x%08x",
                aMessage.Handle() );
        aMessage.Complete( KErrNone );
        isCancelled = ETrue;
        }
    return isCancelled;
    }

// ---------------------------------------------------------------------------
// CSecurityDialogNotifierSession::ServerAuthenticationFailureL()
// ---------------------------------------------------------------------------
//
void CSecurityDialogNotifierSession::ServerAuthenticationFailureL( const RMessage2& aMessage )
    {
    TRACE( "CSecurityDialogNotifierSession::ServerAuthenticationFailureL, begin" );
    GetInputBufferL( aMessage );

    __ASSERT_DEBUG( iOperationHandler == NULL, User::Invariant() );
    iOperationHandler = CServerAuthFailOperation::NewL( *this, aMessage, KOutputParam );
    iOperationHandler->StartL( *iInputBuffer );

    TRACE( "CSecurityDialogNotifierSession::ServerAuthenticationFailureL, end" );
    }

// ---------------------------------------------------------------------------
// CSecurityDialogNotifierSession::GetInputBufferL()
// ---------------------------------------------------------------------------
//
void CSecurityDialogNotifierSession::GetInputBufferL( const RMessage2& aMessage )
    {
    TInt inputLength = aMessage.GetDesLength( KInputParam );
    TRACE( "CSecurityDialogNotifierSession::GetInputBufferL, inputLength=%d", inputLength );
    __ASSERT_ALWAYS( inputLength > 0, User::Leave( KErrCorrupt ) );
    if( iInputBuffer )
        {
        delete iInputBuffer;
        iInputBuffer = NULL;
        }
    iInputBuffer = HBufC8::NewL( inputLength );
    TPtr8 inputBufferPtr( iInputBuffer->Des() );
    aMessage.ReadL( KInputParam, inputBufferPtr );
    TRACE( "CSecurityDialogNotifierSession::GetInputBufferL, read complete" );
    }

