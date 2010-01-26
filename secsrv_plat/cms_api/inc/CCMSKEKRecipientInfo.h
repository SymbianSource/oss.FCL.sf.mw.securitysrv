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
* Description: 
*
*/



#ifndef CCMSKEKRecipientInfo_H
#define CCMSKEKRecipientInfo_H

#include "CCMSRecipientInfo.h"

// FORWARD DECLARATION
class CCMSKeyIdentifier;

// CONSTANTS
const TInt KCMSKEKRecipientInfoVersion = 4;
const TTagType KCMSKEKRecipientInfoTag = 2;

// CLASS DECLARATION
/**
*  KEKRecipientInfo type module
*
*  ASN.1 definition:
*    KEKRecipientInfo ::= SEQUENCE {
*        version CMSVersion,  -- always set to 4
*        kekid KEKIdentifier,
*        keyEncryptionAlgorithm KeyEncryptionAlgorithmIdentifier,
*        encryptedKey EncryptedKey }
*
*  @lib cms.lib
*  @since 2.8
*/
class CCMSKEKRecipientInfo : public CCMSRecipientInfo
    {
    public:  // Constructors and destructor
		/**
        * Two-phased constructor. Leaves the object in cleanup stack.
        */
        IMPORT_C static CCMSKEKRecipientInfo* NewLC();

        /**
        * Two-phased constructor.
        */
        IMPORT_C static CCMSKEKRecipientInfo* NewL();

        /**
        * Two-phased constructor. Copies of the parameters are made.
        *
        * @param aKekid KEKIdentifier
        * @param aKeyEncryptionAlgorithm KeyEncryptionAlgorithmIdentifier
        * @param aEncryptedKey EncryptedKey value
        */
        IMPORT_C static CCMSKEKRecipientInfo* NewL(
            const CCMSKeyIdentifier& aKekid,
            const CCMSX509AlgorithmIdentifier& aKeyEncryptionAlgorithm,
            const TDesC8& aEncryptedKey );

        /**
        * Destructor.
        */
        virtual ~CCMSKEKRecipientInfo();

    public: // New functions
        /**
        * Getter for EncryptedKey
        * @since 2.8
        * @return EncryptedKey
        */
		IMPORT_C const TDesC8& EncryptedKey() const;

        /**
        * Getter for KEKIdentifier
        * @since 2,8
        * @return KEKIdentifier
        */
		IMPORT_C const CCMSKeyIdentifier& Kekid() const;

		/**
        * Setter for EncryptedKey. Creates a copy of the parameters.
        * @since 2.8
        * @param aEncryptedKey EncryptedKey value.
        */
		IMPORT_C void SetEncryptedKeyL(
            const TDesC8& aEncryptedKey );

		/**
        * Setter for KEKIdentifier
        * @since 2.8
        * @param aKekid KEKIdentifier
        */
		IMPORT_C void SetKekidL(
            const CCMSKeyIdentifier& aKekid );

    public: // Functions from base classes

	 	/**
        * From MCMSModule
        * @since 2.8
        * @param aRawData raw-data to be parsed in this instance
        */
        void DecodeL( const TDesC8& aRawData );

	 	/**
        * From MCMSModule
        * @since 2.8
        * @return Returns ASN1 encoder and leaves it in CleanupStack
        */
        CASN1EncBase* EncoderLC() const;
        
        /**
         * From CCMSRecipientInfo
         * @since 2.8
         * @return Returns ASN1 encoder with explicit tag 1 and leaves
         * it in CleanupStack.
         */
        CASN1EncBase* TaggedEncoderLC() const;
        
    protected:  // New functions

	   /**
        * Protected construction to allow derivation
        */
        IMPORT_C void ConstructL(
            const CCMSKeyIdentifier& aKekid,
            const CCMSX509AlgorithmIdentifier& aKeyEncryptionAlgorithm,
            const TDesC8& aEncryptedKey );

        /**
        * C++ constructor.
        */
        IMPORT_C CCMSKEKRecipientInfo( );

    private:    // Data
        // EncryptedKey, owned
        TDesC8* iEncryptedKey;
        // KEKIdentifier, owned
        CCMSKeyIdentifier* iKekid;

    };

#endif      // CCMSKEKRecipientInfo_H

// End of File
