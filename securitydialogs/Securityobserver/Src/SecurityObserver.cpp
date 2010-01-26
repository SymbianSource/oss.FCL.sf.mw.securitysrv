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
* Description:  Observer for ETel security events. Uses SecurityNotifier
*               which calls appropriate dialog to be opened from SecUi.
*
*
*/

#include <w32std.h>
#include <e32std.h>
#include <coedef.h>

#include <e32base.h>
#include <etelmm.h>
#include <eiksrv.h>
#include    <e32property.h>
#include    <PSVariables.h>   // Property values
#include <CoreApplicationUIsPrivatePSKeys.h>
#include <SecurityNotifier.h>
#include <MmTsy_names.h>
#include "SecurityObserver.h"
#include "SecObsNotify.h"
#include "SecurityObserver.hrh"


//  LOCAL CONSTANTS AND MACROS
	/*****************************************************
	*	Series 60 Customer / TSY
	*	Needs customer TSY implementation
	*****************************************************/

_LIT(SecObserver,"SecurityObserver");

const TInt KTriesToConnectServer( 2 );
const TInt KTimeBeforeRetryingServerConnection( 50000 );
const TInt PhoneIndex( 0 );

// LOCAL FUNCTION PROTOTYPES
LOCAL_C void StartL();


//===================== LOCAL FUNCTIONS ====================
//
//----------------------------------------------------------
// E32Main
//----------------------------------------------------------
//
GLDEF_C TInt E32Main()
    {
    __UHEAP_MARK;
    CTrapCleanup* theCleanup = CTrapCleanup::New();
    TRAPD(ret,StartL());
    delete theCleanup;
    __UHEAP_MARKEND;
    if (ret)
        {
        // Panic here in order to enter SW
        // reset mechanism.
        User::Panic(SecObserver,ret);
        }
    return(KErrNone);
    }

#if defined(__WINS__)
EXPORT_C TInt WinsMain(TAny *)
    {
    E32Main();
    return KErrNone;
    }
#endif  // __WINS__

//
//----------------------------------------------------------
// StartL() SecurityObserver starter.
//----------------------------------------------------------
//
LOCAL_C void StartL()
    {
    __UHEAP_MARK;

    #if defined(_DEBUG)
    RDebug::Print(_L("Starting SecurityObsever"));
    #endif


#ifdef __WINS__

	// do nothing...

#else  // __WINS__
	TInt simStatus;
#ifndef RD_STARTUP_CHANGE
    RProperty property;
    for(;;)
	{	
        property.Get(KUidSystemCategory, KPSUidSimCStatusValue, simStatus);
        if	(simStatus > EPSCSimInitWait)
            break;
        User::After(100000);
	}
#endif //RD_STARTUP_CHANGE
#endif  // __WINS__
	// rename thread to "SecurityObserver"
    User::RenameThread(SecObserver);
	
	//Initialise the PubSub variable used to distinguish between code requests 
	//originated from DOS and SecUi
	_LIT_SECURITY_POLICY_PASS(KReadPolicy); 
	_LIT_SECURITY_POLICY_C1(KWritePolicy, ECapabilityWriteDeviceData);   
    TInt pSresult = RProperty::Define(KPSUidCoreApplicationUIs, KCoreAppUIsSecUIOriginatedQuery, RProperty::EInt, KReadPolicy, KWritePolicy);    
 	RProperty::Set(KPSUidCoreApplicationUIs, KCoreAppUIsSecUIOriginatedQuery, ECoreAppUIsETelAPIOriginated);
    #if defined(_DEBUG)
    RDebug::Print(_L("SecurityObsever PS Define result:´%d"), pSresult);
	#endif
	
    // Install active scheduler
	CActiveScheduler* theScheduler = new (ELeave) CActiveScheduler();
	CleanupStack::PushL(theScheduler);
	CActiveScheduler::Install(theScheduler);

	// Start Observer 
    CSecurityObserver * theObserver = CSecurityObserver::NewL();
    CleanupStack::PushL(theObserver);

	#if defined(_DEBUG)
    RDebug::Print(_L("SecurityObsever has been started"));
	#endif

    // Run the scheduler
    CActiveScheduler::Start();
   
	


    CleanupStack::PopAndDestroy(2);//theScheduler and theObserver

    __UHEAP_MARKEND;
    }
// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CSecurityObserver::NewL()
// Constructs a new entry.
// ----------------------------------------------------------
//
CSecurityObserver* CSecurityObserver::NewL()
    {
    CSecurityObserver* self = new (ELeave) CSecurityObserver();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

//
// ----------------------------------------------------------
// CSecurityObserver::ConstructL()
// Symbian OS constructor
// ----------------------------------------------------------
//
void CSecurityObserver::ConstructL()
    {
	/*****************************************************
	*	Series 60 Customer / ETel
	*	Series 60  ETel API
	*****************************************************/
	/*****************************************************
	*	Series 60 Customer / TSY
	*	Needs customer TSY implementation
	*****************************************************/
	TInt err( KErrGeneral );
    TInt thisTry( 0 );
    RTelServer::TPhoneInfo PhoneInfo;
	//connect to ETel

	thisTry = 0;

	// connect to ETel server
	while ( ( err = iServer.Connect() ) != KErrNone && ( thisTry++ ) <= KTriesToConnectServer )
        {
        User::After( KTimeBeforeRetryingServerConnection );
        }
    User::LeaveIfError( err );

    // load TSY
    err = iServer.LoadPhoneModule( KMmTsyModuleName );
    if ( err != KErrAlreadyExists )
        {
        // may return also KErrAlreadyExists if some other
        // is already loaded the tsy module. And that is
        // not an error.
        User::LeaveIfError( err );
        }

	// open phone
	User::LeaveIfError(iServer.GetPhoneInfo(PhoneIndex, PhoneInfo));
	User::LeaveIfError(iPhone.Open(iServer,PhoneInfo.iName));

	//  Add this active object to the scheduler.
	CActiveScheduler::Add(this);
 
	//start notifier controller
	iNotifierController = CSecObsNotify::NewL(this);

	//Sim lock observer
	iSimLockObserver = CSimLockObserver::NewL(iNotifierController);
			
	
	#if defined(_DEBUG)
    RDebug::Print(_L("SecurityObsever: Checking sim lock status"));
	#endif

	// check sim lock state at startup
#ifndef RD_STARTUP_CHANGE
    TInt ret2;
    iProperty.Get(KUidSystemCategory, KPSUidSimLockStatusValue, ret2);

    	if (ret2 == EPSSimLockRestrictionOn)
		{
		iNotifierController->StartNotifier(ESecuritySimLockRestrictionOn);
		}
	else
		{
		// start observing sim lock events
		iSimLockObserver->Start();
		}
#else //RD_STARTUP_CHANGE
        iSimLockObserver->Start();
#endif //RD_STARTUP_CHANGE

	#if defined(_DEBUG)
    RDebug::Print(_L("SecurityObsever: Checking sim status"));
	#endif

#ifndef RD_STARTUP_CHANGE
	// check sim state at startup
	TInt simStatus;
	iProperty.Get(KUidSystemCategory, KPSUidSimCStatusValue, simStatus);
	
	switch (simStatus)
		{
#if defined(__PROTOCOL_WCDMA) || defined(__UPIN)
        case EPSCSimUPinVerifyRequired:
            #if defined(_DEBUG)
			RDebug::Print(_L("SecurityObsever: UPin required "));
		    #endif
	
			iUPinRequired = ETrue;
			iNotifierController->StartNotifier(ESecurityNotifierUPin);
			// set the first notification request to ETel active
			StartListen();
			return;
#endif //__PROTOCOL_WCDMA
        case EPSCSimPinVerifyRequired:
			#if defined(_DEBUG)
			RDebug::Print(_L("SecurityObsever: Pin required "));
			#endif
	
			iPin1Required = ETrue;
			iNotifierController->StartNotifier(ESecurityNotifierPin1);
			// set the first notification request to ETel active
			StartListen();
			return;
        case EPSCSimBlocked:
			#if defined(_DEBUG)
			RDebug::Print(_L("SecurityObsever: Puk required "));
			#endif

			iPuk1Required = ETrue;
			iNotifierController->StartNotifier(ESecurityNotifierPuk1);
			// set the first notification request to ETel active
			StartListen();
			return;
#if defined(__PROTOCOL_WCDMA) || defined(__UPIN)
        case EPSCSimUPinBlocked:
			#if defined(_DEBUG)
			RDebug::Print(_L("SecurityObsever: UPuk required "));
			#endif

			iUPukRequired = ETrue;
			iNotifierController->StartNotifier(ESecurityNotifierUPuk);
			// set the first notification request to ETel active
			StartListen();
			return;
#endif //__PROTOCOL_WCDMA
		default:
			break;
		}

#ifndef __WINS__
	// check if security code is needed at startup
    for (;;)
		{		
		iProperty.Get(KUidSystemCategory, KPSUidSecurityCodeStatusValue, ret2);
		if (ret2 == EPSSecurityCodeRequired)
			{
			iPassPhraseRequired = ETrue;
			iNotifierController->StartNotifier(ESecurityNotifierSecurityCode);	
			break;	
			}
		if (ret2 == EPSSecurityCodeNotRequired)
			{
			break;
			}
		User::After(100000);
		}
#endif  // __WINS__


	#if defined(_DEBUG)
    RDebug::Print(_L("SecurityObsever: No Pin/Puk required. Start listening security events"));
	#endif
#endif //RD_STARTUP_CHANGE
	// set the first notification request to ETel active
	StartListen();
	}
	
//
// ----------------------------------------------------------
// CSecurityObserver::CSecurityObserver()
// C++ constructor
// ----------------------------------------------------------
// 

CSecurityObserver::CSecurityObserver(): CActive(0),iPin1Required(EFalse)
										,iPuk1Required(EFalse)
										,iPassPhraseRequired(EFalse)
    {
    }

//
// ----------------------------------------------------------
// CSecurityObserver::~CSecurityObserver()
// C++ destructor. Stops observing ETel events and closes ETel connection.
// ----------------------------------------------------------
//
CSecurityObserver::~CSecurityObserver()
    {
	/*****************************************************
	*	Series 60 Customer / ETel
	*	Series 60  ETel API
	*****************************************************/
	/*****************************************************
	*	Series 60 Customer / TSY
	*	Needs customer TSY implementation
	*****************************************************/
    //  Call DoCancel() if iActive
    Cancel();

    if (iServer.Handle())
        {
        iPhone.Close();           
        iServer.UnloadPhoneModule(KMmTsyModuleName);
        iServer.Close();
        }

	delete iSimLockObserver;
	iSimLockObserver = NULL;
	delete iNotifierController; 
	iNotifierController = NULL;
	}
//
// ----------------------------------------------------------
// CSecurityObserver::StartListen()
// Starts observing ETel security events
// ----------------------------------------------------------
//
void CSecurityObserver::StartListen()
    {
	#if defined(_DEBUG)
    RDebug::Print(_L("(SECURITYOBSERVER)CSecurityObsever::StartListen()"));
	#endif
	if (!IsActive())
		{    		
		iPhone.NotifySecurityEvent(iStatus, iEvent);
		SetActive();
		}
	}
//
// ----------------------------------------------------------
// CSecurityObserver::RunL()
// Handles security events received from ETel.
// ----------------------------------------------------------
// 
void CSecurityObserver::RunL()
    {
	/*****************************************************
	*	Series 60 Customer / ETel
	*	Series 60  ETel API
	*****************************************************/
	#if defined(_DEBUG)
    RDebug::Print(_L("(SECURITYOBSERVER)CSecurityObsever::RunL()"));
	#endif
    if  (iStatus != KErrNone)
        return;
		
	switch(iEvent)
		{
        #if defined(__PROTOCOL_WCDMA) || defined(__UPIN)
        case RMobilePhone::EUniversalPinRequired:            
            
	   	   	#if defined(_DEBUG)
	    	RDebug::Print(_L("SecurityObsever: Handling EUniversalPinRequired event"));
			#endif
		  
			iUPinRequired = ETrue;
		    iNotifierController->StartNotifier(ESecurityNotifierUPin);
            break;
       case RMobilePhone::EUniversalPukRequired:

		    #if defined(_DEBUG)
			RDebug::Print(_L("SecurityObsever: Handling EUniversalPukRequired event"));
			#endif
		  
            iUPukRequired = ETrue;
		    iNotifierController->StartNotifier(ESecurityNotifierUPuk);
            break;
       #endif //__PROTOCOL_WCDMA        
       case RMobilePhone::EPin1Required:            
            
		   	#if defined(_DEBUG)
			RDebug::Print(_L("SecurityObsever: Handling EPin1Required event"));
			#endif
		  
			iPin1Required = ETrue;
		    iNotifierController->StartNotifier(ESecurityNotifierPin1);
            break;
       case RMobilePhone::EPuk1Required:

		    #if defined(_DEBUG)
			RDebug::Print(_L("SecurityObsever: Handling EPuk1Required event"));
			#endif
		  
            iPuk1Required = ETrue;
		    iNotifierController->StartNotifier(ESecurityNotifierPuk1);
            break;
       case RMobilePhone::EPin2Required:

			#if defined(_DEBUG)
			RDebug::Print(_L("SecurityObsever: Handling EPin2Required event"));
			#endif

			iNotifierController->StartNotifier(ESecurityNotifierPin2);
            break;
       case RMobilePhone::EPuk2Required:
			
			#if defined(_DEBUG)
			RDebug::Print(_L("SecurityObsever: Handling EPuk2Required event"));
			#endif
		   
			iNotifierController->StartNotifier(ESecurityNotifierPuk2);
            break;
       case RMobilePhone::EPhonePasswordRequired:

			#if defined(_DEBUG)
			RDebug::Print(_L("SecurityObsever: Handling EPhonePasswordRequired event"));
			#endif

		    iPassPhraseRequired = ETrue;
		    iNotifierController->StartNotifier(ESecurityNotifierSecurityCode);
            break;
       #if defined(__PROTOCOL_WCDMA) || defined(__UPIN)
       case RMobilePhone::EUniversalPinVerified:
			iUPinRequired = EFalse;
		    break;
	   case RMobilePhone::EUniversalPukVerified:
		    iUPukRequired = EFalse;
		    break;
       #endif //__PROTOCOL_WCDMA
	   case RMobilePhone::EPin1Verified:
			iPin1Required = EFalse;
		    break;
	   case RMobilePhone::EPuk1Verified:
		    iPuk1Required = EFalse;
		    break;
	   case RMobilePhone::EPhonePasswordVerified:
		    iPassPhraseRequired = EFalse;
		    break;
	   default:
			break;
		}	
	StartListen();
	}
//
// ----------------------------------------------------------
// CSecurityObserver::DoCancel()
// Stops observing ETel events.
// ----------------------------------------------------------
// 
void CSecurityObserver::DoCancel()
    {
	/*****************************************************
	*	Series 60 Customer / ETel
	*	Series 60  ETel API
	*****************************************************/
	#if defined(_DEBUG)
    RDebug::Print(_L("(SECURITYOBSERVER)CSecurityObsever::DoCancel()"));
	#endif
	iPhone.CancelAsyncRequest(EMobilePhoneNotifySecurityEvent);
	}

// End of file
