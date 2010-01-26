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
* Description:  API for managing PIN
*
*/


//INCLUDES
#include "WimPin.h"
#include "WimMgmt.h"
#include "WimTrace.h"
#include <mctauthobject.h>





// ================= MEMBER FUNCTIONS  for CWimPin=======================
//
// C++ default constructor. Initialize the member variables
//
//
CWimPin::CWimPin( TWimPin aPin, TPinAddress aPinAddr )
:CActive( EPriorityStandard ), iReference( aPinAddr ), iPinType( aPin )
    {
    }
      
      
// -----------------------------------------------------------------------------
//CWimPin::NewL()
//Two phased construction
// -----------------------------------------------------------------------------
//
EXPORT_C CWimPin* CWimPin::NewL( TWimPin aPin, 
                                 const TPinAddress aPinAddr,
                                 TDesC& aTokenLabel )
    {
    _WIMTRACE ( _L( "CWimPin::NewL()" ) );
    __ASSERT_ALWAYS( aPinAddr, User::Leave( KErrArgument ) );
    CWimPin* self = new( ELeave ) CWimPin( aPin, aPinAddr );
    CleanupStack::PushL( self );
    self->ConstructL( aTokenLabel );
    CleanupStack::Pop( self );
    return self;
    }


// -----------------------------------------------------------------------------
// CWimPin::ConstructL()
// ConstructL -this method can leave
// -----------------------------------------------------------------------------
//
void CWimPin::ConstructL( const TDesC& aTokenLabel )  
    {
    _WIMTRACE ( _L( "CWimPin::ConstructL()" ) );
    CActiveScheduler::Add( this );
    iTokenLabel = aTokenLabel.AllocL();
    iLabel = HBufC::NewL( KLabelLen ); 
    }

// -----------------------------------------------------------------------------
// CWimPin::~CWimPin()
// Destructor, allocated memory is released.
// -----------------------------------------------------------------------------
//
EXPORT_C CWimPin::~CWimPin()
    {
    _WIMTRACE ( _L( "CWimPin::~CWimPin()" ) );
    Cancel();
    delete iLabel;
    delete iTokenLabel;
    }

// -----------------------------------------------------------------------------
// CWimPin::EnablePinQuery()
// Enables PIN query of the PIN
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimPin::EnablePinQuery( TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimPin::EnablePinQuery()" ) );
    
    iClientStatus = &aStatus;
    aStatus = KRequestPending;
    
    iPhase = EEnablePinQueryStart;
    SignalOwnStatusAndComplete();
    }

// -----------------------------------------------------------------------------
// CWimPin::CancelEnablePinQuery()
// Cancel Enables PIN query of the PIN
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimPin::CancelEnablePinQuery()
    {
    _WIMTRACE ( _L( "CWimPin::CancelEnablePinQuery()" ) );
    
    if( IsActive() )
        {
    	Cancel();
        }
    }

// -----------------------------------------------------------------------------
// CWimPin::DisablePinQuery()
// Disables PIN query of the PIN
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimPin::DisablePinQuery( TRequestStatus& aStatus )
    {  
    _WIMTRACE ( _L( "CWimPin::DisablePinQuery()" ) );
    
    iClientStatus = &aStatus;
    aStatus = KRequestPending;
    
    iPhase = EDisablePinQueryStart;
    SignalOwnStatusAndComplete();
    
    }

// -----------------------------------------------------------------------------
// CWimPin::CancelDisablePinQuery()
// Cancel Disables PIN query of the PIN
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimPin::CancelDisablePinQuery()
    {  
    _WIMTRACE ( _L( "CWimPin::CancelDisablePinQuery()" ) );
    
    if( IsActive() )
        {
    	Cancel();
        }
    }    

// -----------------------------------------------------------------------------
// CWimPin::ChangePin()
// Changes PIN
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimPin::ChangePin( TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimPin::ChangePin()" ) );
    iClientStatus = &aStatus;
    aStatus = KRequestPending;
    
    iPhase = EChangePinStart;
    SignalOwnStatusAndComplete();  
    }

// -----------------------------------------------------------------------------
// CWimPin::CancelChangePin()
// Cancel Changes PIN
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimPin::CancelChangePin()
    {
    _WIMTRACE ( _L( "CWimPin::CancelChangePin()" ) );
    if( IsActive() )
        {
    	Cancel();
        }
    }
// -----------------------------------------------------------------------------
// CWimPin::UnblockPin()
// Unblocks PIN 
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimPin::UnblockPin( TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimPin::UnblockPin()" ) );

    iClientStatus = &aStatus;
    aStatus = KRequestPending;
    
    iPhase = EUnblockPinStart;
    SignalOwnStatusAndComplete();
        
    }

// -----------------------------------------------------------------------------
// CWimPin::CancelUnblockPin()
// Cancel Unblocks PIN 
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimPin:: CancelUnblockPin()
    {
    _WIMTRACE ( _L( "CWimPin::CancelUnblockPin()" ) );

    if( IsActive() )
        {
    	Cancel();
        }
        
    }    

// -----------------------------------------------------------------------------
// CWimPin::VerifyPin()
// Verifies the entered PIN request.
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimPin::VerifyPin( TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimPin::VerifyPin()" ) );

    iClientStatus = &aStatus;
    aStatus = KRequestPending;
    iPhase = EVerifyPinStart;
    SignalOwnStatusAndComplete();
   
    }
    
// -----------------------------------------------------------------------------
// CWimPin::CancelVerifyPin()
// Cancel Verifies the entered PIN request.
// -----------------------------------------------------------------------------
//    
EXPORT_C void CWimPin::CancelVerifyPin()
    {
    _WIMTRACE ( _L( "CWimPin::CancelVerifyPin()" ) );

    if( IsActive() )
        {
    	Cancel();
        }
   
    }

// -----------------------------------------------------------------------------
// CWimPin::Label
// Returns PIN's label.
// -----------------------------------------------------------------------------
//
EXPORT_C TPtrC CWimPin::Label() 
    {
    _WIMTRACE ( _L( "CWimPin::Label()" ) );
    return iLabel->Des();    
    }


// -----------------------------------------------------------------------------
// CWimPin::PinStatus()
// returns the status of PIN.
// -----------------------------------------------------------------------------
//
EXPORT_C TWimPinStatus CWimPin::PinStatus() 
    {
    _WIMTRACE ( _L( "CWimPin::PinStatus()" ) );
    return iPinStatus;
    }

// -----------------------------------------------------------------------------
// CWimPin::PinNumber()
// returns the number of the PIN. It can be 0,1,2....
// -----------------------------------------------------------------------------
//
EXPORT_C TUint8 CWimPin::PinNumber()
    {
    _WIMTRACE ( _L( "CWimPin::PinNumber()" ) );
    return iPinNumber;
    }

// -----------------------------------------------------------------------------
// CWimPin::RunL()
// Different phases are handled here
// iPhase = EEnablePinQueryStart: Sends enable PIN query message to server.
// iPhase = EEnablePinQueryEnd: Deallocates pckgbuffer used for message sending
//                              and completes request with received error code.
// iPhase = EDisablePinQueryStart: Sends disable PIN query message to server.
// iPhase = EDisablePinQueryEnd: Deallocates pckgbuffer used for message sending
//                               and completes request with received error code.
// iPhase = EChangePinStart: Sends change PIN message to server. 
// iPhase = EChangePinEnd: Dealloactes pckgbuffer used for message sending and 
//                         completes request with received error code.
// iPhase = EUnblockPinStart: Sends unblock PIN message to server
// iPhase = EUnblockPinEnd: Deallocates pckgbuffer used for message sending
//                               and completes request with received error code.
// iPhase = EVerifyPinStart: Sends verify PIN message to server.
// iPhase = EVerifyPinEnd:Deallocates pckgbuffer used for message sending
//                               and completes request with received error code.
// -----------------------------------------------------------------------------
//
void CWimPin::RunL()
    {
    _WIMTRACE ( _L( "CWimPin::RunL Start()" ) );
    if( iStatus.Int() == KErrCancel )
        {
        User::RequestComplete( iClientStatus, KErrCancel );
    	return;
        }
        
    switch( iPhase ) 
        {
        case EEnablePinQueryStart:
            {
            _WIMTRACE ( _L( "CWimPin::RunL EEnablePinQueryStart" ) );
            iCurrentPhase = EEnablePinQueryStart;
            
            PinParams( iPinParams );
            iPinStateRequest.iEnable = ETrue; //Enable PIN query
            iClientSession->EnableDisablePinQueryL( iReference,
                                                    iPinStateRequest,
                                                    iPinParams,
                                                    iStatus );
            iPhase = EEnablePinQueryEnd;
            SetActive();
            break;
            }
        case EEnablePinQueryEnd:
            {
            iCurrentPhase = EEnablePinQueryEnd;
            
            iClientSession->DeallocMemoryFromEnableDisablePinQuery();
            
            iReceivedStatus = iStatus.Int();
            iPhase = EUpdatePinStatus;
            SignalOwnStatusAndComplete();
            break;
            }
        case EDisablePinQueryStart:
            {
            _WIMTRACE ( _L( "CWimPin::RunL EDisablePinQueryStart" ) );
            iCurrentPhase = EDisablePinQueryStart;
            
            PinParams( iPinParams );
            iPinStateRequest.iEnable = EFalse; //Disable PIN query
            iClientSession->EnableDisablePinQueryL( iReference,
                                                        iPinStateRequest,
                                                        iPinParams,
                                                        iStatus );
            iPhase = EDisablePinQueryEnd;
            SetActive();
            break;
            }
        case EDisablePinQueryEnd:
            {
            
            iCurrentPhase = EDisablePinQueryEnd;
            
            iClientSession->DeallocMemoryFromEnableDisablePinQuery();
            
            iReceivedStatus = iStatus.Int();
            iPhase = EUpdatePinStatus;
            SignalOwnStatusAndComplete();
            break;
            }
        case EChangePinStart:
            {
            _WIMTRACE ( _L( "CWimPin::RunL EChangePinStart" ) );
            iCurrentPhase = EChangePinStart;

            
            PinParams( iPinParams );            
            iClientSession->PinOperationL( iReference,
                                           iPinParams, 
                                           EChangePINReq, 
                                           iStatus );
            iPhase = EChangePinEnd;
            SetActive();
            break;
            }
        case EChangePinEnd:
            {
            
            iCurrentPhase = EChangePinEnd;
            
            iClientSession->DeallocMemoryFromEnableDisablePinQuery();
            
            iReceivedStatus = iStatus.Int();
            iPhase = EUpdatePinStatus;
            SignalOwnStatusAndComplete();
            break;
            }
        case EUnblockPinStart:
            {
            _WIMTRACE ( _L( "CWimPin::RunL EUnblockPinStart" ) );
            iCurrentPhase = EUnblockPinStart;
            
            PinParams( iPinParams );
            iClientSession->PinOperationL( iReference, 
                                           iPinParams,
                                           EUnblockPinReq,
                                           iStatus );
            iPhase = EUnblockPinEnd;
            SetActive();
            break;
            }
        case EUnblockPinEnd:
            {
            
            iCurrentPhase = EUnblockPinEnd;
            
            iClientSession->DeallocMemoryFromEnableDisablePinQuery();
            
            iReceivedStatus = iStatus.Int();
            iPhase = EUpdatePinStatus;
            SignalOwnStatusAndComplete();
            break;
            }
        case EVerifyPinStart:
            {
            _WIMTRACE ( _L( "CWimPin::RunL EVerfiyPinStart" ) );
            iCurrentPhase = EVerifyPinStart;
            
            PinParams( iPinParams );
            iClientSession->PinOperationL( iReference, 
                                           iPinParams,
                                           EVerifyPinReq,
                                           iStatus );
            iPhase = EVerifyPinEnd;
            SetActive();
            break;
            }
        case EVerifyPinEnd:
            {
            
            iCurrentPhase = EVerifyPinEnd;
            
            iClientSession->DeallocMemoryFromEnableDisablePinQuery();
            
            iReceivedStatus = iStatus.Int();
            iPhase = EUpdatePinStatus;
            SignalOwnStatusAndComplete();
            break;
            }
        case EUpdatePinStatus:
            {
            _WIMTRACE ( _L( "CWimPin::RunL EUpdatePinStatus" ) );
            iCurrentPhase = EUpdatePinStatus;
            
            PinInfo( iStatus );
            iPhase = EUpdatePinStatusDone;
            SetActive();
            break;	
            }
        case EUpdatePinStatusDone:
            {
            
            iCurrentPhase = EUpdatePinStatusDone;
            
             if ( iStatus.Int() == KErrNone )
		        {
		        TPtr label = iLabel->Des();
		        label.Copy( iPinStruct.iLabel.Ptr(), iPinStruct.iLabel.Length() );
		        label.ZeroTerminate();
		        iPinStatus = iPinStruct.iStatus;
		        iPinNumber = iPinStruct.iPinNumber;
		        
		        User::RequestComplete( iClientStatus, iReceivedStatus );
		        }
		     else
		        {
		        User::RequestComplete( iClientStatus, iStatus.Int() );	
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
// CWimPin::DoCancel()
// Not supported
// -----------------------------------------------------------------------------
void CWimPin::DoCancel()
    {
    switch( iCurrentPhase )
	    {
	    case EVerifyPinStart:
	        {
	    	iClientSession->CancelPinOperation( iReference, 
                                                ECancelVerifyPin );
	    	break;
	        }
	    case EEnablePinQueryStart:
	    case EDisablePinQueryStart:
	        {
	        iClientSession->CancelEnableDisablePin( iReference );
	    	break;
	        }
	       
	    case EChangePinStart:
	        {
	        iClientSession->CancelPinOperation( iReference, 
                                                ECancelChangePin );
	    	break;
	        }
	     case EUnblockPinStart:
	        {
	        iClientSession->CancelPinOperation( iReference, 
                                                ECancelUnblockPin );
	        break;	
	        }
	        
	     default: break;   
	    }
	    
    User::RequestComplete( iClientStatus, KErrCancel );
    }

// -----------------------------------------------------------------------------
// CWimPin::RunError()
//
// The active scheduler calls this function if this active object's RunL() 
// function leaves. This gives this active object the opportunity to perform 
// any necessary cleanup.
// After array's cleanup, complete request with received error code.
// -----------------------------------------------------------------------------
TInt CWimPin::RunError( TInt aError )
    {
    iClientSession->DeallocMemoryFromEnableDisablePinQuery();
    User::RequestComplete( iClientStatus, aError );
    return KErrNone;    
    }

// -----------------------------------------------------------------------------
// CWimPin::PinParams()
// Constructs PIN -params and returns it
// -----------------------------------------------------------------------------
void CWimPin::PinParams( TPINParams& aPinParams)
    {
    _WIMTRACE ( _L( "CWimPin::PinParams()" ) );
    aPinParams.iMaxLength = KMaxPinLen;
    aPinParams.iMinLength = KMinPinLen;
    aPinParams.iPINLabel.Copy( *iLabel );
    aPinParams.iTokenLabel.Copy( *iTokenLabel );
    }


// -----------------------------------------------------------------------------
// CWimPin::SignalOwnStatusAndComplete()       
// Sets own iStatus to KRequestPending, and signals it 
// with User::RequestComplete() -request. This gives change 
// active scheduler to run other active objects. After a quick
// visit in active scheduler, signal returns to RunL() and starts next
// phase of operation. 
// @return void
// -----------------------------------------------------------------------------
void CWimPin::SignalOwnStatusAndComplete()
    {
    _WIMTRACE ( _L( "CWimPin::SignalOwnStatusAndComplete()" ) );
    iStatus = KRequestPending;
    SetActive();
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, KErrNone );
    }

// -----------------------------------------------------------------------------
// CWimPin::PINChangeable 
// Checks can you change the PIN number.
// -----------------------------------------------------------------------------
//
TBool CWimPin::PinChangeable()
    {
    _WIMTRACE ( _L( "CWimPin::PinChangeable()" ) );
    return !( iPinStatus & EChangeDisabled );    
    }


// -----------------------------------------------------------------------------
// CWimPin:::PinInfo()
// Get PIN info. Gets always latest information from WIM.
// -----------------------------------------------------------------------------
//
void CWimPin::PinInfo( TRequestStatus& aStatus ) 
    {   
    _WIMTRACE ( _L( "CWimPin::GetPinInfo()" ) );

    iPinStruct.iLabel.Delete( 0, KLabelLen );
    iPinStruct.iStatus = 0;
    
    iClientSession->PINInfo( iReference, iPinStruct, aStatus );

    }

// -----------------------------------------------------------------------------
// CWimPin::SetClientSession()
// Sets client session
// -----------------------------------------------------------------------------
//
void CWimPin::SetClientSession( RWimMgmt* aClientSession )
    {    
     _WIMTRACE ( _L( "CWimPin::SetClientSession()" ) );
    iClientSession = aClientSession;
    }

// -----------------------------------------------------------------------------
// CWimPin::SetLabel()
// Sets label
// -----------------------------------------------------------------------------
//
void CWimPin::SetLabel( TBuf<KLabelLen>& aLabel )
    {
    _WIMTRACE ( _L( "CWimPin::SetLabel()" ) );	
 	  TPtr label = iLabel->Des();
    label.Copy( aLabel.Ptr(), aLabel.Length() );
    label.ZeroTerminate(); 
    }
    
// -----------------------------------------------------------------------------
// CWimPin::SetPinStatus()
// Sets Pin Status
// -----------------------------------------------------------------------------
//   
void CWimPin::SetPinStatus( TWimPinStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimPin::SetPinStatus()" ) );	
	  iPinStatus = aStatus;
    }

// -----------------------------------------------------------------------------
// CWimPin::SetPinNumber()
// Sets pin number
// -----------------------------------------------------------------------------
//
void CWimPin::SetPinNumber( TUint8& aPinNumber )
    {
    _WIMTRACE ( _L( "CWimPin::SetPinNumber()" ) );	
    iPinNumber = aPinNumber;
    }
