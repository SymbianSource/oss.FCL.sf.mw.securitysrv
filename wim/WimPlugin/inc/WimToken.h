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
* Description:  A token represents one instance of a particular kind of 
*                cryptographic module
*
*/



#ifndef CWIMTOKEN_H
#define CWIMTOKEN_H

//  INCLUDES

#include "WimSecModule.h"
#include <e32base.h>
#include <ct.h>

// FORWARD DECLARATION

class CWimTokenListener;
class MCTCertStore;
class MCTWritableCertStore;
class MCTKeyStore;
class MCTAuthenticationObjectList;

// CLASS DECLARATION

/**
*  CWimToken is a class which implements one cryptographic interface
*
*
*  @lib   WimPlugin
*  @since Series60 2.1
*/
class CWimToken : public CBase, public MCTToken
    {
	public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        * @param aWimSecModule (IN) Current security module
        * @param aTokenType (IN) Created token's token type
        * @return  An instance of this class
        */
        static CWimToken* NewL( CWimSecModule* aWimSecModule, 
										 MCTTokenType& aTokenType );
        
        /**
        * Destructor.
        */
        virtual ~CWimToken();

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
        const TDesC& Information( 
                                TTokenInformation aRequiredInformation );

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
		*		  Any other system error code
		* @return void
		*/
		void NotifyOnRemoval( TRequestStatus& aStatus );

		/**
		* Cancels the NotifyOnRemoval request.
		* @return void
		*/
		void CancelNotify();

	protected: // Functions from base class MCTToken

        /**
        * Returns a reference to pointer of an interface object.
		* The caller owns all the parameters.
        * @param  aRequiredInterface (IN) The UID of the interface that 
		*		  should be returned
		* @param  aReturnedInterface (OUT). This will be set to a pointer to 
		*		  the returned interface on success and
		*		  to NULL if it isn't supported by this token
        * @param  aStatus (IN/OUT) Async. request status.
		*		  KErrNone, if all ok and interface returned
		*		  KErrCancel, if call canceled
		*		  KErrNotSupported, if interface not supported
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

	public: // New functions

		/**
		* Returns a pointer to current security module.
		* @return A pointer to current security module
		*/
		CWimSecModule* WimSecModule();

		/**
		* Returns a pointer to token listener
		* @return A pointer to token listener
		*/
		CWimTokenListener* TokenListener();

		/**
		* Returns a boolean indicating if token is removed
		* @return True, if token removed, false if not
		*/
		TBool TokenRemoved();

    protected:  // New functions
        
        /**
        * Returns a reference to a variable to be used as a reference counter
        * @return  A reference to reference counter
        */
        TInt& ReferenceCount();

	private: // Constructors
    
        /**
        * Default constructor.
        * @param aWimSecModule (IN) Current security module
        * @param aTokenType (IN) Created token's token type
        */
        CWimToken( CWimSecModule* aWimSecModule, 
				   MCTTokenType& aTokenType );

        void ConstructL();

	private: // Own function

        /**
        * Returns as a parameter a reference to pointer of an interface object.
        * @param  aRequiredInterface (IN) The UID of the interface that 
		*		  should be returned
		* @param  aReturnedInterface (OUT). This will be set to a pointer to 
		*		  the returned interface on success and
		*		  to NULL if it isn't supported by this token
        * @param  aStatus (IN/OUT) Async. request status.
		*		  KErrNone, if all ok and interface returned
        *         KErrHardwareNotAvailable, if Wim card suddenly removed
        *         Any other system wide error code (e.g. KErrNoMemory)
		* @return void
        */
        void MakeInterfaceL( TUid aRequiredInterface,
                             MCTTokenInterface*& aReturnedInterface );
    private:    
		
		// A pointer to current security module.
		// This class don't own the pointed object.
		CWimSecModule* iWimSecModule;

		// Reference to the token type of this token. 
		// This class don't own the referenced object.
		MCTTokenType& iTokenType;

		// A pointer to label buffer that contains the name of this token.
		// This class owns the pointed object.
		HBufC* iLabel;

		// A variable needed for reference counting
		TInt iCount;

		// Used in token notify on removal
		CWimTokenListener* iWimTokenListener;

  		// A pointer to certificate store
		// Ownership of pointed object is transferred to caller
		MCTCertStore* iCertStoreIf;

		// A pointer to writable certificate
		// Ownership of pointed object is transferred to caller
		MCTWritableCertStore* iCertStoreWritableIf;

		// A pointer to Wim key store
		// Ownership of pointed object is transferred to caller
		MCTKeyStore* iKeyStoreIf;

		// A pointer to Wim authentication object list
		// Ownership of pointed object is transferred to caller
		MCTAuthenticationObjectList* iAuthObjectListIf;

    };

        /**
        * Helper class to filter token types
        */

class TCurrentTokenType : public MCTTokenTypeFilter
    {

    public: // From base class MCTTokenTypeFilter
    
        /**
        * Sets value to given token type info.
        * @param  aTokenType  (IN) Info class for token type
        * @return ETrue if current token type is same as given token type
        *         EFalse, if not
        */
        virtual TBool Accept( const CCTTokenTypeInfo& aTokenType ) const;

    };


#endif      // CWIMTOKEN_H   
            
// End of File
