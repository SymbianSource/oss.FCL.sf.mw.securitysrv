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



#ifndef CCMSContentInfo_H
#define CCMSContentInfo_H

//  INCLUDES
#include "CCMSSequence.h"

// FORWARD DECLARATIONS
// Extension
class MCMSContentInfoExtension;

// CLASS DECLARATION
/**
*  ContentInfo module
*
*  ASN.1 Definition:
*  ContentInfo ::= SEQUENCE {
*       contentType ContentType,
*       content [0] EXPLICIT ANY DEFINED BY contentType }
*
*  ContentType ::= OBJECT IDENTIFIER
*
*
*  @lib cms.lib
*  @since 2.8
*/
class CCMSContentInfo : public CCMSSequence
    {
    public:  // Destructor
        
		/**
        * Two-phased constructor. Leaves the created object in cleanup stack.
        */
        IMPORT_C static CCMSContentInfo* NewLC();

        /**
        * Two-phased constructor.
        * Takes copy of the both contetn type and content. Leaves the
        * created object in cleanup stack.
        * @param aAttributeType	The attribute type in desc e.g.
        *						content type = "1.2.840.113549.1.7.1" (ID-Data).
        * @param aContent		Content of this ContentInfo.
        */
        IMPORT_C static CCMSContentInfo* NewLC( const TDesC& aContentType,
                                                const TDesC8& aContent );
        
        /**
        * Destructor.
        */
        virtual ~CCMSContentInfo();

    public: // New functions

        /**
		* Getter for content type
		* @since 2,8
		* @return Content type in TDesC reference
		*/
		IMPORT_C const TDesC& ContentType() const;


		/**
        * Setter for content type, takes copy
        * @since 2.8
        * @param aContentType	Content type to be set as TDesC.
        */
		IMPORT_C void SetContentTypeL( const TDesC& aAttributeType );

        /**
        * Getter for content
        * @since 3.0
        * @return Content in TDesC8 reference
        */
		IMPORT_C const TDesC8& Content() const;

        /**
        * Setter for content, takes copy
        * @since 3.0
        * @param aContent Content to be set
        */
		IMPORT_C void SetContentL( const TDesC8& aContent );

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
        IMPORT_C void ConstructL();

	   /**
        * Protected construction to allow derivation
        */
        IMPORT_C void ConstructL( const TDesC& aContentType );

        /**
        * Protected construction to allow derivation
        */
        IMPORT_C void ConstructL( const TDesC& aContentType,
                                  const TDesC8& aContent );
        
        /**
        * C++ default constructor.
        */
        IMPORT_C CCMSContentInfo();

    protected:    // Data
        // Content Type, owned
        HBufC* iContentType;

        // Content, owned.
        HBufC8* iContent;

        // Reserved for extensions
        MCMSContentInfoExtension* iReserved;
    };

#endif      // CCMSContentInfo_H

// End of File
