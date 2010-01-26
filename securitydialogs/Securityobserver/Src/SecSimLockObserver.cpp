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
* Description:  Observer for Sim Locking events.
*               Shows "Sim restriction on" note if necessary.
*
*
*/


#include    "SecSimLockObserver.h"

#ifndef RD_STARTUP_CHANGE
#include    <PSVariables.h>   // Property values
#else //RD_STARTUP_CHANGE
#include    <StartupDomainPSKeys.h>
#endif //RD_STARTUP_CHANGE 

#include    "SecObsNotify.h"
#include    "SecurityObserver.hrh"

// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CSimLockObserver::NewL()
// Constructs a new entry with given values.
// ----------------------------------------------------------
//
CSimLockObserver* CSimLockObserver::NewL(CSecObsNotify* aNotifierController)
    {
    CSimLockObserver* self = new (ELeave) CSimLockObserver(aNotifierController);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }
//
// ----------------------------------------------------------
// CSimLockObserver::CSimLockObserver()
// Destructor
// ----------------------------------------------------------
//
CSimLockObserver::~CSimLockObserver()
    {
    Cancel();
    }
//
// ----------------------------------------------------------
// CSimLockObserver::Start()
// Starts listening event
// ----------------------------------------------------------
//
TInt CSimLockObserver::Start()
    {
    if (IsActive())
        return KErrInUse;
    iStatus = KRequestPending;
#ifndef RD_STARTUP_CHANGE
    iProperty.Attach(KUidSystemCategory, KPSUidSimLockStatusValue); 
#else //RD_STARTUP_CHANGE
    iProperty.Attach(KPSUidStartup, KStartupSimLockStatus); 
#endif //RD_STARTUP_CHANGE   
    iProperty.Subscribe(iStatus);
    SetActive();
    return KErrNone;
    }

//
// ----------------------------------------------------------
// CSimLockObserver::Stopt()
// Stops listening event
// ----------------------------------------------------------
//
void CSimLockObserver::Stop()
    {
    Cancel();
    }
//
// ----------------------------------------------------------
// CCSimLockObserver::CSimLockObserver()
// C++ constructor
// ----------------------------------------------------------
//
CSimLockObserver::CSimLockObserver(CSecObsNotify* aNotifierController) :
                                    CActive(0)
                                    ,iNotifierController(aNotifierController)
    {

    }
//
// ----------------------------------------------------------
// CSimLockObserver::ConstructL()
// Symbian OS default constructor
// ----------------------------------------------------------
//
void CSimLockObserver::ConstructL()
    {
    // Add this active object to the scheduler.
    CActiveScheduler::Add(this);
    }
//
// ----------------------------------------------------------
// CPhoneObserver::RunL()
//
// ----------------------------------------------------------
//
void CSimLockObserver::RunL()
    {
    // Show "Sim restriction on" note
    TInt simLockStatus;
#ifndef RD_STARTUP_CHANGE
    iProperty.Get(KUidSystemCategory, KPSUidSimLockStatusValue, simLockStatus);
    if (simLockStatus == EPSSimLockRestrictionOn)
#else //RD_STARTUP_CHANGE
    iProperty.Get(KPSUidStartup, KStartupSimLockStatus, simLockStatus);
    if (simLockStatus == ESimLockRestrictionOn)
#endif //RD_STARTUP_CHANGE   
        {
        iNotifierController->StartNotifier(ESecuritySimLockRestrictionOn);
        }
    else
        {
        // Continue observing system agent event
        Start();
        }
    }
//
// ----------------------------------------------------------
// CSimLockObserver::DoCancel()
// Cancels event listening
// ----------------------------------------------------------
//
void CSimLockObserver::DoCancel()
    {
    iProperty.Cancel();
    }

// End of file
