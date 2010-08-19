/*
* Copyright (c) 2003-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Keeps track, creates and destroys connector objects
*
*/



// INCLUDE FILES
#include    "ScardConnectionRegistry.h"
#include    "ScardServer.h"
#include    "ScardConnector.h"
#include    "ScardServerBase.h"
#include    "WimTrace.h"

#ifdef _DEBUG // for logging
#include    "ScardLogs.h"
#include    <flogger.h> 
#endif

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CScardConnectionRegistry::CScardConnectionRegistry
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CScardConnectionRegistry::CScardConnectionRegistry()
    {
    _WIMTRACE(_L("WIM|Scard|CScardConnectionRegistry::CScardConnectionRegistry|Begin"));
    }

// -----------------------------------------------------------------------------
// CScardConnectionRegistry::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CScardConnectionRegistry::ConstructL( CScardServer* aServer )
    {
    _WIMTRACE(_L("WIM|Scard|CScardConnectionRegistry::ConstructL|Begin"));
    iServer = aServer;
    iConnectors = new( ELeave ) CArrayFixFlat<TConnectionHandle>( 1 );
    }

// -----------------------------------------------------------------------------
// CScardConnectionRegistry::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CScardConnectionRegistry* CScardConnectionRegistry::NewL(
    CScardServer* aServer )
    {
    _WIMTRACE(_L("WIM|Scard|CScardConnectionRegistry::NewL|Begin"));
    CScardConnectionRegistry* self = new( ELeave ) CScardConnectionRegistry();
    
    CleanupStack::PushL( self );
    self->ConstructL( aServer );
    CleanupStack::Pop( self );

    return self;
    }

    
// Destructor
CScardConnectionRegistry::~CScardConnectionRegistry()
    {
    _WIMTRACE(_L("WIM|Scard|CScardConnectionRegistry::~CScardConnectionRegistry|Begin"));
    while ( iConnectors->Count() )
        {
        TConnectionHandle& handle = iConnectors->At( 0 );
        delete handle.iConnector;
        iConnectors->Delete( 0 );
        }
    delete iConnectors;
    }


// -----------------------------------------------------------------------------
// CScardConnectionRegistry::ConnectToReaderL
// Connect to reader
// -----------------------------------------------------------------------------
//
void CScardConnectionRegistry::ConnectToReaderL(
    CScardSession* aClient, 
    const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM|Scard|CScardConnectionRegistry::ConnectToReaderL|Begin"));
#ifdef _DEBUG
    RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName,     
        EFileLoggingModeAppend, 
        _L( "CScardConnectionRegistry::ConnectToReaderL entered.\n" ) );
#endif
    TConnectionHandle handle;
    handle.iClient = aClient;
    CScardConnector* conn = NULL;

    TRAPD( err, conn = CScardConnector::NewL( this, aMessage ) );
    if ( err )
        {
#ifdef _DEBUG
        RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
            EFileLoggingModeAppend, 
            _L( "CScardConnectionRegistry::ConnectToReaderL: creating connector\
            failed: %d \n" ), err );
#endif
        aMessage.Complete( err );
        return;
        }
    CleanupStack::PushL( conn );
    handle.iConnector = conn;
    iConnectors->AppendL( handle );
    conn->ConnectToReaderL();
    CleanupStack::Pop(conn);
    }

// -----------------------------------------------------------------------------
// CScardConnectionRegistry::ReconnectToReader
// Find the connector managing the session, and instruct it to reconnect.
// If the connector is not found, panic will occur.
// -----------------------------------------------------------------------------
//
/*
void CScardConnectionRegistry::ReconnectToReaderL(
    CScardSession* aClient, 
    const RMessage& aMessage )
    {
    _WIMTRACE(_L("WIM|Scard|CScardConnectionRegistry::ReconnectToReader|Begin"));
    for ( TInt i( 0 ); i < iConnectors->Count(); i++ )
        {
        if ( iConnectors->At( i ).iClient == aClient )
            {
            //  The session needs to be detached from the previous reader 
            //  (it wasn't the one we wanted after all)
            aClient->DisconnectFromReader();

            //  Now tell the connector to start reconnecting
            iConnectors->At( i ).iConnector->ReconnectToReaderL( aMessage );
            return;
            }
        }
    User::Panic( _L( "Connector registry failure" ), 
        KScServerPanicInternalError );
    }
*/

// -----------------------------------------------------------------------------
// CScardConnectionRegistry::CancelConnection
// Cancel connection
// -----------------------------------------------------------------------------
//
void CScardConnectionRegistry::CancelConnection( CScardSession* aSession )
    {
    _WIMTRACE(_L("WIM|Scard|CScardConnectionRegistry::CancelConnection|Begin"));
    for ( TInt i( 0 ); i < iConnectors->Count(); i++ )
        {
        if ( iConnectors->At( i ).iClient == aSession )
            {
            iConnectors->At( i ).iConnector->Cancel();
            return;
            }
        }
    }

// -----------------------------------------------------------------------------
// CScardConnectionRegistry::ConnectDone
// Connection has been established. Complete client message.
// -----------------------------------------------------------------------------
//
void CScardConnectionRegistry::ConnectDone(
    CScardConnector* aConnector,
    const TReaderID& aReaderID, 
    const TInt& aErrorCode )
    {
    _WIMTRACE(_L("WIM|Scard|CScardConnectionRegistry::ConnectDone|Begin"));
#ifdef _DEBUG
    RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName,     
        EFileLoggingModeAppend, 
        _L( "CScardConnectionRegistry::ConnectDone to reader %d with status\
        %d.\n" ), aReaderID, aErrorCode );
#endif
    for ( TInt i( 0 ); i < iConnectors->Count(); i++ )
        {
#ifdef _DEBUG
        RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName,     
            EFileLoggingModeAppend, 
            _L( "CScardConnectionRegistry::ConnectDone finding correct\
            connector out of %d.\n" ), iConnectors->Count() );
#endif
        TConnectionHandle& connection = iConnectors->At( i );
        if ( connection.iConnector == aConnector )
            {
#ifdef _DEBUG
            RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName,     
                EFileLoggingModeAppend, 
                _L( "CScardConnectionRegistry::ConnectDone connector\
                found.\n" ) );
#endif
            connection.iClient->ConnectionDone( aReaderID, 
                connection.iConnector->Message(), aErrorCode );
            connection.iConnector->Message().Complete( aErrorCode );
            return;
            }
        }
    User::Panic( _L( "Registry failure" ), KScServerPanicInternalError );
    }

// -----------------------------------------------------------------------------
// CScardConnectionRegistry::RemoveConnector
// Remove connector.
// -----------------------------------------------------------------------------
//
void CScardConnectionRegistry::RemoveConnector( CScardConnector* aConnector )
    {
    _WIMTRACE(_L("WIM|Scard|CScardConnectionRegistry::RemoveConnector|Begin"));
#ifdef _DEBUG
    RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName,     
        EFileLoggingModeAppend, 
        _L( "CScardConnectionRegistry::RemoveConnector.\n" ) );
#endif
    for ( TInt i( 0 ); i < iConnectors->Count(); i++ )
        {
        if ( iConnectors->At( i ).iConnector == aConnector )
            {
#ifdef _DEBUG
            RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName,     
                EFileLoggingModeAppend, 
                _L( "CScardConnectionRegistry::RemoveConnector connector\
                found.\n" ) );
#endif
            iConnectors->Delete( i );
            return;
            }
        }
    User::Panic( _L( "Registry failure" ), KScServerPanicInternalError );
    }

// -----------------------------------------------------------------------------
// CScardConnectionRegistry::Server
// Return pointer to Scard server object
// -----------------------------------------------------------------------------
//
CScardServer* CScardConnectionRegistry::Server() const
    {
    return iServer;
    }

// -----------------------------------------------------------------------------
// CScardConnectionRegistry::Connection
// Return connection handle
// -----------------------------------------------------------------------------
//
TConnectionHandle& CScardConnectionRegistry::Connection( TInt aLocation ) const
    { 
    return iConnectors->At( aLocation ); 
    }

//  End of File  
