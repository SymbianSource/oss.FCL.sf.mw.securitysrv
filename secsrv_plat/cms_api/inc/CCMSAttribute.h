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



#ifndef CCMSAttribute_H
#define CCMSAttribute_H

//  INCLUDES
#include "CCMSSequence.h"
#include <badesca.h>

// FORWARD DECLARATIONS
// Extension
class MCMSAttributeExtension;


// CLASS DECLARATION
/**
*  Attribute type module
*
*  ASN.1 Definition:
*  Attribute ::= SEQUENCE {
*  		attrType OBJECT IDENTIFIER,
*  		attrValues SET OF AttributeValue }
*
*  AttributeValue ::= ANY
*
*  @lib cms.lib
*  @since 2.8
*/
class CCMSAttribute : public CCMSSequence
    {
    public:  // Constructors and destructor
		/**
        * Two-phased constructor.
        * NOTE: Encrypt will leave with KErrBadDescriptor if
        * valid Attribute Type has not setted.
        */
        IMPORT_C static CCMSAttribute* NewLC();

	   /**
        * Two-phased constructor.
        * NOTE: Encrypt will leave with KErrBadDescriptor if
        * valid Attribute Type has not setted.
        */
        IMPORT_C static CCMSAttribute* NewL();
		
		/**
        * Two-phased constructor.
        * Takes copy of the both type and values.
        * @param aAttributeType	The attribute type in desc e.g.
        *						content type = "1.2.840.113549.1.9.3".
        * @param aAttriteValues	Set of attribute values. Each attribute must be
        *						already encoded since they are added as is.
        */
        IMPORT_C static CCMSAttribute* NewL( const TDesC& aAttributeType,
										     const CDesC8Array& aAttributeValues );

        /**
        * Two-phased constructor.
        * Takes copy of the both type and values.
		* Leaves newly created instance in CleanupStack
        * @param aAttributeType	The attribute type in desc e.g.
        *						content type = "1.2.840.113549.1.9.3".
        * @param aAttriteValues	Set of attribute values. Each attribute must be
        *						already encoded since they are added as is.
        */
        IMPORT_C static CCMSAttribute* NewLC( const TDesC& aAttributeType,
											  const CDesC8Array& aAttributeValues );

		/**
        * Two-phased constructor.
        * Same as above although takes only one value
        * @param aAttributeType	The attribute type in desc e.g.
        *						content type = "1.2.840.113549.1.9.3".
        * @param aAttriteValue	Single attribute value. Attribute must be
        *						already encoded since it is added as is.
        */
        IMPORT_C static CCMSAttribute* NewL( const TDesC& aAttributeType,
											 const TDesC8& aAttributeValue );

		/**
        * Two-phased constructor.
        * Same as above although takes only one value
		* Leaves newly created instance in CleanupStack
        * @param aAttributeType	The attribute type in desc e.g.
        *						content type = "1.2.840.113549.1.9.3".
        * @param aAttriteValue	Single attribute value. Attribute must be
        *						already encoded since it is added as is.
        */
        IMPORT_C static CCMSAttribute* NewLC( const TDesC& aAttributeType,
											  const TDesC8& aAttributeValue );

        /**
        * Destructor.
        */
        virtual ~CCMSAttribute();

    public: // New functions

        /**
		* Getter for attribute type
		* @since 2,8
		* @return Attribute type in TDesC reference
		*/
		IMPORT_C const TDesC& AttributeType() const;

        /**
        * Getter for attribute values
        * @since 2.8
        * @return Attribute values in CDesC8ArrayFlat reference
        */
		IMPORT_C const CDesC8Array& AttributeValues() const;

		/**
        * Setter for attribute type, takes copy
        * @since 2.8
        * @param aAttributeType	Attribute type to be set as TDesC.
        */
		IMPORT_C void SetAttributeTypeL( const TDesC& aAttributeType );

        /**
        * Setter for attribute values
        * @since 2,8
        * @param aSerialNumber Array of attribute values
        */
		IMPORT_C void SetAttributeValuesL( const CDesC8Array& aAttributeValues );

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
        IMPORT_C void ConstructL( const TDesC& aAttributeType,
								  const CDesC8Array& aAttributeValues );

	   /**
        * Protected construction to allow derivation
        */
        IMPORT_C void ConstructL( const TDesC& aAttributeType,
								  const TDesC8& aAttributeValue );

        /**
        * C++ default constructor.
        */
        IMPORT_C CCMSAttribute();

    private:    // Data
        // Attribute Type, owned
        HBufC* iAttributeType;
        // Attribute values, owned
		CDesC8ArrayFlat* iAttributeValues;

		// Reserved for extensions
		MCMSAttributeExtension* iReserved;
    };

#endif      // CCMSAttribute_H

// End of File
