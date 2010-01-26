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
* Description:  Implementation of Unlocking event observer
*
*/

#include    <PSVariables.h>
#include    <coreapplicationuisdomainpskeys.h>
#include    "RLLockObserver.h"	
#include    "RemoteLockTrace.h"


// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CRLLockObserver::NewL()
// Constructs a new entry with given values.
// ----------------------------------------------------------
//
CRLLockObserver* CRLLockObserver::NewL( MRLLockObserverHandler* aHandler )
    {
    CRLLockObserver* self = new (ELeave) CRLLockObserver( aHandler );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }
//
// ----------------------------------------------------------
// CRLLockObserver::CRLLockObserver()
// Destructor
// ----------------------------------------------------------
//
CRLLockObserver::~CRLLockObserver()
    {
    Cancel();
    }
//
// ----------------------------------------------------------
// CRLLockObserver::Start()
// Starts listening KCoreAppUIsAutolockStatus/
// KPSUidAutolockStatusValue event 
// ----------------------------------------------------------
//
TInt CRLLockObserver::Start()
    {
    RL_TRACE_PRINT(" [ rl.exe ] CLockObserver::Start() "); 
    
    if (IsActive())
        {
        return KErrInUse;
        }   
        
    iStatus = KRequestPending;
    
    #ifndef RD_STARTUP_CHANGE   
    iProperty.Attach( KUidSystemCategory, KPSUidAutolockStatusValue ); 
    #else //RD_STARTUP_CHANGE
    iProperty.Attach( KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus ); 
    #endif //RD_STARTUP_CHANGE    
    iProperty.Subscribe(iStatus);
    
    SetActive();
    return KErrNone;
    }
//
// ----------------------------------------------------------
// CRLLockObserver::CRLLockObserver()
// C++ constructor
// ----------------------------------------------------------
// 
CRLLockObserver::CRLLockObserver( MRLLockObserverHandler* aHandler ) 
    : CActive(0), iHandler( aHandler )
    {                            
    }
//
// ----------------------------------------------------------
// CRLLockObserver::ConstructL()
// ----------------------------------------------------------
// 
void CRLLockObserver::ConstructL()
    {
    RL_TRACE_PRINT(" [ rl.exe ] CLockObserver::ConstructL() ");
    // Add this active object to the scheduler.
    CActiveScheduler::Add(this);
    //Start listenning
    Start();
    }
//
// ----------------------------------------------------------
// CRLLockObserver::RunL()
// Called when device (autolock) is activated from menu.
// ----------------------------------------------------------
// 
void CRLLockObserver::RunL()
    {
    RL_TRACE_PRINT(" [ rl.exe ] CLockObserver::RunL() ");

    TInt autolockState;
    iProperty.Get( autolockState );
    
    #ifndef RD_STARTUP_CHANGE
    if (autolockState == EPSAutolockOff )
    #else //RD_STARTUP_CHANGE
    if (autolockState == EAutolockOff )
    #endif //RD_STARTUP_CHANGE
        {
        RL_TRACE_PRINT(" [ rl.exe ] CLockObserver::RunL remove Memory card Password ");
        iHandler->HandleUnlockEvent();
        } 
    Start();
    }
//
// ----------------------------------------------------------
// CRLLockObserver::DoCancel()
// Cancels event listening
// ----------------------------------------------------------
// 
void CRLLockObserver::DoCancel()
    {
    RL_TRACE_PRINT(" [ rl.exe ] CLockObserver::DoCancel() ");
    iProperty.Cancel();
    }
    
// End of file

