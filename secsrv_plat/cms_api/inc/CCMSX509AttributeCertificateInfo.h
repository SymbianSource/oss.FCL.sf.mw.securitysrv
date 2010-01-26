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
* Description:  X.509 AttributeCertificateInfo type
*
*/



#ifndef CCMSX509AttributeCertificateInfo_H
#define CCMSX509AttributeCertificateInfo_H

#include "CCMSSequence.h"

// FORWARD DECLARATIONS
class CCMSX509GeneralNames;
class CCMSX509IssuerSerial;
class CCMSX509AlgorithmIdentifier;
class CCMSAttribute;

// Extensions
class MCMSX509AttributeCertificateInfoExtension;

// CLASS DECLARATION
/**
*  X.509 AttributeCertificateInfo type module
*
*  ASN.1 definition:
*   AttributeCertificateInfo ::= SEQUENCE {
*    version                Version DEFAULT v1,
*    subject
*     CHOICE {baseCertificateID  [0]  IssuerSerial, --  associated
*                                                with a Public Key Certificate--
*             subjectName        [1]  GeneralNames}, --  associated  with a name
*    issuer                 GeneralNames, --  CA issuing the attribute
*                                             certificate
*    signature              AlgorithmIdentifier,
*    serialNumber           CertificateSerialNumber,
*    attCertValidityPeriod  AttCertValidityPeriod,
*    attributes             SEQUENCE OF Attribute,
*    issuerUniqueID         UniqueIdentifier OPTIONAL,
*    extensions             Extensions OPTIONAL
*   }
*
*   CertificateSerialNumber ::= INTEGER
*
*   AttCertValidityPeriod ::= SEQUENCE {
*    notBeforeTime  GeneralizedTime,
*    notAfterTime   GeneralizedTime
*   }
*
*  @lib cms.lib
*  @since 3.0
*/
class CCMSX509AttributeCertificateInfo : public CCMSSequence
    {

    private:
        /**
         * Container for certificate data
         */
        NONSHARABLE_CLASS( CAttributeCertificateData ) : public CBase
            {
            public: // destructor
                ~CAttributeCertificateData();
            public: // data
                // version
                TInt iVersion;

                // subject value, owned
                CCMSX509IssuerSerial* iBaseCertificateID;
                // subject value, owned
                CCMSX509GeneralNames* iSubjectName;

                // issuer, owned
                CCMSX509GeneralNames* iIssuer;
                // signature, owned
                CCMSX509AlgorithmIdentifier* iSignature;
                // serialNumber
                TInt iSerialNumber;
                
                // attCertValidityPeriod not before time
                TTime iNotBeforeTime;
                // attCertValidityPeriod not after time
                TTime iNotAfterTime;

                // attributes, owned
                CArrayPtr< CCMSAttribute >* iAttributes;

                // issuerUniqueID, optional, owned
                TDesC8* iIssuerUniqueID;
            };
        
    public:  // Constructors and destructor
		/**
        * Two-phased constructor.
        */
        IMPORT_C static CCMSX509AttributeCertificateInfo* NewL();

        /**
        * Two-phased constructor. Copies of the parameters are made.
        * 
        * @param aBaseCertificateID subject, associated with a Public
        * Key Certificate
        * @param aIssuer CA issuing the attribute certificate
        * @param aSignature signature value
        * @param aSerialNumber serialNumber value
        * @param aNotBeforeTime attCertValidityPeriod value
        * @param aNotAfterTime attCertValidityPeriod value
        * @param aAttributes attributes value
        */
        IMPORT_C static CCMSX509AttributeCertificateInfo* NewL(
            const CCMSX509IssuerSerial& aBaseCertificateID,
            const CCMSX509GeneralNames& aIssuer,
            const CCMSX509AlgorithmIdentifier& aSignature,
            const TInt aSerialNumber,
            const TTime& aNotBeforeTime,
            const TTime& aNotAfterTime,
            const CArrayPtr< CCMSAttribute >& aAttributes );
            
        /**
        * Two-phased constructor. Copies of the parameters are made.
        * 
        * @param aSubjectName subject, associated with a name
        * @param aIssuer CA issuing the attribute certificate
        * @param aSignature signature value
        * @param aSerialNumber serialNumber value
        * @param aNotBeforeTime attCertValidityPeriod value
        * @param aNotAfterTime attCertValidityPeriod value
        * @param aAttributes attributes value
        */
        IMPORT_C static CCMSX509AttributeCertificateInfo* NewL(
            const CCMSX509GeneralNames& aSubjectName,
            const CCMSX509GeneralNames& aIssuer,
            const CCMSX509AlgorithmIdentifier& aSignature,
            const TInt aSerialNumber,
            const TTime& aNotBeforeTime,
            const TTime& aNotAfterTime,
            const CArrayPtr< CCMSAttribute >& aAttributes );

        
        /**
        * Destructor.
        */
        virtual ~CCMSX509AttributeCertificateInfo();

    public: // New functions
        /**
        * Getter for Version.
        * @since 3.0
        * @return Version value
        */
		IMPORT_C TInt Version() const;

        /**
        * Getter for baseCertifiedID (subject value)
        * @since 3.0
        * @return X.509 IssuerSerial value, or NULL if subject is not
        * baseCertificateID
        */
		IMPORT_C const CCMSX509IssuerSerial* BaseCertificateID() const;

        /**
        * Getter for subjectName (subject value)
        * @since 3.0
        * @return X.509 GeneralNames value, or NULL if subject
        * is not subjectName
        */
		IMPORT_C const CCMSX509GeneralNames* SubjectName() const;

        /**
        * Getter for issuer.
        * @since 3.0
        * @return X.509 GeneralNames value
        */
		IMPORT_C const CCMSX509GeneralNames& Issuer() const;

        /**
        * Getter for signature.
        * @since 3.0
        * @return X.509 AlgorithmIdentifier value
        */
		IMPORT_C const CCMSX509AlgorithmIdentifier& Signature() const;

        /**
        * Getter for serialNumber.
        * @since 3.0
        * @return X.509 CertificateSerialNumber value
        */
		IMPORT_C TInt SerialNumber() const;

        /**
        * Getter for notBeforeTime (part of attCertValidityPeriod).
        * @since 3.0
        * @return notBeforeTime value
        */
		IMPORT_C const TTime& NotBeforeTime() const;

        /**
        * Getter for notAfterTime (part of attCertValidityPeriod).
        * @since 3.0
        * @return notAfterTime value
        */
		IMPORT_C const TTime& NotAfterTime() const;

        /**
        * Getter for attributes
        * @since 3.0
        * @return attribute array
        */
		IMPORT_C const CArrayPtr< CCMSAttribute >& Attributes() const;

        /**
        * Getter for issuerUniqueID
        * @since 3.0
        * @return X.509 UniqueIdentifier (BIT STRING), or NULL if
        * issuerUniqueID is absent.
        */
		IMPORT_C const TDesC8* IssuerUniqueID() const;
        
        /**
        * Setter for Version
        * @since 3.0
        * @param aVersion Version value.
        */
		IMPORT_C void SetVersion( const TInt aVersion );

		/**
        * Setter for baseCertificateID. Creates a copy. Deletes also
        * subjectName value.
        * @since 3.0
        * @param aBaseCertificateID subject value, associated  with a
        * Public Key Certificate 
        */
		IMPORT_C void SetBaseCertificateIDL(
            const CCMSX509IssuerSerial& aBaseCertificateID );

		/**
        * Setter for subjectName. Creates a copy. Deletes also
        * baseCertificateID value.
        * @since 3.0
        * @param aSubjectName subject value, associated  with a name
        */
		IMPORT_C void SetSubjectNameL(
            const CCMSX509GeneralNames& aSubjectName );

        /**
        * Setter for issuer. Creates a copy.
        * @since 3.0
        * @param aIssuer X.509 GeneralNames value
        */
		IMPORT_C void SetIssuerL( const CCMSX509GeneralNames& aIssuer );

        /**
        * Setter for signature. Creates a copy.
        * @since 3.0
        * @param aSignature X.509 AlgorithmIdentifier value
        */
		IMPORT_C void
        SetSignatureL( const CCMSX509AlgorithmIdentifier& aSignature );

        /**
        * Setter for serialNumber.
        * @since 3.0
        * @param aSerialNumber X.509 CertificateSerialNumber value
        */
		IMPORT_C void SetSerialNumber( const TInt aSerialNumber );

        /**
        * Setter for notBeforeTime (part of attCertValidityPeriod).
        * Creates a copy
        * @since 3.0
        * @param aNotBeforeTime notBeforeTime value
        */
		IMPORT_C void SetNotBeforeTimeL( const TTime& aNotBeforeTime );

        /**
        * Setter for notAfterTime (part of attCertValidityPeriod).
        * Creates a copy.
        * @since 3.0
        * @param aNotAfterTime notAfterTime value
        */
		IMPORT_C void SetNotAfterTimeL( const TTime& aNotAfterTime );

        /**
        * Setter for attributes. Creates a copy.
        * @since 3.0
        * @param aAttributes attribute array
        */
		IMPORT_C void
        SetAttributesL( const CArrayPtr< CCMSAttribute >& aAttributes );

        /**
        * Setter for issuerUniqueID. Creates a copy.
        * @since 3.0
        * @param aIssuerUniqueID X.509 UniqueIdentifier (BIT STRING)
        */
		IMPORT_C void SetIssuerUniqueIDL( const TDesC8& aIssuerUniqueID );
                
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
            const CCMSX509IssuerSerial& aBaseCertificateID,
            const CCMSX509GeneralNames& aIssuer,
            const CCMSX509AlgorithmIdentifier& aSignature,
            const TInt aSerialNumber,
            const TTime& aNotBeforeTime,
            const TTime& aNotAfterTime,
            const CArrayPtr< CCMSAttribute >& aAttributes );
            
       /**
        * Protected construction to allow derivation
        */
        IMPORT_C void ConstructL(
            const CCMSX509GeneralNames& aSubjectName,
            const CCMSX509GeneralNames& aIssuer,
            const CCMSX509AlgorithmIdentifier& aSignature,
            const TInt aSerialNumber,
            const TTime& aNotBeforeTime,
            const TTime& aNotAfterTime,
            const CArrayPtr< CCMSAttribute >& aAttributes );

        /**
         * Protected construction to allow derivation
         */
        IMPORT_C void ConstructL( );
        
        /**
        * C++ default constructor.
        */
        IMPORT_C CCMSX509AttributeCertificateInfo( );

    private:    // Data
        // Contains all members, owned
        CAttributeCertificateData* iData;

        // Reserved for extensions
        MCMSX509AttributeCertificateInfoExtension* iReserved;
        
    };

#endif      // CCMSX509AttributeCertificateInfo_H

// End of File
