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
* Description:  Notification Registry for SmartCard.
*
*/



#ifndef CSCARDNOTIFYREGISTRY_H
#define CSCARDNOTIFYREGISTRY_H


//  INCLUDES
#include "ScardServerBase.h"
#include "ScardNotifyObserver.h"


// CONSTANTS
const TReaderID KServerNotifier = -1;

//  FORWARD DECLARATIONS
class CScardServer;
class CScardNotifier;

// CLASS DECLARATION

/**
*  Keeps track of notifiers.
*
*  @lib Scard.lib
*  @since Series60 2.1
*/
class CScardNotifyRegistry : public CBase, public MScardNotifyObserver
    {

    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        IMPORT_C static  CScardNotifyRegistry* NewL( CScardServer* aServer );

        /**
        * Destructor.
        */
        virtual ~CScardNotifyRegistry();
   
    public: // New functions
        
        /**
        * Creates a notifier if necessary.
        * @param aMessage Message
        * @return void
        */
        void AddReaderEventListenerL( const RMessage2& aMessage );

        /**
        * Removes the notifier.
        * @param aMessage Message
        * @return void
        */
        void RemoveReaderEventListener( const RMessage2& aMessage );

        /**
        * Removes all notifiers for given client.
        * @param aClient Client thread
        * @return void
        */
        void RemoveReaderEventListeners( const RThread& aClient );
    
        /**
        * Notify clients about card event.
        * @param aEvent Event
        * @param aReaderID Reader ID
        * @return void
        */
        IMPORT_C void NotifyCardEvent( TScardServiceStatus aEvent,
                                       TReaderID aReaderID );

        /**
        * Return pointer to Server
        * @return Pointer to the Server object
        */
        CScardServer* Server();

        /**
        * Cancels the request.
        * @param aStatus Request status
        * @return void
        */
        void Cancel( TRequestStatus& aStatus );

    private:
        
        /**
        * C++ default constructor.
        */
        CScardNotifyRegistry(); 

        /**
        * Symbian 2nd phase constructor.
        */
        void ConstructL( CScardServer* aServer);  

    private:    // Data
        // Array of notifiers. Owned.
        CArrayPtrFlat<CScardNotifier>* iNotifiers;    
        // Server object. Not owned.
        CScardServer*                  iServer;
    };

#endif      // CSCARDREGISTRY_H

// End of File
