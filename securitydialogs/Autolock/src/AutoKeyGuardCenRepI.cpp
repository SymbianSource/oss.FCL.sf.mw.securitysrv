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
#include "AutoKeyguardObserver.h"
#include "AutoKeyguardCenRepI.h"
 


// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CAutoKeyguardCenRepI::NewLC()
// Two-phased constructor.
// ----------------------------------------------------------
//
CAutoKeyguardCenRepI* CAutoKeyguardCenRepI::NewLC(CAutoKeyguardObserver* aObserver)
	{
	CAutoKeyguardCenRepI* self = new (ELeave) CAutoKeyguardCenRepI(aObserver);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

//
// ----------------------------------------------------------
// CAutoKeyguardCenRepI::NewL()
// Two-phased constructor.
// ----------------------------------------------------------
//
CAutoKeyguardCenRepI* CAutoKeyguardCenRepI::NewL(CAutoKeyguardObserver* aObserver)
	{
	CAutoKeyguardCenRepI* self = NewLC(aObserver);
	CleanupStack::Pop(); //self
	return self;
	}

//
// ----------------------------------------------------------
// CAutoKeyguardCenRepI::~CAutoKeyguardCenRepI()
// Destructor
// ----------------------------------------------------------
//
CAutoKeyguardCenRepI::~CAutoKeyguardCenRepI()
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
// CAutoKeyguardCenRepI::CAutoKeyguardCenRepI()
// C++ default constructor
// ----------------------------------------------------------
//	
CAutoKeyguardCenRepI::CAutoKeyguardCenRepI(CAutoKeyguardObserver* aObserver):iObserver(aObserver)
	{
	}

//
// ----------------------------------------------------------
// CAutoKeyguardCenRepI::ConstructL()
// Symbian OS default constructor
// ----------------------------------------------------------
//	
void CAutoKeyguardCenRepI::ConstructL()
	{
	// init cenrep connection	
	iSession = CRepository::NewL(KCRUidSecuritySettings);

    iNotifyHandler = CCenRepNotifyHandler::NewL(*this, *iSession, CCenRepNotifyHandler::EIntKey, KSettingsAutomaticKeyguardTime);
    iNotifyHandler->StartListeningL();
	}

//
// ----------------------------------------------------------
// CAutoKeyguardCenRepI::HandleNotifyInt()
// Handles autoKeyguard period changes. Called by CenRep.
// ----------------------------------------------------------
//	
void CAutoKeyguardCenRepI::HandleNotifyInt(TUint32 aId, TInt /*aNewValue*/)
	{
    if(aId == KSettingsAutomaticKeyguardTime)
    {
    #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutoKeyguardCenRepI::HandleNotifyInt() Reset timeout"));
    #endif
	    ResetInactivityTimeout();
    }
	return;
	}

void CAutoKeyguardCenRepI::HandleNotifyError(TUint32 /*aId*/, TInt /*error*/, CCenRepNotifyHandler* /*aHandler*/)
    {
    return;
    }

void CAutoKeyguardCenRepI::HandleNotifyGeneric(TUint32 aId)
    {
        if ( aId == NCentralRepositoryConstants::KInvalidNotificationId )
        {//Repository wide reset caused generic notification
        #if defined(_DEBUG)
    	RDebug::Print(_L("(AUTOLOCK)CAutoKeyguardCenRepI::HandleNotifyGeneric() Reset timeout"));
    	#endif
            ResetInactivityTimeout();
        }
    return;
    }

//
// ----------------------------------------------------------
// CAutoKeyguardCenRepI::ResetInactivityTimeoutL()
// Resets autoKeyguard timer
// ----------------------------------------------------------
//	
void CAutoKeyguardCenRepI::ResetInactivityTimeout()
	{
	iObserver->ResetInactivityTimeout();
	}

//
// ----------------------------------------------------------
// CAutoKeyguardCenRepI::Timeout()
// Returns currents autoKeyguard period (in seconds)
// ----------------------------------------------------------
//	
TInt CAutoKeyguardCenRepI::Timeout()
	{
    TInt period = 0;
    iSession->Get(KSettingsAutomaticKeyguardTime, period);
	return period;
	}

// END OF FILE
