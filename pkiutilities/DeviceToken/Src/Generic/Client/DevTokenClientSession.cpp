/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Implementation of DevTokenClientSession
*
*/



#include <e32std.h>
#include <e32uid.h>
#include <e32debug.h>
#include "DevTokenUtils.h"
#include "DevTokenServerName.h"
#include "DevTokenClientSession.h"
#include "DevtokenLog.h"

//Forward declaration
static TInt StartServer();

_LIT(KDevTokenServerImg,"DevTokenServer");

// Tokentype session class for Device token certificate stores
// Both device certificate store and trust site certificate store share
// the same token type.

// ======== LOCAL FUNCTIONS ========

// -----------------------------------------------------------------------------
// TInt StartServer()
// Client-server startup code
// -----------------------------------------------------------------------------
//
TInt StartServer()
    {
    // Server startup is different for WINS in EKA1 mode ONLY (lack of process
    // emulation - we load the library in this instance
    const TUidType serverUid(KNullUid, KNullUid, KUidDevTokenServer);

    RProcess server;  
    TInt r = server.Create(KDevTokenServerImg, KNullDesC, serverUid);
    
    RDebug::Print( _L(" StartServer ") );
    
    if ( r != KErrNone )
        {
        RDebug::Print( _L("StartServer1 %d"), r );
        return r;
        }

    // Synchronise with the process to make sure it hasn't died straight away
    TRequestStatus stat;
    server.Rendezvous(stat);
    if ( stat != KRequestPending )
        {
        // logon failed - server is not yet running, so cannot have terminated
        server.Kill(0);       // Abort startup
        }
    else
        {
        // logon OK - start the server
        server.Resume();
        }

    RDebug::Print( _L(" StartServer 2") );

    // Wait to synchronise with server - if it dies in the meantime, it
    // also gets completed
    User::WaitForRequest(stat); 
    
    RDebug::Print( _L(" StartServer 3") );
    // We can't use the 'exit reason' if the server panicked as this
    // is the panic 'reason' and may be '0' which cannot be distinguished
    // from KErrNone
    r = (server.ExitType()==EExitPanic) ? KErrGeneral : stat.Int();
    
    RDebug::Print( _L(" StartServer 4 %d"), r );
    
    server.Close();
    return (r);
    }
    
    
// ======== MEMBER FUNCTIONS ========

// -----------------------------------------------------------------------------
// RDevTokenClientSession::RDevTokenClientSession()
// -----------------------------------------------------------------------------
//
RDevTokenClientSession::RDevTokenClientSession()
    {
    }


// -----------------------------------------------------------------------------
// RDevTokenClientSession::ClientSessionL()
// -----------------------------------------------------------------------------
//
RDevTokenClientSession* RDevTokenClientSession::ClientSessionL()
    {
    return new( ELeave ) RDevTokenClientSession();
    }


// -----------------------------------------------------------------------------
// RDevTokenClientSession::SendRequest()
// -----------------------------------------------------------------------------
//
TInt RDevTokenClientSession::SendRequest(TDevTokenMessages aRequest, const TIpcArgs& aArgs) const
    {
    return SendReceive(aRequest, aArgs);
    }


// -----------------------------------------------------------------------------
// RDevTokenClientSession::SendAsyncRequest()
// -----------------------------------------------------------------------------
//
void RDevTokenClientSession::SendAsyncRequest(TDevTokenMessages aRequest, const TIpcArgs& aArgs, TRequestStatus* aStatus) const
    {
    __ASSERT_ALWAYS(aStatus, DevTokenPanic(EBadArgument));
    if (aStatus)
        {
        *aStatus = KRequestPending;
        SendReceive(aRequest, aArgs, *aStatus);
        }
    }


// -----------------------------------------------------------------------------
// TInt RDevTokenClientSession::Connect(EDevTokenEnum aToken)
// Connect to the server, attempting to start it if necessary
// -----------------------------------------------------------------------------
//
TInt RDevTokenClientSession::Connect(EDevTokenEnum aToken)
    {
    // The version is made up of three pieces of information:
    // 1. iMajor - The token we want to talk to
    // 2. iMinor - The protocol version number
    // 3. iBuild - unused
    TVersion version(aToken, KDevTokenProtolVersion, 0);

    TInt retry=2;
    for (;;)
        {
        TInt r=CreateSession(KDevTokenServerName, version, 1);

        TRACE_PRINT("connect");
        
        if ( r!=KErrNotFound && r!=KErrServerTerminated )
            {
            RDebug::Print( _L("connect1 %d"), r );
            return r; 
            }

        if (--retry==0)
            {
            RDebug::Print( _L(" connect2 %d"), r );
            return r; 
            }

        r=StartServer();
        
        if ( r!=KErrNone && r!=KErrAlreadyExists )
            {
            RDebug::Print( _L(" connect3 %d"), r );
            return r;  
            }

        }
    }

//EOF

