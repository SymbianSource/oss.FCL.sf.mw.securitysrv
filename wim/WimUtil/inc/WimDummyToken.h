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
* Description:  Dummy Token and TokenType for Client/Server
*               information interchange
*
*/



#ifndef CWIMDUMMYTOKEN_H
#define CWIMDUMMYTOKEN_H

//  INCLUDES
#include <ct.h>

// FORWARD DECLARATIONS
class CWimDummyTokenType;


// CLASS DECLARATION

/**
*  Class for token handling.
*  Messaging between WimClient and WimServer requires a reference to token,
*  which cannot be a null reference. Because the use of actual token is not
*  required and because it would consume memory for no reason, dummy token is
*  used.
*
*  @lib WimUtil.lib
*  @since Series60 3.0
*/
class CWimDummyToken : public CBase, public MCTToken
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CWimDummyToken* NewL( MCTTokenType& aTokenType );
        
        /**
        * Destructor.
        */
        virtual ~CWimDummyToken();

    public: // New functions

        /**
        * Sets serial number to token
        * @param  Serian number to be set
        */
        void SetSerialNumber( TDesC& aSerialNumber );

    public: // Functions from base class MCTToken
        
        /**
        * Returns a reference to this token's label.
        * @return  A Reference to this token's label.
        */
        const TDesC& Label();
        
        /**
        * Returns the associated token type.
        * @return A reference to token's token type
        */
        MCTTokenType& TokenType();
        
        /**
        * Returns the token's handle which is TCTTokenHandle class.
        * @return tokens handle
        */
        TCTTokenHandle Handle();
        
        /** 
        * Returns the specified information string about the token.
        * @param aRequiredInformation (IN) Specifies the required information
        *        needed from the token. See value from TTokenInformation.
        * @return TDesC&  (OUT) Information string of the token, or KNullDesC,
        *         if token information cannot be given
        */
        const TDesC& Information( TTokenInformation aRequiredInformation );

        /**
        * Notifies the client when the token has been removed. Notify is 
        * given by changing the status code of given TRequestStatus.
        * The caller should give listener's TRequestStatus in pending state 
        * to this function.
        * Note: this is not an asynchronous call, it only stores the
        * address of given TRequestStatus for later notifications.
        * @param  aStatus (IN/OUT) Async. request status.
        *         KErrPending, token is on it's place
        *         KErrCancel, caller has cancelled to notify token removal
        *         KErrHardwareNotAvailable, token is removed
        *         Any other system error code
        * @return void
        */
        void NotifyOnRemoval( TRequestStatus& aStatus );

        /**
        * Cancels the NotifyOnRemoval request.
        * @return void
        */
        void CancelNotify();

                /**
        * Returns a reference to a variable to be used as a reference counter
        * @return  A reference to reference counter
        */
        TInt& ReferenceCount();

    protected: // Functions from base class MCTToken

        /**
        * Returns a reference to pointer of an interface object.
        * The caller owns all the parameters.
        * @param  aRequiredInterface (IN) The UID of the interface that 
        *         should be returned
        * @param  aReturnedInterface (OUT). This will be set to a pointer to 
        *         the returned interface on success and
        *         to NULL if it isn't supported by this token
        * @param  aStatus (IN/OUT) Async. request status.
        *         KErrNone, if all ok and interface returned
        *         KErrCancel, if call canceled
        *         KErrNotSupported, if interface not supported
        *         KErrHardwareNotAvailable, if Wim card suddenly removed
        *         Any other system wide error code (e.g. KErrNoMemory)
        * @return void
        */
        void DoGetInterface( TUid aRequiredInterface,
                             MCTTokenInterface*& aReturnedInterface, 
                             TRequestStatus& aStatus );
        
        /**
        * Cancels DoGetInterface operation.
        * @return Boolean value: true, if cancel is appropriate, false if
        *         cancel is not appropriate
        */
        TBool DoCancelGetInterface();

    private:

        /**
        * C++ default constructor.
        */
        CWimDummyToken( MCTTokenType& aTokenType );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private:    // Data

        MCTTokenType& iTokenType;
        // A variable needed for reference counting
        TInt          iCount;
        // Serial number of the token
        HBufC*        iSerialNumber;
         
    };

class CWimDummyTokenType : public CCTTokenType
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CWimDummyTokenType* NewL();
        
        /**
        * Destructor.
        */
        virtual ~CWimDummyTokenType();

    public: // Functions from base class MCTTokenType
        
        /**
        * Lists all the tokens of this type. Caller owns all parameters and
        * is responsible to destroy all objects in the received RCPointerArray.
        * @param  aTokens (OUT) The returned tokens are added to this array.
        * @param  aStatus (IN/OUT). Asyncronous return status: 
        *         KErrNone, if no errors occurred
        *         KErrNotFound, if Wim information not found
        *         KErrCancel, if list operation cancelled
        *         Any other system wide error code
        * @return void
        */
        void List( RCPointerArray<HBufC>& aTokens, 
                   TRequestStatus& aStatus );

        /**
        * Cancel a list operation.
        * @return void
        */
        void CancelList();

        /**
        * Opens a specified token with token info.
        * The caller owns all the parameters.
        * @param  aTokenInfo (IN) The info of the required token (token label)
        * @param  aToken (OUT) The returned token.
        * @param  aStatus (IN/OUT) Asynchronous return status.  
        *         KErrNone, if token opening succeeded
        *         KErrNotFound, if token not found by aTokenInfo
        *         Any other system wide error code
        * @return void
        */
        void OpenToken( const TDesC& aTokenInfo, 
                        MCTToken*& aToken,
                        TRequestStatus& aStatus );
    
        /**
        * Opens a specified token with handle
        * The caller owns all the parameters.
        * @param  aHandle (IN) The handle of the required token
        * @param  aToken (OUT) The returned token.
        * @param  aStatus (IN/OUT) Asynchronous return status.  
        *         KErrNone, if token opening succeeded
        *         KErrNotFound, if token not found by aHandle
        *         Any other system wide error code
        * @return void
        */
        void OpenToken( TCTTokenHandle aHandle, 
                        MCTToken*& aToken,
                        TRequestStatus& aStatus );
    
         /**
        * Cancel an OpenToken operation.
        * @return void
        */
        void CancelOpenToken();      

    private: // Constructors

        /**
        * Second phase constructor
        */
        void ConstructL();

        /**
        * Default constructor
        */
        CWimDummyTokenType();

    };

#endif      // CWIMDUMMYTOKEN_H   
            
// End of File
