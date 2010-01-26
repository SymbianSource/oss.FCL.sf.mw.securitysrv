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
* Description:  Handles individual channels on a singular SC 
*               (which equals one SC reader handler) Can be queried
*               for information about channels, controls traffic etc.
*
*/



#ifndef CSCARDCHANNELMANAGER_H
#define CSCARDCHANNELMANAGER_H

// INCLUDES
#include "ScardServerBase.h"
#include "ScardDefs.h"

// FORWARD DECLARATIONS
class CScardAccessControl;
class CScardMessageStack;
class CScardCommandTimer;

// CLASS DECLARATION

/**
*  Scard channel manager.
*  Handles individual channels on a singular Smart card
*  @lib Scard.lib
*  @since Series60 2.1
*/
class CScardChannelManager : public CBase
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CScardChannelManager* NewL();

        /**
        * Destructor.
        */
        virtual ~CScardChannelManager();

    public: // New functions

        /**
        * Give the session the right to use the channel
        * @param aChannel Channel to be added
        * @param aSessionID Session ID
        * @return Always returns ETrue
        */
        TBool AddSessionToChannelL( const TInt8 aChannel, 
                                    const TInt aSessionID );

        /**
        * Remove session from the given channel
        * @param aChannel Channel
        * @param aSessionID Session ID to be removed
        * @return Always returns EFalse
        */
        TBool RemoveSessionL( const TInt8 aChannel, 
                              const TInt aSessionID );

        /**
        * Does the session have permission to the channel
        * @param aChannel Channel
        * @param aSessionID SessionID
        * @return ETrue if session is on channel, otherwise EFalse
        */
        TBool SessionIsOnChannel( const TInt8 aChannel, 
                                  const TInt aSessionID ) const;

        /**
        * This function returns an int16 that spesifies which channels have
        * been opened and which of them are reserved
        * @return Two-byte parameter telling:
        *   b0 - Channel 0 open (should always be set)
        *   b1 - Channel 0 reserved
        *   b2 - Channel 1 open
        *   b3 - Channel 1 reserved
        *   ...
        *   b7 - Channel 3 open
        *   b8 - Channel 3 reserved
        *                
        *   b9-b15 Maximum number of channels available (at the moment always 4)
        */
        TUint16 ChannelStatus();

        /**
        * Is the channel reserved? Return ID of session reserving the channel
        * @param aChannel Channel
        * @return Reserver session ID of the channel
        */
        TInt ChannelReservedL( const TInt8 aChannel ) const;

        /**
        * Remove the exclusive access from one channel.
        * @param aChannel Channel
        * @param aSessionID Session ID
        * @return void
        */
        void FreeChannelL( const TInt8 aChannel,
                           const TInt aSessionID );

        /**
        * Remove any exclusive accesses.
        * @param aChannel Channel
        * @return void
        */
        void FreeChannels( const TInt aSessionID );

        /**
        * Is the session on the channel, and is the channel number legal.
        * @param aChannel Channel
        * @param aSessionID SessionID
        * @return void
        */
        void ValidateChannelL( const TInt8 aChannel,
                               const TInt aSessionID );

        /**
        * Get the number of the next unreserved logical channel.
        * @param aChannel Channel
        * @return Number of next unreserved channel
        */
        TInt8 UnreservedLogicalChannel() const;

        /**
        * Has the channel been opened yet.
        * @param aChannel Channel
        * @return ETrue if channel already reserved, otherwise EFalse
        */
        TBool ChannelOpenedYet( const TInt8 aChannel );

        /**
        * Cancels the message.
        * @param aTimer Pointer to command timer
        * @return void
        */
        void CancelByTimeOut( CScardCommandTimer* aTimer );

        /**
        * Cancels all the messages for given session.
        * @param aSessionID SessionID
        * @return void
        */
        void CancelAll( const TInt aSessionID );

        /**
        * Puts message on bottom of message stack.
        * @param aMessage Message to be pushed to bottow of stack
        * @return void
        */
        void PushMessageToBottomL( const TMessageHandle& aMessage );
        
        /**
        * Puts message on top of message stack.
        * @param aMessage Message to be pushed
        * @return void
        */
        void PushMessageToTopL( const TMessageHandle& aMessage );

        /**
        * Get message from top of the message stack.
        * @return TMessageHandle
        */
        const TMessageHandle MessageFromTop();
        
        /**
        * Get next message
        * @return TMessageHandle
        */
        const TMessageHandle NextMessageL();
        
        /**
        * NextMessageFromFree
        * @param aChannel Channel
        * @return TMessageHandle
        */
        const TMessageHandle NextMessageFromFree( const TInt8 aChannel );

        /**
        * Handle channels when card is removed.
        * @return void
        */
        void CardRemoved();
    
    private:
        
        /**
        * C++ default constructor.
        */
        CScardChannelManager();
        
        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private:
        // The messages
        CScardMessageStack* iMessageStack;
    
        // The sessions that are joined/channel
        CArrayPtrFlat< CArrayFixFlat<TInt> >* iChannels;

        //  IDs of sessions that are currently reserving the channels
        TInt iReserverID[KMaxChannels];
    };

#endif      // CSCARDCHANNELMANAGER_H

// End of File
