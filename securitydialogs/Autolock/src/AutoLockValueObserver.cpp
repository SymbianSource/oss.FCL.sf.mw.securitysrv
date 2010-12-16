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
*   Observer for phone events. Used to deactive/active the side-key
*
*
*/


#include    <e32property.h>
#include  <PSVariables.h>
#include <ctsydomainpskeys.h>
#include  "AutolockAppUiPS.h"
#include  "AutoLockValueObserverPS.h"
#include <coreapplicationuisdomainpskeys.h>
#include <startupdomainpskeys.h>

#include <apgtask.h>
#include <secuisecurityhandler.h>

// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CValueObserver::NewL()
// Constructs a new entry with given values.
// ----------------------------------------------------------
//
CValueObserver* CValueObserver::NewL(CAutolockAppUi* aAppUi)
    {
    #if defined(_DEBUG) 
    RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    #endif    
    CValueObserver* self = new (ELeave) CValueObserver(aAppUi);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }
//
// ----------------------------------------------------------
// CValueObserver::CValueObserver()
// Destructor
// ----------------------------------------------------------
//
CValueObserver::~CValueObserver()
    { 
    Cancel();
    iProperty.Close();  
    }
//
// ----------------------------------------------------------
// CValueObserver::Start()
// Starts listening KUidCurrentCall event  
// ----------------------------------------------------------
//
TInt CValueObserver::Start()
    {
    if (IsActive())
        return KErrInUse;
    iStatus = KRequestPending;
    //iProperty.Attach(KPSUidCtsyCallInformation, KCTsyCallState); 
    iProperty.Subscribe(iStatus);
    SetActive();
    #if defined(_DEBUG)
    RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    #endif
    return KErrNone;
    }
//
// ----------------------------------------------------------
// CValueObserver::Stop()
// Stops listening KUidCurrentCall event 
// ----------------------------------------------------------
//
void CValueObserver::Stop()
  {
  #if defined(_DEBUG)
  RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
  #endif
  Cancel();
  }
//
// ----------------------------------------------------------
// CLockObserver::CLockObserver()
// C++ constructor
// ----------------------------------------------------------
// 
CValueObserver::CValueObserver(CAutolockAppUi* aAppUi) : CActive(0), iAppUi(aAppUi)
  {               
    #if defined(_DEBUG)     
    RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    #endif
    }
//
// ----------------------------------------------------------
// CLockObserver::ConstructL()
// Symbian OS default constructor
// ----------------------------------------------------------
// 
void CValueObserver::ConstructL()
    {
    // Add this active object to the scheduler.   
    //User::LeaveIfError(iProperty.Attach(KPSUidCtsyCallInformation, KCTsyCallState));    
    TInt ret = iProperty.Attach( KPSUidCtsyCallInformation, KCTsyCallState );
    #if defined(_DEBUG)
    RDebug::Printf( "%s %s (%u) Attach value=%d", __FILE__, __PRETTY_FUNCTION__, __LINE__, ret );
    #endif  
    
    RProperty::Get(KPSUidCtsyCallInformation, KCTsyCallState, iKCTsyCallState);
    #if defined(_DEBUG) 
    RDebug::Printf( "%s %s (%u) iKCTsyCallState=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iKCTsyCallState );
    #endif    

        
     if ( ret == KErrNone )  {   
        CActiveScheduler::Add(this);        
     }
     else  {   
         // If an error occurred, we can't get call state info which we really need
         // so leave
         User::Leave( ret );
     }        
      
    //CActiveScheduler::Add(this);  
    }
//
// ----------------------------------------------------------
// CValueObserver::RunL()
// 
// ----------------------------------------------------------
// 
void CValueObserver::RunL()
  {
    #if defined(_DEBUG)
    RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    #endif
    TInt atForeground = iAppUi->IsForeground();
    TInt value(EStartupUiPhaseUninitialized);
    RProperty::Get(KPSUidStartup, KPSStartupUiPhase, value);
    TInt callState;
    iProperty.Get( callState );
    #if defined(_DEBUG)
    RDebug::Printf( "%s %s (%u) iKCTsyCallState=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iKCTsyCallState );
    RDebug::Printf( "%s %s (%u) callState=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, callState );
    RDebug::Printf( "%s %s (%u) EPSCTsyCallStateNone=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, EPSCTsyCallStateNone );
    RDebug::Printf( "%s %s (%u) EPSCTsyCallStateUninitialized=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, EPSCTsyCallStateUninitialized );
    RDebug::Printf( "%s %s (%u) atForeground=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, atForeground );
    RDebug::Printf( "%s %s (%u) KPSStartupUiPhase value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, value );
    RDebug::Printf( "%s %s (%u) EStartupUiPhaseSystemWelcomeDone=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, EStartupUiPhaseSystemWelcomeDone );
    #endif

      if( value<EStartupUiPhaseSystemWelcomeDone )
          {
          	if(iKCTsyCallState==EPSCTsyCallStateUninitialized && callState==EPSCTsyCallStateNone)
          		{
			          #if defined(_DEBUG)
			          RDebug::Printf( "%s %s (%u) nothing for EPSCTsyCallStateUninitialized->EPSCTsyCallStateNone =%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iKCTsyCallState );
			          #endif
          			Start();
          			iKCTsyCallState = callState;
          			return;
          		}
          }
		iKCTsyCallState = callState;
    if ((callState == EPSCTsyCallStateNone || callState == EPSCTsyCallStateDisconnecting) && !atForeground)
        {
      if( value<EStartupUiPhaseSystemWelcomeDone )
          {
          #if defined(_DEBUG)
          RDebug::Printf( "%s %s (%u) re-start=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 1 );
          #endif
          iAppUi->BringAppToForegroundL();
          Start();
          }
      else
        {
            // app back to foreground
        TInt iAppUi_Locked = iAppUi->Locked();        
        #if defined(_DEBUG)
        TInt alocked = RProperty::Get(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, value);
        RDebug::Printf( "%s %s (%u) alocked=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, alocked );
        RDebug::Printf( "%s %s (%u) iAppUi_Locked=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iAppUi_Locked );
        #endif
        Start(); 
        if(iAppUi_Locked && callState != EPSCTsyCallStateDisconnecting)
          {
          #if defined(_DEBUG)
          RDebug::Printf( "%s %s (%u) BringAppToForegroundL=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 1 );
          #endif
          iAppUi->BringAppToForegroundL();
          }
       Start();          
      }
    }
    else
    {
        {
        if( value<EStartupUiPhaseSystemWelcomeDone )
          {
          Start();  
          #if defined(_DEBUG)
          RDebug::Printf( "%s %s (%u) 1=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 1 );
          #endif
          if( callState != EPSCTsyCallStateNone && callState != EPSCTsyCallStateUninitialized && callState != EPSCTsyCallStateDisconnecting && !atForeground)
            {
            #if defined(_DEBUG)
            RDebug::Printf( "%s %s (%u) 2=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 2 );
            #endif
            iAppUi->BringAppToForegroundL();
            }
          else if( (callState == EPSCTsyCallStateNone || callState == EPSCTsyCallStateUninitialized || callState == EPSCTsyCallStateDisconnecting) && atForeground)
            {
            #if defined(_DEBUG)
            RDebug::Printf( "%s %s (%u) calling BringAppToForegroundL=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 1 );
            #endif
            iAppUi->BringAppToForegroundL();
            #if defined(_DEBUG)
            RDebug::Printf( "%s %s (%u) calling SwitchToPreviousAppL=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 2 );
            #endif
            iAppUi->SwitchToPreviousAppL();
            #if defined(_DEBUG)
            RDebug::Printf( "%s %s (%u) done=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 3 );
            #endif
            }
          }
          else
              {
              TInt iAppUi_Locked = iAppUi->Locked();
              #if defined(_DEBUG)
              RDebug::Printf( "%s %s (%u) iAppUi_Locked=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iAppUi_Locked );
              #endif
              if (iAppUi_Locked && callState >= EPSCTsyCallStateConnected && !atForeground && value>=EStartupUiPhaseSystemWelcomeDone)
                {
                if ( AknLayoutUtils::PenEnabled() )
                    {
                    TApaTaskList apaTaskList(CCoeEnv::Static()->WsSession());
                    #define KPhoneAppUid TUid::Uid(0x100058B3)

                    TApaTask apaTask = apaTaskList.FindApp(KPhoneAppUid);
                    if (apaTask.Exists())
                      {
                      #if defined(_DEBUG)
                      RDebug::Printf( "%s %s (%u) Bring phone to background=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
                      #endif
                      // Bring phone to background
                      apaTask.SendToBackground();
                      }
                    }
                }
              }
        }
    Start();
    }
  
  }
//
// ----------------------------------------------------------
// CValueObserver::DoCancel()
// Cancels event listening
// ----------------------------------------------------------
// 
void CValueObserver::DoCancel()
    {
    #if defined(_DEBUG) 
    RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    #endif    
    iProperty.Cancel();
    //iProperty.Close();    
    }

// End of file
