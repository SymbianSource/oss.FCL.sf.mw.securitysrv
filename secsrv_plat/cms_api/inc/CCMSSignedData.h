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



#ifndef CCMSSignedData_H
#define CCMSSignedData_H

//  INCLUDES
#include "CCMSSequence.h"
#include "CCMSEncapsulatedContentInfo.h"
#include "CCMSX509AlgorithmIdentifier.h"
#include "CCMSSignerInfo.h"
#include "CCMSX509CertificateList.h"
#include "CCMSCertificateChoices.h"
#include <badesca.h>

// FORWARD DECLARATIONS

// Extension
class MCMSSignedDataExtension;


// CLASS DECLARATION
/**
*  Attribute type module
*
*  ASN.1 Definition:
*  SignedData ::= SEQUENCE {
*  	 version CMSVersion,
*    digestAlgorithms DigestAlgorithmIdentifiers,
*    encapContentInfo EncapsulatedContentInfo,
*    certificates [0] IMPLICIT CertificateSet OPTIONAL,
*    crls [1] IMPLICIT CertificateRevocationLists OPTIONAL,
*    signerInfos SignerInfos }
*
*  DigestAlgorithmIdentifiers ::= SET OF DigestAlgorithmIdentifier
*
*  SignerInfos ::= SET OF SignerInfo
*
*  @lib cms.lib
*  @since 2.8
*/
class CCMSSignedData : public CCMSSequence
    {
    public:  // Constructors and destructor
		/**
        * Two-phased constructor.
        * NOTE: Encrypt will leave with KErrArgument if
        * valid parameters are not set.
        */
        IMPORT_C static CCMSSignedData* NewLC();

        /**
        * Two-phased constructor.
        * Takes copy of all parameters.
        * Leaves newly created instance in CleanupStack
		*
		* @param aDigestAlgorithmIdentifiers Array of digest algorithm 
		*		 identifiers, mandatory
        * @param aContentInfo Encapsulated content info, mandatory
		* @param aSignerInfos, Array of signer infos, mandatory
        */
        IMPORT_C static CCMSSignedData* NewLC(
			const CArrayPtr< CCMSX509AlgorithmIdentifier >& aDigestAlgorithmIdentifiers,
        	const CCMSEncapsulatedContentInfo& aContentInfo,
        	const CArrayPtr< CCMSSignerInfo >& aSignerInfos );

		/**
        * Two-phased constructor.
        * Same as above although takes only one value
		* Leaves newly created instance in CleanupStack
		* @param aDigestAlgorithmIdentifiers Array of digest algorithm 
		*		 identifiers, mandatory
        * @param aContentInfo Encapsulated content info, mandatory
		* @param aSignerInfos, Array of signer infos, mandatory
		* @param aCertificates Array of certificates choices, optional
		* @param aRevokedCertificates Array of revoked certificates, optional
        */
        IMPORT_C static CCMSSignedData* NewLC(
			const CArrayPtr< CCMSX509AlgorithmIdentifier >& aDigestAlgorithmIdentifiers,
        	const CCMSEncapsulatedContentInfo& aContentInfo,
        	const CArrayPtr< CCMSSignerInfo >& aSignerInfos,
        	const CArrayPtr< CCMSCertificateChoices >* aCertificates,
        	const CArrayPtr< CCMSX509CertificateList >* aRevokedCertificates );

        /**
        * Destructor.
        */
        virtual ~CCMSSignedData();

    public: // New functions

    	/**
		* Getter for Version
		*
		* Version is the syntax version number.  If no attribute
		* certificates are present in the certificates field, the
		* encapsulated content type is id-data, and all of the elements of
		* SignerInfos are version 1, then the value of version shall be 1.
		* Alternatively, if attribute certificates are present, the
		* encapsulated content type is other than id-data, or any of the
		* elements of SignerInfos are version 3, then the value of version
		* shall be 3.
		*
		* @since 2.8
		* @return version number, 1 or 3
		*/
		IMPORT_C TInt Version() const;

    	/**
		* Getter for DigestAlgorithmIdentifiers
		*
		* DigestAlgorithms is a collection of message digest algorithm
		* identifiers.  There may be any number of elements in the
		* collection, including zero.  Each element identifies the message
		* digest algorithm, along with any associated parameters, used by
		* one or more signer.  The collection is intended to list the
		* message digest algorithms employed by all of the signers, in any
		* order, to facilitate one-pass signature verification.
		*
		* @since 2.8
		* @return Array of digest algorithm identifiers.
		*/
		IMPORT_C const CArrayPtr< CCMSX509AlgorithmIdentifier >& DigestAlgorithmIdentifiers() const;

    	/**
		* Getter for EncapsulatedContentInfo
		*
		* EncapsulatedContentInfo is the signed content, consisting of a content
		* type identifier and the content itself.
		* 
		* @since 2.8
		* @return Encapsulated content info.
		*/
        IMPORT_C const CCMSEncapsulatedContentInfo& EncapsulatedContentInfo() const;

    	/**
		* Getter for SignerInfos
		*
		* SignerInfos is a collection of per-signer information.  There may
		* be any number of elements in the collection, including zero.
		*
		* @since 2.8
		* @return Array of signer infos, array might be also empty
		*/
        IMPORT_C const CArrayPtr< CCMSSignerInfo >& SignerInfos() const;

    	/**
		* Getter for Certificate
		*
		* Certificates is a collection of certificates.  It is intended that
		* the set of certificates be sufficient to contain chains from a
		* recognized "root" or "top-level certification authority" to all of
		* the signers in the signerInfos field.  There may be more
		* certificates than necessary, and there may be certificates
		* sufficient to contain chains from two or more independent top-
		* level certification authorities.  There may also be fewer
		* certificates than necessary, if it is expected that recipients
		* have an alternate means of obtaining necessary certificates (e.g.,
		* from a previous set of certificates).  
		*
		* @since 2.8
		* @return Array of certificates or NULL if certificates are absent
		*/
        IMPORT_C const CArrayPtr< CCMSCertificateChoices >* Certificates() const;

    	/**
		* Getter for RevokedCertificates
		*
		* RevokedCertificates is a collection of certificate revocation lists (CRLs).  
		* It is intended that the set contain information sufficient to
		* determine whether or not the certificates in the certificates
		* field are valid, but such correspondence is not necessary.  There
		* may be more CRLs than necessary, and there may also be fewer CRLs
		* than necessary.
		*
		* @since 2.8
		* @return Array of revoked certificates or NULL if there is no revoked
		*	      certificates
		*/
        IMPORT_C 
		const CArrayPtr< CCMSX509CertificateList >* RevokedCertificates() const;

		/**
        * Setter for DigestAlgorithmIdentifiers, takes copy
		*
        * @since 2.8
        * @param aDigestAlgorithmIdentifiers Array of digest algorithm identifiers
        */
		IMPORT_C void SetDigestAlgorithmIdentifiersL(
			const CArrayPtr< CCMSX509AlgorithmIdentifier >&
											aDigestAlgorithmIdentifiers );

		/**
        * Setter for EncapsulatedContentInfo, takes copy
        * @since 2.8
        * @param aContentInfo Encapsulated content info
        */
        IMPORT_C void SetEncapsulatedContentInfoL(
			const CCMSEncapsulatedContentInfo& aContentInfo );

		/**
        * Setter for SignerInfos, takes copy
        * @since 2.8
        * @param aSignerInfos Array of signer info, array can be also empty
        */
        IMPORT_C void SetSignerInfosL(
			const CArrayPtr< CCMSSignerInfo >& aSignerInfos );

		/**
        * Setter for Certificates, takes copy
        * @since 2.8
        * @param aCertificates Array of certificates or NULL if certificates
		*	     are intented to remove from this instance
        */
        IMPORT_C void SetCertificatesL(
			const CArrayPtr< CCMSCertificateChoices >* aCertificates );

		/**
        * Setter for RevokedCertificates, takes copy
        * @since 2.8
        * @param aRevokedCertificates Array of revoked certificates or NULL if
		*		 removing revoked certificates from this instance
        */
        IMPORT_C void SetRevokedCertificatesL(
			const CArrayPtr< CCMSX509CertificateList >* aRevokedCertificates );

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
        IMPORT_C void ConstructL(
			const CArrayPtr< CCMSX509AlgorithmIdentifier >& aDigestAlgorithmIdentifiers,
        	const CCMSEncapsulatedContentInfo& aContentInfo,
        	const CArrayPtr< CCMSSignerInfo >& aSignerInfos,
        	const CArrayPtr< CCMSCertificateChoices >* aCertificates,
        	const CArrayPtr< CCMSX509CertificateList >* aRevokedCertificates );

        /**
        * C++ default constructor.
        */
        IMPORT_C CCMSSignedData();

	private:
		
		/**
		 * Finds out right version with following rules:
		 *
		 * If no attribute certificates are present in the certificates field, the
		 * encapsulated content type is id-data, and all of the elements of
		 * SignerInfos are version 1, then the value of version shall be 1.
		 *
		 * Alternatively, if attribute certificates are present, the
		 * encapsulated content type is other than id-data, or any of the
		 * elements of SignerInfos are version 3, then the value of version
		 * shall be 3.
		 */
		void ValidateVersion();

    private:    // Data
		TInt iVersion;

		// DigestAlgorithmIdentifiers, owned
		CArrayPtr< CCMSX509AlgorithmIdentifier >* iDigestAlgorithmIdentifiers;
		// Encapsulate Content Info, owned
		CCMSEncapsulatedContentInfo* iContentInfo;
		// Signer Infos, owned
		CArrayPtr< CCMSSignerInfo >* iSignerInfos;
		// Certificates, owned, null if absent
		CArrayPtr< CCMSCertificateChoices >* iCertificates;
		// Revoked certificates, owned, null if absent
		CArrayPtr< CCMSX509CertificateList >* iRevokedCertificates;

		// Reserved for extensions
		MCMSSignedDataExtension* iReserved;
    };

#endif      // CCMSSignedData_H

// End of File
