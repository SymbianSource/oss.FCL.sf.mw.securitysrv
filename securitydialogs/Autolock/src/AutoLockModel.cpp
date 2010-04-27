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

#include <aknappui.h>
#include <activitymanager.h>
#include <bldvariant.hrh>
#include <featmgr.h>
#include <AknNotifierController.h>
#include <featmgr.h>
#include <e32property.h>
#include <PSVariables.h>
#include <startupdomainpskeys.h>
#include <coreapplicationuisdomainpskeys.h>
#include <ctsydomainpskeys.h>

#include <SCPClient.h>

#include "AutoLockModelPS.h"  
#include "AutolockAppUiPS.h"
#include "AutoLockLockObserverPS.h"
#include "AutoLockCenRepI.h"


const TInt AutoLockOff(60000);



// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CAutoLockModel::NewL()
// ----------------------------------------------------------
//

CAutoLockModel* CAutoLockModel::NewL(CAutolockAppUi* aAppUi, TBool aLocked)
  {
  CAutoLockModel* self = new (ELeave) CAutoLockModel(aAppUi);
  CleanupStack::PushL(self);
  self->ConstructL( aLocked );
  CleanupStack::Pop(); //self
  return self;
  }
//
// ----------------------------------------------------------
// CAutoLockModel::CAutoLockModel()
// C++ default constructor
// ----------------------------------------------------------
// 
CAutoLockModel::CAutoLockModel(CAutolockAppUi* aAppUi) : 
 iAppUi( aAppUi ), iMonitoring(EFalse)  
  {
  }

//
// ----------------------------------------------------------
// CAutoLockModel::ConstructL()
// Symbian OS default constructor
// ----------------------------------------------------------
//
void CAutoLockModel::ConstructL( TBool aLocked )
  {
    FeatureManager::InitializeLibL();
  #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutoLockModel::ConstructL() BEGIN"));
  #endif  
    iCenRepI = CAutolockCenRepI::NewL(iAppUi);
  // lock status observer
  iLockObserver = CLockObserver::NewL(iAppUi);
  // Activitymanager
  iActivityManager = CUserActivityManager::NewL(CActive::EPriorityStandard);
  StartActivityMonitoringL();
    // In GSM the device is always unlocked.
    // In CDMA, SecClientUi will lock the device on boot-up if needed.
    if ( aLocked == EFalse ) {       
       SetLockedL(EAutolockOff);
       #if defined(_DEBUG)
       RDebug::Print(_L("(AUTOLOCK)CAutoLockModel::ConstructL() EAutolockOff"));
       #endif       
    }     
       
    #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutoLockModel::ConstructL() END"));
    #endif
  }
  //
// ----------------------------------------------------------
// CAutoLockModel::StartActivityMonitoringL()
// Start monitoring user activity
// ----------------------------------------------------------
//
void CAutoLockModel::StartActivityMonitoringL()
  { 
  SetActivityManagerL();
  }

//
// ----------------------------------------------------------
// CAutoLockModel::StopActivityMonitoring()
// Stop monitoring user activity
// ----------------------------------------------------------
//
void CAutoLockModel::StopActivityMonitoring()
  {
  CancelActivityManager();
  }

//
// ----------------------------------------------------------
// CAutoLockModel::SetActivityManagerL()
// Initializes activymanager   
// ----------------------------------------------------------
//
void CAutoLockModel::SetActivityManagerL()
  {
  if (AutoLockTimeout() )
    {
    iActivityManager->Start(AutoLockTimeout(), TCallBack(HandleInactiveEventL,this), 
                TCallBack(HandleActiveEventL,this));
    }
  else
    {
    iActivityManager->Start(AutoLockOff, TCallBack(HandleInactiveEventL,this), 
                TCallBack(HandleActiveEventL,this));
    }

  }
//
// ----------------------------------------------------------
// CAutoLockModel::CancelActivityManager()
// UnInitializes activymanager   
// ----------------------------------------------------------
//
void CAutoLockModel::CancelActivityManager()
  {
    if ( iActivityManager )
        {
      iActivityManager->Cancel();
        }
  delete iActivityManager;
  iActivityManager = NULL;
  }
//
// ----------------------------------------------------------
// CAutoLockModel::~CAutoLockModel()
// Destructor   
// ----------------------------------------------------------
//
CAutoLockModel::~CAutoLockModel()
  {
    delete iCenRepI;
  delete iLockObserver;
  StopActivityMonitoring();
  FeatureManager::UnInitializeLib();
    // close custom phone
  }
//
// ----------------------------------------------------------
// CAutoLockModel::AutoLockTimeout()
// Returns current autolock period
// ----------------------------------------------------------
//
TInt CAutoLockModel::AutoLockTimeout()
  {
    return iCenRepI->Timeout();
  }
//
// ----------------------------------------------------------
// CAutoLockModel::ResetInactivityTimeoutL()
// Gets autolock period and starts monitoring user activity 
// ----------------------------------------------------------
//
void CAutoLockModel::ResetInactivityTimeout()
  { 
  if (AutoLockTimeout() )
    {
    iActivityManager->SetInactivityTimeout(AutoLockTimeout());
    }
  else
    {
    iActivityManager->SetInactivityTimeout(AutoLockOff);
    }
  }
//
// ----------------------------------------------------------
// CAutoLockModel::HandleActiveEventL()
// Handles Active event. Called by ActivityManager
// ----------------------------------------------------------
//
TInt CAutoLockModel::HandleActiveEventL(TAny* /*aPtr*/)
  {
  return KErrNone;
  }

//
// ----------------------------------------------------------
// CAutoLockModel::HandleInactiveEventL()
// Handles InActive event. Called by ActivityManager
// ----------------------------------------------------------
//
TInt CAutoLockModel::HandleInactiveEventL(TAny* aPtr)
  {
  if ( STATIC_CAST(CAutoLockModel*, aPtr)->AutoLockTimeout() )
    {
      TInt value(EStartupUiPhaseUninitialized);
      RProperty::Get(KPSUidStartup, KPSStartupUiPhase, value);
      //Don't lock unless boot is over.
      if(value == EStartupUiPhaseAllDone)
        {
        #if defined(_DEBUG)
        RDebug::Print(_L("(AUTOLOCK)CAutoLockModel::HandleInactiveEventL() Boot over"));
        #endif
        #ifdef RD_REMOTELOCK
        STATIC_CAST(CAutoLockModel*, aPtr)->LockSystemL(ETimerLocked);
        #else
        STATIC_CAST(CAutoLockModel*, aPtr)->LockSystemL(EAutolockOn);
        #endif //RD_REMOTELOCK
        }
      else
      {
        #if defined(_DEBUG)
        RDebug::Print(_L("(AUTOLOCK)CAutoLockModel::HandleInactiveEventL() In boot; don't lock"));
        #endif
      }
    }
  return KErrNone;
  }

//
// ----------------------------------------------------------
// CAutoLockModel::LockSystemL()
// Locks system
// ----------------------------------------------------------
//
void CAutoLockModel::LockSystemL(TInt aAutolockState)
  {
  #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutoLockModel::LockSystemL() BEGIN"));
    #endif
    // If already locked, do nothing. Otherwise we'll end up
    // on top of Screensaver
    // Check if iSideKey2 is zero or not (locked if nonzero)
    // Also, phone should not be locked if PUK1 code query is up.
#ifdef FF_STARTUP_OMA_DM_SUPPORT  // New booting order  Start ID:  MVKS-7PZDZ5
    TInt autolock_value = 0;
    RProperty::Get(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, autolock_value);  
    if (autolock_value == EAutolockStatusUninitialized)
    {
        return;         
    }   
#endif  //End ID:  MVKS-7PZDZ5
    if (iAppUi->Locked() || iAppUi->IsPinBlocked())
        {
        return;
        }
  
  TInt lightStatus=EForcedLightsUninitialized; 
  RProperty::Get(KPSUidCoreApplicationUIs,KLightsVTForcedLightsOn,lightStatus ); 
  //If display is forced on. don't lock 
  if(lightStatus == EForcedLightsOn ) 
    {
    #if defined(_DEBUG) 
    RDebug::Print(_L("(AUTOLOCK)CAutoLockModel::LockSystemL() Display is forced on. Device not locked")); 
    #endif 
    return;
    }

    //Check which state we are in to see if it's OK to lock the phone
    //In CDMA when there is no SIM (RUIM) support we should be able to lock
    //the phone after reboot. In this case ESWStateNormal is too late to lock the phone
    //and other states below are needed.
    TBool okToLock = EFalse;
    TInt sysState = 0;
    iProperty.Get(KPSUidStartup, KPSGlobalSystemState, sysState);    
    //If NOT in CDMA the Autolock should come up only after the phone has booted up.
    if ( FeatureManager::FeatureSupported( KFeatureIdProtocolCdma ) || iAppUi->HiddenReset())
        {
        if( (sysState == ESwStateNormalRfOn       ||
            sysState == ESwStateNormalRfOff       ||
             sysState == ESwStateCriticalPhaseOK) &&
            (aAutolockState > EAutolockOff) ) // EMKK-7N3G7R
            {
            #if defined(_DEBUG)
            RDebug::Print(_L("(AUTOLOCK)CAutoLockModel::LockSystemL() LOCKED AFTER HIDDEN RESET"));
            #endif
            okToLock = ETrue;
            }
        }
    else //Feature Manager
        {
        if( (sysState == ESwStateNormalRfOn       ||
             sysState == ESwStateNormalRfOff)      &&
            (aAutolockState > EAutolockOff) ) // EMKK-7N3G7R
            {
            okToLock = ETrue;
            }
        }
   TInt tarmFlag=0;
if(FeatureManager::FeatureSupported(KFeatureIdSapTerminalControlFw ))    
{
    // Get the TARM admin flag value
  TInt tRet = RProperty::Get( KSCPSIDAutolock, SCP_TARM_ADMIN_FLAG_UID, tarmFlag );
    
    if ( tRet != KErrNone )
        {
        #if defined(_DEBUG)
        RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::BringAppToForegroundL():\
            Warning: failed to get TARM Admin Flag state"));
        #endif
        }
    else
        {
        #if defined(_DEBUG)
        RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::BringAppToForegroundL(): TARM flag: %d"),
            tarmFlag );
        #endif
        }
}
    
    TInt callState=0;
    iProperty.Get(KPSUidCtsyCallInformation, KCTsyCallState, callState);
TBool isConditionSatisfied = EFalse;
if(FeatureManager::FeatureSupported(KFeatureIdSapTerminalControlFw ))
{        
    if ( ( callState != EPSCTsyCallStateNone ) && (!( tarmFlag & KSCPFlagAdminLock )) )    
          isConditionSatisfied = ETrue;
}
else
{
    if ( callState != EPSCTsyCallStateNone )
        isConditionSatisfied = ETrue;
}
    if (isConditionSatisfied)
        {
        TBool remoteLocked(EFalse);
        #ifdef RD_REMOTELOCK
        remoteLocked = (aAutolockState == ERemoteLocked);
        #endif //RD_REMOTELOCK
         if(remoteLocked) 
         {
            #if defined(_DEBUG)
            RDebug::Print(_L("(AUTOLOCK)CAutoLockModel::LockSystemL() REMOTE LOCKED"));
            #endif
            okToLock = ETrue;
            
         }
         else
            okToLock = EFalse;        
        }
       
    if (!iAppUi->IsForeground() && okToLock)
        {
        #if defined(_DEBUG)
        RDebug::Print(_L("(AUTOLOCK)CAutoLockModel::LockSystemL() LOCK PHONE"));
        #endif   
    // close fast-swap window
    CEikonEnv::Static()->DismissTaskList();
    // inform Avokon & Other app that system is locked
    // unless the value has already been set in secuisystemlock
    #ifdef RD_REMOTELOCK
    if(aAutolockState != EManualLocked)
        {
            #if defined(_DEBUG)
                RDebug::Print(_L("(AUTOLOCK)CAutoLockModel::LockSystemL() Timer/Remote locked: %d"), aAutolockState);
                #endif
            SetLockedL(aAutolockState);       
        }
    else if((aAutolockState == EManualLocked)  && !iAppUi->Locked() && iAppUi->HiddenReset())
         {   //set the PubSub key if we are to be locked after a hidden reset has occurred.
             #if defined(_DEBUG)
                 RDebug::Print(_L("(AUTOLOCK)CAutoLockModel::LockSystemL() HIDDEN RESET LOCK"));
                 #endif
             SetLockedL(aAutolockState);     
         }
    else
        {   //Normal manual lock from power key. Just set the CenRep key.
            iCenRepI->SetLockedL(okToLock);
        }
    #else //! RD_REMOTELOCK
    SetLockedL(aAutolockState);
    #endif//RD_REMOTELOCK
    // lock keys
    iAppUi->LockKeysL();
    // app to foreground
    iAppUi->BringAppToForegroundL();
         // Reset inactivity time so that Screensaver gets to
        // run again after its timeout. We'll ignore the new
        // inactivity timeout, if already locked
        // RDebug::Printf( "%s %s (%u) CR 428-469 avoid User::ResetInactivityTime=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
        // User::ResetInactivityTime();
    }
    #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutoLockModel::LockSystemL() END"));
    #endif
} 
  
//
// ----------------------------------------------------------
// CAutoLockModel::SetLocked
// Sets lockvalue in Publish & Subscribe and Central Repository
// ----------------------------------------------------------
//
void CAutoLockModel::SetLockedL(TInt aAutolockState)
  {
  #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutoLockModel::SetLockedL() begin"));
    #endif
  TBool locked = (aAutolockState > EAutolockOff);
  if (locked)
    {
        iProperty.Set(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, aAutolockState);
        #if defined(_DEBUG)
        RDebug::Print(_L("(AUTOLOCK)CAutoLockModel::SetLockedL() LOCK"));
        #endif
    }   
  else
    {       
        iProperty.Set(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, EAutolockOff);
        #if defined(_DEBUG)
        RDebug::Print(_L("(AUTOLOCK)CAutoLockModel::SetLockedL() UNLOCK"));         
        #endif
    }

    iCenRepI->SetLockedL(locked);
    #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutoLockModel::SetLockedL() end"));
    #endif
  }

// END OF FILE
