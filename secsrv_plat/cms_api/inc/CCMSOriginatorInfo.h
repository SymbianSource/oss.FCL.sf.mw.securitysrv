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



#ifndef CCMSOriginatorInfo_H
#define CCMSOriginatorInfo_H

#include "CCMSSequence.h"

// FORWARD DECLARATION
class CCMSCertificateChoices;
class CCMSX509CertificateList;

// CLASS DECLARATION
/**
*  CMS OriginatorInfo type module
*
*  ASN.1 definition:
*      OriginatorInfo ::= SEQUENCE {
*        certs [0] IMPLICIT CertificateSet OPTIONAL,
*        crls [1] IMPLICIT CertificateRevocationLists OPTIONAL } 
*
*      CertificateSet ::= SET OF CertificateChoices
*
*      CertificateRevocationLists ::= SET OF CertificateList
*
*  @lib cms.lib
*  @since 2.8
*/
class CCMSOriginatorInfo : public CCMSSequence
    {

    public:  // Constructors and destructor
		/**
        * Two-phased constructor.
        */
        IMPORT_C static CCMSOriginatorInfo* NewL();

        /**
        * Destructor.
        */
        virtual ~CCMSOriginatorInfo();

    public: // New functions

        /**
        * Getter for certs.
        * @since 2.8
        * @return CertificateSet or NULL
        */
		IMPORT_C const CArrayPtr< CCMSCertificateChoices >* Certs() const;

        /**
        * Getter for crls.
        * @since 2.8
        * @return CertificateRevocationLists or NULL
        */
		IMPORT_C const CArrayPtr< CCMSX509CertificateList >* Crls() const;

        /**
        * Setter for certs. Takes ownership.
        * @since 2.8
        * @param aCerts CertificateSet
        */
		IMPORT_C void
        SetCerts( CArrayPtr< CCMSCertificateChoices >* aCerts );

        /**
        * Setter for crls. Takes ownership.
        * @since 2.8
        * @param aCrls CertificateRevocationLists
        */
		IMPORT_C void
        SetCrls( CArrayPtr< CCMSX509CertificateList >* aCrls );

        /**
         * Decoder method with implicit tag checking.
         * @since 2.8
         * @param aRawData raw-data to be parsed in this instance
         * @param aImplicitTag tag to be checked
         */
        void DecodeImplicitTagL( const TDesC8& aRawData,
                                 const TTagType aImplicitTag );         
        
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
        * C++ default constructor.
        */
        IMPORT_C CCMSOriginatorInfo( );

    private: // New Methods

        /**
         * Decodes internal data from a array of ASN.1 decoders
         * @param aItems items to internalize
         */
        void DecodeArrayL( CArrayPtr< TASN1DecGeneric >* aItems );
        
    private:    // Data
        // certs, owned
        CArrayPtr< CCMSCertificateChoices >* iCerts;

        // crls, owned
        CArrayPtr< CCMSX509CertificateList >* iCrls;
        
    };

#endif      // CCMSOriginatorInfo_H

// End of File
