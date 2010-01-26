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
* Description:  Internally used panic functions and test macros
 *
*/


#ifndef __LOCKAPPUTILS_H__
#define __LOCKAPPUTILS_H__

// INCLUDES
#include "lockapp.hrh"
#include "lockapptrace.h"
#include <w32std.h>

/**
 * Queries bits in aStatusMask specified by the aQueryMask
 */
TBool IsBitFieldSet( TUint aStatusMask, TUint aQueryMask );

/**
 * Sets bits in aResultMask specified by the aSetMask
 */
void SetBitField( TUint &aResultMask, TUint aSetMask, TBool aSet );

/**
 * Panic the LockApp (should only be used in debug)
 *
 * @param aPanic Identifies the reason to Panic
 */
void DoPanic( TLockAppPanic aPanic );

/**
 * Panic LockAppServer client for sending
 * illegal message
 *
 * @param aMessage Event id
 * @param aPanic Identifies the reason to Panic
 */
void PanicClient( const RMessagePtr2& aMessage, TLockAppPanic aPanic );

/**
 * Sends application spesific events to Sysap.
 * Used mainly for lights control.
 *
 * @param aMessage Event id
 */
void SendMessageToSysAp(TInt aMessage);

/**
 * Sends a key event to the windowgroup in background.
 * Used mainly for sending red and green keys to phone app when
 * devicelock or keyguard is in foreground.
 *
 * @param aKey key event
 * @param aType key event type
 */
void SendKeyToPhoneApp(const TKeyEvent& aKey, TEventCode aType);

#endif // __LOCKAPPTILS_H__

