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
* Description:  Registry of smart card readers.
*
*/



#ifndef CSCARDREADERREGISTRY_H
#define CSCARDREADERREGISTRY_H

//  INCLUDES
#include "ScardDefs.h"
#include <f32file.h>    // For RFs

//  FORWARD DECLARATIONS
class CScardServer;
class MScardReader;
class MScardReaderLauncher;
class CScardAccessControl;

// DATA TYPES
struct TLauncherStruct
    {
    RFs         iSession;
    RLibrary    iLibrary;
    TGroupID    iGroupID;
    TReaderID   iReaderID;
    MScardReaderLauncher* iLauncher;
    TLauncherStruct();
    };

// CLASS DECLARATION

/**
*  Reader registry.
*  This class handles all server-side connections to the reader/card database. 
*  Creates reader handlers, queries whether a given reader is supported and 
*  gives lists of all the currently open or closed readers.
*
*  @lib Scard.lib
*  @since Series60 2.1
*/
class CScardReaderRegistry : public CBase
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        * @param aServer Pointer to CScardServer object
        */
        static CScardReaderRegistry* NewL( CScardServer* aServer ); 

        /**
        * Destructor.
        */
        virtual ~CScardReaderRegistry();

    public: // New functions

        /**
        * Loads reader handler module with given ID.
        * @param aReaderID Reader ID
        * @return pointer to MScardReader object
        */
        MScardReader* LoadReaderL( const TReaderID aReaderID );

        /**
        * Unload the reader module with given ID.
        * @param aReaderID Reader ID
        * @return void
        */
        void CloseReader( const TReaderID aReaderID );

        /**
        * Get ID for name.
        * @param aReaderName Reader name
        * @return TReaderID object
        */
        TReaderID ReaderID( const TScardReaderName& aReaderName );

        /**
        * Get name for ID.
        * @param aReaderID Reader ID
        * @return TScardDBName object
        */
        const TScardReaderName FriendlyName( const TReaderID aReaderID );

        /**
        * Is reader supported or not?
        * @param aReaderName Reader name
        * @return ETrue if supported, else EFalse
        */
        TBool IsSupported( const TScardReaderName& aReaderName );

        /**
        * Lists open readers.
        * @param aACList (OUT) List of readers
        * @return void
        */
        void ListOpenReadersL( CArrayFix<CScardAccessControl*>* aACList );

        /**
        * List the readers, which have not opened yet.
        * @param aACList (OUT) List of readers
        * @return void
        */
        void ListClosedReadersL( CArrayFix<TReaderID>* aIDList );

    private:

        /**
        * C++ default constructor.
        * @param aServer Pointer to CScardServer object
        */
        CScardReaderRegistry( CScardServer* aServer );
        
        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();
    
        /**
        * Loads the reader handler dll corresponding to given parameters
        * @param aGroupID Group ID
        * @param aReaderID Reader ID
        * @return Pointer to MScardReaderLauncher object
        */
        MScardReaderLauncher* LoadFactoryModuleL( const TGroupID aGroupID,
                                                  const TReaderID aReaderID );

    private:    // Data
        // Handle for server. Not owned
        CScardServer*  iServer; 
        // Storage to keep track of loaded factories. Owned.
        CArrayFixFlat<TLauncherStruct>* iLaunchersInUse;
    };

#endif      // CSCARDREADERREGISTRY_H

// End of File
