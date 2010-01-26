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
* Description:  CMS KeyIdentifier type
*
*/



#ifndef CCMSKeyIdentifier_H
#define CCMSKeyIdentifier_H

#include "CCMSSequence.h"

// FORWARD DECLARATIONS
// Extension
class MCMSKeyIdentifierExtension;

// CLASS DECLARATION
/**
*  This module handles two ASN.1 types, RecipientKeyIdentifier and
*  KEKIdentifier.
*
*  ASN.1 definitions:
*    RecipientKeyIdentifier ::= SEQUENCE {
*        subjectKeyIdentifier SubjectKeyIdentifier,
*        date GeneralizedTime OPTIONAL,
*        other OtherKeyAttribute OPTIONAL }
*
*    SubjectKeyIdentifier ::= OCTET STRING
*
*    KEKIdentifier ::= SEQUENCE {
*        keyIdentifier OCTET STRING,
*        date GeneralizedTime OPTIONAL,
*        other OtherKeyAttribute OPTIONAL }
*
*  @lib cms.lib
*  @since 3.0
*/
class CCMSKeyIdentifier : public CCMSSequence
    {
    public:  // Constructors and destructor
		/**
        * Two-phased constructor.
        */
        IMPORT_C static CCMSKeyIdentifier* NewL();

        /**
        * Two-phased constructor. Copies of the parameters are made.
        *
        * @param aKeyIdentifier identifies the recipient's
        * certificate or the the key-encryption key that was
        * previously distributed to the sender and one or more recipients.
        */
        IMPORT_C static CCMSKeyIdentifier* NewL(
            const TDesC8& aKeyIdentifier );

        /**
        * Two-phased constructor. Copies of the parameters are made.
        *
        * @param aKeyIdentifier identifies the recipient's
        * certificate or the the key-encryption key that was
        * previously distributed to the sender and one or more recipients.
        * @param aDate specifies which of the
        * recipient's previously distributed UKMs was used by the
        * sender. In case of KEKIdentifier, specifies a single
        * key-encryption key from a set.
        */
        IMPORT_C static CCMSKeyIdentifier* NewL(
            const TDesC8& aKeyIdentifier,
            const TTime& aDate );

        /**
        * Destructor.
        */
        virtual ~CCMSKeyIdentifier();

    public: // New functions
        /**
        * Getter for KeyIdentifier
        * @since 3.0
        * @return KeyIdentifier
        */
		IMPORT_C const TDesC8& KeyIdentifier() const;

        /**
        * Getter for Date. Ownership is not transferred.
        * @since 2,8
        * @return Date. Ownership is not transferred.
        */
		IMPORT_C const TTime* Date() const;

		/**
        * Setter for KeyIdentifier. Creates a copy of the parameters.
        * @since 3.0
        * @param aKeyIdentifier KeyIdentifier value.
        */
		IMPORT_C void SetKeyIdentifierL(
            const TDesC8& aKeyIdentifier );

        /**
        * Setter for Date. Creates a copy of the parameters.
        * @since 3.0
        * @param aDate Date value
        */
		IMPORT_C void SetDateL( const TTime& aDate );

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
        IMPORT_C void ConstructL( const TDesC8& aKeyIdentifier );

       /**
        * Protected construction to allow derivation
        */
        IMPORT_C void ConstructL( const TDesC8& aKeyIdentifier,
                                  const TTime& aDate );

        /**
        * C++ default constructor.
        */
        IMPORT_C CCMSKeyIdentifier();

    private: // New Methods

        /**
         * Decodes internal data from a array of ASN.1 decoders
         * @param aItems items to internalize
         */
        void DecodeArrayL( CArrayPtr< TASN1DecGeneric >* aItems );

    private:    // Data
        // KeyIdentifier, owned
        TDesC8* iKeyIdentifier;
        // Date, owned
		TTime* iDate;

        // Reserved for extensions
        MCMSKeyIdentifierExtension* iReserved;
        
    };

#endif      // CCMSKeyIdentifier_H

// End of File
