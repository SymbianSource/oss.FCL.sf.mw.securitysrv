/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Autolock local variation Central Repository keys.
 *
*/


#ifndef AUTOLOCKPRIVATECRKEYS_H
#define AUTOLOCKPRIVATECRKEYS_H

// =============================================================================
// Autolock Configuration API
// =============================================================================
const TUid KCRUidAutolockConf = { 0x102824AE };

/**
 * Bitmask used in configuring automatic keylock.
 */
const TUint32 KAutoKeyLockConf = 0x00000001;

// =============================================================================
// Automatic Keylock Local Variation constants (KAutoKeyLockConf)
// =============================================================================

/** 
 * KAutolockFeatureIdFlipOpenDisabled is an on/off setting for 
 * disabling automatic keyguard locking when flip is open. By default off.
 * Possible values: 0 (locking enabled), 1 (locking diabled) <-- 
 */
const TInt KAutoKeylockFeatureIdFlipOpenDisabled= 0x01;  // 2^0

#endif
