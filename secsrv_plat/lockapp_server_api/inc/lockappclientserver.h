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
* Description:  LockApp Server information
 *
*/


#ifndef __LOCKAPPCLIENTSERVER_H__
#define __LOCKAPPCLIENTSERVER_H__

/**
 * LockApp application Uid
 */
const TUid KLockAppUid =
    {
    0x10283322
    };

/**
 * LockApp service Uid and differentiator
 */
const TUid KLockAppServiceUid =
    {
    0x00000001
    };
const TUint KLockAppServerDiff = 0x00000002;

/**
 * LockApp session version numbers
 */
const TUint KLockAppServMajorVersion = 1;
const TUint KLockAppServMinorVersion = 0;
const TUint KLockAppServBuildVersion = 0;

/**
 * LockApp name and process name
 */
_LIT( KLockAppServerName, "LockApp");
_LIT( KLockAppServerImg, "LockApp");

/**
 * LockApp messages
 */
enum TLockAppMessageReason
    {
    ELockAppEnableKeyguard = 1,
    ELockAppDisableKeyguard,
    ELockAppEnableDevicelock,
    ELockAppDisableDevicelock,
    ELockAppOfferKeyguard,
    ELockAppOfferDevicelock,
    ELockAppShowKeysLockedNote,
#ifdef _DEBUG
    ELockAppTestInternal = 100,
    ELockAppTestDestruct,
    ELockAppTestPanicClient
#endif
    };

#endif // __LOCKAPPCLIENTSERVER_H__
