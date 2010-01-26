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


#include <centralrepository.h> 
#include <settingsinternalcrkeys.h>
#include "AutoLockModelPS.h"
#include "AutolockAppUiPS.h"
#include "AutoLockCenRepI.h"
 


// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CAutolockCenRepI::NewLC()
// Two-phased constructor.
// ----------------------------------------------------------
//
CAutolockCenRepI* CAutolockCenRepI::NewLC(CAutolockAppUi* aAppUi)
	{
	CAutolockCenRepI* self = new (ELeave) CAutolockCenRepI(aAppUi);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

//
// ----------------------------------------------------------
// CAutolockCenRepI::NewL()
// Two-phased constructor.
// ----------------------------------------------------------
//
CAutolockCenRepI* CAutolockCenRepI::NewL(CAutolockAppUi* aAppUi)
	{
	CAutolockCenRepI* self = NewLC(aAppUi);
	CleanupStack::Pop(); //self
	return self;
	}

//
// ----------------------------------------------------------
// CAutolockCenRepI::~CAutolockCenRepI()
// Destructor
// ----------------------------------------------------------
//
CAutolockCenRepI::~CAutolockCenRepI()
	{
	if(iNotifyHandler)
	{
		iNotifyHandler->StopListening();
		delete iNotifyHandler;	
	}
    if(iSession)
		delete iSession;
	}
//
// ----------------------------------------------------------
// CAutolockCenRepI::CAutolockCenRepI()
// C++ default constructor
// ----------------------------------------------------------
//	
CAutolockCenRepI::CAutolockCenRepI(CAutolockAppUi* aAppUi):iAppUi(aAppUi)
	{
	}

//
// ----------------------------------------------------------
// CAutolockCenRepI::ConstructL()
// Symbian OS default constructor
// ----------------------------------------------------------
//	
void CAutolockCenRepI::ConstructL()
	{
	// init cenrep connection	
	iSession = CRepository::NewL(KCRUidSecuritySettings);

    iNotifyHandler = CCenRepNotifyHandler::NewL(*this, *iSession, CCenRepNotifyHandler::EIntKey, KSettingsAutoLockTime);
    iNotifyHandler->StartListeningL();
	}

//
// ----------------------------------------------------------
// CAutolockCenRepI::HandleNotifyInt()
// Handles autolock period changes. Called by CenRep.
// ----------------------------------------------------------
//	
void CAutolockCenRepI::HandleNotifyInt(TUint32 aId, TInt /*aNewValue*/)
	{
    if(aId == KSettingsAutoLockTime)
    {
	    ResetInactivityTimeout();
    }
	return;
	}

void CAutolockCenRepI::HandleNotifyError(TUint32 /*aId*/, TInt /*error*/, CCenRepNotifyHandler* /*aHandler*/)
    {
    return;
    }

void CAutolockCenRepI::HandleNotifyGeneric(TUint32 aId)
    {
        if ( aId == NCentralRepositoryConstants::KInvalidNotificationId )
        {//Repository wide reset caused generic notification
            ResetInactivityTimeout();
        }
    return;
    }

//
// ----------------------------------------------------------
// CAutolockCenRepI::ResetInactivityTimeoutL()
// Resets autolock timer
// ----------------------------------------------------------
//	
void CAutolockCenRepI::ResetInactivityTimeout()
	{
	iAppUi->Model()->ResetInactivityTimeout();
	}

//
// ----------------------------------------------------------
// CAutolockCenRepI::Timeout()
// Returns currents autolock period (in seconds)
// ----------------------------------------------------------
//	
TInt CAutolockCenRepI::Timeout()
	{
    TInt period = 0;
    iSession->Get(KSettingsAutoLockTime, period);
	return period * 60;
	}
//
// ----------------------------------------------------------
// CAutolockCenRepI::SetLocked ()
// Sets lock on/off in CenRep
// ----------------------------------------------------------
//	
void CAutolockCenRepI::SetLockedL(TBool aLockValue)
	{
	TInt lockValue = 0;
	if (aLockValue)
		{
		lockValue = 1;
		}
    CRepository* repository = CRepository::NewL(KCRUidSecuritySettings);
    repository->Set(KSettingsAutolockStatus, lockValue);
    delete repository;
	}
// END OF FILE
