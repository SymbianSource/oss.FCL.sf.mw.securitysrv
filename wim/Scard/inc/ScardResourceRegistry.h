/*
* Copyright (c) 2003 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Registry of smart card resources.
*
*/



#ifndef CSCARDRESOURCEREGISTRY_H
#define CSCARDRESOURCEREGISTRY_H

//  INCLUDES
#include "ScardDefs.h"

//  FORWARD DECLARATIONS
class CScardServer;
class MScardReader;

// CLASS DECLARATION

/**
*  Manages resources (e.g. cards and service providers).
*
*  @lib Scard.lib
*  @since Series60 2.1
*/
class CScardResourceRegistry : public CBase
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        * @param aServer Pointer to CScardServer object
        */
        static CScardResourceRegistry* NewL( CScardServer* aServer ); 

        /**
        * Destructor.
        */
        virtual ~CScardResourceRegistry();

    public: // New functions

        /**
        * Handle card event
        * @param aEvent Card event
        * @param aReaderID Reader ID
        * @return void
        */
        void CardEvent( const TScardServiceStatus aEvent, 
                        const TReaderID aReaderID );

    private:
        
        /**
        * C++ default constructor.
        */
        CScardResourceRegistry();

        /**
        * By default Symbian 2nd phase constructor is private.
        * @param aServer Pointer to CScardServer object
        */
        void ConstructL( CScardServer* aServer );
 
    private:    // Data
        // Pointer to server object. Not owned.
        CScardServer* iServer;
    };

#endif      // CSCARDRESOURCEREGISTRY_H

// End of File
