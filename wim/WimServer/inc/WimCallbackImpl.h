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
* Description:  A class with static operations that provides implementation
*               for the WIM callback functions. Used by the WIM server.
*
*/


#ifndef CWIMCALLBACK_H
#define CWIMCALLBACK_H

//  INCLUDES
#include <e32base.h>
#include "Wimi.h"       //WIMI definitions

// FORWARD DECLARATIONS
class CWimResponse;

// CLASS DECLARATION

/**
*  WIM Callback implementation.
*  Static callback funtions.
*
*  @since Series60 2.1
*/
class CWimCallBack : public CBase
    {
    public:

        /**
        * Respond to InitOK operation
        * @param aStatus - WIMI status
        * @return void
        */
        static void InitOkResp( WIMI_STAT aStatus );
  
        /**
        * Response to CloseDown request
        * @param aStatus - WIMI status
        * @return void
        */
        static void CloseDownResp( WIMI_STAT aStatus );

        /**
        * Complete response
        * @param aTrId - transaction id
        * @param aStatus - WIMI status
        * @return void
        */
        static void CompleteResponse( WIMI_TransactId_t aTrId,
                                      WIMI_STAT aStatus );

        /* ------------ Digital signature ----------------------------------- */
        /**
        * Respond to the digital signature operation. Completes client
        * request
        * @since Series60 2.6
        * @param aTrId - transaction id
        * @param aStatus - operation status 
        * @param aSignLen - signature length
        * @param aSign - signature 
        * @return void
        */
        static void SignResp( WIMI_TransactId_t aTrId,
                              WIMI_STAT aStatus,
                              TUint8 aSignLen,
                              TUint8* aSign );

        /**
        * Write signed text to client's memory area. Called by SignResp().
        * @since Series60 2.6
        * @param aTrId - transaction id
        * @param aSignLen - signature length
        * @param aSign - signature 
        * @return void
        */
        static void SignRespL( CWimResponse* aTrId,
                               TUint8 aSignLen,
                               TUint8* aSign );

        /**
        * Set request status of SignTextHandler. Used when continuing signing
        * operation if wrong PIN entered.
        * @since Series60 2.6
        * @param aStatus - RequestStatus of SignTextHandler
        * @return void
        */
        static void SetSignTextRequestStatus( TRequestStatus* aStatus );

        /* ------------ Certificate management ------------------------------ */
        /** 
        * Respond to the certificate fetch operation
        * @param aTrId - transaction id
        * @param aStatus - operation status 
        * @param aCertRef - the reference to certificate that was fetched
        * @param aCertLen - certificate content length
        * @param aCert - certificate content 
        * @return void
        */
        static void CertificateResp( WIMI_TransactId_t aTrId,
                                     WIMI_STAT aStatus,
                                     WIMI_Ref_t* aCertRef,
                                     TUint16 aCertLen,
                                     TUint8* aCert );

        /**
        * Respond to the certificate delete operation
        * @since Series60 2.6
        * @param aTrId - transaction id
        * @param aStatus - operation status
        * @return void
        */
        static void CertificateDeleteResp( WIMI_TransactId_t aTrId,
                                           WIMI_STAT aStatus );

        /**
        * Respond to the certificate store operation
        * @since Series60 2.6
        * @param aTrId - transaction id
        * @param aStatus - operation status
        * @param aCertRef - the reference to certificate that was stored
        * @return void
        */
        static void CertificateStoreResp( WIMI_TransactId_t aTrId,
                                          WIMI_STAT aStatus,
                                          WIMI_Ref_t* aCertRef );


        /* ------------ PIN management -------------------------------------- */
        /**
        * Response to VerifyPIN request
        * @since Series60 2.6
        * @param aTrId - transaction id
        * @param aStatus - operation status
        * @return void
        */
        static void VerifyPINResp( WIMI_TransactId_t aTrId, WIMI_STAT aStatus );

        /**
        * Response to ChangePIN request
        * @since Series60 2.6
        * @param aTrId - transaction id
        * @param aStatus - operation status
        * @return void
        */
        static void ChangePINResp( WIMI_TransactId_t aTrId, WIMI_STAT aStatus );

        /**
        * Response to UnblockPIN request
        * @since Series60 2.6
        * @param aTrId - transaction id
        * @param aStatus - operation status
        * @return void
        */
        static void UnblockPINResp( WIMI_TransactId_t aTrId,
                                    WIMI_STAT aStatus );

        /**
        * Response to EnablePIN request
        * @since Series60 2.6
        * @param aTrId - transaction id
        * @param aStatus - operation status
        * @return void
        */
        static void EnablePINResp( WIMI_TransactId_t aTrId, WIMI_STAT aStatus );

        /* ------------ Hardware driver support ----------------------------- */
        /**
        * Respond to CardInserted operation
        * @param aReader - Reader ID
        * @param aStatus - Status of operation
        * @param aWIMRef - Reference to WIMI
        * @return void
        */
        static void CardInsertedResp( TUint8 aReader,
                                      WIMI_STAT aStatus,
                                      WIMI_Ref_t* aWIMRef );

        /**
        * Open connection to card
        * @param aReader - Reader
        * @return KWimStatusOK/KWimStatusIOError
        */
        static TUint8 Open( TUint8 aReader );

        /**
        * Close connection to the card
        * @param aReader - Reader
        * @return KWimStatusOK
        */
        static TUint8 Close( TUint8 aReader );

        /**
        * APDU Request
        * @param aReader - Reader
        * @param aApdu - APDU
        * @param aApduLen - ADDU length
        * @return void
        */
        static void APDUReq( TUint8 aReader, TUint8* aApdu, TUint16 aApduLen );

        /**
        * Get ATR bytes request
        * @param aReader - Reader
        * @return void
        */
        static void GetATRReq( TUint8 aReader );

        /**
        * Get reader list reguest
        * @return void
        */
        static void GetReaderListReq();

        /**
        * Read content of SIM file.
        * @param aReaderId -  Reader ID
        * @param aPath - Path to the file to read
        * @param aOffset - Offset from the beginning of the file
        * @param aSize - Amount of data to read, aSize=0 means that whole
        *                    file must be read
        * @return void
        */
        static void ReadSimFileReq( TUint8 aReaderId,
                                    TUint8* aPath,
                                    TUint16 aOffset,          
                                    TUint16 aSize );

        /**
        * Response for OMA Smart Card Provisioning file request.
        * @param aTrId - Transaction ID, contains message information
        * @param aStatus - Operation status 
        * @param aFileLen - Provisioning file length
        * @param aFile - Provisioning file content
        * @return void
        */
        static void OmaProvisioningResp( WIMI_TransactId_t aTrId,
                                         WIMI_STAT aStatus,
                                         TUint16 aFileLen,
                                         TUint8 *aFile );
                                         
        /**
        * Cancel pin operation
        * Stop the APDU sending
        */                                 
        static void CancelPinOperation();
        
        /**
        * Cancel Wim Initialize 
        * Stop the APDU sending
        */
        static void CancelWimInitialize();
        
        /**
        * This function is called by CWimServer destructor,
        * to delete the gApdu object.
        */
        static void CWimCallBack::DeletegApdu();
                             
    private:

        /** 
        * Respond to the certificate fetch operation
        * @param aTrId - transaction id
        * @param aStatus - operation status 
        * @param aWimiRef - the reference to WIMI
        * @param aCertLen - certificate content length
        * @param aCert - certificate content 
        * @return void
        */
        static void CertificateRespL( CWimResponse* aTrId,
                                      WIMI_STAT aStatus,
                                      WIMI_Ref_t* aWimiRef,
                                      TUint16 aCertLen,
                                      TUint8* aCert );

        /**
        * Read content of SIM file. Leaving function, 
        * leave is trapped in ReadSimFileReg
        * @param aReaderId - Reader ID
        * @param aPath - Path to the file to read
        * @param aOffset - Offset from the beginning of the file
        * @param aSize - Amount of data to read, aSize=0 means that whole
        *                file must be read
        * @return void
        */
        static void DoReadSimFileReqL( TUint8 aReaderId,
                                       TUint8* aPath,
                                       TUint16 aOffset,          
                                       TUint16 aSize );

        /**
        * Copy OMA Provisioning file to client memory
        * @param aTrId - Transaction ID, contains message information
        * @param aFileLen - Provisioning file length
        * @param aFile - Provisioning file content
        * @return void
        */
        static void OmaProvisioningRespL( CWimResponse* aTrId,
                                          TUint16 aFileLen,
                                          TUint8* aFile );

    };

#endif      // CWIMCALLBACK_H

// End of File
