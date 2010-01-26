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
* Description:  X.509 GeneralNames type
*
*/



#ifndef CCMSX509GeneralNames_H
#define CCMSX509GeneralNames_H

#include "CCMSSequence.h"

// FORWARD DECLARATIONS
class CCMSX509GeneralName;

// Extensions
class MCMSX509GeneralNamesExtension;

// CLASS DECLARATION
/**
*  X.509 GeneralNames type module
*
*  ASN.1 definition:
*    GeneralNames ::= SEQUENCE SIZE (1..MAX) OF GeneralName
*
*  @lib cms.lib
*  @since 3.0
*/
class CCMSX509GeneralNames : public CCMSSequence
    {
    public:  // Constructors and destructor
		/**
        * Two-phased constructor.
        */
        IMPORT_C static CCMSX509GeneralNames* NewL();

        /**
        * Two-phased constructor. Copies of the parameters are made.
        *
        * @param aGeneralNames array of GeneralNames.
        */
        IMPORT_C static CCMSX509GeneralNames* NewL(
            const CArrayPtr< CCMSX509GeneralName >& aGeneralNames );

        /**
         * Two-phased copy constructor.
         * Note: You can't copy an empty GeneralNames. The only way to
         * create an empty GeneralNames (usable only for decoding) is
         * through the parameterless version of NewL().
         * @param aGeneralNames source to be copied
         */
        IMPORT_C static CCMSX509GeneralNames* NewL(
            const CCMSX509GeneralNames& aGeneralNames );
        
        /**
        * Destructor.
        */
        virtual ~CCMSX509GeneralNames();

    public: // New functions
        /**
        * Getter for GeneralNames
        * @since 3.0
        * @return array of GeneralNames
        */
		IMPORT_C const CArrayPtr< CCMSX509GeneralName >& GeneralNames() const;

		/**
        * Setter for GeneralNames. Creates a copy of the parameters.
        * @since 3.0
        * @param aGeneralNames array of GeneralNames
        */
		IMPORT_C void SetGeneralNamesL(
            const CArrayPtr< CCMSX509GeneralName >& aGeneralNames );

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
            const CArrayPtr< CCMSX509GeneralName >& aGeneralNames );

        /**
        * C++ default constructor.
        */
        IMPORT_C CCMSX509GeneralNames();

    private:    // Data
        // GeneralNames, owned
        CArrayPtr< CCMSX509GeneralName >* iGeneralNames;

        // Reserved for extensions
        MCMSX509GeneralNamesExtension* iReserved;
    };

#endif      // CCMSX509GeneralNames_H

// End of File
