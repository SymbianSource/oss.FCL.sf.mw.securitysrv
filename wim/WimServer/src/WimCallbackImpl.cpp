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
* Description:  This file contain the implementation of CWimCallBack class.
*
*/



// INCLUDE FILES
#include    "WimCallbackImpl.h"
#include    "WimSession.h"
#include    "WimServer.h"
#include    "WimResponse.h"
#include    "WimApduImpl.h"
#include    "WimSimFileHandler.h"
#include    "WimUtilityFuncs.h"
#include    "WimConsts.h"
#include    "WimTrace.h"
#include    <x509cert.h>

// These variables are static because static function calls from WIMI
static CWimApdu* gApdu = NULL;
static CWimSimFileHandler* gSimFileHandler = NULL;
static TRequestStatus* gStatus = NULL;

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWimCallBack::InitOkResp
// Respond to InitOK operation
// -----------------------------------------------------------------------------
//
void CWimCallBack::InitOkResp( WIMI_STAT aStatus )
    {
    _WIMTRACE2(_L("WIM|WIMServer|CWimCallBack::InitOkResp|Begin aStatus=%d"), aStatus);
    CWimServer::iWimStatus = CWimUtilityFuncs::MapWIMError( aStatus );   
    
    if ( aStatus == WIMI_Ok )
        {
        CWimServer::SetWimInitialized( ETrue, KErrNone ); // WIM has initialized
        _WIMTRACE(_L("WIM|WIMServer|CWimCallBack::InitOkResp|Initialized=True"));
        }
    else
        {
        if( aStatus == WIMI_ERR_UserCancelled )
	        {
	        CWimServer::iWimStatus = KErrCancel;
	        CWimServer::SetWimInitialized( EFalse, KErrCancel );
            WIMI_CloseDownReq();
	        }
        else
	        {
	        CWimServer::SetWimInitialized( EFalse, CWimServer::iWimStatus ); // WIM has not initialized	
            WIMI_CloseDownReq();
	        }
        
        _WIMTRACE(_L("WIM|WIMServer|CWimCallBack::InitOkResp|Initialized=False"));
        }
    }

// -----------------------------------------------------------------------------
// CWimCallBack::CloseDownResp
// Respond to CloseDown operation
// -----------------------------------------------------------------------------
//
void CWimCallBack::CloseDownResp( WIMI_STAT aStatus )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCallBack::CloseDownResp | Begin"));
    CWimServer::iWimStatus = CWimUtilityFuncs::MapWIMError( aStatus );
    CWimServer::SetWimInitialized( EFalse, CWimServer::iWimStatus ); // WIM has not initialized
    delete gSimFileHandler; // Close connection to CustomAPI
    gSimFileHandler = NULL;
    }

// -----------------------------------------------------------------------------
// CWimCallBack::CompleteResponse
// Complete response
// -----------------------------------------------------------------------------
//
void CWimCallBack::CompleteResponse(
    WIMI_TransactId_t aTrId,
    WIMI_STAT aStatus )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCallBack::CompleteResponse | Begin"));
    TWimReqTrId* trIdType = ( TWimReqTrId* )aTrId;
    CWimResponse* pTrId = ( CWimResponse* )( trIdType->iReqTrId );
    pTrId->iStatus = aStatus;
    pTrId->CompleteMsgAndDelete();
    delete trIdType;
    }

/* ------------ Digital signature ------------------------------------------- */

// -----------------------------------------------------------------------------
// CWimCallBack::SetSignTextRequestStatus
// Set RequestStatus of SignTextHandler
// -----------------------------------------------------------------------------
//
void CWimCallBack::SetSignTextRequestStatus( TRequestStatus* aStatus )
    {
    gStatus = aStatus;
    }

// -----------------------------------------------------------------------------
// CWimCallBack::SignResp
// Respond to the digital signature operation
// -----------------------------------------------------------------------------
//
void CWimCallBack::SignResp(
    WIMI_TransactId_t aTrId, 
    WIMI_STAT aStatus,
    TUint8 aSignLen,
    TUint8* aSign )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCallBack::SignResp | Begin"));

    TWimReqTrId* trIdType = ( TWimReqTrId* )aTrId;
    CWimResponse* trId = ( CWimResponse* )( trIdType->iReqTrId );
    trId->iStatus = aStatus;

    // In case PIN was incorrect try ask PIN again, don't complete message
    // If PIN is blocked show note and don't complete message yet.
    if ( aStatus != WIMI_ERR_BadPIN && aStatus != WIMI_ERR_PINBlocked && aStatus != WIMI_ERR_CardIOError )
        {
        if ( trId->iStatus == WIMI_Ok )
            {
            TRAP( trId->iError, SignRespL( trId, aSignLen, aSign ) );
            }
        WIMI_Ref_pt pWimRefTemp = NULL;
        pWimRefTemp = WIMI_GetWIMRef( 0 );
        if ( pWimRefTemp )  // Close the WIM
            {
            WIMI_CloseWIM( pWimRefTemp );
            free_WIMI_Ref_t( pWimRefTemp );
            }
        trId->CompleteMsgAndDelete(); 
        delete trIdType;
        }

    // Complete SignTextHandler status
    if ( gStatus )
        {
        User::RequestComplete( gStatus, aStatus );
        }
    else // SignTextHandler not active, complete message with error if not done
        {
        if ( aStatus == WIMI_ERR_BadPIN || aStatus == WIMI_ERR_PINBlocked || aStatus == WIMI_ERR_CardIOError )
            {
            trId->CompleteMsgAndDelete();
            delete trIdType;
            }
        }
    }

// -----------------------------------------------------------------------------
// CWimCallBack::SignRespL
// Write signed text to client memory. Signature allocated in WIMI is freed 
// also by WIMI.
// -----------------------------------------------------------------------------
//
void CWimCallBack::SignRespL(
    CWimResponse* aTrId,
    TUint8 aSignLen,
    TUint8* aSign )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCallBack::SignRespL | Begin"));

    RMessage2& message = aTrId->iMessage;

    TPtr8 signaturePtr( aSign, aSignLen, aSignLen );

    message.WriteL( 3, signaturePtr );
    }

/* ------------ Certificate management -------------------------------------- */

// -----------------------------------------------------------------------------
// CWimCallBack::CertificateResp
// Respond to Certificate fetch operation
// -----------------------------------------------------------------------------
//
void CWimCallBack::CertificateResp( 
    WIMI_TransactId_t aTrId,
    WIMI_STAT aStatus,
    WIMI_Ref_t* aCertRef,
    TUint16 aCertlen,
    TUint8* aCert )
    {
    if ( aTrId )
    	{
        _WIMTRACE(_L("WIM | WIMServer | CWimCallBack::CertificateResp | Begin"));
        TWimReqTrId* trIdType = ( TWimReqTrId* )aTrId;
        CWimResponse* pTrId = ( CWimResponse* )( trIdType->iReqTrId );
        pTrId->iStatus = aStatus;
        if ( pTrId->iStatus == WIMI_Ok)
            {
            TRAP( pTrId->iError,
                CertificateRespL( pTrId, aStatus, aCertRef, aCertlen, aCert ) );
            }
        pTrId->CompleteMsgAndDelete(); 
        delete trIdType;
    	}
    else
    	{
    	_WIMTRACE(_L("WIM | WIMServer | CWimCallBack::CertificateResp | certhandler starts this"));
    	if ( gStatus )
	        {
	        TPtrC8 certPtr( aCert, aCertlen );
	        CX509Certificate* cert = NULL;
	        TRAPD( err, cert =  CX509Certificate::NewL( certPtr ) );
	        _WIMTRACE2(_L("WIM | WIMServer | CWimCallBack::CertificateResp | x509 err %d"), err );
	        User::RequestComplete( gStatus, err );
	        delete cert;
	        cert = NULL;
	        }
    	}
    }

// -----------------------------------------------------------------------------
// CWimCallBack::CertificateRespL
// Respond to the certificate fetch operation
// -----------------------------------------------------------------------------
//
void CWimCallBack::CertificateRespL(
    CWimResponse* aTrId,
    WIMI_STAT /*aStatus*/,
    WIMI_Ref_t* /*aWimiRef*/,
    TUint16 aCertLen,
    TUint8* aCert )
    {
    _WIMTRACE(_L("WIM | WIMServer | CertificateRespL | Begin"));
    _WIMTRACE2(_L("WIM | WIMServer | The length of certificate is %d"), aCertLen);
    
    RMessage2& message = aTrId->iMessage;
        
    if ( aTrId->iStatus == WIMI_Ok )
        {
        TPtr8 certPtr( aCert, aCertLen, aCertLen );
        message.WriteL( 2, certPtr );
        }
    }

// -----------------------------------------------------------------------------
// CWimCallBack::CertificateDeleteResp
// Respond to the certificate delete operation
// -----------------------------------------------------------------------------
//
void CWimCallBack::CertificateDeleteResp( 
    WIMI_TransactId_t aTrId,
    WIMI_STAT aStatus )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCallBack::CertificateDeleteResp | Begin"));
    CompleteResponse( aTrId, aStatus );
    }

// -----------------------------------------------------------------------------
// CWimCallBack::CertificateStoreResp
// Respond to the certificate store operation
// -----------------------------------------------------------------------------
//
void CWimCallBack::CertificateStoreResp(
    WIMI_TransactId_t aTrId,
    WIMI_STAT aStatus,
    WIMI_Ref_t* /*aCertRef*/ )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCallBack::CertificateStoreResp | Begin"));
    TWimReqTrId* trIdType = ( TWimReqTrId* )aTrId;
    CWimResponse* pTrId = (CWimResponse*)( trIdType->iReqTrId );
    pTrId->iStatus = aStatus;

    pTrId->CompleteMsgAndDelete();
    delete trIdType;
    }

/* ------------ PIN management ---------------------------------------------- */

// -----------------------------------------------------------------------------
// CWimCallBack::VerifyPINResp
// Respond to Verify PIN operation
// -----------------------------------------------------------------------------
//
void CWimCallBack::VerifyPINResp( WIMI_TransactId_t aTrId, WIMI_STAT aStatus )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCallBack::VerifyPINResp | Begin"));
    if( aTrId )
	    {
	    TWimReqTrId* trIdType = ( TWimReqTrId* )aTrId;
	    CWimResponse* trId = ( CWimResponse* )( trIdType->iReqTrId );
	    trId->iStatus = aStatus;

	    // In case PIN was incorrect try ask PIN again, don't complete message
	    // If PIN is blocked show note and don't complete message yet.
        if ( aStatus != WIMI_ERR_BadPIN && aStatus != WIMI_ERR_PINBlocked && aStatus != WIMI_ERR_CardIOError )
	        { 	
	        trId->CompleteMsgAndDelete(); 
	        delete trIdType;
	        }
	        
	    if ( gStatus )
	        {
	        User::RequestComplete( gStatus, aStatus );
	        }
	    else // SignTextHandler not active, complete message with error if not done
	        {
	        if ( aStatus == WIMI_ERR_BadPIN || aStatus == WIMI_ERR_PINBlocked || aStatus == WIMI_ERR_CardIOError )
	            {
	            trId->CompleteMsgAndDelete();
	            delete trIdType;
	            }
	        }
	    }
    else
        {
    	if ( gStatus )
	        {
	        User::RequestComplete( gStatus, CWimUtilityFuncs::MapWIMError( aStatus ) );
	        }
        }    	 	
    }
    

// -----------------------------------------------------------------------------
// CWimCallBack::ChangePINResp
// Respond to Change PIN operation
// -----------------------------------------------------------------------------
//
void CWimCallBack::ChangePINResp( WIMI_TransactId_t aTrId, WIMI_STAT aStatus )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCallBack::ChangePINResp | Begin"));

    TWimReqTrId* trIdType = ( TWimReqTrId* )aTrId;
    CWimResponse* trId = ( CWimResponse* )( trIdType->iReqTrId );
    trId->iStatus = aStatus;

    // In case PIN was incorrect try ask PIN again, don't complete message
    // If PIN is blocked show note and don't complete message yet.
    if ( aStatus != WIMI_ERR_BadPIN && aStatus != WIMI_ERR_PINBlocked && aStatus != WIMI_ERR_CardIOError )
        {
        if( CWimServer::Server()->WimTimer()->TimeRemaining() == 0 )
            {
            _WIMTRACE(_L("WIM | WIMServer | CWimCallBack::ChangePINResp | TimeRemaing =0 "));
            WIMI_Ref_pt pWimRefTemp = NULL;
            pWimRefTemp = WIMI_GetWIMRef( 0 );
            if ( pWimRefTemp )  // Close the WIM
                {
                WIMI_CloseWIM( pWimRefTemp );
                free_WIMI_Ref_t( pWimRefTemp );
                }
            }    	
        trId->CompleteMsgAndDelete(); 
        delete trIdType;
        }
    
     
    // Complete SignTextHandler status
    if ( gStatus )
        {
        User::RequestComplete( gStatus, aStatus );
        }
    else // SignTextHandler not active, complete message with error if not done
        {
        if ( aStatus == WIMI_ERR_BadPIN || aStatus == WIMI_ERR_PINBlocked || aStatus == WIMI_ERR_CardIOError )
            {
            trId->CompleteMsgAndDelete();
            delete trIdType;
            }
        }
      _WIMTRACE(_L("WIM | WIMServer | CWimCallBack::ChangePINResp | 10"));   
    }

// -----------------------------------------------------------------------------
// CWimCallBack::UnblockPINResp
// Respond to Unblock PIN operation
// -----------------------------------------------------------------------------
//
void CWimCallBack::UnblockPINResp( WIMI_TransactId_t aTrId, WIMI_STAT aStatus )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCallBack::UnblockPINResp | Begin"));

    TWimReqTrId* trIdType = ( TWimReqTrId* )aTrId;
    CWimResponse* trId = ( CWimResponse* )( trIdType->iReqTrId );
    trId->iStatus = aStatus;

    // In case PIN was incorrect try ask PIN again, don't complete message
    // If PIN is blocked show note and don't complete message yet.
  
    if ( aStatus != WIMI_ERR_BadPIN && aStatus != WIMI_ERR_PINBlocked && aStatus != WIMI_ERR_CardIOError )
        {
        _WIMTRACE(_L("WIM | WIMServer | CWimCallBack::SignResp | TimeRemaing =0 "));
        WIMI_Ref_pt pWimRefTemp = NULL;
        pWimRefTemp = WIMI_GetWIMRef( 0 );
 
        if ( pWimRefTemp )  // Close the WIM
            {
            WIMI_CloseWIM( pWimRefTemp );
            free_WIMI_Ref_t( pWimRefTemp );
            }
        
        //  WIMI_InitializeReq( &CWimServer::Server()->iWimCallBack );
        
        trId->CompleteMsgAndDelete(); 
        delete trIdType;
        }

    // Complete SignTextHandler status
    if ( gStatus )
        {
        User::RequestComplete( gStatus, aStatus );
        }
    else // SignTextHandler not active, complete message with error if not done
        {
        if ( aStatus == WIMI_ERR_BadPIN || aStatus == WIMI_ERR_PINBlocked || aStatus == WIMI_ERR_CardIOError )
            {
            trId->CompleteMsgAndDelete();
            delete trIdType;
            }
        }
    }

// -----------------------------------------------------------------------------
// CWimCallBack::EnablePINResp
// Respond to Enable PIN Response
// -----------------------------------------------------------------------------
//
void CWimCallBack::EnablePINResp( WIMI_TransactId_t aTrId, WIMI_STAT aStatus )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCallBack::EnablePINResp | Begin"));
     
    TWimReqTrId* trIdType = ( TWimReqTrId* )aTrId;
    CWimResponse* trId = ( CWimResponse* )( trIdType->iReqTrId );
    trId->iStatus = aStatus;

    // In case PIN was incorrect try ask PIN again, don't complete message
    // If PIN is blocked show note and don't complete message yet.
    if ( aStatus != WIMI_ERR_BadPIN && aStatus != WIMI_ERR_PINBlocked && aStatus != WIMI_ERR_CardIOError )
        {
        if( CWimServer::Server()->WimTimer()->TimeRemaining() == 0 )
            {
            _WIMTRACE(_L("WIM | WIMServer | CWimCallBack::SignResp | TimeRemaing =0 "));
            WIMI_Ref_pt pWimRefTemp = NULL;
            pWimRefTemp = WIMI_GetWIMRef( 0 );
            if ( pWimRefTemp )  // Close the WIM
                {
                WIMI_CloseWIM( pWimRefTemp );
                free_WIMI_Ref_t( pWimRefTemp );
                }
            }    	
        trId->CompleteMsgAndDelete(); 
        delete trIdType;
        }

    // Complete SignTextHandler status
    if ( gStatus )
        {
        User::RequestComplete( gStatus, aStatus );
        }
    else // SignTextHandler not active, complete message with error if not done
        {
        if ( aStatus == WIMI_ERR_BadPIN || aStatus == WIMI_ERR_PINBlocked || aStatus == WIMI_ERR_CardIOError )
            {
            trId->CompleteMsgAndDelete();
            delete trIdType;
            }
        }
    }

/* ------------ Hardware driver support ------------------------------------- */
// -----------------------------------------------------------------------------
// CWimCallBack::CardInsertedResp
// Respond to CardInserted operation
// -----------------------------------------------------------------------------
//
void CWimCallBack::CardInsertedResp(
    TUint8 /*aReader*/,
    WIMI_STAT /*aStatus*/,
    WIMI_Ref_t* /*aWIMRef*/ )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCallBack::CardInsertedResp | Begin"));
    }

// -----------------------------------------------------------------------------
// CWimCallBack::Open
// Open connection to card
// -----------------------------------------------------------------------------
//
TUint8 CWimCallBack::Open( TUint8 aReader )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCallBack::Open | Begin"));
    __ASSERT_ALWAYS( gApdu, User::Panic( _L( "Null pointer" ), KErrGeneral ) );
    return gApdu->Open( aReader );
    }

// -----------------------------------------------------------------------------
// CWimCallBack::Close
// Close connection to card
// -----------------------------------------------------------------------------
//
TUint8 CWimCallBack::Close( TUint8 aReader )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCallBack::Close | Begin"));
    __ASSERT_ALWAYS( gApdu, User::Panic( _L( "Null pointer" ), KErrGeneral ) );

    TUint8 ret;
    ret = gApdu->Close( aReader );
    
     _WIMTRACE(_L("WIM | WIMServer | CWimCallBack::Close | 1"));
    return ret;
    }

// -----------------------------------------------------------------------------
// CWimCallBack::APDUReq
// APDU Request
// -----------------------------------------------------------------------------
//
void CWimCallBack::APDUReq( TUint8 aReader, TUint8* aApdu, TUint16 aApdulen )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCallBack::APDUReq | Begin"));
    __ASSERT_ALWAYS( gApdu, User::Panic( _L( "Null pointer" ), KErrGeneral ) );
    
    WIMI_BinData_t* ret_apdu =
        ( WIMI_BinData_t* )WSL_OS_Malloc( sizeof( WIMI_BinData_t ) );
    TUint16 sw;

    TUint8 status = gApdu->SendAPDU( aReader, aApdu, aApdulen );
    
    sw = gApdu->ResponseSW();
    TPtrC8 ptr = gApdu->ResponseApdu();
    ret_apdu->pb_buf = ( TUint8* )ptr.Ptr();
    ret_apdu->ui_buf_length = ( TUint16 )ptr.Length();
    
    WIMI_APDUResp( aReader, status, ret_apdu, sw );
    WSL_OS_Free( ret_apdu );
    }

// -----------------------------------------------------------------------------
// CWimCallBack::GetATRReq
// Not supported in Series 60
// -----------------------------------------------------------------------------
//
void CWimCallBack::GetATRReq( TUint8 /*aReader*/ )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCallBack::GetATRReq | Begin"));
    }

// -----------------------------------------------------------------------------
// CWimCallBack::GetReaderListReq
// Get reader list request
// -----------------------------------------------------------------------------
//
void CWimCallBack::GetReaderListReq()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCallBack::GetReaderListReq | Begin"));
    WIMI_BinData_t* list = 
        ( WIMI_BinData_t * )WSL_OS_Malloc( sizeof( WIMI_BinData_t ) ) ;

    TInt ret = KErrNone;
    TUint8 retval = KErrNone;

    if ( !gApdu )
        {
        TRAP( ret, gApdu = CWimApdu::NewL() ); // New CWimApdu
        }

    if ( ret != KErrNone )
        {
        _WIMTRACE2(_L("WIM|WIMServer|CWimCallBack::GetReaderListReq|CWimApdu::NewL() leaved with %d"), ret);
        retval = KWimStatusIOError;
        list->ui_buf_length = 0;
        list->pb_buf = NULL;
        }
    else
        {
        TRAP( ret, retval = gApdu->RequestListL() );
    
        if ( ret != KErrNone )
            {
            retval = KWimStatusIOError;
            }
        list->ui_buf_length = gApdu->StatusListLength();
        _WIMTRACE2(_L("WIM|WIMServer|CWimCallBack::GetReaderListReq|statuslistlength=%d"), list->ui_buf_length);
        list->pb_buf = (TUint8*)gApdu->StatusList().Ptr();
        }
    _WIMTRACE2(_L("WIM|WIMServer|CWimCallBack::GetReaderListReq|retval=%d"), retval);

    if ( retval == KErrNone )
        {
        WIMI_GetReaderListResp( WIMI_APDU_OK, list );
        }
    else
        {
        WIMI_GetReaderListResp( WIMI_APDU_TRANSMISSION_ERROR, list );
        }
    WSL_OS_Free( list );
    }

/* ------------ SIM file reading and OMA Provisioning ----------------------- */

// -----------------------------------------------------------------------------
// CWimCallBack::ReadSimFileReq
// Read SIM file request
// -----------------------------------------------------------------------------
//
void CWimCallBack::ReadSimFileReq( TUint8 aReaderId,
                                   TUint8* aPath,
                                   TUint16 aOffset,          
                                   TUint16 aSize )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCallBack::ReadSimFileReq | Begin"));

    TRAPD( ret, DoReadSimFileReqL( aReaderId, aPath, aOffset, aSize ) );

    if ( ret != KErrNone )  // DoReadSimFileReq leaved
        {
        _WIMTRACE2(_L("WIM | WIMServer | CWimCallBack::ReadSimFileReq | Trapped Leave, ret = %d"), ret );

        WIMI_BinData_t responseBytes;
        responseBytes.pb_buf = NULL;
        responseBytes.ui_buf_length = 0;
        TUint8 status = WIMI_Err;
        // WIMI call with zero buffer
        WIMI_SIM_ReadFileResp( aReaderId, status, &responseBytes );
        }
    }

// -----------------------------------------------------------------------------
// CWimCallBack::ReadSimFileReq
// Read SIM file request
// -----------------------------------------------------------------------------
//
void CWimCallBack::DoReadSimFileReqL( TUint8 aReaderId,
                                      TUint8* aPath,
                                      TUint16 aOffset,          
                                      TUint16 aSize )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCallBack::DoReadSimFileReqL | Begin"));

    if ( !gSimFileHandler ) 
        {
        gSimFileHandler = CWimSimFileHandler::NewL();
        }

    RMmCustomAPI::TSimFilePath filePath;
    filePath.Copy( aPath, KSimFilePathLength ); // SIM file path is 8 bytes

    gSimFileHandler->ReadSimFileL( aReaderId,
                                   filePath,
                                   aOffset,
                                   aSize );

    _WIMTRACE(_L("WIM | WIMServer | CWimCallBack::DoReadSimFileReqL | End"));
    }

// -----------------------------------------------------------------------------
// CWimCallBack::OmaProvisioningResp
// Response to OMA Provisioning Smart Card file request
// -----------------------------------------------------------------------------
//
void CWimCallBack::OmaProvisioningResp( WIMI_TransactId_t aTrId,
                                        WIMI_STAT aStatus,
                                        TUint16 aFileLen,
                                        TUint8* aFile )
    {
    _WIMTRACE2(_L("WIM | WIMServer | CWimCallBack::OmaProvisioningResp, aFileLen = %d"), aFileLen);

    TWimReqTrId* trIdType = ( TWimReqTrId* )aTrId;
    CWimResponse* pTrId = ( CWimResponse* )( trIdType->iReqTrId );
    pTrId->iStatus = aStatus;

    if ( pTrId->iStatus == WIMI_Ok )
        {
        TRAP( pTrId->iError, OmaProvisioningRespL( pTrId, aFileLen, aFile ) );
        _WIMTRACE2(_L("WIM | WIMServer | CWimCallBack::OmaProvisioningResp, TRAP(OmaProvisioningRespL)=%d"), pTrId->iError);
        }

    pTrId->CompleteMsgAndDelete(); 
    delete trIdType;
    _WIMTRACE(_L("WIM | WIMServer | CWimCallBack::OmaProvisioningResp | End"));
    }

// -----------------------------------------------------------------------------
// CWimCallBack::OmaProvisioningRespL
// Write OMA Provisioning Smart Card file to client memory
// -----------------------------------------------------------------------------
//
void CWimCallBack::OmaProvisioningRespL( CWimResponse* aTrId,
                                         TUint16 aFileLen,
                                         TUint8* aFile )
    {
    _WIMTRACE2(_L("WIM | WIMServer | CWimCallBack::OmaProvisioningRespL, aFileLen = %d"), aFileLen);

    RMessage2& message = aTrId->iMessage;
        
    TOmaProv omaProvisioning;
    omaProvisioning.iSize = aFileLen;

    TPckgBuf<TOmaProv> omaFilePckg( omaProvisioning );
    message.ReadL( 0, omaFilePckg );

    TPtr8 omaFilePtr( aFile, aFileLen, aFileLen );
    message.WriteL( 1, omaFilePtr );
    }

// -----------------------------------------------------------------------------
// CWimCallBack::CancelPinOperation
// Cancel pin operation by stop apdu sending
// -----------------------------------------------------------------------------
//
void CWimCallBack::CancelPinOperation()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCallBack::CancelPinOperation Begin"));
	gApdu->CancelApduSending();
	
	WIMI_APDUResp( 0, WIMI_ERR_UserCancelled, NULL, 0 );
	
	_WIMTRACE(_L("WIM | WIMServer | CWimCallBack::CancelPinOperation End"));
    }

// -----------------------------------------------------------------------------
// CWimCallBack::CancelWimInitialize
// Cancel Wim Initialize by stop apdu sending
// -----------------------------------------------------------------------------
//    
void CWimCallBack::CancelWimInitialize()
    {
	 _WIMTRACE(_L("WIM | WIMServer | CWimCallBack::CancelWimInitialize Begin"));
	gApdu->CancelApduSending();
	
	WIMI_APDUResp( 0, WIMI_ERR_UserCancelled, NULL, 0 );
	
	_WIMTRACE(_L("WIM | WIMServer | CWimCallBack::CancelWimInitialize End"));
    }
    
// -----------------------------------------------------------------------------
// CWimCallBack::DeletegApdu
//
// -----------------------------------------------------------------------------
//
void CWimCallBack::DeletegApdu()
    {
	if ( gApdu )
	    {
		delete gApdu;
		gApdu= NULL;
	    }
    }

// End of File
