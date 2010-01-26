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
* Description:   The header file of DevTokenServerDebug
*
*/



#ifndef __DEVTOKENSERVERDEBUG_H__
#define __DEVTOKENSERVERDEBUG_H__

#ifdef _DEBUG

#include <e32std.h>


/**
 * OOM testing functionality.
 *
 *  @lib 
 *  @since S60 v3.2
 */
class DevTokenServerDebug
    {
    public:
        // Server-side OOM testing
        static void StartOOMTest();
        
        static void IncHeapFailPoint();
        
        static void ResetHeapFail();
        
        static void PauseOOMTest();
        
        static void ResumeOOMTest();
        
        // Heap checking
        static void HeapCheckStart();
        
        static void HeapCheckEnd();
        
    private:
        
        static void HeapErrorL(TBool aError);
        
    private:
        
        static TInt iCounter;
        
        static TInt iStartCount;
        
        static TInt iPauseCount;
        
        static TInt iInitialAllocCount;
    };

#endif // _DEBUG

#endif //__DEVTOKENSERVERDEBUG_H__

//EOF

