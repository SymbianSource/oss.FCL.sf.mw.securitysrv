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
*       CSecObsNotify - starts and cancels notifier plug-ins.
*       Handles PIN/PIN2/PUK/PUK2/... notifiers
*
*/


//  Include Files
#include    "SecObsNotify.h"
#include    "SecurityObserver.h"
#include    <SecurityNotifier.h>

#ifndef RD_STARTUP_CHANGE
#include    <SysStartup.h>
#endif //RD_STARTUP_CHANGE

#include    "SecurityObserver.hrh"

#ifdef __SAP_TERMINAL_CONTROL_FW
#include <SCPClient.h>
#endif //__SAP_TERMINAL_CONTROL_FW

//===========================================================
//  MEMBER FUNCTIONS
//===========================================================
//
// ----------------------------------------------------------
// CSecObsNotify::NewL()
// Constructs a new entry with given values.
// ----------------------------------------------------------
//
CSecObsNotify* CSecObsNotify::NewL(CSecurityObserver* aObserver)
    {
    CSecObsNotify* self = new(ELeave) CSecObsNotify(aObserver);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }
//
// ----------------------------------------------------------
// CSecObsNotify::~CSecObsNotify
// C++ destructor.
// ----------------------------------------------------------
//
CSecObsNotify::~CSecObsNotify()
    {
    Cancel();
    delete iPhoneObserver;
    }
//
// ----------------------------------------------------------
// CSecObsNotify::CSecObsNotify
// C++ constructor
// ----------------------------------------------------------
//
CSecObsNotify::CSecObsNotify(CSecurityObserver* aObserver) : CActive(0),
                                 iObserver(aObserver),
                                 iActiveNotifier(ESecurityNotifierNone),
                                 iPendingNotifier(ESecurityNotifierNone)
    {
    }
//
// ----------------------------------------------------------
// CSecObsNotify::ConstructL
// Adds this active object to the scheduler.
// ----------------------------------------------------------
//
void CSecObsNotify::ConstructL()
    {
    // Add this active object to the scheduler.
    CActiveScheduler::Add(this);
    iPhoneObserver = CPhoneObserver::NewL(this,iObserver);
    }

//
// ----------------------------------------------------------
// CSecObsNotify::RunL
//
// ----------------------------------------------------------
//
void CSecObsNotify::RunL()
    {
    #if defined(_DEBUG)
	RDebug::Print(_L("CSecObsNotify::RunL()"));
	#endif
#ifndef RD_STARTUP_CHANGE

#if defined(__PROTOCOL_WCDMA) || defined(__UPIN) 
    if (iActiveNotifier == ESecurityNotifierUPin)
        {
        if (iObserver->IsUPinRequired() && !iObserver->IsUPukRequired() && SysStartup::State() != ESWStateNormal)
            {
            iPhoneObserver->Stop();
            iPhoneObserver->Start(iActiveNotifier);
            }
        }
  
#endif //__PROTOCOL_WCDMA || __UPIN
    if (iActiveNotifier == ESecurityNotifierPin1)
        {
        if (iObserver->IsPin1Required() && !iObserver->IsPuk1Required() && SysStartup::State() != ESWStateNormal)
            {
            iPhoneObserver->Stop();
            iPhoneObserver->Start(iActiveNotifier);
            }
        }
#endif // RD_STARTUP_CHANGE    

    if (iActiveNotifier == ESecurityNotifierPuk1)
        {
        if (iObserver->IsPuk1Required())
            {
            iPhoneObserver->Stop();
            iPhoneObserver->Start(iActiveNotifier);
            }
        }
    #if defined(__PROTOCOL_WCDMA) || defined(__UPIN)
    if (iActiveNotifier == ESecurityNotifierUPuk)
        {
        if (iObserver->IsUPukRequired())
            {
            iPhoneObserver->Stop();
            iPhoneObserver->Start(iActiveNotifier);
            }
        }
    #endif //__PROTOCOL_WCDMA || __UPIN

#ifndef RD_STARTUP_CHANGE

    if (iActiveNotifier == ESecurityNotifierSecurityCode)
        {
        #if defined(_DEBUG)
	    RDebug::Print(_L("CSecObsNotify::RunL(): ESecurityNotifierSecurityCode"));
	    #endif
        if (iObserver->IsPassPhraseRequired() && SysStartup::State() != ESWStateNormal)
            {
            #if defined(_DEBUG)
	        RDebug::Print(_L("CSecObsNotify::RunL(): ESecurityNotifierSecurityCode: IF"));
	        #endif
            iPhoneObserver->Stop();
            iPhoneObserver->Start(iActiveNotifier);
            #if defined(_DEBUG)
	        RDebug::Print(_L("CSecObsNotify::RunL(): ESecurityNotifierSecurityCode: IF: END"));
	        #endif
            }
        }
#endif // RD_STARTUP_CHANGE 

    CancelActiveNotifier();

    if (iPendingNotifier != ESecurityNotifierNone)
        {
        #if defined(_DEBUG)
	    RDebug::Print(_L("CSecObsNotify::RunL(): start pending notifier"));
	    #endif
        // previous notifier cancelled and we have pending notifier
        StartNotifier(iPendingNotifier);

#ifndef RD_STARTUP_CHANGE
        #if defined(__PROTOCOL_WCDMA) || defined(__UPIN)
        if(((iPendingNotifier == ESecurityNotifierPuk1) || (iPendingNotifier == ESecurityNotifierUPuk)) && iObserver->IsPassPhraseRequired())
        #else
        if((iPendingNotifier == ESecurityNotifierPuk1) && iObserver->IsPassPhraseRequired())
        #endif //__PROTOCOL_WCDMA || __UPIN
            { 
            //PUK code query has been written in top of Sec code query in boot;
            //it arrived before PIN code event was completed and Sec code event
            //could be moved from pending to active. Putting sec code event
            //back to pending event.
            iPendingNotifier=ESecurityNotifierSecurityCode;
            }
        else
            {
            iPendingNotifier=ESecurityNotifierNone;
            }
#else // RD_STARTUP_CHANGE
        iPendingNotifier=ESecurityNotifierNone;
#endif // RD_STARTUP_CHANGE     
        }
  
    }
//
// ----------------------------------------------------------
// CSecObsNotify::CancelActiveNotifier
// Cancels notifier
// ----------------------------------------------------------
//
void CSecObsNotify::CancelActiveNotifier()
    {
    if (iActiveNotifier != ESecurityNotifierNone)
        {
        iNotifier.CancelNotifier(KSecurityNotifierUid);
        iNotifier.Close();
        iActiveNotifier=ESecurityNotifierNone;
        }
    }
//
// ----------------------------------------------------------
// CSecObsNotify::StartNotifier
// Starts given notifier asyncronously
// ----------------------------------------------------------
//
void CSecObsNotify::StartNotifier( TSecurityNotifier aNotifierToStart )
    {
    #if defined(_DEBUG)
	RDebug::Print(_L("CSecObsNotify::StartNotifier"));
	#endif
	    
    #ifdef __SAP_TERMINAL_CONTROL_FW
    // Check with the SCP Server whether this notification request resulted from an admin command,
    // we're only interested in the security code events
    if ( aNotifierToStart == ESecurityNotifierSecurityCode )        
        {        
        TBool isNormalCall = ETrue;
        
        RSCPClient SCPClient;
        if ( SCPClient.Connect() == KErrNone )
            {
            if ( SCPClient.QueryAdminCmd( ESCPCommandLockPhone ) )
                {
                #if defined(_DEBUG)
        	    RDebug::Print(_L("CSecObsNotify::SCP admin command, no action required"));
	            #endif
	    
	            isNormalCall = EFalse;
                }
            
            SCPClient.Close();            
            }
        
        if ( !isNormalCall )
            {
            // Nothing further to be done in this method
            return;
            }
        }
    #endif //__SAP_TERMINAL_CONTROL_FW  
    	
    if(IsActive())
        {
        #if defined(_DEBUG)
	    RDebug::Print(_L("CSecObsNotify::StartNotifier IsActive"));
	    #endif
        
        if (iActiveNotifier != ESecurityNotifierNone)
            {
                #if defined(_DEBUG)
	            RDebug::Print(_L("CSecObsNotify::StartNotifier: iActiveNotifier != ESecurityNotifierNone"));
	            #endif
                iPendingNotifier = aNotifierToStart; // to be started when previous has ended
                return;
            }

        }
    #if defined(_DEBUG)
	RDebug::Print(_L("CSecObsNotify::StartNotifier: SWITCH"));
	#endif
	TInt err = KErrNone;
    switch (aNotifierToStart)
        {
        #if defined(__PROTOCOL_WCDMA) || defined(__UPIN)
        case ESecurityNotifierUPin:
            {
            #if defined(_DEBUG)
	        RDebug::Print(_L("CSecObsNotify::StartNotifier: UPin"));
	        #endif
            err = iNotifier.Connect();
            if(err == KErrNone)
            	iNotifier.StartNotifierAndGetResponse(iStatus, KSecurityNotifierUid, KUPinRequired,iDummy);
            break;
            }
        case ESecurityNotifierUPuk:
            {
            #if defined(_DEBUG)
	        RDebug::Print(_L("CSecObsNotify::StartNotifier: UPuk"));
	        #endif
	        err = iNotifier.Connect();
            if(err == KErrNone)
            	iNotifier.StartNotifierAndGetResponse(iStatus, KSecurityNotifierUid, KUPukRequired,iDummy);
            break;
            }
        #endif //__PROTOCOL_WCDMA
        case ESecurityNotifierPin1:
            {
            #if defined(_DEBUG)
	        RDebug::Print(_L("CSecObsNotify::StartNotifier: Pin1"));
	        #endif
            err = iNotifier.Connect();
            if(err == KErrNone)
            	iNotifier.StartNotifierAndGetResponse(iStatus, KSecurityNotifierUid, KPin1Required,iDummy);
            break;
            }
        case ESecurityNotifierPuk1:
            {
            #if defined(_DEBUG)
	        RDebug::Print(_L("CSecObsNotify::StartNotifier: Puk1"));
	        #endif
            err = iNotifier.Connect();
            if(err == KErrNone)
            	iNotifier.StartNotifierAndGetResponse(iStatus, KSecurityNotifierUid, KPuk1Required,iDummy);
            break;
            }
        case ESecurityNotifierPin2:
            {
            #if defined(_DEBUG)
	        RDebug::Print(_L("CSecObsNotify::StartNotifier: Pin2"));
	        #endif
            err = iNotifier.Connect();
            if(err == KErrNone)
            	iNotifier.StartNotifierAndGetResponse(iStatus, KSecurityNotifierUid, KPin2Required,iDummy);
            break;
            }
        case ESecurityNotifierPuk2:
            {
            #if defined(_DEBUG)
	        RDebug::Print(_L("CSecObsNotify::StartNotifier: Puk2"));
	        #endif
            err = iNotifier.Connect();
            if(err == KErrNone)
            	iNotifier.StartNotifierAndGetResponse(iStatus, KSecurityNotifierUid, KPuk2Required,iDummy);
            break;
            }
        case ESecurityNotifierSecurityCode:
            {
            #if defined(_DEBUG)
	        RDebug::Print(_L("CSecObsNotify::StartNotifier: Security Code"));
	        #endif
            #if defined(_DEBUG)
	        RDebug::Print(_L("CSecObsNotify::StartNotifier ESecurityNotifierSecurityCode"));
	        #endif
            err = iNotifier.Connect();
            if(err == KErrNone)
            	iNotifier.StartNotifierAndGetResponse(iStatus, KSecurityNotifierUid, KPassPhraseRequired,iDummy);
            #if defined(_DEBUG)
	        RDebug::Print(_L("CSecObsNotify::StartNotifier SEC: StartNotifierAndGetResponse"));
	        #endif
            break;
            }
        case ESecuritySimLockRestrictionOn:
            {
            #if defined(_DEBUG)
	        RDebug::Print(_L("CSecObsNotify::StartNotifier: Sim Lock"));
	        #endif
            err = iNotifier.Connect();
            if(err == KErrNone)
            	iNotifier.StartNotifierAndGetResponse(iStatus, KSecurityNotifierUid, KSimTerminated,iDummy);
			break;
            }
        default:
            iActiveNotifier = ESecurityNotifierNone;
            return;
        }
    if (err != KErrNone)
    {
    	_LIT(panicCategory, "SecObsNotify PANIC");
    	TInt reason(1);
    	User::Panic(panicCategory, reason); 	
    }
    
    iActiveNotifier = aNotifierToStart;
    #if defined(_DEBUG)
	RDebug::Print(_L("CSecObsNotify::StartNotifier SetActive"));
	#endif
    SetActive();
    }

//
// ----------------------------------------------------------
// CSecObsNotify::StartNotifier
// Close open notifier
// ----------------------------------------------------------
//
void CSecObsNotify::DoCancel()
    {
    CancelActiveNotifier();
    }

//  End of File
