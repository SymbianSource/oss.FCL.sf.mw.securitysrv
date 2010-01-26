/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Lock App internal utils
 *
*/


#include <e32base.h>
#include <coemain.h>
#include <apgwgnam.h>
#include "lockapputils.h"

#define KSysApUid TUid::Uid(0x100058F3)
#define KPhoneAppUid TUid::Uid(0x100058B3)

// ---------------------------------------------------------------------------
// Queries bits in aStatusMask specified by the aQueryMask
// ---------------------------------------------------------------------------
TBool IsBitFieldSet( TUint aStatusMask, TUint aQueryMask )
    {
    return (aStatusMask & aQueryMask);
    }

// ---------------------------------------------------------------------------
// Sets bits in aResultMask specified by the aSetMask
// ---------------------------------------------------------------------------
void SetBitField( TUint &aResultMask, TUint aSetMask, TBool aSet )
    {
    if ( aSet )
        {
        aResultMask = aResultMask | aSetMask;
        }
    else
        {
        aResultMask = aResultMask & ~ aSetMask;
        }
    }

// ---------------------------------------------------------------------------
// Panic function for LockApp internal panics.
// ---------------------------------------------------------------------------
void DoPanic( TLockAppPanic aPanic )
    {
    _LIT(KPanic,"LockApp");
    User::Panic( KPanic, aPanic );
    }

// ---------------------------------------------------------------------------
// Causes client Panic (illegal use of the server services).
// Used RMessagePtr2::Panic() also completes the message.
// ---------------------------------------------------------------------------
void PanicClient(const RMessagePtr2& aMessage, TLockAppPanic aPanic )
    {
    _LIT(KPanic, "LockAppServer");
    aMessage.Panic( KPanic, aPanic );
    }

// ---------------------------------------------------------------------------
// Sends application spesific events to Sysap.
// Used mainly for lights control.
// ---------------------------------------------------------------------------
void SendMessageToSysAp( TInt aMessage )
    {
    RWsSession& ws = CCoeEnv::Static()->WsSession( );
    TInt wgId = 0;
    CApaWindowGroupName::FindByAppUid( KSysApUid, ws, wgId );

    // if sysap window group exists
    if ( wgId )
        {
        TWsEvent event;
        event.SetType( aMessage );
        event.SetTimeNow( );
        ws.SendEventToWindowGroup( wgId, event );
        }
    }

// ---------------------------------------------------------------------------
// Sends a key event to the windowgroup in background.
// Used mainly for sending red and green keys to phone app when
// devicelock or keyguard is in foreground.
// ---------------------------------------------------------------------------
void SendKeyToPhoneApp( const TKeyEvent& aKey, TEventCode aType )
    {
    RWsSession& ws = CCoeEnv::Static()->WsSession( );
    TInt wgId = 0;
    CApaWindowGroupName::FindByAppUid( KPhoneAppUid, ws, wgId );

    // if sysap window group exists
    if ( wgId )
        {
        TWsEvent event;
        *event.Key() = aKey;
        event.SetType( aType );
        event.SetTimeNow( );
        ws.SendEventToWindowGroup( wgId, event );
        }
    }

// END OF FILE
