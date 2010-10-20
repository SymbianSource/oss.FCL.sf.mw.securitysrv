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
#include "secuidialogoperbasicpinquery.h" // CBasicPinQueryOperation
#include "secuidialogstrace.h"           // TRACE macro
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
		RDEBUG("0", 0);
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
    RDEBUG("0", 0);
    Server().RemoveSession();
    delete iOperationHandler;
    iOperationHandler = NULL;
    delete iInputBuffer;
    iInputBuffer = NULL;
    RDEBUG("0x99", 0x99);
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifierSession::CreateL()
// ---------------------------------------------------------------------------
//
void CSecuiDialogNotifierSession::CreateL()
    {
    RDEBUG("0", 0);
    Server().AddSession();
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifierSession::ServiceL()
// ---------------------------------------------------------------------------
//
void CSecuiDialogNotifierSession::ServiceL( const RMessage2& aMessage )
    {
    RDEBUG("aMessage.Handle()", aMessage.Handle());
    TRAPD( error, DispatchMessageL( aMessage ) );
    RDEBUG("error", error);
    if( error && !aMessage.IsNull() )
        {
        RDEBUG("Complete aMessage.Handle()", aMessage.Handle());
        aMessage.Complete( error );
        }
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifierSession::OperationComplete()
// ---------------------------------------------------------------------------
//
void CSecuiDialogNotifierSession::OperationComplete()
    {
    RDEBUG("0", 0);
    delete iOperationHandler;
    iOperationHandler = NULL;
    RDEBUG("0x99", 0x99);
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifierSession::CSecuiDialogNotifierSession()
// ---------------------------------------------------------------------------
//
CSecuiDialogNotifierSession::CSecuiDialogNotifierSession()
    {
    RDEBUG("0", 0);
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifierSession::ConstructL()
// ---------------------------------------------------------------------------
//
void CSecuiDialogNotifierSession::ConstructL()
    {
    RDEBUG("0", 0);
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
    RDEBUG("0", 0);
    if( !IsOperationCancelled( aMessage ) )
        {
        TInt lOperation = aMessage.Function();

		// from AskSecCodeInAutoLockL
		if(lOperation==0x100+6 /*RMobilePhone::EPhonePasswordRequired*/)
			{
			RDEBUG("query from AskSecCodeInAutoLockL . No need to start Autolock.exe", 0);
			}
		else
			{
	    CKeyguardAccessApi* iKeyguardAccess = CKeyguardAccessApi::NewL( );
	   	RDEBUG("0", 0);
			TInt err = iKeyguardAccess->AutolockStatus( 0x100, lOperation );	// start server, if needed 
			RDEBUG("err", err);
			delete iKeyguardAccess;
			}
			RDEBUG("lOperation", lOperation);
				if( lOperation >= 0x1000 )	// flag for iStartup
					lOperation -= 0x1000;
			RDEBUG("new lOperation", lOperation);
        if( lOperation < 0x200 )
            {
            BasicPinOperationL( aMessage );
            }
        else
	        	{
						RDEBUG("KErrNotSupported", KErrNotSupported);
	          User::Leave( KErrNotSupported );
	          }
        }
    RDEBUG("0x99", 0x99);
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
            RDEBUG("0", 0);
            iOperationHandler->CancelOperation();
            }
        RDEBUG("completing aMessage.Handle()", aMessage.Handle());
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
    RDEBUG("inputLength", inputLength);
    __ASSERT_ALWAYS( inputLength > 0, User::Leave( KErrCorrupt ) );
    if( iInputBuffer )
        {
        delete iInputBuffer;
        iInputBuffer = NULL;
        }
    iInputBuffer = HBufC8::NewL( inputLength );
    TPtr8 inputBufferPtr( iInputBuffer->Des() );
    aMessage.ReadL( KInputParam, inputBufferPtr );
    RDEBUG("0x99", 0x99);
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifierSession::ServerAuthenticationFailureL()
// ---------------------------------------------------------------------------
//
void CSecuiDialogNotifierSession::ServerAuthenticationFailureL( const RMessage2& /* aMessage */ )
    {
    RDEBUG("not used 0x99", 0x99);
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifierSession::BasicPinOperationL()
// ---------------------------------------------------------------------------
//
void CSecuiDialogNotifierSession::BasicPinOperationL( const RMessage2& aMessage )
    {
    RDEBUG("0", 0);
    GetInputBufferL( aMessage );

    ASSERT( iOperationHandler == NULL );
    iOperationHandler = CBasicPinQueryOperation::NewL( *this, aMessage, KOutputParam );
    iOperationHandler->StartL( *iInputBuffer );

    RDEBUG("0x99", 0x99);
    }

