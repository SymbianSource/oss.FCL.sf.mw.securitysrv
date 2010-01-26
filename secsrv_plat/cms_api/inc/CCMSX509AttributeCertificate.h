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
* Description:  X.509 AttributeCertificate type
*
*/



#ifndef CCMSX509AttributeCertificate_H
#define CCMSX509AttributeCertificate_H

#include "CCMSSequence.h"
#include "CCMSX509Signed.h"

// FORWARD DECLARATIONS
class CCMSX509AttributeCertificateInfo;
class CCMSX509AlgorithmIdentifier;

// Extensions
class MCMSX509AttributeCertificateExtension;

// CLASS DECLARATION
/**
*  X.509 AttributeCertificateInfo type module
*
*  ASN.1 definition:
* AttributeCertificate ::= SIGNED{AttributeCertificateInfo}
*
* SIGNATURE{ToBeSigned} ::= SEQUENCE {
*  algorithmIdentifier  AlgorithmIdentifier,
*  encrypted            ENCRYPTED-HASH{ToBeSigned}
* }
*
* SIGNED{ToBeSigned} ::= SEQUENCE {
*  toBeSigned  ToBeSigned,
*  COMPONENTS OF SIGNATURE{ToBeSigned}
* }
*
*  @lib cms.lib
*  @since 3.0
*/
class CCMSX509AttributeCertificate : public CCMSX509Signed
    {

    public:  // Constructors and destructor
		/**
        * Two-phased constructor.
        */
        IMPORT_C static CCMSX509AttributeCertificate* NewL();

        /**
        * Two-phased constructor. Copies of the parameters are made.
        * 
        * @param aInfo AttributeCertificateInfo, contains the actual
        * data of the certificate.
        * @param aAlgorithmIdentifier identifies the algorithm used in
        * the signature 
        * @param aEncrypted must be the result of applying a hashing
        * procedure to the DER-encoded octets of a value of
        * aInfo and then applying an encipherment
        * procedure to those octets
        */
        IMPORT_C static CCMSX509AttributeCertificate* NewL(
            const CCMSX509AttributeCertificateInfo& aInfo,
            const CCMSX509AlgorithmIdentifier& aAlgorithmIdentifier,
            const TDesC8& aEncrypted
            );
        
        /**
        * Destructor.
        */
        virtual ~CCMSX509AttributeCertificate();

    public: // New functions
        /**
        * Getter for the info.
        * @since 3.0
        * @return X.509 AttributeCertificateInfo value
        */
		IMPORT_C const CCMSX509AttributeCertificateInfo& Info() const;

        /**
        * Setter for the info. Creates a copy.
        * @since 3.0
        * @param aInfo X.509 AttributeCertificateInfo
        */
		IMPORT_C void SetInfoL( const CCMSX509AttributeCertificateInfo& aInfo );

        /**
         * Decoder method with implicit tag checking.
         * @since 3.0
         * @param aRawData raw-data to be parsed in this instance
         * @param aImplicitTag tag to be checked
         */
        void DecodeImplicitTagL( const TDesC8& aRawData,
                                 const TTagType aImplicitTag );
        
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
            const CCMSX509AttributeCertificateInfo& aInfo,
            const CCMSX509AlgorithmIdentifier& aAlgorithmIdentifier,
            const TDesC8& aEncrypted
            );

        /**
         * Protected construction to allow derivation
         */
        IMPORT_C void ConstructL( );
        
        /**
        * C++ default constructor.
        */
        IMPORT_C CCMSX509AttributeCertificate( );
        
    private: // New Methods

        /**
         * Decodes internal data from a array of ASN.1 decoders
         * @param aItems items to internalize
         */
        void DecodeArrayL( CArrayPtr< TASN1DecGeneric >* aItems );

    private:    // Data
        // Contains the actual attribute certificate (without
        // signature). Owned.
        CCMSX509AttributeCertificateInfo* iInfo;

        // Reserved for extensions
        MCMSX509AttributeCertificateExtension* iReserved;
        
    };

#endif      // CCMSX509AttributeCertificate_H

// End of File
