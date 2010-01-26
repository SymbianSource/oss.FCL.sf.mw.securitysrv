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
* Description:  SmartCard server.
*
*/



#ifndef CSCARDSERVER_H
#define CSCARDSERVER_H

//  INCLUDES
#include "ScardReaderLauncher.h"


//  FORWARD DECLARATIONS
class CScardAccessControl;
class CScardAccessControlRegistry;
class CScardSession;
class CScardNotifyRegistry;
class CScardReaderRegistry;
class CScardResourceRegistry;
class CScardConnectionRegistry;


// CLASS DECLARATION

/**
*  Server class.
*  Contains an object container index which provides subsessions with 
*  object containers.
*
*  @lib Scard.lib
*  @since Series60 2.1
*/
class CScardServer : public CServer2, public MScardReaderService
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CScardServer* NewL();

        /**
        * Destructor.
        */
        virtual ~CScardServer();

    public: // New functions

        /**
        * Panic server
        * @param aPanic Panic code 
        * @return void
        */
        static void PanicServer( const TInt aPanic );
        
        /**
        * List the readers, which have not opened yet.
        * @param aStarted Thread parameter
        * @return TInt
        */
        static TInt ThreadFunction( TAny* aStarted );
    
        /**
        * Opens session.
        * @param aVersion Version of the server
        * @return Pointer to created session
        */
        virtual CSession2* CScardServer::NewSessionL( 
            const TVersion& aVersion,
            const RMessage2& aMessage ) const;
    
        /**
        * Return pointers to registries.
        * @return Pointer to CScardNotifyRegistry object
        */
        inline CScardNotifyRegistry* NotifyRegistry() const;

        /**
        * Return pointer to reader registry.
        * @return Pointer to CScardReaderRegistry object
        */
        inline CScardReaderRegistry* FactoryRegistry() const;

        /**
        * Return pointer to an access controller.
        * @param aReaderID Reader ID
        * @return Pointer to CScardAccessControl object
        */
        inline CScardAccessControl* FindAccessControl(
            const TReaderID aReaderID ) const;

        /**
        * Scan through the readers to see if this reader is supported 
        * (so it is usable).
        * @param aReaderName Name of the reader
        * @return ETrue if reader is supported else EFalse
        */
        inline const TBool ReaderSupported(
            const TScardReaderName& aReaderName ) const;
    
        /**
        * Scan the access controllers to see if reader is loaded at the moment.
        * @param aReaderName Name of the reader
        * @return ETrue if reader is in use, otherwise EFalse
        */
        inline const TBool ReaderInUse( 
            const TScardReaderName& aReaderName ) const;
        
        /**
        * Scan the database to see if group is loaded at the moment.
        * @param aGroupName Name of the group
        * @return ETrue if group is in use, otherwise EFalse
        */
        inline const TBool GroupInUse(
            const TScardReaderName& aGroupName ) const;

        /**
        * Return pointer to notify observer.
        * @return Pointer to MScardNotifyObserver object
        */
        inline MScardNotifyObserver* NotifyObserver();

        /**
        * Return pointer to access control registry object.
        * @return Pointer to CScardAccessControlRegistry object
        */
        inline CScardAccessControlRegistry* AccessRegistry() const;

        /**
        * Return pointer to reader registry object.
        * @return Pointer to CScardReaderRegistry object
        */
        inline CScardReaderRegistry* ReaderRegistry() const;

        /**
        * Return pointer to connection registry object.
        * @return Pointer to CScardConnectionRegistry object
        */
        inline CScardConnectionRegistry* ConnectionRegistry() const;

        /**
        * Return pointer to Reader service object
        * @return Pointer to MScardReaderService object
        */
        inline MScardReaderService* ReaderService();

        /**
        * Scan database to see ID belonging to this name.
        * @param aFriendlyName DB Name
        * @return Reader ID for given reader name
        */
        inline const TReaderID ReaderID(
            const TScardReaderName& aFriendlyName ) const;

        /**
        * Scan the database for the name belonging to this ID.
        * @param aReaderID Reader ID
        * @return Name of the given reader ID
        */
        inline const TScardReaderName FriendlyName(
            const TReaderID aReaderID ) const;

        /**
        * Connect to reader.
        * @param aSession Pointer to session
        * @param aMessage Message
        * @return void
        */
        inline void ConnectToReaderL( CScardSession* aSession, 
                                      const RMessage2& aMessage );
    
        /**
        * If a card is inserted / removed, we need to update the list 
        * of cards in use.
        * @param aEvent Card event
        * @param aReaderID Reader ID
        * @return void
        */
        inline void CardEvent( const TScardServiceStatus aEvent,
                               const TReaderID aReaderID ); 

    private:

        /**
        * C++ default constructor.
        */
        CScardServer();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private:    // Data
        // Pointer to connection registry. Owned.
        CScardConnectionRegistry*    iConnectionRegistry;
        // Pointer to access control registry. Owned.
        CScardAccessControlRegistry* iControlRegistry;
        // Pointer to notify registry. Owned.
        CScardNotifyRegistry*        iNotifyRegistry;
        // Pointer to reader registry. Owned.
        CScardReaderRegistry*        iReaderFactoryRegistry;
        // Pointer to resource registry. Owned.
        CScardResourceRegistry*      iResourceRegistry;
        // Name of the server. Owned.
        HBufC*                       iName;
    };

#include "ScardServer.inl"

#endif      // CSCARDSERVER_H

// End of File
