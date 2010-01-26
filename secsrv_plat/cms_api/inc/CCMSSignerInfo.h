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



#ifndef CCMSSignerInfo_H
#define CCMSSignerInfo_H

//  INCLUDES
#include "CCMSSequence.h"
#include "CCMSIssuerAndSerialNumber.h"
#include "CCMSAttribute.h"
#include "CCMSX509AlgorithmIdentifier.h"
#include "CCMSX509Certificate.h"

#include <badesca.h>

// FORWARD DECLARATIONS
class CASN1EncBase;
// Extension
class MCMSSignerInfoExtension;


// CLASS DECLARATION
/**
*  SignerInfo module
*
*  SignerInfo ::= SEQUENCE {
*  version CMSVersion,
*  sid SignerIdentifier,
*  digestAlgorithm DigestAlgorithmIdentifier,
*  signedAttrs [0] IMPLICIT SignedAttributes OPTIONAL,
*  signatureAlgorithm SignatureAlgorithmIdentifier,
*  signature SignatureValue,
*  unsignedAttrs [1] IMPLICIT UnsignedAttributes OPTIONAL }
*
*  SignerIdentifier ::= CHOICE {
*  issuerAndSerialNumber IssuerAndSerialNumber,
*  subjectKeyIdentifier [0] SubjectKeyIdentifier }
*
*  @lib cms.lib
*  @since 2.8
*/
class CCMSSignerInfo : public CCMSSequence
    {
	private:
		/*
		 * Container for signer info data
		 */
		NONSHARABLE_CLASS( CSignerInfoData ) : public CBase
			{
			public: // Destructor
				~CSignerInfoData();
			public: // data
				// version
				TInt iVersion;

				// Issuer and serial, owned
				CCMSIssuerAndSerialNumber* iIssuerAndSerial;

				// Subject Key ID, owned
				HBufC8* iSubjectKeyIdentifier;

				// Signed attributes, owned
				CArrayPtrFlat<CCMSAttribute>* iSignedAttributes;
				// Unsigned attribute, owned
				CArrayPtrFlat<CCMSAttribute>* iUnsignedAttributes;

				// Digested algorithm identifier, owned
				CCMSX509AlgorithmIdentifier* iDigestAI;
				// Signature algorithm identifier, owned
				CCMSX509AlgorithmIdentifier* iSignatureAI;

				// Signature value, owned
				HBufC8* iSignatureValue;
			};

    public:  // Constructors and destructor
		/**
        * Two-phased constructor.
        * NOTE: Encrypt will leave with KErrBadDescriptor if
        * valid Attribute Type has not setted.
        */
        IMPORT_C static CCMSSignerInfo* NewL();

        /**
        * Two-phased constructor.
        * Takes copy of the both type and values.
        * @param aCertificate		Used certificate
        * @param aMessageDigest	    Message Digest value
        */
        IMPORT_C static CCMSSignerInfo* NewL( const CCMSX509Certificate& aCertificate,
											  const TDesC8& aMessageDigest );

		/**
		* Two-phased constructor.
		* Takes copy of the both type and values.
		* @param aCertificateUrl	Used certificate url
		* @param aSubjectKeyID		Subject Key ID
		* @param aMessageDigest	    Message Digest value
		*/
		IMPORT_C static CCMSSignerInfo* NewL( const TDesC8& aCertificateUrl,
											  const TDesC8& aSubjectKeyID,
								  			  const TDesC8& aMessageDigest );
        /**
        * Destructor.
        */
        virtual ~CCMSSignerInfo();

    public: // New functions

        /**
		* Getter for Version
		* @since 2,8
		* @return Version number (1 or 3)
		*/
		IMPORT_C TInt CMSVersion() const;

		/**
		* Getter for IssuerAndSerialNumber
		* This will be present when normal certificate is used
		* @since 2,8
		* @return IssuerAndSerialNumber or NULL if not present
		*/
		IMPORT_C const CCMSIssuerAndSerialNumber* IssuerAndSerialNumber() const;

		/**
		* Getter for SubjectKeyIdentifier
		* This will be present only if certificate url is used
		* @since 2,8
		* @return SubjectKeyIdentifier or NULL if not present
		*/
		IMPORT_C const TDesC8* SubjectKeyIdentifier() const;

		/**
		* Getter for DigestAlgorithmIdentifier
		* Default value is SHA-1
		* @since 2,8
		* @return DigestAlgorithmIdentifier in CX509AlgorithmIdentifier instance
		*/
		IMPORT_C const CCMSX509AlgorithmIdentifier& DigestAlgorithmIdentifier() const;

		/**
		* Getter for SignedAttributes
		* @since 2,8
		* @return SignedAttributes in pointer array
		*/
		IMPORT_C const CArrayPtrFlat<CCMSAttribute>& SignedAttributes() const;

        /**
         * Gets signedAttributes in encoded form, with SET OF tag instead of [0].
         * This data is supposed to be signed.
         * since 3.0
         * @return encoded form of signedAttributes, NULL if there are no signedAttributes
         */
        IMPORT_C HBufC8* SignedAttributesEncodedL() const;
        
		/**
		* Getter for SignatureAlgorithmIdentifier
		* Default value is RSA
		* @since 2,8
		* @return SignatureAlgorithmIdentifier in CX509AlgorithmIdentifier instance
		*/
		IMPORT_C const CCMSX509AlgorithmIdentifier& SignatureAlgorithmIdentifier() const;

		/**
		* Getter for SignatureValue
		* @since 2,8
		* @return SignatureValue in TDesC8 reference
		*/
		IMPORT_C const TDesC8& SignatureValue() const;

		/**
		* Getter for UnsignedAttributes
		*
		* @since 2,8
		* @return UnsignedAttributes in array
		*/
		IMPORT_C const CArrayPtrFlat<CCMSAttribute>& UnsignedAttributes() const;

        /**
		* Setter for Certificate
		*
		* @since 2,8
		* @param aCertificate certificate to be set
		*/
		IMPORT_C void SetCertificateL( const CCMSX509Certificate& aCertificate );

		/**
		* Getter for SubjectKeyIdentifier
		* @since 2,8
		* @param aCertificateUrl certificate url to be set
		* @param aSubjectKeyIdentifier SubjectKeyIdentifier to be set
		*/
		IMPORT_C void SetCertificateUrlL( const TDesC8& aCertificateUrl,
										  const TDesC8& aSubjectKeyIdentifier );

		/**
		* Setter for DigestAlgorithmIdentifier
		* Transfers ownership
		* @since 2,8
		* @param aDigestAI Digest algorithm identifier to be set
		*/
		IMPORT_C void SetDigestAlgorithmIdentifier(
						CCMSX509AlgorithmIdentifier& aDigestAI );

		/**
		* Setter for SignatureAlgorithmIdentifier
		* Transfers ownership
		* @since 2,8
		* @param aSignatureAI Signature algorithm identifier to be set
		*/
		IMPORT_C void SetSignatureAlgorithmIdentifier(
						CCMSX509AlgorithmIdentifier& aSignatureAI );

		/**
		* Setter for SignatureValue
		* @since 2,8
		* @param aSignatureValue Signature value to be set
		*/
		IMPORT_C void SetSignatureValueL( const TDesC8& aSignatureValue );


		/**
		 * Setter for Signed attributes
		 * Support for ContentType, MessageDigest,
		 * SigningTime and SigningCert
		 * If some of the parameters are not wanted to add use KNullDesC or
		 * KNullDesC8
		 * @since 2,8
		 * @param aContentType ContentType in OID form, e.g. "1.2.840.113549.1.7.1"
		 * @param aMessageDigest Message Digest
		 * @param aSigningTime In most cases current time
		 * @param aCertHash Certificate hash
		 */
		IMPORT_C void SetSignedAttributesL( const TDesC& aContentType,
											const TDesC8& aMessageDigest,
											const TTime& aSigningTime,
											const TDesC8* aCertHash );
		/**
		 * Setter for Unsigned attributes
		 * Transfers ownership
		 * @since 2,8
		 * @param aUnsignedAttributes Array of attributes,
		 *							  to be set in unsigned attributes.
		 */
		IMPORT_C void SetUnsignedAttributesL( CArrayPtrFlat<CCMSAttribute>&
											  aUnsignedAttributes );

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
        IMPORT_C void ConstructL( const TDesC8& aCertificateUrl,
        						  const TDesC8& aSubjectKeyID,
								  const TDesC8& aMessageDigest );

		/**
        * Protected construction to allow derivation
        */
        IMPORT_C void ConstructL( const CCMSX509Certificate& aCertificate,
								  const TDesC8& aMessageDigest );
        /**
        * C++ default constructor.
        */
        IMPORT_C CCMSSignerInfo();

        /**
         * Default construction
         */
        virtual void BaseConstructL( const TDesC8& aMessageDigest );

	private: // New functions

		/**
		 * Decodes attributes to array
		 */
		void DecodeAttributesL( const TDesC8& aRawData,
								CArrayPtrFlat<CCMSAttribute>* aAttributes );

		/**
		 * Creates certificate url attribute
		 */
		CCMSAttribute* CreateCertificateUrlLC( const TDesC8& aCertificateUrl );

    private:    // Data
		// contains all members, owned
		CSignerInfoData* iData;

		// Reserved for extensions
		MCMSSignerInfoExtension* iReserved;
    };

#endif      // CCMSSignerInfo_H

// End of File
