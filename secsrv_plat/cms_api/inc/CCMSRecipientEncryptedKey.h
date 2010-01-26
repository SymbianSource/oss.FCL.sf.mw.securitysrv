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



#ifndef CCMSRecipientEncryptedKey_H
#define CCMSRecipientEncryptedKey_H

#include "CCMSSequence.h"

// FORWARD DECLARATIONS
class CCMSIssuerAndSerialNumber;
class CCMSKeyIdentifier;

// CLASS DECLARATION
/**
*  RecipientEncryptedKey type module
*
*  ASN.1 definition:
*  RecipientEncryptedKey ::= SEQUENCE {
*        rid KeyAgreeRecipientIdentifier,
*        encryptedKey EncryptedKey }
*
*  KeyAgreeRecipientIdentifier ::= CHOICE {
*        issuerAndSerialNumber IssuerAndSerialNumber,
*        rKeyId [0] IMPLICIT RecipientKeyIdentifier }
*
*  EncryptedKey ::= OCTET STRING
*
*  @lib cms.lib
*  @since 2.8
*/
class CCMSRecipientEncryptedKey : public CCMSSequence
    {
    public:  // Constructors and destructor
		/**
        * Two-phased constructor.
        */
        IMPORT_C static CCMSRecipientEncryptedKey* NewL();

        /**
        * Two-phased constructor. Copies of the parameters are made.
        *
        * @param aIssuerAndSerialNumber rid (KeyAgreeRecipientIdentifier) value
        * @param aEncryptedKey EncryptedKey value
        */
        IMPORT_C static CCMSRecipientEncryptedKey* NewL(
            const CCMSIssuerAndSerialNumber& aIssuerAndSerialNumber,
            const TDesC8& aEncryptedKey );

        /**
        * Two-phased constructor. Copies of the parameters are made.
        *
        * @param aRKeyId rid (KeyAgreeRecipientIdentifier) value
        * @param aEncryptedKey EncryptedKey value
        */
        IMPORT_C static CCMSRecipientEncryptedKey* NewL(
            const CCMSKeyIdentifier& aRKeyId,
            const TDesC8& aEncryptedKey );


        /**
        * Destructor.
        */
        virtual ~CCMSRecipientEncryptedKey();

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
        * Getter for rKeyId. Ownership is not transferred.
        * @since 2,8
        * @return rKeyId or NULL if rid is not rKeyId.
        */
		IMPORT_C const CCMSKeyIdentifier* RKeyId() const;

		/**
        * Setter for EncryptedKey. Creates a copy of the parameters.
        * @since 2.8
        * @param aEncryptedKey EncryptedKey value.
        */
		IMPORT_C void SetEncryptedKeyL(
            const TDesC8& aEncryptedKey );

		/**
        * Setter for IssuerAndSerialNumber. Creates a copy of the parameters.
        * Note that any existing rKeyId value is deleted.
        * @since 2.8
        * @param aIssuerAndSerialNumber IssuerAndSerialNumber value.
        */
		IMPORT_C void SetIssuerAndSerialNumberL(
            const CCMSIssuerAndSerialNumber& aIssuerAndSerialNumber );

		/**
        * Setter for rKeyId. Creates a copy of the parameters.
        * Note that any existing IssuerAndSerialNumber value is deleted.
        * @since 2.8
        * @param aRKeyId RecipientEncryptedKey value.
        */
		IMPORT_C void SetRKeyIdL(
            const CCMSKeyIdentifier& aRKeyId );


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

    protected:  // New functions

	   /**
        * Protected construction to allow derivation
        */
        IMPORT_C void ConstructL(
            const CCMSIssuerAndSerialNumber& aIssuerAndSerialNumber,
            const TDesC8& aEncryptedKey );

       /**
        * Protected construction to allow derivation
        */
        IMPORT_C void ConstructL(
            const CCMSKeyIdentifier& aRKeyId,
            const TDesC8& aEncryptedKey );

        /**
        * C++ default constructor.
        */
        IMPORT_C CCMSRecipientEncryptedKey();

    private:    // Data
        // EncryptedKey, owned
        TDesC8* iEncryptedKey;
        // IssuerAndSerialNumber, owned
        CCMSIssuerAndSerialNumber* iIssuerAndSerialNumber;
        // RecipientEncryptedKey, owned
        CCMSKeyIdentifier* iRKeyId;

    };

#endif      // CCMSRecipientEncryptedKey_H

// End of File
