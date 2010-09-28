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
*   SecurityNotifier implementation. Notifier calls appropriate
*   dialog to be opened from SecUi.
*
*/


#include "SecurityNotifier.h"
#include <e32std.h>
#include <secui.h>
#include <secuisecurityhandler.h>
#include <e32property.h>
#include <securityuisprivatepskeys.h>
#include <eikenv.h>
#include <startupdomainpskeys.h>
#include <mmtsy_names.h>
#include <securitynotification.h>
#include <AknQueryDialog.h>
#include <featmgr.h>
#include <SCPClient.h>
#include <apgcli.h>
#include <keyguardaccessapi.h>

//  LOCAL CONSTANTS AND MACROS
  /*****************************************************
  * Series 60 Customer / TSY
  * Needs customer TSY implementation
  *****************************************************/

const TInt KTriesToConnectServer( 2 );
const TInt KTimeBeforeRetryingServerConnection( 50000 );
const TInt PhoneIndex( 0 );
const TInt KDelayPeriod(200000);


// ================= EXPORTED FUNCTIONS ====================

// ---------------------------------------------------------
//
// Lib main entry point: Creates an notifiers array.
//
// ---------------------------------------------------------
EXPORT_C CArrayPtr<MEikSrvNotifierBase2>* NotifierArray()
    {
    CArrayPtrFlat<MEikSrvNotifierBase2>* array = new CArrayPtrFlat<MEikSrvNotifierBase2>(1);

    if (array)
        {
        TRAPD(err,
            {
            MEikSrvNotifierBase2* securityNotifier = CSecurityNotifier::NewL();
            CleanupStack::PushL(securityNotifier);
            array->AppendL(securityNotifier);
            CleanupStack::Pop(securityNotifier);
            });

        if (err)
            {
            TInt count = array->Count();
            while (count--)
                (*array)[count]->Release();
            delete array;
            array = NULL;
            }
        }

    return (array);
    }

// ================= LOCAL FUNCTIONS =======================

TBool IsAdminCall()
    {
    TBool isAdminCall = EFalse;
 if(FeatureManager::FeatureSupported(KFeatureIdSapTerminalControlFw ))
    {
    RSCPClient scpClient;
    if ( scpClient.Connect() == KErrNone )
        {
        if ( scpClient.QueryAdminCmd( ESCPCommandLockPhone ) )
            {
            #if defined(_DEBUG)
          RDebug::Print(_L("CSecObsNotify::SCP admin command, no action required"));
          #endif

          isAdminCall = ETrue;
            }

        scpClient.Close();
        }
   }
    return isAdminCall;
    }


// ================= CSecurityNotifier =======================
//
// ----------------------------------------------------------
// CSecurityNotifier::NewL()
// Create new CSecurityNotifier object.
// ----------------------------------------------------------
//
MEikSrvNotifierBase2* CSecurityNotifier::NewL()
    {
    MEikSrvNotifierBase2* self = new(ELeave) CSecurityNotifier;
    return self;
    }
//
// ----------------------------------------------------------
//  CSecurityNotifier::CSecurityNotifier()
//  Default constructor: Sets the active object's priority and
//  puts itself to the active scheduler stack.
// ----------------------------------------------------------
//
CSecurityNotifier::CSecurityNotifier(): CActive(EPriorityStandard)
    {
    CActiveScheduler::Add(this);
    TRAP_IGNORE( FeatureManager::InitializeLibL() );
    }
//
// ----------------------------------------------------------
// CSecurityNotifier::~CSecurityNotifier()
// Destructor
// ----------------------------------------------------------
//
CSecurityNotifier::~CSecurityNotifier()
    {
      FeatureManager::UnInitializeLib();
    }
//
// ----------------------------------------------------------
// CSecurityNotifier::Release()
// Called when all resources allocated by notifiers should be freed.
// ----------------------------------------------------------
//
void CSecurityNotifier::Release()
    {
    delete this;
    }
//
// ----------------------------------------------------------
// CSecurityNotifier::Release()
// This function is called once when DLL is loaded.
// Adds CSecurityNotifier resource file to the list maintained by CCoeEnv.
// ----------------------------------------------------------
//
MEikSrvNotifierBase2::TNotifierInfo CSecurityNotifier::RegisterL()
    {
  #if defined(_DEBUG)
    RDebug::Print(_L("(SECURITYNOTIFIER)CSecurityNotifier::RegisterL()"));
  #endif
    iInfo.iUid = KSecurityNotifierUid;
    iInfo.iChannel = KSecurityNotifierChannel;
    iInfo.iPriority = ENotifierPriorityHigh;
    return iInfo;
    }
//
// ----------------------------------------------------------
// CSecurityNotifier::Info()
// Info
// ----------------------------------------------------------
//
MEikSrvNotifierBase2::TNotifierInfo CSecurityNotifier::Info() const
    {
  #if defined(_DEBUG)
    RDebug::Print(_L("(SECURITYNOTIFIER)CSecurityNotifier::Info()"));
  #endif
    return iInfo;
    }

//
// ----------------------------------------------------------
// CSecurityNotifier::StartL()
// Will be called by the manager to start the notifier. The contents of
// the buffer are passed unchanged from the RNotifier call, and can be used
// by the notifier implementation.
// ----------------------------------------------------------
//
TPtrC8 CSecurityNotifier::StartL(const TDesC8& /*aBuffer*/)
    {
  #if defined(_DEBUG)
    RDebug::Print(_L("(SECURITYNOTIFIER)CSecurityNotifier::StartL()"));
  #endif
    TPtrC8 ret(KNullDesC8);
    return (ret);
    }
//
// ----------------------------------------------------------
// CSecurityNotifier::StartL()
// Asynchronic notifier launch.
// ----------------------------------------------------------
//
void CSecurityNotifier::StartL(const TDesC8& aBuffer, TInt aReturnVal,const RMessagePtr2& aMessage)
    {
  #if defined(_DEBUG)
    RDebug::Print(_L("(SECURITYNOTIFIER)CSecurityNotifier::StartL2()"));
  #endif
    TRAPD(err, GetParamsL(aBuffer, aReturnVal, aMessage));
    if (err)
        {
        aMessage.Complete(err);
        User::Leave(err);
        }
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECURITYNOTIFIER)CSecurityNotifier::StartL2() Start BEGIN"));
    #endif
    }
//
// ----------------------------------------------------------
// CSecurityNotifier::GetParamsL()
// Initialize parameters and jump to RunL
// ----------------------------------------------------------
//
void CSecurityNotifier::GetParamsL(const TDesC8& aBuffer, TInt aReturnVal, const RMessagePtr2& aMessage)
    {
    
  /*****************************************************
  * Series 60 Customer / ETel
  * Series 60  ETel API
  *****************************************************/
    iMessage = aMessage;
    iReturnVal = aReturnVal;
    TBool skipQuery = EFalse; // In some cases the query is handled by some other entity and SecurityNotifier should skip it.

  #if defined(_DEBUG)
    RDebug::Print(_L("(SECURITYNOTIFIER)CSecurityNotifier::GetParamsL() Start BEGIN"));
    #endif
    

    TSecurityNotificationPckg pckg;
    pckg.Copy( aBuffer );
    iStartup = pckg().iStartup;
    TInt lEvent = pckg().iEvent;
    iEvent = static_cast<RMobilePhone::TMobilePhoneSecurityEvent>(lEvent);
    if(lEvent==100+RMobilePhone::EPhonePasswordRequired)
    	{
    	// from AskSecCodeInAutoLockL
    	iEvent = RMobilePhone::EPhonePasswordRequired;
    	}

  #if defined(_DEBUG)
  RDebug::Printf( "%s %s (%u) iStartup =%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iStartup );
  RDebug::Printf( "%s %s (%u) lEvent =%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, lEvent );
  RDebug::Printf( "%s %s (%u) iEvent =%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iEvent );
  RDebug::Printf( "%s %s (%u) iReturnVal =%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iReturnVal );
  #endif

    // Forces Autolock to load
    TInt err = KErrNone;

		if(iEvent == RMobilePhone::EPhonePasswordRequired && lEvent==100+RMobilePhone::EPhonePasswordRequired)
			{
		  #if defined(_DEBUG)
		  RDebug::Printf( "%s %s (%u) query from AskSecCodeInAutoLockL . No need to start Autolock.exe =%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
		  #endif
			}
		else
			{
	    CKeyguardAccessApi* iKeyguardAccess = CKeyguardAccessApi::NewL( );
	   	RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
			err = iKeyguardAccess->ShowKeysLockedNote( );
			RDebug::Printf( "%s %s (%u) err=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, err );
			delete iKeyguardAccess;
			}

if(FeatureManager::FeatureSupported(KFeatureIdSapTerminalControlFw ))
    {
    if(iEvent == RMobilePhone::EPhonePasswordRequired)
        {
            skipQuery = IsAdminCall(); // SCP handles the call.
        }
}

    if ( skipQuery )
        {
        iMessage.Write( iReturnVal, TPckgBuf<TInt>( KErrNone ) );
        iMessage.Complete( KErrNone );
        }
    else
        {
        // Call SetActive() so RunL() will be called by the active scheduler
        SetActive();
        iStatus = KRequestPending;
        TRequestStatus* stat = &iStatus;
      #if defined(_DEBUG)
        RDebug::Print(_L("CSecurityNotifier::GetParamsL() End"));
        #endif
        User::RequestComplete(stat, KErrNone); // jump to RunL
        }
    }
//
// ----------------------------------------------------------
// CSecurityNotifier::RunL()
// Show query
// ----------------------------------------------------------
void CSecurityNotifier::RunL()
    {
  /*****************************************************
  * Series 60 Customer / ETel
  * Series 60  ETel API
  *****************************************************/
  /*****************************************************
  * Series 60 Customer / TSY
  * Needs customer TSY implementation
  *****************************************************/

    TInt err( KErrGeneral );
    TInt thisTry( 0 );
  RTelServer::TPhoneInfo PhoneInfo;
  #if defined(_DEBUG)
    RDebug::Print(_L("CSecurityNotifier::RunL() Start"));
    #endif
    /*All server connections are tried to be made KTriesToConnectServer times because occasional
    fails on connections are possible, at least on some servers*/

    // connect to ETel server
  #if defined(_DEBUG)
    RDebug::Print(_L("CSecurityNotifier::RunL() connect to ETel server"));
    #endif
    while ( ( err = iServer.Connect() ) != KErrNone && ( thisTry++ ) <= KTriesToConnectServer )
        {
        User::After( KTimeBeforeRetryingServerConnection );
        }
    User::LeaveIfError( err );

    thisTry = 0;

    // load TSY
  #if defined(_DEBUG)
    RDebug::Print(_L("CSecurityNotifier::RunL() load TSY"));
    #endif
    
    if ( !iPhone.SubSessionHandle() )    
    {
      err = iServer.LoadPhoneModule( KMmTsyModuleName );
      if ( err != KErrAlreadyExists )
        {
        // may also return KErrAlreadyExists if something
        // else has already loaded the TSY module. And that is
        // not an error.
        User::LeaveIfError( err );
        }

       // open phones
      #if defined(_DEBUG)
      RDebug::Print(_L("CSecurityNotifier::RunL() open phones"));
      #endif
      User::LeaveIfError(iServer.SetExtendedErrorGranularity(RTelServer::EErrorExtended));
      User::LeaveIfError(iServer.GetPhoneInfo(PhoneIndex, PhoneInfo));
      User::LeaveIfError(iPhone.Open(iServer,PhoneInfo.iName));
    }   
        
    RProperty Property;
    CleanupClosePushL( Property );
    err = Property.Set(KPSUidStartup, KStartupSecurityCodeQueryStatus, ESecurityQueryActive);
    User::LeaveIfError( err );

    // initialize security ui
  #if defined(_DEBUG)
    RDebug::Print(_L("CSecurityNotifier::RunL() initialize security ui"));
    #endif
    CSecurityHandler* handler = new(ELeave) CSecurityHandler(iPhone);
    CleanupStack::PushL(handler);
    TSecUi::InitializeLibL();
    
    TBool StartUp = iStartup;

    TInt secUiOriginatedQuery(ESecurityUIsSecUIOriginated);
    err = KErrNone;
    if(!StartUp)
    {
        err = Property.Get(KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery, secUiOriginatedQuery);
    }
    
    // handle event
    TInt result = KErrNone;
    //Bring the window group to foreground
    ( CEikonEnv::Static() )->BringForwards(ETrue);
    
    TRAPD( error, handler->HandleEventL( iEvent, iStartup, result ) );

   
    // if something went wrong cancel the code request
    if (error)
        {
    #if defined(_DEBUG)
    RDebug::Print(_L("CSecurityNotifier::RunL() ERROR: %d"), error);
    #endif
    TBool wcdmaSupported(FeatureManager::FeatureSupported( KFeatureIdProtocolWcdma ));
    TBool upinSupported(FeatureManager::FeatureSupported( KFeatureIdUpin ));
        switch (iEvent)
            {
            case RMobilePhone::EUniversalPinRequired:
                if(wcdmaSupported || upinSupported)
                  {
                   iPhone.AbortSecurityCode(RMobilePhone::ESecurityUniversalPin);
                  }
                break;
            case RMobilePhone::EUniversalPukRequired:
                if(wcdmaSupported || upinSupported)
                  {
                   iPhone.AbortSecurityCode(RMobilePhone::ESecurityUniversalPuk);
                  }
                break;
      case RMobilePhone::EPin1Required:
                iPhone.AbortSecurityCode(RMobilePhone::ESecurityCodePin1);
                break;
      case RMobilePhone::EPuk1Required:
                iPhone.AbortSecurityCode(RMobilePhone::ESecurityCodePuk1);
                break;
      case RMobilePhone::EPin2Required:
                iPhone.AbortSecurityCode(RMobilePhone::ESecurityCodePin2);
                break;
      case RMobilePhone::EPuk2Required:
                iPhone.AbortSecurityCode(RMobilePhone::ESecurityCodePuk2);
                break;
      case RMobilePhone::EPhonePasswordRequired:
                iPhone.AbortSecurityCode(RMobilePhone::ESecurityCodePhonePassword);
                break;
            default:
                break;
            }
        }

    // uninitialize security ui
    CleanupStack::PopAndDestroy(handler); // handler
    TSecUi::UnInitializeLib();
    Property.Set(KPSUidStartup, KStartupSecurityCodeQueryStatus, ESecurityQueryNotActive);
    CleanupStack::PopAndDestroy( &Property );

    //close ETel connection
    if (iServer.Handle())
        {
        iPhone.Close();
        iServer.UnloadPhoneModule(KMmTsyModuleName);
        iServer.Close();
        }

    User::LeaveIfError(error);

    // Complete message and free resources
    iMessage.Write( iReturnVal, TPckgBuf<TInt>( result ) );
    iMessage.Complete(KErrNone);
    iReturnVal = KErrNone;
    //Leave the window group to foreground for a short time to absorb key presses so that autolock has time to activate.
    if(!StartUp)
    User::After(KDelayPeriod);
  ( CEikonEnv::Static() )->BringForwards(EFalse);
  #if defined(_DEBUG)
    RDebug::Print(_L("CSecurityNotifier::RunL() End"));
    #endif
    }

// ----------------------------------------------------------
// This method will be called by framework (CActive)
// if active object is still active.
// Does nothing here.
// ----------------------------------------------------------
//
void CSecurityNotifier::DoCancel()
    {
    }
//
// ----------------------------------------------------------
//  CSecurityNotifier::Cancel()
//  Will be called by the manager to stop the notifier. Nothing happens when
//  a call to cancel is made on a notifier that hasn't started.
// ----------------------------------------------------------
//
void CSecurityNotifier::Cancel()
    {
    }
//
// ----------------------------------------------------------
//  CSecurityNotifier::UpdateL()
//  Will be called by the manager to update an already started  notifier.
//  The contents of the buffer are passed unchanged from the RNotifier call,
//  and can be used by the notifier implementation
//  Nothing happens when a call to update is made on a notifier that hasn't started.
// ----------------------------------------------------------
//
TPtrC8 CSecurityNotifier::UpdateL(const TDesC8& /*aBuffer*/)
    {
    return TPtrC8();
    }
//
// ----------------------------------------------------------
// CE32Dll()
// DLL entry point
// ----------------------------------------------------------
//

#ifndef EKA2

GLDEF_C TInt E32Dll( TDllReason /*aReason*/)
    {
    return KErrNone;
    }

#endif

// end of file
