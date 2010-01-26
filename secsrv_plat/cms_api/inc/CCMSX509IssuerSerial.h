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



#ifndef CCMSX509IssuerSerial_H
#define CCMSX509IssuerSerial_H

#include "CCMSSequence.h"

// FORWARD DECLARATIONS
class CCMSX509GeneralNames;

// CLASS DECLARATION
/**
*  X.509 IssuerSerial type module
*
*  ASN.1 definition:
*   IssuerSerial ::= SEQUENCE {
*    issuer     GeneralNames,
*    serial     CertificateSerialNumber,
*    issuerUID  UniqueIdentifier OPTIONAL
*   }
*
*   CertificateSerialNumber ::= INTEGER
*
*   UniqueIdentifier ::= BIT STRING
*
*  @lib cms.lib
*  @since 2.8
*/
class CCMSX509IssuerSerial : public CCMSSequence
    {
    public:  // Constructors and destructor
		/**
        * Two-phased constructor.
        */
        IMPORT_C static CCMSX509IssuerSerial* NewL();

        /**
        * Two-phased constructor. Copies of the parameters are made.
        *
        * @param aIssuer X.509 GeneralNames value
        * @param aSerial X.509 CertificateSerialNumber value
        */
        IMPORT_C static CCMSX509IssuerSerial* NewL(
            const CCMSX509GeneralNames& aIssuer,
            const TInt aSerial );

        /**
        * Two-phased constructor. Copies of the parameters are made.
        *
        * @param aIssuer X.509 GeneralNames value
        * @param aSerial X.509 CertificateSerialNumber value
        * @param aIssuerUID X.509 UniqueIdentifier value
        */
        IMPORT_C static CCMSX509IssuerSerial* NewL(
            const CCMSX509GeneralNames& aIssuer,
            const TInt aSerial,
            const TDesC8& aIssuerUID );

        /**
        * Destructor.
        */
        virtual ~CCMSX509IssuerSerial();

    public: // New functions
        /**
        * Getter for Issuer.
        * @since 2.8
        * @return GeneralNames value
        */
		IMPORT_C const CCMSX509GeneralNames& Issuer() const;

        /**
        * Getter for Serial
        * @since 2.8
        * @return CertificateSerialNumber value
        */
		IMPORT_C TInt Serial() const;

        /**
        * Getter for IssuerUID
        * @since 2.8
        * @return X.509 UniqueIdentifier value or NULL
        */
		IMPORT_C const TDesC8* IssuerUID() const;

		/**
        * Setter for Issuer. Creates a copy of the parameters.
        * @since 2.8
        * @param aIssuer GeneralNames value.
        */
		IMPORT_C void SetIssuerL(
            const CCMSX509GeneralNames& aIssuer );

		/**
        * Setter for Serial.
        * @since 2.8
        * @param aSerial CertificateSerialNumber value.
        */
		IMPORT_C void SetSerial(
            const TInt aSerial );

		/**
        * Setter for IssuerUID. Creates a copy of the parameters.
        * @since 2.8
        * @param aIssuerUID UniqueIdentifier value
        */
		IMPORT_C void SetIssuerUIDL(
            const TDesC8& aIssuerUID );


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
            const CCMSX509GeneralNames& aIssuer );
        
       /**
        * Protected construction to allow derivation
        */
        IMPORT_C void ConstructL(
            const CCMSX509GeneralNames& aIssuer,
            const TDesC8& aIssuerUID );
            
        /**
        * C++ default constructor.
        */
        IMPORT_C CCMSX509IssuerSerial( const TInt aSerial );

    private:    // Data
        // Issuer, owned
        CCMSX509GeneralNames* iIssuer;
        // X.509 CertificateSerialNumber, owned
        TInt iSerial;
        // X.509 UniqueIdentifier, owned
        TDesC8* iIssuerUID;

    };

#endif      // CCMSX509IssuerSerial_H

// End of File
