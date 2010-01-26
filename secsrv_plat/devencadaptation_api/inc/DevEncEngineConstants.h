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
* Description:  Enumerations used in the application UI.
 *
*/


#ifndef __DEVENCADAPTATION_H_
#define __DEVENCADAPTATION_H_


//  Constants

_LIT(KEncryptionDll, "\\sys\\bin\\nfeengine.dll");

enum TDiskStatus
    {
    EUnmounted 	= 0,
    EDecrypted 	= 8,
    EDecrypting = 9,
    EEncrypted 	= 10,
    EEncrypting = 11,
    EWiping 	= 12,
    ECorrupted 	= 13,
    };

enum TEncCommands
    {
    EDiskStatus	 = 0,
    EEncryptDisk = 1,
    EDecryptDisk = 2,
    EWipeDisk    = 3
    };

//- Constants ---------------------------------------------------------------
// The KFileServerUidValue which is defined in f32file.h.
const TUid  KEncUtility = {0x100039e3};

const TUint KDEToThreadKey 	= 1;
const TUint KDEToUiKey 		= 2;
const TUint KDEToExtKey 	= 3; 
const TUint KDEProgressToUiKey 	= 4;
const TUint KDEStatusToUiKey 	= 5;

//- Macros ------------------------------------------------------------------
// Calculates pub&sub key for given drive and id. Top 8 bits are used for the
// drives. Bottom 8 bits are used for the ids. The rest of the bits are
// reserved and use zero value. The key layout:
//          dddddddd0000000000000000kkkkkkkk
//          ^bit 31                        ^bit 0
#define ENC_KEY(drive, id) (((drive) << 24) | (0xFF & (id)))


#endif // __DEVENCADAPTATION_H_
