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
* Description:  X.509 Certificate type
*
*/



#ifndef CCMSX509Certificate_H
#define CCMSX509Certificate_H

#include "CCMSX509Signed.h"

// FORWARD DECLARATIONS
class CCMSX509AlgorithmIdentifier;
class CCMSX509SubjectPublicKeyInfo;
class CX500DistinguishedName;
class CCMSX509Validity;
class CX509CertExtension;
class CX509Certificate;

// Extensions
class MCMSX509CertificateExtension;

// CLASS DECLARATION
/**
*  X.509 CertificateInfo type module
*
*  ASN.1 definition:
* --  basic certificate definition
* Certificate ::=
*   SIGNED
*     {SEQUENCE {version                  [0]  Version DEFAULT v1,
*                serialNumber             CertificateSerialNumber,
*                signature                AlgorithmIdentifier,
*                issuer                   Name,
*                validity                 Validity,
*                subject                  Name,
*                subjectPublicKeyInfo     SubjectPublicKeyInfo,
*                issuerUniqueIdentifier   [1] IMPLICIT UniqueIdentifier OPTIONAL,
*                --  if present, version must be v2 or v3
*                subjectUniqueIdentifier  [2] IMPLICIT UniqueIdentifier OPTIONAL,
*                --  if present, version must be v2 or v3
*                extensions               [3]  Extensions OPTIONAL
*     }} --  If present, version must be v3 
*
*  @lib cms.lib
*  @since 3.0
*/
class CCMSX509Certificate : public CCMSX509Signed
    {

    private:
        /**
         * Container for certificate data
         */
        NONSHARABLE_CLASS( CCertificateData ) : public CBase
            {
            public: // destructor
                ~CCertificateData();

            public: // data
                // version
                TInt iVersion;

                // serialNumber, owned
                HBufC8* iSerialNumber;

                // signature AlgorithmIdentifier, owned
                CCMSX509AlgorithmIdentifier* iSignature;

                // issuer Name, owned
                CX500DistinguishedName* iIssuer;

                // validity, owned
                CCMSX509Validity* iValidity;

                // subject Name, owned
                CX500DistinguishedName* iSubject;

                // subjectPublicKeyInfo, owned
                CCMSX509SubjectPublicKeyInfo* iSubjectPublicKeyInfo;

                // issuerUniqueIdentifier UniqueIdentifier ::= BIT STRING, owned
                HBufC8* iIssuerUniqueIdentifier;

                // subjectUniqueIdentifier UniqueIdentifier, owned
                HBufC8* iSubjectUniqueIdentifier;
            };                
        
    public:  // Constructors and destructor
		/**
        * Two-phased constructor.
        */
        IMPORT_C static CCMSX509Certificate* NewL();

        /**
         * Two-phased constructor. Copies the data from the
         * CX509Certificate parameter.
         *
         * @param aCertificate Certificate from which the data is
         * copied.
         */
        IMPORT_C static CCMSX509Certificate* NewL(
            const CX509Certificate& aCertificate );
        
        /**
        * Two-phased constructor. Copies of the parameters are made.
        *
        * @param aSerialNumber CertificateSerialNumber value
        * @param aSignature AlgorithmIdentifier value
        * @param aIssuer Name value
        * @param aValidity Validity value
        * @param aSubject Name value
        * @param aSubjectPublicKeyInfo SubjectPublicKeyInfo value
        * @param aAlgorithmIdentifier identifies the algorithm used in
        * the signature 
        * @param aEncrypted must be the result of applying a hashing
        * procedure to the DER-encoded octets of a value of
        * toBeSigned and then applying an encipherment
        * procedure to those octets
        */
        IMPORT_C static CCMSX509Certificate* NewL(
            const TDesC8& aSerialNumber,
            const CCMSX509AlgorithmIdentifier& aSignature,
            const CX500DistinguishedName& aIssuer,
            const CCMSX509Validity& aValidity,
            const CX500DistinguishedName& aSubject,
            const CCMSX509SubjectPublicKeyInfo& aSubjectPublicKeyInfo,
            const CCMSX509AlgorithmIdentifier& aAlgorithmIdentifier,
            const TDesC8& aEncrypted
            );
        
        /**
        * Destructor.
        */
        virtual ~CCMSX509Certificate();

    public: // New functions
        /**
        * Getter for the version.
        * @since 3.0
        * @return Version
        */
		IMPORT_C TInt Version() const;

        /**
        * Getter for the serialNumber.
        * @since 3.0
        * @return CertificateSerialNumber
        */
		IMPORT_C const TDesC8& SerialNumber() const;

        /**
        * Getter for the signature.
        * @since 3.0
        * @return AlgorithmIdentifier
        */
		IMPORT_C const CCMSX509AlgorithmIdentifier& Signature() const;

        /**
        * Getter for the issuer.
        * @since 3.0
        * @return X.500 DistinguishedName of the issuer of the certificate.
        */
		IMPORT_C const CX500DistinguishedName& Issuer() const;

        /**
        * Getter for the validity.
        * @since 3.0
        * @return Validity
        */
		IMPORT_C const CCMSX509Validity& Validity() const;

        /**
        * Getter for the subject.
        * @since 3.0
        * @return X.500 DistinguishedName of the subject of the certificate.
        */
		IMPORT_C const CX500DistinguishedName& Subject() const;

        /**
        * Getter for the subjectPublicKeyInfo.
        * @since 3.0
        * @return SubjectPublicKeyInfo
        */
		IMPORT_C const CCMSX509SubjectPublicKeyInfo&
        SubjectPublicKeyInfo() const;

        /**
        * Getter for the issuerUniqueIdentifier.
        * @since 3.0
        * @return UniqueIdentifier or NULL
        */
		IMPORT_C const TDesC8*
        IssuerUniqueIdentifier() const;

        /**
        * Getter for the subjectUniqueIdentifier.
        * @since 3.0
        * @return UniqueIdentifier or NULL
        */
		IMPORT_C const TDesC8*
        SubjectUniqueIdentifier() const;

        /**
        * Setter for the version.
        * @since 3.0
        * @param aVersion Version
        */
		IMPORT_C void SetVersion( const TInt aVersion );

        /**
        * Setter for the serialNumber.
        * @since 3.0
        * @param aSerialNumber CertificateSerialNumber
        */
		IMPORT_C void SetSerialNumberL( const TDesC8& aSerialNumber );

        /**
        * Setter for the signature.
        * @since 3.0
        * @param aSignature AlgorithmIdentifier
        */
		IMPORT_C void
        SetSignatureL( const CCMSX509AlgorithmIdentifier& aSignature );

        /**
        * Setter for the issuer.
        * @since 3.0
        * @param aIssuer The X.500 DistinguishedName of the issuer of
        * the certificate
        */
		IMPORT_C void
        SetIssuerL( const CX500DistinguishedName& aIssuer );

        /**
        * Setter for the validity.
        * @since 3.0
        * @param aValidity Validity
        */
		IMPORT_C void
        SetValidityL( const CCMSX509Validity& aValidity );

        /**
        * Setter for the Subject.
        * @since 3.0
        * @param aSubject X.500 DistinguishedName of the subject of
        * the certificate
        */
		IMPORT_C void
        SetSubjectL( const CX500DistinguishedName& aSubject );

        /**
        * Setter for the SubjectPublicKeyInfo.
        * @since 3.0
        * @param aSubjectPublicKeyInfo SubjectPublicKeyInfo
        */
		IMPORT_C void SetSubjectPublicKeyInfoL(
            const CCMSX509SubjectPublicKeyInfo& aSubjectPublicKeyInfo );

        /**
        * Setter for the issuerUniqueIdentifier. Also makes sure
        * version is v2 or v3 (integer values 1 and 2). 
        * @since 3.0
        * @param aIssuerUniqueIdentifier UniqueIdentifier (BIT STRING)
        */
		IMPORT_C void SetIssuerUniqueIdentifierL(
            const TDesC8& aIssuerUniqueIdentifier );

        /**
        * Setter for the subjectUniqueIdentifier. Also makes sure
        * version is v2 or v3 (integer values 1 and 2).
        * @since 3.0
        * @param aSubjectUniqueIdentifier UniqueIdentifier (BIT STRING)
        */
		IMPORT_C void SetSubjectUniqueIdentifierL(
            const TDesC8& aSubjectUniqueIdentifier );

        /**
         * Returns ASN1 encoder for the ToBeSigned part.
         * @since 3.0
         * @return ASN1 encoder for the ToBeSigned part.
         */
        IMPORT_C CASN1EncBase* ToBeSignedEncoderLC() const;
        
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

    protected:  // New functions

	   /**
        * Protected construction to allow derivation
        */
        IMPORT_C void ConstructL(
            const TDesC8& aSerialNumber,
            const CCMSX509AlgorithmIdentifier& aSignature,
            const CX500DistinguishedName& aIssuer,
            const CCMSX509Validity& aValidity,
            const CX500DistinguishedName& aSubject,
            const CCMSX509SubjectPublicKeyInfo& aSubjectPublicKeyInfo,
            const CCMSX509AlgorithmIdentifier& aAlgorithmIdentifier,
            const TDesC8& aEncrypted
            );

        /**
         * Protected construction to allow derivation
         */
        IMPORT_C void ConstructL( );

        /**
         * Protected construction to allow derivation.
         */
        IMPORT_C void ConstructL( const CX509Certificate& aCertificate );
        
        /**
        * C++ default constructor.
        */
        IMPORT_C CCMSX509Certificate( );

    private: // New functions

        /**
         * Copies the data from the CX509Certificate object. 
         */
        void SetDataL( const CX509Certificate& aCertificate );
        
    private:    // Data
        // Contains the actual data, owned
        CCertificateData* iData;

        // Reserved for extensions
        MCMSX509CertificateExtension* iReserved;
        
    };

#endif      // CCMSX509Certificate_H

// End of File
