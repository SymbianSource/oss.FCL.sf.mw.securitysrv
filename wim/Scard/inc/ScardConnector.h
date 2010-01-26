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
* Description:  A connection to a smart card.
*
*/



#ifndef CSCARDCONNECTOR_H
#define CSCARDCONNECTOR_H

// INCLUDE FILES
#include "ScardSession.h"
#include "ScardEventStack.h"

// CONSTANTS
const TUint8 KOpenReader = 0x40;

//FORWARD DECLARATIONS
class CScardAccessControlRegistry;
class CScardAccessControl;
class CScardConnectionRegistry;
class CScardConnectionTimer;
//class TScardATR;

// STRUCTURES
struct TConnectionParameter
    {
    TPtr*             iReaderName;
    TScardReaderName* iExcluded;
    
    TScardATR*        iATRBytes;
    TPtr8*            iAIDBytes;
    
    TBool             iNewCardOnly;
    TBool             iNewReaderOnly;

    TConnectionParameter();
    };

//  Handle to a complete connection attempt
struct TConnection
    {
    //  Access controller for that reader
    CScardAccessControl* iCtrl;
    //  The reader object this connection is attached and it's ID
    MScardReader*        iReader;
    TReaderID            iReaderID;
    //  This connection's session id
    TInt                 iSessionID;
    //  Constructor added for convenience    
    TConnection();
    };

//  Little shortcut to use when comparing two connections
TBool operator==( const TConnection&, const TConnection& );

//  The states the connection process can be in
enum TScardConnectionState
    {
    EActive = 1,    //  actively pursuing new connections
    //  all clear, a clean connection was established (or an error occurred)
    EConnectionComplete,
    //  one stage completed, waiting for the client to confirm
    EWaitingForConfirm,
    EReconnecting,  // Rrunning the while loop inside ReconnectToReaderL()
    EInitialising,  // Initialising the reader(s)
    //  connection was established inside the while loop, 
    //  imminent suicide as soon as possible
    EDying,
    ETimedOut       // Hit a timeout. Die ASAP.
    };

// CLASS DECLARATION

/**
*  Actual connector class. 
*
*  @lib Scard.lib
*  @since Series60 2.1
*/
class CScardConnector : public CScardSession
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        * @param aConnRegistry Pointer to connection registry
        * @param aClient Client thread
        * @param aMessage Client message
        */ 
        static CScardConnector* NewL( CScardConnectionRegistry* aConnRegistry,
                                      RThread& aClient, 
                                      const RMessage2& aMessage );
        
        /**
        * Destructor.
        */
        virtual ~CScardConnector();

    public: // New functions

        /**
        * The timer has sent a signal indicating that this connection 
        * attempt has timed out
        * @return void
        */
        void ConnectionTimedOut();

        /**
        * Cancel connection. It appears this connection is not wanted after all.
        * @return void
        */
        void Cancel();

    public: // Functions from base classes
    
        /**
        * From CScardSession Override ConnectToReader.
        * @return void
        */
        void ConnectToReaderL(); 

        /**
        * From CScardSession Override CardEvent
        * Handles card events. Panics if there is no connection to reader.
        * @param aEvent Scard event
        * @param aATR ATR bytes
        * @param aReaderID Reader ID
        * @return void
        */
        virtual void CardEvent( const TScardServiceStatus aEvent,
                                const TScardATR& aATR, 
                                const TReaderID& aReaderID );
    
        /**
        * From CScardSession All clear, a connection has been established.
        * @param aReaderID ReaderID
        * @param aErrorCode Error code
        * @return void
        */
        void ConnectionDone( const TReaderID aReaderID, 
                             const TInt& aErrorCode );
        

        /**
        * From CSharableSession. Return client message.
        * @return RMessage
        */
        RMessage2& Message();

    private:

        /**
        * C++ default constructor.
        * @param aConnRegistry Pointer to connection registry
        * @param aClient Client thread
        */
        CScardConnector( CScardConnectionRegistry* aConnRegistry, 
                         RThread& aClient );

        /**
        * By default Symbian 2nd phase constructor is private.
        * @param aMessage Client message
        */
        void ConstructL( const RMessage2& aMessage );

        /**
        * Parse the message to get limiting factors
        * @return void
        */
        void ReadLimitsL();

        /**
        * Check all conditions set for the new card. This method checks if
        * ATR bytes and AID bytes match with given values in connection 
        * requirements. Currently AID and ATR bytes are not supported.
        * @param aConnection Connection 
        * @param aATR ATR bytes
        * @return ETrue if connection conditions are met, else EFalse
        */
        TBool CheckConditions( TConnection& aConnection, 
                               const TScardATR* aATR );

        /**
        * Make and add to list a new connection.
        * @param aMessage Client message
        * @param aReaderID Reader ID
        * @return ETrue if connectin succesfully created, otherwise EFalse.
        */
        TBool NewConnectionL( const RMessage2 aMessage,
                              const TReaderID aReaderID );

        /**
        * Find connection for giben reader
        * @param aReaderID Reader ID
        * @return Reference to TConnection object
        */
        TConnection& FindReaderConnectionL( const TReaderID& aReaderID );

    private:    // Data
        // Container for all connection attempt handles. Owned.
        CArrayFixFlat<TConnection>* iConnections;
        // Pointer to connection registry. Not owned.
        CScardConnectionRegistry*   iConnectionRegistry;
        // The message from the client
        RMessage2*                   iClientMessage;
        // Storage for card events to be handled later. Owned.
        CScardEventStack*           iStack;
        //  Life status indicator
        TInt                        iState;
        // Contacting a single reader or many?
        TBool                       iOneReaderMode;
        // Timer for this connector. Owned.
        CScardConnectionTimer*      iTimer;
        // The parameter block for this connection attempt
        TConnectionParameter        iParameters;
    };

#endif      // CSCARDCONNECTOR_H

// End of File
