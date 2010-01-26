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
* Description:  Handles the creation and destruction of individual access 
*                controllers
*
*/



#ifndef CSCARDACCESSCONTROLREGISTRY_H
#define CSCARDACCESSCONTROLREGISTRY_H

//  INCLUDES
#include "ScardBase.h"
#include "ScardDefs.h"

//  FORWARD DECLARATIONS
class CScardAccessControl;
class CScardServer;
class MScardReader;

// CLASS DECLARATION

/**
*  Handles the creation and destruction of individual access controllers.
*
*  @lib Scard.lib
*  @since Series60 2.1
*/
class CScardAccessControlRegistry : public CBase
    {
    public:  // Constructors and destructor
            
        /**
        * Two-phased constructor.
        * @param aServer Pointer to server object
        */
        static CScardAccessControlRegistry* NewL( CScardServer* aServer );
    
        /**
        * Destructor
        */
        virtual ~CScardAccessControlRegistry();

    public: // New functions

        /**
        * Give pointer to the actual access control object for the 
        * spesified reader
        * @param aReaderID ReaderID
        * @return Pointer to found ScardAccessControl object
        */
        CScardAccessControl* FindAccessController( const TReaderID aReaderID );
        
        /**
        * Return access controller object
        * @param aReaderID ReaderID
        * @return Pointer to CScardAccessControl object
        */
        CScardAccessControl* AccessController( const TReaderID aReaderID );

        /**
        * Used by the server to get a Reader object
        * @param aReaderID ReaderID
        * @return Pointer to MScardReader object
        */
        MScardReader* Reader( const TReaderID aReaderID ) const;
    
        /**
        * Check if this reader has a controller loaded
        * @param aReaderID ReaderID
        * @return ETrue if reader loaded, EFalse if not
        */
        TBool ReaderHandlerLoaded( const TReaderID aReaderID ) const;

        /**
        * Return pointer to CScardServer
        * @return Pointer to CScardServer object
        */
        CScardServer* Server();

        /**
        * An access controller has been disconnected
        * @param aControl Pointer to CScardAccessControl object
        * @return void
        */
        void ControllerRetired( CScardAccessControl* aControl );

    private:
        /**
        * C++ default constructor.
        */
        CScardAccessControlRegistry();

        /**
        * By default Symbian 2nd phase constructor is private.
        * @param aServer Pointer to server object
        */
        void ConstructL( CScardServer* aServer );

        //  
        /**
        * Destroy a handler object from the registry
        * @param aIndex Index of access controller in registry
        * @return void
        */
        //  
        void RemoveAccessController( const TInt aIndex );

    private:    //Data
        // Array of AccessControllers. Owned.
        CArrayFixFlat<CScardAccessControl*>* iAccessControllers;
        // Pointer to Server object. Not owned.
        CScardServer*                        iServer;
    };

#endif      // CSCARDACCESSCONTROLREGISTRY_H

// End of File
