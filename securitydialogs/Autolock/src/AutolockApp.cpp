/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:
*
*/


// INCLUDE FILES
#include    "AutolockApp.h"
#include    "AutolockDocument.h"
#include <eikstart.h>
#include <apgcli.h>
#include <apgtask.h>
#include <coemain.h>
#include <apacmdln.h>

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CAutolockApp::AppDllUid()
// Returns application UID
// ---------------------------------------------------------
//
TUid CAutolockApp::AppDllUid() const
    {
    return KUidAutolock;
    }

// ---------------------------------------------------------
// CAutolockApp::CreateDocumentL()
// Creates CAutolockDocument object
// ---------------------------------------------------------
//
CApaDocument* CAutolockApp::CreateDocumentL()
    {
    RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    return CAutolockDocument::NewL( *this );
    }

// ================= OTHER EXPORTED FUNCTIONS ==============
//

LOCAL_C CApaApplication* NewApplication()
    {
    	RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );

    TInt use_old_autolock=1;
    if(use_old_autolock)
    	{
			// start autolocksrv instead of autolock . This is a backup solution to use in case that not all SysAp and Avkon changes are implemented
	 	  RDebug::Printf( "%s %s (%u) 1=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0x1 );
	 	  CCoeEnv *env = CCoeEnv::Static();
	    TApaTaskList taskList( env->WsSession() );
	    RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    	const TUid KAutolockSrvAppUid = { 0xE0022E73 };
	    TApaTask task( taskList.FindApp( KAutolockSrvAppUid ) );
	    RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
	    if ( !task.Exists() )
	        {
	    		RDebug::Printf( "%s %s (%u) no KAutolockSrvAppUid found. Creating=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
	        RApaLsSession ls;                   
	        User::LeaveIfError(ls.Connect());   
	        CleanupClosePushL(ls);         
	        
	        CApaCommandLine* commandLine = CApaCommandLine::NewLC();
	        commandLine->SetExecutableNameL( _L("autolocksrv.exe" ) );     
	        commandLine->SetCommandL( EApaCommandRun );
	        
		    // Try to launch the application.        
	        User::LeaveIfError(ls.StartApp(*commandLine));
	    	  RDebug::Printf( "%s %s (%u) autolocksrv.exe created=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0x7 );
	        
	        CleanupStack::PopAndDestroy(2); // commandLine, ls
			}
	   }
		RDebug::Printf( "%s %s (%u) exiting=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0x1 );
    return new CAutolockApp;
    }

GLDEF_C TInt E32Main()
    {
    RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );

    return EikStart::RunApplication(NewApplication);
    }

// End of File  

