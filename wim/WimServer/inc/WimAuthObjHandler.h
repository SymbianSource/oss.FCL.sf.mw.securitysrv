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
* Description:  WIM Authentication Object Handler
*
*/



#ifndef CWIMAUTHOBJHANDLER_H
#define CWIMAUTHOBJHANDLER_H

//  INCLUDES
#include <e32base.h>
#include <secdlg.h>
#include "WimResponse.h"
#include "WimUtilityFuncs.h"

// FORWARD DECLARATIONS
class CWimSecurityDlgHandler;
class CWimMemMgmt;
class CWimUtilityFuncs;

// CONSTANTS
_LIT( KUnblockingPinLabel, "Unblocking PIN" );

// CLASS DECLARATION

/**
*  WIM Authentication Object Handler.
*  Handles authentication objects, such as PIN.
*
*  @since Series60 2.1
*/
class CWimAuthObjHandler : public CActive
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CWimAuthObjHandler* NewL();
        
        /**
        * Destructor.
        */
        virtual ~CWimAuthObjHandler();

    public: // New functions
        
        /**
        * Get PIN count from the WIM card.
        * @param aMessage Encapsulates a client request.
        * @return void 
        */
        void GetPINCountL( const RMessage2& aMessage ) const;
        
        /**
        * Get reference list for PINs.
        * @param aMessage Encapsulates a client request.
        * @param aWimMgmt Pointer to WIM reference management class.
        * @return void
        */
        void GetPINRefListL( const RMessage2& aMessage,
                             CWimMemMgmt* aWimMgmt );

        /**
        * Get detailed information about the PIN.
        * @param aMessage Encapsulates a client request.
        * @return void
        */
        void GetPINInfoL( const RMessage2& aMessage ) ;
        
        /**
        * Get detailed information about the PINs.
        * @param aMessage Encapsulates a client request.
        * @return void
        */
        void GetPINsInfoL( const RMessage2& aMessage );
        
        /**
        * Verifies PIN number.
        * @since Series60 2.6
        * @param aMessage Encapsulates a client request.
        * @param aShowDialog Tells whether to show security dialog
                             for PIN query or not.
        * @return void
        */
        void VerifyPINRequestL( const RMessage2& aMessage, TBool aShowDialog );
        
        /**
        * Cancel Verifies PIN number.
        * @since Series60 3.0
        * @param aMessage Encapsulates a client request.
        * @return void
        */
        void CancelVerifyPin( const RMessage2& aMessage ); 
        
        /**
        * Verifies Disabled PIN status.
        * @since Series60 3.0
        * @param aMessage Encapsulates a client request.
        * @param aShowDialog Tells whether to show security dialog
                             for PIN query or not.
        * @return void
        */
        void VerifyDisabledPINRequestL( const RMessage2& aMessage );
        
        /**
        * Changes a PIN number.
        * @since Series60 2.6
        * @param aMessage Encapsulates a client request.
        * @return void
        */
        void ChangePINRequestL( const RMessage2& aMessage );
        
        /**
        * Cancel Changes a PIN number.
        * @since Series60 3.0
        * @param aMessage Encapsulates a client request.
        * @return void
        */
        void CancelChangePin( const RMessage2& aMessage ); 
        
        /**
        * Enables/disables PIN query.
        * @since Series60 2.6
        * @param aMessage Encapsulates a client request.
        * @return void
        */
        void EnablePINReqL( const RMessage2& aMessage );
        
        /**
        * Enables/disables PIN query.
        * @since Series60 3.0
        * @param aMessage Encapsulates a client request.
        * @return void
        */
        void CancelEnablePin( const RMessage2& aMessage ); 
       
        /**
        * Unblocks blocked PIN.
        * @since Series60 2.6
        * @param aMessage Encapsulates a client request.
        * @return void
        */
        void UnblockPinReqL( const RMessage2& aMessage );
        
        /**
        * Cancel Unblocks blocked PIN.
        * @since Series60 3.0
        * @param aMessage Encapsulates a client request.
        * @return void
        */
        void CancelUnblockPin( const RMessage2& aMessage ); 
        
        /**
        * Retrive authentication object's info
        * @since S60 3.2
        * @param aMessage Encapsulates a client request.
        * @return void
        */
        void RetrieveAuthObjectsInfo( const RMessage2& aMessage );
    
    private:

        /**
        * C++ default constructor.
        */
        CWimAuthObjHandler();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        /**
        * Continue Verify PIN request after PIN is asked. Calls WIMI.
        * @since Series60 2.6
        * @return void
        */
        void VerifyPINAskPIN();
        void ContinueVerifyPINReqL();

        /**
        * Continue Change PIN request after PIN is asked. Calls WIMI.
        * @since Series60 2.6
        * @return void
        */
        void ChangePINAskPIN();
        void ContinueChangePINReqL();

        /**
        * Continue Enable/Disable PIN request after PIN is asked. Calls WIMI.
        * @since Series60 2.6
        * @return void
        */
        void EnablePINAskPIN();
        void ContinueEnablePINReqL();

        /**
        * Continue Unblock PIN request after PIN is asked. Calls WIMI.
        * @since Series60 2.6
        * @return void
        */
        void UnblockPINAskPIN();
        void ContinueUnblockPinReqL();

        /**
        * From CActive. Asyncronous PIN query ready. Call ContinueXXX
        * function depending what we are requesting.
        */
        void RunL();

        /**
        * From CActive. Cancel asynchronous request.
        */
        void DoCancel();

        /**
        * From CActive. Handle leave from RunL()
        * @param aError leave code from RunL()
        */
        TInt RunError( TInt aError );

    private:    // Data
        // Enumerator for ongoing request
        enum TAuthObjectRequest
            {
            EVerifyPINAskPIN,
            EVerifyPINCallBackResponse,
            EChangePINAskPIN,
            EChangePINCallBackResponse,       
            EEnablePINAskPIN,
            EEnablePINCallBackResponse,    
            EDisablePINAskPIN,
            EDisablePINCallBackResponse,
            EUnblockPINAskPIN,
            EUnblockPINCallBackResponse,
            EShowPINBlocked,
            EShowCardIsRemoved,
            EWIMIError,
            EIdle
            };

        // Pointer to utility function class. Owned.
        CWimUtilityFuncs*       iWimUtilFuncs;
        // Pointer to security handler class. Owned.
        CWimSecurityDlgHandler* iWimSecDlg;
        // Authentication object ongoing request 
        TAuthObjectRequest      iRequest;
      
        // PIN value that user has given
        TPINValue               iPinValue;
        // Old PIN value in PIN change
        TPINValue               iOldPinValue;
        // New PIN value in Change PIN and Unblock PIN
        TPINValue               iNewPinValue;
        // Unblocking PIN
        TPINValue               iUnblockingPinValue;
        // Enable/Disable PIN query 
        TBool                   iEnablePinQuery;
        
        TBool                   iRetry;
        
        TBool                   iShowDialog;
        
        CWimResponse*           iResponseID;
        
        TWimReqTrId*            iTrId;
        
        TPINParams              iPinParams;
        
        TPINParams              iUnblockingPinParams;
        TBool                   iDisablePINBlocked; 
        TBool                   iUnblockPING;
        
        WIMI_Ref_pt             iPinGRef;
                 
    };

#endif      // CWIMAUTHOBJHANDLER_H  
            
// End of File
