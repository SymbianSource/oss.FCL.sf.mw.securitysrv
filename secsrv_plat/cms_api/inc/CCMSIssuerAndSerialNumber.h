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
* Description:  ?Description
*
*/



#ifndef CCMSIssuerAndSerialNumber_H
#define CCMSIssuerAndSerialNumber_H

//  INCLUDES
#include "CCMSSequence.h"

// FORWARD DECLARATIONS
class CX500DistinguishedName;

// Extension
class MCMSIssuerAndSerialNumberExtension;


// CLASS DECLARATION
/**
*  IssuerAndSerialNumber type module
*
*  ASN.1 definition:
*  IssuerAndSerialNumber ::= SEQUENCE {
* 		issuer Name,
*  		serialNumber CertificateSerialNumber }
*
*  @lib cms.lib
*  @since 2.8
*/
class CCMSIssuerAndSerialNumber : public CCMSSequence
    {
    public:  // Constructors and destructor
		/**
        * Two-phased constructor.
        */
        IMPORT_C static CCMSIssuerAndSerialNumber* NewL();

        /**
        * Two-phased constructor.
        * Does not take ownership of the issuer name
        * @param aBinaryData	The encoded binary representation of issuer name.
        * @param aSerialNumber	Serial number.
        */
        IMPORT_C static CCMSIssuerAndSerialNumber* NewL(
											const CX500DistinguishedName& aIssuerName,
        								    const TDesC8& aSerialNumber );
        /**
        * Destructor.
        */
        virtual ~CCMSIssuerAndSerialNumber();

    public: // New functions
        /**
        * Getter for issuer name
        * @since 2.8
        * @return Issuer name in CX500DistinguishedName reference
        */
		IMPORT_C const CX500DistinguishedName& IssuerName() const;

        /**
        * Getter for serial number
        * @since 2,8
        * @return Serial number
        */
		IMPORT_C const TDesC8& SerialNumber() const;

		/**
        * Setter for issuer name, takes copy of issuer name
        * @since 2.8
        * @param aBinaryData	Issuer name in CX500DistinguishedName.
        */
		IMPORT_C void SetIssuerNameL( const CX500DistinguishedName& aIssuerName );

        /**
        * Setter for serial number
        * @since 2,8
        * @param aSerialNumber Serial number to be set
        */
		IMPORT_C void SetSerialNumberL( const TDesC8& aSerialNumber );

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
        IMPORT_C void ConstructL( const CX500DistinguishedName& aBinaryData,
								  const TDesC8& aSerialNumber );

        /**
        * C++ default constructor.
        */
        IMPORT_C CCMSIssuerAndSerialNumber();

    private:    // Data
        // Issuer name, owned
        CX500DistinguishedName* iIssuerName;
        // Serial number, owned
		HBufC8* iSerialNumber;

		// Reserved for extensions
		MCMSIssuerAndSerialNumberExtension* iReserved;
    };

#endif      // CCMSIssuerAndSerialNumber_H

// End of File
