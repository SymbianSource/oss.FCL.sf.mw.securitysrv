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
* Description:  Private Publish&Subscribe definitions of the
 *                Security UIs subsystem
 *
*/


#ifndef SECURITYUISPRIVATEPSKEYS_H
#define SECURITYUISPRIVATEPSKEYS_H

// INCLUDES

//CONSTANTS

const TUid KPSUidSecurityUIs = { 0x100059b5 };
// =============================================================================
// Security Code UI Originator API
// =============================================================================

// Use TUid KPSUidSecurityUIs = { 0x100059b5 } 

/**
 * Used by SecUI to differentiate between ETel API originated and SecUI originated
 * security queries.
 * Old Shared Data constant name: KSecUIOriginatedQuery
 */
const TUint32 KSecurityUIsSecUIOriginatedQuery = 0x00000301;
enum TSecurityUIsSecUIOriginatedQuery
    {
    ESecurityUIsSecUIOriginatedUninitialized = 0,
    ESecurityUIsETelAPIOriginated,
    ESecurityUIsSecUIOriginated,
    ESecurityUIsSystemLockOriginated,
    ESecurityUIsFpsOriginated
    };

/**
 * Used by SecUI to tell if a query request set by some ETELMM API lock setting function (i.e. SetXXXXSetting)
 * has been canceled since canceling the setting request does not prompt a query cancel event from ETEL.
 * Old Shared Data constant name: KSecUIOriginatedQuery
 */    
const TUint32 KSecurityUIsQueryRequestCancel = 0x00000302;
enum TSecurityUIsQueryRequestCancel
    {
    ESecurityUIsQueryRequestUninitialized = 0,
    ESecurityUIsQueryRequestOk,
    ESecurityUIsQueryRequestCanceled
    };

/**
 * Used by Autolock to tell which application has enabled/disabled the keyguard/devicelock , and at which moment.
 */    
const TUint32 KSecurityUIsLockInitiatorUID  = 0x00000303;
const TUint32 KSecurityUIsLockInitiatorTimeHigh = 0x00000304;
const TUint32 KSecurityUIsLockInitiatorTimeLow  = 0x00000305;

/**
 * Used to tell SysAp to switch-on the lights.
 */    
const TUint32 KSecurityUIsLights  = 0x00000308;
enum TSecurityUIsLights
    {
    ESecurityUIsLightsUninitialized = 0,
    ESecurityUIsLightsLockOnRequest,
    ESecurityUIsLightsQueryOnRequest,
    ESecurityUIsLightsLockOffRequest,
    ESecurityUIsLightsLastValue
    };

#endif // SECURITYUISPRIVATEPSKEYS_H

// End of File
