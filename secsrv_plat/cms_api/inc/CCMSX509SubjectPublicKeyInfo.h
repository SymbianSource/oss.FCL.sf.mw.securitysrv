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
* Description: 
*
*/



#ifndef CCMSX509SubjectPublicKeyInfo_H
#define CCMSX509SubjectPublicKeyInfo_H

#include "CCMSSequence.h"

// FORWARD DECLARATIONS
class CCMSX509AlgorithmIdentifier;
class CSubjectPublicKeyInfo;

// CLASS DECLARATION
/**
*  X.509 SubjectPublicKeyInfo type module
*
*  ASN.1 definition:
*   SubjectPublicKeyInfo ::= SEQUENCE {
*    algorithm         AlgorithmIdentifier,
*    subjectPublicKey  BIT STRING
*   }
*
*  @lib cms.lib
*  @since 2.8
*/
class CCMSX509SubjectPublicKeyInfo : public CCMSSequence
    {

    public:  // Constructors and destructor
		/**
        * Two-phased constructor.
        */
        IMPORT_C static CCMSX509SubjectPublicKeyInfo* NewL();

        /**
        * Two-phased constructor. Copies of the parameters are made.
        * 
        * @param aAlgorithm AlgorithmIdentifier
        * @param aSubjectPublicKey bit string
        */
        IMPORT_C static CCMSX509SubjectPublicKeyInfo* NewL(
            const CCMSX509AlgorithmIdentifier& aAlgorithm,
            const TDesC8& aSubjectPublicKey );

        /**
         * Two-phased constructor.
         *
         * @param aSubjectPublicKeyInfo CSubjectPublicKeyInfo which
         * the data is copied from.
         */
        IMPORT_C static CCMSX509SubjectPublicKeyInfo* NewL(
            const CSubjectPublicKeyInfo& aSubjectPublicKeyInfo );
        
        /**
        * Destructor.
        */
        virtual ~CCMSX509SubjectPublicKeyInfo();

    public: // New functions

        /**
        * Getter for algorithm.
        * @since 2.8
        * @return X.509 AlgorithmIdentifier value
        */
		IMPORT_C const CCMSX509AlgorithmIdentifier& Algorithm() const;

        /**
        * Getter for subjectPublicKey.
        * @since 2.8
        * @return bit string in a descriptor
        */
		IMPORT_C const TDesC8& SubjectPublicKey() const;

        /**
        * Setter for algorithm. Creates a copy.
        * @since 2.8
        * @param aAlgorithm X.509 AlgorithmIdentifier value
        */
		IMPORT_C void
        SetAlgorithmL( const CCMSX509AlgorithmIdentifier& aAlgorithm );

        /**
        * Setter for subjectPublicKey.
        * @since 2.8
        * @param aSubjectPublicKey bit string
        */
		IMPORT_C void SetSubjectPublicKeyL( const TDesC8& aSubjectPublicKey );

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
        IMPORT_C void ConstructL(
            const CCMSX509AlgorithmIdentifier& aAlgorithm,
            const TDesC8& aSubjectPublicKey );
       /**
        * Protected construction to allow derivation
        */
        IMPORT_C void ConstructL(
            const CSubjectPublicKeyInfo& aSubjectPublicKeyInfo );
        
        /**
         * Protected construction to allow derivation
         */
        IMPORT_C void ConstructL( );
        
        /**
        * C++ default constructor.
        */
        IMPORT_C CCMSX509SubjectPublicKeyInfo( );

    private:    // Data
        // AlgorithmIdentifier, owned
        CCMSX509AlgorithmIdentifier* iAlgorithm;

        // subjectPublicKey, owned
        HBufC8* iSubjectPublicKey;

    };

#endif      // CCMSX509SubjectPublicKeyInfo_H

// End of File
