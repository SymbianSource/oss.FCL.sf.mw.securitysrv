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
* Description: 
*
*/



#ifndef CCMSX509CertificateList_H
#define CCMSX509CertificateList_H

//  INCLUDES
#include "CCMSSequence.h"
#include <x509cert.h>

// FORWARD DECLARATIONS
class CCMSX509AlgorithmIdentifier;
// Extension
class MCMSX509CertificateList;

// CLASS DECLARATION
/**
 * These two classes are containers for CertificateList data
 */
class CCMSX509RevokedCertificate : public CBase
	{
	public: // Constructor and destructor
	   /**
        * C++ default constructor.
        */
		IMPORT_C CCMSX509RevokedCertificate();

       /**
        * Destructor.
        */
		virtual ~CCMSX509RevokedCertificate();

	public: // Data
		TInt iUserCertificateSerialNumber;
		TTime iRevokationDate;
		// Owned extensions
		CArrayPtrFlat< CX509CertExtension >* iExtensions;
	};

class CCMSX509CertificateListParameters : public CBase
	{
	public: // Constructor and destructor
	   /**
        * C++ default constructor.
        */
		IMPORT_C CCMSX509CertificateListParameters();

       /**
        * Destructor.
        */
		virtual ~CCMSX509CertificateListParameters();

	public: // Data
		TInt iVersion;
		// Signature algorithm, owned
		CCMSX509AlgorithmIdentifier* iSignatureAlgorithm;
		// Issuer, owned
		CX500DistinguishedName* iIssuer;
		TTime iThisUpdate;
		TTime iNextUpdate;
		// Owned revoked certificates
		CArrayPtrFlat< CCMSX509RevokedCertificate >* iRevokedCertificates;
		// Owned extensions
		CArrayPtrFlat< CX509CertExtension >* iExtensions;
		// Owned signature
		HBufC8* iSignature;
	};

// CLASS DECLARATION
/**
*  X509 CertificateList
*
*  CertificateList  ::=  SEQUENCE  {
*       tbsCertList          TBSCertList,
*       signatureAlgorithm   AlgorithmIdentifier,
*       signature            BIT STRING  }
*
*  TBSCertList  ::=  SEQUENCE  {
*       version                 Version OPTIONAL,
*                                    -- if present, shall be v2
*       signature               AlgorithmIdentifier,
*       issuer                  Name,
*       thisUpdate              Time,
*       nextUpdate              Time OPTIONAL,
*       revokedCertificates     SEQUENCE OF SEQUENCE  {
*            userCertificate         CertificateSerialNumber,
*            revocationDate          Time,
*            crlEntryExtensions      Extensions OPTIONAL
*                                           -- if present, shall be v2
*                                 }  OPTIONAL,
*       crlExtensions           [0] Extensions OPTIONAL
*                                          -- if present, shall be v2 -- }
*
*  Name            ::=   CHOICE { -- only one possibility for now --
*                                   rdnSequence  RDNSequence }
*
*  RDNSequence     ::=   SEQUENCE OF RelativeDistinguishedName
*
*  RelativeDistinguishedName  ::=
*                      SET SIZE (1 .. MAX) OF AttributeTypeAndValue
*
*  Version  ::=  INTEGER  {  v1(0), v2(1), v3(2)  }
*
*  CertificateSerialNumber  ::=  INTEGER
*
*  Time ::= CHOICE {
*       utcTime        UTCTime,
*       generalTime    GeneralizedTime }
*
*  @lib cms.lib
*  @since 2.8
*/
class CCMSX509CertificateList : public CCMSSequence
    {
    public:  // Constructors and destructor
		/**
        * Two-phased constructor.
        * Leaves newly created instance in CleanupStack
        */
        IMPORT_C static CCMSX509CertificateList* NewLC();

        /**
        * Two-phased constructor.
        * Leaves newly created instance in CleanupStack
        * @param aSigningCertificate	Signing certificate
        * @param aRevokedCertificates 	List of revoked certificates
        */
        IMPORT_C static CCMSX509CertificateList* NewLC(
			const CX509Certificate& aSigningCertificate,
			const CArrayPtrFlat<CX509Certificate>& aRevokedCertificates );
        /**
        * Two-phased constructor.
        * Leaves newly created instance in CleanupStack
        * @param aParameters	Parameters used for creating list,
		*						ownership transferred
        */
		IMPORT_C static CCMSX509CertificateList* NewLC(
			CCMSX509CertificateListParameters& aParameters );
        /**
        * Destructor.
        */
        virtual ~CCMSX509CertificateList();

    public: // New functions

        /**
		* Getter for CertificateList parameters
		* @since 2,8
		* @return Signing certificate in CX509Certificate reference
		*/
		IMPORT_C const CCMSX509CertificateListParameters&
													Parameters() const;

        /**
        * Setter for CertificateList parameters, takes ownership
        * @since 2.8
        * @param aSigningCertificate X.509 certificate to be set as signing
        */
		IMPORT_C void SetParametersL(
				CCMSX509CertificateListParameters& aSigningCertificate );


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
        IMPORT_C void ConstructL();

	   /**
        * Protected construction to allow derivation
        */
        IMPORT_C void ConstructL( const CX509Certificate& aSigningCertificate,
				const CArrayPtrFlat<CX509Certificate>& aRevokedCertificates  );

        /**
        * C++ default constructor.
        */
        IMPORT_C CCMSX509CertificateList();

	private:	// New functions

		/**
		 * Encodes Extensions
		 */
		CASN1EncSequence* EncodeExtensionsLC(
				const CArrayPtrFlat<CX509CertExtension>* aExtensions ) const;

		/**
		 * Decodes Extensions
		 */
		CArrayPtrFlat<CX509CertExtension>* DecodeExtensionsL(
										const TASN1DecGeneric* aExtensions );


    private:    // Data
        // List parameters, owned
        CCMSX509CertificateListParameters* iParams;

		// Reserved for extensions
		MCMSX509CertificateList* iReserved;
    };

#endif      // CCMSX509CertificateList_H

// End of File
