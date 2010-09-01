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
#include    <AknNotifierController.h>
#include    <rmmcustomapi.h>
#include    "secuisecuritysettings.h"
#include    "SecUiWait.h"
#include    <mmtsy_names.h>
#include 	<e32property.h>
#include <ctsydomainpskeys.h>
#include    <securityuisprivatepskeys.h>
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
// ----------------------------------------------------------
//
EXPORT_C void CSystemLock::SetLockedL()
    {
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSystemLock::SetLockedL()"));
    #endif
    // close fast-swap window
    CEikonEnv::Static()->DismissTaskList();

#ifdef __WINS__
    // can not verify security code in emulator ---> lock system 
#ifdef RD_REMOTELOCK
		iProperty.Set(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, EManualLocked);
#else// !RD_REMOTELOCK
        iProperty.Set(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, EAutolockOn);
#endif//RD_REMOTELOCK
#else //__WINS__

    if(IsActive())
        return;


    RMobilePhone::TMobilePhoneLock lockType = RMobilePhone::ELockPhoneDevice;
    RMobilePhone::TMobilePhoneLockInfoV1 lockInfo;
    RMobilePhone::TMobilePhoneLockInfoV1Pckg lockInfoPkg(lockInfo);
    RMobilePhone::TMobilePhoneLockSetting lockChange(RMobilePhone::ELockSetDisabled);
    CWait* wait = CWait::NewL();
    CleanupStack::PushL( wait );
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSystemLock::SetLockedL() GetLockInfo"));
    #endif
    iPhone.GetLockInfo(wait->iStatus, lockType, lockInfoPkg);
    if (wait->WaitForRequestL() == KErrNone)
        {
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSystemLock::SetLockedL() KErrNone"));
        #endif
        if (lockInfo.iSetting == RMobilePhone::ELockSetDisabled)
            {
            #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CSystemLock::SetLockedL() ELockSetDisabled"));
            #endif
            // ask code
            #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CSystemLock::SetLockedL() SetLockSetting"));
            #endif
            //iCustomPhone.CheckSecurityCode(iStatus, RMmCustomAPI::ESecurityCodePassPhrase);
            lockChange = RMobilePhone::ELockSetEnabled;
            RProperty::Set(KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery, ESecurityUIsSystemLockOriginated);
            iPhone.SetLockSetting(iStatus, lockType, lockChange);
            SetActive();
            }        
        else
            {
            #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CSystemLock::SetLockedL() Lock System"));
            #endif
 // lock system 
#ifdef RD_REMOTELOCK
		iProperty.Set(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, EManualLocked);
#else// !RD_REMOTELOCK
        iProperty.Set(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, EAutolockOn);
#endif //RD_REMOTELOCK

            #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CSystemLock::SetLockedL() Lock System OK"));
            #endif
            }
        }
    else
        {
        // ask code 
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSystemLock::SetLockedL() ask code (SLS) "));
        #endif
        lockChange = RMobilePhone::ELockSetEnabled;
        RProperty::Set(KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery, ESecurityUIsSystemLockOriginated);
        iPhone.SetLockSetting(iStatus, lockType, lockChange);
        SetActive();
        }
    CleanupStack::PopAndDestroy();
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSystemLock::SetLockedL() END"));
    #endif // DEBUG
    #endif // WINS
    }
//
// ----------------------------------------------------------
// CSystemLock::RunL()
// Handles query result
// ----------------------------------------------------------
// 
void CSystemLock::RunL()
    {    
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
        		AknNotifierController::HideAllNotifications(ETrue);
        		// query approved -> lock system  
			#ifdef RD_REMOTELOCK
				iProperty.Set(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, EManualLocked);
			#else// !RD_REMOTELOCK
        		iProperty.Set(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, EAutolockOn);
        	#endif//RD_REMOTELOCK
        		AknNotifierController::HideAllNotifications(EFalse);
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
