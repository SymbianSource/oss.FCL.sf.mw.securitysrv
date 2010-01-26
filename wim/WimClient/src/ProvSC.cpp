/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  API which handles OMA Provisioning related operations
*
*/




//INCLUDE FILES
#include "WimCertMgmt.h"
#include "WimMgmt.h"
#include "ProvSC.h"
#include "WimTrace.h"


// -----------------------------------------------------------------------------
// CWimOMAProv::CWimOMAProv()
// Default constructor
// -----------------------------------------------------------------------------
//
CWimOMAProv::CWimOMAProv(): CActive( EPriorityStandard )
    {
    }

// -----------------------------------------------------------------------------
// CWimOMAProv* CWimOMAProv::NewL()
// Symbian 2 phase construction
// -----------------------------------------------------------------------------
//
EXPORT_C CWimOMAProv* CWimOMAProv::NewL()
    {
    CWimOMAProv* self = new( ELeave ) CWimOMAProv();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CWimOMAProv::ConstructL()
// Second phase
// -----------------------------------------------------------------------------
//
void CWimOMAProv::ConstructL()
    {
    _WIMTRACE ( _L( "CWimOMAProv::ConstructL" ) );
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CWimOMAProv::~CWimOMAProv()
// Destructor. All allocated memory is released.
// -----------------------------------------------------------------------------
//
EXPORT_C CWimOMAProv::~CWimOMAProv()
    {
    _WIMTRACE ( _L( "CWimOMAProv::~CWimOMAProv" ) );
    Cancel();
    DeallocMemoryFromOmaProvStruct();
    
    if( iConnectionHandle )
        {
        iConnectionHandle->Close();
        delete iConnectionHandle;
        }
    
    if ( iClientSession )
        {
        iClientSession->Close();
        delete iClientSession;
        }
    }

// -----------------------------------------------------------------------------
// CWimOMAProv::Initialize()
// Initializes the WIM server cache.
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimOMAProv::Initialize( TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimOMAProv::Initialize" ) );
    aStatus = KRequestPending;
    iClientStatus = &aStatus;
    iPhase = EInitialize;
    SignalOwnStatusAndComplete();
    }

// -----------------------------------------------------------------------------
// CWimOMAProv::CancelInitialize()
// Cancels outgoing Initialize operation.
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimOMAProv::CancelInitialize()
    {
    _WIMTRACE ( _L( "CWimOMAProv::CancelInitialize" ) );
    Cancel();
    }

// -----------------------------------------------------------------------------
// CWimOMAProv::GetSize()
// Fetches the size of provisioning information string. This information can
// be used as a notification, does provisioning information exist.
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimOMAProv::GetSize( TInt& aSize, 
                                    const TOMAType& aOMAType, 
                                    TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimOMAProv::GetSize" ) );
    aStatus = KRequestPending;
    iClientStatus = &aStatus;

    if ( !iClientSession || !iConnectionHandle )
        {
        User::RequestComplete( iClientStatus, KErrGeneral );
        }
    else
        {
        iSize = &aSize;
        iOMAType = aOMAType;
        iPhase = EGetSize;
        SignalOwnStatusAndComplete();
        }
    }

// -----------------------------------------------------------------------------
// CWimOMAProv::CancelGetSize()
// Cancels ongoing GetSize operation
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimOMAProv::CancelGetSize()
    {
    _WIMTRACE ( _L( "CWimOMAProv::CancelGetSize" ) );
    Cancel();
    }

// -----------------------------------------------------------------------------
// CWimOMAProv::Retrieve()
// Fetches the whole data of provisioning information string.
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimOMAProv::Retrieve( const TOMAType& aOMAType, 
                                     TDes8& aOMAData, 
                                     TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimOMAProv::Retrieve" ) );    
    aStatus = KRequestPending;
    iClientStatus = &aStatus;

    if ( !iClientSession || !iConnectionHandle )
        {
        User::RequestComplete( iClientStatus, KErrGeneral );
        return;
        }

    if ( aOMAData.MaxLength() )
        {
        iOMAType = aOMAType;
        iData = &aOMAData;
        iPhase = ERetrieve;
        SignalOwnStatusAndComplete();
        }
    else
        {
        User::RequestComplete( iClientStatus, KErrGeneral );
        }
    }

// -----------------------------------------------------------------------------
// CWimOMAProv::CancelRetrieve()
// Cancels ongoing retrieve operation
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimOMAProv::CancelRetrieve()
    {
    _WIMTRACE ( _L( "CWimOMAProv::CancelRetrieve" ) );
    Cancel();
    }

// -----------------------------------------------------------------------------
// CWimOMAProv::RunL()
// Different phases are handled here
// iPhase = EInitialize: Gets ClientSession handle. Tries to connect
//          to existing session. If there is no existing session, a new session 
//          is created.
// iPhase = EConnectClientSession: If new session was needed to be created, 
//          new connecting request is needed.
// iPhase = EInitializeWim: Sends initialize request to server and waits
//          asyncronously.
// iPhase = EInitializeDone: Initialization is ready.
// iPhase = EGetSize: Gets OMA data type and data size from WIM Server
// iPhase = EGetSizeDone: WIM server has sent data to client and client copies
//                        data forward to caller.
// iPhase = ERetrieve: Retrieves OMA binary data from WIM Server according to
//                     type
// iPhase = ERetrieveDone: WIM Server has sent data and client copies data 
//                         forward to caller.
// -----------------------------------------------------------------------------
//
void CWimOMAProv::RunL()
    {
    switch ( iPhase ) 
        {
        case EInitialize:
            {    
            _WIMTRACE ( _L( "CWimOMAProv::RunL case: EInitialize" ) );
             //get ClientSession handle
            iClientSession = RWimMgmt::ClientSessionL();

            iPhase = EConnectClientSession;
            TInt retval = 0;
            iWimStartErr = iClientSession->Connect();
            if ( iWimStartErr == KErrNotFound ) 
                {
                retval = iClientSession->StartWim();
                if( retval != KErrNone )
                    {
                    User::RequestComplete( iClientStatus, retval ); 
                    }
                else
                    {
                    SignalOwnStatusAndComplete();
                    }
                }
            else
                {
                 SignalOwnStatusAndComplete(); 
                }
            break;
            }
        case EConnectClientSession:
            {
            _WIMTRACE ( _L( "CWimOMAProv::RunL case: EConnectClientSession" ) );
            //Raise iPhase
            iPhase = EInitializeWim; 
            if ( iWimStartErr )
                {
                //New session was needed to be created. Try to reconnect.
                iWimStartErr = iClientSession->Connect();
                if ( iWimStartErr )
                    {
                    //Failed to connect. No reason to continue.
                    User::RequestComplete( iClientStatus, iWimStartErr );
                    }
                else
                    {
                    SignalOwnStatusAndComplete();
                    }
                }
            else
                {
                SignalOwnStatusAndComplete();
                }
            break;
            }
        case EInitializeWim:
            {
            _WIMTRACE ( _L( "CWimOMAProv::RunL case: EInitializeWim" ) );
            //Initialize WIM
            SetActive();
            iClientSession->Initialize( iStatus );
            iPhase = EInitializeDone;
            break;   
            }
        case EInitializeDone:
            {
            _WIMTRACE ( _L( "CWimOMAProv::RunL case: EInitializeDone" ) );
            if ( iStatus.Int() == KErrNone )
                {
                 //Get connection handle to WIM Server.
                iConnectionHandle = RWimCertMgmt::ClientSessionL(); 
                }
            User::RequestComplete( iClientStatus, iStatus.Int() );
            break;
            }
        case EGetSize:
            {
            _WIMTRACE ( _L( "CWimOMAProv::RunL case: EGetSize" ) );
            TOmaProv omaProv;
            omaProv.iOmaType = iOMAType;
            iConnectionHandle->RetrieveOmaDataL( omaProv, 
                                                 iStatus, 
                                                 EGetOMAFileSize );
            iPhase = EGetSizeDone;
            SetActive();
            break;
            }
        case EGetSizeDone:
            {
            _WIMTRACE ( _L( "CWimOMAProv::RunL case: EGetSizeDone" ) );
            if ( iStatus.Int() == KErrNone )
                {
                TPckgBuf<TOmaProv> omaProvPckgBuf;
    
                omaProvPckgBuf.operator = 
                    ( *iConnectionHandle->TOmaProvPckgBuf()->PckgBuf() );

                *iSize = omaProvPckgBuf().iSize;
                }
            iConnectionHandle->DeallocOmaDataPckgBuf();
            User::RequestComplete( iClientStatus, iStatus.Int() );
            break;
            }
        case ERetrieve:
            {
            _WIMTRACE ( _L( "CWimOMAProv::RunL case: ERetrieve" ) );
            AllocMemoryForOmaProvStructL( iData->MaxLength() );
            TOmaProv omaProv;
            omaProv.iOmaData = iOmaProvBufPtr;
            omaProv.iOmaType = iOMAType;
            iConnectionHandle->RetrieveOmaDataL( omaProv, 
                                                 iStatus,
                                                 EGetOMAFile );
            iPhase = ERetrieveDone;
            SetActive();
            break;
            }
        case ERetrieveDone:
            {
            _WIMTRACE ( _L( "CWimOMAProv::RunL case: ERetrieveDone" ) );
            if ( iStatus.Int() == KErrNone )
                {
                TPtr8 ptr = iOmaProvBuf->Des();
                iData->Copy( ptr );
                }
            DeallocMemoryFromOmaProvStruct();
            iConnectionHandle->DeallocOmaDataPckgBuf();
            User::RequestComplete( iClientStatus, iStatus.Int() );
            break;
            }
        default:
            {
            break;
            }
        }
    }

// -----------------------------------------------------------------------------
// CWimOMAProv::AllocMemoryForOmaProvStructL()
// Allocates memory for OMA binary data
// -----------------------------------------------------------------------------
void CWimOMAProv::AllocMemoryForOmaProvStructL( const TInt aDataLength )
    {
    _WIMTRACE ( _L( "CWimOMAProv::AllocMemoryForOmaProvStructL" ) );
    if ( aDataLength )
        {
        iOmaProvBuf = HBufC8::NewL( aDataLength );
        iOmaProvBufPtr = new( ELeave ) TPtr8( iOmaProvBuf->Des() );
        }
    }

// -----------------------------------------------------------------------------
// CWimOMAProv::DeallocMemoryFromOmaProvStruct()
// Deallocates memory from member variables
// -----------------------------------------------------------------------------
void CWimOMAProv::DeallocMemoryFromOmaProvStruct()
    {
    _WIMTRACE ( _L( "CWimOMAProv::DeallocMemoryFromOmaProvStruct" ) );
    delete iOmaProvBuf;
    delete iOmaProvBufPtr;
    iOmaProvBuf = NULL;
    iOmaProvBufPtr = NULL;
    }

// -----------------------------------------------------------------------------
// CWimOMAProv::DoCancel()
// Deallocates member variables and completes client status with
// KErrCancel error code.
// -----------------------------------------------------------------------------
//
void CWimOMAProv::DoCancel()
    {
    _WIMTRACE ( _L( "CWimOMAProv::DoCancel" ) );
    if ( iConnectionHandle && 
         ( iPhase == EGetSizeDone || iPhase == ERetrieveDone ) )
        {
        iConnectionHandle->DeallocOmaDataPckgBuf();
        }
    DeallocMemoryFromOmaProvStruct();
    User::RequestComplete( iClientStatus, KErrCancel );
    }

// -----------------------------------------------------------------------------
// CWimOMAProv::RunError()
// The active scheduler calls this function if this active object's RunL() 
// function leaves. This gives this active object the opportunity to perform 
// any necessary cleanup.
// After cleanup, complete request with received error code.
// -----------------------------------------------------------------------------
//
TInt CWimOMAProv::RunError( TInt aError )
    {
    _WIMTRACE ( _L( "CWimOMAProv::RunError Error = %d" ) );
    if ( iConnectionHandle && 
         ( iPhase == EGetSizeDone || iPhase == ERetrieveDone ) )
        {
        iConnectionHandle->DeallocOmaDataPckgBuf();
        }
    DeallocMemoryFromOmaProvStruct();
    User::RequestComplete( iClientStatus, aError );
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CWimOMAProv::SignalOwnStatusAndComplete()       
// Sets own iStatus to KRequestPending, and signals it 
// with User::RequestComplete() -request. This gives chance 
// active scheduler to run other active objects. After a quick
// visit in actives cheduler, signal returns to RunL() and starts next
// phase of operation. 
// -----------------------------------------------------------------------------
//
void CWimOMAProv::SignalOwnStatusAndComplete()
    {
    _WIMTRACE ( _L( "CWimOMAProv::SignalOwnStatusAndComplete" ) );
    iStatus = KRequestPending;
    SetActive();
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, KErrNone );
    }

// End of File



















