/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia	Corporation - initial contribution.
*
* Contributors:
*
* Description: 
*		Obsererver for Set System Locked event	
*
*
*/

#include		<bldvariant.hrh>
#include		<e32property.h>
#include	<PSVariables.h>
#include		<coreapplicationuisdomainpskeys.h>

#include	"AutoLockLockObserverPS.h"
#include	"AutolockAppUiPS.h"
#include	"AutoLockModelPS.h"


// ================= MEMBER	FUNCTIONS	=======================
//
// ----------------------------------------------------------
// CLockObserver::NewL()
// Constructs	a	new	entry	with given values.
// ----------------------------------------------------------
//
CLockObserver* CLockObserver::NewL(CAutolockAppUi* aAppUi)
		{
		CLockObserver* self	=	new	(ELeave) CLockObserver(aAppUi);
		CleanupStack::PushL(self);
		self->ConstructL();
		CleanupStack::Pop();
		return self;
		}
//
// ----------------------------------------------------------
// CLockObserver::CLockObserver()
// Destructor
// ----------------------------------------------------------
//
CLockObserver::~CLockObserver()
		{
		Cancel();
		iProperty.Close();
		}
//
// ----------------------------------------------------------
// CNoSimCard::Start()
// Starts	listening	KUidAutolockStatus event 
// ----------------------------------------------------------
//
TInt CLockObserver::Start()
		{
		if (IsActive())
				return KErrInUse;
		iStatus	=	KRequestPending;
		iProperty.Attach(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus);		 
		iProperty.Subscribe(iStatus);
		SetActive();
		return KErrNone;
		}
//
// ----------------------------------------------------------
// CLockObserver::CLockObserver()
// C++ constructor
// ----------------------------------------------------------
// 
CLockObserver::CLockObserver(CAutolockAppUi* aAppUi) : CActive(0), iAppUi(aAppUi)
	{														 
		}
//
// ----------------------------------------------------------
// CLockObserver::ConstructL()
// Symbian OS	default	constructor
// ----------------------------------------------------------
// 
void CLockObserver::ConstructL()
		{
	    
	#if	defined(_DEBUG)	
    RDebug::Print(_L("(AUTOLOCK)CLockObserver::ConstructL()"));
    #endif
        
		// Add this	active object	to the scheduler.
	CActiveScheduler::Add(this);
	    
	TInt ret;
	_LIT_SECURITY_POLICY_PASS(KReadPolicy);	
	_LIT_SECURITY_POLICY_C1(KWritePolicy,	ECapabilityWriteDeviceData);
	
	ret	=	RProperty::Define(KPSUidCoreApplicationUIs,	KCoreAppUIsAutolockStatus, RProperty::EInt,	KReadPolicy, KWritePolicy);
		if(ret !=	KErrAlreadyExists)
				User::LeaveIfError(ret);
				    
	// Begin obsering	PubSub event	
	Start();	
		}
//
// ----------------------------------------------------------
// CLockObserver::RunL()
// Called	when device	(autolock) is	activated	from menu.
// ----------------------------------------------------------
// 
void CLockObserver::RunL()
	{
	// set ui	locked.
		TInt autolockState;
		iProperty.Get( autolockState );
		if (autolockState	>	EAutolockOff)
				{
		iAppUi->Model()->LockSystemL(autolockState);
		}
	// Continue	observing	PubSub event	
	Start();	
	}
//
// ----------------------------------------------------------
// CLockObserver::DoCancel()
// Cancels event listening
// ----------------------------------------------------------
// 
void CLockObserver::DoCancel()
		{
		iProperty.Cancel();
		}
// End of	file
