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
* Description:  Internal Central Repository keys.
 *
 *
*/


#ifndef LOCK_DOMAIN_CR_KEYS_H
#define LOCK_DOMAIN_CR_KEYS_H

// INCLUDES
#include <centralrepository.h>

const TUid KCRUidLockConf = { 0x10283322 };

/**
 * Products must configure this value depending on 
 * what lock/unlock approach is supported by the device hardware
 *
 * Possible product modes are are:
 *
 * EKeyguardDefaultHardware = 0 // no addional hardware support
 * EKeyguardOnePositionSwitch = 1 // one position spring adjusted switch
 * EKeyguardTwoPositionSwitch = 2 // on-off type keyguard switch
 *
 * EKeyguardDefaultHardware is default in the platform.
 */
const TUint32 KKeyguardHardwareConf = 0x00000001;

enum TLockHardware
    {
    EKeyguardDefaultHardware = 0,
    EKeyguardOnePositionSwitch,
    EKeyguardTwoPositionSwitch
    };

/**
 * Products and applications can set on run-time if keyguard
 * feature should be momentarily disabled/enabled.
 *
 * Possible run-time modes are are:
 *
 * EKeyguardNotAllowed = 0 // enabling keyguard is not allowed
 * EKeyguardAllowed = 1 // keyguard works normally
 *
 * EKeyguardAllowed is default in the platform.
 */    
const TUint32 KLockKeyguardAllow = 0x00000002;

enum TKeyguardAllow
    {
    EKeyguardNotAllowed = 0,
    EKeyguardAllowed = 1,
    };

/**
 * Products and applications can configure lock timer 
 * interval in microseconds.
 *
 * KLockDefaultInterval of 2000000 microseconds is default in the platform.
 */       
const TUint32 KLockTimerInterval = 0x00000003;

/**
 * Products and applications can change between different  
 * Keylock policy configurations by changing policy mode.
 * 
 * Possible run-time modes are are:
 *
 * ELockDefault = 0 // Used key configuration in default setup
 * ELockSlider = 1 // Used key configuration in slider setup
 * 
 * Products can define new modes if needed.
 * 
 * Configuration ELockDefault is default in the platform.
 */       
const TUint32 KLockPolicyMode = 0x00000004;

/**  
 * Physical keys (scan code) used for locking/unlocking/query have been stored 
 * in central repository keys with identification value of defined as     
 * 0x000000XX Key configurations supported in the mode+type, e.g. 00 = left soft key + right soft key, 01 = left soft key + Fn
 * 0x00000X00 Key code sequence number. 1 = primary key , 2 = secondary key
 * 0x0000X000 from TLockPolicyType, 1= activate lock, 2 = deactivate lock, 3 = devicelock query
 * 0x000X0000 from TLockPolicyMode, 1 = default configuration, 2 = slider configuration
 *
 * The default platform locking/unlocking/query configurations have been listed below.
 */    

/**
 * Pre-defined policymodes, 1 = default configuration, 2 = slider configuration
 * Modes are identified in a hexadecimal reserved in mask 0x000X0000.
 */
enum TLockPolicyMode
    {
    ELockNone = 0,
    ELockDefault = 1,
    ELockSlider = 2,
    };

/**
 * Key code sequence number. 1 = primary key , 2 = secondary key.
 * Key number is identified in a hexadecimal reserved in mask 0x00000X00.
 */
const TUint32 KLockFirstPrimaryKey = 0x00000100;
const TUint32 KLockFirstSecondaryKey= 0x00000200;

/**
 * ============================================================================
 * Policy mode default (left soft key+* for unlock/lock and left soft key for device lock)
 * (not used directly)
 * ============================================================================
 */ 
    
/**
 * First scan code key used for locking the phone in the default mode.
 * Works when mode = ELockDefault and type = EActivateKeyguard.
 * Default value is left soft key value with scan code 0xa4.
 */
const TUint32 KLockDefaultLockPrimaryConf00 = 0x00011100;
    
/**
 * Second scan code key used for locking the phone in the default mode.
 * Works when mode = ELockDefault and type = EActivateKeyguard.
 * Default value is star key on ITU-keypad with scan code 0x2a.
 */
const TUint32 KLockDefaultLockSecondaryConf00 = 0x00011200;

/**
 * First scan code key used for unlocking the phone in the default mode.
 * Works when mode = ELockDefault and type = EDeactivateKeyguard.
 * Default value is left soft key with scan code 0xa4.
 */
const TUint32 KLockDefaultUnlockPrimaryConf00 = 0x00012100;
    
/**
 * Second scan code key used for unlocking the phone in the default mode.
 * Works when mode = ELockDefault and type = EDeactivateKeyguard.
 * Default value is star key with scan code 0x2a.
 */
const TUint32 KLockDefaultUnlockSecondaryConf00 = 0x00012200;

/**
 * Scan code key used for showing security query when device is locked 
 * in the default mode.
 * Works when mode = ELockDefault and type = EDevicelockQuery.
 * Default value is left soft key with scan code 0xa4.
 */
const TUint32 KLockDefaultSecQueryPrimaryConf00 = 0x00013100;

/**
 * ============================================================================
 * Policy mode slider (left soft key+right soft key for unlock/lock and left soft key for device lock)
 * (not used directly)
 * ============================================================================
 */ 
 
/**
 * First scan code key used for locking the phone in the slider mode.
 * Works when mode = ELockSlider and type = EActivateKeyguard.
 * Default value is left soft key with scan code 0xa4.
 */
const TUint32 KLockSliderLockPrimaryConf00 = 0x00021100;

/**
 * Second scan code key used for locking the phone in the slider mode.
 * Works when mode = ELockSlider and type = EActivateKeyguard.
 * Default value is right soft key with scan code 0xa5.
 */
const TUint32 KLockSliderLockSecondaryConf00 = 0x00021200;

/**
 * First scan code key used for unlocking the phone in the slider mode.
 * Works when mode = ELockSlider and type = EDeactivateKeyguard.
 * Default value is left soft key with scan code 0xa4.
 */
const TUint32 KLockSliderUnlockPrimaryConf00 = 0x00022100;

/**
 * Second scan code key used for unlocking the phone in the slider mode.
 * Works when mode = ELockSlider and type = EDeactivateKeyguard.
 * Default value is right soft key with scan code 0xa5.
 */
const TUint32 KLockSliderUnlockSecondaryConf00 = 0x00022200;

/**
 * Scan code key used for showing security query when device is locked 
 * in the slider mode.
 * Works when mode = ELockSlider and type = EDevicelockQuery.
 * Default value is left soft key with scan code 0xa4.
 */
const TUint32 KLockSliderSecQueryPrimaryConf00 = 0x00023100;

#endif

// End of file
