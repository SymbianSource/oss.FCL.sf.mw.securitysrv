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



#ifndef CCMSOriginatorIdentifierOrKey_H
#define CCMSOriginatorIdentifierOrKey_H

// INCLUDES
#include <e32base.h>
#include <asn1cons.h>
#include "MCMSModule.h"

// FORWARD DECLARATIONS
class CCMSIssuerAndSerialNumber;
class CCMSOriginatorPublicKey;
class CASN1EncBase;

// CLASS DECLARATION
/**
*  OriginatorIdentifierOrKey type module
*
*  ASN.1 definition:
*  OriginatorIdentifierOrKey ::= CHOICE {
*        issuerAndSerialNumber IssuerAndSerialNumber,
*        subjectKeyIdentifier [0] SubjectKeyIdentifier,
*        originatorKey [1] OriginatorPublicKey }
*  @lib cms.lib
*  @since 2.8
*/
class CCMSOriginatorIdentifierOrKey : public CBase, public MCMSModule
    {
    public: // data types
        enum TType
            {
            ESubjectKeyIdentifier = 0,
            EOriginatorKey = 1,
            EIssuerAndSerialNumber
            };

    public:  // Constructors and destructor
		/**
        * Two-phased constructor.
        */
        IMPORT_C static CCMSOriginatorIdentifierOrKey* NewL();

        /**
        * Two-phased constructor. Copies of the parameters are made.
        *
        * @param aIssuerAndSerialNumber identifies sender's certificate
        */
        IMPORT_C static CCMSOriginatorIdentifierOrKey* NewL(
            const CCMSIssuerAndSerialNumber& aIssuerAndSerialNumber );

        /**
        * Two-phased constructor. Copies of the parameters are made.
        *
        * @param aSubjectKeyIdentifier X.509 subjectKeyIdentifier
        */
        IMPORT_C static CCMSOriginatorIdentifierOrKey* NewL(
            const TDesC8& aSubjectKeyIdentifier );

        /**
        * Two-phased constructor. Copies of the parameters are made.
        *
        * @param aOriginatorKey includes algorithm identifier and public key
        */
        IMPORT_C static CCMSOriginatorIdentifierOrKey* NewL(
            const CCMSOriginatorPublicKey& aOriginatorKey );

        /**
        * Destructor.
        */
        virtual ~CCMSOriginatorIdentifierOrKey();

    public: // New functions
        /**
         * Getter for the CHOICE type. There is no corresponding
         * setter method as the type is set implicitly by the member
         * value setters.
         * @since 2.8
         * @return the CHOICE type
         */
        IMPORT_C TType Type() const;

        /**
        * Getter for IssuerAndSerialNumber. Leaves with KErrNotFound
        * if the type of this CHOICE is not EIssuerAndSerialNumber.
        * @since 2.8
        * @return IssuerAndSerialNumber
        */
		IMPORT_C const CCMSIssuerAndSerialNumber&
        IssuerAndSerialNumberL() const;

        /**
        * Getter for the X.509 subjectKeyIdentifier. Leaves with
        * KErrNotFound if the type of this CHOICE is not ESubjectKeyIdentifier.
        * @since 2,8
        * @return subjectKeyIdentifier
        */
		IMPORT_C const TDesC8& SubjectKeyIdentifierL() const;

        /**
         * Getter for OriginatorKey. Leaves with KErrNotFound if the
         * type of this CHOICE is not EOriginatorKey.
         * @since 2,8
         * @return OriginatorKey
         */
        IMPORT_C const CCMSOriginatorPublicKey& OriginatorKeyL() const;

		/**
        * Setter for IssuerAndSerialNumber. Create a copy of the parameters.
        * @since 2.8
        * @param aIssuerAndSerialNumber identifies sender's certificate
        */
		IMPORT_C void SetIssuerAndSerialNumberL(
            const CCMSIssuerAndSerialNumber& aIssuerAndSerialNumber );

        /**
        * Setter for subjectKeyIdentifier
        * @since 2,8
        * @param aSubjectKeyIdentifier X.509 subjectKeyIdentifier
        */
		IMPORT_C void SetSubjectKeyIdentifierL(
            const TDesC8& aSubjectKeyIdentifier );

        /**
         * Setter for originatorKey
         * @since 2.8
         * @param aOriginatorKey includes algorithm identifier and
         * public key
         */
        IMPORT_C void SetOriginatorKeyL(
            const CCMSOriginatorPublicKey& aOriginatorKey );

    public: // Functions from base classes

        /**
        * From MCMSModule
        * @since 2.8
        * @param aResult Allocates and sets result in this variable
        * @return ?description
        */
        void EncodeL( HBufC8*& aResult ) const;

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
            const CCMSIssuerAndSerialNumber& aIssuerAndSerialNumber );

        /**
        * Protected construction to allow derivation
        */
        IMPORT_C void ConstructL(
            const TDesC8& aSubjectKeyIdentifier );

        /**
        * Protected construction to allow derivation
        */
        IMPORT_C void ConstructL(
            const CCMSOriginatorPublicKey& aOriginatorKey );

        /**
        * C++ default constructor.
        */
        IMPORT_C CCMSOriginatorIdentifierOrKey();

    private: // New functions

        /**
         * Deletes old value and changes the type of the choice.
         * @param aNewType The type of the CHOICE will be set to
         * aNewType.
         */
        void DeleteOldValueAndChangeTypeL( TType aNewType );

        /**
         * Creates Encoder for the SubjectKeyIdentifier value 
         * @return ASN1 encoder
         */
        CASN1EncBase* EncoderSubjectKeyIdentifierLC() const;

        /**
         * Creates Encoder for the OriginatorKey value
         * @return ASN1 encoder
         */
        CASN1EncBase* EncoderOriginatorKeyLC() const;
        
    private:    // Data

        // defines the type of this CHOICE
        TType iType;

        // IssuerAndSerialNumber value. owned.
        CCMSIssuerAndSerialNumber* iIssuerAndSerialNumber;

        // SubjectKeyIdentifier value. owned.
        HBufC8* iSubjectKeyIdentifier;

        // OriginatorPublicKey value. owned.
        CCMSOriginatorPublicKey* iOriginatorKey;

    };

#endif      // CCMSOriginatorIdentifierOrKey_H

// End of File
