/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Startup place for client & server, message handling.
*
*/


//INCLUDES
#include "WimClient.h"
#include "WimSig.h"
#include "WimTrace.h"

#ifdef __WINS__
#include <c32comm.h>

#endif // WINS


// -----------------------------------------------------------------------------
// FindServerFileName()
// Return the name of the WIM server file
// -----------------------------------------------------------------------------
//
TInt RWimClient::FindServerFileName( TFileName& aServer )
    {
    // just return .EXE name, it's loaded from /sys/bin
    aServer = KWimServerFile;
    return ( KErrNone );
    }


// -----------------------------------------------------------------------------
// StartWim()
// Start the WIM server
// -----------------------------------------------------------------------------
//
TInt RWimClient::StartWim()
    {
    _WIMTRACE( _L( "StartWim() ") );
    // just get .EXE name, it's loaded from /sys/bin
    TFileName serverFileName( KWimServerFile ); 
    TRequestStatus stat;

    RProcess server;
    _WIMTRACE( _L( "StartWim Process() start" ) );
    TInt r = server.Create( serverFileName, 
                            KNullDesC,
                            TUidType( KNullUid, 
                                      KNullUid, 
                                      KWimServerUid ) );
    if ( r != KErrNone )
        {
        return r;
        }

    _WIMTRACE( _L( "StartWim Process() started" ) );


    _WIMTRACE( _L( "wait util server release mutex..." ) );

    server.Rendezvous( stat );
    
    _WIMTRACE( _L( "mutex is released" ) );
  

    
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

    _WIMTRACE( _L( "RWimClient::StartWim() | Wait status to complete" ) );

    User::WaitForRequest( stat ); // Wait server to signal is has started
    
    _WIMTRACE2( _L( "RWimClient::StartWim() | complete with error %d" ), stat.Int() );
 
    if ( stat.Int() == KErrAlreadyExists )
        {
        r = KErrNone;
        }
    else
        {
        r = stat.Int();
        }

    server.Close();
    _WIMTRACE( _L( "RWimClient::StartWim() | End" ) );
    
    
    return r;
    }

// -----------------------------------------------------------------------------
// RWimClient::RWimClient()
// Default constructor
// -----------------------------------------------------------------------------
//
RWimClient::RWimClient()
    {
    _WIMTRACE ( _L( "RWimClient::RWimClient()" ) );
    }


// -----------------------------------------------------------------------------
// RWimClient::~RWimClient()
// Destructor
// -----------------------------------------------------------------------------
//
RWimClient::~RWimClient()
    {
    _WIMTRACE ( _L( "RWimClient::~RWimClient()" ) );
    }

// -----------------------------------------------------------------------------
// RWimClient::Connect()
// Connect to WIMI server
// -----------------------------------------------------------------------------
//
TInt RWimClient::Connect()
    {
    _WIMTRACE ( _L( "RWimClient::Connect()" ) );
#ifdef __WINS__
    StartC32();
#endif
    TInt retryCount = 0;
    TInt retval = 0;
    retval = CreateSession( KWIMServerName, 
        Version(), 
        KMessageSlotsNum );
    if ( retval == KErrServerTerminated )
        {
        while ( (retval == KErrServerTerminated ) 
            && ( retryCount++ < KWimServerTerminatingMaxRetryCount) )
            {   
            /* Server can take some time to shutdown, retry in a moment */
            User::After( KWimServerTerminatingRetryTimeout );
            retval = CreateSession( KWIMServerName, 
                Version(), 
                KMessageSlotsNum );
            }
        }
    return retval; 
    }


// -----------------------------------------------------------------------------
// RWimClient::Version() const
// Return version
// -----------------------------------------------------------------------------
//
TVersion RWimClient::Version( void ) const
    {
    _WIMTRACE ( _L( "RWimClient::Version()" ) );
    return( TVersion( KWIMServMajorVersionNumber, 
                      KWIMServMinorVersionNumber, 
                      KWIMServBuildVersionNumber ) );
    }

// -----------------------------------------------------------------------------
// RWimClient::SendReceiveData()
// Send message synchronously to server
// -----------------------------------------------------------------------------
//
TInt  RWimClient::SendReceiveData( TWimServRqst aFn, TIpcArgs& aIpcArgs )
    {
    _WIMTRACE ( _L( "RWimClient::SendReceiveData()" ) );
    return SendReceive( aFn, aIpcArgs ); 
    }

// -----------------------------------------------------------------------------
// RWimClient::SendReceiveData()
// Send message asyncronously to server
// -----------------------------------------------------------------------------
//
void RWimClient::SendReceiveData( TWimServRqst aFn, 
                                  TIpcArgs& aIpcArgs, 
                                  TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "RWimClient::SendReceiveData()" ) );
    SendReceive( aFn, aIpcArgs, aStatus );
    }

// -----------------------------------------------------------------------------
// RWimClient::FreeAddrLst()
// Free the address list
// -----------------------------------------------------------------------------
//
void RWimClient::FreeAddrLst( const TUint32 addrLst )
    {
    _WIMTRACE ( _L( "RWimClient::FreeAddrLst()" ) );

    TIpcArgs args;
    args.Set( 0, addrLst );
    SendReceiveData( EFreeMemoryLst, args );
    }

// -----------------------------------------------------------------------------
// RWimClient::FreeWIMAddrLst()
// Free the address list
// -----------------------------------------------------------------------------
//
void RWimClient::FreeWIMAddrLst( const TWimAddressList addrLst, 
                                 const TUint aSize )
    {
    _WIMTRACE ( _L( "RWimClient::FreeWIMAddrLst()" ) );

    TIpcArgs args;
    args.Set( 0, addrLst[0] );
    args.Set( 1, aSize );
    
    SendReceiveData( EFreeWIMMemoryLst, args );
    }

// -----------------------------------------------------------------------------
// RWimClient::FreeWIMAddr()
// Free the address
// -----------------------------------------------------------------------------
//
void RWimClient::FreeWIMAddr( const TWimAddress aAddr )
    {
    _WIMTRACE ( _L( "RWimClient::FreeWIMAddr()" ) );
    TIpcArgs args;
    args.Set( 0, aAddr );
    SendReceiveData( EFreeMemory, args );
    }

// -----------------------------------------------------------------------------
// RWimClient::Initialize()
// Sends initialization command to Server.
// -----------------------------------------------------------------------------
//
void RWimClient::Initialize( TRequestStatus& aStatus )
    {
    aStatus = KRequestPending;
    _WIMTRACE ( _L( "RWimClient::Initialize( TRequestStatus& aStatus )" ) );
    TIpcArgs args;
    SendReceiveData( EWimInitialize, args, aStatus );
    }

// -----------------------------------------------------------------------------
// RWimClient::CancelInitialize()
// Cancel initialization command to Server.
// -----------------------------------------------------------------------------
//
void RWimClient::CancelInitialize()
    {
    _WIMTRACE ( _L( "RWimClient::CancelInitialize()" ) );
	TIpcArgs args;
    SendReceiveData( ECancelWimInitialize, args );
    }
// End of File



