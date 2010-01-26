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
* Description: 
*
*/



#ifndef MCMSModule_H
#define MCMSModule_H

//  INCLUDES
#include <e32std.h>

// FORWARD DECLARATIONS
class CASN1EncBase;


// CLASS DECLARATION
/**
*  Base class for all CMS modules
*
*  @lib cms.lib
*  @since 2.8
*/
class MCMSModule
    {

    public: // New functions

        /**
        * To be implemented in derived classes, used to
        * encrypt specified module to descriptor
        * Will leave with KErrArgument if current instance of module is invalid
        * @since 2.8
        * @param aResult Allocates and sets result of encryption in this variable
        */
        virtual void EncodeL( HBufC8*& aResult ) const = 0;

	 	/**
        * To be implemented in derived classes, used to
        * decrypt raw data to current instance.
        * Will leave with KErrArgument if give data is not valid
        * @since 2.8
        * @param aRawData raw-data to be parsed in this instance
        */
        virtual void DecodeL( const TDesC8& aRawData ) = 0;

		/**
		 * To be implemented in derived classes, used to
		 * retrieve ASN1 Encoder
		 * Leaves returned encoder in CleanupStack
		 */
		virtual CASN1EncBase* EncoderLC() const = 0;
    };

#endif      // MCMSModule_H

// End of File
