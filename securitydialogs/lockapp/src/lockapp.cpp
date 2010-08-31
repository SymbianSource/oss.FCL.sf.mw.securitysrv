/*
 * Copyright (c) 2000 Nokia Corporation and/or its subsidiary(-ies). 
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
 * Description: Very small version of lockapp, which simply launches Autolock
 * This is needed becauses starter has the hardcoded name "lockapp"
 *
 */

// INCLUDES

#include <e32svr.h>
#include <centralrepository.h>

#include <aknglobalpopupprioritycontroller.h>
#include <apgcli.h>
#include <apgtask.h>
#include <eikenv.h>
#include <e32property.h>
#include <secuisecuritysettings.h>
#include <coreapplicationuisdomainpskeys.h>

// ----------------------------------------------------------------------------------------
// Server startup code
// ----------------------------------------------------------------------------------------
static void RunServerL()
    {
    CActiveScheduler* s = new (ELeave) CActiveScheduler;
    CleanupStack::PushL(s);
    CActiveScheduler::Install(s);

    // start autolock instead of lockapp . This is a backup solution to use in case that not all SysAp and Avkon changes are implemented
    /* No need to check the task. A process should not run twice
     TApaTaskList taskList( CCoeEnv::Static()->WsSession() );	// can also use CCoeEnv::Static()	CEikonEnv::Static()
     const TUid KAutolockSrvAppUid = { 0x100059B5 };
     TApaTask task( taskList.FindApp( KAutolockSrvAppUid ) );
     if( !task.Exists())
     */

    RApaLsSession ls;
    User::LeaveIfError(ls.Connect());
    CleanupClosePushL(ls);

		/************/
    _LIT_SECURITY_POLICY_C1(KWritePolicy, ECapabilityWriteDeviceData);
    TInt ret = RProperty::Define(KPSUidCoreApplicationUIs,
            KCoreAppUIsAutolockStatus, RProperty::EInt, TSecurityPolicy(TSecurityPolicy::EAlwaysPass),
            TSecurityPolicy(TSecurityPolicy::EAlwaysPass));

    TInt autolockState;
    RProperty::Get(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, autolockState);
    if(autolockState==EAutolockStatusUninitialized)
    	{
    	autolockState = EAutolockOff;	// not-initialized means that the unlock-query hasn't been displayed. Therefore the device should not stay locked.
    	}
    ret = RProperty::Set(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus,	
                   autolockState);	// this might re-set it. That's not bad. It will re-notify all listeners.
    RProperty::Get(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, autolockState);
    RDebug::Printf("%s %s (%u) autolockState=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, autolockState);
		/************/

    CApaCommandLine* commandLine = CApaCommandLine::NewLC();
    commandLine->SetExecutableNameL(_L("autolock.exe"));
    commandLine->SetCommandL(EApaCommandRun);
    // Try to launch the application.        
    TInt err = ls.StartApp(*commandLine); // this migh fail
    RDebug::Printf("%s %s (%u) Start: autolock.exe err=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, err);

    CleanupStack::PopAndDestroy(2); // commandLine, ls

    // Initialisation complete, now signal the client
    RProcess::Rendezvous( KErrNone);

    // Ready to run
    CActiveScheduler::Start();

    // Cleanup the server and scheduler
    CleanupStack::PopAndDestroy(2);
    }

// Server process entry-point
TInt E32Main()
    {
    CTrapCleanup* cleanup = CTrapCleanup::New();
    TInt r = KErrNoMemory;
    if (cleanup)
        {
        TRAP(r, RunServerL());
        delete cleanup;
        }
    RDebug::Printf("%s %s (%u) r=%x", __FILE__, __PRETTY_FUNCTION__,
            __LINE__, r);
    return r;
    }

// End of file
