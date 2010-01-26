/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   The header file of DevTokenUtil
*
*/



#ifndef __DEVTOKENUTIL_H__
#define __DEVTOKENUTIL_H__

#include "DevTokenTypesEnum.h"
#include "DevTokenCliServ.h"

// Panic the client
void PanicClient(const RMessage2& aMessage, ETokenTypeServerPanic aPanic);
// Panic the server
void PanicServer(ETokenTypeServerPanic aPanicCode);
// Assert macro
#define assert(TEST, PANIC) __ASSERT_ALWAYS(TEST, PanicServer(PANIC))

class RFs;

// Length of filename buffer for store path
const TUint KMaxFilenameLength = 60;

/**
 * Files utilities used by all servers.
 
 * @lib 
 * @since S60 v3.2
 */
struct FileUtils
    {
    // Determine whether a file exists
    static TBool ExistsL(RFs& aFs, const TDesC& aFile);
    
    // Ensure all directories in a file's path exist, creating them if not
    static void EnsurePathL(RFs& aFs, const TDesC& aFile);
    
    // Copy a file from one location to another
    static void CopyL(RFs& aFs, const TDesC& aSouce, const TDesC& aDest);
    
    // Construct a filename in the server's private area
    static void MakePrivateFilenameL(RFs& aFs, const TDesC& aLeafName, TDes& aNameOut);
    
    // Construct a filename in the server's private area in ROM
    static void MakePrivateROMFilenameL(RFs& aFs, const TDesC& aLeafName, TDes& aNameOut);
    };

#endif  // __DEVTOKENUTIL_H__
