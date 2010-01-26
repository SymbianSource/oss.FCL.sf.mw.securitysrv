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
#include "WimMgmt.h"
#include "WimTrace.h"

// -----------------------------------------------------------------------------
// RWimMgmt::RWimMgmt()
// Default constructor
// -----------------------------------------------------------------------------
//
RWimMgmt::RWimMgmt()
    {
    _WIMTRACE ( _L( "RWimMgmt::RWimMgmt()" ) );
    iPinStateRequestBufAllocated = EFalse;
    iPinParamsBufAllocated = EFalse;
    iPinInfoLstPtr = NULL;
    iPinModule = NULL;
    }


// -----------------------------------------------------------------------------
// RWimMgmt::~RWimMgmt()
// Destructor
// -----------------------------------------------------------------------------
//
RWimMgmt::~RWimMgmt()
    {
    _WIMTRACE ( _L( "RWimMgmt::~RWimMgmt()" ) );
    delete iPinInfoLstPtr;
    delete iPinModule;
    }

// -----------------------------------------------------------------------------
// RWimMgmt::WIMCount()
// Returns  TUint - the number of WIMs
// -----------------------------------------------------------------------------
//
TUint RWimMgmt::WIMCount()
    {
    _WIMTRACE ( _L( "RWimMgmt::WIMCount()" ) );

    TPckgBuf<TUint> pckg;

    TIpcArgs args;
    args.Set( 0, &pckg );
    
    if ( SendReceiveData( EGetWIMCount, args ) == KErrNone )
        {
        return pckg();
        }
    else
        {
        return 0;
        }
    }

// -----------------------------------------------------------------------------
// RWimMgmt::IsOpen()
// Tests whether WIM is open or not.
// -----------------------------------------------------------------------------
//
TBool RWimMgmt::IsOpen( const TWimAddress aWimAddr )
    {
    _WIMTRACE ( _L( "RWimMgmt::IsOpen()" ) );

    TPckgBuf<TBool> pckg;
    
    TIpcArgs args;
    args.Set( 0, aWimAddr );
    args.Set( 1, &pckg );
    
    SendReceiveData( EIsWIMOpen, args );
    
    return pckg();
    }

// -----------------------------------------------------------------------------
// RWimMgmt::CloseWIM()
// Closes the WIM and returns the status of the WIMI_CloseWIM()
// -----------------------------------------------------------------------------
//
TInt RWimMgmt::CloseWIM( const TWimAddress aWimAddr )
    {
    _WIMTRACE ( _L( "RWimMgmt::CloseWIM()" ) );

    TIpcArgs args;
    args.Set( 0, aWimAddr );
    
    return SendReceiveData( EWIMClose, args );
    }

// -----------------------------------------------------------------------------
// RWimMgmt::CloseAfter()
// Returns the time which is set as WIM closeout time
// -----------------------------------------------------------------------------
//
TInt RWimMgmt::CloseAfter()
    {
    _WIMTRACE ( _L( "RWimMgmt::CloseAfter()" ) );

    TPckgBuf<TInt> pckg;
    
    TIpcArgs args;
    args.Set( 0, &pckg );
    
    if ( SendReceiveData( EGetCloseWIMAfter, args ) == KErrNone )
        {
        return pckg();
        }
    else
        {
        return 0;
        }
    }

// -----------------------------------------------------------------------------
// RWimMgmt::CloseAfter()
// Returns TUint - the timeout in minutes, which tells how long 
// WIM Security Module will be open
// -----------------------------------------------------------------------------
//
TInt RWimMgmt::TimeRemaining()
    {
    _WIMTRACE ( _L( "RWimMgmt::TimeRemaining()" ) );

    TPckgBuf<TInt> pckg;
    
    TIpcArgs args;
    args.Set( 0, &pckg );
      
    if ( SendReceiveData( EWimTimeRemaining, args ) == KErrNone )
        {
        return pckg();
        }
    else
        {
        return 0;
        }
    }
// -----------------------------------------------------------------------------
// RWimMgmt::SetCloseAfter()
// Sets closing time for WIM.
// -----------------------------------------------------------------------------
//
void RWimMgmt::SetCloseAfter( const TUint aCloseAfter )
    {
    _WIMTRACE ( _L( "RWimMgmt::SetCloseAfter()" ) );
    
    TIpcArgs args;
    args.Set( 0, aCloseAfter );
    
    SendReceiveData( ECloseWIMAfter, args );
    }


// -----------------------------------------------------------------------------
// RWimMgmt::WIMRefs()
// Gets the array of WIM structures. aCount is the number of WIMs
// -----------------------------------------------------------------------------
//
TInt RWimMgmt::WIMRefs( TWimAddressList aWimAddrLst, TUint8 aCount )
    {
    _WIMTRACE ( _L( "RWimMgmt::WIMRefs()" ) );

    TInt length = ( TInt )( aCount * sizeof( TWimAddress ) );
    TPtr8 pt( ( TUint8* )aWimAddrLst, length, length );
    
    TIpcArgs args;
    args.Set( 0, &pt );
    
    return SendReceiveData( EGetWIMRefs, args );
    }

// -----------------------------------------------------------------------------
// RWimMgmt::WIMInfo()
// Gets the WIM information in to aTWimSecModuleStruct
// -----------------------------------------------------------------------------
//
TInt RWimMgmt::WIMInfo( const TWimAddress aWimAddr, 
                        TWimSecModuleStruct& aTWimSecModuleStruct )
    {
    _WIMTRACE ( _L( "RWimMgmt::WIMInfo()" ) );

    TPckg<TWimSecModuleStruct> wimModule( aTWimSecModuleStruct );
    
    TIpcArgs args;
    args.Set( 0, aWimAddr );
    args.Set( 1, &wimModule );
    
    return SendReceiveData( EGetWIMInfo, args );
    }


// -----------------------------------------------------------------------------
// RWimMgmt::PINCount()
// Returns  TUint - the number of PINs
// -----------------------------------------------------------------------------
//
TUint RWimMgmt::PINCount( const TWimAddress aWimAddr )
    {
    _WIMTRACE ( _L( "RWimMgmt::PINCount()" ) );

    TPckgBuf<TUint> pckg;
 
    TIpcArgs args;
    args.Set( 0, aWimAddr );
    args.Set( 1, &pckg );

    if ( SendReceiveData( EGetPINCount, args ) == KErrNone )
        {
        return pckg();
        }
    else
        {
        return 0;
        }
    }

// -----------------------------------------------------------------------------
// RWimMgmt::IsBlocked()
// Checks is PIN blocked or not.
// -----------------------------------------------------------------------------
//
TInt RWimMgmt::IsBlocked( const TPinAddress aPinAddr )
    {
    _WIMTRACE ( _L( "RWimMgmt::IsBlocked()" ) );
    
    TIpcArgs args;
    args.Set( 0, aPinAddr );

    return SendReceiveData( EIsPinBlocked, args );
    }

// -----------------------------------------------------------------------------
// RWimMgmt::IsDisabledPINBlocked()
// Checks is PIN blocked or not.
// -----------------------------------------------------------------------------
//
TInt RWimMgmt::IsDisabledPINBlocked( const TPinAddress aPinAddr )
    {
    _WIMTRACE ( _L( "RWimMgmt::IsDisabledPINBlocked()" ) );
    
    TIpcArgs args;
    args.Set( 0, aPinAddr );
    
    return SendReceiveData( EIsDisabledPinBlocked, args );
    }


// -----------------------------------------------------------------------------
// RWimMgmt::PINInfo()
// Gets the PIN information in to rPinStruct
// -----------------------------------------------------------------------------
//
void RWimMgmt::PINInfo( const TPinAddress aPinAddr, 
                        TWimPinStruct& aPinStruct,
                        TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "RWimMgmt::PINInfo()" ) );
    
    if( iPinModule != NULL )
    {
    delete iPinModule;
    iPinModule = NULL; 	
    }
    
    iPinModule = new TPckg<TWimPinStruct>( aPinStruct );
    
    TIpcArgs args;
    args.Set( 0, aPinAddr );
    args.Set( 1, iPinModule );
  
    SendReceiveData( EGetPINInfo, args, aStatus );
    }


// -----------------------------------------------------------------------------
// RWimMgmt::PINRefs()
// Gets the array of PIN structures.
// -----------------------------------------------------------------------------
//
TInt RWimMgmt::PINRefs( const TWimAddress aWimAddr, 
                        TUint32& aPinLstAddr, 
                        TPinAddressList aPinAddrLst, 
                        TUint8 aCount )
    {
    _WIMTRACE ( _L( "RWimMgmt::PINRefs()" ) );
    TInt status = 0;

    TInt16 length = ( TInt16 ) ( aCount * sizeof( TWimAddress ) );
    TPtr8 pt( ( TUint8* ) aPinAddrLst, length, length );
    TPckgBuf<TUint32> lstAddr;
    
    TIpcArgs args;
    args.Set( 0, aWimAddr );
    args.Set( 1, &lstAddr );
    args.Set( 2, &pt );
    
    status = SendReceiveData( EGetPINRefs, args );
    
    if ( status == KErrNone )
        {
        aPinLstAddr = lstAddr();
        }
    else
        {
        aPinLstAddr = NULL;
        }
        
    return status;
    }
    
    
// -----------------------------------------------------------------------------
// RWimMgmt::PINsInfo() Asynchronous
// Initialize elements in PinNR array
// -----------------------------------------------------------------------------
//    
void RWimMgmt::PINsInfo( const TWimAddress aWimAddr, 
               CArrayFixFlat<TWimPinStruct>& aPinInfoLst,
               TUint8 aCount,
               TRequestStatus& aStatus )
  {
   _WIMTRACE ( _L( "RWimMgmt::PINsInfo()" ) );
   TInt16 length = ( TInt16 ) ( aCount * sizeof( TWimPinStruct ) );
    
   if( iPinInfoLstPtr == NULL )
	  {
	  iPinInfoLstPtr = new TPtr8( (TText8*)&aPinInfoLst[0],length, length ); 
	  }
   else
      {
      iPinInfoLstPtr->Set( (TText8*)&aPinInfoLst[0], length, length );
      } 
  
   TIpcArgs args;
   args.Set( 0, aWimAddr );
   args.Set( 1, iPinInfoLstPtr );
   args.Set( 2, aCount );
    
   aStatus = KRequestPending;
   SendReceiveData( EGetPINsInfo, args, aStatus );         	
   }

// -----------------------------------------------------------------------------
// RWimMgmt::PINsInfo() Synchronous
// Initialize elements in PinNR array
// -----------------------------------------------------------------------------
// 
TInt RWimMgmt::PINsInfo( const TWimAddress aWimAddr, 
               CArrayFixFlat<TWimPinStruct>& aPinInfoLst,
               TUint8 aCount )
  {
  _WIMTRACE ( _L( "RWimMgmt::PINsInfo() synchronous" ) );
    
  TInt16 length = ( TInt16 ) ( aCount * sizeof( TWimPinStruct ) );
    
  if( iPinInfoLstPtr == NULL )
	  {
	  iPinInfoLstPtr = new TPtr8( (TText8*)&aPinInfoLst[0],length,length );	
	  }
  else
      {
      iPinInfoLstPtr->Set( (TText8*)&aPinInfoLst[0], length, length );	
      }  
   
   TIpcArgs args;
   args.Set( 0, aWimAddr );
   args.Set( 1, iPinInfoLstPtr );
   args.Set( 2, aCount );
    
   return SendReceiveData( EGetPINsInfo, args );
        	
   }

// -----------------------------------------------------------------------------
// RWimMgmt::EnableDisablePinQueryL()
// Handles Enable and Disable PIN requests. Server
// knows wanted operation by flag which is set in TPINStateRequest.
// -----------------------------------------------------------------------------
//
void RWimMgmt::EnableDisablePinQueryL( const TPinAddress aPinAddr, 
                                       const TPINStateRequest& aPinStateRequest,
                                       const TPINParams& aPinParams,
                                       TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "RWimMgmt::EnableDisablePinQuery()" ) );

    iPinStateRequestBuf = new( ELeave )  
        CWimCertPckgBuf<TPINStateRequest>( aPinStateRequest );
    iPinStateRequestBufAllocated = ETrue;
        
    iPinParamsBuf = new( ELeave )  
        CWimCertPckgBuf<TPINParams>( aPinParams );
    iPinParamsBufAllocated = ETrue;

    TIpcArgs args;
    args.Set( 0, aPinAddr );
    args.Set( 1, iPinStateRequestBuf->PckgBuf() );
    args.Set( 2, iPinParamsBuf->PckgBuf() );

    aStatus = KRequestPending;
    SendReceiveData( EEnablePINReq, args, aStatus );
    }

// -----------------------------------------------------------------------------
// RWimMgmt::CancelEnableDisablePinQueryL()
// Cancel Enable and Disable PIN requests. Server
// knows wanted operation by flag which is set in TPINStateRequest.
// -----------------------------------------------------------------------------
//
void RWimMgmt::CancelEnableDisablePin( const TPinAddress aPinAddr )
    {
    _WIMTRACE ( _L( "RWimMgmt::CancelEnableDisablePinQuery()" ) );
    TIpcArgs args;
    args.Set( 0, aPinAddr );
    SendReceiveData( ECancelEnablePin, args );
    }


// -----------------------------------------------------------------------------
// RWimMgmt::DeallocMemoryFromEnableDisablePinQuery()
// Deallocates memory from pckgBuf member variables
// -----------------------------------------------------------------------------
//
void RWimMgmt::DeallocMemoryFromEnableDisablePinQuery()
    {
    _WIMTRACE ( _L( "RWimMgmt::DeallocMemoryFromEnableDisablePinQuery()" ) );

    if ( iPinStateRequestBufAllocated )
        {
        delete iPinStateRequestBuf;
        iPinStateRequestBuf = NULL;
        iPinStateRequestBufAllocated = EFalse;
        }
    if ( iPinParamsBufAllocated )
        {
        delete iPinParamsBuf;
        iPinParamsBuf = NULL;
        iPinParamsBufAllocated = EFalse;
        }
    }


// -----------------------------------------------------------------------------
// RWimMgmt::PinOperationL()
// PinOperationL handles several PIN operations: it can be
// verify PIN request, change PIN request or unblock PIN request. The 
// type of request is handled via aOpCode.
// -----------------------------------------------------------------------------
//
void RWimMgmt::PinOperationL( const TPinAddress aPinAddr,
                              const TPINParams aPinParams,
                              const TWimServRqst aOpCode,
                              TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "RWimMgmt::PinOperationL" ) );

    iPinParamsBuf = new( ELeave ) CWimCertPckgBuf<TPINParams>( aPinParams );
    iPinParamsBufAllocated = ETrue;

    TIpcArgs args;
    args.Set( 0, aPinAddr );
    args.Set( 1, iPinParamsBuf->PckgBuf() );
    
    aStatus = KRequestPending;
    SendReceiveData( aOpCode, args, aStatus );
    }

// -----------------------------------------------------------------------------
// RWimMgmt::CancelPinOperationL()
// PinOperationL handles several PIN operations: it can be
// verify PIN request, change PIN request or unblock PIN request. The 
// type of request is handled via aOpCode.
// -----------------------------------------------------------------------------
//
void RWimMgmt::CancelPinOperation( const TPinAddress aPinAddr,
                                   const TWimServRqst aOpCode )
    {
    _WIMTRACE ( _L( "RWimMgmt::CancelPinOperation" ) );
    TIpcArgs args;
    args.Set( 0, aPinAddr );
    SendReceiveData( aOpCode, args );
    }

// -----------------------------------------------------------------------------
// RWimMgmt::ClientSessionL()
// Returns new RWimMgmt object
// -----------------------------------------------------------------------------
//
RWimMgmt* RWimMgmt::ClientSessionL()
    {
    _WIMTRACE ( _L( "RWimMgmt::ClientSessionL()" ) );
    return new( ELeave ) RWimMgmt();
    }

// End of File
