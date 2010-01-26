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
* Description:  CMS KeyAgreeRecipientInfo type
*
*/



#ifndef CCMSKeyAgreeRecipientInfo_H
#define CCMSKeyAgreeRecipientInfo_H

// INCLUDES
#include "CCMSRecipientInfo.h"

// FORWARD DECLARATION
class CCMSOriginatorIdentifierOrKey;
class CCMSRecipientEncryptedKey;

//Extension
class MCMSKeyAgreeRecipientInfo;

// CONSTANTS
const TInt KCMSKeyAgreeRecipientInfoVersion = 3;
const TTagType KCMSKeyAgreeRecipientInfoTag = 1;

// CLASS DECLARATION
/**
*  KeyAgreeRecipientInfo type module
*
*  ASN.1 definition:
*    KeyAgreeRecipientInfo ::= SEQUENCE {
*        version CMSVersion,  -- always set to 3
*        originator [0] EXPLICIT OriginatorIdentifierOrKey,
*        ukm [1] EXPLICIT UserKeyingMaterial OPTIONAL,
*        keyEncryptionAlgorithm KeyEncryptionAlgorithmIdentifier,
*        recipientEncryptedKeys RecipientEncryptedKeys }
*
*    RecipientEncryptedKeys ::= SEQUENCE OF RecipientEncryptedKey
*
*    UserKeyingMaterial ::= OCTET STRING
*
*  @lib cms.lib
*  @since 3.0
*/
class CCMSKeyAgreeRecipientInfo : public CCMSRecipientInfo
    {
    public:  // Constructors and destructor
		/**
        * Two-phased constructor. Leaves the object in cleanup stack.
        */
        IMPORT_C static CCMSKeyAgreeRecipientInfo* NewLC();

        /**
        * Two-phased constructor.
        */
        IMPORT_C static CCMSKeyAgreeRecipientInfo* NewL();
        
        /**
        * Two-phased constructor. Copies of the parameters are made.
        * @param aOriginator OriginatorIdentifierOrKey value
        * @param aKeyEncryptionAlgorithm KeyEncryptionAlgorithmIdentifier value
        * @param aRecipientEncryptedKeys RecipientEncryptedKeys value
        */
        IMPORT_C static CCMSKeyAgreeRecipientInfo* NewL(
            const CCMSOriginatorIdentifierOrKey& aOriginator,
            const CCMSX509AlgorithmIdentifier& aKeyEncryptionAlgorithm,
            const CArrayPtr< CCMSRecipientEncryptedKey >& aRecipientEncryptedKeys );

        /**
        * Two-phased constructor. Copies of the parameters are made.
        * @param aOriginator OriginatorIdentifierOrKey value
        * @param aUkm UserKeyingMaterial value
        * @param aKeyEncryptionAlgorithm KeyEncryptionAlgorithmIdentifier value
        * @param aRecipientEncryptedKeys RecipientEncryptedKeys value
        */
        IMPORT_C static CCMSKeyAgreeRecipientInfo* NewL(
            const CCMSOriginatorIdentifierOrKey& aOriginator,
            const TDesC8& aUkm,
            const CCMSX509AlgorithmIdentifier& aKeyEncryptionAlgorithm,
            const CArrayPtr< CCMSRecipientEncryptedKey >& aRecipientEncryptedKeys );

        /**
        * Destructor.
        */
        virtual ~CCMSKeyAgreeRecipientInfo();

    public: // New functions

        /**
        * Getter for Originator
        * @since 3.0
        * @return OriginatorIdentifierOrKey
        */
		IMPORT_C const CCMSOriginatorIdentifierOrKey& Originator() const;

        /**
        * Getter for RecipientEncryptedKeys
        * @since 3.0
        * @return array of RecipientEncryptedKeys
        */
		IMPORT_C const CArrayPtr< CCMSRecipientEncryptedKey >&
        RecipientEncryptedKeys() const;

        /**
        * Getter for UserKeyingMaterial. Ownership is not transferred.
        * @since 2,8
        * @return UserKeyingMaterial or NULL
        */
		IMPORT_C const TDesC8* UserKeyingMaterial() const;

		/**
        * Setter for Originator. Creates a copy of the parameters.
        * @since 3.0
        * @param aOriginator OriginatorIdentifierOrKey
        */
		IMPORT_C void SetOriginatorL(
            const CCMSOriginatorIdentifierOrKey& aOriginator );

        /**
        * Setter for RecipientEncryptedKeys. Creates a copy of the parameters.
        * @since 3.0
        * @param aRecipientEncryptedKeys Array of RecipientEncryptedKeys
        */
		IMPORT_C void SetRecipientEncryptedKeysL(
            const CArrayPtr< CCMSRecipientEncryptedKey >& aRecipientEncryptedKeys );

		/**
        * Setter for UserKeyingMaterial. Creates a copy of the parameters.
        * @since 3.0
        * @param aUkm UserKeyingMaterial
        */
		IMPORT_C void SetUserKeyingMaterialL(
            const TDesC8& aUkm );

    public: // Functions from base classes

	 	/**
        * From MCMSModule
        * @since 3.0
        * @param aRawData raw-data to be parsed in this instance
        */
        void DecodeL( const TDesC8& aRawData );

	 	/**
        * From MCMSModule
        * @since 3.0
        * @return Returns ASN1 encoder and leaves it in CleanupStack
        */
        CASN1EncBase* EncoderLC() const;

        /**
         * From CCMSRecipientInfo
         * @since 3.0
         * @return Returns ASN1 encoder with explicit tag 1 and leaves
         * it in CleanupStack.
         */
        CASN1EncBase* TaggedEncoderLC() const;
         
          
        
    protected:  // New functions

        /**
         * Protected constructor to allow derivation
         */
        IMPORT_C void ConstructL( );

	   /**
        * Protected construction to allow derivation
        */
        IMPORT_C void ConstructL(
            const CCMSOriginatorIdentifierOrKey& aOriginator,
            const CCMSX509AlgorithmIdentifier& aKeyEncryptionAlgorithm,
            const CArrayPtr< CCMSRecipientEncryptedKey >& aRecipientEncryptedKeys );

       /**
        * Protected construction to allow derivation
        */
        IMPORT_C void ConstructL(
            const CCMSOriginatorIdentifierOrKey& aOriginator,
            const TDesC8& aUkm,
            const CCMSX509AlgorithmIdentifier& aKeyEncryptionAlgorithm,
            const CArrayPtr< CCMSRecipientEncryptedKey >& aRecipientEncryptedKeys );

        /**
        * C++ constructor.
        */
        IMPORT_C CCMSKeyAgreeRecipientInfo( );

    private:    // Data
        // originator, owned
        CCMSOriginatorIdentifierOrKey* iOriginator;

        // keyEncryptionAlgorithm, owned
        CCMSX509AlgorithmIdentifier* iKeyEncryptionAlgorithm;

        // recipientEncryptedKeys, owned
        CArrayPtr< CCMSRecipientEncryptedKey >* iRecipientEncryptedKeys;

        // UserKeyingMaterial, owned
        TDesC8* iUserKeyingMaterial;

        // Reserved for extensions
        MCMSKeyAgreeRecipientInfo* iReserved;
        
    };

#endif      // CCMSKeyAgreeRecipientInfo_H

// End of File
