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
* Description:  WIM Sign Text Handler
*
*/



#ifndef CWIMSIGNTEXTHANDLER_H
#define CWIMSIGNTEXTHANDLER_H

//  INCLUDES
#include <secdlg.h>
#include "WimResponse.h"
#include "WimUtilityFuncs.h"

// FORWARD DECLARATIONS
class CWimUtilityFuncs;
class CWimSecurityDlgHandler;

// CLASS DECLARATION

/**
*  WIM Sign Text Handler.
*  Handles Sign Text operations. 
*
*  @since Series60 2.6
*/
class CWimSignTextHandler : public CActive
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CWimSignTextHandler* NewL();
        
        /**
        * Destructor.
        */
        virtual ~CWimSignTextHandler();

    public: // New functions
        
        /**
        * Start the SignText operation.
        * @param aMessage Encapsulates a client request.
        * @return void 
        */
        void SignTextL( const RMessage2& aMessage );
      
    private:

        /**
        * C++ default constructor.
        */
        CWimSignTextHandler();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        /**
        * Continues signing operation after signing PIN has asked from user.
        * Calls WIMI_SignReq() method which take care of actual sign operation.
        * Response (signed text) to call is handled in CWimCallBack::SignResp().
        * @return void 
        */
        void ContinueSigningL();

        /**
        * Ask PIN from user. Shows PIN query dialog.
        * @return void 
        */
        void AskPin();

        /**
        * Get parameters for PIN. iKeyReferense has to be set before
        * this function is used.
        * @param aPinParams PIN parameters that has to be set
        * @return void 
        */
        void GetPinParamsL( TPINParams& aPinParams ) const;

        /**
        * Clean member data
        * @return void 
        */
        void CleanUp();

        /**
        * From CActive. RunL: Handles signing operation as state machine.
        */
        void RunL();

        /**
        * From CActive. DoCancel: Asyncronous request cancelled
        */
        void DoCancel();

        /**
        * From CActive. The active scheduler calls this function if this active
        * object's RunL() function leaves.
        * Handles necessary cleanup and completes request with
        * received error code.
        * @param aError Leave code
        * @return Error code to Active Scheduler, is always zero.
        */ 
        TInt RunError( TInt aError );

    private:    // Data

        // Enumerator for ongoing sign text state
        enum TSignTextState
            {
            EAskPin,
            ECallbackResponse,
            EShowPinBlocked,
            EShowCardIsRemoved,
            EWimiError,
            ESigningDone
            };

        // Pointer to utility function class. Owned.
        CWimUtilityFuncs*       iWimUtilFuncs;
        // Pointer to security handler class. Owned.
        CWimSecurityDlgHandler* iWimSecDlg;
        // PIN for signing
        TPINValue               iSigningPin;
        // Pointer to WimResponse for completing the client request. 
        // Deleted here or in callback function.
        CWimResponse*           iResponseID;
        // Transaction ID. This is deleted in CWimCallBack::SignResp() 
        // or here in case of WIMI error.
        TWimReqTrId*            iTrId;
        // Key reference. Owned.
        TAny*                   iKeyReference;
        // State of signing operation
        TSignTextState          iSigningState;
        // Buffer for data to be signed. Owned.
        HBufC8*                 iSigningDataBuf;
        // Pointer to data to be signer. Owned.
        TPtr8*                  iSigningDataPtr;
        // Flag to tell if PIN query is retry or not
        TBool                   iRetry;
        // PIN parameters
        TPINParams              iPinParams;
                
    };

#endif      // CWIMSIGNTEXTHANDLER_H  
            
// End of File
