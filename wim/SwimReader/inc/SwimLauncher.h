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
* Description:  For launching SwimReader
*
*/


#ifndef CSWIMREADERLAUNCHER_H
#define CSWIMREADERLAUNCHER_H

//  INCLUDES
#include "ScardReaderLauncher.h"

/**
*  Launcher class
*  
*
*  @lib SwimReader.lib
*  @since Series60 2.1
*/
class CSwimReaderLauncher : public MScardReaderLauncher, public CBase 
    {
    public: // Constructors and destructor
     
        /**
        * C++ default constructor.
        */
        CSwimReaderLauncher();

        /**
        * Destructor.
        */
        virtual ~CSwimReaderLauncher();
    
    public: // New functions
        
        /**
        * Symbian 2nd phase constructor
        */
        void ConstructL( MScardReaderService* aService );

        /**
        * Delete reader 
        * @param  aReader Reader ID to be deleted
        * @return void
        */
        void DeleteReader( TReaderID aReaderID );
        
        /**
        * Create new reader object
        * @param aReaderID reader's ID 
        * @return pointer to created Scard reader
        */    
        MScardReader* CreateReaderL( TReaderID aReaderID );
 
    private:    // Data
        // Pointer to reader object. Owned.
        MScardReader*        iReader;
        // Pointer to reader service object. Not owned.
        MScardReaderService* iService;
    };

#endif      // CSWIMREADERLAUNCHER_H

// End of File
