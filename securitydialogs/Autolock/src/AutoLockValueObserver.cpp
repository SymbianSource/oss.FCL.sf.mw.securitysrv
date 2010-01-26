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
    TInt callState;
    iProperty.Get( callState );
    if (callState == EPSCTsyCallStateNone && !iAppUi->IsForeground())
        {
		// app back to foreground
		iAppUi->BringAppToForegroundL();
		}
    else
		{
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
