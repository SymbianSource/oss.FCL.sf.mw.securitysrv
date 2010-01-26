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
* Description:  Obsererver for Set System Locked event  
*
*
*/


#include    <e32property.h>
#include	<PSVariables.h>
#include	<coreapplicationuisdomainpskeys.h>
#include <ctsydomainpskeys.h>
#include    <securityuisprivatepskeys.h>
#include "secuicodequerydialog.h"
#include "secui.hrh"
#include "SecUiLockObserver.h"

// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CSecUiLockObserver::NewL()
// Constructs a new entry with given values.
// ----------------------------------------------------------
//
CSecUiLockObserver* CSecUiLockObserver::NewL(CCodeQueryDialog* aDialog, TInt aType)
    {
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecUiLockObserver::NewL() BEGIN"));
    #endif
    CSecUiLockObserver* self = new (ELeave) CSecUiLockObserver(aDialog, aType);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecUiLockObserver::NewL() END"));
    #endif
    return self;
    }
//
// ----------------------------------------------------------
// CSecUiLockObserver::CSecUiLockObserver()
// Destructor
// ----------------------------------------------------------
//
CSecUiLockObserver::~CSecUiLockObserver()
    {
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecUiLockObserver::~CSecUiLockObserver"));
    #endif
    Cancel();
    }
//
// ----------------------------------------------------------
// CSecUiLockObserver::Start()
// Starts listening an event 
// ----------------------------------------------------------
//
TInt CSecUiLockObserver::Start()
    {
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecUiLockObserver::Start() BEGIN"));
    #endif
    if (IsActive())
    {
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecUiLockObserver::Start() In use!!!"));
    #endif
    return KErrInUse;
    }
        
    iStatus = KRequestPending;
    
    switch(iType)
        {
            case ESecUiDeviceLockObserver:
                #if defined(_DEBUG)
                RDebug::Print(_L("(SECUI)CSecUiLockObserver::Start() Device Lock Observer"));
                #endif
                iProperty.Attach(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus); 
                break;
            
            case ESecUiRequestStateObserver:
                    #if defined(_DEBUG)
                    RDebug::Print(_L("(SECUI)CSecUiLockObserver::Start() Request State Observer"));
                    #endif
                    iProperty.Attach(KPSUidSecurityUIs, KSecurityUIsQueryRequestCancel); 
                break;
            case ESecUiCallStateObserver:
                #if defined(_DEBUG)
                RDebug::Print(_L("(SECUI)CSecUiLockObserver::Start() Call State Observer"));
                #endif
                iProperty.Attach(KPSUidCtsyCallInformation, KCTsyCallState); 
                break;
            default:
                break;
        }
    
    iProperty.Subscribe(iStatus);
    SetActive();
    iSubscribedToEvent = ETrue;
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecUiLockObserver::Start() END"));
    #endif
    return KErrNone;
    }
//
// ----------------------------------------------------------
// CLockObserver::CLockObserver()
// C++ constructor
// ----------------------------------------------------------
// 
CSecUiLockObserver::CSecUiLockObserver(CCodeQueryDialog* aDialog, TInt aType) : CActive(0), iDialog(aDialog), iSubscribedToEvent(EFalse), iType(aType)
	{                            
    }
//
// ----------------------------------------------------------
// CSecUiLockObserver::ConstructL()
// Symbian OS default constructor
// ----------------------------------------------------------
// 
void CSecUiLockObserver::ConstructL()
    {
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecUiLockObserver::ConstructL() BEGIN"));
    #endif
    // Add this active object to the scheduler.
	CActiveScheduler::Add(this);
	// Begin obsering PubSub event  
	Start();
	#if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecUiLockObserver::ConstructL() END"));
    #endif	
    }
//
// ----------------------------------------------------------
// CSecUiLockObserver::RunL()
// Called by Active Scheduler
// ----------------------------------------------------------
// 
void CSecUiLockObserver::RunL()
	{
	#if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecUiLockObserver::RunL() BEGIN"));
    #endif
    
    switch(iType)
        {
            case ESecUiDeviceLockObserver:
                 TInt autolockState;
                 iProperty.Get( autolockState );
                 if (autolockState > EAutolockOff)
                    {
                    #if defined(_DEBUG)
                    RDebug::Print(_L("(SECUI)CSecUiLockObserver::RunL() TryCancelQueryL Device Lock"));
                    #endif
                	iDialog->TryCancelQueryL(ESecUiDeviceLocked);
                	iSubscribedToEvent = EFalse;
                    }
                break;
            
            case ESecUiRequestStateObserver:
                 TInt requestState;
                 iProperty.Get( requestState );
                 if(requestState == ESecurityUIsQueryRequestCanceled)
                    {
                     #if defined(_DEBUG)
                    RDebug::Print(_L("(SECUI)CSecUiLockObserver::RunL() TryCancelQueryL Req Canceled"));
                    #endif
                	iDialog->TryCancelQueryL(EEikBidCancel);
                	iSubscribedToEvent = EFalse;   
                    }
                break;
            case ESecUiCallStateObserver:
                TInt callState;
                iProperty.Get( callState );
                if(callState > EPSCTsyCallStateNone)
                    {
                     #if defined(_DEBUG)
                    RDebug::Print(_L("(SECUI)CSecUiLockObserver::RunL() TryCancelQueryL Active Call"));
                    #endif
                	iDialog->TryCancelQueryL(EEikBidCancel);
                	iSubscribedToEvent = EFalse;   
                    }
                break;
            default:
                break;
        }

   
	#if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecUiLockObserver::RunL() END"));
    #endif
	}
//
// ----------------------------------------------------------
// CSecUiLockObserver::DoCancel()
// Cancels event listening
// ----------------------------------------------------------
// 
void CSecUiLockObserver::DoCancel()
    {
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecUiLockObserver::DoCancel() BEGIN"));
    #endif
    if(iSubscribedToEvent)
    	iProperty.Cancel();
    iStatus = KErrNone;
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecUiLockObserver::DoCancel() END"));
    #endif
    }
//
// ----------------------------------------------------------
// CSecUiLockObserver::StopObserver()
// Cancels event listening
// ----------------------------------------------------------
//     
void CSecUiLockObserver::StopObserver()
	{
	#if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecUiLockObserver::StopObserver() BEGIN"));
    #endif
    Cancel();
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecUiLockObserver::StopObserver() END"));
    #endif
    }
//
// ----------------------------------------------------------
// CSecUiLockObserver::StartObserver()
// Start event listening
// ----------------------------------------------------------
//     
void CSecUiLockObserver::StartObserver()
	{
	#if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecUiLockObserver::StartObserver() BEGIN"));
    #endif
    Start();
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecUiLockObserver::StartObserver() END"));
    #endif
    }
//
// ----------------------------------------------------------
// CSecUiLockObserver::SetAddress()
// Start event listening
// ----------------------------------------------------------
//  
void CSecUiLockObserver::SetAddress(CCodeQueryDialog* aDialog)
{
	#if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecUiLockObserver::SetAddress() BEGIN"));
    #endif
	iDialog = aDialog;
	#if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecUiLockObserver::SetAddress() END"));
    #endif
}
// End of file
