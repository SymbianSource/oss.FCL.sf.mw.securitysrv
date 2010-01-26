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
* Description:  Implements the functional and communication levels of module
*
*/


// INCLUDE FILES 
#include    "SwimReader.h"
#include    "SwimReaderIF.h"
#include    "rmmcustomapi.h"    // TApdu
#include    "WimTrace.h"        // For trace logging
#include    <mmtsy_names.h>     // TSY and Phone name

#ifdef _DEBUG
#include    <flogger.h>
#endif

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CSwimReader::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSwimReader* CSwimReader::NewL( CSwimReaderIF* aInterface )
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReader::NewL|Begin"));
    CSwimReader* self = new( ELeave ) CSwimReader( aInterface );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    return self;
    }

// -----------------------------------------------------------------------------
// CSwimReader::CSwimReader
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CSwimReader::CSwimReader( CSwimReaderIF* aInterface )
    : CActive( EPriorityNormal )
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReader::CSwimReader|Begin"));
    CActiveScheduler::Add( this );
    iInterface = aInterface;
    iReaderAttached = ETrue;
    
    // Currently (and probably always) APDU interface and forecoming
    // APDU server will support only one reader, which number is 0.
    iPreferredReader = 0;
    }

// -----------------------------------------------------------------------------
// CSwimReader::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CSwimReader::ConstructL()
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReader::ConstructL|Begin"));
#ifdef _DEBUG        
    RFileLogger::WriteFormat( KSwimReaderLogDir, KSwimReaderLogFileName,     
        EFileLoggingModeAppend, 
        _L( "CSwimReader::ConstructL: Creating new RTelServer object...\n" ) );
#endif
    iEtelServer = new( ELeave ) RTelServer();    
    }

// -----------------------------------------------------------------------------
// CSwimReader:: ~CSwimReader
// Destructor
// -----------------------------------------------------------------------------
//
CSwimReader::~CSwimReader()
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReader::~CSwimReader|Begin"));
    Close();
    CleanUp();
    delete iMsg;
    delete iEtelServer;
    delete iTempBuf;
    }

// -----------------------------------------------------------------------------
// CSwimReader::APDUReqL
// Send APDU command aCommandAPDU to the ETEL server.
// Response is set to aResponseAPDU.
// -----------------------------------------------------------------------------
//
void CSwimReader::APDUReqL(
    TRequestStatus& aStatus,
    const TPtrC8&   aCommandAPDU,
    TPtr8&          aResponseAPDU,    
    TUint8          aServiceType,
    TUint8          aCardReader,
    TUint8          aAppType,
    TUint8          aPaddingByte )
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReader::APDUReqL1|Begin"));
    iClientStatus = &aStatus;
    aStatus = KRequestPending;
    APDUReqL( aCommandAPDU, aResponseAPDU, aServiceType, aCardReader, 
              aAppType, aPaddingByte );
    iPhase = EAPDURequest;
    SetActive();
    }

// -----------------------------------------------------------------------------
// CSwimReader::Close
// Close ETEL connection and connection to CustomAPI.
// -----------------------------------------------------------------------------
//
void CSwimReader::Close()
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReader::Close|Begin"));
    Cancel();
    if ( iOpen )
        {
        iApdu.Close();
        iEtelServer->Close();
        iOpen = EFalse;
        }
    }
    
// -----------------------------------------------------------------------------
// CSwimReader::DoCancel
// Asynchronous request cancelled
// -----------------------------------------------------------------------------
//
void CSwimReader::DoCancel()
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReader::DoCancel|Begin"));
    
    if( iPhase == EAPDURequest )
        {
    	iApdu.CancelAPDUReq();
        }
        
#ifdef _DEBUG            
    RFileLogger::WriteFormat( KSwimReaderLogDir, KSwimReaderLogFileName,         
        EFileLoggingModeAppend, 
        _L( "CSwimReader::DoCancel: Clean up.\n" ) );
#endif
    CleanUp();
#ifdef _DEBUG            
    RFileLogger::WriteFormat( KSwimReaderLogDir, KSwimReaderLogFileName,         
        EFileLoggingModeAppend, 
        _L( "CSwimReader::DoCancel: Complete request with status %d.\n" ),
            iStatus.Int() );
#endif
    User::RequestComplete( iClientStatus, iStatus.Int() );

    }

// -----------------------------------------------------------------------------
// CSwimReader::PreferredReaderStatus
// Get preferred reader statuses from the iPreferredReaderStatus member.
// -----------------------------------------------------------------------------
//
void CSwimReader::PreferredReaderStatus( TUint8& aReaderStatus )
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReader::PreferredReaderStatus|Begin"));
    aReaderStatus = iPreferredReaderStatus;
    }

// -----------------------------------------------------------------------------
// CSwimReader::SelectCardReader
// From given status bytes, select a card reader, which
// is present and card is also present in the reader.
// -----------------------------------------------------------------------------
//
TUint8 CSwimReader::SelectCardReader(
    TDesC8&  aReaderStatuses,
    TUint8& aPreferredReaderStatus,
    TUint8  aOldPreferredReader /* = NO_PREFERRED_READER */ )
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReader::SelectCardReader|Begin"));
    TInt i;
    TInt size;
    TUint8 readerStatus;

    size = aReaderStatuses.Length();

    for ( i = 0; i < size; i++ )
        {
        readerStatus = aReaderStatuses[i];

        _WIMTRACE2(_L("WIM|SwimReader|CSwimReader::SelectCardReader|ReaderStatus=%d"), 
            (TUint8)aReaderStatuses[i]);

        // First check that this isn't the reader the caller has
        // already tried to use and find inappropriate for some reason.
        if ( i != aOldPreferredReader )
            {
            // If reader and card are present, this reader is good for us.
            if ( (readerStatus & KCardReaderPresent) &&
                (readerStatus & KCardPresent) )
                {
                aPreferredReaderStatus = readerStatus;
                _WIMTRACE2(_L("WIM|SwimReader|CSwimReader::SelectCardReader|PreferredReaderStatus1=%d"),
                    readerStatus);

                // Reader number is stored in first three bits,
                // so return only them.
                return ( static_cast< TUint8 >( readerStatus & KIdentityMask ) );
                }
            }
        }

    // If not found, use a one where reader is present.
    for ( i = 0; i < size; i++ )
        {
        readerStatus = aReaderStatuses[i];

        // First check that this isn't the reader the caller has
        // already tried to use and find inappropriate for some reason.
        if ( i != aOldPreferredReader )
            {
            // If reader is present, this reader is good for us.
            if ( readerStatus & KCardReaderPresent )
                {
                _WIMTRACE2(_L("WIM|SwimReader|CSwimReader::SelectCardReader|PreferredReaderStatus2=%d"),
                    readerStatus);
                aPreferredReaderStatus = readerStatus;

                // Reader number is stored in first three bits,
                // so return only them.
                return ( static_cast< TUint8 >( readerStatus & KIdentityMask ) );
                }
            }
        }

    // If nothing found, return a one caller gave to us.
    return aOldPreferredReader;
    }

// -----------------------------------------------------------------------------
// CSwimReader::SetCardInserted
// Set card inserted
// -----------------------------------------------------------------------------
//
void CSwimReader::SetCardInserted( TBool aIsInserted )
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReader::SetCardInserted|Begin"));
    iCardInserted = aIsInserted;
    }

// -----------------------------------------------------------------------------
// CSwimReader::WakeUpL
// Initializes the reader. Also make APDU_LIST to get
// all statuses of card readers attached to the device.
// -----------------------------------------------------------------------------
//
void CSwimReader::WakeUpL( TRequestStatus& aStatus, TDes8& aHistoricals )
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReader::WakeUpL|Begin"));
    iHistoricals = aHistoricals;
    iClientStatus = &aStatus;
    aStatus = KRequestPending;

    // Set reader not open
    iOpen = EFalse;

    _WIMTRACE2(_L("WIM|SwimReader|CSwimReader::WakeUpL|iPreferredReader=%d"),
        iPreferredReader);

    // If WakeUp failed last time, preferred reader might be initialized
    // wrong, use reader 0
    if ( iPreferredReader == KNoPreferredReader )
        {
        iPreferredReader = 0;
        _WIMTRACE(_L("WIM|SwimReader|CSwimReader::WakeUpL|iPreferredReader changed to 0"));
        }
    
    TInt ret = 0;

    // Connect to the Etel server.
    ret = iEtelServer->Connect();
    if ( ret != KErrNone )
        {
#ifdef _DEBUG        
        RFileLogger::WriteFormat( KSwimReaderLogDir, KSwimReaderLogFileName,
            EFileLoggingModeAppend, 
            _L( "CSwimReader::WakeUpL: connection to etel server failed: %d" ),
            ret );
#endif
        User::Leave( ret );
        }

    // Load phone module.
    ret = iEtelServer->LoadPhoneModule( KMmTsyModuleName );
    
    if ( ret != KErrNone )
        {
#ifdef _DEBUG        
        RFileLogger::WriteFormat( KSwimReaderLogDir, KSwimReaderLogFileName,
            EFileLoggingModeAppend, 
            _L( "CSwimReader::WakeUpL: loading phone module failed: %d" ), 
            ret );
#endif
        User::Leave( ret );
        }

    // Open CustomAPI connection
    ret = iApdu.Open( *iEtelServer, KMmTsyPhoneName );

    if ( ret != KErrNone )
        {
#ifdef _DEBUG        
        RFileLogger::WriteFormat( KSwimReaderLogDir, KSwimReaderLogFileName,
            EFileLoggingModeAppend, 
            _L( "CSwimReader::WakeUpL: phone open failed: %d" ), 
            ret );
#endif
        iEtelServer->Close();
        User::Leave( ret );
        }

    // Make LIST to get status of only available reader.
    if ( iReaderStatuses )
        {
        delete iReaderStatuses;
        iReaderStatuses = NULL;
        }
    if ( iReaderStatusesPtr )
        {
        delete iReaderStatusesPtr;
        iReaderStatusesPtr = NULL;
        }
    iReaderStatuses = HBufC8::NewL( KMaxReaderAmount );
    iReaderStatusesPtr = new ( ELeave ) TPtr8( iReaderStatuses->Des() );

#ifdef _DEBUG        
        RFileLogger::WriteFormat( KSwimReaderLogDir, KSwimReaderLogFileName,
            EFileLoggingModeAppend, 
            _L( "CSwimReader::WakeUpL: Getting list..." ) );
#endif
    _WIMTRACE2(_L("WIM|SwimReader|CSwimReader::WakeUpL|iPreferredReader=%d"),
        iPreferredReader);

    TRAPD( err, APDUReqL( iCmdBytes, *iReaderStatusesPtr, KList,
                          iPreferredReader ) );

    if ( err )
        {
#ifdef _DEBUG        
        RFileLogger::WriteFormat( KSwimReaderLogDir, KSwimReaderLogFileName,
            EFileLoggingModeAppend, 
            _L( "CSwimReader::WakeUpL: get list failed: %d" ), err );
#endif
        iInterface->Notify( EReaderRemoved );
        iApdu.Close();
        iEtelServer->Close();
        User::RequestComplete( iClientStatus, err );
        }
    else
        {
        _WIMTRACE2(_L("WIM|SwimReader|CSwimReader::WakeUpL|*iReaderStatusesPtr=%d"),
            *iReaderStatusesPtr);
        iPhase = EWakeUpAPDURequest;
        SetActive();
        }
    }

// -----------------------------------------------------------------------------
// CSwimReader::APDUReqL
// Send APDU command aCommandAPDU to the ETEL server.
// Response is set to aResponseAPDU.
// -----------------------------------------------------------------------------
//
void CSwimReader::APDUReqL(
    const TPtrC8& aCommandAPDU,
    TPtr8&        aResponseAPDU,    
    TUint8        aServiceType,
    TUint8        aCardReader,
    TUint8        aAppType,
    TUint8        aPaddingByte )
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReader::CSwimReaderLauncher|Begin"));
    // If reader not given, use preferred reader,
    if ( aCardReader == KNoPreferredReader )
        {
        // If preferred reader is not set, quess 0.
        if ( iPreferredReader != KNoPreferredReader )
            {
            aCardReader = iPreferredReader;
            }
        else
            {
            aCardReader = 0;
            }
        }

    iHeader.SetHeader( aServiceType, aCardReader, aAppType, aPaddingByte );

    // ...and call next function.
    APDUReqL( aCommandAPDU, aResponseAPDU, iHeader );
    }

// -----------------------------------------------------------------------------
// CSwimReader::APDUReqL
// Send APDU command aCommandAPDU to the ETEL server.
// Response is set to aResponseAPDU.
// -----------------------------------------------------------------------------
//
void CSwimReader::APDUReqL(
    const TPtrC8&       aCommandAPDU,
    TPtr8&              aResponseAPDU,
    TSwimApduReqHeader& aReqHeader )
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReader::APDUReqL|Begin"));
    iReqHeader = &aReqHeader;
    iResponseAPDU = &aResponseAPDU; 
    iTempBuf = 0;
    
    if ( iMsg )
    {
    delete iMsg;
    iMsg = NULL;	
    }
    
    iMsg = new( ELeave ) RMmCustomAPI::TApdu();   
 
    // Set APDU header to APDU message.
    iMsg->iInfo = aReqHeader.Data();

    // Copy command APDU to response APDU and use it to both
    // transfer request to lower levels and to receive response.
    if ( &aCommandAPDU )
        {
        iMaxLen = aResponseAPDU.MaxLength();

        if ( iMaxLen >= aCommandAPDU.Length() )
            {
            aResponseAPDU = aCommandAPDU;

            iMsg->iData = &aResponseAPDU;
            }
        else
            {
            // Response APDU isn't long enough for Command APDU,
            // which makes a situation a little more complicated.
            // Create a new buffer with enough space.
            if( iTempBuf )
            {
            delete iTempBuf;
            iTempBuf = NULL;
            }
            if( iTempPtr )
            {
            delete iTempPtr;
            iTempPtr = NULL;
            }
            
	    iTempBuf = HBufC8::NewL( aCommandAPDU.Length() );
            iTempPtr = new( ELeave )TPtr8( iTempBuf->Des() );
	    iTempPtr->Copy( aCommandAPDU );
	    iMsg->iData = iTempPtr;
            }
        }
    iApdu.APDUReq( iStatus, *iMsg );
    }

// -----------------------------------------------------------------------------
// CSwimReader::CleanUp
// Clean up allocated buffers
// -----------------------------------------------------------------------------
//
void CSwimReader::CleanUp()
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReader::CleanUp|Begin"));
    delete iReaderStatusesPtr;
    iReaderStatusesPtr = NULL;
    delete iReaderStatuses;
    iReaderStatuses = NULL;
    delete iTempBuf;
    iTempBuf = NULL;
    delete iMsg;
    iMsg = NULL;
    delete iTempPtr;
    iTempPtr = NULL;
    }

// -----------------------------------------------------------------------------
// CSwimReader::RunL
// Handle asyncronous APDU requests
// -----------------------------------------------------------------------------
//
void CSwimReader::RunL()
    {
    _WIMTRACE3(_L("WIM|SwimReader|CSwimReader::RunL|iPhase=%d, iStatus=%d"),
        iPhase, iStatus.Int() );

    if ( iStatus.Int() == 0 )
        {
        switch ( iPhase )
            {
            case EWakeUpAPDURequest:
                {
                TInt error = HandleOpenAPDUReq();

                iOpen = ETrue;
#ifdef _DEBUG        
                RFileLogger::WriteFormat( KSwimReaderLogDir, 
                    KSwimReaderLogFileName, EFileLoggingModeAppend, 
                    _L( "CSwimReader::RunL: WakeUp completed: %d" ), 
                    error );
#endif

                User::RequestComplete( iClientStatus, error );

                break;
                }
            case EAPDURequest:
                {
                TInt error = HandleAPDUReq();

#ifdef _DEBUG        
                RFileLogger::WriteFormat( KSwimReaderLogDir, 
                    KSwimReaderLogFileName, EFileLoggingModeAppend, 
                    _L( "CSwimReader::RunL: APDU request completed: %d" ), 
                    error );
#endif
                User::RequestComplete( iClientStatus, error );
                break;
                }

            }
        }
    else
        {
#ifdef _DEBUG                
        RFileLogger::WriteFormat( KSwimReaderLogDir, KSwimReaderLogFileName,
            EFileLoggingModeAppend, 
            _L( "CSwimReader::RunL: Error, iPhase: %d iStatus: %d" ),
            iPhase, iStatus.Int() );
#endif
        iApdu.Close();
        iEtelServer->Close();
        CleanUp();
        User::RequestComplete( iClientStatus, iStatus.Int() );
        }
    }

// -----------------------------------------------------------------------------
// CSwimReader::HandleAPDUReq
// Handle APDU responses
// -----------------------------------------------------------------------------
//
TInt CSwimReader::HandleAPDUReq()
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReader::HandleAPDUReq|Begin"));
    TInt retValue;

    delete iMsg;
    iMsg = NULL;

    // If we had to use temp buffer, copy response from
    // temp buffer to response buffer given by a caller.
    if ( iTempBuf != 0 )
        {
        iMaxLen = iResponseAPDU->MaxLength();

        if ( iMaxLen >= iTempBuf->Length() )
            {
            *iResponseAPDU = iTempBuf->Des();
            }
        else
            {
            *iResponseAPDU = iTempBuf->Left( iMaxLen );
            }

        delete iTempBuf;
        }

    // Create TSwimApduRespHeader from response info.
    TSwimApduRespHeader respHeader( *(iReqHeader->Data()) );

    // Get APDU status from response header...
    iApduStatus = respHeader.Status();

#ifdef _DEBUG        
    RFileLogger::WriteFormat( KSwimReaderLogDir, KSwimReaderLogFileName,
        EFileLoggingModeAppend, 
        _L( "CSwimReader::HandleAPDUReq: response status: %d" ),
        iApduStatus );

    RFileLogger::HexDump( KSwimReaderLogDir, KSwimReaderLogFileName,
        EFileLoggingModeAppend, 
        _S( "CSwimReader:: HandleAPDUReq APDU received:\n" ), 0,
        iResponseAPDU->Ptr(), iResponseAPDU->Length() );
#endif

    // ...and switch it to SCard error codes.
    switch ( iApduStatus )
        {
        case KApduOK:
        case KErrNone: // Accept also KErrNone
            {
            retValue = KErrNone;
            break;
            }
        case KApduTransmissionError:
            {
            retValue = KScReaderErrCommunicationFailure;
            break;
            }
        case KApduCardMute:
            {
            retValue = KScReaderErrResponseTimeout;
            break;
            }
        case KApduCardDisconnected:
            {
            retValue = KScReaderErrNoCard;
            break;
            }
        case KApduError:
            {
            retValue = KScReaderErrCardFailure;
            break;
            }
        case KApduReaderNotValid:
            {
            retValue = KScReaderErrNoReader;
            break;
            }
        case KApduFormatError:
            {
            retValue = KScErrBadArgument;
            break;
            }
        case KApduTypeNotValid:
            {
            retValue = KScErrBadArgument;
            break;
            }
        // DOS SIM server errors
        case KSimServError:
        case KSimServNotready:
        case KSimServNoservice:
            {
            retValue = KScReaderErrReaderFailure;
            break;
            }
        default:
            retValue = KScErrUnknown;
            break;
        }

        // Check actual length of the data received and compare it to
        // the length of the response buffer allocated by the caller.
        if ( respHeader.DataLength() > iResponseAPDU->MaxLength() 
             && !retValue )
            {
            // If there was more data than fitted to the buffer, tell to the 
            // caller that the buffer was insufficient.
            retValue = KScErrInsufficientBuffer;
            }
        _WIMTRACE2(_L("WIM|SwimReader|CSwimReader::HandleAPDUReq|return %d"),
            retValue);
        return retValue;
    }

// -----------------------------------------------------------------------------
// CSwimReader::HandleOpenAPDUReq
// Handle APDU request for GetReaderList request 
// -----------------------------------------------------------------------------
//
TInt CSwimReader::HandleOpenAPDUReq()
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReader::HandleOpenAPDUReq|Begin"));
    TInt error = HandleAPDUReq();

    if ( error == KErrNone )
        {
        // Select one reader, which looks like a most promising
        // candidate to contain something useful.
        iPreferredReader = SelectCardReader( *iReaderStatuses, 
                                             iPreferredReaderStatus );
        }
    return error;
    }

// End of File
