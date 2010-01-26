/*
* Copyright (c) 2002-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Entry point for DevEncStarter
*
*/


#include <c32comm.h>
#include "DevEncStarterEngine.h"
#include "DevEncLog.h"

// LITERALS
_LIT( KDevEncStarterName, "DevEncStarter" );

// LOCAL FUNCTION PROTOTYPES
static void StartDevEncStarterL();

// ============================= LOCAL FUNCTIONS ==============================

// ----------------------------------------------------------------------------
// Entry-point for the USB Watcher.
// ----------------------------------------------------------------------------
//
TInt E32Main()
    {
    DFLOG( "DevEncStarter => E32Main" );
    TInt ret;
    
    // rename the thread so it is easy to find the panic application
    ret = User::RenameThread( KDevEncStarterName );
    
    if (ret == KErrNone)
        {
        __UHEAP_MARK;
        // create clean-up stack
        CTrapCleanup* cleanup = CTrapCleanup::New();
    
        TRAP( ret, StartDevEncStarterL() );
    
        delete cleanup; // destroy clean-up stack
        __UHEAP_MARKEND;
        }
    
    DFLOG( "DevEncStarter => E32Main ended" );
    return ret; // and return
    }

// ----------------------------------------------------------------------------
// Constructs and installs the active scheduler, constructs USB Watcher's
// objects.
// ----------------------------------------------------------------------------
//
static void StartDevEncStarterL()
    {
    DFLOG( "DevEncStarter => StartDevEncStarterL" );
    CActiveScheduler* activeScheduler = new (ELeave) CActiveScheduler();
    CleanupStack::PushL( activeScheduler );
    CActiveScheduler::Install( activeScheduler );

    // Create the engine
    CDevEncStarterEngine* engine = CDevEncStarterEngine::NewLC();
    
    // Start the active scheduler, the active scheduler is stopped
   	// by the engine, when the phone is shutting down.
   	CActiveScheduler::Start();
    	
   	// Cleanup.
   	CleanupStack::PopAndDestroy( engine );	
   	CleanupStack::PopAndDestroy( activeScheduler );
   	DFLOG( "DevEncStarter => StartDevEncStarterL ended" );
    }

// End of file
