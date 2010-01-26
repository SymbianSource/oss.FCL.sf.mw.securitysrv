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
* Description:  This object is the one responsible for trafficing between
*                the Smart Card reader and the sessions.
*
*/



// INCLUDE FILES
#include    "ScardServer.h"
#include    "ScardReaderRegistry.h"
#include    "ScardAccessControl.h"
#include    "ScardAccessControlRegistry.h"
#include    "ScardCommandTimer.h"
#include    "ScardConnector.h"
#include    <c32comm.h>         // Needed for RCommServ in WINS
#include    "WimTrace.h"

#ifdef _DEBUG // for logging
#include    "ScardLogs.h"
#include    <flogger.h> 
#endif

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// TMessageHandle::TMessageHandle
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
TMessageHandle::TMessageHandle(
    const RMessage2& aMessage,
    const TInt aSessionID,
    const TReaderID aReaderID,
    const TInt8 aChannel,
    const TInt8 aAddition)
    : iMessage( aMessage ),
      iSessionID( aSessionID ),
      iReaderID( aReaderID ), 
      iCancelled( EFalse ),
      iChannel( aChannel ),
      iTimer( NULL ), 
      iAdditionalParameter( aAddition )
    {
    }

// -----------------------------------------------------------------------------
// TMessageHandle::TMessageHandle
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
TMessageHandle::TMessageHandle()
    : iMessage(), 
      iSessionID( ENoSession ),
      iReaderID( 0 ),
      iCancelled( EFalse ), 
      iChannel( 0 ),
      iTimer( NULL ),
      iAdditionalParameter( 0 )
    {
    }

// -----------------------------------------------------------------------------
// CScardAccessControl::CScardAccessControl
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CScardAccessControl::CScardAccessControl(
    const TReaderID aReaderID, 
    CScardAccessControlRegistry* aControlRegistry )
    : CActive( EPriorityNormal ), 
      iSessionRegistry( NULL ),
      iReaderActive( EFalse ),
      iIsCreated( EFalse ),
      iIsOpen( EFalse ),
      iNextSessionID( 1 ),
      iReader( NULL ),
      iReaderID( aReaderID ),
      iControlRegistry( aControlRegistry ),
      iManager( NULL ),
      iLifeMode( ECanBeDeleted )
    {
    _WIMTRACE(_L("WIM|Scard|CScardAccessControl::CScardAccessControl|Begin"));
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CScardAccessControl::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CScardAccessControl::ConstructL()
    {
    _WIMTRACE(_L("WIM|Scard|CScardAccessControl::ConstructL|Begin"));
    iSessionRegistry = new( ELeave ) CArrayFixFlat<TReaderSession>( 1 );
    iManager = CScardChannelManager::NewL();
    }

// -----------------------------------------------------------------------------
// CScardAccessControl::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CScardAccessControl* CScardAccessControl::NewL(
    const TReaderID aReaderID, 
    CScardAccessControlRegistry* aControl )
    {
    _WIMTRACE(_L("WIM|Scard|CScardAccessControl::NewL|Begin"));
    CScardAccessControl* self = new( ELeave ) CScardAccessControl( aReaderID, 
        aControl );
    
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

#ifdef _DEBUG    
    RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
        EFileLoggingModeAppend, 
        _L( "Access controller created.\n" ) );
#endif
    return self;
    }

    
// Destructor
CScardAccessControl::~CScardAccessControl()
    {
    _WIMTRACE(_L("WIM|Scard|CScardAccessControl::~CScardAccessControl|Begin"));
    Cancel();

    if ( iIsCreated )
        {
        DetachSessionFromReader( EAccessMasterID );
        }
    if ( iControlRegistry )
        {
        iControlRegistry->ControllerRetired( this );
        }
    delete iSessionRegistry;
    delete iManager;
#ifdef _DEBUG    
    RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
        EFileLoggingModeAppend, 
        _L( "Access controller destroyed.\n" ) );
#endif
    }

// -----------------------------------------------------------------------------
// CScardAccessControl::InitiateCommunication
// Set the reader as active. Prevents polling and other sessions 
// from using the reader. Returns the request status of this object.
// -----------------------------------------------------------------------------
//
TRequestStatus& CScardAccessControl::InitiateCommunication(
    const TInt aSessionID,
    const RMessage2& aMessage, 
    const TInt32 aTimeOut,
    const TInt8 aChannel,
    const TUint8 aAdditionalParameter )
    {
    _WIMTRACE(_L("WIM|Scard|CScardAccessControl::InitiateCommunication|Begin"));
#ifdef _DEBUG    
    RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
        EFileLoggingModeAppend, 
        _L( "CScardAccessControl::InitiateCommunication session: %d, channel:\
        %d, reader: %d\n" ), aSessionID, aChannel, iReaderID );
#endif
    iReaderActive = ETrue;

    TMessageHandle handle( aMessage, aSessionID, iReaderID, aChannel, 
        aAdditionalParameter );
    if ( aTimeOut )
        {
#ifdef _DEBUG    
        RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
            EFileLoggingModeAppend, 
            _L( "CScardAccessControl::InitiateCommunication create command\
            timer\n" ) );
#endif
        TRAPD( err, 
            handle.iTimer = CScardCommandTimer::NewL( aTimeOut, this ) );
        if ( err )
            {
            return iStatus;
            }
        handle.iTimer->StartTiming();
        }

    TRAPD( err, iManager->PushMessageToTopL( handle ) );
    if ( !err )
        {
        SetActive();
        }
    iStatus = err;
    _WIMTRACE2(_L("WIM|Scard|CScardAccessControl::InitiateCommunication|End|iStatus=%d"), err);
    return iStatus;
    }

// -----------------------------------------------------------------------------
// CScardAccessControl::ReaderIsReady
// Ask if the reader is available to use for aSessionID.
// -----------------------------------------------------------------------------
//
TBool CScardAccessControl::ReaderIsReady(
    const TInt aSessionID, 
    const TInt8 aChannel ) const 
    {
    _WIMTRACE(_L("WIM|Scard|CScardAccessControl::ReaderIsReady|Begin"));
    TInt res( 0 );

    TRAPD( err, res = iManager->ChannelReservedL( aChannel ) );
    if ( err )
        {
        return EFalse;
        }
    if ( res )
        {
        if ( res != aSessionID && aSessionID != EAccessMasterID )
            {
#ifdef _DEBUG    
            RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
            EFileLoggingModeAppend, 
            _L( "CScardAccessControl::ReaderIsReady: Channel reserved by\
            another session.\n" ) );
#endif
            return EFalse;
            }
        }
    
    if ( iReaderActive ) 
        {
#ifdef _DEBUG    
        RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
            EFileLoggingModeAppend, 
            _L( "CScardAccessControl::ReaderIsReady: Reader active.\n" ) );
#endif
        return EFalse;
        }
    else
        {
        return ETrue;
        }
    }

// -----------------------------------------------------------------------------
// CScardAccessControl::FreeChannelL
// Free current reservation (if this session has a reservation)
// If there are messages in the stack, service them.
// -----------------------------------------------------------------------------
//
void CScardAccessControl::FreeChannelL(
    const TInt aSessionID, 
    const TInt8 aChannel )
    {
    _WIMTRACE(_L("WIM|Scard|CScardAccessControl::FreeChannelL|Begin"));
    if ( aChannel == KAllChannels )
        {
        iManager->FreeChannels( aSessionID );
        }
    else
        {
        iManager->FreeChannelL( aChannel, aSessionID );
        }

    if ( !iReaderActive )
        {
        HandleNextMessageL( iManager->NextMessageFromFree( aChannel ) );
        }
    }

// -----------------------------------------------------------------------------
// CScardAccessControl::RunL
// Handle completed reader messages by finding the caller, and calling 
// the complete function. Also take the next message from the stack (if 
// present) and start handling it.
// -----------------------------------------------------------------------------
//
void CScardAccessControl::RunL()
    {
    _WIMTRACE(_L("WIM|Scard|CScardAccessControl::RunL|Begin"));
#ifdef _DEBUG        
    RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName,     
        EFileLoggingModeAppend, 
        _L( "CScardAccessControl::RunL\n" ) );
#endif
    iLifeMode = ECanNotDelete;

    //  reader has finished, mark it as inactive
    iReaderActive = EFalse;
    
    //  Get the message that was serviced, and the session that made the 
    //  request
    TMessageHandle msgHandle = iManager->MessageFromTop();

    //  Controller needs to know if a connection has been successful
    if ( msgHandle.iAdditionalParameter == KOpenReader ) 
        {
        // If connection was succesful, ConnectionDone() is called from
        // notifier. Otherwise, call it from here.
        if ( iStatus == KErrNone )
            {
            iIsOpen = ETrue;
            }
        else 
            {
            iControlRegistry->Server()->ConnectionRegistry()->
                Connection( 0 ).iConnector->ConnectionDone( 
                msgHandle.iReaderID, iStatus.Int() );
            }
        }

    //  Handle result in appropriate session
    //  In case the operation was cancelled, continue from the next operation
    //  w/o completing the last one.
    //  In case of connecting to reader, ConnectionDone() takes care of 
    //  completing the message. 
    if ( !msgHandle.iCancelled &&
        msgHandle.iAdditionalParameter != KOpenReader )
        {
        CScardSession* session = SessionBase( msgHandle.iSessionID );
        if ( !session )
            {
            User::Panic( _L( "Message stack fault" ), 
                KScServerPanicInternalError );
            }
#ifdef _DEBUG    
        RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
            EFileLoggingModeAppend, 
            _L( "CScardAccessControl::RunL: Message %x completed with status\
            %d.\n" ), msgHandle.iAdditionalParameter, iStatus.Int() );
#endif
        session->AsynchronousServiceComplete( msgHandle, iStatus.Int() );
        }

    //  The session may have indicated that it's about time for this 
    //  object to destroy itself
    if ( iLifeMode == EDestroyASAP )
        {
        delete this;
        return;
        }

    iLifeMode = ECanBeDeleted;

    //  Check for queued messages. 
    HandleNextMessageL( iManager->NextMessageL() );
    }

// -----------------------------------------------------------------------------
// CScardAccessControl::DoCancel
// Instruct the reader to cancel
// -----------------------------------------------------------------------------
//
void CScardAccessControl::DoCancel()
    {
    _WIMTRACE(_L("WIM|Scard|CScardAccessControl::DoCancel|Begin"));
    if ( iReader )
        {
#ifdef _DEBUG    
        RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
            EFileLoggingModeAppend, 
            _L( "CScardAccessControl::DoCancel: Canceling transmissions.\n" ) );
#endif
        TRAPD( err, CancelTransmissionsL( EAccessMasterID ) );
        
        if ( err != KErrNone )
            {
            // Do nothing
            }
        }
    }

// -----------------------------------------------------------------------------
// CScardAccessControl::QueueExecution
// Queue the execution of a command until it can be serviced
// -----------------------------------------------------------------------------
//
void CScardAccessControl::QueueExecution(
    const RMessage2& aMessage, 
    const TInt aSessionID,
    const TInt32 aTimeOut, 
    const TInt8 aChannel, 
    const TInt8 aParameter )
    {
    _WIMTRACE(_L("WIM|Scard|CScardAccessControl::QueueExecution|Begin"));
    TMessageHandle tmp( aMessage, aSessionID, iReaderID, aChannel, aParameter );
    TInt err;
    if ( aTimeOut )
        {
        TRAP( err, tmp.iTimer = CScardCommandTimer::NewL( aTimeOut, this ) );
        if ( err )
            {
            tmp.iMessage.Complete( err );
            }
        tmp.iTimer->StartTiming();
        }

    TRAP( err, iManager->PushMessageToBottomL( tmp ) );
    if ( err )
        {
        tmp.iMessage.Complete( err );
        }
    }

// -----------------------------------------------------------------------------
// CScardAccessControl::QueueChannelOperation
// Queue channel operation
// -----------------------------------------------------------------------------
//
void CScardAccessControl::QueueChannelOperation(
    const RMessage2& aMessage, 
    const TInt aSessionID, 
    const TInt32 aTimeOut, 
    const TInt8 aChannel )
    {
    _WIMTRACE(_L("WIM|Scard|CScardAccessControl::QueueChannelOperation|Begin"));
    TMessageHandle tmp( aMessage, aSessionID, iReaderID, aChannel,
        KReservation );
    if ( aTimeOut )
        {
        TRAPD( err, tmp.iTimer = CScardCommandTimer::NewL( aTimeOut, this ) );
        if ( err )
            {
            tmp.iMessage.Complete( err );
            }
        tmp.iTimer->StartTiming();
        }

    TRAPD( err, iManager->PushMessageToBottomL( tmp ) );
    if ( err )
        {
        tmp.iMessage.Complete( err );
        }
    }

// -----------------------------------------------------------------------------
// CScardAccessControl::IsAttached
// Is this session attached to the reader yet?
// -----------------------------------------------------------------------------
//
TBool CScardAccessControl::IsAttached( CScardSession* aSession ) const
    {
    _WIMTRACE(_L("WIM|Scard|CScardAccessControl::IsAttached|Begin"));
    TInt sessionCount = iSessionRegistry->Count();
    for ( TInt i( 0 ); i < sessionCount; i++ )
        {
        if ( (*iSessionRegistry)[i].SessionBase == aSession )
            {
            return ETrue;
            }
        }
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CScardAccessControl::SessionBase
// Returns CScardSession* belonging to this session id
// -----------------------------------------------------------------------------
//
CScardSession* CScardAccessControl::SessionBase( const TInt aSessionID )
    {
    _WIMTRACE(_L("WIM|Scard|CScardAccessControl::SessionBase|Begin"));
    TInt sessionCount = iSessionRegistry->Count();
    for ( TInt i( 0 ); i < sessionCount; i++ )
        {
        if ( (*iSessionRegistry)[i].SessionID == aSessionID )
            {
            return (*iSessionRegistry)[i].SessionBase;
            }
        }
    return NULL;
    }

// -----------------------------------------------------------------------------
// CScardAccessControl::CardEvent
// Handle card event
// -----------------------------------------------------------------------------
//
void CScardAccessControl::CardEvent(
    const TScardServiceStatus aEvent, 
    const TScardATR& aATR )
    {
    _WIMTRACE(_L("WIM|Scard|CScardAccessControl::CardEvent|Begin"));

    if ( aATR.Length() )
        {
        iATR = aATR;
        }

    if ( aEvent == EScardRemoved )
        {
        iManager->CardRemoved();
        }

    TInt sessionCount = iSessionRegistry->Count();

    for ( TInt index( 0 ); index < sessionCount ; index++ )
        {
        iSessionRegistry->At( index ).SessionBase->CardEvent( aEvent, aATR, 
            iReaderID );
        }
    }

// -----------------------------------------------------------------------------
// CScardAccessControl::CancelTransmissionsL
// Cancel Transmissions
// -----------------------------------------------------------------------------
//
void CScardAccessControl::CancelTransmissionsL( const TInt aSessionID )
    {
    _WIMTRACE(_L("WIM|Scard|CScardAccessControl::CancelTransmissions|Begin"));
#ifdef _DEBUG    
    RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName,     
        EFileLoggingModeAppend, 
        _L( "CScardAccessControl::CancelTransmissions: Purge messages from \
        stack.\n" ) );
#endif
    //  First purge the messages from the stack
    iManager->CancelAll( aSessionID );

    //  If the message currently served is for the calling session, cancel it 
    //  from the reader also and put it back
    TMessageHandle tmp = iManager->MessageFromTop();
    if ( tmp.iSessionID == aSessionID || aSessionID == EAccessMasterID )
        {
#ifdef _DEBUG    
        RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName,     
            EFileLoggingModeAppend, 
            _L( "CScardAccessControl::CancelTransmissions: Cancel message from\
            reader.\n" ) );
#endif
     
        iReader->CancelTransmit();
        
        if ( aSessionID == EAccessMasterID )
            {
            // reader not active anymore
            iReaderActive = EFalse;
            }    
        }

    if ( tmp.iSessionID != ENoSession && aSessionID != EAccessMasterID )
        {
        iManager->PushMessageToTopL( tmp );
        }

    //  Finish by clearing any reservations the cancelling session has done
#ifdef _DEBUG    
    RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName,     
        EFileLoggingModeAppend, 
        _L( "CScardAccessControl::CancelTransmissions: Free reserved\
        channels.\n" ) );
#endif
    iManager->FreeChannels( aSessionID );
    }

// -----------------------------------------------------------------------------
// CScardAccessControl::DetachSessionFromReader
// Detach session from reader
// -----------------------------------------------------------------------------
//
void CScardAccessControl::DetachSessionFromReader( const TInt aSessionID )
    {
    _WIMTRACE(_L("WIM|Scard|CScardAccessControl::DetachSessionFromReader|Begin"));
    if ( aSessionID == EAccessMasterID )
        {
#ifdef _DEBUG    
        RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
            EFileLoggingModeAppend, 
            _L( "CScardAccessControl::DetachSessionFromReader: Detach all\
            sessions...\n" ) );
#endif

        //  detach all sessions and delete all their messages
        DequeueOperations( EAccessMasterID );
        while ( iSessionRegistry->Count() )
            {
            iSessionRegistry->Delete( 0 );
            }

        //  all sessions have been detached, so kill the reader (if it's around)
        if ( iReader )
            {
            iReader->Close();
            }
        iControlRegistry->Server()->FactoryRegistry()->CloseReader(
                iReaderID );
        iReader = NULL;
        iIsOpen = EFalse;
        iIsCreated = EFalse;
        }
    
    else
        {
#ifdef _DEBUG    
        RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
            EFileLoggingModeAppend, 
            _L( "CScardAccessControl::DetachSessionFromReader: Detach session\
            %d...\n" ), aSessionID );
#endif
        TInt i( 0 );
        if ( !IsAttached( SessionBase( aSessionID ) ) ) 
            {
#ifdef _DEBUG    
            RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
                EFileLoggingModeAppend, 
                _L( "CScardAccessControl::DetachSessionFromReader: Session not\
                attached!!!\n" ), aSessionID );
#endif
            return;
            }
        
        //  locate the session to be detached
        for ( ; (*iSessionRegistry)[i].SessionID != aSessionID; i++ )
            {;}
        
        iSessionRegistry->Delete( i );

        //  pop out all messages queued to it
        DequeueOperations( aSessionID );

        //  Free any possible transactions with the smart card
        TRAPD( error, FreeChannelL( aSessionID, KAllChannels ) );

        if ( error != KErrNone )
            {
            // Do nothing
            }
        }
    }

// -----------------------------------------------------------------------------
// CScardAccessControl::DequeueOperations
// Cancel all pending messages
// -----------------------------------------------------------------------------
//
void CScardAccessControl::DequeueOperations( const TInt aSessionID )
    {
    _WIMTRACE(_L("WIM|Scard|CScardAccessControl::DequeueOperations|Begin"));
    iManager->CancelAll( aSessionID );
    }

// -----------------------------------------------------------------------------
// CScardAccessControl::InitialiseReader
// Initialise reader handler
// -----------------------------------------------------------------------------
//
TBool CScardAccessControl::InitialiseReader(
    const TInt aSessionID, 
    const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM|Scard|CScardAccessControl::InitialiseReader|Begin"));
    if ( iIsOpen )
        {
#ifdef _DEBUG    
        RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
            EFileLoggingModeAppend, 
            _L( "CScardAccessControl::InitialiseReader: Reader already\
            open.\n" ) );
#endif
        return ETrue;
        }

    if ( ReaderIsReady( EAccessMasterID, 0 ) )
        {
        InitiateCommunication( aSessionID, aMessage, static_cast< TInt32>( 0 ),
            0, KOpenReader );
#if defined (__WINS__)
        RCommServ commServer;
        TInt error = commServer.Connect();
        if ( error == KErrNone)
            {
            commServer.Close();         	
            }
#endif
#ifdef _DEBUG    
        RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
            EFileLoggingModeAppend, 
            _L( "CScardAccessControl::InitialiseReader: Opening Reader\n" ) );
#endif
        iReader->Open( iStatus );
        }
#ifdef _DEBUG    
    else 
        {
        RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
          EFileLoggingModeAppend, 
          _L( "CScardAccessControl::InitialiseReader: Reader not ready!.\n" ) );
        }
#endif
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CScardAccessControl::AttachSessionToReaderL
// Attach session to reader
// -----------------------------------------------------------------------------
//
MScardReader* CScardAccessControl::AttachSessionToReaderL(
    CScardSession* aSession, 
    TInt &aSessionID )
    {
    _WIMTRACE(_L("WIM|Scard|CScardAccessControl::AttachSessionToReaderL|Begin"));
    TReaderSession newSession;
    newSession.SessionID = iNextSessionID;
    newSession.SessionBase = aSession;

    TInt readerCreationError( 0 );  

    //  If no other session has connected yet or the reader launcher has been
    //  unable to create the reader object, go here...
    if ( !iIsCreated )
        {
#ifdef _DEBUG    
        RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
            EFileLoggingModeAppend, 
            _L( "CScardAccessControl::AttachSessionToReader: reader not\
            created!\n" ) );
#endif
        //  ... and ask the launcher to create the reader object
        CScardReaderRegistry* reg = 
            iControlRegistry->Server()->FactoryRegistry();
        TRAP( readerCreationError, iReader = reg->LoadReaderL( iReaderID ) );
        }
    
#ifdef _DEBUG        
    RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName,     
        EFileLoggingModeAppend, 
        _L( "CScardAccessControl::AttachSessionToReader: session ID %d\n" ),
        newSession.SessionID );
#endif

    iSessionRegistry->AppendL( newSession );
    
    // Place session ID and increment the next ID
    aSessionID = iNextSessionID++;

    //  All sessions are immediately on channel 0
    iManager->AddSessionToChannelL( KChannel0, aSessionID );

    if ( !readerCreationError && iReader )
        {
        iIsCreated = ETrue;
        return iReader;
        }
    else
        {
#ifdef _DEBUG    
        RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
            EFileLoggingModeAppend, 
            _L( "CScardAccessControl::AttachSessionToReader: reader creation\
            failed!!!\n" ) );
#endif
        iIsCreated = EFalse;
        User::Leave( readerCreationError );
        return NULL;
        }
    }

//  End of File  
