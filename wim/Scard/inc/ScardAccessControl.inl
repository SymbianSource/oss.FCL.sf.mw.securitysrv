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
* Description:  Inline functions for smart card access control
*
*/



//  INCLUDES
#include "ScardMessageStack.h"
#include "ScardSession.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CScardAccessControl::ReaderID
// Return reader ID
// -----------------------------------------------------------------------------
//
inline const TReaderID CScardAccessControl::ReaderID() const 
    {
    return iReaderID;
    }

// -----------------------------------------------------------------------------
// CScardAccessControl::Reader
// Return reader object
// -----------------------------------------------------------------------------
//
inline MScardReader* CScardAccessControl::Reader() const
    {
    return iReader;
    }

// -----------------------------------------------------------------------------
// CScardAccessControl::CancelByTimeOut
// Cancel operations by timeout
// -----------------------------------------------------------------------------
//
void CScardAccessControl::CancelByTimeOut( CScardCommandTimer* aTimer )
    {
    iManager->CancelByTimeOut( aTimer );
    }

// -----------------------------------------------------------------------------
// CScardAccessControl::ChannelStatus
// Return channel status
// -----------------------------------------------------------------------------
//
inline TUint16 CScardAccessControl::ChannelStatus()
    {
    return iManager->ChannelStatus();
    }

// -----------------------------------------------------------------------------
// CScardAccessControl::ChannelOpenedYet
// Is channel opened yet. Return ETrue/EFalse
// -----------------------------------------------------------------------------
//
inline const TBool CScardAccessControl::ChannelOpenedYet( const TInt8 aChannel )
    {
    return iManager->ChannelOpenedYet( aChannel );
    }

// -----------------------------------------------------------------------------
// CScardAccessControl::AddSessionToChannelL
// Add session to the channel
// -----------------------------------------------------------------------------
//
inline const TBool CScardAccessControl::AddSessionToChannelL(
    const TInt8 aChannel,
    const TInt aSessionID )
    {
    return iManager->AddSessionToChannelL( aChannel, aSessionID );
    }

// -----------------------------------------------------------------------------
// CScardAccessControl::UnreservedLogicalChannel
// Return unreserved logical channel
// -----------------------------------------------------------------------------
//
inline const TInt8 CScardAccessControl::UnreservedLogicalChannel() const
    {
    return iManager->UnreservedLogicalChannel();
    }

// -----------------------------------------------------------------------------
// CScardAccessControl::RemoveSessionFromChannelL
// Remove Session From Channel
// -----------------------------------------------------------------------------
//
inline const TBool CScardAccessControl::RemoveSessionFromChannelL(
    const TInt8 aChannel,
    const TInt aSessionID )
    {
    return iManager->RemoveSessionL( aChannel, aSessionID );
    }

// -----------------------------------------------------------------------------
// CScardAccessControl::ValidateChannelL
// Validate channel
// -----------------------------------------------------------------------------
//
inline void CScardAccessControl::ValidateChannelL(
    const TInt8 aChannel, 
    const TInt aSessionID )
    {
    iManager->ValidateChannelL( aChannel, aSessionID );
    }

// -----------------------------------------------------------------------------
// CScardAccessControl::HandleNextMessageL
// Handle Next Message
// -----------------------------------------------------------------------------
//
inline void CScardAccessControl::HandleNextMessageL(
    const TMessageHandle& aHandle )
    {
    if ( aHandle.iSessionID != ENoSession )
        {
        ( SessionBase( aHandle.iSessionID ) )->ServiceL( aHandle );
        }
    }

// End of File
