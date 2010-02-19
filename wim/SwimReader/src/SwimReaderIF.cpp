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
* Description:  Implementation of SIM/WIM reader interface class
*
*/


// INCLUDE FILES  
#include    "SwimReaderIF.h"
#include    "SwimReader.h"

#ifdef _TIMER_TRIGGERED_EVENT   // Timer triggered card event simulation enabled
#include    "SwimSysAgentObserver_TimerTriggered.h"
#else
#include    "SwimSysAgentObserver.h"
#endif

#include    "WimTrace.h" // for trace logging

#ifdef _DEBUG
#include <flogger.h>
#endif


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CSwimReaderIF::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSwimReaderIF* CSwimReaderIF::NewL(
    MScardNotifyObserver* aNotifyObserver, 
    TReaderID aReaderID, 
    CSwimReaderLauncher* aLauncher )
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReaderIF::NewL|Begin"));
    CSwimReaderIF* self = new( ELeave ) CSwimReaderIF( aNotifyObserver, 
                                                       aReaderID, aLauncher );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    return self;
    }


// Destructor
CSwimReaderIF::~CSwimReaderIF() 
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReaderIF::~CSwimReaderIF|Begin"));
    delete iFunctionalLevel;
    if ( iSysAgentObserver )
        {
        iSysAgentObserver->Cancel();
        }
    delete iSysAgentObserver;
    Cancel();
    }

// -----------------------------------------------------------------------------
// CSwimReaderIF::CSwimReaderIF
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CSwimReaderIF::CSwimReaderIF(
    MScardNotifyObserver* aNotifyObserver, 
    TReaderID aReaderID, 
    CSwimReaderLauncher* aLauncher )
    : CActive( EPriorityNormal )
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReaderIF::CSwimReaderIF|Begin"));
    CActiveScheduler::Add( this );
    iLauncher = aLauncher;
    iNotifyObserver = aNotifyObserver;
    iID = aReaderID; 
    }

// -----------------------------------------------------------------------------
// CSwimReaderIF::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CSwimReaderIF::ConstructL()
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReaderIF::ConstructL|Begin"));
    iFunctionalLevel = CSwimReader::NewL( this );
    iSysAgentObserver = CSwimSysAgentObserver::NewL( this );
    iSysAgentObserver->Start(); //Start Listening
    }

// -----------------------------------------------------------------------------
// CSwimReaderIF::CancelTransmit
// Cancel transmit
// -----------------------------------------------------------------------------
//
void CSwimReaderIF::CancelTransmit()
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReaderIF::CancelTransmit|Begin"));
    Cancel();
    }

// -----------------------------------------------------------------------------
// CSwimReaderIF::CancelTransmit
// Close ETEL server connection.
// -----------------------------------------------------------------------------
//
TInt CSwimReaderIF::Close()
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReaderIF::Close|Begin"));
    Cancel();
    iFunctionalLevel->Close();
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CSwimReaderIF::DoCancel
// Asyncronous call cancelled
// -----------------------------------------------------------------------------
//
void CSwimReaderIF::DoCancel()
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReaderIF::DoCancel|Begin"));
    iFunctionalLevel->Cancel();
    User::RequestComplete( iClientStatus, KErrCancel );
    }

// -----------------------------------------------------------------------------
// CSwimReaderIF::GetATR
// Not supported in Series 60
// -----------------------------------------------------------------------------
//
TInt CSwimReaderIF::GetATR( TScardATR& /*anATR*/ )
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReaderIF::GetATR|Begin"));
    return KScErrNotSupported;
    }

// -----------------------------------------------------------------------------
// CSwimReaderIF::GetCapabilities
// Get card status or precense bits according to aTag parameter
// -----------------------------------------------------------------------------
//
TBool CSwimReaderIF::GetCapabilities(
    TRequestStatus& aStatus,
    const TInt32   aTag,
    TPtr8&         aValue,
    const TInt32 /*aTimeout*/ )
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReaderIF::GetCapabilities|Begin"));
    iClientStatus = &aStatus;
    aStatus = KRequestPending;
    
    switch ( aTag )  
        {
        case KCardPrecence:
            {
            TUint8 tempstatus;
            iFunctionalLevel->PreferredReaderStatus( tempstatus );
            tempstatus = tempstatus && 0x02;
            aValue.Copy( &tempstatus, 1 );
            aValue.SetLength( 1 );
            User::RequestComplete( iClientStatus, KErrNone );
            break;
            }
        case KCardStatus:
            {
            TUint8 temp;
            iFunctionalLevel->PreferredReaderStatus( temp );
#ifdef _DEBUG        
            RFileLogger::WriteFormat( KSwimReaderLogDir, 
                KSwimReaderLogFileName, EFileLoggingModeAppend, 
                _L( "CSwimReaderIF::GetCapabilities: %d" ), temp );
#endif
            aValue.Copy( &temp, 1 );
            aValue.SetLength( 1 );
            User::RequestComplete( iClientStatus, KErrNone );
            break;
            }
        default:
            {
#ifdef _DEBUG        
            RFileLogger::WriteFormat( KSwimReaderLogDir, 
                KSwimReaderLogFileName, EFileLoggingModeAppend, 
                _L( "CSwimReaderIF::GetCapabilities: unsupported tag 0x%x" ), 
                aTag );
#endif
            User::RequestComplete( iClientStatus, KErrNotSupported );
            }
        }
    return ETrue;   
    }

// -----------------------------------------------------------------------------
// CSwimReaderIF::Notify
// Notify notifyObserver about card event
// -----------------------------------------------------------------------------
//
void CSwimReaderIF::Notify( TScardServiceStatus aStatus )
    {
    _WIMTRACE2(_L("WIM|SwimReader|CSwimReaderIF::Notify|Begin aStatus = %d"), aStatus);
    iNotifyObserver->NotifyCardEvent( aStatus, iID );
    }

// -----------------------------------------------------------------------------
// CSwimReaderIF::OpenAsync
// Uses CSwimReader::WakeUpL to initialize a connection to the ETEL server.
// -----------------------------------------------------------------------------
//
void CSwimReaderIF::OpenAsync( TRequestStatus& aStatus )
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReaderIF::OpenAsync|Begin"));
    iClientStatus = &aStatus;
    aStatus = KRequestPending;
    
#ifdef _DEBUG        
    RFileLogger::WriteFormat( KSwimReaderLogDir, KSwimReaderLogFileName, 
        EFileLoggingModeAppend, 
        _L( "CSwimReaderIF::Open: Opening Reader..." ) );
#endif

    TRAPD( err, iFunctionalLevel->WakeUpL( iStatus, iHistoricals ) );
    if ( err )
        {
#ifdef _DEBUG        
        RFileLogger::WriteFormat( KSwimReaderLogDir, KSwimReaderLogFileName, 
            EFileLoggingModeAppend, 
            _L( "CSwimReaderIF::Open: WakeUpL leaves with %d !!!" ), err );
#endif
        User::RequestComplete( iClientStatus, err );
        }
    else
        {
        iIFPhase = EOpen;
        SetActive();
        }

    }

// -----------------------------------------------------------------------------
// CSwimReaderIF::TransmitToCard
// Transmits given CommandAPDU message to card and
// gets response to ResponseAPDU message.
// -----------------------------------------------------------------------------
//
void CSwimReaderIF::TransmitToCard(
    TRequestStatus& aStatus,
    const TPtrC8&   aCommandAPDU,
    TPtr8&          aResponseAPDU,
    const TInt32 /*aTimeout*/ )
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReaderIF::TransmitToCard|Begin"));
    iClientStatus = &aStatus;
    aStatus = KRequestPending;

#ifdef _DEBUG        
        RFileLogger::WriteFormat( KSwimReaderLogDir, KSwimReaderLogFileName, 
            EFileLoggingModeAppend, 
            _L( "CSwimReaderIF::TransmitToCard: Sending APDU to card..." ) );
#endif
    TRAPD( err, iFunctionalLevel->APDUReqL( iStatus, aCommandAPDU, 
                                            aResponseAPDU ) );
    if ( err )
        {
#ifdef _DEBUG        
        RFileLogger::WriteFormat( KSwimReaderLogDir, KSwimReaderLogFileName, 
            EFileLoggingModeAppend, 
            _L( "CSwimReaderIF::TransmitToCard: APDUReqL leaves with %d !!!" ), 
            err );
#endif
        User::RequestComplete( iClientStatus, err );
        }
    else
        {
        iIFPhase = ETransmitToCard;
        SetActive();
        }
    }

// PRIVATE FUNCTIONS:

// -----------------------------------------------------------------------------
// CSwimReaderIF::HandleOpen
// Handle open request
// -----------------------------------------------------------------------------
//
void CSwimReaderIF::HandleOpen()
    {
    _WIMTRACE2(_L("WIM|SwimReader|CSwimReaderIF::HandleOpen|Begin iStatus=%d"), iStatus.Int() );
    if ( iStatus.Int() != KErrNone ) // a system-spes. error has occ, map to own
        {
        switch ( iStatus.Int() )
            {
            //  The reader or smart card did not respond in time
            case KErrTimedOut:
                iErr = KScReaderErrResponseTimeout;
                break;
            //  Access denied occurs if the comm port could not be opened, i.e.
            //  someone else is using it.
            case KErrAccessDenied:
                iErr = KScReaderErrCommunicationFailure;
                break;
            //  (Some of the) memory allocations failed
            case KErrNoMemory:
                iErr = KScErrNoMemory;
                break;
            //  Something else happened...
            default:
                iErr = KScErrGeneral;
                break;
            }
        }
    else
        {
        iErr = KErrNone;
        }
    User::RequestComplete( iClientStatus, iStatus.Int() );
    }

// -----------------------------------------------------------------------------
// CSwimReaderIF::RunL
// Handle asyncronous response
// -----------------------------------------------------------------------------
//
void CSwimReaderIF::RunL()
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReaderIF::RunL|Begin"));
    switch ( iIFPhase )
        {
        case EOpen:
            {
#ifdef _DEBUG        
            RFileLogger::WriteFormat( KSwimReaderLogDir, KSwimReaderLogFileName, 
                EFileLoggingModeAppend, 
                _L( "CSwimReaderIF::RunL: Open completed with %d" ), 
                iStatus.Int() );
#endif
            HandleOpen();
            if ( iErr == KErrNone )
                {
                Notify( EScardInserted );
                iFunctionalLevel->SetCardInserted( ETrue );
                }
            else 
                {
                Notify( EScardRemoved );
                iFunctionalLevel->SetCardInserted( EFalse );
                }
                
            break;
            }
        case ETransmitToCard:
            {
#ifdef _DEBUG        
            RFileLogger::WriteFormat( KSwimReaderLogDir, KSwimReaderLogFileName, 
                EFileLoggingModeAppend, 
                _L( "CSwimReaderIF::RunL: TransmitToCard completed with %d" ), 
                iStatus.Int() );
#endif
            User::RequestComplete( iClientStatus, iStatus.Int() );
            break;
            }

        default:
            {
#ifdef _DEBUG        
            RFileLogger::WriteFormat( KSwimReaderLogDir, KSwimReaderLogFileName, 
                EFileLoggingModeAppend, 
                _L( "CSwimReaderIF::RunL: Invalid phase %d!" ), 
                iIFPhase );
#endif
            User::RequestComplete( iClientStatus, iStatus.Int() );
            break;
            }
        }   
    _WIMTRACE3(_L("WIM|SwimReader|CSwimReaderIF::RunL|End iIFPhase=%d iErr=%d"), iIFPhase, iErr);
    }

// End of File
