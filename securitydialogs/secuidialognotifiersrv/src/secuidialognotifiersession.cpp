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
* Description:  Secui dialog notifier server session
*
*/

#include "secuidialognotifiersession.h"  // CSecuiDialogNotifierSession
#include "secuidialognotifierserver.h"   // CSecuiDialogNotifierServer
#include "secuidialognotifierservername.h" // KSecuiDialogsCancelOperation
// #include "secuidialogoperserverauthfail.h" // CServerAuthFailOperation
#include "secuidialogoperbasicpinquery.h" // CBasicPinQueryOperation
#include "secuidialogstrace.h"           // TRACE macro
#include <secdlgimpldefs.h>                 // TSecurityDialogOperation
#include <keyguardaccessapi.h>

const TInt KInputParam = 0;
const TInt KOutputParam = 1;


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CSecuiDialogNotifierSession::NewL()
// ---------------------------------------------------------------------------
//
CSecuiDialogNotifierSession* CSecuiDialogNotifierSession::NewL()
    {
		TRACE( "CBasicPinQueryOperation::RunL, 1 =%d", 1 );
    CSecuiDialogNotifierSession* self = new( ELeave ) CSecuiDialogNotifierSession;
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifierSession::~CSecuiDialogNotifierSession()
// ---------------------------------------------------------------------------
//
CSecuiDialogNotifierSession::~CSecuiDialogNotifierSession()
    {
    TRACE( "CSecuiDialogNotifierSession::~CSecuiDialogNotifierSession, begin" );
    Server().RemoveSession();
    delete iOperationHandler;
    iOperationHandler = NULL;
    delete iInputBuffer;
    iInputBuffer = NULL;
    TRACE( "CSecuiDialogNotifierSession::~CSecuiDialogNotifierSession, end" );
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifierSession::CreateL()
// ---------------------------------------------------------------------------
//
void CSecuiDialogNotifierSession::CreateL()
    {
    TRACE( "CSecuiDialogNotifierSession::~CSecuiDialogNotifierSession" );
    Server().AddSession();
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifierSession::ServiceL()
// ---------------------------------------------------------------------------
//
void CSecuiDialogNotifierSession::ServiceL( const RMessage2& aMessage )
    {
    TRACE( "CSecuiDialogNotifierSession::ServiceL, message 0x%08x", aMessage.Handle() );
    TRAPD( error, DispatchMessageL( aMessage ) );
    TRACE( "CSecuiDialogNotifierSession::ServiceL, dispatched, error %d", error );
    if( error && !aMessage.IsNull() )
        {
        TRACE( "CSecuiDialogNotifierSession::ServiceL, completing msg 0x%08x",
                aMessage.Handle() );
        aMessage.Complete( error );
        }
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifierSession::OperationComplete()
// ---------------------------------------------------------------------------
//
void CSecuiDialogNotifierSession::OperationComplete()
    {
    TRACE( "CSecuiDialogNotifierSession::OperationComplete, begin" );
    delete iOperationHandler;
    iOperationHandler = NULL;
    TRACE( "CSecuiDialogNotifierSession::OperationComplete, end" );
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifierSession::CSecuiDialogNotifierSession()
// ---------------------------------------------------------------------------
//
CSecuiDialogNotifierSession::CSecuiDialogNotifierSession()
    {
    TRACE( "CSecuiDialogNotifierSession::CSecuiDialogNotifierSession" );
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifierSession::ConstructL()
// ---------------------------------------------------------------------------
//
void CSecuiDialogNotifierSession::ConstructL()
    {
    TRACE( "CSecuiDialogNotifierSession::ConstructL" );
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifierSession::Server()
// ---------------------------------------------------------------------------
//
CSecuiDialogNotifierServer& CSecuiDialogNotifierSession::Server()
    {
    return *static_cast< CSecuiDialogNotifierServer* >(
            const_cast< CServer2* >( CSession2::Server() ) );
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifierSession::DispatchMessageL()
// ---------------------------------------------------------------------------
//
void CSecuiDialogNotifierSession::DispatchMessageL( const RMessage2& aMessage )
    {
    TRACE( "CSecuiDialogNotifierSession::DispatchMessageL, begin" );
    if( !IsOperationCancelled( aMessage ) )
        {
        TInt lOperation = aMessage.Function();
        TSecurityDialogOperation operation =
                static_cast< TSecurityDialogOperation >( lOperation );
        TRACE( "CSecuiDialogNotifierSession::DispatchMessageL, operation=%d", operation );

		// from AskSecCodeInAutoLockL
		if(lOperation==0x100+6 /*RMobilePhone::EPhonePasswordRequired*/)
			{
		  TRACE( "CSecuiDialogNotifierSession::DispatchMessageL, query from AskSecCodeInAutoLockL . No need to start Autolock.exe =%d", 0 );
			}
		else
			{
	    CKeyguardAccessApi* iKeyguardAccess = CKeyguardAccessApi::NewL( );
	   	TRACE( "CBasicPinQueryOperation::RunL, 1 =%d", 1 );
			TInt err = iKeyguardAccess->ShowKeysLockedNote( );
			TRACE( "CBasicPinQueryOperation::RunL, err =%d", err );
			delete iKeyguardAccess;
			}
			TRACE( "CBasicPinQueryOperation::RunL, lOperation =%d", lOperation );
				if( lOperation >= 0x1000 )	// flag for iStartup
					lOperation -= 0x1000;
			TRACE( "CBasicPinQueryOperation::RunL, new lOperation =%d", lOperation );
        if( lOperation < 0x200 )
            {
            BasicPinOperationL( aMessage );
            }
        else
	        	{
	  				TRACE( "CBasicPinQueryOperation::RunL, lOperation =%d", lOperation );
	          User::Leave( KErrNotSupported );
	          }
        }
    TRACE( "CSecuiDialogNotifierSession::DispatchMessageL, end" );
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifierSession::IsOperationCancelled()
// ---------------------------------------------------------------------------
//
TBool CSecuiDialogNotifierSession::IsOperationCancelled( const RMessage2& aMessage )
    {
    TBool isCancelled = EFalse;
    if( aMessage.Function() == KSecuiDialogCancelOperation )
        {
        if( iOperationHandler )
            {
            TRACE( "CSecuiDialogNotifierSession::CheckIfOperationCancelledL, cancelling" );
            iOperationHandler->CancelOperation();
            }
        TRACE( "CSecuiDialogNotifierSession::CheckIfOperationCancelledL, completing msg 0x%08x",
                aMessage.Handle() );
        aMessage.Complete( KErrNone );
        isCancelled = ETrue;
        }
    return isCancelled;
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifierSession::GetInputBufferL()
// ---------------------------------------------------------------------------
//
void CSecuiDialogNotifierSession::GetInputBufferL( const RMessage2& aMessage )
    {
    TInt inputLength = aMessage.GetDesLength( KInputParam );
    TRACE( "CSecuiDialogNotifierSession::GetInputBufferL, inputLength=%d", inputLength );
    __ASSERT_ALWAYS( inputLength > 0, User::Leave( KErrCorrupt ) );
    if( iInputBuffer )
        {
        delete iInputBuffer;
        iInputBuffer = NULL;
        }
    iInputBuffer = HBufC8::NewL( inputLength );
    TPtr8 inputBufferPtr( iInputBuffer->Des() );
    aMessage.ReadL( KInputParam, inputBufferPtr );
    TRACE( "CSecuiDialogNotifierSession::GetInputBufferL, read complete" );
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifierSession::ServerAuthenticationFailureL()
// ---------------------------------------------------------------------------
//
void CSecuiDialogNotifierSession::ServerAuthenticationFailureL( const RMessage2& aMessage )
    {
    TRACE( "CSecuiDialogNotifierSession::ServerAuthenticationFailureL, begin" );
    GetInputBufferL( aMessage );

    ASSERT( iOperationHandler == NULL );
    // iOperationHandler = CServerAuthFailOperation::NewL( *this, aMessage, KOutputParam );
    // iOperationHandler->StartL( *iInputBuffer );

    TRACE( "CSecuiDialogNotifierSession::ServerAuthenticationFailureL, end" );
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifierSession::BasicPinOperationL()
// ---------------------------------------------------------------------------
//
void CSecuiDialogNotifierSession::BasicPinOperationL( const RMessage2& aMessage )
    {
    TRACE( "CSecuiDialogNotifierSession::BasicPinOperationL, begin" );
    GetInputBufferL( aMessage );

    ASSERT( iOperationHandler == NULL );
    iOperationHandler = CBasicPinQueryOperation::NewL( *this, aMessage, KOutputParam );
    iOperationHandler->StartL( *iInputBuffer );

    TRACE( "CSecuiDialogNotifierSession::BasicPinOperationL, end" );
    }

