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



#ifndef CCMSX509Validity_H
#define CCMSX509Validity_H

#include "CCMSSequence.h"

// FORWARD DECLARATIONS
class CValidityPeriod;

// CLASS DECLARATION
/**
*  X.509 Validity type module
*
*  ASN.1 definition:
* Validity ::= SEQUENCE {notBefore  Time,
*                        notAfter   Time
* }
*
*  @lib cms.lib
*  @since 2.8
*/
class CCMSX509Validity : public CCMSSequence
    {

    public:  // Constructors and destructor
		/**
        * Two-phased constructor.
        */
        IMPORT_C static CCMSX509Validity* NewL();

        /**
        * Two-phased constructor.
        * 
        * @param aNotBefore notBefore time
        * @param aNotAfter notAfter time
        */
        IMPORT_C static CCMSX509Validity* NewL(
            const TTime& aNotBefore,
            const TTime& aNotAfter );

        /**
         * Two-phased constructor.
         *
         * @param aValidityPeriod CValidityPeriod which the data is copied from.
         */
        IMPORT_C static CCMSX509Validity* NewL(
            const CValidityPeriod& aValidityPeriod );
        
        /**
        * Destructor.
        */
        virtual ~CCMSX509Validity();

    public: // New functions

        /**
        * Getter for notBefore.
        * @since 2.8
        * @return notBefore time value
        */
		IMPORT_C const TTime& NotBefore() const;

        /**
        * Getter for notAfter.
        * @since 2.8
        * @return notAfter time value
        */
		IMPORT_C const TTime& NotAfter() const;

        /**
        * Setter for notBefore.
        * @since 2.8
        * @param aNotBefore notBefore time
        */
		IMPORT_C void
        SetNotBefore( const TTime& aNotBefore );

        /**
        * Setter for notAfter.
        * @since 2.8
        * @param aNotAfter notAfter time
        */
		IMPORT_C void SetNotAfter( const TTime& aNotAfter );

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
        * C++ constructor.
        */
        IMPORT_C CCMSX509Validity( const TTime& aNotBefore,
                                   const TTime& aNotAfter );

        /**
        * C++ default constructor.
        */
        IMPORT_C CCMSX509Validity( );

    private:    // Data
        // notBefore
        TTime iNotBefore;

        // notAfter
        TTime iNotAfter;
        
    };

#endif      // CCMSX509Validity_H

// End of File
