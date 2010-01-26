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



#ifndef CCMSCertificateChoices_H
#define CCMSCertificateChoices_H

// INCLUDES
#include <e32base.h>
#include <asn1cons.h>
#include "MCMSModule.h"

// FORWARD DECLARATIONS
class CCMSX509Certificate;
class CCMSX509AttributeCertificate;

// CONSTANTS
const TTagType KCMSAttrCertTag = 1;

// CLASS DECLARATION
/**
*  CertificateChoices type module
*
*  ASN.1 definition:
*       CertificateChoices ::= CHOICE {
*         certificate Certificate,                 -- See X.509
*         extendedCertificate [0] IMPLICIT ExtendedCertificate,
*                                                  -- Obsolete
*         attrCert [1] IMPLICIT AttributeCertificate }
*                                                  -- See X.509 and X9.57
* 
*  @lib cms.lib
*  @since 2.8
*/
class CCMSCertificateChoices : public CBase, public MCMSModule
    {

    public:  // Constructors and destructor
		/**
        * Two-phased constructor.
        */
        IMPORT_C static CCMSCertificateChoices* NewL();

		/**
        * Two-phased constructor. Leaves a copy in cleanup stack.
        */
        IMPORT_C static CCMSCertificateChoices* NewLC();

        /**
        * Two-phased constructor. Copies of the parameters are made.
        *
        * @param aCertificate X.509 Certificate
        */
        IMPORT_C static CCMSCertificateChoices* NewL(
            const CCMSX509Certificate& aCertificate );

        /**
        * Two-phased constructor. Copies of the parameters are made.
        * Leaves a copy in cleanup stack.
        *
        * @param aCertificate X.509 Certificate
        */
        IMPORT_C static CCMSCertificateChoices* NewLC(
            const CCMSX509Certificate& aCertificate );

        /**
        * Two-phased constructor. Copies of the parameters are made.
        *
        * @param aSubjectKeyIdentifier X.509 subjectKeyIdentifier
        */
        IMPORT_C static CCMSCertificateChoices* NewL(
            const CCMSX509AttributeCertificate& aAttrCert );

        /**
        * Two-phased constructor. Copies of the parameters are made.
        * Leaves a copy in cleanup stack.
        *
        * @param aSubjectKeyIdentifier X.509 subjectKeyIdentifier
        */
        IMPORT_C static CCMSCertificateChoices* NewLC(
            const CCMSX509AttributeCertificate& aAttrCert );

        /**
        * Destructor.
        */
        virtual ~CCMSCertificateChoices();

    public: // New functions

        /**
        * Getter for certificate.
        * @since 2.8
        * @return Certificate or NULL
        */
		IMPORT_C const CCMSX509Certificate* Certificate() const;

        /**
        * Getter for attrCert.
        * @since 2.8
        * @return attrCert or NULL
        */
		IMPORT_C const CCMSX509AttributeCertificate* AttrCert() const;

		/**
        * Setter for Certificate. Create a copy of the parameters.
        * @since 2.8
        * @param aCertificate X.509 Certificate
        */
		IMPORT_C void SetCertificateL(
            const CCMSX509Certificate& aCertificate );

		/**
        * Setter for attrCert. Create a copy of the parameters.
        * @since 2.8
        * @param aAttrCert X.509 AttributeCertificate
        */
		IMPORT_C void SetAttrCertL(
            const CCMSX509AttributeCertificate& aAttrCert );


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
            const CCMSX509Certificate& aCertificate );

        /**
        * Protected construction to allow derivation
        */
        IMPORT_C void ConstructL(
            const CCMSX509AttributeCertificate& aAttrCert );

        /**
        * C++ default constructor.
        */
        IMPORT_C CCMSCertificateChoices();

    private:    // Data

        // certificate value
        CCMSX509Certificate* iCertificate;

        // attrCert value
        CCMSX509AttributeCertificate* iAttrCert;
        
    };

#endif      // CCMSCertificateChoices_H

// End of File
