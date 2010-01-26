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


#ifndef CCMSRecipientInfo_H
#define CCMSRecipientInfo_H

//  INCLUDES
#include 	"CCMSSequence.h"

// FORWARD DECLARATIONS
class CCMSX509AlgorithmIdentifier;

// CLASS DECLARATION
/**
*  Base class for RecipientInfo -type modules
*
*  @lib cms.lib
*  @since 2.8
*/
class CCMSRecipientInfo : public CCMSSequence
    {

    public: // Destructor

        /**
        * Destructor.
        */
        virtual ~CCMSRecipientInfo();
        
    public: // New functions

        /**
         * Returns the encoder wrapped within appropriate tag.
         * Must be implemented in derived classes.
         * @since 2.8
         * @return encoder for a tagged object
         */
        virtual CASN1EncBase* TaggedEncoderLC() const = 0;
        
        /**
         * Getter for version number.
         * @since 2.8
         * @return version (0, 2, 3 or 4)
         */
        IMPORT_C TInt Version() const;

        /**
         * Getter for keyEncryptionAlgorithm
         * @since 2.8
         * @return Algorithm identifier
         */
        IMPORT_C const CCMSX509AlgorithmIdentifier&
        KeyEncryptionAlgorithm() const;

        /**
         * Setter for keyEncryptionAlgorithm. Makes a copy.
         * @since 2.8
         * @param aKeyEncryptionAlgorithm Algorithm identifier
         */
        IMPORT_C void SetKeyEncryptionAlgorithmL(
            const CCMSX509AlgorithmIdentifier& aKeyEncryptionAlgorithm );
        
    protected:  // New functions
       /**
        * C++ default constructor.
        */
        CCMSRecipientInfo( TInt aVersion );

        /**
         * Constructs the member variables. Makes a copy of
         * aKeyEncryptionAlgorithm.
         */
        void BaseConstructL(
            const CCMSX509AlgorithmIdentifier& aKeyEncryptionAlgorithm );

        /**
         * Adds version encoding to root sequence
         * @param aRoot sequence where version encoding is added
         */
        void AddVersionL( CASN1EncSequence* aRoot ) const;

        /**
         * Adds keyEncryptionAlgorithm encoding to root sequence
         * @param aRoot sequence where keyEncryptionAlgorithm encoding
         * is added
         */
        void AddKeyEncryptionAlgorithmL( CASN1EncSequence* aRoot ) const;
        
    protected: // Data

        // syntax version number.
        TInt iVersion;

        // KeyEncryptionAlgorithmIdentifier, owned
        CCMSX509AlgorithmIdentifier* iKeyEncryptionAlgorithm;
        
    };

#endif      // CCMSRecipientInfo_H

// End of File
