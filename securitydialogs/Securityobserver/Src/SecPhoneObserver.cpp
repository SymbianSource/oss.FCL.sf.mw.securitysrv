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
* Description:  Observer for phone events. Used to re-active
*               code query after emergency call
*
*
*/


#include    "SecPhoneObserver.h"

#ifndef RD_STARTUP_CHANGE
#include    <SysStartup.h>
#endif //RD_STARTUP_CHANGE

#include    <PSVariables.h>   // Property values
#include    <TelephonyInternalPSKeys.h>
#include    "SecObsNotify.h"
#include    "SecurityObserver.h"
#include    "SecurityObserver.hrh"

// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CPhoneObserver::NewL()
// Constructs a new entry with given values.
// ----------------------------------------------------------
//
CPhoneObserver* CPhoneObserver::NewL(CSecObsNotify* aNotifierController,CSecurityObserver* aObserver)
    {
    CPhoneObserver* self = new (ELeave) CPhoneObserver(aNotifierController,aObserver);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }
//
// ----------------------------------------------------------
// CPhoneObserver::CPhoneObserver()
// Destructor
// ----------------------------------------------------------
//
CPhoneObserver::~CPhoneObserver()
    {
    Cancel();
    }
//
// ----------------------------------------------------------
// CPhoneObserver::Start()
// Starts listening KUidCurrentCall event
// ----------------------------------------------------------
//
TInt CPhoneObserver::Start(TSecurityNotifier aNotifier)
    {
    if (IsActive())
        return KErrInUse;
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECURITYOBSERVER) CPhoneObserver::Start"));
    #endif
    iNotifier = aNotifier;
    iStatus = KRequestPending;
    iProperty.Attach(KPSUidTelephonyCallHandling, KTelephonyCallState); 
    iProperty.Subscribe(iStatus);
    SetActive();
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECURITYOBSERVER) CPhoneObserver::Start Observer active"));
    #endif
    return KErrNone;
    }

//
// ----------------------------------------------------------
// CPhoneObserver::Start()
// Stops listening KUidCurrentCall event
// ----------------------------------------------------------
//
void CPhoneObserver::Stop()
    {
    Cancel();
    }
//
// ----------------------------------------------------------
// CPhoneObserver::CPhoneObserver()
// C++ constructor
// ----------------------------------------------------------
//
CPhoneObserver::CPhoneObserver(CSecObsNotify* aNotifierController,CSecurityObserver* aObserver) :
                                                                    CActive(0)
                                                                    ,iNotifierController(aNotifierController)
                                                                    ,iObserver(aObserver)
    {
    }
//
// ----------------------------------------------------------
// CPhoneObserver::ConstructL()
// Symbian OS default constructor
// ----------------------------------------------------------
//
void CPhoneObserver::ConstructL()
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
void CPhoneObserver::RunL()
    {
    TInt callState;
    iProperty.Get(KPSUidTelephonyCallHandling, KTelephonyCallState, callState);
    if (callState == EPSTelephonyCallStateNone)
        {
        
        switch (iNotifier)
            {
#if defined(__PROTOCOL_WCDMA) || defined(__UPIN)
#ifndef RD_STARTUP_CHANGE
            case ESecurityNotifierUPin:
                #if defined(_DEBUG)
                RDebug::Print(_L("(SECURITYOBSERVER) CPhoneObserver::RunL() UPIN"));
                #endif
                if (iObserver->IsUPinRequired() && !iObserver->IsUPukRequired())
                    iNotifierController->StartNotifier(iNotifier);
                break;
#endif //RD_STARTUP_CHANGE
            case ESecurityNotifierUPuk:
                #if defined(_DEBUG)
                RDebug::Print(_L("(SECURITYOBSERVER) CPhoneObserver::RunL() UPUK"));
                #endif
                if (iObserver->IsUPukRequired())
                    iNotifierController->StartNotifier(iNotifier);
                break;
#endif //__PROTOCOL_WCDMA

#ifndef RD_STARTUP_CHANGE
            case ESecurityNotifierPin1:
                #if defined(_DEBUG)
                RDebug::Print(_L("(SECURITYOBSERVER) CPhoneObserver::RunL() PIN1"));
                #endif
                if (iObserver->IsPin1Required() && !iObserver->IsPuk1Required())
                    iNotifierController->StartNotifier(iNotifier);
                break;
#endif //RD_STARTUP_CHANGE
            case ESecurityNotifierPuk1:
                #if defined(_DEBUG)
                RDebug::Print(_L("(SECURITYOBSERVER) CPhoneObserver::RunL() PUK1"));
                #endif
                if (iObserver->IsPuk1Required())
                    iNotifierController->StartNotifier(iNotifier);
                break;
#ifndef RD_STARTUP_CHANGE
            case ESecurityNotifierSecurityCode:
                #if defined(_DEBUG)
                RDebug::Print(_L("(SECURITYOBSERVER) CPhoneObserver::RunL() Sec Code"));
                #endif
                if (iObserver->IsPassPhraseRequired() && SysStartup::State() != ESWStateNormal)
                    iNotifierController->StartNotifier(iNotifier);
                break;
#endif //RD_STARTUP_CHANGE
            default:
                break;
            }
        }
    else
        {
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECURITYOBSERVER) CPhoneObserver::RunL() Call Active"));
        #endif
        // Continue observing system agent event
        Start(iNotifier);
        }
    }
//
// ----------------------------------------------------------
// CValueObserver::DoCancel()
// Cancels event listening
// ----------------------------------------------------------
//
void CPhoneObserver::DoCancel()
    {
    iProperty.Cancel();
    }

// End of file
