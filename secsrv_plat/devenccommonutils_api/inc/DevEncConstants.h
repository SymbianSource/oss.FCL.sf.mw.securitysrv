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


#ifndef __DEVENCCONSTANTS_H__
#define __DEVENCCONSTANTS_H__


enum TDevEncUiMemoryType
    {
    EPhoneMemory,
    EMemoryCard,
    EPrimaryPhoneMemory
    };

enum TDevEncUiMemoryEntityState
    {
    EMemStateUnknown = 0,
    EMemDecrypted    = 1,
    EMemEncrypting   = 2,
    EMemEncrypted    = 3,
    EMemDecrypting   = 4,
    EMemCorrupted    = 5
    };

enum TDevEncUiMmcStatus
    {
    EMmcNotPresent = 0,
    EMmcNotReadable = 1,
    EMmcOk = 2   
    };
    
enum TDevEncControl
    {
    EDevEncUiFullControl   = 100, //DE UI has full control
    EDMControlsPhoneMemory = 101, //DM controls phone memory
    EDMControlsMmc         = 102, //DM controls Memory card
    EDMFullControl         = 103  //DM has full control
    };
#endif // __DEVENCCONSTANTS_H__
