/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:
*
*/

//INCLUDE FILES
#include "WimCertMgmt.h"
#include "WimMgmt.h" // To initialize WimServer
#include "JavaUtils.h"
#include "WimTrace.h"


// -----------------------------------------------------------------------------
// CWimJavaUtils::CWimJavaUtils()
// Default constructor
// -----------------------------------------------------------------------------
//
CWimJavaUtils::CWimJavaUtils(): CActive( EPriorityStandard )
    {
    CActiveScheduler::Add( this );
    iAuthObjsInfoList = NULL;
    }

// -----------------------------------------------------------------------------
// CWimJavaUtils* CWimJavaUtils::NewL()
// Symbian 2 phase construction
// -----------------------------------------------------------------------------
//
EXPORT_C CWimJavaUtils* CWimJavaUtils::NewL()
    {
    CWimJavaUtils* self = new( ELeave ) CWimJavaUtils();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CWimJavaUtils::ConstructL()
// Second phase
// -----------------------------------------------------------------------------
//
void CWimJavaUtils::ConstructL()
    {
    _WIMTRACE ( _L( "CWimJavaUtils::ConstructL" ) );    
    }

// -----------------------------------------------------------------------------
// CWimJavaUtils::~CWimJavaUtils()
// Destructor. All allocated memory is released.
// -----------------------------------------------------------------------------
//
EXPORT_C CWimJavaUtils::~CWimJavaUtils()
    {
    _WIMTRACE ( _L( "CWimJavaUtils::~CWimJavaUtils" ) );
    Cancel();
    DeallocMemoryFromJavaProvStruct();
    DeallocMemoryFromPathAndLabel();
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
	if( iAuthObjsInfoList )
		{
		iAuthObjsInfoList->Close();
		delete iAuthObjsInfoList;
		}
		
	if( iAuthIdList )
	    {
		iAuthIdList->Close();
		delete iAuthIdList;
	    }
    }

// -----------------------------------------------------------------------------
// CWimJavaUtils::Initialize()
// Initializes the WIM server cache.
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimJavaUtils::Initialize( TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimJavaUtils::Initialize" ) );
    aStatus = KRequestPending;
    iClientStatus = &aStatus;
    iPhase = EInitialize;
    SignalOwnStatusAndComplete();
    }

// -----------------------------------------------------------------------------
// CWimJavaUtils::CancelInitialize()
// Cancels outgoing Initialize operation.
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimJavaUtils::CancelInitialize()
    {
    _WIMTRACE ( _L( "CWimJavaUtils::CancelInitialize" ) );
    Cancel();
    }

// -----------------------------------------------------------------------------
// CWimJavaUtils::GetACIFSize()
// Fetches the size of provisioning information string. This information can
// be used as a notification, does provisioning information exist.
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimJavaUtils::GetACIFSize( TInt& aSize, 
                                        TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimJavaUtils::GetACIFSize" ) );
    aStatus = KRequestPending;
    iClientStatus = &aStatus;

    if ( !iClientSession || !iConnectionHandle )
        {
        User::RequestComplete( iClientStatus, KErrGeneral );
        }
    else
        {
        iSize = &aSize;
        iPhase = EGetACIFSize;
        SignalOwnStatusAndComplete();
        }
    }


// -----------------------------------------------------------------------------
// CWimJavaUtils::GetACFSize()
// aSize ( OUT )
// aACFPath ( IN )
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimJavaUtils::GetACFSize( TInt& aSize, 
                                        const TDesC8& aACFPath,
                                        TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimJavaUtils::GetACFSize" ) );
    aStatus = KRequestPending;
    iClientStatus = &aStatus;

    if ( !iClientSession || !iConnectionHandle )
        {
        User::RequestComplete( iClientStatus, KErrGeneral );
        }
    else
        {
        iSize = &aSize;
        iACFPath.Copy( aACFPath );
        iPhase = EGetACFSize;
        SignalOwnStatusAndComplete();
        }
    }



// -----------------------------------------------------------------------------
// CWimJavaUtils::CancelGetACIFSize()
// Cancels ongoing GetSize operation
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimJavaUtils::CancelGetACIFSize()
    {
    _WIMTRACE ( _L( "CWimJavaUtils::CancelGetACIFSize" ) );
    Cancel();
    }
    

// -----------------------------------------------------------------------------
// CWimJavaUtils::CancelGetACFSize()
// Cancels ongoing GetSize operation
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimJavaUtils::CancelGetACFSize()
    {
    _WIMTRACE ( _L( "CWimJavaUtils::CancelGetACFSize" ) );
    Cancel();
    }    

// -----------------------------------------------------------------------------
// CWimJavaUtils::RetrieveACIFContent()
// Fetches the whole data of provisioning information string.
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimJavaUtils::RetrieveACIFContent( TDes8& aACIFData, 
                                     TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimJavaUtils::RetrieveACIFContent" ) );    
    aStatus = KRequestPending;
    iClientStatus = &aStatus;

    if ( !iClientSession || !iConnectionHandle )
        {
        User::RequestComplete( iClientStatus, KErrGeneral );
        return;
        }

    if ( aACIFData.MaxLength() )
        {
        iData = &aACIFData;
        iPhase = ERetrieveACIFContent;
        SignalOwnStatusAndComplete();
        }
    else
        {
        User::RequestComplete( iClientStatus, KErrGeneral );
        }
    }


// -----------------------------------------------------------------------------
// CWimJavaUtils::RetrieveACFContent()
// Fetches the whole data of provisioning information string.
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimJavaUtils::RetrieveACFContent( TDes8& aACFData, 
                                                const TDesC8& aACFPath,
                                                TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimJavaUtils::RetrieveACFContent" ) );    
    aStatus = KRequestPending;
    iClientStatus = &aStatus;

    if ( !iClientSession || !iConnectionHandle )
        {
        User::RequestComplete( iClientStatus, KErrGeneral );
        return;
        }

    if ( aACFData.MaxLength() && aACFPath.Length() )
        {
        iData = &aACFData;
        iACFPath.Copy(aACFPath);
        iPhase = ERetrieveACFContent;
        SignalOwnStatusAndComplete();
        }
    else
        {
        User::RequestComplete( iClientStatus, KErrGeneral );
        }
    }
    
    
// -----------------------------------------------------------------------------
// CWimJavaUtils::RetrieveWimLabelAndPath()
// Fetches wim label and path.
// -----------------------------------------------------------------------------
//    
EXPORT_C void CWimJavaUtils::RetrieveWimLabelAndPath( TDes8& aWimLabel,
                                                     TDes8& aPath,
                                                     TRequestStatus& aStatus )
    {
	_WIMTRACE ( _L( "CWimJavaUtils::RetrieveWimLabelAndPath" ) );    
    aStatus = KRequestPending;
    iClientStatus = &aStatus;

    if ( !iClientSession || !iConnectionHandle )
        {
        User::RequestComplete( iClientStatus, KErrGeneral );
        return;
        }

    if( aWimLabel.MaxLength() && aPath.MaxLength() )
	    {
	    iLabel = &aWimLabel;
	    iPath = &aPath;
	    iPhase = ERetrieveWimLabelAndPath;
	    SignalOwnStatusAndComplete();	
	    }
    else
	    {
	    User::RequestComplete( iClientStatus, KErrGeneral );	
	    }
     
    }
                                                     

// -----------------------------------------------------------------------------
// CWimJavaUtils::GetAuthObjsInfo()
// -----------------------------------------------------------------------------
//    
EXPORT_C void CWimJavaUtils::GetAuthObjsInfo( const RArray<TInt>& aAuthIdList, 
                                       RArray<TJavaPINParams>& aAuthObjsInfoList,
                                       TRequestStatus& aStatus )
    {
	_WIMTRACE ( _L( "CWimOMAProv::GetAuthObjsInfo" ) );
	aStatus = KRequestPending;
	iClientStatus = &aStatus;
	
	if ( !iClientSession || !iConnectionHandle )
        {
        User::RequestComplete( iClientStatus, KErrGeneral );
        return;
        }

    if ( aAuthIdList.Count() )
        {
        if( iAuthObjsInfoList )
            {
        	delete iAuthObjsInfoList;
        	iAuthObjsInfoList = NULL;
            }
            
        TRAPD( err, iAuthObjsInfoList = new (ELeave) RArray<TJavaPINParams>( aAuthIdList.Count() ) );    
        
        if ( err != KErrNone )
            {
            User::RequestComplete( iClientStatus, err );	
            return;
        	  }
        
        if( iAuthIdList )
            {
        	delete iAuthIdList;
        	iAuthIdList = NULL;
            }
        
        TRAP( err, iAuthIdList = new (ELeave ) RArray<TInt>( aAuthIdList.Count() ) );
        
        if ( err != KErrNone )
            {
            User::RequestComplete( iClientStatus, err );	
            return;	
        	  }
        
        for( TInt i = 0; i < aAuthIdList.Count() ; i ++ )
            {
        	iAuthIdList->Append( aAuthIdList[i]);
            }
            
        iClientsAuthObjsInfoList = &aAuthObjsInfoList;
		
		TJavaPINParams pinp;
		for( TInt i = 0; i < aAuthIdList.Count(); i++ )
			{
			iAuthObjsInfoList->Append( pinp );
			}
			
        iPhase = EGetAuthObjsInfo;
        SignalOwnStatusAndComplete();
        }
    else
        {
        User::RequestComplete( iClientStatus, KErrGeneral );
        }
    
    
    }
      

// -----------------------------------------------------------------------------
// CWimJavaUtils::CancelACIFRetrieveContent()
// Cancels ongoing retrieve operation
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimJavaUtils::CancelRetrieveACIFContent()
    {
    _WIMTRACE ( _L( "CWimJavaUtils::CancelRetrieveACIFContent" ) );
    Cancel();
    }


// -----------------------------------------------------------------------------
// CWimJavaUtils::CancelACFRetrieveContent()
// Cancels ongoing retrieve operation
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimJavaUtils::CancelRetrieveACFContent()
    {
    _WIMTRACE ( _L( "CWimJavaUtils::CancelRetrieveACFContent" ) );
    Cancel();
    }


// -----------------------------------------------------------------------------
// CWimJavaUtils::CancelACFRetrieveWimLabelAndPath()
// Cancels ongoing retrieve operation
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimJavaUtils::CancelRetrieveWimLabelAndPath()
    {
    _WIMTRACE ( _L( "CWimJavaUtils::CancelACFRetrieveWimLabelAndPath" ) );
    Cancel();
    }

// -----------------------------------------------------------------------------
// CWimJavaUtils::CancelGetAuthObjsInfo()
// -----------------------------------------------------------------------------
//    
EXPORT_C void CWimJavaUtils::CancelGetAuthObjsInfo()
    {
    _WIMTRACE ( _L( "CWimJavaUtils::CancelGetAuthObjsInfo" ) );
	Cancel();
    }
    
// -----------------------------------------------------------------------------
// CWimJavaUtils::RunL()
// Different phases are handled here
// iPhase = EInitialize: Gets ClientSession handle. Tries to connect
//          to existing session. If there is no existing session, a new session 
//          is created.
// iPhase = EConnectClientSession: If new session was needed to be created, 
//          new connecting request is needed.
// iPhase = EInitializeWim: Sends initialize request to server and waits
//          asyncronously.
// iPhase = EInitializeDone: Initialization is ready.
// iPhase = EGetACIFSize: Gets data type and data size from WIM Server
// iPhase = EGetACIFSizeDone: WIM server has sent data to client and client copies
//                        data forward to caller.
// iPhase = ERetrieveACIFContent: Retrieves binary data from WIM Server according to
//                     type
// iPhase = ERetrieveACIFContentDone: WIM Server has sent data and client copies data 
//                         forward to caller.
// -----------------------------------------------------------------------------
//
void CWimJavaUtils::RunL()
    {
    switch ( iPhase ) 
        {
        case EInitialize:
            {    
            _WIMTRACE ( _L( "CWimJavaUtils::RunL case: EInitialize" ) );
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
            _WIMTRACE ( _L( "CWimJavaUtils::RunL case: EConnectClientSession" ) );
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
            _WIMTRACE ( _L( "CWimJavaUtils::RunL case: EInitializeWim" ) );
            //Initialize WIM
            SetActive();
            iClientSession->Initialize( iStatus );
            iPhase = EInitializeDone;
            break;   
            }
        case EInitializeDone:
            {
            _WIMTRACE ( _L( "CWimJavaUtils::RunL case: EInitializeDone" ) );
            if ( iStatus.Int() == KErrNone )
                {
                 //Get connection handle to WIM Server.
                iConnectionHandle = RWimCertMgmt::ClientSessionL(); 
                }
            User::RequestComplete( iClientStatus, iStatus.Int() );
            break;
            }
        case EGetACIFSize:
            {
            _WIMTRACE ( _L( "CWimJavaUtils::RunL case: EGetACIFSize" ) );
            TJavaProv javaProv;
              
            iConnectionHandle->RetrieveACIFDataL( javaProv, 
                                                 iStatus, 
                                                 EGetACIFFileSize );
            iPhase = EGetACIFSizeDone;
            SetActive();
            break;
            }
       
        case EGetACFSize:
            {
            _WIMTRACE ( _L( "CWimJavaUtils::RunL case: EGetACFSize" ) );
            TJavaProv javaProv;
            javaProv.iPath = &iACFPath;
            iConnectionHandle->RetrieveACFDataL( javaProv, 
                                                 iStatus, 
                                                 EGetACFFileSize );
            iPhase = EGetACFSizeDone;
            SetActive();
            break;	
            }
         case EGetAuthObjsInfo:
            {
            _WIMTRACE ( _L( "CWimJavaUtils::RunL case: EGetAuthObjsInfo" ) );
            
            iConnectionHandle->RetrieveAuthObjsInfoL( *iAuthIdList,
                                                *iAuthObjsInfoList,
                                                iStatus, 
                                                ERetrieveAuthObjsInfo );                        
            iPhase  = EGetAuthObjsInfoDone;
            SetActive();
        	break;
        	
            }
        case EGetACIFSizeDone:
            {
            _WIMTRACE ( _L( "CWimJavaUtils::RunL case: EGetACIFSizeDone" ) );
            if ( iStatus.Int() == KErrNone )
                {
                TPckgBuf<TJavaProv> javaProvPckgBuf;
    
                javaProvPckgBuf.operator = 
                    ( *iConnectionHandle->TJavaProvPckgBuf()->PckgBuf() );

                *iSize = javaProvPckgBuf().iSize;
                }
            iConnectionHandle->DeallocJavaDataPckgBuf();
            User::RequestComplete( iClientStatus, iStatus.Int() );
            break;
            }
            
         case EGetACFSizeDone:
            {
            _WIMTRACE ( _L( "CWimJavaUtils::RunL case: EGetACFSizeDone" ) );
            if ( iStatus.Int() == KErrNone )
                {
                TPckgBuf<TJavaProv> javaProvPckgBuf;
    
                javaProvPckgBuf.operator = 
                    ( *iConnectionHandle->TJavaProvPckgBuf()->PckgBuf() );

                *iSize = javaProvPckgBuf().iSize;
                }
            iConnectionHandle->DeallocJavaDataPckgBuf();
            User::RequestComplete( iClientStatus, iStatus.Int() );
            break;
            }   
            
        case ERetrieveACIFContent:
            {
            _WIMTRACE ( _L( "CWimJavaUtils::RunL case: EACIFRetrieve" ) );
            AllocMemoryForJavaProvStructL( iData->MaxLength() );
            TJavaProv javaProv;
            javaProv.iJavaData = iJavaProvBufPtr;
            iConnectionHandle->RetrieveACIFDataL( javaProv, 
                                                 iStatus,
                                                 EGetACIFFile );
            iPhase = ERetrieveACIFContentDone;
            SetActive();
            break;
            }
            
        case ERetrieveACFContent:
            {
            _WIMTRACE ( _L( "CWimJavaUtils::RunL case: EACFRetrieve" ) );
            AllocMemoryForJavaProvStructL( iData->MaxLength() );
            TJavaProv javaProv;
            javaProv.iPath = &iACFPath;
            javaProv.iJavaData = iJavaProvBufPtr;
            
            iConnectionHandle->RetrieveACFDataL( javaProv, 
                                                 iStatus,
                                                 EGetACFFile );
            iPhase = ERetrieveACFContentDone;
            SetActive();
            break;
            }   
            
        case ERetrieveACIFContentDone:
            {
            _WIMTRACE ( _L( "CWimJavaUtils::RunL case: ERetrieveACIFContentDone" ) );
            if ( iStatus.Int() == KErrNone )
                {
                TPtr8 ptr = iJavaProvBuf->Des();
                iData->Copy( ptr );
                }
            DeallocMemoryFromJavaProvStruct();
            iConnectionHandle->DeallocJavaDataPckgBuf();
            User::RequestComplete( iClientStatus, iStatus.Int() );
            break;
            }
            
        case ERetrieveACFContentDone:
            {
            _WIMTRACE ( _L( "CWimJavaUtils::RunL case: ERetrieveACFContentDone" ) );
            if ( iStatus.Int() == KErrNone )
                {
                TPtr8 ptr = iJavaProvBuf->Des();
                iData->Copy( ptr );
                }
            DeallocMemoryFromJavaProvStruct();
            iConnectionHandle->DeallocJavaDataPckgBuf();
            User::RequestComplete( iClientStatus, iStatus.Int() );
            break;
            } 
        case EGetAuthObjsInfoDone:
            {
            TInt count = iAuthObjsInfoList->Count();
            for( TInt i =0; i < count ; i++ )
	            {
	            if((*iAuthObjsInfoList)[i].iMinLength > 0 )
		            {
		            iClientsAuthObjsInfoList->Append( (*iAuthObjsInfoList)[i] );	
		            }
	            }
            User::RequestComplete( iClientStatus, iStatus.Int() );
            break;	
            }
            
        case ERetrieveWimLabelAndPath:
            {
            _WIMTRACE ( _L( "CWimJavaUtils::RunL case: EACFRetrieveWimLabelAndPath" ) );
            AllocMemoryForPathAndLabelL();
            
        	iConnectionHandle->RetrieveLabelAndPathL( *iLabelBufPtr,
        	                                          *iPathBufPtr, 
                                                      iStatus,
                                                      EGetLabelAndPath );
            iPhase = ERetrieveWimLabelAndPathDone;
            SetActive();
            break;
            }
        
        case ERetrieveWimLabelAndPathDone:  
            {
            _WIMTRACE ( _L( "CWimJavaUtils::RunL case: ERetrieveWimLabelAndPathDone" ) );
            if( iStatus.Int() == KErrNone )
	            {
	            iLabel->Copy( iLabelBuf->Des() );
	            iPath->Copy( iPathBuf->Des() );
	            }
            DeallocMemoryFromPathAndLabel();     
          
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
// CWimJavaUtils::AllocMemoryForJavaProvStructL()
// Allocates memory for Java binary data
// -----------------------------------------------------------------------------
void CWimJavaUtils::AllocMemoryForJavaProvStructL( const TInt aDataLength )
    {
    _WIMTRACE ( _L( "CWimJavaUtils::AllocMemoryForJavaProvStructL" ) );
    if ( aDataLength )
        {
        iJavaProvBuf = HBufC8::NewL( aDataLength );
        iJavaProvBufPtr = new( ELeave ) TPtr8( iJavaProvBuf->Des() );
        }
    }

// -----------------------------------------------------------------------------
// CWimJavaUtils::DeallocMemoryFromJavaProvStruct()
// Deallocates memory from member variables
// -----------------------------------------------------------------------------
void CWimJavaUtils::DeallocMemoryFromJavaProvStruct()
    {
    _WIMTRACE ( _L( "CWimJavaUtils::DeallocMemoryFromJavaProvStruct" ) );
    delete iJavaProvBuf;
    delete iJavaProvBufPtr;
    iJavaProvBuf = NULL;
    iJavaProvBufPtr = NULL;
    }

// -----------------------------------------------------------------------------
// CWimJavaUtils::AllocMemoryForPathAndLabelBufferL()
// Allocates memory for path and label buffer
// -----------------------------------------------------------------------------
void CWimJavaUtils::AllocMemoryForPathAndLabelL()
    {
    _WIMTRACE ( _L( "CWimJavaUtils::AllocMemoryForPathAndLabelL" ) );
 
    iLabelBuf = HBufC8::NewL( KLabelMaxLength );
    iLabelBufPtr = new( ELeave ) TPtr8( iLabelBuf->Des() );
    
    iPathBuf = HBufC8::NewL( KPathMaxLength );
    iPathBufPtr = new( ELeave ) TPtr8( iPathBuf->Des() );
    }

// -----------------------------------------------------------------------------
// CWimJavaUtils::DeallocMemoryFromPathAndLabel()
// Deallocates memory from path and label
// -----------------------------------------------------------------------------
void CWimJavaUtils::DeallocMemoryFromPathAndLabel()
    {
    _WIMTRACE ( _L( "CWimJavaUtils::DeallocMemoryFromPathAndLabel" ) );
    delete iLabelBuf;
    delete iPathBuf;
    delete iLabelBufPtr;
    delete iPathBufPtr;
    
    iLabelBuf = NULL;
    iLabelBufPtr = NULL;
    iPathBuf = NULL;
    iPathBufPtr = NULL;
    }


// -----------------------------------------------------------------------------
// CWimJavaUtils::DoCancel()
// Deallocates member variables and completes client status with
// KErrCancel error code.
// -----------------------------------------------------------------------------
//
void CWimJavaUtils::DoCancel()
    {
    _WIMTRACE ( _L( "CWimJavaUtils::DoCancel" ) );
    if ( iConnectionHandle && 
         ( iPhase == EGetACIFSizeDone || iPhase == ERetrieveACIFContentDone ) )
        {
        iConnectionHandle->DeallocJavaDataPckgBuf();
        }
    DeallocMemoryFromJavaProvStruct();
    User::RequestComplete( iClientStatus, KErrCancel );
    }

// -----------------------------------------------------------------------------
// CWimJavaUtils::RunError()
// The active scheduler calls this function if this active object's RunL() 
// function leaves. This gives this active object the opportunity to perform 
// any necessary cleanup.
// After cleanup, complete request with received error code.
// -----------------------------------------------------------------------------
//
TInt CWimJavaUtils::RunError( TInt aError )
    {
    _WIMTRACE ( _L( "CWimJavaUtils::RunError Error = %d" ) );
    if ( iConnectionHandle && 
         ( iPhase == EGetACIFSizeDone || iPhase == ERetrieveACIFContentDone ) )
        {
        iConnectionHandle->DeallocJavaDataPckgBuf();
        }
    DeallocMemoryFromJavaProvStruct();
    User::RequestComplete( iClientStatus, aError );
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CWimJavaUtils::SignalOwnStatusAndComplete()       
// Sets own iStatus to KRequestPending, and signals it 
// with User::RequestComplete() -request. This gives chance 
// active scheduler to run other active objects. After a quick
// visit in actives cheduler, signal returns to RunL() and starts next
// phase of operation. 
// -----------------------------------------------------------------------------
//
void CWimJavaUtils::SignalOwnStatusAndComplete()
    {
    _WIMTRACE ( _L( "CWimJavaUtils::SignalOwnStatusAndComplete" ) );
    iStatus = KRequestPending;
    SetActive();
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, KErrNone );
    }

// End of File



















