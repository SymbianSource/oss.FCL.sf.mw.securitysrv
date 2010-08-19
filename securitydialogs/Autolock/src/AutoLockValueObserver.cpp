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
*		Observer for phone events. Used to deactive/active the side-key
*
*
*/


#include    <e32property.h>
#include	<PSVariables.h>
#include <ctsydomainpskeys.h>
#include	"AutolockAppUiPS.h"
#include	"AutoLockValueObserverPS.h"
#include <coreapplicationuisdomainpskeys.h>
#include <startupdomainpskeys.h>

// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CValueObserver::NewL()
// Constructs a new entry with given values.
// ----------------------------------------------------------
//
CValueObserver* CValueObserver::NewL(CAutolockAppUi* aAppUi)
    {
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
    iProperty.Attach(KPSUidCtsyCallInformation, KCTsyCallState); 
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
    #if defined(_DEBUG)
    RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
		#endif
	CActiveScheduler::Add(this);	
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
		RDebug::Printf( "%s %s (%u) callState=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, callState );
		RDebug::Printf( "%s %s (%u) EPSCTsyCallStateNone=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, EPSCTsyCallStateNone );
		RDebug::Printf( "%s %s (%u) EPSCTsyCallStateUninitialized=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, EPSCTsyCallStateUninitialized );
		RDebug::Printf( "%s %s (%u) atForeground=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, atForeground );
		RDebug::Printf( "%s %s (%u) KPSStartupUiPhase value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, value );
		RDebug::Printf( "%s %s (%u) EStartupUiPhaseSystemWelcomeDone=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, EStartupUiPhaseSystemWelcomeDone );
		#endif


    if (callState == EPSCTsyCallStateNone && !atForeground)
        {
			if( value<EStartupUiPhaseSystemWelcomeDone )
					{
					#if defined(_DEBUG)
					RDebug::Printf( "%s %s (%u) re-start=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 1 );
					#endif
					Start();
					}
			else
				{
						// app back to foreground
				TInt iAppUi_Locked = iAppUi->Locked();
				TInt alocked = RProperty::Get(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, value);
				#if defined(_DEBUG)
				RDebug::Printf( "%s %s (%u) alocked=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, alocked );
				RDebug::Printf( "%s %s (%u) iAppUi_Locked=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iAppUi_Locked );
				#endif
				if(iAppUi_Locked)
					{
					#if defined(_DEBUG)
					RDebug::Printf( "%s %s (%u) BringAppToForegroundL=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 1 );
					#endif
					iAppUi->BringAppToForegroundL();
					}
			}
		}
    else
		{
				{
				if( value<EStartupUiPhaseSystemWelcomeDone )
					{
					#if defined(_DEBUG)
					RDebug::Printf( "%s %s (%u) 1=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 1 );
					#endif
					if( callState != EPSCTsyCallStateNone && callState != EPSCTsyCallStateUninitialized && !atForeground)
						{
						#if defined(_DEBUG)
						RDebug::Printf( "%s %s (%u) 2=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 2 );
						#endif
						iAppUi->BringAppToForegroundL();
						}
					else if( (callState == EPSCTsyCallStateNone || callState == EPSCTsyCallStateUninitialized) && atForeground)
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
    iProperty.Cancel();
    }

// End of file
