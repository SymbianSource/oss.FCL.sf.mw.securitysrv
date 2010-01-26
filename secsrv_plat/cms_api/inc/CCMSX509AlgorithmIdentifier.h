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



#ifndef CCMSX509AlgorithmIdentifier_H
#define CCMSX509AlgorithmIdentifier_H

//  INCLUDES
#include "CCMSSequence.h"
#include <signed.h>
#include <x509cert.h>

// FORWARD DECLARATIONS
// Extension
class MCMSX509AlgorithmIdentifier;


// CLASS DECLARATION
/**
*  X509 algorith identifier extension
*
*  @lib cms.lib
*  @since 2.8
*/
class CCMSX509AlgorithmIdentifier : public CCMSSequence
    {
    public:  // Constructors and destructor
		/**
        * Two-phased constructor.
        * Default Algorith Identifier is RSA
        */
        IMPORT_C static CCMSX509AlgorithmIdentifier* NewL();

        /**
        * Two-phased constructor.
        * @param aAlgorithmId	Used AlgorithmID
        */
        IMPORT_C static CCMSX509AlgorithmIdentifier* NewL(
											const TAlgorithmId& aAlgorithmId );

		/**
        * Two-phased constructor.
        * @param aAlgorithmIdentifier	The algorithm ID object to be used
        */
        IMPORT_C static CCMSX509AlgorithmIdentifier* NewL(
							const CAlgorithmIdentifier& aAlgorithmIdentifier );

	   /**
        * Two-phased constructor.
        * @param aAlgorithmIdentifier	The algorithm ID for algorithm
		* @param aDigestIdentifier	The algorithm ID for digest
        */
        IMPORT_C static CCMSX509AlgorithmIdentifier* NewL(
							const CAlgorithmIdentifier& aAlgorithmIdentifier,
							const CAlgorithmIdentifier& aDigestIdentifier );

        /**
        * Destructor.
        */
        virtual ~CCMSX509AlgorithmIdentifier();

    public: // New functions

        /**
		* Getter for AlgorithmIdentifier
		* @since 2,8
		* @return Algorithm identifier
		*/
		IMPORT_C const CAlgorithmIdentifier& AlgorithmIdentifier() const;

        /**
        * Setter for AlgorithmIdentifier
        * @since 2.8
		* @param aAlgorithmIdentifier	The algorithm ID
        */
		IMPORT_C void SetAlgorithmIdentifierL(
							const CAlgorithmIdentifier& aAlgorithmIdentifier );

		/**
		* Getter for DigestAlgorithmIdentifier
		* @since 2,8
		* @return Digest algorithm identifier or NULL if absent
		*/
		IMPORT_C const CAlgorithmIdentifier* DigestAlgorithm() const;

        /**
        * Setter for DigestAlgorithmIdentifier
        * @since 2.8
		* @param aAlgorithmIdentifier The algorithm ID for digest or NULL if
		*							  removing digest algorithm ID
        */
		IMPORT_C void SetDigestAlgorithmL(
							const CAlgorithmIdentifier* aAlgorithmIdentifier );

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
        IMPORT_C void ConstructL( const TAlgorithmId& aAlgorihmId );

	   /**
        * Protected construction to allow derivation
        */
        IMPORT_C void ConstructL( const CAlgorithmIdentifier& aAlgorithmIdentifier );

        /**
        * C++ default constructor.
        */
        IMPORT_C CCMSX509AlgorithmIdentifier();

    private:    // Data
        // Algorithm identifier, owned
        CAlgorithmIdentifier* iAlgorithmIdentifier;

		// Digest algorithm identifier, owned
        CAlgorithmIdentifier* iDigestAlgorithmIdentifier;

		// Reserved for extensions
		MCMSX509AlgorithmIdentifier* iReserved;
    };

#endif      // CCMSX509AlgorithmIdentifier_H

// End of File
