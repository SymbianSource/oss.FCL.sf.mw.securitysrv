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
* Description:  This object is the one responsible for trafficing between
*               the Smart Card reader and the sessions.
*
*/



#ifndef CSCARDACCESSCONTROL_H
#define CSCARDACCESSCONTROL_H

//  INCLUDES
#include "ScardDefs.h"
#include "ScardChannelManager.h" // required by inline functions

// FORWARD DECLARATIONS
class CScardSession;
class MScardReader;
class CScardMessageRegistry;
class CScardAccessControl;
class CScardAccessControlRegistry;
class CScardCommandTimer;
class CScardChannelManager;

// CLASS DECLARATION

/**
*  Handles traffic between the Smart Card reader and the sessions.
*  
*  @lib Scard.lib
*  @since Series60 2.1
*/
class CScardAccessControl : public CActive
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        * @param aReaderID Reader ID
        * @param aControl Pointer to AccessControlRegistry
        */
        static CScardAccessControl* NewL(
            const TReaderID aReaderID, 
            CScardAccessControlRegistry* aControl );

        /**
        * Destructor.
        */
        virtual ~CScardAccessControl();
    
    public: // New functions
        
        /**
        * Attach a new session to the reader.
        * @param aSession session
        * @param aSessionID session ID
        * @return Pointer to reader.
        */
        MScardReader* AttachSessionToReaderL( CScardSession* aSession, 
                                              TInt &aSessionID );

        /**
        * Is this session attached to this reader.
        * @param aSession session
        * @return true if session is attached.
        */
        TBool IsAttached( CScardSession* aSession ) const;

        /**
        * Detach a connection between reader and session.
        * @param aSessionID session ID
        * @return void
        */
        void DetachSessionFromReader( const TInt aSessionID );
    
        /**
        * Initialise reader by calling Open.
        * @param aSessionID session ID
        * @param aMessage message
        * @return EFalse if succesful, ETrue if reader is already open.
        */
        TBool InitialiseReader( const TInt aSessionID, 
                                const RMessage2& aMessage );
    
        /**
        * Is the reader available for data transmit
        * @param aSessionID session ID
        * @param aChannel channel
        * @return ETrue if reader is ready for data transmission.
        */
        TBool ReaderIsReady( const TInt aSessionID, 
                             const TInt8 aChannel ) const;
    
        /**
        * Set access control in a state that enables communication with the 
        * reader. Also sets control object active, and gives iStatus back to 
        * caller.
        * @param aSessionID session ID
        * @param aMessage message
        * @param aTimeOut time-out in microseconds (0 = no time-out).
        * @param aChannel Logical channel
        * @param aAdditionalParameter proprietary parameter sent to reader.
        * @return ETrue if reader is ready for data transmission.
        */
        TRequestStatus& InitiateCommunication(
            const TInt aSessionID, 
            const RMessage2& aMessage, 
            const TInt32 aTimeOut, 
            const TInt8 aChannel,
            const TUint8 aAdditionalParameter = 0x00 );
    
        /**
        * Free reader's reservation.
        * @param aSessionId session ID
        * @param aChannel channel
        */
        void FreeChannelL( const TInt aSessionID, const TInt8 aChannel );

        /**
        * Stack the given command for completion later.
        * @param aMessage message
        * @param aSessionID session ID
        * @param aTimeOut time-out in milliseconds (0 = no time-out).
        * @param aChannel channel
        */
        void QueueExecution( const RMessage2& aMessage,
                             const TInt aSessionID, 
                             const TInt32 aTimeOut,
                             const TInt8 aChannel, 
                             const TInt8 aParameter = 0 );

        /**
        * Stack the given command for completion later with slightly higher 
        * priority.
        * @param aMessage message
        * @param aSessionID session ID
        * @param aTimeOut time-out in milliseconds (0 = no time-out).
        * @param aChannel channel
        */
        void QueueChannelOperation( const RMessage2& aMessage, 
                                    const TInt aSessionID,
                                    const TInt32 aTimeOut, 
                                    const TInt8 aChannel );

        /**
        * Remove operations for the session from the stack.
        * @param aSessionID session ID
        */
        void DequeueOperations( const TInt aSessionID );

        /**
        * Handle a card event.
        * @param aEvent card event
        * @param aATR ATR-bytes
        */
        void CardEvent( const TScardServiceStatus aEvent, 
                        const TScardATR& aATR );

        /**
        * Cancel everything from the session.
        * @param aSessionID session ID
        */
        void CancelTransmissionsL( const TInt aSessionID );

        /**
        * Cancel an operation because the commands timer has run out.
        * @param aTimer command timer
        */
        inline void CancelByTimeOut( CScardCommandTimer* aTimer );

        /**
        * ID of the reader this object controls.
        * @return reader ID
        */
        inline const TReaderID ReaderID() const;

        /**
        * The reader object this object control.
        * @return pointer to reader object.
        */
        inline MScardReader* Reader() const;

        /**
        * Give a session access rigths to the channel.
        * @param aChannel channel
        * @param aSessionID session ID
        * @return ETrue on success.
        */
        inline const TBool AddSessionToChannelL( const TInt8 aChannel, 
                                                 const TInt aSessionID );

        /**
        * Remove the session from the channel.
        * @param aChannel channel
        * @param aSessionID session ID
        * @return ETrue on success.
        */
        inline const TBool RemoveSessionFromChannelL( const TInt8 aChannel, 
                                                      const TInt aSessionID );

        /**
        * Has a channel been opened yet.
        * @param aChannel channel
        * @return ETrue if channel has been opened.
        */
        inline const TBool ChannelOpenedYet( const TInt8 aChannel );

        /**
        * Getter for channel status.
        * @return channel status
        */
        inline TUint16 ChannelStatus();

        /**
        * Is the channel number legal, and is the session on the channel.
        * @param aChannel channel number
        * @param aSessionID session id
        */
        inline void ValidateChannelL( const TInt8 aChannel, 
                                      const TInt aSessionID );

        /**
        * Get next unreserved logical channel.
        * @return number of next unreserved logical channel
        */
        inline const TInt8 UnreservedLogicalChannel() const;


    private: // Functions from base classes

        /**
        * From CActive cancel the request.
        */
        void DoCancel();

        /**
        * From CActive complete the request.
        */
        void RunL();

    private:

        /**
        * C++ default constructor.
        * @param aReaderID Reader ID
        * @param aControl Pointer to AccessControlRegistry
        */
        CScardAccessControl( const TReaderID aReaderID, 
                             CScardAccessControlRegistry* aControl );
    
        /**
        * By default Symbian OS constructor is private.
        */
        void ConstructL();
    
        /**
        * Get a pointer to the session with ID aSessionID.
        * @param aSessionID Session ID
        * @return Pointer to session
        */
        CScardSession* SessionBase( const TInt aSessionID );

        /**
        * Handles next message.
        * @param aHandle Handle of message
        */
        inline void HandleNextMessageL( const TMessageHandle& aHandle );

    private:    // Data

        // Reader session struct
        struct TReaderSession
            {
            CScardSession* SessionBase;
            TInt SessionID;
            };

        // Life modes
        enum TLifeMode
            {
            ECanBeDeleted,
            ECanNotDelete,
            EDestroyASAP
            };

        //  Container for the client sessions
        CArrayFixFlat<TReaderSession>* iSessionRegistry;
    
        // state variables for the reader
        TBool           iReaderActive;
        TBool           iIsCreated;
        TBool           iIsOpen;
        TInt            iNextSessionID;

        //  the actual reader object parameters
        MScardReader* iReader;
        TReaderID     iReaderID;

        //  Owner of this object
        CScardAccessControlRegistry* iControlRegistry;

        //  Internal channel manager
        CScardChannelManager* iManager;
    
        //  ATR bytes of the SC in the reader this object controls
        TScardATR iATR;

        // Life mode
        TLifeMode iLifeMode;
        
    };

#include "ScardAccessControl.inl"

#endif  // CSCARDACCESSCONTROL_H

// End of File
