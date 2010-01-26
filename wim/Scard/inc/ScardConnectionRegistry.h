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
* Description:  Keeps track, creates and destroys connector objects
*
*/



#ifndef CSCARDCONNECTIONREGISTRY_H
#define CSCARDCONNECTIONREGISTRY_H

// INCLUDE FILES
#include "ScardDefs.h"


//FORWARD DECLARATIONS
class CScardAccessControlRegistry;
class CScardAccessControl;
class CScardServer;
class CScardConnector;
class CScardSession;
class MScardReader;


// DATA TYPES
struct TConnectionHandle
    {
    CScardConnector* iConnector;  // This is introduced in forward declarations
    CScardSession*   iClient;
    TConnectionHandle()
        :iConnector( NULL ), iClient( NULL )
        {}
    };

// CLASS DECLARATION

/**
*  Keeps track, creates and destroys connector objects.
*
*  @lib Scard.lib
*  @since Series60 2.1
*/
class CScardConnectionRegistry : public CBase
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        * @param aServer Pointer to server object
        */
        static CScardConnectionRegistry* NewL( CScardServer* aServer );
        
        /**
        * Destructor.
        */
        virtual ~CScardConnectionRegistry();
    
    public: // New functions
        
        /**
        * Creates the connector and instructs it to connect.
        * @param aSession Pointer to session
        * @param aMessage Message
        * @return void
        */
        void ConnectToReaderL( CScardSession* aSession, 
                               const RMessage2& aMessage );

        /**
        * Finds the connector managing the session and instructs it cancel. 
        * Panics if the connector is not found.
        * @param aSession
        * @return void
        */
        void CancelConnection( CScardSession* aSession );

        /**
        * Connection completed.
        * @param aConnector Pointer to connector
        * @param aReaderID Reader ID
        * @param aErrorCode Error code that is returned in error
        * @return void
        */
        void ConnectDone( CScardConnector* aConnector, 
                          const TReaderID& aReaderID, 
                          const TInt& aErrorCode );

        /**
        * Removes connector from registry.
        * @param aConnector Pointer to connector to be removed
        * @return void
        */
        void RemoveConnector( CScardConnector* aConnector );

        /**
        * Return pointer to ScardServer object
        * @return Pointer to ScardServer object
        */
        CScardServer* Server() const;

        /**
        * Return connection handle
        * @param aLocation Index in iConnectors array
        * @return Connection handle
        */
        TConnectionHandle& Connection( TInt aLocation ) const;

    private:
        
        /**
        * C++ default constructor.
        */
        CScardConnectionRegistry();

        /**
        * By default Symbian 2nd phase constructor is private.
        * @param aServer Pointer to server object
        */
        void ConstructL( CScardServer* aServer );

    private:    // Data
        // Pointer to Scard Server. Not owned.
        CScardServer* iServer;
        // Array of connectors. Owned.
        CArrayFixFlat<TConnectionHandle>* iConnectors;
    };

#endif      // CSCARDCONNECTIONREGISTRY_H

// End of File
