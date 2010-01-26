/*
* Copyright (c) 2003 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  A connection to a smart card
*
*/



// INCLUDE FILES
#include    "ScardConnector.h"
#include    "ScardConnectionRegistry.h"
#include    "ScardAccessControl.h"
#include    "ScardAccessControlRegistry.h"
#include    "ScardConnectionTimer.h"
#include    "ScardServer.h"
#include    "ScardServerBase.h"
#include    "WimTrace.h"

#ifdef _DEBUG // for logging
#include    "ScardLogs.h"
#include    <flogger.h> 
#endif

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// TConnectionParameter::TConnectionParameter
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
TConnectionParameter::TConnectionParameter()
    : iReaderName( NULL ), 
      iExcluded( NULL ), 
      iATRBytes( NULL ),
      iAIDBytes( NULL ),
      iNewCardOnly( EFalse ), 
      iNewReaderOnly( EFalse )
    {
    _WIMTRACE(_L("WIM|Scard|TConnectionParameter::TConnectionParameter|Begin"));
    }

// -----------------------------------------------------------------------------
// TConnection::TConnection
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
TConnection::TConnection()
    : iCtrl( NULL ), 
      iReader( NULL ), 
      iReaderID( 0 ), 
      iSessionID( ENoSession )
    {
    }

// -----------------------------------------------------------------------------
// Compare two connections
// -----------------------------------------------------------------------------
//
TBool operator==(
    const TConnection& aConnection1, 
    const TConnection& aConnection2 )
    {
    if ( aConnection1.iReader == aConnection2.iReader && 
        aConnection1.iCtrl == aConnection2.iCtrl &&
        aConnection1.iSessionID == aConnection2.iSessionID &&
        aConnection1.iReaderID == aConnection2.iReaderID )
        {
        return ETrue;
        }
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CScardConnector::CScardConnector
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CScardConnector::CScardConnector(
    CScardConnectionRegistry* aConnRegistry, 
    RThread& /*aClient*/ )
    : CScardSession(),
      iConnectionRegistry( aConnRegistry ),
      iState( EActive )
    {
    _WIMTRACE(_L("WIM|Scard|CScardConnector::CScardConnector|Begin"));
    }


// -----------------------------------------------------------------------------
// CScardConnector::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CScardConnector::ConstructL( const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM|Scard|CScardConnector::ConstructL|Begin"));
    iConnections = new( ELeave ) CArrayFixFlat<TConnection>( 1 );
    iStack = CScardEventStack::NewL( this );
    iClientMessage = new( ELeave ) RMessage2( aMessage ); 
    iState = EActive;
    }

// -----------------------------------------------------------------------------
// CScardConnector::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CScardConnector* CScardConnector::NewL(
    CScardConnectionRegistry* aConnRegistry, 
    RThread& aClient, 
    const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM|Scard|CScardConnector::NewL|Begin"));
    CScardConnector* self = new( ELeave ) CScardConnector( aConnRegistry, 
        aClient );
    
    CleanupStack::PushL( self );
    self->ConstructL( aMessage );
    CleanupStack::Pop( self );

    return self;
    }

    
// Destructor
CScardConnector::~CScardConnector()
    {
    _WIMTRACE(_L("WIM|Scard|CScardConnector::~CScardConnector|Begin"));
    delete iTimer;
    
    //  Delete members of the parameter block
    delete iParameters.iReaderName;
    delete iParameters.iExcluded;
    delete iParameters.iATRBytes;
    delete iParameters.iAIDBytes;

    //  Detach from all connections
    if ( iConnections->Count() )
        {
        TConnection& connection = iConnections->At( 0 );
        //  Detach and delete all connections
        while ( iConnections->Count() )
            {
            connection = iConnections->At( 0 );
            TInt err = KErrNone;
            TRAP( err, connection.iCtrl->CancelTransmissionsL(
                                                      connection.iSessionID ) );
            connection.iCtrl->DetachSessionFromReader( connection.iSessionID );
            iConnections->Delete( 0 );
            }
        }

    iConnections->Reset();
    delete iConnections;
    delete iClientMessage;
    delete iStack;
    }


// -----------------------------------------------------------------------------
// CScardConnector::ConnectToReaderL
// The connection is attempted and if it is successful
// the session requesting the connection is notified. If the connection
// can not yet be made, this object will remain waiting until all the
// required parameters are matched or a timeout occurs.
// -----------------------------------------------------------------------------
//
void CScardConnector::ConnectToReaderL()
    {
    _WIMTRACE(_L("WIM|Scard|CScardConnector::ConnectToReaderL|Begin"));
    // Get the limit factors
    TRAPD( messageError, ReadLimitsL() ); 
    if ( messageError == KScErrBadArgument || 
        messageError == KScErrNotSupported )
        {
#ifdef _DEBUG
        RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
            EFileLoggingModeAppend, 
            _L( "CScardConnector::ConnectToReader: ReadLimits failed: %d \n" ), 
            messageError );
#endif
        iConnectionRegistry->ConnectDone( this, 0, messageError );
        iConnectionRegistry->RemoveConnector( this );
        delete this;
        return;
        }
    else if ( messageError != KErrNone )
        {
        User::Panic( _L( "Not enough free memory" ), KScPanicNoMemory );
        }

    //  Now go to all the ACs you need and attach to them. If the reader 
    //  is spesific, only one entry is required. Otherwise all are.
    if ( iParameters.iReaderName )
        {
#ifdef _DEBUG
        RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName,     
            EFileLoggingModeAppend, 
            _L( "CScardConnector::ConnectToReader Connecting to specific\
            reader: %S\n" ),
           iParameters.iReaderName);
#endif
        iOneReaderMode = ETrue;
        //  Is the chosen one supported ??
        if ( !( iConnectionRegistry->Server()->ReaderSupported( 
            *iParameters.iReaderName ) ) )
            {
#ifdef _DEBUG
            RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
                EFileLoggingModeAppend, 
                _L( "CScardConnector::ConnectToReader ReaderSupported\
                failed: \n" ) );
#endif
            iConnectionRegistry->ConnectDone( this, 0, KScErrUnknownReader );
            iConnectionRegistry->RemoveConnector( this );
            delete this;
            return;
            }

        //  Grab the reader id (this shouldn't fail if the previous step 
        //  went ok)
        const TReaderID readerID = 
            iConnectionRegistry->Server()->ReaderID( *iParameters.iReaderName );

        //  If the reader can't be open when connecting, check it
        if ( iParameters.iNewReaderOnly )
            {
            if ( iConnectionRegistry->Server()->AccessRegistry()->
                ReaderHandlerLoaded( readerID ) )
                {
#ifdef _DEBUG
                RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
                    EFileLoggingModeAppend, 
                    _L( "CScardConnector::ConnectToReader Failed! Only new\
                    readers allowed!\n" ) );
#endif
                iConnectionRegistry->ConnectDone( this, 0, 
                    KScErrAlreadyExists );
                iConnectionRegistry->RemoveConnector( this );
                delete this;
                return;
                }
            }
        //  Create the access controller and the reader
        TBool opened( EFalse );
        TRAPD( readerError, opened = NewConnectionL( (*iClientMessage), 
            readerID ) );
        
        //  If the reader object could not be created, look no further, 
        //  we're in trouble
        if ( readerError != KErrNone )
            {
#ifdef _DEBUG
            RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
                EFileLoggingModeAppend, 
                _L( "CScardConnector::ConnectToReader NewConnectionL failed:\
                %d \n" ), readerError );
#endif
            iConnectionRegistry->ConnectDone( this, 0, readerError );
            iConnectionRegistry->RemoveConnector( this );
            delete this;
            return;
            }

        //  The reader was created successfully.

        //  1.  If the reader is open and if old cards are acceptable, 
        //  check conditions

        if ( opened && !iParameters.iNewCardOnly )
            {
            TScardATR atr;
            iConnections->At( 0 ).iReader->GetATR( atr );
            const TBool ok( CheckConditions( iConnections->At( 0 ), &atr ) );
            if ( ok )
                {
#ifdef _DEBUG
                RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
                    EFileLoggingModeAppend, 
                    _L( "CScardConnector::ConnectToReader: Connection\
                    succesfully established.\n" ) );
#endif
                ConnectionDone( iConnections->At( 0 ).iReaderID, KErrNone );
                return;
                }
#ifdef _DEBUG
            RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
                EFileLoggingModeAppend, 
                _L( "CScardConnector::ConnectToReader CheckConditions\
                failed! \n" ) );
#endif
            }

        //  2.  In other cases, just stay waiting for either the OpenReader 
        //  or condition checking to go through or for a card event to occur.
        return;
        }
    else
        {
        //  Contact (almost) all readers, dead or alive...
#ifdef _DEBUG
        RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName,     
            EFileLoggingModeAppend, 
           _L("CScardConnector::ConnectToReader Connecting to all readers.\n") );
#endif
        CScardReaderRegistry* rr = 
            iConnectionRegistry->Server()->ReaderRegistry();

        //  First list the ones not yet opened
        CArrayFixFlat<TReaderID>* closelist = NULL;
        closelist = new( ELeave ) CArrayFixFlat<TReaderID>( 1 );
        CleanupStack::PushL( closelist );
        rr->ListClosedReadersL( closelist );

        //  then list the ones that are currently open (if necessary)
        CArrayPtrFlat<CScardAccessControl>* openlist = NULL;
        if ( !iParameters.iNewReaderOnly )
            {
            openlist = new( ELeave ) CArrayPtrFlat<CScardAccessControl>( 1 );
            CleanupStack::PushL( openlist );
            rr->ListOpenReadersL( openlist );
            }

        TReaderID excludeID( -1 );
        if ( iParameters.iExcluded )
            {
#ifdef _DEBUG
            RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName,     
                EFileLoggingModeAppend, 
               _L( "CScardConnector::ConnectToReader Get ID of excluded\
               reader.\n" ) );
#endif
            excludeID = iConnectionRegistry->Server()->ReaderID( 
                *iParameters.iExcluded );
            }
        
        //  Attach to open readers
        if ( !iParameters.iNewReaderOnly )
            {
#ifdef _DEBUG
            RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName,     
                EFileLoggingModeAppend, 
               _L( "CScardConnector::ConnectToReader Attach to open readers" ) );
#endif
            for ( TInt i( 0 ); i < openlist->Count(); i++ )
                {
                TReaderID next = openlist->At( i )->ReaderID();
                if ( next != excludeID )
                    {
                    TBool open( EFalse );
                    //  Create the connection
                    TRAPD( readerError, 
                        open = NewConnectionL( *iClientMessage, next ) );

                    //  If the reader is already open and old cards are 
                    //  acceptable, check SC for the parameters.

                    if ( !readerError && open && !iParameters.iNewCardOnly )
                        {
                        const TInt index = iConnections->Count() - 1;
                        TScardATR atr;
                        iConnections->At( index ).iReader->GetATR( atr );
                        TBool ok = CheckConditions( iConnections->At( 
                            iConnections->Count() - 1 ), &atr );
                        if ( ok )
                            {
                            //delete openlist & closelist
                            CleanupStack::PopAndDestroy( 2 ); 
                            ConnectionDone( next, KErrNone );
                            return;
                            }
                        }
                    else if ( open && iParameters.iNewCardOnly )
                        {
                        //delete openlist & closelist
                        CleanupStack::PopAndDestroy( 2 ); 
                        // old cards are not acceptable, return error
                        ConnectionDone( next, KScErrNotFound );
                        return;
                        }    
                    }
                }
            CleanupStack::PopAndDestroy( openlist );  //  delete openlist
            }
        
        //  attach to new readers
        for ( TInt i( 0 ); i < closelist->Count(); i++ )
            {
#ifdef _DEBUG
            RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName,     
                EFileLoggingModeAppend, 
               _L( "CScardConnector::ConnectToReader Attach to new readers" ) );
#endif
            TReaderID next = closelist->At( i );
            if ( next != excludeID )
                {
                TInt err = KErrNone;
                TRAP( err, NewConnectionL( (*iClientMessage), next ) );
                }
            }

        CleanupStack::PopAndDestroy( closelist );  //  delete closelist
        }

    //  If no readers could be contacted at all ( == the handlers could not be 
    //  created) why continue?
    if ( !iConnections->Count() )
        {
        iState = EConnectionComplete;
        ConnectionDone( 0, KScErrNotFound );
        return;
        }

    if ( iConnections->Count() == 1 )
        {
        iOneReaderMode = ETrue;
        }
    }

// -----------------------------------------------------------------------------
// CScardConnector::CardEvent
// Handle card event
// -----------------------------------------------------------------------------
//
void CScardConnector::CardEvent(
    const TScardServiceStatus aEvent, 
    const TScardATR& aATR, 
    const TReaderID& aReaderID )
    {
    _WIMTRACE(_L("WIM|Scard|CScardConnector::CardEvent|Begin"));
    //  Card event handling is postponed if waiting for confirmation on
    //  a connection attempt
    if ( iState == EWaitingForConfirm )
        {
        iStack->QueueEvent( aReaderID, aEvent );
        return;
        }

    //  Find the connection handling the reader and see if it meets the 
    //  criteria
    TConnection* conn = NULL;
    TRAPD( err, conn = &( FindReaderConnectionL( aReaderID ) ) );

    //  The connection must be found...
    __ASSERT_ALWAYS( !err, User::Panic( _L( "Connector stack failure" ), 
        KScServerPanicInternalError ) );

    //  Now see what kind of event has occurred
    switch ( aEvent )
        {
        case EScardRemoved: // Flow through
        case EReaderRemoved:
            {
            break;
            }
        case EScardInserted:
            {
            //  A new card has been inserted, match it against the parameters
            const TBool ok = CheckConditions( *conn, &aATR );
            if ( ok )
                {
                ConnectionDone( conn->iReaderID, KErrNone );
                }
            break;
            }
        default:
            {
            break;
            }
        }
    }

// -----------------------------------------------------------------------------
// CScardConnector::ReadLimitsL
// Extract the limit parameters for this connection attempt
// -----------------------------------------------------------------------------
//
void CScardConnector::ReadLimitsL()
    {
    _WIMTRACE(_L("WIM|Scard|CScardConnector::ReadLimitsL|Begin"));
    TInt limiters = (*iClientMessage).Int0();

    //  Check out the limiting factors
    
    //  The name of the reader is specified
    if ( limiters & KExplicitReader )
        {
        TInt desLen = (*iClientMessage).GetDesLength( 1 );
        HBufC* name = HBufC::NewL( desLen );
        CleanupStack::PushL( name );
        iParameters.iReaderName = new( ELeave ) TPtr( name->Des() );
        TRAPD( err, (*iClientMessage).ReadL( 1, *iParameters.iReaderName ) );
        if ( err != KErrNone )
            {
            (*iClientMessage).Panic( _L( "SCardServer" ), 
                KScServerPanicBadDescriptor );
            User::Leave( KScErrBadArgument );
            }
        CleanupStack::Pop( name );
        }

    //  The specified must not be contacted
    else if ( limiters & KExcludedReader )
        {
        TInt desLen = (*iClientMessage).GetDesLength( 1 );
        HBufC* bytes = HBufC::NewL( desLen );
        CleanupStack::PushL( bytes );
        iParameters.iExcluded = new( ELeave ) TScardReaderName( bytes->Des() );
        TRAPD( err, (*iClientMessage).ReadL( 1, *iParameters.iExcluded ) );
        if ( err != KErrNone )
            {
            (*iClientMessage).Panic( _L( "SCardServer" ), 
                KScServerPanicBadDescriptor );
            User::Leave( KScErrBadArgument );
            }
        CleanupStack::Pop( bytes );
        }

    //  Only newly inserted cards are acceptable
    if ( limiters & KNewReadersOnly )
        {
        iParameters.iNewReaderOnly = ETrue;
        }

    //  The ATR bytes of the card are spesified
    if ( limiters & KATRSpesified )
        {
        User::Leave( KScErrNotSupported );
        }

    //  The spesified application must be located within the SC
    else if ( limiters & KApplicationSpesified )
        {
        User::Leave( KScErrNotSupported );
        }

    //  Only newly inserted cards are acceptable
    if ( limiters & KNewCardsOnly )
        {
        iParameters.iNewCardOnly = ETrue;
        }

    //  Is there a time limit as well ?
    TInt32 timeOut = reinterpret_cast< TInt32 >( iClientMessage->Ptr3() );

    if ( timeOut )
        {
        iTimer = CScardConnectionTimer::NewL( this, timeOut );
        }
    }

// -----------------------------------------------------------------------------
// CScardConnector::CheckConditions
// Check if connection conditions are met.
// -----------------------------------------------------------------------------
//
TBool CScardConnector::CheckConditions(
    TConnection& /*aConnection*/, 
    const TScardATR* aATR )
    {
    _WIMTRACE(_L("WIM|Scard|CScardConnector::CheckConditions|Begin"));
    //  If no parameters are spesified, announce a match
    if ( !(iParameters.iATRBytes || iParameters.iAIDBytes ) )
        {
#ifdef _DEBUG
        RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
            EFileLoggingModeAppend, 
            _L( "CScardConnector::CheckConditions: no parameters\
            specified.\n" ) );
#endif

        iState = EConnectionComplete;
        return ETrue;
        }

    //  Now check the parameters one by one

    //  ATR match ?
    if ( iParameters.iATRBytes )
        {
        if ( aATR )
            {
            if ( *iParameters.iATRBytes == *aATR )
                {
#ifdef _DEBUG
                RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
                    EFileLoggingModeAppend, 
                    _L( "CScardConnector::CheckConditions: ATR match." ) );
#endif

                iState = EConnectionComplete;
                return ETrue;
                }
            }
#ifdef _DEBUG
        RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName,     
            EFileLoggingModeAppend, 
            _L( "CScardConnector::CheckConditions: ATR mismatch!\n" ) );
#endif

        return EFalse;
        }

    //  The card has the requiered application ?
    else if ( iParameters.iAIDBytes )
        {
#ifdef _DEBUG
        RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName,     
            EFileLoggingModeAppend, 
            _L( "CScardConnector::CheckConditions: AIDBytes not supported!\n" ) );
#endif
        return EFalse;
        }
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CScardConnector::NewConnectionL
// Create new connection to the reader.
// -----------------------------------------------------------------------------
//
TBool CScardConnector::NewConnectionL(
    const RMessage2 aMessage, 
    const TReaderID aReaderID )
    {
    _WIMTRACE(_L("WIM|Scard|CScardConnector::NewConnectionL|Begin"));
    TConnection conn;

    conn.iReaderID = aReaderID;

    //  Assign an access controller to this connection
    conn.iCtrl = iConnectionRegistry->Server()->AccessRegistry()->
        AccessController( aReaderID );
    
    //  Make sure there was enough memory for the access controller
    __ASSERT_MEMORY( !(conn.iCtrl) );

    //  attempt to create the connection's reader
    TRAPD( readerError, conn.iReader = conn.iCtrl->AttachSessionToReaderL( 
        this, conn.iSessionID ) );

    //  If creating the reader succeeds, attempt to initialise the reader
    if ( readerError == KErrNone )
        {
#ifdef _DEBUG    
        RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
            EFileLoggingModeAppend, 
            _L( "CScardConnector::NewConnectionL: Reader (ID: %d)\
            succesfully created.\n" ), aReaderID );
#endif
        const TBool ok( conn.iCtrl->InitialiseReader( conn.iSessionID, 
            aMessage ) );
        iConnections->AppendL( conn );
        return ok;
        }

#ifdef _DEBUG
    RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName,         
        EFileLoggingModeAppend, 
        _L( "CScardConnector::NewConnectionL: FAILED! error: %d\n" ),
        readerError );
#endif

    conn.iCtrl->DetachSessionFromReader( conn.iSessionID );
    User::Leave( readerError );
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CScardConnector::FindReaderConnectionL
// Find connection for given reader.
// -----------------------------------------------------------------------------
//
TConnection& CScardConnector::FindReaderConnectionL(
    const TReaderID& aReaderID )
    {
    _WIMTRACE(_L("WIM|Scard|CScardConnector::FindReaderConnectionL|Begin"));
    TInt count = iConnections->Count();
    for ( TInt i( 0 ); i < count; i++ )
        {
        if ( iConnections->At( i ).iReaderID == aReaderID )
            {
            return iConnections->At( i );
            }
        }
    User::Leave( KScErrNotFound );
    return iConnections->At( 0 ); //just to satisfy compiler
    }

// -----------------------------------------------------------------------------
// CScardConnector::ConnectionDone
// Connection has been established. Remove connector.
// -----------------------------------------------------------------------------
//
void CScardConnector::ConnectionDone(
    const TReaderID aReaderID, 
    const TInt& aErrorCode )
    {
    _WIMTRACE(_L("WIM|Scard|CScardConnector::ConnectionDone|Begin"));
#ifdef _DEBUG
    RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
        EFileLoggingModeAppend, _L( "CScardConnector::ConnectionDone \
        status: %d \n" ), aErrorCode );
#endif
    if ( aErrorCode != KErrNone )
        {
        iState = EConnectionComplete;
        }
    if ( iTimer )
        {
#ifdef _DEBUG
        RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
            EFileLoggingModeAppend, _L( "CScardConnector::ConnectionDone Cancel\
            timer. \n" ) );
#endif
        iTimer->Cancel();
        }

    if ( aErrorCode == KScErrTimeOut )
        {
        iConnectionRegistry->Server()->
            FindAccessControl( aReaderID )->Cancel();
      
        }

    iConnectionRegistry->ConnectDone( this, aReaderID, aErrorCode );
    if ( iState == EConnectionComplete )
        {
        iConnectionRegistry->RemoveConnector( this );
        delete this;
        }
    }

// -----------------------------------------------------------------------------
// CScardConnector::Cancel
// Cancel connection. Call ConnectionDone with error code KScErrCancelled.
// -----------------------------------------------------------------------------
//
void CScardConnector::Cancel()
    {
    _WIMTRACE(_L("WIM|Scard|CScardConnector::Cancel|Begin"));
    iState = EConnectionComplete;
    ConnectionDone( 0, KScErrCancelled );
    }

// -----------------------------------------------------------------------------
// CScardConnector::Message
// Return client message.
// -----------------------------------------------------------------------------
//
RMessage2& CScardConnector::Message()
    {
    _WIMTRACE(_L("WIM|Scard|CScardConnector::Message|Begin"));
    return *iClientMessage;
    }

// -----------------------------------------------------------------------------
// CScardConnector::ConnectionTimedOut
// Timeout has occurred. Connectin done with KScErrTimeOut.
// -----------------------------------------------------------------------------
//
void CScardConnector::ConnectionTimedOut()
    {
    _WIMTRACE(_L("WIM|Scard|CScardConnector::ConnectionTimedOut|Begin"));
#ifdef _DEBUG
    RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
        EFileLoggingModeAppend, 
        _L( "CScardConnector::ConnectionTimedOut state=%d\n" ), iState );
#endif
    if ( iState == EWaitingForConfirm )
        {
        iState = ETimedOut;
        return;
        }
    else
        {
        iState = EConnectionComplete;
        ConnectionDone( iConnections->At( 0 ).iReaderID, KScErrTimeOut );
        }
    }

//  End of File  
