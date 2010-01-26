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
* Description:  Session for the Scard server, to a single client-side session
*
*/



// INCLUDE FILES
#include    "ScardBase.h"
#include    "ScardReader.h"
#include    "ScardClsv.h"
#include    "ScardNotifyRegistry.h"
#include    "ScardSession.h"
#include    "ScardServer.h"
#include    "ScardAccessControl.h"
#include    "WimTrace.h"

#ifdef _DEBUG // for logging
#include    "ScardLogs.h"
#include    <flogger.h> 
#endif

// LOCAL CONSTANTS AND MACROS
const TInt KResponseLengthCloseChannel = 0;
const TInt KResponseLengthOpenChannel  = 1;
const TInt KCommandLengthCloseChannel  = 4;
const TInt KCommandLengthOpenChannel   = 6;

// Assert that the channel has been opened for this session, and if not, leave
#define __ASSERT_CHANNEL_OPEN( a ) TRAPD( channelClosed,\
                    iAccessCtrl->ValidateChannelL( ( a ), iSessionID ) );\
                    if ( channelClosed )\
                        {\
                        aMessage.Complete( KScServerErrIllegalChannel );\
                        return;\
                        }

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CScardSession::CScardSession
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CScardSession::CScardSession()
    : CSession2()
    {
    _WIMTRACE(_L("WIM|Scard|CScardSession::CScardSession|Begin"));
    }

// -----------------------------------------------------------------------------
// CScardSession::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CScardSession::ConstructL( CScardServer* aServer )
    {
    _WIMTRACE(_L("WIM|Scard|CScardSession::ConstructL|Begin"));
    iServer         = aServer;
    iNotifyRegistry = iServer->NotifyRegistry();
    }

// -----------------------------------------------------------------------------
// CScardSession::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CScardSession* CScardSession::NewL( CScardServer* aServer )
    {
    _WIMTRACE(_L("WIM|Scard|CScardSession::NewL|Begin"));
    CScardSession* self = new( ELeave ) CScardSession;
    
    CleanupStack::PushL( self );
    self->ConstructL( aServer );
    CleanupStack::Pop( self );

    return self;
    }

// Destructor
CScardSession::~CScardSession()
    {
    DeleteBuffers();
    }


// -----------------------------------------------------------------------------
// CScardSession::ServiceL
// Handle the client message
// -----------------------------------------------------------------------------
//
void CScardSession::ServiceL( const TMessageHandle& aMessageHandle )
    {
    _WIMTRACE(_L("WIM|Scard|CScardSession::ServiceL|Begin"));
    ServiceL( aMessageHandle.iMessage );
    }

// -----------------------------------------------------------------------------
// CScardSession::CloseSession
// Clean up the reader event listeners
// -----------------------------------------------------------------------------
//
void CScardSession::CloseSession( const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM|Scard|CScardSession::CloseSession|Begin"));
    if ( iAccessCtrl && iAccessCtrl->IsAttached( this ) )
        {
        iAccessCtrl->DetachSessionFromReader( iSessionID );
        }

    RThread client;
    aMessage.Client( client );
    iNotifyRegistry->RemoveReaderEventListeners( client );
    }

// -----------------------------------------------------------------------------
// CScardSession::AsynchronousServiceComplete
// An asynchronous service has been completed by the reader.
// -----------------------------------------------------------------------------
//
void CScardSession::AsynchronousServiceComplete(
    const TMessageHandle& aMessageHandle, 
    const TInt aErrorCode )
    {
    _WIMTRACE(_L("WIM|Scard|CScardSession::AsynchronousServiceComplete|Begin"));
    const RMessage2& aMessage = aMessageHandle.iMessage;
    //  In all cases, if the operation has encountered a problem, ignore 
    //  results and just complain to the client that something went wrong.
    if ( aErrorCode != KErrNone )
        {
#ifdef _DEBUG
         RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
            EFileLoggingModeAppend, 
            _L( "CScardSession::AsynchronousServiceComplete ERROR: %d.\n" ),
            aErrorCode );
#endif

        DeleteBuffers();
        aMessage.Complete( aErrorCode );
        return;
        }

    //  Now, see what got serviced
    _WIMTRACE2(_L("WIM|Scard|CScardSession::AsynchronousServiceComplete|Event=%d"), aMessage.Function());
    switch ( aMessage.Function() )
        {
        case EScardServerTransmitToCard:
            {
#ifdef _DEBUG
             RFileLogger::Write( KScardLogDir, KScardLogFileName, 
                EFileLoggingModeAppend, 
                _L( "Transmit to card completed.\n" ) );
#endif
            //  Write response to client and cleanup
            WriteToClient( aMessage, *iResponse, 1 );
            DeleteBuffers();
            aMessage.Complete( KErrNone );
            break;
            }

        case EScardServerGetCapabilities:
            {
#ifdef _DEBUG
             RFileLogger::Write( KScardLogDir, KScardLogFileName, 
                EFileLoggingModeAppend, 
                _L( "Get capabilities completed.\n" ) );
#endif
            //  Write response to client and cleanup
            WriteToClient( aMessage, *iResponse, 2 );
            DeleteBuffers();
            aMessage.Complete( KErrNone );
            break;
            }

        case EScardServerManageChannel:
            {
#ifdef _DEBUG
             RFileLogger::Write( KScardLogDir, KScardLogFileName, 
                EFileLoggingModeAppend, 
                _L( "Manage channel completed.\n" ) );
#endif

            //  Which type of channel management got serviced?
            switch ( aMessage.Int0() )
                {
                case EOpenAnyChannel:
                    {
                    //  Was there a proper response 
                    //  (aka. the command was successful)
                    if ( iResponse->Length() == KOpenChannelResponseLength )
                        {
                        //  Add this session to the newly opened channel
                        TBool ok( EFalse );
                        TRAPD( mem, ok = iAccessCtrl->AddSessionToChannelL( 
                            (*iResponse)[0], iSessionID ) ); 
                        __ASSERT_MEMORY( mem );
                        if ( !ok )
                            {
                            aMessage.Complete( KScErrNotSupported );
                            return;
                            }
                        }
                    //  Then write the card's response back to the client 
                    //  and cleanup
                    WriteToClient( aMessage, *iResponse, 1 );
                    DeleteBuffers();                    
                    aMessage.Complete( KErrNone );
                    break;
                    }
                case ECloseChannel:
                    {
                    //  Channel closed, OK
                    aMessage.Complete( KErrNone );
                    DeleteBuffers();
                    break;
                    }
                default:
                    {
                    break;
                    }
                }
            return;
            }

        default:
            {
            User::Panic( _L( "Message Stack Corrupted" ), 
                KScServerPanicInternalError );
            break;
            }
        }
    }

// -----------------------------------------------------------------------------
// CScardSession::ServiceL
// Message handler for the session.
// Dispatches requests to the appropriate handler
// Trap harness for dispatcher
// -----------------------------------------------------------------------------
//
void CScardSession::ServiceL( const RMessage2& aMessage )
    {
    _WIMTRACE2(_L("WIM|Scard|CScardSession::ServiceL|Begin|Event=%d"), aMessage.Function());
    //  Divide the operation to the handler functions
    switch ( aMessage.Function() )
        {
        //  These functions are synchronous, and don't use message stack,
        //  so we can complete the messages here.
        case EScardServerNotifyChange:
            {
            iNotifyRegistry->AddReaderEventListenerL( aMessage );
            // Does not complete message because it is completed when card
            // event occurs
            break;
            }
        case EScardServerCancelNotifyChange:
            {
            iNotifyRegistry->RemoveReaderEventListener( aMessage );
            aMessage.Complete( KErrNone );
            break;
            }
        case EScardServerCloseSession:
            {
            CloseSession( aMessage );
            aMessage.Complete( KErrNone );
            break;
            }
        case EScardServerDisconnectFromReader:
            {
            DisconnectFromReader();
            aMessage.Complete( KErrNone );
            break;
            }
        case EScardServerCancelTransmit:
            {
            CancelTransmitL();
            aMessage.Complete( KErrNone );
            break;
            }
        //  These functions are asynchronous, and (may) put their messages
        //  in the message stack, so they do completion internally.
        case EScardServerConnectToReader:
            {
            TInt desLen = aMessage.GetDesMaxLength( 1 );
            _WIMTRACE2(_L( "WIM|Scard|EScardServerConnectToReader desLen %d" ), desLen );
            if ( desLen <= 0 )
               {
               _WIMTRACE(_L(" WIM|Scard|EScardServerConnectToReader Descriptor is bad"));
               aMessage.Panic( _L( "ScardServer" ), KErrBadDescriptor );
               break;
               }
            ConnectToReaderL( aMessage );
            break;
            }
         case EScardServerGetATR:
            {
            aMessage.Complete( KErrNotSupported );
            break;
            }
        case EScardServerTransmitToCard:
            {
            TransmitToCard( aMessage );
            break;
            }
        case EScardServerGetCapabilities:
            {
            GetCapabilities( aMessage );
            break;
            }
        case EScardServerManageChannel:
            {
            ManageChannel( aMessage );
            break;
            }
        
        //  A corrupt request
        default:
            aMessage.Panic( _L( "ScardServer" ), KScServerPanicBadRequest );
            break;
        }
    }

// -----------------------------------------------------------------------------
// CScardSession::ConnectToReaderL
// Ask Access Control to add this session to registry, and update session ID
// -----------------------------------------------------------------------------
//
void CScardSession::ConnectToReaderL( const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM|Scard|CScardSession::ConnectToReaderL|Begin"));
#ifdef _DEBUG
    RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
        EFileLoggingModeAppend,
        _L( "CScardSession::ConnectToReaderL entered.\n" ) );
#endif
    //  If already connected, what more could the client want??
    if ( iConnectedToReader )
        {
#ifdef _DEBUG
        RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
         EFileLoggingModeAppend, 
         _L( "CScardSession::ConnectToReaderL FAILED: already connected.\n" ) );
#endif
        aMessage.Complete( KScErrAlreadyExists );
        return;
        }
    // Store client thread from message to member to be used in PanicClient().
    aMessage.Client( iClient );
    // If not, get on with it...            
    iServer->ConnectToReaderL( this, aMessage );
    }

// -----------------------------------------------------------------------------
// CScardSession::DisconnectFromReader
// Ask Access Control to detach this session from the reader.
// -----------------------------------------------------------------------------
//
void CScardSession::DisconnectFromReader()
    {
    _WIMTRACE(_L("WIM|Scard|CScardSession::DisconnectFromReader|Begin"));
    //  If connected at all, disconnect
    if ( iConnectedToReader )
        {
        iAccessCtrl->DetachSessionFromReader( iSessionID );
        iConnectedToReader = EFalse;
        iAccessCtrl = NULL;
        }
    }

// -----------------------------------------------------------------------------
// CScardSession::GetCapabilities
// Get capabilities of the card
// -----------------------------------------------------------------------------
//
void CScardSession::GetCapabilities( const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM|Scard|CScardSession::GetCapabilities|Begin"));
    if ( !iConnectedToReader )
        {
        aMessage.Panic( _L( "Reader not contacted" ), 
            KScPanicNoResourceConnection );
        return;
        }
    const TInt8 channel( 0 );

    if ( iAccessCtrl->ReaderIsReady( iSessionID, channel ) )
        {
        TRequestStatus& status =
            iAccessCtrl->InitiateCommunication( iSessionID, aMessage,
                reinterpret_cast< TInt32>( aMessage.Ptr3() ), channel );
        __ASSERT_MEMORY( status.Int() );
        TInt32 commandCode( 0 );
        commandCode = aMessage.Int1();
        commandCode |= ( aMessage.Int0() << 8 ) & 0xff00;

        //  Get the response buffer
        TInt error = AllocateResponse( aMessage, 2 );
        if ( error )
            {
            aMessage.Complete( error );
            return;
            }

        TInt32 timeout = reinterpret_cast< TInt32>( aMessage.Ptr3() );
        //  ...do the control command
        iReader->GetCapabilities( status, commandCode, *iResponse, timeout );
        }
    //  The reader was busy, queue
    else
        {
#ifdef _DEBUG
        RFileLogger::Write( KScardLogDir, KScardLogFileName, 
        EFileLoggingModeAppend, 
        _L("CScardSession::GetCapabilities()-reader busy, message queued.\n") );
#endif

        iAccessCtrl->QueueExecution( aMessage, iSessionID,
            reinterpret_cast< TInt32 >( aMessage.Ptr3() ), channel );
        }
    }

// -----------------------------------------------------------------------------
// CScardSession::TransmitToCard
// Transmit data to the card
// -----------------------------------------------------------------------------
//
void CScardSession::TransmitToCard( const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM|Scard|CScardSession::TransmitToCard|Begin"));
    if ( !iConnectedToReader )
        {
        aMessage.Panic( _L( "Reader not contacted" ), 
            KScPanicNoResourceConnection );
        return;
        }

    // check if this client has the access rights for the reader
    // and reader is ready
    const TInt8 channel( static_cast< TInt8>( aMessage.Int3() ) );
    __ASSERT_CHANNEL_OPEN( channel );

    if ( iAccessCtrl->ReaderIsReady( iSessionID, channel ) )
        {
        //  Read the command APDU from the client
        TInt error = ReadFromClient( aMessage, iCommandBuffer, iCommand, 0 );
        if ( error )
            {
#ifdef _DEBUG
    RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
        EFileLoggingModeAppend, 
        _L("CScardSession::TransmitToCard()-Cmd APDU couldn't be read from\
        client. Error: %d.\n"), error );
#endif
            return;
            }

        //  If the channel number is not the same one as asked for
        //  in the parameter 3 of the message, adjust the command...
        if ( ( (*iCommand)[KCLA] & 0x0f) != channel )
            {
#ifdef _DEBUG
            RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
            EFileLoggingModeAppend, 
            _L("CScardSession::TransmitToCard() -- Channel changed from %d\
             to: %d.\n"), (*iCommand)[KCLA] & 0x0f, channel & 0x0f );
#endif
            (*iCommand)[KCLA] &= 0xf0;
            (*iCommand)[KCLA] |= channel;
            }

        //  No manage channel apdu's are allowed. 
        //  Use CScardComm::ManageChannel() instead.
        if ( (*iCommand)[KINS] == KManageChannel )
            {
#ifdef _DEBUG
            RFileLogger::Write( KScardLogDir, KScardLogFileName, 
                EFileLoggingModeAppend, 
                _L( "CScardSession::TransmitToCard() -- Error! Manage channel\
                APDU.\n" ) );
#endif
            delete iCommand;
            iCommand = NULL;
            aMessage.Complete( KScServerErrIllegalOperation );
            return;
            }

        //  Alloc response 
        error = AllocateResponse( aMessage, 1 );
        if ( error )
            {
#ifdef _DEBUG
            RFileLogger::Write( KScardLogDir, KScardLogFileName, 
                EFileLoggingModeAppend, 
                _L( "CScardSession::TransmitToCard() -- Could not allocate\
                response.\n" ) );
#endif
            return;
            }

        //  Transfer APDU
        TRequestStatus& sr =
            iAccessCtrl->InitiateCommunication( iSessionID, aMessage,
                reinterpret_cast< TInt32>( aMessage.Ptr2() ), channel );
        __ASSERT_MEMORY( sr.Int() );
        const TInt32 timeout = static_cast< TInt32>( aMessage.Int2() );
#ifdef _DEBUG
        RFileLogger::HexDump( KScardLogDir, KScardLogFileName, 
            EFileLoggingModeAppend, 
            _S( "CScardSession::TransmitToCard() command APDU:\n" ), 0,
            iCommand->Ptr(), iCommand->Length() );
#endif
        iReader->TransmitToCard( sr, *iCommand, *iResponse, timeout );
        }
    //  The reader was busy, queue
    else
        {
#ifdef _DEBUG
        RFileLogger::Write( KScardLogDir, KScardLogFileName, 
            EFileLoggingModeAppend, 
            _L( "CScardSession::TransmitToCard() -- reader busy. message\
            queued.\n" ) );
#endif
        iAccessCtrl->QueueExecution( aMessage, iSessionID, 
            static_cast< TInt32>( aMessage.Int2() ), channel );
        }
    }

// -----------------------------------------------------------------------------
// CScardSession::CancelTransmit
// Cancel all transmissions belonging to this session
// -----------------------------------------------------------------------------
//
void CScardSession::CancelTransmitL()
    {
    _WIMTRACE(_L("WIM|Scard|CScardSession::CancelTransmit|Begin"));
    // It is not clear if this session is attempting to connect, so cancel any
    // connection attempts as well... If this session is not connecting at the
    // moment, this does nothing.
    iServer->ConnectionRegistry()->CancelConnection( this );

    if ( iAccessCtrl )
        {
        //  Cancel from the reader
        iAccessCtrl->CancelTransmissionsL( iSessionID );
        }
    }

// -----------------------------------------------------------------------------
// CScardSession::ManageChannel
// Manage logical channel
// -----------------------------------------------------------------------------
//
void CScardSession::ManageChannel( const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM|Scard|CScardSession::ManageChannel|Begin"));
    if ( !iConnectedToReader )
        {
        aMessage.Panic( _L( "Reader not contacted" ), 
            KScPanicNoResourceConnection );
        return;
        }
    
    const TUint8 maxChannels( 4 );

    //  What exactly does the client want to do?
    const TInt operationCode = aMessage.Int0();
    _WIMTRACE2(_L("WIM|Scard|CScardSession::ManageChannel|operationCode=%d"), operationCode);
    switch ( operationCode )
        {
        //  Open a channel and the card will assign the channel number
        case EOpenAnyChannel:
            {
            //  If we're already on all the channels, return error indicating 
            //  that no more channels are available
            TBool allOpen( ETrue );
            TInt8 i( 0 );
            for ( i = 1; i < maxChannels; i++ )
                {
                TRAPD( closed, iAccessCtrl->ValidateChannelL( i, iSessionID ) );
                if ( closed )
                    {
                    allOpen = EFalse;
                    break;
                    }
                }
            if ( allOpen )
                {
#ifdef _DEBUG
                RFileLogger::Write( KScardLogDir, KScardLogFileName, 
                    EFileLoggingModeAppend, 
                    _L( "CScardSession::ManageChannel() -- Open any channel:\
                    all channels open.\n" ) );
#endif
                _WIMTRACE(_L("WIM|Scard|CScardSession::ManageChannel|no more channels available"));
                aMessage.Complete( KScErrFull );
                return;
                }

            //  Check if all channels have already been opened.
            //  (Of course this session is not yet on all of them)
            allOpen = ETrue;
            for ( i = 1 ; i < maxChannels ; i++ ) 
                {
                if ( !( iAccessCtrl->ChannelOpenedYet( i ) ) )  
                    {
                    allOpen = EFalse;
                    break;
                    }
                }

            //  If all channels have been opened, get the first unreserved
            //  one and assign the session to it. If no unreserved channels
            //  are found, put the message on hold for the next available channel.
            if ( allOpen )
                {
                _WIMTRACE(_L("WIM|Scard|CScardSession::ManageChannel|all channels open"));
                const TInt8 channel = iAccessCtrl->UnreservedLogicalChannel();
                if ( channel > 0 )
                    {
                    _WIMTRACE(_L("WIM|Scard|CScardSession::ManageChannel|unreserved channels found"));
                    TRAPD( err, iAccessCtrl->AddSessionToChannelL( 
                        channel, iSessionID ) ); 
                    __ASSERT_MEMORY( err );
                    _WIMTRACE(_L("WIM|Scard|CScardSession::ManageChannel|session added to channel"));
                    aMessage.Complete( KErrNone );
                    return;
                    }
                else
                    {
                    _WIMTRACE(_L("WIM|Scard|CScardSession::ManageChannel|no unreserved channels found"));
                    iAccessCtrl->QueueChannelOperation( aMessage, iSessionID,
                        reinterpret_cast< TInt32>( aMessage.Ptr2() ),
                        KAllChannels );
                    return;
                    }
                }
            _WIMTRACE(_L("WIM|Scard|CScardSession::ManageChannel|some channels not open"));
            //  The card has logical channels available that have not been 
            //  yet opened. Make the command and send it to the SC
            if ( iAccessCtrl->ReaderIsReady( iSessionID, KChannel0 ) )
                {
                _WIMTRACE(_L("WIM|Scard|CScardSession::ManageChannel|reader ready"));
                DoCommandAndTransmit( 
                    aMessage,
                    KOpenChannel,
                    0,
                    KCommandLengthOpenChannel,
                    KResponseLengthOpenChannel,
                    reinterpret_cast< TInt32 >( aMessage.Ptr2() ),
                    KChannel0 );
                return;
                }
            //  The reader was busy, queue
            else
                {
#ifdef _DEBUG
                RFileLogger::Write( KScardLogDir, KScardLogFileName, 
                    EFileLoggingModeAppend, 
                    _L( "CScardSession::ManageChannel() -- reader busy.\
                    message queued.\n" ) );
#endif
                _WIMTRACE(_L("WIM|Scard|CScardSession::ManageChannel|reader busy"));
                iAccessCtrl->QueueExecution( aMessage, iSessionID,
                    reinterpret_cast< TInt32>( aMessage.Ptr2() ), KChannel0 );
                return;
                }
            //break; //unreachable
            }

        //  Close a channel
        case ECloseChannel:
            {
            const TUint8 channel( static_cast< TUint8>( aMessage.Int1() ) );
            TBool stillOpen( EFalse );
            //  Remove this session from the channel
            TRAPD( err, stillOpen = iAccessCtrl->RemoveSessionFromChannelL( 
                channel, iSessionID ) );
            if ( err != KErrNone )
                {
#ifdef _DEBUG
                RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
                    EFileLoggingModeAppend, 
                    _L( "CScardSession::ManageChannel() -- Close channel:\
                    Error %d in removing session.\n" ), err );
#endif
                aMessage.Complete( err );
                return;
                }

            //  If no one else is using the channel send CloseChannel to card
            if ( !stillOpen )
                {
                if ( iAccessCtrl->ReaderIsReady( iSessionID, KChannel0 ) )
                    {
                    //  If possible send the close channel APDU
                    DoCommandAndTransmit( 
                        aMessage,
                        KCloseChannel,
                        channel, 
                        KCommandLengthCloseChannel,
                        KResponseLengthCloseChannel,
                        reinterpret_cast< TInt32>( aMessage.Ptr2() ),
                        KChannel0 );
                    return;
                    }

                //  The reader is busy, queue
                else
                    {
#ifdef _DEBUG
                    RFileLogger::Write( KScardLogDir, KScardLogFileName, 
                        EFileLoggingModeAppend, 
                        _L( "CScardSession::ManageChannel() -- reader busy.\
                        message queued.\n" ) );
#endif
                    iAccessCtrl->QueueExecution( aMessage, iSessionID, 0, 
                        KChannel0 );
                    return;
                    }
                }
            //  Some one else is still on the channel, so we can't physically 
            //  close it from the SC
            else
                {
                aMessage.Complete( KErrNone );
                return;
                }
            //break; //unreachable
            }

        //  Check channel status
        case EChannelStatus:
            {
            //  Get status
            TUint16 status = iAccessCtrl->ChannelStatus();
            TBuf8<2> buf(2);
            // Split the 16 bit integer to two 8-bit bytes
            buf[0] = static_cast< TInt8 >( status & 0x00ff );
            buf[1] = static_cast< TInt8 >( ( status & 0xff00 ) >> 8 );

            //  Inform client
            WriteToClient( aMessage, buf, 1 );
            aMessage.Complete( KErrNone );
            break;
            }
        //  Something's gotten warped
        default:
            aMessage.Complete( KScErrBadArgument );
            break;
        }
    }

// -----------------------------------------------------------------------------
// CScardSession::DoCommandAndTransmit
// Create a command APDU for ManageChannel operations, and send it
// -----------------------------------------------------------------------------
//
void CScardSession::DoCommandAndTransmit(
    const RMessage2& aMessage, 
    const TUint8 aCommand, 
    const TUint8 aP2,
    const TInt8 aCommandLength,
    const TUint8 aResponseLength, 
    const TInt32 aTimeOut, 
    const TUint8 aChannel )
    {
    _WIMTRACE(_L("WIM|Scard|CScardSession::DoCommandAndTransmit|Begin"));

    TRAPD( memFail, DoCommandAndTransmitL( aMessage,
                                           aCommand,
                                           aP2,
                                           aCommandLength,
                                           aResponseLength,
                                           aTimeOut,
                                           aChannel ) );
    __ASSERT_MEMORY( memFail );
    }

// -----------------------------------------------------------------------------
// CScardSession::DoCommandAndTransmitL
// Create a command APDU for ManageChannel operations, and send it
// -----------------------------------------------------------------------------
//
void CScardSession::DoCommandAndTransmitL(
    const RMessage2& aMessage,
    const TUint8 aCommand,
    const TUint8 aP2,
    const TInt8 aCommandLength,
    const TUint8 aResponseLength,
    const TInt32 aTimeOut,
    const TUint8 aChannel )
    {
    _WIMTRACE(_L("WIM|Scard|CScardSession::DoCommandAndTransmitL|Begin"));
    TRequestStatus& status = iAccessCtrl->InitiateCommunication( iSessionID,
        aMessage, aTimeOut, static_cast< TInt8 >( aChannel ) );
    __ASSERT_MEMORY( status.Int() );

    // allocate and assert the command
    iCommandBuffer = HBufC8::NewL( aCommandLength );
    iCommand = new( ELeave ) TPtr8( iCommandBuffer->Des() );

    // Allocate and assert the response

    // response + 2 status bytes
    iResponseBuffer = HBufC8::NewL( aResponseLength + 2 );
    iResponse = new( ELeave ) TPtr8( iResponseBuffer->Des() );

    //  Make the command APDU
    iCommand->SetLength( 4 );
    (*iCommand)[KCLA] = aChannel;
    (*iCommand)[KINS] = KManageChannel;
    (*iCommand)[KP1] = aCommand;
    (*iCommand)[KP2] = aP2;

    if ( aResponseLength )
        {
        //  one byte length
        iCommand->SetLength( 5 );
        (*iCommand)[KLcHigh] = aResponseLength;
        }

    //  Transmit the APDU to the SC
#ifdef _DEBUG
    RFileLogger::HexDump( KScardLogDir, KScardLogFileName,
        EFileLoggingModeAppend,
        _S( "CScardSession::DoCommandAndTransmit() command APDU:\n" ), 0,
        iCommand->Ptr(), iCommand->Length() );
#endif

    iReader->TransmitToCard( status, *iCommand, *iResponse, aTimeOut );
    _WIMTRACE(_L("WIM|Scard|CScardSession::DoCommandAndTransmitL|End"));
    return;
    }

// -----------------------------------------------------------------------------
// CScardSession::WriteToClient
// Write the provided descriptor to the client thread, 8 bit version.
// -----------------------------------------------------------------------------
//
void CScardSession::WriteToClient(
    const RMessage2& aMessage, 
    const TDesC8& aResponseBuffer, 
    const TInt aPtrIndex )
    {
    _WIMTRACE(_L("WIM|Scard|CScardSession::WriteToClient|Begin"));

    TInt desLen = DesLength( aMessage, aPtrIndex, ETrue ); //Len of descriptor;

    //  Make sure there is always enough space for the response
    __ASSERT_ALWAYS( desLen >= aResponseBuffer.Length(), 
        aMessage.Panic( _L( "Response space too short" ), 
            KScServerPanicBadDescriptor ) );

    //  Write
    TRAPD( desError, aMessage.WriteL( aPtrIndex, aResponseBuffer ) );

    if ( desError )
        {
        aMessage.Panic( _L( "ScardServer" ), KScServerPanicBadDescriptor );
        }
    }

// -----------------------------------------------------------------------------
// CScardSession::WriteToClient
// Write the provided descriptor to the client thread, 16 bit version.
// -----------------------------------------------------------------------------
//
void CScardSession::WriteToClient(
    const RMessage2& aMessage, 
    const TDesC16& aResponseBuffer, 
    const TInt aPtrIndex )
    {
    _WIMTRACE(_L("WIM|Scard|CScardSession::|Begin"));
    TInt desLen = DesLength( aMessage, aPtrIndex, ETrue ); //Len of descriptor
    _WIMTRACE2(_L("WIM|Scard|CScardSession:: %d "), desLen );
    _WIMTRACE2(_L("WIM|Scard|CScardSession:: %d "), aResponseBuffer.Length() );

    //  Make sure there is always enough space for the response
    __ASSERT_ALWAYS( desLen >= aResponseBuffer.Length(), 
        aMessage.Panic( _L( "Response space too short" ), 
            KScServerPanicBadDescriptor ) );

    //  Write
    TRAPD( desError, aMessage.WriteL( aPtrIndex, aResponseBuffer ) );

    if ( desError )
        {
        aMessage.Panic( _L( "ScardServer" ), KScServerPanicBadDescriptor );
        }
    }

// -----------------------------------------------------------------------------
// CScardSession::ReadFromClient
// 8 bit version
// -----------------------------------------------------------------------------
//
TInt CScardSession::ReadFromClient(
    const RMessage2& aMessage, 
    HBufC8*& aBuffer, 
    TPtr8*& aPointer, 
    const TInt aPtrIndex )
    {
    _WIMTRACE(_L("WIM|Scard|CScardSession::ReadFromClient|Begin"));
    TInt desLen = DesLength( aMessage, aPtrIndex, EFalse );

    //  Allocate the buffer and pointer to it
    TRAPD( memFail, aBuffer = HBufC8::NewL( desLen ) );
    __ASSERT_MEMORY( memFail );

    aPointer = new TPtr8( aBuffer->Des() );
    __ASSERT_MEMORY( !aPointer );

    //  Read the descriptor and that's it
    TRAPD( desError, aMessage.ReadL( aPtrIndex, *aPointer ) );
    if ( desError )
        {
        aMessage.Panic( _L( "ScardServer" ), KScServerPanicBadDescriptor );
        return KScErrGeneral;
        }
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CScardSession::AllocateResponse
// Allocate space for the internal response buffer. The lenght of the 
// necessary space is deduced from the message.
// -----------------------------------------------------------------------------
//
TInt CScardSession::AllocateResponse( const RMessage2& aMessage,
                                      const TInt aPtrIndex )
    {
    _WIMTRACE(_L("WIM|Scard|CScardSession::AllocateResponse|Begin"));
    TInt desLen = DesLength( aMessage, aPtrIndex, ETrue ); //Len of descriptor;

    //  If no space is needed, why bother
    if ( desLen <= 0 )
        {
        return -1;
        }

    //  Allocate and assert the response buffer
    TRAPD( memFail, AllocateResponseL( desLen ) );
    __ASSERT_MEMORY( memFail );

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CScardSession::AllocateResponse
// Allocate space for the internal response buffer. The lenght of the
// necessary space is deduced from the message.
// -----------------------------------------------------------------------------
//
void CScardSession::AllocateResponseL( TInt aDesLen )
    {
    _WIMTRACE(_L("WIM|Scard|CScardSession::AllocateResponseL|Begin"));

    //  Allocate the response buffer
    iResponseBuffer = HBufC8::NewL( aDesLen );
    iResponse = new( ELeave ) TPtr8( iResponseBuffer->Des() );
    }

// -----------------------------------------------------------------------------
// CScardSession::DeleteBuffers
// Delete all reserved buffers
// -----------------------------------------------------------------------------
//
void CScardSession::DeleteBuffers()
    {
    _WIMTRACE(_L("WIM|Scard|CScardSession::DeleteBuffers|Begin"));
    delete iCommand; 
    delete iResponse;
    iCommand = NULL; 
    iResponse = NULL;
    delete iCommandBuffer; 
    delete iResponseBuffer;
    iCommandBuffer = NULL; 
    iResponseBuffer = NULL;
    _WIMTRACE(_L("WIM|Scard|CScardSession::DeleteBuffers|End"));
    }

// -----------------------------------------------------------------------------
// CScardSession::ConnectionDone
// Connection has established (maybe with error)
// -----------------------------------------------------------------------------
//
void CScardSession::ConnectionDone(
    const TReaderID& aReaderID, 
    const RMessage2& aMessage, 
    const TInt aErrorCode )
    {
    _WIMTRACE(_L("WIM|Scard|CScardSession::ConnectionDone|Begin"));
    //  Connection successfully completed, attach to the reader and write the
    //  reader's name to the client thread
    if ( aErrorCode == KErrNone )
        {
        iAccessCtrl = iServer->FindAccessControl( aReaderID );
        __ASSERT_MEMORY( !iAccessCtrl );
        TRAPD( readerError, iReader = iAccessCtrl->AttachSessionToReaderL(
            this, iSessionID ) );
        if ( readerError == KErrNone )
            {
            iConnectedToReader = ETrue;
            TScardReaderName name( iServer->FriendlyName( aReaderID ) );
            WriteToClient( aMessage, name, 1 );
            }
        else 
            {
#ifdef _DEBUG
            RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
                EFileLoggingModeAppend, 
                _L( "CScardSession::ConnectionDone() -- Error %d when attaching\
                session to reader: \n" ), readerError );
#endif
            iConnectedToReader = EFalse;
            }
        }
    //  No connection could be established
    else
        {
#ifdef _DEBUG
        RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
            EFileLoggingModeAppend, 
            _L( "CScardSession::ConnectionDone() -- ERROR: %d.\n" ), 
            aErrorCode );
#endif
        iConnectedToReader = EFalse;
        }
    }

// -----------------------------------------------------------------------------
// CScardSession::DesLength
// Return length of descriptor in client message
// -----------------------------------------------------------------------------
//
TInt CScardSession::DesLength(
    const RMessage2& aMessage,
    const TInt aPtrIndex,
    const TBool aMaxLength )
    {
    _WIMTRACE(_L("WIM|Scard|CScardSession::DesLength|Begin"));
    TInt desLen( 0 );

    if ( aMaxLength ) //Get max length
        {
        desLen = aMessage.GetDesMaxLength( aPtrIndex );
        }
    else
        {
        desLen = aMessage.GetDesLength( aPtrIndex );
        }

    return desLen;
    }

// -----------------------------------------------------------------------------
// CScardSession::MessagePointer
// Return pointer to wanted block of client message 
// -----------------------------------------------------------------------------
//
TAny* CScardSession::MessagePointer(
    const RMessage2& aMessage,
    const TInt aPtrIndex )
    {
    _WIMTRACE(_L("WIM|Scard|CScardSession::MessagePointer|Begin"));
    TAny* messagePointer = NULL;
    
    //  See from which pointer we are reading and set parameters accordingly
    switch ( aPtrIndex )
        {
        case 0:
            {
            messagePointer = ( TAny* )aMessage.Ptr0();
            break;
            }
        case 1:
            {
            messagePointer = ( TAny* )aMessage.Ptr1();
            break;
            }
        case 2:
            {
            messagePointer = ( TAny* )aMessage.Ptr2();
            break;
            }
        case 3:
            {
            messagePointer = ( TAny* )aMessage.Ptr3();
            break;
            }
        default:
            {
            User::Panic( _L( "Bad index" ), KScServerPanicInternalError );
            }
        }
    return messagePointer;
    }

// -----------------------------------------------------------------------------
// CScardSession::CardEvent
// This is here only for derivative class CScardConnector
// It implements and needs this function
// -----------------------------------------------------------------------------
//
void CScardSession::CardEvent(
    const TScardServiceStatus, 
    const TScardATR&, 
    const TReaderID& )
    {
    _WIMTRACE(_L("WIM|Scard|CScardSession::CardEvent|Begin"));
    }

//  End of File  
