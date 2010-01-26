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



#ifndef CCMSOriginatorPublicKey_H
#define CCMSOriginatorPublicKey_H

#include "CCMSSequence.h"

// FORWARD DECLARATIONS
class CCMSX509AlgorithmIdentifier;

// CLASS DECLARATION
/**
*  OriginatorPublicKey type module
*
*  ASN.1 definition:
*  OriginatorPublicKey ::= SEQUENCE {
*    algorithm AlgorithmIdentifier,
*    publicKey BIT STRING }
*
*  @lib cms.lib
*  @since 2.8
*/
class CCMSOriginatorPublicKey : public CCMSSequence
    {
    public:  // Constructors and destructor
		/**
        * Two-phased constructor.
        */
        IMPORT_C static CCMSOriginatorPublicKey* NewL();

        /**
         * Two-phased copy constructor.
         */
        IMPORT_C static CCMSOriginatorPublicKey* NewL(
            const CCMSOriginatorPublicKey& aOriginatorPublicKey );

        /**
         * Two-phased copy constructor. Leaves the created object to
         * cleanup stack.
         */
        IMPORT_C static CCMSOriginatorPublicKey* NewLC(
            const CCMSOriginatorPublicKey& aOriginatorPublicKey );

        /**
        * Two-phased constructor. Copies of the parameters are made.
        *
        * @param aAlgorithm Algorithm Identifier.
        * @param aPublicKey The public key. Must be in big-endian order.
        */
        IMPORT_C static CCMSOriginatorPublicKey* NewL(
            const CCMSX509AlgorithmIdentifier& aAlgorithm,
            const TDesC8& aPublicKey );

        /**
        * Two-phased constructor. Copies of the parameters are made.
        * Leaves the created object to cleanup stack
        *
        * @param aAlgorithm Algorithm Identifier.
        * @param aPublicKey The public key. Must be in big-endian order.
        */
        IMPORT_C static CCMSOriginatorPublicKey* NewLC(
            const CCMSX509AlgorithmIdentifier& aAlgorithm,
            const TDesC8& aPublicKey );

        /**
        * Destructor.
        */
        virtual ~CCMSOriginatorPublicKey();

    public: // New functions
        /**
        * Getter for algorithm identifier
        * @since 2.8
        * @return The algorithm identifier
        */
		IMPORT_C const CCMSX509AlgorithmIdentifier& Algorithm() const;

        /**
        * Getter for public key
        * @since 2,8
        * @return public key
        */
		IMPORT_C const TDesC8& PublicKey() const;

		/**
        * Setter for algorithm identifier. Create a copy of the parameters.
        * @since 2.8
        * @param aAlgorithm the algorithm identifier
        */
		IMPORT_C void SetAlgorithmL(
            const CCMSX509AlgorithmIdentifier& aAlgorithm );

        /**
        * Setter for public key. Creates a copy of the parameter.
        * @since 2,8
        * @param aPublicKey the public key in big-endian format
        */
		IMPORT_C void SetPublicKeyL( const TDesC8& aPublicKey );

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
        IMPORT_C void ConstructL( const CCMSX509AlgorithmIdentifier& aAlgorithm,
								  const TDesC8& aPublicKey );

        /**
        * C++ default constructor.
        */
        IMPORT_C CCMSOriginatorPublicKey();

    private:    // Data
        // Issuer name, owned
        CCMSX509AlgorithmIdentifier* iAlgorithm;
        // public key, owned
		HBufC8* iPublicKey;

    };

#endif      // CCMSOriginatorPublicKey_H

// End of File
