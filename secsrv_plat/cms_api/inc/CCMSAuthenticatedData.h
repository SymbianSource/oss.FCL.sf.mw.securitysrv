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
* Description:  CMS AuthenticatedData type
*
*/



#ifndef CCMSAuthenticatedData_H
#define CCMSAuthenticatedData_H

//  INCLUDES
#include "CCMSSequence.h"
#include "CCMSEncapsulatedContentInfo.h"
#include "CCMSX509AlgorithmIdentifier.h"
#include <badesca.h>

// FORWARD DECLARATIONS
// Extension

class MCMSAuthenticatedDataExtension;
class CCMSOriginatorInfo;
class CCMSRecipientInfo;
class CCMSAttribute;
class CCMSCertificateChoices;
class CCMSX509CertificateList;

// CLASS DECLARATION
/**
*  AuthenticatedData type module
*
*  ASN.1 Definition:
*
*     AuthenticatedData ::= SEQUENCE {
*        version CMSVersion,
*        originatorInfo [0] IMPLICIT OriginatorInfo OPTIONAL,
*        recipientInfos RecipientInfos,
*        macAlgorithm MessageAuthenticationCodeAlgorithm,
*        digestAlgorithm [1] DigestAlgorithmIdentifier OPTIONAL,
*        encapContentInfo EncapsulatedContentInfo,
*        authenticatedAttributes [2] IMPLICIT AuthAttributes OPTIONAL,
*        mac MessageAuthenticationCode,
*        unauthenticatedAttributes [3] IMPLICIT UnauthAttributes OPTIONAL }
*
*     AuthAttributes ::= SET SIZE (1..MAX) OF Attribute
*     UnauthAttributes ::= SET SIZE (1..MAX) OF Attribute
* 
*     RecipientInfos ::= SET OF RecipientInfo
*
*     MessageAuthenticationCode ::= OCTET STRING
* 
*     MessageAuthenticationCodeAlgorithm ::= AlgorithmIdentifier
*     DigestAlgorithmIdentifier ::= AlgorithmIdentifier
*
*  @lib cms.lib
*  @since 3.0
*/
class CCMSAuthenticatedData : public CCMSSequence
    {
    private:
        /**
         * Container for the actual data
         */
        NONSHARABLE_CLASS( CAuthenticatedDataData ) : public CBase
            {
            public: // destructor
                ~CAuthenticatedDataData();

            public: // data
                // version
                TInt iVersion;

                // originatorInfo, owned
                CCMSOriginatorInfo* iOriginatorInfo;
                
                // recipientInfos, owned
                CArrayPtr< CCMSRecipientInfo >* iRecipientInfos;

                // macAlgorithm, owned
                CCMSX509AlgorithmIdentifier* iMacAlgorithm;

                // digestAlgorithm, owned
                CCMSX509AlgorithmIdentifier* iDigestAlgorithm;

                // encapContentInfo, owned
                CCMSEncapsulatedContentInfo* iEncapContentInfo;

                // authenticatedAttributes, owned
                CArrayPtr< CCMSAttribute >* iAuthenticatedAttributes;

                // mac, owned
                HBufC8* iMac;

                // unauthenticatedAttributes, owned
                CArrayPtr< CCMSAttribute >* iUnauthenticatedAttributes;
                
            };
        
    public:  // Constructors and destructor
		/**
        * Two-phased constructor.
        * NOTE: Encrypt will leave with KErrArgument if
        * valid parameters are not set.
        */
        IMPORT_C static CCMSAuthenticatedData* NewLC();

        /**
        * Two-phased constructor.
        * Takes ownership of all parameters, except aMac which is copied.
        * Leaves newly created instance in CleanupStack
        */
        IMPORT_C static CCMSAuthenticatedData* NewLC(
			CArrayPtr< CCMSRecipientInfo >* aRecipientInfos,
            CCMSX509AlgorithmIdentifier* aMacAlgorithm,
        	CCMSEncapsulatedContentInfo* aEncapContentInfo,
        	const TDesC8& aMac );

        /**
        * Destructor.
        */
        virtual ~CCMSAuthenticatedData();

    public: // New functions
        /**
         * Getter for version
         * @since 3.0
         * @return CMSVersion
         */
        IMPORT_C TInt Version() const;

        /**
         * Getter for originatorInfo
         * @since 3.0
         * @return OriginatorInfo or NULL
         */
        IMPORT_C const CCMSOriginatorInfo* OriginatorInfo() const;
        
        /**
         * Getter for recipientInfos.
         * @since 3.0
         * @return array of RecipientInfos
         */
        IMPORT_C const CArrayPtr< CCMSRecipientInfo >& RecipientInfos() const;

        /**
         * Getter for macAlgorithm. 
         * @since 3.0
         * @return MessageAuthenticationCodeAlgorithm
         */
        IMPORT_C const CCMSX509AlgorithmIdentifier& MacAlgorithm() const;

        /**
         * Getter for digestAlgorithm
         * @since 3.0
         * @return DigestAlgorithmIdentifier or NULL
         */
        IMPORT_C const CCMSX509AlgorithmIdentifier* DigestAlgorithm() const;

        /**
         * Getter for encapContentInfo
         * @since 3.0
         * @return EncapsulatedContentInfo
         */
        IMPORT_C const CCMSEncapsulatedContentInfo& EncapContentInfo() const;

        /**
         * Getter for authenticatedAttributes
         * @since 3.0
         * @return array of Attributes or NULL
         */
        IMPORT_C const CArrayPtr< CCMSAttribute >*
        AuthenticatedAttributes() const;

        /**
         * Getter for mac
         * @since 3.0
         * @return MessageAuthenticationCode
         */
        IMPORT_C const TDesC8& Mac() const;

        /**
         * Getter for unauthenticatedAttributes
         * @since 3.0
         * @return  array of Attributes or NULL
         */
        IMPORT_C const CArrayPtr< CCMSAttribute >*
        UnauthenticatedAttributes() const;

        /**
         * Setter for version. Default value is 0.
         * @since 3.0
         * @param aVersion CMSVersion value
         */
        IMPORT_C void SetVersion( TInt aVersion );
        
        /**
         * Setter for originatorInfo. Takes ownership.
         * @since 3.0
         * @param aOriginatorInfo OriginatorInfo
         */
        IMPORT_C void SetOriginatorInfoL(
            CCMSOriginatorInfo* aOriginatorInfo );

        /**
         * Setter for recipientInfos. Takes ownership. 
         * @since 3.0
         * @param aRecipientInfos array of RecipientInfos. There must
         * be at least one element in the array, otherwise thois
         * method will leave with KErrArgument
         */
        IMPORT_C void SetRecipientInfosL(
            CArrayPtr< CCMSRecipientInfo >* aRecipientInfos );

        /**
         * Setter for macAlgorithm. Takes ownership.
         * @since 3.0
         * @param aMacAlgorithm MessageAuthenticationCodeAlgorithm. If
         * this parameter is null, this method leaves with KErrArgument.
         */
        IMPORT_C void SetMacAlgorithmL(
            CCMSX509AlgorithmIdentifier* aMacAlgorithm );

        /**
         * Setter for digestAlgorithm. Takes ownership.
         * @since 3.0
         * @param aDigestAlgorithm DigestAlgorithmIdentifier
         */
        IMPORT_C void SetDigestAlgorithmL(
            CCMSX509AlgorithmIdentifier* aDigestAlgorithm );


        /**
         * Setter for encapContentInfo. Takes ownership.
         * @since 3.0
         * @param aEncapContentInfo EncapsulatedContentInfo. If this
         * parameter is NULL, this method will leave with KErrArgument.
         */
        IMPORT_C void SetEncapContentInfoL(
            CCMSEncapsulatedContentInfo* aEncapContentInfo );

        /**
         * Setter for authenticatedAttributes. Takes ownership.
         * @since 3.0
         * @param aAuthenticatedAttributes array of attributes.
         */
        IMPORT_C void SetAuthenticatedAttributesL(
            CArrayPtr< CCMSAttribute >* aAuthenticatedAttributes );

        /**
         * Setter for mac. Creates a copy.
         * @since 3.0
         * @param aMac MessageAuthenticationCode
         */
        IMPORT_C void SetMacL( const TDesC8& aMac );

        /**
         * Setter for unauthenticatedAttributes. Takes ownership.
         * @since 3.0
         * @param aUnuthenticatedAttributes array of attributes.
         */
        IMPORT_C void SetUnauthenticatedAttributesL(
            CArrayPtr< CCMSAttribute >* aUnauthenticatedAttributes );

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
        IMPORT_C void ConstructL();

	   /**
        * Protected construction to allow derivation
        */
        IMPORT_C void ConstructL(
			CArrayPtr< CCMSRecipientInfo >* aRecipientInfos,
            CCMSX509AlgorithmIdentifier* aMacAlgorithm,
        	CCMSEncapsulatedContentInfo* aEncapContentInfo,
        	const TDesC8& aMac );
        
        /**
        * C++ default constructor.
        */
        IMPORT_C CCMSAuthenticatedData();

    private: // New functions

        /**
         * Decodes an attribute sequence.
         */
        CArrayPtrFlat< CCMSAttribute >* DecodeAttributesL(
            TASN1DecGeneric* aAttributesDec );
        
    private:    // Data
        // Contains the actual data, ownedw
        CAuthenticatedDataData* iData;

        // Reserved for extensions
        MCMSAuthenticatedDataExtension* iReserved;
        
    };

#endif      // CCMSAuthenticatedData_H

// End of File
