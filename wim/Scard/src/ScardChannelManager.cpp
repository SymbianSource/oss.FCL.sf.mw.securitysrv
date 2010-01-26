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
* Description:  Handles individual channels on a singular SC (which equals one
*                Smart Card reader handler). Can be queried for information 
*                about channels, controls traffic etc.
*
*/



// INCLUDE FILES
#include    "ScardAccessControl.h"
#include    "ScardBase.h"
#include    "ScardChannelManager.h"
#include    "ScardMessageStack.h"
#include    "WimTrace.h"


// MACROS
//  Assert that a session is on a channel  
#define __ASSERT_SESSION( a, b ) if ( !SessionIsOnChannel( (a), (b) ) )\
    User::Leave( KScServerErrIllegalChannel );
//  Assert that the channel number is legal, or leave
#define __ASSERT_CHANNEL_LEAVE( a ) if ( (a) < 0 || (a) > KMaxChannels - 1 )\
    User::Leave( KScServerErrIllegalChannel )

//  Assert that the channel number is legal, or return false
#define __ASSERT_CHANNEL_FALSE( a ) if ( (a) < 0 || (a) > KMaxChannels - 1 )\
    return EFalse

// LOCAL FUNCTION PROTOTYPES
TUint8 pow2( const TInt aPower );


// ============================= LOCAL FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// pow2
// Returns: 2 powered to given value
// -----------------------------------------------------------------------------
//

TUint8 pow2( const TInt aPower )
    {
    _WIMTRACE(_L("WIM|Scard|CScardChannelManager::pow2|Begin"));
    TUint8 result = 1;
    for ( TInt i = 0; i < aPower; i++ )
        {
        result *= 2;
        }
    return result;
    }

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CScardChannelManager::CScardChannelManager
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CScardChannelManager::CScardChannelManager()
    {
    _WIMTRACE(_L("WIM|Scard|CScardChannelManager::CScardChannelManager|Begin"));
    }

// -----------------------------------------------------------------------------
// CScardChannelManager::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CScardChannelManager::ConstructL()
    {
    _WIMTRACE(_L("WIM|Scard|CScardChannelManager::ConstructL|Begin"));
    iChannels = new( ELeave ) CArrayPtrFlat<CArrayFixFlat<TInt> >( 1 );

    //  Channel 0 is always open
    CArrayFixFlat<TInt>* zeroChannel = new( ELeave ) CArrayFixFlat<TInt>( 1 );
    CleanupStack::PushL( zeroChannel );
    iChannels->AppendL( zeroChannel );

    // others are opened as the need arises
    iChannels->AppendL( static_cast< CArrayFixFlat<TInt>* >( NULL ) );
    iChannels->AppendL( static_cast< CArrayFixFlat<TInt>* >( NULL ) );
    iChannels->AppendL( static_cast< CArrayFixFlat<TInt>* >( NULL ) );

    iMessageStack = CScardMessageStack::NewL();
    CleanupStack::Pop( zeroChannel );
    }

// -----------------------------------------------------------------------------
// CScardChannelManager::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CScardChannelManager* CScardChannelManager::NewL()
    {
    _WIMTRACE(_L("WIM|Scard|CScardChannelManager::NewL|Begin"));
    CScardChannelManager* self = new( ELeave ) CScardChannelManager;
    
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    return self;
    }

    
// Destructor
CScardChannelManager::~CScardChannelManager()
    {
    _WIMTRACE(_L("WIM|Scard|CScardChannelManager::~CScardChannelManager|Begin"));
    if ( iChannels )
        {
        iChannels->ResetAndDestroy();
        }
    delete iChannels;
    delete iMessageStack;
    }


// -----------------------------------------------------------------------------
// CScardChannelManager::ChannelStatus
// Give available channels
// Returns two-byte parameter telling:
// b0 - Channel 0 open (should always be set)
// b1 - Channel 0 reserved
// b2 - Channel 1 open
// b3 - Channel 1 reserved
// ...
// b7 - Channel 3 open
// b8 - Channel 3 reserved
//                
// b9-b15 Maximum number of channels available (at the moment always 4)
//
// -----------------------------------------------------------------------------
//
TUint16 CScardChannelManager::ChannelStatus()
    {
    _WIMTRACE(_L("WIM|Scard|CScardChannelManager::ChannelStatus|Begin"));
    TUint16 free( 0 );
    free = static_cast< TUint16 >( 0x04 << 8 ); // 4 channels available

    //  Get the status for each individual channel, and set the according bits
    for ( TInt i = 0; i < KMaxChannels; i++ )
        {
        if ( iChannels->At( i ) )    
            {
            free |= pow2( 2 * i );
            }
        if ( iReserverID[i] )
            {
            free |= pow2( 2 * i + 1 );
            }
        }
    return free;
    }

// -----------------------------------------------------------------------------
// CScardChannelManager::SessionIsOnChannel
// Does the session have permission to the channel.
// -----------------------------------------------------------------------------
//
TBool CScardChannelManager::SessionIsOnChannel(
    const TInt8 aChannel, 
    const TInt aSessionID ) const
    {
    _WIMTRACE(_L("WIM|Scard|CScardChannelManager::SessionIsOnChannel|Begin"));
    __ASSERT_CHANNEL_FALSE( aChannel );
    if ( iChannels->At( aChannel ) )
        {
        TInt count = iChannels->At( aChannel )->Count();
        for ( TInt j( 0 ); j < count; j++ )
            {
            if ( iChannels->At( aChannel )->At( j ) == aSessionID )
                {
                _WIMTRACE(_L("WIM|Scard|CScardChannelManager::SessionIsOnChannel|Permission granted"));
                return ETrue;
                }
            }
        }
    _WIMTRACE(_L("WIM|Scard|CScardChannelManager::SessionIsOnChannel|Permission denied"));
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CScardChannelManager::AddSessionToChannelL
// Give the session the right to use the channel
// -----------------------------------------------------------------------------
//
TBool CScardChannelManager::AddSessionToChannelL(
    const TInt8 aChannel, 
    const TInt aSessionID )
    {
    _WIMTRACE(_L("WIM|Scard|CScardChannelManager::AddSessionToChannelL|Begin"));
    __ASSERT_CHANNEL_FALSE( aChannel );
    if ( !SessionIsOnChannel( aChannel, aSessionID ) )
        {
        // if the channel is not yet opened...
        if ( !iChannels->At( aChannel ) )
            {
            CArrayFixFlat<TInt>* channel = NULL;
            channel = new( ELeave ) CArrayFixFlat<TInt>( 1 );
            iChannels->At( aChannel ) = channel;
            }
        iChannels->At( aChannel )->AppendL( aSessionID );
        }
    return ETrue;
    }

// -----------------------------------------------------------------------------
// CScardChannelManager::RemoveSessionL
// Remove session entries from the channel.
// -----------------------------------------------------------------------------
//
TBool CScardChannelManager::RemoveSessionL(
    const TInt8 aChannel, 
    const TInt aSessionID )
    {
    _WIMTRACE(_L("WIM|Scard|CScardChannelManager::RemoveSessionL|Begin"));
    __ASSERT_CHANNEL_LEAVE( aChannel );
    if ( SessionIsOnChannel( aChannel, aSessionID ) )
        {
        for ( TInt i( 0 ); i < iChannels->At( aChannel )->Count(); i++ )
            {
            // remove the indivídual from the channel
            if ( iChannels->At( aChannel )->At( i ) == aSessionID )
                {
                iChannels->At( aChannel )->Delete( i );
                i = iChannels->At( aChannel )->Count(); // Stop loop
                }
            }
        //  if the channel is empty (except channel 0), delete it
        if ( aChannel != 0 && !iChannels->At( aChannel )->Count() )
            {
            delete iChannels->At( aChannel );
            iChannels->At( aChannel ) = NULL;
            return EFalse;
            }
        }
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CScardChannelManager::PushMessageToBottomL
// Push message to bottom of message stack
// -----------------------------------------------------------------------------
//
void CScardChannelManager::PushMessageToBottomL(
    const TMessageHandle& aMessage )
    {
    _WIMTRACE(_L("WIM|Scard|CScardChannelManager::PushMessageToBottomL|Begin"));
    __ASSERT_CHANNEL_LEAVE( aMessage.iChannel );
    __ASSERT_SESSION( aMessage.iChannel, aMessage.iSessionID );
    __ASSERT_MEMORY( iMessageStack->PushToBottom( aMessage ) );
    }

// -----------------------------------------------------------------------------
// CScardChannelManager::PushMessageToTopL
// Push message to top of message stack
// -----------------------------------------------------------------------------
//
void CScardChannelManager::PushMessageToTopL(
    const TMessageHandle& aMessage )
    {
    _WIMTRACE(_L("WIM|Scard|CScardChannelManager::PushMessageToTopL|Begin"));
    __ASSERT_CHANNEL_LEAVE( aMessage.iChannel );
    __ASSERT_SESSION( aMessage.iChannel, aMessage.iSessionID );
    __ASSERT_MEMORY( iMessageStack->PushToTop( aMessage ) );
    }

// -----------------------------------------------------------------------------
// CScardChannelManager::NextMessageL
// Get the next available message from the stack (preserving
// the order of other messages)
// -----------------------------------------------------------------------------
//
const TMessageHandle CScardChannelManager::NextMessageL()
    {
    _WIMTRACE(_L("WIM|Scard|CScardChannelManager::NextMessage|Begin"));
    TInt count = iMessageStack->Count();
    TInt index( 0 );
    while ( index < count )
        {
        TMessageHandle tmp = iMessageStack->FromPositionL( index );

        // If the channel is reserved (for someone else), 
        // put message back to wait
        if ( iReserverID[tmp.iChannel] && iReserverID[tmp.iChannel] != 
            tmp.iSessionID )
            {
            iMessageStack->PushToPositionL( index, tmp );
            index++;
            }
        else
            {
            return tmp;
            }
        }
    TMessageHandle noMessage;
    return noMessage;
    }

// -----------------------------------------------------------------------------
// CScardChannelManager::MessageFromTop
// Get message from top of message stack
// -----------------------------------------------------------------------------
//
const TMessageHandle CScardChannelManager::MessageFromTop()
    {
    _WIMTRACE(_L("WIM|Scard|CScardChannelManager::MessageFromTop|Begin"));
    return iMessageStack->FromTop();
    }

// -----------------------------------------------------------------------------
// CScardChannelManager::CancelAll
// Cancel all messages from the stack that match the criteria
// -----------------------------------------------------------------------------
//
void CScardChannelManager::CancelAll( const TInt aSessionID )
    {
    _WIMTRACE(_L("WIM|Scard|CScardChannelManager::CancelAll|Begin"));
    if ( aSessionID == EAccessMasterID )
        {
        iMessageStack->CancelAll( KScErrCancelled );
        }
    else
        {
        iMessageStack->CancelAll( aSessionID, KScErrCancelled );
        }
    }

// -----------------------------------------------------------------------------
// CScardChannelManager::ChannelReservedL
// Return reserver session of the channel
// -----------------------------------------------------------------------------
//
TInt CScardChannelManager::ChannelReservedL(
    const TInt8 aChannel ) const
    {
    _WIMTRACE(_L("WIM|Scard|CScardChannelManager::ChannelReservedL|Begin"));
    __ASSERT_CHANNEL_LEAVE( aChannel );
    return iReserverID[aChannel];
    }

// -----------------------------------------------------------------------------
// CScardChannelManager::FreeChannelL
// Free channel
// -----------------------------------------------------------------------------
//
void CScardChannelManager::FreeChannelL(
    const TInt8 aChannel, 
    const TInt aSessionID )
    {
    _WIMTRACE(_L("WIM|Scard|CScardChannelManager::FreeChannelL|Begin"));
    __ASSERT_CHANNEL_LEAVE( aChannel );
    if ( iReserverID[aChannel] == aSessionID )
        {
        iReserverID[aChannel] = ENoSession;
        }
    }

// -----------------------------------------------------------------------------
// CScardChannelManager::FreeChannels
// Free all channels this session has reserved
// -----------------------------------------------------------------------------
//
void CScardChannelManager::FreeChannels( const TInt aSessionID )
    {
    _WIMTRACE(_L("WIM|Scard|CScardChannelManager::FreeChannels|Begin"));
    for ( TInt i( 0 ); i < KMaxChannels; i++ )
        {
        if ( iReserverID[i] == aSessionID )
            {
            iReserverID[i] = 0;
            }
        }
    }

// -----------------------------------------------------------------------------
// CScardChannelManager::ChannelOpenedYet
// Return ETrue if channel is allready opened, otherwise return EFalse
// -----------------------------------------------------------------------------
//
TBool CScardChannelManager::ChannelOpenedYet( const TInt8 aChannel )
    {
    _WIMTRACE(_L("WIM|Scard|CScardChannelManager::ChannelOpenedYet|Begin"));
    __ASSERT_CHANNEL_LEAVE( aChannel );
    if ( iChannels->At( aChannel ) )
        {
        _WIMTRACE(_L("WIM|Scard|CScardChannelManager::ChannelOpenedYet|True"));
        return ETrue;
        }
    _WIMTRACE(_L("WIM|Scard|CScardChannelManager::ChannelOpenedYet|False"));
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CScardChannelManager::UnreservedLogicalChannel
// First channel w/o a reservation
// -----------------------------------------------------------------------------
//
TInt8 CScardChannelManager::UnreservedLogicalChannel() const
    {
    _WIMTRACE(_L("WIM|Scard|CScardChannelManager::UnreservedLogicalChannel|Begin"));
    for ( TInt8 i( 0 ); i < KMaxChannels; i++ )
        {
        if ( !iReserverID[i] )
            {
            return i;
            }
        }
    return -1;
    }

// -----------------------------------------------------------------------------
// CScardChannelManager::NextMessageFromFree
// Return next message from free 
// -----------------------------------------------------------------------------
//
const TMessageHandle CScardChannelManager::NextMessageFromFree(
    const TInt8 aChannel)
    {
    _WIMTRACE(_L("WIM|Scard|CScardChannelManager::NextMessageFromFree|Begin"));
    TMessageHandle tmp = iMessageStack->NextReservation( aChannel );
    if ( tmp.iSessionID != ENoSession )
        {
        return tmp;
        }
    
    TMessageHandle nextMessage;

    TRAPD( err, nextMessage = NextMessageL() );
    
    if ( err != KErrNone )
        {
        TMessageHandle noMessage;
        return noMessage;
        }
    else
        {
        return nextMessage;
        }
    }

// -----------------------------------------------------------------------------
// CScardChannelManager::ValidateChannelL
// Channel is legal & open for this session
// -----------------------------------------------------------------------------
//
void CScardChannelManager::ValidateChannelL(
    const TInt8 aChannel, 
    const TInt aSessionID )
    {
    _WIMTRACE(_L("WIM|Scard|CScardChannelManager::ValidateChannelL|Begin"));
    __ASSERT_CHANNEL_LEAVE( aChannel );
    if ( iChannels->At( aChannel ) )
        {
        for ( TInt j( 0 ); j < iChannels->At( aChannel )->Count(); j++ )
            {
            if ( iChannels->At( aChannel )->At( j ) == aSessionID )
                {
                return;
                }
            }
        }
    _WIMTRACE(_L("WIM|Scard|CScardSession::ValidateChannelL|Leave"));
    User::Leave( KScErrNotFound );
    }

// -----------------------------------------------------------------------------
// CScardChannelManager::CardRemoved
// Card removed. Clear all card-spesific data & transmissions
// -----------------------------------------------------------------------------
//
void CScardChannelManager::CardRemoved()
    {
    _WIMTRACE(_L("WIM|Scard|CScardChannelManager::CardRemoved|Begin"));
    // Cancel all card operations
    iMessageStack->CancelCardOperations();

    // remove all session entries from channel registers
    for ( TInt i( 1 ); i < iChannels->Count(); i++ )
        {
        delete iChannels->At( i );
        iChannels->At( i ) = static_cast< CArrayFixFlat<TInt>* >( NULL );
        iReserverID[i] = ENoSession;
        }
    }

// -----------------------------------------------------------------------------
// CScardChannelManager::CancelByTimeOut
// Cancel messages by timeout
// -----------------------------------------------------------------------------
//
void CScardChannelManager::CancelByTimeOut( CScardCommandTimer* aTimer )
    {
    _WIMTRACE(_L("WIM|Scard|CScardChannelManager::CancelByTimeOut|Begin"));
    iMessageStack->CancelByTimeOut( aTimer );
    }

//  End of File  
