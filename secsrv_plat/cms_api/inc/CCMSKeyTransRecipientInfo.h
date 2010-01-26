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
* Description:  ?Description
*
*/



#ifndef CCMSKeyTransRecipientInfo_H
#define CCMSKeyTransRecipientInfo_H

#include "CCMSRecipientInfo.h"

// FORWARD DECLARATION
class CCMSIssuerAndSerialNumber;

// CONSTANTS
const TInt KCMSKeyTransRecipientInfoIssuerAndSerialNumberVersion = 0;
const TInt KCMSKeyTransRecipientInfoSubjectKeyIdentifierVersion = 2;

// CLASS DECLARATION
/**
*  KeyTransRecipientInfo type module
*
*  ASN.1 definition:
*    KeyTransRecipientInfo ::= SEQUENCE {
*        version CMSVersion,  -- always set to 0 or 2
*        rid RecipientIdentifier,
*        keyEncryptionAlgorithm KeyEncryptionAlgorithmIdentifier,
*        encryptedKey EncryptedKey }
*
*    RecipientIdentifier ::= CHOICE {
*        issuerAndSerialNumber IssuerAndSerialNumber,
*        subjectKeyIdentifier [0] SubjectKeyIdentifier }
*
*  @lib cms.lib
*  @since 2.8
*/
class CCMSKeyTransRecipientInfo : public CCMSRecipientInfo
    {
    public:  // Constructors and destructor
		/**
        * Two-phased constructor. Leaves the object in cleanup stack.
        */
        IMPORT_C static CCMSKeyTransRecipientInfo* NewLC();

        /**
        * Two-phased constructor.
        */
        IMPORT_C static CCMSKeyTransRecipientInfo* NewL();

        /**
        * Two-phased constructor. Copies of the parameters are made.
        *
        * @param aIssuerAndSerialNumber rid (RecipientIdentifier) value
        * @param aKeyEncryptionAlgorithm KeyEncryptionAlgorithmIdentifier
        * @param aEncryptedKey EncryptedKey value
        */
        IMPORT_C static CCMSKeyTransRecipientInfo* NewL(
            const CCMSIssuerAndSerialNumber& aIssuerAndSerialNumber,
            const CCMSX509AlgorithmIdentifier& aKeyEncryptionAlgorithm,
            const TDesC8& aEncryptedKey );

        /**
        * Two-phased constructor. Copies of the parameters are made.
        * Leaves the object in cleanup stack.
        *
        * @param aSubjectKeyIdentifier rid (RecipientIdentifier) value
        * @param aKeyEncryptionAlgorithm KeyEncryptionAlgorithmIdentifier
        * @param aEncryptedKey EncryptedKey value
        */
        IMPORT_C static CCMSKeyTransRecipientInfo* NewLC(
            const TDesC8& aSubjectKeyIdentifier,
            const CCMSX509AlgorithmIdentifier& aKeyEncryptionAlgorithm,
            const TDesC8& aEncryptedKey );
        
        /**
        * Two-phased constructor. Copies of the parameters are made.
        *
        * @param aSubjectKeyIdentifier rid (RecipientIdentifier) value
        * @param aKeyEncryptionAlgorithm KeyEncryptionAlgorithmIdentifier
        * @param aEncryptedKey EncryptedKey value
        */
        IMPORT_C static CCMSKeyTransRecipientInfo* NewL(
            const TDesC8& aSubjectKeyIdentifier,
            const CCMSX509AlgorithmIdentifier& aKeyEncryptionAlgorithm,
            const TDesC8& aEncryptedKey );


        /**
        * Destructor.
        */
        virtual ~CCMSKeyTransRecipientInfo();

    public: // New functions
        /**
        * Getter for EncryptedKey
        * @since 2.8
        * @return EncryptedKey
        */
		IMPORT_C const TDesC8& EncryptedKey() const;

        /**
        * Getter for IssuerAndSerialNumber. Ownership is not transferred.
        * @since 2,8
        * @return IssuerAndSerialNumber or NULL if rid is not
        * IssuerAndSerialNumber
        */
		IMPORT_C const CCMSIssuerAndSerialNumber* IssuerAndSerialNumber() const;

        /**
        * Getter for subjectKeyIdentifier. Ownership is not transferred.
        * @since 2,8
        * @return subjectKeyIdentifier or NULL if rid is not
        * subjectKeyIdentifier.
        */
		IMPORT_C const TDesC8* SubjectKeyIdentifier() const;

		/**
        * Setter for EncryptedKey. Creates a copy of the parameters.
        * @since 2.8
        * @param aEncryptedKey EncryptedKey value.
        */
		IMPORT_C void SetEncryptedKeyL(
            const TDesC8& aEncryptedKey );

		/**
        * Setter for IssuerAndSerialNumber. Creates a copy of the parameters.
        * Note that any existing subjectKeyIdentifier value is
        * deleted. Also version is set to 0.
        * @since 2.8
        * @param aIssuerAndSerialNumber IssuerAndSerialNumber value.
        */
		IMPORT_C void SetIssuerAndSerialNumberL(
            const CCMSIssuerAndSerialNumber& aIssuerAndSerialNumber );

		/**
        * Setter for subjectKeyIdentifier. Creates a copy of the parameters.
        * Note that any existing IssuerAndSerialNumber value is
        * deleted. Also version is set to 2.
        * @since 2.8
        * @param aSubjectKeyIdentifier SubjectKeyIdentifier value.
        */
		IMPORT_C void SetSubjectKeyIdentifierL(
            const TDesC8& aSubjectKeyIdentifier );

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
            const CCMSIssuerAndSerialNumber& aIssuerAndSerialNumber,
            const CCMSX509AlgorithmIdentifier& aKeyEncryptionAlgorithm,
            const TDesC8& aEncryptedKey );

       /**
        * Protected construction to allow derivation
        */
        IMPORT_C void ConstructL(
            const TDesC8& aSubjectKeyIdentifier,
            const CCMSX509AlgorithmIdentifier& aKeyEncryptionAlgorithm,
            const TDesC8& aEncryptedKey );

        /**
        * C++ constructor.
        */
        IMPORT_C CCMSKeyTransRecipientInfo( TInt aVersion );

    private:    // Data
        // EncryptedKey, owned
        TDesC8* iEncryptedKey;
        // IssuerAndSerialNumber, owned
        CCMSIssuerAndSerialNumber* iIssuerAndSerialNumber;
        // RecipientEncryptedKey, owned
        TDesC8* iSubjectKeyIdentifier;

    };

#endif      // CCMSKeyTransRecipientInfo_H

// End of File
