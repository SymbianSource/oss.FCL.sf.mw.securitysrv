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
* Description:  X.509 General Name type
*
*/



#ifndef CCMSX509GeneralName_H
#define CCMSX509GeneralName_H

// INCLUDES
#include <e32base.h>
#include <asn1cons.h>
#include "MCMSModule.h"

// FORWARD DECLARATIONS
// Extensions
class MCMSX509GeneralNameExtension;

// CLASS DECLARATION
/**
*  X.509 GeneralName type module
*
*  ASN.1 definition:
*  GeneralName ::= CHOICE {
*    otherName                  [0]  INSTANCE OF OTHER-NAME,
*    rfc822Name                 [1]  IA5String,
*    dNSName                    [2]  IA5String,
*    x400Address                [3]  ORAddress,
*    directoryName              [4]  Name,
*    ediPartyName               [5]  EDIPartyName,
*    uniformResourceIdentifier  [6]  IA5String,
*    iPAddress                  [7]  OCTET STRING,
*    registeredID               [8]  OBJECT IDENTIFIER
*  }
*
*
*  @lib cms.lib
*  @since 3.0
*/
class CCMSX509GeneralName : public CBase, public MCMSModule
    {

    public:  // Constructors and destructor
		/**
        * Two-phased constructor.
        */
        IMPORT_C static CCMSX509GeneralName* NewL();

        /**
        * Two-phased constructor. Copies of the parameters are made.
        *
        * @param aTag the type of the choice
        * @param aData The data in DER encoded format
        */
        IMPORT_C static CCMSX509GeneralName* NewL(
            const TTagType aTag,
            const TDesC8& aData );

        /**
        * Destructor.
        */
        virtual ~CCMSX509GeneralName();

    public: // New functions

        /**
        * Getter for the tag.
        * @since 3.0
        * @return TTagType
        */
		IMPORT_C TTagType Tag() const;

        /**
        * Getter for the data
        * @since 3.0
        * @return data in DER encoded format
        */
		IMPORT_C const TDesC8& Data() const;

		/**
        * Setter for the tag
        * @since 3.0
        * @param aTag tag of the choice
        */
		IMPORT_C void SetTagL(
            const TTagType aTag );

        /**
        * Setter for data
        * @since 3.0
        * @param aData data of the choice
        */
		IMPORT_C void SetDataL(
            const TDesC8& aData );

    public: // Functions from base classes

        /**
        * From MCMSModule
        * @since 3.0
        * @param aResult Allocates and sets result in this variable
        */
        void EncodeL( HBufC8*& aResult ) const;

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
            const TTagType aTag,
            const TDesC8& aData );

        /**
        * C++ default constructor.
        */
        IMPORT_C CCMSX509GeneralName( );


    private:    // Data

        // defines the type of this CHOICE
        TTagType iTag;

        // Data
        HBufC8* iData;

        // Reserved for extensions
        MCMSX509GeneralNameExtension* iReserved;
        
    };

#endif      // CCMSX509GeneralName_H

// End of File
