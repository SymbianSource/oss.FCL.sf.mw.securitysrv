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
* Description:  System Lock interface
*
*
*/


#include    <e32property.h>
#include    <PSVariables.h>   // Property values
#include    <coreapplicationuisdomainpskeys.h>
#include    "SecUiSystemLock.h"
#include    <eikenv.h>
// #include    <AknNotifierController.h>
#include    <rmmcustomapi.h>
#include    "secuisecuritysettings.h"
#include    "SecUiWait.h"
#include    <mmtsy_names.h>
#include 	<e32property.h>
#include <ctsydomainpskeys.h>
#include    <securityuisprivatepskeys.h>
#include    <devicelockaccessapi.h>

    /*****************************************************
    *    Series 60 Customer / TSY
    *    Needs customer TSY implementation
    *****************************************************/
//  LOCAL CONSTANTS AND MACROS  

const TInt KTriesToConnectServer( 2 );
const TInt KTimeBeforeRetryingServerConnection( 50000 );

// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CSystemLock::NewL()    
// 
// ----------------------------------------------------------
// 
EXPORT_C CSystemLock* CSystemLock::NewL()
    {
    RDebug::Printf( "%s %s (%u) this should not be called=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    CSystemLock* self = new(ELeave) CSystemLock();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }
//
// ----------------------------------------------------------
// CSystemLock::ConstructL()    
// 
// ----------------------------------------------------------
// 
void CSystemLock::ConstructL()    
    {    
    RDebug::Printf( "%s %s (%u) this should not be called=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
    /*****************************************************
    *    Series 60 Customer / TSY
    *    Needs customer TSY implementation
    *****************************************************/
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSystemLock::ConstructL()"));
    #endif
    TInt err( KErrGeneral );
    TInt thisTry( 0 );
    
    /* All server connections are tried to be made KTiesToConnectServer times because occasional
    fails on connections are possible, at least on some servers */
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
        // May also return KErrAlreadyExists if something else
        // has already loaded the TSY module. And that is
        // not an error.
        User::LeaveIfError( err );
        }

    // open phones
    User::LeaveIfError(iPhone.Open(iServer, KMmTsyPhoneName));
    CActiveScheduler::Add(this);            
    }    
// ----------------------------------------------------------
// CSystemLock::CSystemLock()
// C++ constructor
// ----------------------------------------------------------
// 
CSystemLock::CSystemLock() : CActive(0)                    
    {    
    }
//
// ----------------------------------------------------------
// CSystemLock::CSystemLock()
// Destructor
// ----------------------------------------------------------
//
EXPORT_C CSystemLock::~CSystemLock()
    {
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
    /*****************************************************
    *    Series 60 Customer / TSY
    *    Needs customer TSY implementation
    *****************************************************/

    Cancel();
   
     // close phone
    if (iPhone.SubSessionHandle())
        iPhone.Close();
    //close ETel connection
    if (iServer.Handle())
        {
        iServer.UnloadPhoneModule(KMmTsyModuleName);
        iServer.Close();
        }
    }
//
// ----------------------------------------------------------
// CSystemLock::SetLockedL()
// Activates system lock
// this was used by SysAp, but it's not longer used
// ----------------------------------------------------------
//
EXPORT_C void CSystemLock::SetLockedL()
    {
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
    RDebug::Printf( "%s %s (%u) this should not be called=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSystemLock::SetLockedL()"));
    #endif
    // close fast-swap window
            RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
            CDevicelockAccessApi* iDevicelockAccess = CDevicelockAccessApi::NewL( );
           	RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
						iDevicelockAccess->OfferDevicelock();
						// this will do				EnableDevicelock( EDevicelockManual );
						RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    }
//
// ----------------------------------------------------------
// CSystemLock::RunL()
// Handles query result
// this was used by SysAp, but it's not longer used
// ----------------------------------------------------------
// 
void CSystemLock::RunL()
    {    
    RDebug::Printf( "%s %s (%u) this should not be called=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );

    #if defined(_DEBUG)
    TInt status(iStatus.Int());
    RDebug::Print(_L("(SECUI)CSystemLock::RunL(): %d"), status);
    #endif
    //lower the flag
    RProperty::Set(KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery, ESecurityUIsETelAPIOriginated);
    if    (iStatus == KErrNone)
        {
        	TInt callState;
        	RProperty::Get(KPSUidCtsyCallInformation, KCTsyCallState, callState);
    		//If there is ann ongoing call, phone is not locked.
    	    if (callState == EPSCTsyCallStateNone)
    		{
    			#if defined(_DEBUG)
        		RDebug::Print(_L("(SECUI)CSystemLock::RunL() KErrNone"));
        		#endif
        		// clear notifiers
        		// not any more. Avkon is deprecated. Besides, this function should not be called.
        		// AknNotifierController::HideAllNotifications(ETrue);
        		// query approved -> lock system  
			#ifdef RD_REMOTELOCK
				iProperty.Set(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, EManualLocked);
			#else// !RD_REMOTELOCK
        		iProperty.Set(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, EAutolockOn);
        	#endif//RD_REMOTELOCK
        		// not any more. Avkon is deprecated. Besides, this function should not be called.
        		// AknNotifierController::HideAllNotifications(EFalse);
    		}
        }
    else if((iStatus != KErrCancel) && (iStatus != KErrAbort))
        {   //Code error or something like that. Show the dialog again.
            #if defined(_DEBUG)
        	RDebug::Print(_L("(SECUI)CSystemLock::RunL() Code Error"));
        	#endif
            SetLockedL();
        }
    else
        {
          //User canceled the dialog; do nothing...
        }
        
        
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSystemLock::RunL() END"));
    #endif
    }
//
// ----------------------------------------------------------
// CSecObsNotify::StartNotifier
// Cancels code request
// ----------------------------------------------------------
//
void CSystemLock::DoCancel()
    {
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSystemLock::DoCancel"));
    #endif
    iPhone.CancelAsyncRequest(EMobilePhoneSetLockSetting);
    }

// End of file
