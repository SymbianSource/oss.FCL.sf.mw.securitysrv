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



#ifndef CCMSEncapsulatedContentInfo_H
#define CCMSEncapsulatedContentInfo_H

//  INCLUDES
#include "CCMSContentInfo.h"

// FORWARD DECLARATIONS
// Extension
class MCMSEncapsulatedContentInfoExtension;


// CLASS DECLARATION
/**
*  EncapsulatedContentInfo module
*
*  ASN.1 Definition:
*  EncapsulatedContentInfo ::= SEQUENCE {
*       eContentType ContentType,
*       eContent [0] EXPLICIT OCTET STRING OPTIONAL }
*
*  ContentType ::= OBJECT IDENTIFIER
*
*
*  @lib cms.lib
*  @since 2.8
*/
class CCMSEncapsulatedContentInfo : public CCMSContentInfo
    {
    public:  // Constructors and destructor
		/**
        * Two-phased constructor.
        */
        IMPORT_C static CCMSEncapsulatedContentInfo* NewL();

		/**
        * Two-phased constructor. Leaves the created object in cleanup stack.
        */
        IMPORT_C static CCMSEncapsulatedContentInfo* NewLC();

        /**
        * Two-phased constructor.
        * Takes copy of the both contetn type and content.
        * @param aAttributeType	The attribute type in desc e.g.
        *						content type = "1.2.840.113549.1.7.1" (ID-Data).
        * @param aContent		Content of this ContentInfo. Can be also NULL
        */
        IMPORT_C static CCMSEncapsulatedContentInfo* NewL( const TDesC& aContentType,
											  				const TDesC8* aContent );

        /**
        * Two-phased constructor.
        * Takes copy of the both contetn type and content. Leaves the
        * created object in cleanup stack.
        * @param aAttributeType	The attribute type in desc e.g.
        *						content type = "1.2.840.113549.1.7.1" (ID-Data).
        * @param aContent		Content of this ContentInfo. Can be also NULL
        */
        IMPORT_C static CCMSEncapsulatedContentInfo* NewLC( const TDesC& aContentType,
											  				const TDesC8* aContent );

        /**
        * Destructor.
        */
        virtual ~CCMSEncapsulatedContentInfo();

    public: // New functions

        /**
        * Getter for content
        * @since 2.8
        * @return Content in TDesC8 reference
        */
		IMPORT_C const TDesC8* Content() const;

        /**
        * Setter for content, takes copy
        * @since 2,8
        * @param aContent Content to be set, can be NULL
        */
		IMPORT_C void SetContentL( const TDesC8* aContent );

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
        IMPORT_C void ConstructL();

	   /**
        * Protected construction to allow derivation
        */
        IMPORT_C void ConstructL( const TDesC& aContentType,
								  const TDesC8* aContent );

        /**
        * C++ default constructor.
        */
        IMPORT_C CCMSEncapsulatedContentInfo();

    private:    // Data

		// Reserved for extensions
		MCMSEncapsulatedContentInfoExtension* iReserved;
    };

#endif      // CCMSEncapsulatedContentInfo_H

// End of File
