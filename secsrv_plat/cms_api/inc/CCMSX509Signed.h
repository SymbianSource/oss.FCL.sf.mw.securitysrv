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


#ifndef CCMSX509Signed_H
#define CCMSX509Signed_H

//  INCLUDES
#include 	"CCMSSequence.h"

// FORWARD DECLARATIONS
class CCMSX509AlgorithmIdentifier;

// CLASS DECLARATION
/**
*  Base class for X.509 SIGNED -type modules
*
*  ASN.1 notation:
*
* SIGNATURE{ToBeSigned} ::= SEQUENCE {
*  algorithmIdentifier  AlgorithmIdentifier,
*  encrypted            ENCRYPTED-HASH{ToBeSigned}
* }
* 
* SIGNED{ToBeSigned} ::= SEQUENCE {
*   toBeSigned  ToBeSigned,
*   COMPONENTS OF SIGNATURE{ToBeSigned}
* }
*
*  @lib cms.lib
*  @since 2.8
*/
class CCMSX509Signed : public CCMSSequence
    {

    public: // Destructor

        /**
        * Destructor.
        */
        virtual ~CCMSX509Signed();
        
    public: // New functions

        /**
         * Getter for algorithmIdentifier
         * @since 2.8
         * @return Algorithm identifier
         */
        IMPORT_C const CCMSX509AlgorithmIdentifier&
        AlgorithmIdentifier() const;

        /**
         * Getter for encrypted
         * @since 2.8
         * @return encrypted (BIT STRING in a descriptor)
         */
        IMPORT_C const TDesC8& Encrypted() const;
        
        /**
         * Setter for algorithmIdentifier. Makes a copy.
         * @since 2.8
         * @param aAlgorithmIdentifier Algorithm identifier
         */
        IMPORT_C void SetAlgorithmIdentifierL(
            const CCMSX509AlgorithmIdentifier& aAlgorithmIdentifier );

        /**
         * Setter for encrypted. Makes a copy.
         * @since 2.8
         * @param aEncrypted bit string
         */
        IMPORT_C void SetEncryptedL( const TDesC8& aEncrypted );
        
    protected:  // New functions
       /**
        * C++ default constructor.
        */
        CCMSX509Signed( );

        /**
         * Constructs the member variables. Makes a copy of
         * the parameters
         */
        void BaseConstructL(
            const CCMSX509AlgorithmIdentifier& aAlgorithmIdentifier,
            const TDesC8& aEncrypted );

        /**
         * Creates a signed encoder.
         * @param aToBeSigned ToBeSigned (see the ASN.1 notation). The
         * object is popped from the cleanup stack.
         * @return signed sequence, left in the cleanup stack. 
         */
        CASN1EncSequence* SignAndPopLC( CASN1EncBase* aToBeSigned ) const;

        /**
         * Decodes algorithmIdentifier and encrypted parts from the
         * raw data.
         * @param aRawData data to be decoded
         * @param aAlgorithmIdentifier decoded algorithmIdentifier is
         * stored here
         * @param aEncrypted decoded encrypted is stored here
         * @return ToBeSigned in a generic decoder.
         */
        TASN1DecGeneric DecodeSignatureL(
            const TDesC8& aRawData,
            CCMSX509AlgorithmIdentifier*& aAlgorithmIdentifier,
            HBufC8*& aEncrypted ) const;

        /**
         * Decodes algorithmIdentifier and encrypted parts from an
         * array of items. The first item is returned, and the second
         * and third items are used in decoding process.
         * @param aItemArray array of items
         * @param aAlgorithmIdentifier decoded algorithmIdentifier is
         * stored here
         * @param aEncrypted decoded encrypted is stored here
         * @return ToBeSigned in a generic decoder.
         */
        TASN1DecGeneric DecodeSignatureArrayL(
            CArrayPtr< TASN1DecGeneric >& aItemArray,
            CCMSX509AlgorithmIdentifier*& aAlgorithmIdentifier,
            HBufC8*& aEncrypted ) const;

        
        
    protected: // Data

        // algorithmIdentifier, owned
        CCMSX509AlgorithmIdentifier* iAlgorithmIdentifier;

        // encrypted, owned
        HBufC8* iEncrypted;
        
    };

#endif      // CCMSX509Signed_H

// End of File
