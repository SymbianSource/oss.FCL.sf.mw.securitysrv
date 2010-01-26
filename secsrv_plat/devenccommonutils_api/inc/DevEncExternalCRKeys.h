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
* Description:  Cenrep categories and keys related to Device Encryption
*
*/


#ifndef DEVENCEXTERNALCRKEYS_H_
#define DEVENCEXTERNALCRKEYS_H_

#define KCRDevEncUiSettings           0x200025A6

/**
 * Setting for disabling user-based encryption control.
 * 0: UI has full control
 * 1: DM controls phone memory
 * 2: DM controls memory card
 * 3: DM has full control
 **/
#define KDevEncUiDmControl            0x01

/**
 * User setting for memory card encryption. Does not always reflect the actual encryption state.
 * 0: Encryption off (decrypted)
 * 1: Encryption on (encrypted)
 **/
#define KDevEncUserSettingMemoryCard  0x02

// The KDevEncUiDmControl key can contain none, any or both of these values:
#define KDmControlsPhoneMemory   0x01
#define KDmControlsMemoryCard    0x02

#endif /*DEVENCEXTERNALCRKEYS_H_*/
