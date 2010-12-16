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
* Description:
*
*/


#include <aknappui.h>
#include <aknkeylock.h>
#include <AknUtils.h>
#include <activitymanager.h>
#include <AknNotifierController.h>
#include <centralrepository.h> 
#include <avkondomainpskeys.h>
#include <e32property.h>
#include <PSVariables.h>
#include <startupdomainpskeys.h>
#include <ctsydomainpskeys.h>
#include <activeidle2domainpskeys.h>
#include <coreapplicationuisdomainpskeys.h>
#include <ScreensaverInternalPSKeys.h>
#include <hwrmdomainpskeys.h>
#include "AutoKeyguardCenRepI.h"
#include "AutoKeyguardObserver.h"
#include "AutolockPrivateCRKeys.h"
#include "AutolockAppUiInterface.h"

const TInt AutoKeyguardOff(60000);
// Screensaver "On" status value
const TInt KSsOn = 1;
// Screensaver started fron idle status value
const TInt KSsStartedFromIdle = 1;
//Flip open
const TInt KFlipOpen = 1;

// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CAutoKeyguardObserver::NewL()
// ----------------------------------------------------------
//

CAutoKeyguardObserver* CAutoKeyguardObserver::NewL( MAutolockAppUiInterface* aAppUiI )
	{
	#ifdef RD_AUTO_KEYGUARD
	#if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutoKeyguardObserver::NewL() BEGIN"));
    #endif
	CAutoKeyguardObserver* self = new (ELeave) CAutoKeyguardObserver( aAppUiI );
	CleanupStack::PushL(self);
	self->ConstructL(self);
	CleanupStack::Pop(); //self
	#if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutoKeyguardObserver::NewL() END"));
    #endif
	return self;
	#else
	return NULL;
	#endif //RD_AUTO_KEYGUARD
	}
//
// ----------------------------------------------------------
// CAutoKeyguardObserver::CAutoKeyguardObserver()
// C++ default constructor
// ----------------------------------------------------------
// 
CAutoKeyguardObserver::CAutoKeyguardObserver( MAutolockAppUiInterface* aAppUiI ):
    iAppUiI( aAppUiI )
	{
	}

//
// ----------------------------------------------------------
// CAutoKeyguardObserver::ConstructL()
// Symbian OS default constructor
// ----------------------------------------------------------
//
void CAutoKeyguardObserver::ConstructL(CAutoKeyguardObserver* aObserver)
	{
	#ifdef RD_AUTO_KEYGUARD
	#if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutoKeyguardObserver::ConstructL() BEGIN"));
    #endif	
	//Central Repository handler
    iCenRepI = CAutoKeyguardCenRepI::NewL(aObserver);
	// Activitymanager
	iActivityManager = CUserActivityManager::NewL(CActive::EPriorityStandard);
	StartActivityMonitoringL();
	#if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutoKeyguardObserver::ConstructL() END"));
    #endif
    #else // !RD_AUTO_KEYGUARD
    iCenRepI = NULL;
	iActivityManager = NULL;
    #endif //RD_AUTO_KEYGUARD
	}
//
// ----------------------------------------------------------
// CAutoKeyguardObserver::StartActivityMonitoringL()
// Start monitoring user activity
// ----------------------------------------------------------
//
void CAutoKeyguardObserver::StartActivityMonitoringL()
	{
	#ifdef RD_AUTO_KEYGUARD	
	SetActivityManagerL();
	#endif //RD_AUTO_KEYGUARD
	}

//
// ----------------------------------------------------------
// CAutoKeyguardObserver::StopActivityMonitoring()
// Stop monitoring user activity
// ----------------------------------------------------------
//
void CAutoKeyguardObserver::StopActivityMonitoring()
	{
	#ifdef RD_AUTO_KEYGUARD
	CancelActivityManager();
	#endif // RD_AUTO_KEYGUARD
	}

//
// ----------------------------------------------------------
// CAutoKeyguardObserver::SetActivityManagerL()
// Initializes activymanager   
// ----------------------------------------------------------
//
void CAutoKeyguardObserver::SetActivityManagerL()
	{
	#ifdef RD_AUTO_KEYGUARD
	#if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutoKeyguardObserver::SetActivityManagerL() BEGIN"));
    #endif
	if (AutoKeyguardTimeout() )
		{
		#if defined(_DEBUG)
    	RDebug::Print(_L("(AUTOLOCK)CAutoKeyguardObserver::SetActivityManagerL() ON: Start manager"));
    	#endif
		iActivityManager->Start(AutoKeyguardTimeout(), TCallBack(HandleInactiveEventL,this), 
								TCallBack(HandleActiveEventL,this));
		}
	else
		{
		#if defined(_DEBUG)
    	RDebug::Print(_L("(AUTOLOCK)CAutoKeyguardObserver::SetActivityManagerL() OFF: Start manager"));
    	#endif
		iActivityManager->Start(AutoKeyguardOff, TCallBack(HandleInactiveEventL,this), 
								TCallBack(HandleActiveEventL,this));
		}

	#if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutoKeyguardObserver::SetActivityManagerL() END"));
    #endif
    #endif //RD_AUTO_KEYGUARD
	}
//
// ----------------------------------------------------------
// CAutoKeyguardObserver::CancelActivityManager()
// UnInitializes activymanager   
// ----------------------------------------------------------
//
void CAutoKeyguardObserver::CancelActivityManager()
	{
	#ifdef RD_AUTO_KEYGUARD
    if ( iActivityManager )
        {
	    iActivityManager->Cancel();
        }
    #endif //RD_AUTO_KEYGUARD
	}
//
// ----------------------------------------------------------
// CAutoKeyguardObserver::~CAutoKeyguardObserver()
// Destructor   
// ----------------------------------------------------------
//
CAutoKeyguardObserver::~CAutoKeyguardObserver()
	{
	#ifdef RD_AUTO_KEYGUARD
	if(iCenRepI)
    	delete iCenRepI;
	if(iActivityManager)
		{
		StopActivityMonitoring();
		delete iActivityManager;
		iActivityManager = NULL;
		}
	#endif //RD_AUTO_KEYGUARD
	}
//
// ----------------------------------------------------------
// CAutoKeyguardObserver::AutoKeyguardTimeout()
// Returns current AutoKeyguard period
// ----------------------------------------------------------
//
TInt CAutoKeyguardObserver::AutoKeyguardTimeout()
	{
	#ifdef RD_AUTO_KEYGUARD
    return iCenRepI->Timeout();
    #else
    return 0;
    #endif //RD_AUTO_KEYGUARD
	}
//
// ----------------------------------------------------------
// CAutoKeyguardObserver::ResetInactivityTimeoutL()
// Gets AutoKeyguard period and starts monitoring user activity 
// ----------------------------------------------------------
//
void CAutoKeyguardObserver::ResetInactivityTimeout()
	{
	#ifdef RD_AUTO_KEYGUARD	
	if (AutoKeyguardTimeout() )
		{
		#if defined(_DEBUG)
    	RDebug::Print(_L("(AUTOLOCK)CAutoKeyguardObserver::ResetInactivityTimeoutL() ON"));
    	#endif
		iActivityManager->SetInactivityTimeout(AutoKeyguardTimeout());
		}
	else
		{
		#if defined(_DEBUG)
    	RDebug::Print(_L("(AUTOLOCK)CAutoKeyguardObserver::ResetInactivityTimeoutL() OFF"));
    	#endif
		iActivityManager->SetInactivityTimeout(AutoKeyguardOff);
		}
	#endif //RD_AUTO_KEYGUARD
	}
//
// ----------------------------------------------------------
// CAutoKeyguardObserver::HandleActiveEventL()
// Handles Active event. Called by ActivityManager
// ----------------------------------------------------------
//
TInt CAutoKeyguardObserver::HandleActiveEventL(TAny* /*aPtr*/)
	{
	return KErrNone;
	}

//
// ----------------------------------------------------------
// CAutoKeyguardObserver::HandleInactiveEventL()
// Handles InActive event. Called by ActivityManager
// ----------------------------------------------------------
//
TInt CAutoKeyguardObserver::HandleInactiveEventL(TAny* aPtr)
	{
	#ifdef RD_AUTO_KEYGUARD
	#if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutoKeyguardObserver::HandleInactiveEventL() BEGIN"));
    #endif
	if ( STATIC_CAST(CAutoKeyguardObserver*, aPtr)->AutoKeyguardTimeout() )
		{
		#if defined(_DEBUG)
    	RDebug::Print(_L("(AUTOLOCK)CAutoKeyguardObserver::HandleInactiveEventL() Lock"));
    	#endif
		STATIC_CAST(CAutoKeyguardObserver*, aPtr)->LockKeysL();
		}
	#if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutoKeyguardObserver::HandleInactiveEventL() END"));
    #endif
    #endif //RD_AUTO_KEYGUARD
	return KErrNone;
	}

//
// ----------------------------------------------------------
// CAutoKeyguardObserver::LockKeysL()
// Locks system
// ----------------------------------------------------------
//
void CAutoKeyguardObserver::LockKeysL()
	{
	#ifdef RD_AUTO_KEYGUARD
	#if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutoKeyguardObserver::LockKeysL() BEGIN"));
    #endif
    CRepository* repository = CRepository::NewL(KCRUidAutolockConf);
    TInt keyguardConf(0);
    //Check local keyguard variation key 
    repository->Get(KAutoKeyLockConf, keyguardConf);
    delete repository;
    if(keyguardConf & KAutoKeylockFeatureIdFlipOpenDisabled)
    {
    	#if defined(_DEBUG)
    	RDebug::Print(_L("(AUTOLOCK)CAutoKeyguardObserver::LockKeysL() Flip variation"));
    	#endif
    	TInt flipState(KFlipOpen);
    	RProperty::Get(KPSUidHWRM, KHWRMFlipStatus, flipState);
	TInt lightStatus=EForcedLightsUninitialized; 
	RProperty::Get(KPSUidCoreApplicationUIs,KLightsVTForcedLightsOn,lightStatus ); 

	//If flip is open and feature flag is on. don't lock
	if(flipState == KFlipOpen || lightStatus == EForcedLightsOn)
    	{
    		#if defined(_DEBUG)
    		RDebug::Print(_L("(AUTOLOCK)CAutoKeyguardObserver::LockKeysL() Flip open"));
    		#endif
    		return;
    	}
    		
    }
  {	// prevent automatic keyguard if grip is open
  	TInt gripValue = 0;
  	RProperty::Get(KPSUidHWRM,KHWRMGripStatus,gripValue );
		#if defined(_DEBUG)
		RDebug::Printf( "%s %s (%u) gripValue=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, gripValue );
 		RDebug::Printf( "%s %s (%u) EPSHWRMGripOpen=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, EPSHWRMGripOpen );
		#endif
		if(gripValue == EPSHWRMGripOpen)
			{
    		#if defined(_DEBUG)
    		RDebug::Printf( "%s %s (%u) avoid keyguard because EPSHWRMGripOpen=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, EPSHWRMGripOpen );
    		#endif
    		return;
			}
  }
	{	// REQ 414-5466 Prevention of device lock in context of Hands Free Voice UI
	TInt vuiValue = 0;
	TUid KHFVuiModePSUid = { 0x102818E7 };
	enum THFVuiModePSKeys
	    {
	    EHFVuiPSModeId = 1000
	    };
	TInt tRet = RProperty::Get(KHFVuiModePSUid, EHFVuiPSModeId, vuiValue);	// also 0 if can't get because permissions or because doesn't exists
	#if defined(_DEBUG)
		RDebug::Printf( "%s %s (%u) gatting KHFVuiModePSUid+EHFVuiPSModeId=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, tRet );
		RDebug::Printf( "%s %s (%u) vuiValue=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, vuiValue );
	#endif
	if(vuiValue)
		{
		#if defined(_DEBUG)
		RDebug::Print(_L("(AUTOLOCK)CAutoKeyguardObserver::LockKeysL() Voice functions active. No locking possible."));
		#endif
		return;
		}
	}

	TInt value = 0;
	TBool keylockOn = EFalse;
	TBool autolockOn = EFalse;
	TBool idle = EFalse;
	TInt callState = EPSCTsyCallStateNone;
	TBool okToLock = EFalse;
	TBool screenSaverOn = EFalse;
	TBool screenSaverStertedFromIdle = EFalse;
        TBool startupOver = EFalse;
	TBool codeQueryOpen = EFalse;
    //Get keyguard status
    RProperty::Get(KPSUidAvkonDomain, KAknKeyguardStatus, value);
    keylockOn = (value == EKeyguardLocked);
    //Get call state
    RProperty::Get(KPSUidCtsyCallInformation, KCTsyCallState, callState);
    
    //See whether we are in idle
 		value = 0;   
    RProperty::Get(KPSUidAiInformation, KActiveIdleState, value);
    idle = (value == EPSAiForeground);
    idle = ETrue; // don't care about idle state. Phone should autolock on any UI, not only HomeSreeen.
    
    value = 0;
    RProperty::Get(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, value);
    autolockOn = (value > EAutolockOff);
	
		value = 0;
	  RProperty::Get(KPSUidScreenSaver, KScreenSaverOn, value);
    screenSaverOn = (value == KSsOn);
    
    value = 0;
    RProperty::Get(KPSUidScreenSaver, KScreenSaverActivatedFromIdle, value);
    screenSaverStertedFromIdle = (value == KSsStartedFromIdle);
    
    // See if SIM is accepted
    if(idle)
    	{
	    value = 0;
	    RProperty::Get(KPSUidStartup, KPSSimStatus, value);
	    // automatic lock can't get enabled if SIM-locked.
	    if(value == ESimNotReady)
	    	idle = EFalse;
	    #if defined(_DEBUG)
	    RDebug::Print(_L("(AUTOLOCK)CAutoKeyguardObserver::LockKeysL() KPSSimStatus state: %d"), value);
	    RDebug::Print(_L("(AUTOLOCK)CAutoKeyguardObserver::LockKeysL() ESimUsable: %d"), ESimUsable);
	    #endif
	  	}

    //See if all the startup related queries and graphics has been displayed
    value = 0;
    RProperty::Get(KPSUidStartup, KPSStartupUiPhase, value);
    startupOver = (value == EStartupUiPhaseAllDone);
        
    #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutoKeyguardObserver::LockKeysL() startupOver: %d"), startupOver);
    RDebug::Print(_L("(AUTOLOCK)CAutoKeyguardObserver::LockKeysL() Startup state: %d"), value);
    #endif
	//See if the lock code query is open
    codeQueryOpen = iAppUiI->DeviceLockQueryStatus();
    #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutoKeyguardObserver::LockKeysL() codeQueryOpen: %d"), codeQueryOpen);
    RDebug::Print(_L("(AUTOLOCK)CAutoKeyguardObserver::LockKeysL() autolockOn: %d"), autolockOn);
    #endif
    if (startupOver)
        {
		// If lock code query is open and device lock is on, cancel the query.
        // AppUi will enable keylock when the query is cancelled
        if (autolockOn && codeQueryOpen)
            {
            #if defined(_DEBUG)
            RDebug::Print(_L("(AUTOLOCK)CAutoKeyguardObserver::LockKeysL() Query open, cancel it"));
            #endif
            iAppUiI->CancelDeviceLockQuery();
            return;
            }
        // If keylock is already ON, there is a ongoing call, 
        // autolock is already ON or phone is not in idle, don't lock.
        if (keylockOn || (callState > EPSCTsyCallStateNone) || autolockOn || !idle)
            {
        	    #if defined(_DEBUG)
    		    RDebug::Print(_L("(AUTOLOCK)CAutoKeyguardObserver::LockKeysL() Don't lock"));
    		    RDebug::Print(_L("(AUTOLOCK)keylockstatus: %d"), keylockOn);
    		    RDebug::Print(_L("(AUTOLOCK)callstate: %d"), callState);
    		    RDebug::Print(_L("(AUTOLOCK)autolockstate: %d"), autolockOn);
    		    RDebug::Print(_L("(AUTOLOCK)idlestate: %d"), idle);
    		    RDebug::Print(_L("(AUTOLOCK)screenSaverOn: %d"), screenSaverOn);
    		    RDebug::Print(_L("(AUTOLOCK)screenSaverStertedFromIdle: %d"), screenSaverStertedFromIdle);
    		    #endif
    		    //If screensaver is on, idle does not have foreground. If Screensaver was started from Idle, lock keys.
    		    if(!idle && screenSaverOn && screenSaverStertedFromIdle && (callState <= EPSCTsyCallStateNone))
    			    okToLock = ETrue;
    		    else
    		        okToLock = EFalse;
            }
         else //Otherwise there's no problem...
         		okToLock = ETrue;
        }
    
    
    if (okToLock)
        {
            #if defined(_DEBUG)
            RDebug::Print(_L("(AUTOLOCK)CAutoKeyguardObserver::LockKeysL() Lock"));
            RDebug::Print(_L("(AUTOLOCK)keylockstatus: %d"), keylockOn);
            RDebug::Print(_L("(AUTOLOCK)callstate: %d"), callState);
            RDebug::Print(_L("(AUTOLOCK)autolockstate: %d"), autolockOn);
            RDebug::Print(_L("(AUTOLOCK)idlestate: %d"), idle);
            RDebug::Print(_L("(AUTOLOCK)screenSaverOn: %d"), screenSaverOn);
            RDebug::Print(_L("(AUTOLOCK)screenSaverStertedFromIdle: %d"), screenSaverStertedFromIdle);
            #endif
		//Tell keylock to lock itself.
		RAknKeyLock keylock;
        if ( keylock.Connect() == KErrNone )
          {
          	#if defined(_DEBUG)
    		RDebug::Print(_L("(AUTOLOCK)CAutoKeyguardObserver::LockKeysL() Connect OK"));
    		#endif
    			 if(AknLayoutUtils::PenEnabled())
    			     keylock.EnableWithoutNote(); //Enable without note
    			 else
    			     keylock.EnableKeyLock();
           keylock.Close();
          }
		}
	#if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutoKeyguardObserver::LockKeysL() END"));
    #endif
    #endif //RD_AUTO_KEYGUARD
} 
	

// END OF FILE
