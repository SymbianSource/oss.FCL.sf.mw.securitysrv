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
* Description:   Implementation of DevTokenServerDebug
*
*/



#ifdef _DEBUG

#include <f32file.h>
#include "DevTokenServerDebug.h"

_LIT(KHeapErrorFile, "C:\\devtokenserver_heap_error");

TInt DevTokenServerDebug::iCounter = 0;
TInt DevTokenServerDebug::iStartCount = 0;
TInt DevTokenServerDebug::iPauseCount = 0;
TInt DevTokenServerDebug::iInitialAllocCount = 0;

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// DevTokenServerDebug::StartOOMTest()
// ---------------------------------------------------------------------------
// 
void DevTokenServerDebug::StartOOMTest()
    {
    iStartCount = User::CountAllocCells();
    iCounter = 0;
    }


// ---------------------------------------------------------------------------
// DevTokenServerDebug::IncHeapFailPoint()
// ---------------------------------------------------------------------------
// 
void DevTokenServerDebug::IncHeapFailPoint()
    {
    __UHEAP_FAILNEXT(iCounter); 
    ++iCounter;
    }


// ---------------------------------------------------------------------------
// DevTokenServerDebug::ResetHeapFail()
// ---------------------------------------------------------------------------
// 
void DevTokenServerDebug::ResetHeapFail()
    {
    __UHEAP_RESET;
    }


// ---------------------------------------------------------------------------
// DevTokenServerDebug::PauseOOMTest()
// ---------------------------------------------------------------------------
// 
void DevTokenServerDebug::PauseOOMTest()
    {
    ASSERT(iPauseCount == 0); // Don't nest
    if (iCounter)
        {
        __UHEAP_RESET;
        iPauseCount = User::CountAllocCells();
        }
    }


// ---------------------------------------------------------------------------
// DevTokenServerDebug::ResumeOOMTest()
// ---------------------------------------------------------------------------
// 
void DevTokenServerDebug::ResumeOOMTest()
    {
    if (iCounter)
        {
        ASSERT(iPauseCount > 0);
        __UHEAP_FAILNEXT(iCounter - (iPauseCount - iStartCount));
        iPauseCount = 0;
        }
    }


// ---------------------------------------------------------------------------
// DevTokenServerDebug::HeapCheckStart()
// ---------------------------------------------------------------------------
// 
void DevTokenServerDebug::HeapCheckStart()
    {
    iInitialAllocCount = User::CountAllocCells();
    }


// ---------------------------------------------------------------------------
// DevTokenServerDebug::HeapCheckEnd()
// ---------------------------------------------------------------------------
//
void DevTokenServerDebug::HeapCheckEnd()
    {
    TInt finalAllocCount = User::CountAllocCells();
    TRAP_IGNORE(HeapErrorL(finalAllocCount != iInitialAllocCount));
    // ignore errors
    }


// ---------------------------------------------------------------------------
// DevTokenServerDebug::HeapErrorL()
// Write or delete a file to signify to the test code that there's a memory leak
// in the server.  This is unfortunately the best way of doing it since we can't
// trap a panic when the server's shutting down.
// ---------------------------------------------------------------------------
//
void DevTokenServerDebug::HeapErrorL(TBool aError)
    {
    RFs fs;
    User::LeaveIfError(fs.Connect());

    if (aError)
        {
        RFile file;
        TInt err = file.Create(fs, KHeapErrorFile, EFileWrite | EFileShareExclusive);
        if (err != KErrNone || err != KErrAlreadyExists)
            {
            User::Leave(err);
            }
        file.Close();
        }
    else
        {
        TInt err = fs.Delete(KHeapErrorFile);
        if (err != KErrNone && err != KErrNotFound)
            {
            User::Leave(err);
            }
        }
    }

#endif

//EOF

