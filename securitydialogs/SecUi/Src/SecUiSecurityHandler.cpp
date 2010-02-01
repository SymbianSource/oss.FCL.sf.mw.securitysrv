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
* Description:  Provides api for handling security events.
*
*
*/


#include <aknnotedialog.h>
#include <etelmm.h>
#include <SecUi.rsg>
#include <exterror.h>
#include <textresolver.h>

#ifdef __COVER_DISPLAY
#include <aknmediatorfacade.h>
#include <secondarydisplay/SecondaryDisplayStartupAPI.h>
#endif //__COVER_DISPLAY

#include <centralrepository.h> 
#include <starterclient.h>     //used for RemoveSplashScreen
#include <e32property.h>
#include <PSVariables.h>   // Property values
#include <coreapplicationuisdomainpskeys.h>
#include <startupdomainpskeys.h>
#include <uikon/eiksrvui.h>
#include <settingsinternalcrkeys.h>
#include <securityuisprivatepskeys.h>
#include <AknNotiferAppServerApplication.h>

#include <SCPClient.h>
#include <securitynotification.h>
#include "secui.hrh"
#include "secuisecurityhandler.h"
#include "secuicodequerydialog.h"
#include "secuisecuritysettings.h"
#include "SecUiWait.h"
#include "SecUiLockObserver.h"
#ifdef RD_REMOTELOCK
#include <RemoteLockSettings.h>
#endif // RD_REMOTELOCK
#include <StringLoader.h>
#include <featmgr.h>
//  LOCAL CONSTANTS AND MACROS
const TInt KMaxNumberOfPUKAttempts(10);
const TInt KMaxNumberOfPINAttempts(3);
const TInt KLastRemainingInputAttempt(1);

const TInt KTriesToConnectServer( 2 );
const TInt KTimeBeforeRetryingRequest( 50000 );

// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CSecurityHandler::CSecurityHandler()
// C++ constructor
// ----------------------------------------------------------
//
EXPORT_C CSecurityHandler::CSecurityHandler(RMobilePhone& aPhone):
        iPhone(aPhone), iQueryCanceled(ETrue), iSecurityDlg(NULL), iNoteDlg(NULL) 
    {
        TInt result = iCustomPhone.Open(aPhone);
        TRAP_IGNORE( FeatureManager::InitializeLibL() ); //Shouldn't this panic if FM does not initialise??
    }

//
// ----------------------------------------------------------
// CSecurityHandler::~CSecurityHandler()
// Destructor
// ----------------------------------------------------------
//
EXPORT_C CSecurityHandler::~CSecurityHandler()
    {
    #if defined(_DEBUG)
    RDebug::Print(_L("CSecurityHandler::~CSecurityHandler()"));
    #endif
    if ( iDestroyedPtr )
        {
        *iDestroyedPtr = ETrue;
        iDestroyedPtr = NULL;
        }
    CancelSecCodeQuery();
    iCustomPhone.Close();
    FeatureManager::UnInitializeLib();
    }
//
// ----------------------------------------------------------
// CSecurityHandler::HandleEventL()
// Handles different security events
// ----------------------------------------------------------
//
EXPORT_C void CSecurityHandler::HandleEventL(
    RMobilePhone::TMobilePhoneSecurityEvent aEvent )
    {
    TInt result = KErrNone;
    HandleEventL( aEvent, result );
    }

//
// ----------------------------------------------------------
// CSecurityHandler::HandleEventL()
// Handles different security events
// ----------------------------------------------------------
//
EXPORT_C void CSecurityHandler::HandleEventL(
    RMobilePhone::TMobilePhoneSecurityEvent aEvent,
    TBool aStartup, TInt& aResult )
    {
    iStartup = aStartup;
    HandleEventL( aEvent, aResult );
    }
    
//
// ----------------------------------------------------------
// CSecurityHandler::HandleEventL()
// Handles different security events
// ----------------------------------------------------------
//
EXPORT_C void CSecurityHandler::HandleEventL(
    RMobilePhone::TMobilePhoneSecurityEvent aEvent, TInt& aResult )
    {
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecurityHandler::HandleEventL()"));
    RDebug::Print(_L("(SECUI)CSecurityHandler::HandleEventL() EVENT: %d"), aEvent);
    #endif
    TBool wcdmaSupported(FeatureManager::FeatureSupported( KFeatureIdProtocolWcdma ));
    TBool upinSupported(FeatureManager::FeatureSupported( KFeatureIdUpin ));
 
    switch(aEvent)
            {
            case RMobilePhone::EPin1Required:
                #if defined(_DEBUG)
                RDebug::Print(_L("(SECUI)CSecurityHandler::HandleEventL() Pin1Required"));
                #endif
                aResult = Pin1RequiredL();
                break;
            case RMobilePhone::EPuk1Required:
                #if defined(_DEBUG)
                RDebug::Print(_L("(SECUI)CSecurityHandler::HandleEventL() PUK1Required"));
                #endif
                ((CAknNotifierAppServerAppUi*)(CEikonEnv::Static())->EikAppUi())->SuppressAppSwitching(ETrue);
                TRAPD(err,aResult = Puk1RequiredL());
                ((CAknNotifierAppServerAppUi*)(CEikonEnv::Static())->EikAppUi())->SuppressAppSwitching(EFalse);
                User::LeaveIfError(err);
                break;
            case RMobilePhone::EPin2Required:
                Pin2RequiredL();
                break;
            case RMobilePhone::EPuk2Required:
                Puk2RequiredL();        
                break;
            case RMobilePhone::EUniversalPinRequired:
                if(wcdmaSupported || upinSupported)
                   {
                       aResult = UPinRequiredL();
                   }
                else
                    aResult = KErrNotSupported;
                break;
            case RMobilePhone::EUniversalPukRequired:
                if(wcdmaSupported || upinSupported)
                   {
                       aResult = UPukRequiredL();
                   }
                else
                    aResult = KErrNotSupported;
                break;
            case RMobilePhone::EPhonePasswordRequired:
                aResult = PassPhraseRequiredL();
                break;
            case RMobilePhone::EICCTerminated:
                SimLockEventL();
                break;
            default:
                break;
            }
    #if defined(_DEBUG)
    RDebug::Print( _L( "CSecurityHandler::HandleEventL() returning %d." ), aResult );
    #endif
    }
//
// ----------------------------------------------------------
// CSecurityHandler::AskSecCodeL()
// For asking security code e.g in settings
// ----------------------------------------------------------
//
EXPORT_C TBool CSecurityHandler::AskSecCodeL()
    {        
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecurityHandler::AskSecCodeL()"));
    #endif

    /* if code is still not initialized, then there's no need to ask it. This fixes the error when the RFS requests the code */
    const TUid KCRUidSCPLockCode = {0x2002677B};
    const TUint32 KSCPLockCodeDefaultLockCode = 0x00000001;
	
    CRepository* repository = CRepository::NewL(KCRUidSCPLockCode);
    TInt currentLockStatus = -1;
    TInt res=-1;

    res = repository->Get(KSCPLockCodeDefaultLockCode , currentLockStatus);
    #if defined(_DEBUG)
    RDebug::Printf( "%s %s (%u) res=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, res );
    RDebug::Printf( "%s %s (%u) currentLockStatus=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, currentLockStatus );
    #endif
    delete repository;
    if(res==0 && currentLockStatus==1)
        {
        // code is the default one; no need to request it.
        return ETrue;
        }
    /* end check for default code */

    // Destructor sets thisDestroyed to ETrue
    TBool thisDestroyed( EFalse );
    iDestroyedPtr = &thisDestroyed;
    
    iQueryCanceled = EFalse;
    RMobilePhone::TMobilePassword password;
    RMobilePhone::TMobilePassword required_fourth;
        
    TInt ret = KErrNone;
    TInt err = KErrNone;
    TInt status = KErrNone;
    if(FeatureManager::FeatureSupported(KFeatureIdSapTerminalControlFw ) &&
    		FeatureManager::FeatureSupported(KFeatureIdSapDeviceLockEnhancements))
    {
        // Connect to the SCP server, and request the code query
        RSCPClient scpClient;
        User::LeaveIfError( scpClient.Connect() );
        CleanupClosePushL( scpClient );      
        status = scpClient.SecCodeQuery( password, 
                                      RSCPClient::SCP_OK_CANCEL,
                                      EFalse,
                                      0 );         
                                              
        if ( status != KErrCancel )
            {
            // Set this "true" to indicate that the input wasn't cancelled
            ret = ETrue;
            }
        else
            {
            ret = EFalse;
            }
        
        CleanupStack::PopAndDestroy(); //scpClient                       
  }
  else
  {
    iSecurityDlg = new (ELeave) CCodeQueryDialog (password,SEC_C_SECURITY_CODE_MIN_LENGTH,SEC_C_SECURITY_CODE_MAX_LENGTH,ESecUiNone);
    #ifdef __COVER_DISPLAY
    iSecurityDlg->PublishDialogL(SecondaryDisplay::ECmdShowSecurityQuery, SecondaryDisplay::KCatStartup);
    CAknMediatorFacade* covercl = AknMediatorFacade(iSecurityDlg); // uses MOP, so control provided
	if (covercl) // returns null if __COVER_DISPLAY is not defined
    	{
    	// … -  add data that cover ui is interested in
    	covercl->BufStream().WriteInt32L(SecondaryDisplay::EShowSecCode);// adds int to additional data to be posted to cover ui
    	covercl->BufStream().CommitL(); // no more data to send so commit buf
     	}  
    #endif //__COVER_DISPLAY
	    CSecUiLockObserver* deviceLockStatusObserver = CSecUiLockObserver::NewL(iSecurityDlg);
		CleanupStack::PushL(deviceLockStatusObserver);
		err =KErrNone;
	    TRAP(err,ret = iSecurityDlg->ExecuteLD(R_SECURITY_QUERY));
		CleanupStack::PopAndDestroy(deviceLockStatusObserver);
  }
    
    // check if CSecurityHandler has been "killed"
    if ( thisDestroyed )
        {
        return EFalse;
        }

    iDestroyedPtr = NULL;
    iSecurityDlg = NULL;

    if (err != KErrNone)
        {
        User::Leave(err);
        }

    if (ret)
        {
        while (!iQueryCanceled)
            {
	           if (!FeatureManager::FeatureSupported(KFeatureIdSapDeviceLockEnhancements))
		           {           
		                RMobilePhone::TMobilePhoneSecurityCode secCodeType;
		                secCodeType = RMobilePhone::ESecurityCodePhonePassword;
		                CWait* wait = CWait::NewL();
		                iPhone.VerifySecurityCode(wait->iStatus,secCodeType, password, required_fourth);
		                status = wait->WaitForRequestL();
		                delete wait;
		
		           }
            
            switch(status)
                {        
                case KErrNone:
                    {
                    if(FeatureManager::FeatureSupported(KFeatureIdSapTerminalControlFw ) &&
    										!(FeatureManager::FeatureSupported(KFeatureIdSapDeviceLockEnhancements)))
    								{
                        RSCPClient scpClient;
                        User::LeaveIfError( scpClient.Connect() );
                        CleanupClosePushL( scpClient );

                        TSCPSecCode newCode;
                        newCode.Copy( password );
                        scpClient.StoreCode( newCode );

                        CleanupStack::PopAndDestroy(); //scpClient  
                  	}
                            	
                    iQueryCanceled = ETrue;
                    return ETrue;
                    }                    
                case KErrGsmSSPasswordAttemptsViolation:
                case KErrLocked:
                    {
                    iDestroyedPtr = &thisDestroyed;
                    // security code blocked! 
                    iNoteDlg = new (ELeave) CAknNoteDialog(REINTERPRET_CAST(CEikDialog**,&iNoteDlg));
                    iNoteDlg->SetTimeout(CAknNoteDialog::ELongTimeout);
                    iNoteDlg->SetTone(CAknNoteDialog::EErrorTone);
                    err =KErrNone;
                    TRAP(err,iNoteDlg->ExecuteLD(R_SEC_BLOCKED));
                    
                    // check if CSecurityHandler has been "killed"
                    if ( thisDestroyed )
                        {
                        return EFalse;
                        }
                    
                    iDestroyedPtr = NULL;
                    iNoteDlg = NULL;

                    if (err != KErrNone)
                        {
                        User::Leave(err);
                        }
                    break;
                    }
                case KErrGsm0707IncorrectPassword:
                case KErrAccessDenied:
                    {    
                    iDestroyedPtr = &thisDestroyed;
                    // code was entered erroneusly
                    iNoteDlg = new (ELeave) CAknNoteDialog(REINTERPRET_CAST(CEikDialog**,&iNoteDlg));
                    iNoteDlg->SetTimeout(CAknNoteDialog::ELongTimeout);
                    iNoteDlg->SetTone(CAknNoteDialog::EErrorTone);
                    err =KErrNone;
                    TRAP(err,iNoteDlg->ExecuteLD(R_CODE_ERROR));    
                    
                    // check if CSecurityHandler has been "killed"
                    if ( thisDestroyed )
                        {
                        return EFalse;
                        }
                    
                    iDestroyedPtr = NULL;
                    iNoteDlg = NULL;
                    
                    if (err != KErrNone)
                        {
                        User::Leave(err);
                        }
                    break;
                    }    
                default:
                    {
                    iDestroyedPtr = &thisDestroyed;
                    err =KErrNone;
                    TRAP(err,ShowGenericErrorNoteL(status));
                                        
                    // check if CSecurityHandler has been "killed"
                    if ( thisDestroyed )
                        {
                        return EFalse;
                        }
                    
                    iDestroyedPtr = NULL;
                    iNoteDlg = NULL;
                    
                    if (err != KErrNone)
                        {
                        User::Leave(err);
                        }    
                    break;
                    }
                }     
        
            if (iQueryCanceled)
                {
                ret = EFalse;
                break;
                }    
            
            password = _L("");
            iDestroyedPtr = &thisDestroyed;
             if(FeatureManager::FeatureSupported(KFeatureIdSapTerminalControlFw ) &&
    						FeatureManager::FeatureSupported(KFeatureIdSapDeviceLockEnhancements))  
    				{       
                // Connect to the SCP server, and request the code query
                RSCPClient scpClient;
                User::LeaveIfError( scpClient.Connect() );
                CleanupClosePushL( scpClient );
                status = scpClient.SecCodeQuery( password, 
                                      RSCPClient::SCP_OK_CANCEL,
                                      EFalse,
                                      0 ); 
        
                if ( status != KErrCancel )
                    {
                    // Set this "true" to indicate that the input wasn't cancelled
                    ret = ETrue;
                    }
                else
                    {
                    ret = EFalse;
                    }                                
        
                CleanupStack::PopAndDestroy(); //scpClient                              
          }
          else
          {
            iSecurityDlg = new (ELeave) CCodeQueryDialog (password,SEC_C_SECURITY_CODE_MIN_LENGTH,SEC_C_SECURITY_CODE_MAX_LENGTH,ESecUiNone);
            CSecUiLockObserver* deviceLockStatusObserver = CSecUiLockObserver::NewL(iSecurityDlg);
						CleanupStack::PushL(deviceLockStatusObserver);
						err =KErrNone;
            TRAP(err,ret = iSecurityDlg->ExecuteLD(R_SECURITY_QUERY));         
						CleanupStack::PopAndDestroy(deviceLockStatusObserver);
          }
            
            // check if CSecurityHandler has been "killed"
            if ( thisDestroyed )
                {
                return EFalse;
                }
    
            iDestroyedPtr = NULL;
            iSecurityDlg = NULL;
            
            if (err != KErrNone)
                {
                User::Leave(err);
                }

            if (!ret)
                break;
        } // while
    }    // if

    iQueryCanceled = ETrue;
    return ret;
    }
//
// ----------------------------------------------------------
// CSecurityHandler::CancelSecCodeQuery()    
// Cancels PIN2 and security code queries
// ----------------------------------------------------------
//
EXPORT_C void CSecurityHandler::CancelSecCodeQuery()                
    {
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecurityHandler::CancelSecCodeQuery()"));
    #endif
    if (!iQueryCanceled)
        {
        iQueryCanceled = ETrue;
        if (iSecurityDlg != NULL)
            {
            delete iSecurityDlg;
            }
        if (iNoteDlg != NULL)
            {
            delete iNoteDlg;
            }
        iNoteDlg = NULL;
        iSecurityDlg = NULL;
        }
    }
//
// ----------------------------------------------------------
// CSecurityHandler::AskSecCodeInAutoLock()
// for asking security code in autolock
// ----------------------------------------------------------
//
EXPORT_C TBool CSecurityHandler::AskSecCodeInAutoLockL()
    {
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
    
    #ifdef __WINS__
    return ETrue;
    #else
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecurityHandler::AskSecCodeInAutoLockL()"));
    #endif
    TInt res;
    CWait* wait;
        
    RMobilePhone::TMobilePhoneLockSetting lockChange(RMobilePhone::ELockSetDisabled);
    RMobilePhone::TMobilePhoneLock lockType = RMobilePhone::ELockPhoneDevice;
   
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecurityHandler::AskSecCodeInAutoLockL() get autolock period"));
    #endif

    // get autolock period from Central Repository.
    CRepository* repository = CRepository::NewL(KCRUidSecuritySettings);
    TInt period = 0;
    res = repository->Get(KSettingsAutoLockTime, period);
    delete repository;

    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecurityHandler::AskSecCodeInAutoLockL() autolock period:%d"), res);
    #endif
    if (res == KErrNone)
        {
        // disable autolock in Domestic OS side too if autolock period is 0.
        if (period == 0 )
            {
            #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CSecurityHandler::AskSecCodeInAutoLockL() Call SetLockSetting"));
            #endif

#ifdef RD_REMOTELOCK

            // If remote lock is enabled, don't disable the domestic OS device lock
            // since that would render the RemoteLock useless.
            // Instead just re-set the DOS lock to enabled which as a side effect
            // requests the security code from the user.

            TBool remoteLockStatus( EFalse );
            CRemoteLockSettings* remoteLockSettings = CRemoteLockSettings::NewL();

            if ( remoteLockSettings->GetEnabled( remoteLockStatus ) )
                {
                if ( remoteLockStatus )
                    {
                    // Remote lock is enabled
                    #ifdef _DEBUG
                    RDebug::Print( _L( "(SecUi)CSecurityHandler::AskSecCodeInAutoLockL() - RemoteLock is enabled: lockChange = RMobilePhone::ELockSetEnabled" ) );
                    #endif // _DEBUG

                    lockChange = RMobilePhone::ELockSetEnabled;
                    }
                else
                    {
                    // Remote lock is disabled
                    #ifdef _DEBUG
                    RDebug::Print( _L( "(SecUi)CSecurityHandler::AskSecCodeInAutoLockL() - RemoteLock is disabled: lockChange = RMobilePhone::ELockSetDisabled" ) );
                    #endif // _DEBUG

                    lockChange = RMobilePhone::ELockSetDisabled;
                    }
                }
            else
                {
                // Failed to get remote lock status
                #ifdef _DEBUG
                RDebug::Print( _L( "(SecUi)CSecurityHandler::AskSecCodeInAutoLockL() - Failed to get RemoteLock status" ) );
                #endif // _DEBUG
                }

            delete remoteLockSettings;
            remoteLockSettings = NULL;

#else // not defined RD_REMOTELOCK

                lockChange = RMobilePhone::ELockSetDisabled;

#endif // RD_REMOTELOCK

                wait = CWait::NewL();
                iPhone.SetLockSetting(wait->iStatus,lockType,lockChange);
                res = wait->WaitForRequestL();
                delete wait;
                #if defined(_DEBUG)
                RDebug::Print(_L("(SECUI)CSecurityHandler::AskSecCodeInAutoLockL() SetLockSetting RESULT:%d"), res);
                #endif
            }
        else
            {	// ask security code
                #if defined(_DEBUG)
            	RDebug::Print(_L("(SECUI)CSecurityHandler::AskSecCodeInAutoLockL() Ask sec code via notifier"));
            	#endif
                RNotifier codeQueryNotifier;
                User::LeaveIfError(codeQueryNotifier.Connect());
                CWait* wait = CWait::NewL();
                CleanupStack::PushL(wait);
                TInt queryResponse = 0;
                TPckg<TInt> response(queryResponse);
                TSecurityNotificationPckg params;
                params().iEvent = static_cast<TInt>(RMobilePhone::EPhonePasswordRequired);
                params().iStartup = EFalse;
                #if defined(_DEBUG)
    			RDebug::Print(_L("(SECUI)CSecurityHandler::AskSecCodeInAutoLockL() Start Notifier"));
    			#endif
                codeQueryNotifier.StartNotifierAndGetResponse(wait->iStatus, KSecurityNotifierUid,params, response);
                res = wait->WaitForRequestL();
                CleanupStack::PopAndDestroy(); // wait
              	#if defined(_DEBUG)
            	RDebug::Print(_L("(SECUI)CSecurityHandler::AskSecCodeInAutoLockL() results:"));
            	RDebug::Print(_L("(SECUI)CSecurityHandler::AskSecCodeInAutoLockL() res:%d"), res);
            	RDebug::Print(_L("(SECUI)CSecurityHandler::AskSecCodeInAutoLockL() queryResponse:%d"), queryResponse);
            	#endif
            	if(res == KErrNone)
                	res = queryResponse;
            }
        }
    else
        {
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecurityHandler::AskSecCodeInAutoLockL() KERRSOMETHING:Call SetLockSetting"));
        #endif

#ifdef RD_REMOTELOCK

        // If remote lock is enabled, don't disable the domestic OS device lock
        // since that would render the RemoteLock useless.
        // Instead just re-set the DOS lock to enabled which as a side effect
        // requests the security code from the user.

        TBool remoteLockStatus( EFalse );
        CRemoteLockSettings* remoteLockSettings = CRemoteLockSettings::NewL();

        if ( remoteLockSettings->GetEnabled( remoteLockStatus ) )
            {
            if ( remoteLockStatus )
                {
                // Remote lock is enabled
                #ifdef _DEBUG
                RDebug::Print( _L( "(SecUi)CSecurityHandler::AskSecCodeInAutoLockL() - Failed to get AutoLock status and RemoteLock is enabled: lockChange = RMobilePhone::ELockSetEnabled" ) );
                #endif // _DEBUG

                lockChange = RMobilePhone::ELockSetEnabled;
                }
            else
                {
                // Remote lock is disabled
                #ifdef _DEBUG
                RDebug::Print( _L( "(SecUi)CSecurityHandler::AskSecCodeInAutoLockL() - Failed to get AutoLock status and RemoteLock is disabled: lockChange = RMobilePhone::ELockSetDisabled" ) );
                #endif // _DEBUG

                lockChange = RMobilePhone::ELockSetDisabled;
                }
            }
        else
            {
            // Failed to get remote lock status
            #ifdef _DEBUG
            RDebug::Print( _L( "(SecUi)CSecurityHandler::AskSecCodeInAutoLockL() - Failed to get AutoLock status and failed to get RemoteLock status" ) );
            #endif // _DEBUG
            }

        delete remoteLockSettings;
        remoteLockSettings = NULL;

#else // not defined RD_REMOTELOCK

        // could not get the current autolock time... disable autolock in Domestic OS side.
        lockChange = RMobilePhone::ELockSetDisabled;

#endif // RD_REMOTELOCK

        wait = CWait::NewL();
        iPhone.SetLockSetting(wait->iStatus,lockType,lockChange);
        res = wait->WaitForRequestL();
        delete wait;
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecurityHandler::AskSecCodeInAutoLockL() KES: SetLockSetting RESULT:%d"), res);
        #endif
        }
        
    switch (res)
        {
        case KErrNone:
            {
                return ETrue;
            }

        case KErrGsmSSPasswordAttemptsViolation:
        case KErrLocked:
        case KErrGsm0707IncorrectPassword:
        case KErrAccessDenied:
            {
			return AskSecCodeInAutoLockL();
            }
        case KErrAbort:
        case KErrCancel:
            // user pressed "cancel"
            return EFalse;
        default:
            {
            return AskSecCodeInAutoLockL();
            }
        }
#endif // WINS
    }
//
// ----------------------------------------------------------
// CSecurityHandler::PassPhraseRequired()    
// Handles PassPhraseRequired event
// ----------------------------------------------------------
//
TInt CSecurityHandler::PassPhraseRequiredL()
    {
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecurityHandler::PassPhraseRequiredL()"));
    #endif
    TBool StartUp = iStartup;

    RMobilePhone::TMobilePassword password;
    RMobilePhone::TMobilePassword required_fourth;
    TInt status;
    TInt autolockState;
    TInt err( KErrGeneral );
    err = RProperty::Get(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, autolockState);
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecurityHandler::PassPhraseRequiredL() Autolock Status result: %d"), err);
    #endif
    if(!StartUp)
        User::LeaveIfError( err );
TBool isConditionSatisfied = EFalse;
TInt tarmFlag=0;
if(FeatureManager::FeatureSupported(KFeatureIdSapTerminalControlFw ))
  	{		
	TInt tRet = RProperty::Get( KSCPSIDAutolock, SCP_TARM_ADMIN_FLAG_UID, tarmFlag );    
    
    if ( tRet != KErrNone )
        {
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecurityHandler::PassPhraseRequiredL():\
            Warning: failed to get TARM Admin Flag state"));
        #endif
        }
    else
        {
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecurityHandler::PassPhraseRequiredL(): TARM flag: %d"), tarmFlag );
        #endif
        }       
 
    if ( ( StartUp ) || ( tarmFlag & KSCPFlagResyncQuery ) )
	    			isConditionSatisfied = ETrue;  
		}
		else
		{
		    if (StartUp)
		    isConditionSatisfied = ETrue;  
		}
		if (isConditionSatisfied)
        {
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecurityHandler::PassPhraseRequiredL() Dialog 1"));
        #endif
        // Security code at bootup: No "cancel" softkey; Emergency calls enabled.
      if(FeatureManager::FeatureSupported(KFeatureIdSapTerminalControlFw ) &&
    			FeatureManager::FeatureSupported(KFeatureIdSapDeviceLockEnhancements))
    		{                   	    		    		    		
            // Connect to the SCP server, and request the code query
            RSCPClient scpClient;
            User::LeaveIfError( scpClient.Connect() );
            CleanupClosePushL( scpClient );
                
            status = scpClient.SecCodeQuery( password, 
                                             RSCPClient::SCP_OK_ETEL,
                                             ETrue,
                                             KSCPEtelRequest );              
            // Note that SecCodeQuery doesn't indicate the return value from the dialog            
        
            CleanupStack::PopAndDestroy();  //scpClient                                         
      }  
      else
      {     
        CCodeQueryDialog* securityDlg = new (ELeave) CCodeQueryDialog (password,SEC_C_SECURITY_CODE_MIN_LENGTH,SEC_C_SECURITY_CODE_MAX_LENGTH,ESecUiCodeEtelReqest);
        if(AknLayoutUtils::PenEnabled())
            securityDlg->SetEmergencyCallSupportForCBA( ETrue );
        else
            securityDlg->SetEmergencyCallSupport(ETrue);
        #ifdef __COVER_DISPLAY
        securityDlg->PublishDialogL(SecondaryDisplay::ECmdShowSecurityQuery, SecondaryDisplay::KCatStartup);
        CAknMediatorFacade* covercl = AknMediatorFacade(securityDlg); // uses MOP, so control provided 
		if (covercl) // returns null if __COVER_DISPLAY is not defined
    		{
    		// … -  add data that cover ui is interested in
    		covercl->BufStream().WriteInt32L(SecondaryDisplay::EShowSecCode); // adds int to additional data to be posted to cover ui
    		covercl->BufStream().CommitL(); // no more data to send so commit buf
     		}  
        #endif //__COVER_DISPLAY
        status = securityDlg->ExecuteLD(R_SECURITY_REQUEST_QUERY);
			}
        }
    else if ( (autolockState > EAutolockOff))    
        {
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecurityHandler::PassPhraseRequiredL() Dialog 2"));
        #endif
            // Autolock is On. Security event came from user pressing "unlock".
            // Emergency call support must be enabled and there must be a "cancel" softkey.
         if(FeatureManager::FeatureSupported(KFeatureIdSapTerminalControlFw ) &&
    				FeatureManager::FeatureSupported(KFeatureIdSapDeviceLockEnhancements))     
		    		{                                    
     	    // Connect to the SCP server, and request the code query
            RSCPClient scpClient;
            User::LeaveIfError( scpClient.Connect() );
            CleanupClosePushL( scpClient );
              
            status = scpClient.SecCodeQuery( password, 
                                             RSCPClient::SCP_OK_CANCEL,
                                             ETrue,
                                             KSCPEtelRequest );
            // Note that SecCodeQuery doesn't indicate the return value from the dialog   
        
            CleanupStack::PopAndDestroy(); //scpClient
		    		}
			    else
			    {
        CCodeQueryDialog* securityDlg = new (ELeave) CCodeQueryDialog (password,SEC_C_SECURITY_CODE_MIN_LENGTH,SEC_C_SECURITY_CODE_MAX_LENGTH,ESecUiNone);
        if(AknLayoutUtils::PenEnabled())
            securityDlg->SetEmergencyCallSupportForCBA( ETrue );
        else
            securityDlg->SetEmergencyCallSupport(ETrue);
        #ifdef __COVER_DISPLAY
        securityDlg->PublishDialogL(SecondaryDisplay::ECmdShowSecurityQuery, SecondaryDisplay::KCatStartup);
        CAknMediatorFacade* covercl = AknMediatorFacade(securityDlg); // uses MOP, so control provided 
		if (covercl) // returns null if __COVER_DISPLAY is not defined
    	{
    	// … -  add data that cover ui is interested in
    	covercl->BufStream().WriteInt32L(SecondaryDisplay::EShowSecCode); // adds int to additional data to be posted to cover ui
    	covercl->BufStream().CommitL(); // no more data to send so commit buf
     	}  
        #endif //__COVER_DISPLAY
        status = securityDlg->ExecuteLD(R_SECURITY_QUERY);
			     }
        }
    else    
        {
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecurityHandler::PassPhraseRequiredL() Dialog 3"));
        #endif
            // Code query due to a setting change; "Cancel" softkey active;
            // no emergency call support.
	        if(FeatureManager::FeatureSupported(KFeatureIdSapTerminalControlFw ) &&
	    			FeatureManager::FeatureSupported(KFeatureIdSapDeviceLockEnhancements))
	    		{
     	    // Connect to the SCP server, and request the code query
            RSCPClient scpClient;
            User::LeaveIfError( scpClient.Connect() );
            CleanupClosePushL( scpClient );
              
            status = scpClient.SecCodeQuery( password, 
                                             RSCPClient::SCP_OK_CANCEL,
                                             EFalse,
                                             KSCPEtelRequest );            
            // Note that SecCodeQuery doesn't indicate the return value from the dialog   
        
            CleanupStack::PopAndDestroy(); //scpClient
			    }
			    else
			    {
        iSecurityDlg = new (ELeave) CCodeQueryDialog (password,SEC_C_SECURITY_CODE_MIN_LENGTH,SEC_C_SECURITY_CODE_MAX_LENGTH,ESecUiNone);
        #ifdef __COVER_DISPLAY
        iSecurityDlg->PublishDialogL(SecondaryDisplay::ECmdShowSecurityQuery, SecondaryDisplay::KCatStartup);
        CAknMediatorFacade* covercl = AknMediatorFacade(iSecurityDlg); // uses MOP, so control provided 
		if (covercl) // returns null if __COVER_DISPLAY is not defined
    	{
    	covercl->BufStream().WriteInt32L(SecondaryDisplay::EShowSecCode); // adds int to additional data to be posted to cover ui
    	covercl->BufStream().CommitL(); // no more data to send so commit buf
     	}  
        #endif //__COVER_DISPLAY
        // read a flag to see whether the query is SecUi originated.
        TInt secUiOriginatedQuery(ESecurityUIsETelAPIOriginated);
        RProperty::Get(KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery, secUiOriginatedQuery);
        CSecUiLockObserver* deviceLockStatusObserver = CSecUiLockObserver::NewL(iSecurityDlg);
        CleanupStack::PushL(deviceLockStatusObserver);
        CSecUiLockObserver* queryStatusObserver = CSecUiLockObserver::NewL(iSecurityDlg, ESecUiRequestStateObserver);
        CleanupStack::PushL(queryStatusObserver);
        CSecUiLockObserver* callStatusObserver = NULL;
        if(secUiOriginatedQuery == ESecurityUIsSystemLockOriginated)
            {
                callStatusObserver = CSecUiLockObserver::NewL(iSecurityDlg, ESecUiCallStateObserver);
                CleanupStack::PushL(callStatusObserver);
            }
        status = iSecurityDlg->ExecuteLD(R_SECURITY_QUERY);
        
        if(callStatusObserver == NULL)
            CleanupStack::PopAndDestroy(2); //deviceLockStatusObserver, queryStatusObserver
        else
            CleanupStack::PopAndDestroy(3); //deviceLockStatusObserver, queryStatusObserver, callStatusObserver
        iSecurityDlg = NULL;
			      }
        }
TBool isCondition = EFalse;		
if(!FeatureManager::FeatureSupported(KFeatureIdSapDeviceLockEnhancements))
{
    if (!status || (status == ESecUiEmergencyCall)  
        || (status == EAknSoftkeyEmergencyCall) || (status == ESecUiDeviceLocked))
    isCondition = ETrue;
}
else
{
    if  ( ( status == KErrCancel ) || (status == ESecUiEmergencyCall)  ||
         (status == EAknSoftkeyEmergencyCall) || (status == ESecUiDeviceLocked))
    isCondition = ETrue;
}
		if (isCondition)
        {
		#if defined(_DEBUG)
		RDebug::Print(_L("(SECUI)CSecurityHandler::PassPhraseRequiredL() DIALOG ERROR"));
		#endif
        if (!StartUp)
            {
            #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CSecurityHandler::PassPhraseRequiredL() ABORT CALLED!!!!!!"));
            #endif
            iPhone.AbortSecurityCode(RMobilePhone::ESecurityCodePhonePassword);
            }
        return KErrCancel;
        }

    RMobilePhone::TMobilePhoneSecurityCode secCodeType = RMobilePhone::ESecurityCodePhonePassword;
     CWait* wait = NULL;
if(!FeatureManager::FeatureSupported(KFeatureIdSapDeviceLockEnhancements))
{ 
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecurityHandler::PassPhraseRequiredL() VerifySecurityCode"));
        #endif
        wait = CWait::NewL();
        iPhone.VerifySecurityCode(wait->iStatus,secCodeType, password, required_fourth);
        status = wait->WaitForRequestL();
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecurityHandler::PassPhraseRequiredL() VerifySecurityCode STATUS: %d"), status);
        #endif
        delete wait;
  }
  else
  {
		wait = NULL;
  }
    
    TInt returnValue = status;
    switch(status)
        {        
        case KErrNone:
            #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CSecurityHandler::PassPhraseRequiredL() KErrNone"));
            #endif
            // code approved 
            CSecuritySettings::ShowResultNoteL(R_CONFIRMATION_NOTE, CAknNoteDialog::EConfirmationTone);
        if(FeatureManager::FeatureSupported(KFeatureIdSapTerminalControlFw))    
        {
                // Unset the admin flag if set
                if ( tarmFlag & KSCPFlagResyncQuery )
                    {
                    TInt tRet = RProperty::Get( KSCPSIDAutolock, SCP_TARM_ADMIN_FLAG_UID, tarmFlag );
                    
                    if ( tRet == KErrNone )
	                    {
	                    tarmFlag &= ~KSCPFlagResyncQuery;
	                    tRet = RProperty::Set( KSCPSIDAutolock, SCP_TARM_ADMIN_FLAG_UID, tarmFlag );
	                    }
	            
	                if ( tRet != KErrNone )
                        {
                        #if defined(_DEBUG)
                        RDebug::Print(_L("(SECUI)CSecurityHandler::PassPhraseRequiredL():\
                            FAILED to unset TARM Admin Flag"));
                        #endif
                        }                    
                    }                    	            
				        if(!FeatureManager::FeatureSupported(KFeatureIdSapDeviceLockEnhancements)) 
				        {           
    			RSCPClient scpClient;
                User::LeaveIfError( scpClient.Connect() );
                CleanupClosePushL( scpClient );
                TSCPSecCode newCode;
                newCode.Copy( password );
                scpClient.StoreCode( newCode );
                CleanupStack::PopAndDestroy(); //scpClient
                }

          }
        		
            if (StartUp)
                {
                #if defined(_DEBUG)
                RDebug::Print(_L("(SECUI)CSecurityHandler::PassPhraseRequiredL()KErrNone: Startup; get autolock period."));
                #endif

                // get autolock period from Central Repository.
                CRepository* repository = CRepository::NewL(KCRUidSecuritySettings);
                TInt period = 0;
                TInt res = repository->Get(KSettingsAutoLockTime, period);
                delete repository;
  
     						_LIT_SECURITY_POLICY_PASS(KReadPolicy); 
								_LIT_SECURITY_POLICY_C1(KWritePolicy, ECapabilityWriteDeviceData);
    						RProperty::Define(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, RProperty::EInt, KReadPolicy, KWritePolicy);
                RProperty::Set(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, EAutolockOff);
                #if defined(_DEBUG)
								RDebug::Print(_L("(SECUI)CSecurityHandler::PassPhraseRequiredL() EAutolockOff")); 
								#endif																

                if (res == KErrNone)
                    {
                    // disable autolock in Domestic OS side too if autolock period is 0.
                    if (period == 0 )
                        {
#ifdef RD_REMOTELOCK
                        // If remote lock is enabled, don't disable the domestic OS device lock
                        // since that would render the RemoteLock useless.

                        TBool remoteLockStatus( EFalse );
                        CRemoteLockSettings* remoteLockSettings = CRemoteLockSettings::NewL();

                        if ( remoteLockSettings->GetEnabled( remoteLockStatus ) )
                            {
                            if ( !remoteLockStatus )
                                {
                                // Remote lock is disabled
                                #ifdef _DEBUG
                                RDebug::Print( _L( "(SecUi)CSecurityHandler::PassPhraseRequiredL() - Autolock and RemoteLock are disabled -> disable DOS device lock" ) );
                                #endif // _DEBUG

                                // Disable DOS device lock setting
                                wait = CWait::NewL();
                                iCustomPhone.DisablePhoneLock(wait->iStatus,password);
                                wait->WaitForRequestL();
                                delete wait;
                                }
                            }
                        else
                            {
                            // Failed to get remote lock status
                            #ifdef _DEBUG
                            RDebug::Print( _L( "(SecUi)CSecurityHandler::PassPhraseRequiredL() - Autolock is disabled, but failed to get RemoteLock status, so do nothing." ) );
                            #endif // _DEBUG
                            }

                        delete remoteLockSettings;
                        remoteLockSettings = NULL;

#else // not defined RD_REMOTELOCK

                        #if defined(_DEBUG)
                        RDebug::Print(_L("(SECUI)CSecurityHandler::PassPhraseRequiredL()KErrNone: Startup; DisablePhoneLock."));
                        #endif
                        wait = CWait::NewL();
                        iCustomPhone.DisablePhoneLock(wait->iStatus,password);
                        wait->WaitForRequestL();
                        #if defined(_DEBUG)
                        RDebug::Print(_L("(SECUI)CSecurityHandler::PassPhraseRequiredL()KErrNone: Startup; DisablePhoneLock completed."));
                        #endif
                        delete wait;
#endif // RD_REMOTELOCK
                        }
                    }
                else
                    {
#ifdef RD_REMOTELOCK
                    // If remote lock is enabled, don't disable the domestic OS device lock
                    // since that would render the RemoteLock useless.

                    TBool remoteLockStatus( EFalse );
                    CRemoteLockSettings* remoteLockSettings = CRemoteLockSettings::NewL();

                    if ( remoteLockSettings->GetEnabled( remoteLockStatus ) )
                        {
                        if ( !remoteLockStatus )
                            {
                            // Remote lock is disabled
                            #ifdef _DEBUG
                            RDebug::Print( _L( "(SecUi)CSecurityHandler::PassPhraseRequiredL() - Failed to get Autolock period and RemoteLock is disabled -> disable DOS device lock" ) );
                            #endif // _DEBUG

                            wait = CWait::NewL();
                            iCustomPhone.DisablePhoneLock(wait->iStatus,password);
                            wait->WaitForRequestL();
                            delete wait;
                            }
                        }
                    else
                        {
                        // Failed to get remote lock status
                        #ifdef _DEBUG
                        RDebug::Print( _L( "(SecUi)CSecurityHandler::PassPhraseRequiredL() - Failed to get Autolock period and RemoteLock status, so do nothing." ) );
                        #endif // _DEBUG
                        }

                    delete remoteLockSettings;
                    remoteLockSettings = NULL;

#else // not defined RD_REMOTELOCK

                    #if defined(_DEBUG)
                    RDebug::Print(_L("(SECUI)CSecurityHandler::PassPhraseRequiredL()KErrNone: Startup; Could not get autolock period."));
                    #endif
                    // could not get the current autolock time... disable autolock in Domestic OS side. 
                    wait = CWait::NewL();
                    iCustomPhone.DisablePhoneLock(wait->iStatus,password);
                    wait->WaitForRequestL();
                    #if defined(_DEBUG)
                    RDebug::Print(_L("(SECUI)CSecurityHandler::PassPhraseRequiredL()KErrNone: Startup; NO AUTOLOCK PERIOD; DisablePhoneLock completed."));
                    #endif
                    delete wait;

#endif // RD_REMOTELOCK
                    }
                
                }

            break;    
        case KErrGsmSSPasswordAttemptsViolation:
        case KErrLocked:
            // security code blocked!
            #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CSecurityHandler::PassPhraseRequiredL() ErrGsmSSPasswordAttemptsViolation"));
            #endif
            CSecuritySettings::ShowResultNoteL(R_SEC_BLOCKED, CAknNoteDialog::EErrorTone);
            break;
        case KErrGsm0707IncorrectPassword:
        case KErrAccessDenied:
            #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CSecurityHandler::PassPhraseRequiredL() KErrGsm0707IncorrectPassword"));
            #endif
            CSecuritySettings::ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
            break;
        default:
            #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CSecurityHandler::PassPhraseRequiredL() DEFAULT"));
            #endif
            CSecuritySettings::ShowErrorNoteL(status);
            break;
        }
        
    return returnValue;
    }
//
// ----------------------------------------------------------
// CSecurityHandler::Pin1Required()    
// Handles Pin1Required event
// ----------------------------------------------------------
//
TInt CSecurityHandler::Pin1RequiredL()
    {
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
    
    RMobilePhone::TMobilePassword password;
    RMobilePhone::TMobilePassword required_fourth;
    RMobilePhone::TMobilePhoneSecurityCode secCodeType = RMobilePhone::ESecurityCodePin1;
    RMobilePhone::TMobilePhoneSecurityCodeInfoV5 codeInfo;
    RMobilePhone::TMobilePhoneSecurityCodeInfoV5Pckg codeInfoPkg(codeInfo);
    TBool StartUp = ETrue;   
    TInt secUiOriginatedQuery(ESecurityUIsSecUIOriginatedUninitialized);
    TInt err = KErrNone;
    TInt res = KErrGeneral;
    CWait* wait = CWait::NewL();
    CleanupStack::PushL(wait);
    

    StartUp = iStartup;

    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecurityHandler::Pin1RequiredL()"));
    #endif

    if(!StartUp)
    {
        // read a flag to see whether the query is SecUi originated.
        err = RProperty::Get(KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery, secUiOriginatedQuery);
        
    if ( err != KErrNone )
        {
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecurityHandler::Pin1RequiredL():\
            FAILED to get the SECUI query Flag: %d"), err);
        #endif
        }
    else
            {
               #if defined(_DEBUG)
                RDebug::Print(_L("(SECUI)CSecurityHandler::Pin1RequiredL():\
                    SECUI query Flag: %d"), secUiOriginatedQuery);
                #endif 
            }
    }  
    #if defined(_DEBUG)
    RDebug::Print(_L("CSecurityHandler::Pin1RequiredL() Execute dlg"));
    #endif

    if(StartUp || (secUiOriginatedQuery != ESecurityUIsSecUIOriginated) || (err != KErrNone))
    {	
        iSecurityDlg = new (ELeave) CCodeQueryDialog (password,SEC_C_PIN_CODE_MIN_LENGTH,SEC_C_PIN_CODE_MAX_LENGTH,ESecUiCodeEtelReqest);
        if(AknLayoutUtils::PenEnabled())
            iSecurityDlg->SetEmergencyCallSupportForCBA( ETrue );
        else
            iSecurityDlg->SetEmergencyCallSupport(ETrue);
        #ifdef __COVER_DISPLAY
        iSecurityDlg->PublishDialogL(SecondaryDisplay::ECmdShowSecurityQuery, SecondaryDisplay::KCatStartup);
        CAknMediatorFacade* covercl = AknMediatorFacade(iSecurityDlg); // uses MOP, so control provided 
		if (covercl) // returns null if __COVER_DISPLAY is not defined
    	{
    	// … -  add data that cover ui is interested in
    	covercl->BufStream().WriteInt32L(SecondaryDisplay::EShowPIN1); // adds int to additional data to be posted to cover ui
    	covercl->BufStream().CommitL(); // no more data to send so commit buf
     	}  
        #endif //__COVER_DISPLAY

        wait->SetRequestType(EMobilePhoneGetSecurityCodeInfo);
        iPhone.GetSecurityCodeInfo(wait->iStatus, secCodeType, codeInfoPkg);
        res = wait->WaitForRequestL();
        #if defined(_DEBUG)
        TInt attempts(codeInfo.iRemainingEntryAttempts);
        RDebug::Print(_L("CSecurityHandler::Pin1RequiredL() Remaining Attempts query status: %d"), res);
        RDebug::Print(_L("CSecurityHandler::Pin1RequiredL() Remaining Attempts: %d"), attempts);
        #endif
        User::LeaveIfError(res);
        
        if(codeInfo.iRemainingEntryAttempts == KMaxNumberOfPINAttempts)
            res = iSecurityDlg->ExecuteLD(R_PIN_REQUEST_QUERY);
        else if(codeInfo.iRemainingEntryAttempts > KLastRemainingInputAttempt)
            {
                HBufC* queryPrompt = StringLoader::LoadLC(R_SECUI_REMAINING_PIN_ATTEMPTS, codeInfo.iRemainingEntryAttempts);
                res = iSecurityDlg->ExecuteLD(R_PIN_REQUEST_QUERY, *queryPrompt);
                CleanupStack::PopAndDestroy(queryPrompt);
            }
        else
            {
                HBufC* queryPrompt = StringLoader::LoadLC(R_SECUI_FINAL_PIN_ATTEMPT);
                res = iSecurityDlg->ExecuteLD(R_PIN_REQUEST_QUERY, *queryPrompt);
                CleanupStack::PopAndDestroy(queryPrompt);   
            }
        
        iSecurityDlg = NULL;
        #if defined(_DEBUG)
    	RDebug::Print(_L("CSecurityHandler::Pin1RequiredL() Execute dlg RESULT: %d"), res);
    	#endif
        if ((!res) || (res == ESecUiEmergencyCall) || (res == EAknSoftkeyEmergencyCall))
            { 
            #if defined(_DEBUG)
            RDebug::Print(_L("CSecurityHandler::Pin1RequiredL() R_PIN_REQUEST_QUERY CANCEL!"));
            #endif
            CleanupStack::PopAndDestroy(wait);   
            return KErrCancel;
            }
    }
    else
    {	
        iSecurityDlg = new (ELeave) CCodeQueryDialog (password,SEC_C_PIN_CODE_MIN_LENGTH,SEC_C_PIN_CODE_MAX_LENGTH,ESecUiNone);
        #ifdef __COVER_DISPLAY
        iSecurityDlg->PublishDialogL(SecondaryDisplay::ECmdShowSecurityQuery, SecondaryDisplay::KCatStartup);
        CAknMediatorFacade* covercl = AknMediatorFacade(iSecurityDlg); // uses MOP, so control provided
		if (covercl) // returns null if __COVER_DISPLAY is not defined
    		{
    		// … -  add data that cover ui is interested in
    		covercl->BufStream().WriteInt32L(SecondaryDisplay::EShowPIN1); // adds int to additional data to be posted to cover ui
    		covercl->BufStream().CommitL(); // no more data to send so commit buf
     		}  
        #endif //__COVER_DISPLAY
        
        wait->SetRequestType(EMobilePhoneGetSecurityCodeInfo);
        iPhone.GetSecurityCodeInfo(wait->iStatus, secCodeType, codeInfoPkg);
        res = wait->WaitForRequestL();
        User::LeaveIfError(res);
        
        CSecUiLockObserver* deviceLockStatusObserver = CSecUiLockObserver::NewL(iSecurityDlg);
        CleanupStack::PushL(deviceLockStatusObserver);
        CSecUiLockObserver* queryStatusObserver = CSecUiLockObserver::NewL(iSecurityDlg, ESecUiRequestStateObserver);
        CleanupStack::PushL(queryStatusObserver);
        
        if(codeInfo.iRemainingEntryAttempts == KMaxNumberOfPINAttempts)
            res = iSecurityDlg->ExecuteLD(R_PIN_QUERY);
        else if(codeInfo.iRemainingEntryAttempts > KLastRemainingInputAttempt)
            {
                HBufC* queryPrompt = StringLoader::LoadLC(R_SECUI_REMAINING_PIN_ATTEMPTS, codeInfo.iRemainingEntryAttempts );
                res = iSecurityDlg->ExecuteLD(R_PIN_QUERY, *queryPrompt);
                CleanupStack::PopAndDestroy(queryPrompt);
            }
        else
            {
                HBufC* queryPrompt = StringLoader::LoadLC(R_SECUI_FINAL_PIN_ATTEMPT);
                res = iSecurityDlg->ExecuteLD(R_PIN_QUERY, *queryPrompt);
                CleanupStack::PopAndDestroy(queryPrompt);   
            }
        
        CleanupStack::PopAndDestroy(2); //deviceLockStatusObserver, queryStatusObserver
        iSecurityDlg = NULL;
        if( !res || (res == ESecUiDeviceLocked))
            {
            #if defined(_DEBUG)
            RDebug::Print(_L("CSecurityHandler::Pin1RequiredL() R_PIN_QUERY cancel!"));
            #endif 
            // cancel code request
            iPhone.AbortSecurityCode(RMobilePhone::ESecurityCodePin1);
            CleanupStack::PopAndDestroy(wait);
            return KErrCancel;
            }      
    }
    #if defined(_DEBUG)
    RDebug::Print(_L("CSecurityNotifier::Pin1RequiredL()VerifySecurityCode"));
    #endif
    iPhone.VerifySecurityCode(wait->iStatus,secCodeType, password, required_fourth);
    res = wait->WaitForRequestL();
    CleanupStack::PopAndDestroy(wait); 
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecurityHandler::Pin1RequiredL() VerifySecurityCode STATUS: %d"), res);
    #endif
    TInt returnValue = res;
    switch(res)
        {        
        case KErrNone:
            // code approved 
            #if defined(_DEBUG)
            RDebug::Print(_L("CSecurityHandler::Pin1RequiredL()code approved "));
            #endif
            CSecuritySettings::ShowResultNoteL(R_CONFIRMATION_NOTE, CAknNoteDialog::EConfirmationTone);
            break;
        case KErrGsm0707IncorrectPassword:
        case KErrAccessDenied:
            // code was entered erroneously
            CSecuritySettings::ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
            if(StartUp)
            {
            returnValue = Pin1RequiredL();
            }
            break;
        case KErrGsmSSPasswordAttemptsViolation:
        case KErrLocked:
            // code blocked; show error note and terminate.
            // code blocked
            if(StartUp)
                CSecuritySettings::ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone); 
            break;
        case KErrGsm0707SimWrong:
            // sim lock active
            break;
        default:
            CSecuritySettings::ShowErrorNoteL(res);        
            if(StartUp)
            {        
            returnValue = Pin1RequiredL();
            }
            break;
        }
    return returnValue;
    }
//
// ----------------------------------------------------------
// CSecurityHandler::Puk1Required()
// Handles Puk1Required event
// ----------------------------------------------------------
//
TInt CSecurityHandler::Puk1RequiredL()
    {
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecurityHandler::Puk1RequiredL()"));
    #endif            
    RMobilePhone::TMobilePassword aPassword;
    RMobilePhone::TMobilePassword aNewPassword;
    RMobilePhone::TMobilePhoneSecurityCodeInfoV5 codeInfo;
    RMobilePhone::TMobilePhoneSecurityCodeInfoV5Pckg codeInfoPkg(codeInfo);
    RMobilePhone::TMobilePhoneSecurityCode blockCodeType;
    blockCodeType = RMobilePhone::ESecurityCodePuk1;
    CWait* wait = CWait::NewL();
    CleanupStack::PushL(wait);
    
    TBool StartUp(ETrue);
    StartUp = iStartup;

    TInt res(KErrNone);
    wait->SetRequestType(EMobilePhoneGetSecurityCodeInfo);
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecurityHandler::Puk1RequiredL(): Get Code info"));
    #endif
    iPhone.GetSecurityCodeInfo(wait->iStatus, blockCodeType, codeInfoPkg);
    res = wait->WaitForRequestL();
    
    TInt thisTry = 0;

	// If there was a problem (as there might be in case we're dropping off SIM Access Profile); try again a couple of times.
	while ( res != KErrNone && ( thisTry++ ) <= KTriesToConnectServer )
        {
        User::After( KTimeBeforeRetryingRequest );
        iPhone.GetSecurityCodeInfo(wait->iStatus, blockCodeType, codeInfoPkg);
        res = wait->WaitForRequestL();
        }
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecurityHandler::Puk1RequiredL(): Get Code info result: %d"), res);
    #endif
    //If there's still an error we're doomed. Bail out.
    User::LeaveIfError(res);
    
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecurityHandler::Puk1RequiredL(): Show last note"));
    #endif
    //show the last "Code Error" note of PIN verify result here so it won't be left under the PUK1 dialog
    if(!StartUp && (codeInfo.iRemainingEntryAttempts == KMaxNumberOfPUKAttempts))
        CSecuritySettings::ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
    
    // ask PUK code
    iSecurityDlg = new (ELeave) CCodeQueryDialog (aPassword,SEC_C_PUK_CODE_MIN_LENGTH,SEC_C_PUK_CODE_MAX_LENGTH,ESecUiPukRequired);
    if(AknLayoutUtils::PenEnabled())
        iSecurityDlg->SetEmergencyCallSupportForCBA( ETrue );
    else
        iSecurityDlg->SetEmergencyCallSupport(ETrue);
    #ifdef __COVER_DISPLAY
    iSecurityDlg->PublishDialogL(SecondaryDisplay::ECmdShowSecurityQuery, SecondaryDisplay::KCatStartup);
    CAknMediatorFacade* covercl = AknMediatorFacade(iSecurityDlg); // uses MOP, so control provided
	if (covercl) // returns null if __COVER_DISPLAY is not defined
    	{
    	// … -  add data that cover ui is interested in
    	covercl->BufStream().WriteInt32L(SecondaryDisplay::EShowPUK1); // adds int to additional data to be posted to cover ui
    	covercl->BufStream().CommitL(); // no more data to send so commit buf
     	}  
    #endif //__COVER_DISPLAY   
    
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecurityHandler::Puk1RequiredL(): Show dialog"));
    #endif    
    if(codeInfo.iRemainingEntryAttempts == KMaxNumberOfPUKAttempts)
            res = iSecurityDlg->ExecuteLD(R_PUK_REQUEST_QUERY);
    else if(codeInfo.iRemainingEntryAttempts > KLastRemainingInputAttempt)
       {
         HBufC* queryPrompt = StringLoader::LoadLC(R_SECUI_REMAINING_PUK_ATTEMPTS, codeInfo.iRemainingEntryAttempts);
         res = iSecurityDlg->ExecuteLD(R_PUK_REQUEST_QUERY, *queryPrompt);
         CleanupStack::PopAndDestroy(queryPrompt);
       }
    else
       {
         HBufC* queryPrompt = StringLoader::LoadLC(R_SECUI_FINAL_PUK_ATTEMPT);
         res = iSecurityDlg->ExecuteLD(R_PUK_REQUEST_QUERY, *queryPrompt);
         CleanupStack::PopAndDestroy(queryPrompt);   
       }
    
    if((!res) || (res == ESecUiEmergencyCall) || (res == EAknSoftkeyEmergencyCall))
        {
        CleanupStack::PopAndDestroy(wait);
        return KErrCancel;
        }
        
    RMobilePhone::TMobilePassword verifcationPassword;
    // new pin code query
    iSecurityDlg = new (ELeave) CCodeQueryDialog (aNewPassword,SEC_C_PIN_CODE_MIN_LENGTH,SEC_C_PIN_CODE_MAX_LENGTH,ESecUiPukRequired);
    if(AknLayoutUtils::PenEnabled())
        iSecurityDlg->SetEmergencyCallSupportForCBA( ETrue );
    else
        iSecurityDlg->SetEmergencyCallSupport(ETrue);
    res = iSecurityDlg->ExecuteLD(R_NEW_PIN_CODE_REQUEST_QUERY);
    if((!res) || (res == ESecUiEmergencyCall) || (res == EAknSoftkeyEmergencyCall))
        {
        CleanupStack::PopAndDestroy(wait);    
        return KErrCancel;
        }
  
    // verification code query
    iSecurityDlg = new (ELeave) CCodeQueryDialog (verifcationPassword,SEC_C_PIN_CODE_MIN_LENGTH,SEC_C_PIN_CODE_MAX_LENGTH,ESecUiPukRequired);
    if(AknLayoutUtils::PenEnabled())
        iSecurityDlg->SetEmergencyCallSupportForCBA( ETrue );
    else
        iSecurityDlg->SetEmergencyCallSupport(ETrue);
    res = iSecurityDlg->ExecuteLD(R_VERIFY_NEW_PIN_CODE_REQUEST_QUERY);
    if((!res) || (res == ESecUiEmergencyCall) || (res == EAknSoftkeyEmergencyCall))
        {
        CleanupStack::PopAndDestroy(wait);
        return KErrCancel;
        }
                            
    while (aNewPassword.CompareF(verifcationPassword) != 0) 
        {
        // codes do not match -> note -> ask new pin and verification codes again  
        CSecuritySettings::ShowResultNoteL(R_CODES_DONT_MATCH, CAknNoteDialog::EErrorTone);
        
        verifcationPassword = _L("");
        aNewPassword = _L("");

        // new pin code query
        iSecurityDlg = new (ELeave) CCodeQueryDialog (aNewPassword,SEC_C_PIN_CODE_MIN_LENGTH,SEC_C_PIN_CODE_MAX_LENGTH,ESecUiPukRequired);
        if(AknLayoutUtils::PenEnabled())
            iSecurityDlg->SetEmergencyCallSupportForCBA( ETrue );
        else
            iSecurityDlg->SetEmergencyCallSupport(ETrue);
        res = iSecurityDlg->ExecuteLD(R_NEW_PIN_CODE_REQUEST_QUERY);
        if ((!res) || (res == ESecUiEmergencyCall) || (res == EAknSoftkeyEmergencyCall))
            {
            CleanupStack::PopAndDestroy(wait);
            return KErrCancel;
            }
                
        // verification code query
        iSecurityDlg = new (ELeave) CCodeQueryDialog (verifcationPassword,SEC_C_PIN_CODE_MIN_LENGTH,SEC_C_PIN_CODE_MAX_LENGTH,ESecUiPukRequired);
        if(AknLayoutUtils::PenEnabled())
            iSecurityDlg->SetEmergencyCallSupportForCBA( ETrue );
        else
            iSecurityDlg->SetEmergencyCallSupport(ETrue);
        res = iSecurityDlg->ExecuteLD(R_VERIFY_NEW_PIN_CODE_REQUEST_QUERY);
    	if((!res) || (res == ESecUiEmergencyCall) || (res == EAknSoftkeyEmergencyCall))
            {
            CleanupStack::PopAndDestroy(wait);
            return KErrCancel;
            }
        }            
        
    // send code
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecurityHandler::Puk1RequiredL(): Verify Code"));
    #endif
    iPhone.VerifySecurityCode(wait->iStatus,blockCodeType,aNewPassword,aPassword);
    res = wait->WaitForRequestL();
    CleanupStack::PopAndDestroy(wait);
    
    TInt returnValue = res;
    switch(res)
        {
        case KErrNone:
            // code approved -> note
            CSecuritySettings::ShowResultNoteL(R_PIN_CODE_CHANGED_NOTE, CAknNoteDialog::EConfirmationTone);
            break;
           case KErrGsm0707IncorrectPassword:
        case KErrAccessDenied:
            // wrong PUK code -> note -> ask PUK code again        
            CSecuritySettings::ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
            returnValue = Puk1RequiredL();
            break;
        case KErrGsm0707SimWrong:
            // sim lock active
            break;
        case KErrGsmSSPasswordAttemptsViolation:
        case KErrLocked:
            // sim card rejected.
            break;
        default:
            CSecuritySettings::ShowErrorNoteL(res);        
            returnValue = Puk1RequiredL();
            break;
        }    

        return returnValue;
    }
//
// ----------------------------------------------------------
// CSecurityHandler::Pin2Required()
// Handles Pin2Required event
// ----------------------------------------------------------
//    
void CSecurityHandler::Pin2RequiredL()
    {
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecurityHandler::Pin2RequiredL() BEGIN"));
    #endif
    RMobilePhone::TMobilePassword password;
    RMobilePhone::TMobilePassword required_fourth;
    RMobilePhone::TMobilePhoneSecurityCode secCodeType(RMobilePhone::ESecurityCodePin2);
    RMobilePhone::TMobilePhoneSecurityCodeInfoV5 codeInfo;
    RMobilePhone::TMobilePhoneSecurityCodeInfoV5Pckg codeInfoPkg(codeInfo);
    CWait* wait = CWait::NewL();
    CleanupStack::PushL(wait);
    
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecurityHandler::Pin2RequiredL(): create dialog"));
    #endif
    iSecurityDlg = new (ELeave) CCodeQueryDialog (password,SEC_C_PIN2_CODE_MIN_LENGTH,SEC_C_PIN2_CODE_MAX_LENGTH,ESecUiNone);
    #ifdef __COVER_DISPLAY
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecurityHandler::Pin2RequiredL(): publish dialog"));
    #endif
    iSecurityDlg->PublishDialogL(SecondaryDisplay::ECmdShowSecurityQuery, SecondaryDisplay::KCatStartup);
    CAknMediatorFacade* covercl = AknMediatorFacade(iSecurityDlg); // uses MOP, so control provided 
	if (covercl) // returns null if __COVER_DISPLAY is not defined
    	{
    	// … -  add data that cover ui is interested in
    	covercl->BufStream().WriteInt32L(SecondaryDisplay::EShowPIN2); // adds int to additional data to be posted to cover ui
    	covercl->BufStream().CommitL(); // no more data to send so commit buf
     	}  
    #endif //__COVER_DISPLAY
    
	#if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecurityHandler::Pin2RequiredL(): get PIN2 info"));
    #endif
    
	wait->SetRequestType(EMobilePhoneGetSecurityCodeInfo);
    iPhone.GetSecurityCodeInfo(wait->iStatus, secCodeType, codeInfoPkg);
    TInt ret = wait->WaitForRequestL();
        
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecurityHandler::Pin2RequiredL(): get PIN2 info result: %d"), ret);
    TInt attempts(codeInfo.iRemainingEntryAttempts);
    RDebug::Print(_L("(SECUI)CSecurityHandler::Pin2RequiredL(): attempts remaining: %d"), attempts);
    #endif
    User::LeaveIfError(ret);
    
        CSecUiLockObserver* deviceLockStatusObserver = CSecUiLockObserver::NewL(iSecurityDlg);
	    CleanupStack::PushL(deviceLockStatusObserver);
        CSecUiLockObserver* queryStatusObserver = CSecUiLockObserver::NewL(iSecurityDlg, ESecUiRequestStateObserver);
        CleanupStack::PushL(queryStatusObserver);
        
        if(codeInfo.iRemainingEntryAttempts == KMaxNumberOfPINAttempts)
            ret = iSecurityDlg->ExecuteLD(R_PIN2_QUERY);
        else if(codeInfo.iRemainingEntryAttempts > KLastRemainingInputAttempt)
            {
                HBufC* queryPrompt = StringLoader::LoadLC(R_SECUI_REMAINING_PIN2_ATTEMPTS, codeInfo.iRemainingEntryAttempts );
                ret = iSecurityDlg->ExecuteLD(R_PIN2_QUERY, *queryPrompt);
                CleanupStack::PopAndDestroy(queryPrompt);
            }
        else
            {
                HBufC* queryPrompt = StringLoader::LoadLC(R_SECUI_FINAL_PIN2_ATTEMPT);
                ret = iSecurityDlg->ExecuteLD(R_PIN2_QUERY, *queryPrompt);
                CleanupStack::PopAndDestroy(queryPrompt);   
            }
    CleanupStack::PopAndDestroy(2); //deviceLockStatusObserver, queryStatusObserver
    iSecurityDlg = NULL;
    if (!ret  || (ret == ESecUiDeviceLocked))
        {
        iPhone.AbortSecurityCode(RMobilePhone::ESecurityCodePin2);
        CleanupStack::PopAndDestroy(wait);
        return;
        }

    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecurityHandler::Pin2RequiredL(): Verify Code"));
    #endif
    iPhone.VerifySecurityCode(wait->iStatus,secCodeType,password,required_fourth);
    TInt status = wait->WaitForRequestL();
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecurityHandler::Pin2RequiredL(): destroy wait"));
    #endif
    CleanupStack::PopAndDestroy(wait);

    switch(status)
        {        
        case KErrNone:
            break;
        case KErrGsm0707IncorrectPassword:
        case KErrAccessDenied:
            // code was entered erroneously
            CSecuritySettings::ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);       
            break;
        case KErrGsmSSPasswordAttemptsViolation:
        case KErrLocked:
            // blocked
            CSecuritySettings::ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
            break;
        default:
            CSecuritySettings::ShowErrorNoteL(status);        
            break;
        }
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecurityHandler::Pin2RequiredL(): END"));
    #endif
    }
//
// ----------------------------------------------------------
// CSecurityHandler::Puk2Required()
// Handles Puk2Required event
// ----------------------------------------------------------
//    
void CSecurityHandler::Puk2RequiredL()
    {    
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
    RMobilePhone::TMobilePassword aPassword;
    RMobilePhone::TMobilePassword aNewPassword;
    RMobilePhone::TMobilePassword verifcationPassword;
    RMobilePhone::TMobilePhoneSecurityCodeInfoV5 codeInfo;
    RMobilePhone::TMobilePhoneSecurityCodeInfoV5Pckg codeInfoPkg(codeInfo);
    
    RMobilePhone::TMobilePhoneSecurityCode blockCodeType;
    blockCodeType = RMobilePhone::ESecurityCodePuk2;
    CWait* wait = CWait::NewL();
    CleanupStack::PushL(wait);
    
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecurityHandler::Puk2RequiredL()"));
    #endif
    // ask PUK2
    iSecurityDlg = new (ELeave) CCodeQueryDialog (aPassword,SEC_C_PUK2_CODE_MIN_LENGTH,SEC_C_PUK2_CODE_MAX_LENGTH,ESecUiNone);
    #ifdef __COVER_DISPLAY
    iSecurityDlg->PublishDialogL(SecondaryDisplay::ECmdShowSecurityQuery, SecondaryDisplay::KCatStartup);
    CAknMediatorFacade* covercl = AknMediatorFacade(iSecurityDlg); // uses MOP, so control provided 
	if (covercl) // returns null if __COVER_DISPLAY is not defined
    	{
    	// … -  add data that cover ui is interested in
    	covercl->BufStream().WriteInt32L(SecondaryDisplay::EShowPUK2); // adds int to additional data to be posted to cover ui
    	covercl->BufStream().CommitL(); // no more data to send so commit buf
     	}  
    #endif //__COVER_DISPLAY
    CSecUiLockObserver* deviceLockStatusObserver = CSecUiLockObserver::NewL(iSecurityDlg);
	CleanupStack::PushL(deviceLockStatusObserver);
	
	TInt ret(KErrNone);
    wait->SetRequestType(EMobilePhoneGetSecurityCodeInfo);
    iPhone.GetSecurityCodeInfo(wait->iStatus, blockCodeType, codeInfoPkg);
    ret = wait->WaitForRequestL();
    User::LeaveIfError(ret);
        
    if(codeInfo.iRemainingEntryAttempts == KMaxNumberOfPUKAttempts)
            ret = iSecurityDlg->ExecuteLD(R_PUK2_REQUEST_QUERY);
    else if(codeInfo.iRemainingEntryAttempts > KLastRemainingInputAttempt)
       {
         HBufC* queryPrompt = StringLoader::LoadLC(R_SECUI_REMAINING_PUK2_ATTEMPTS, codeInfo.iRemainingEntryAttempts);
         ret = iSecurityDlg->ExecuteLD(R_PUK2_REQUEST_QUERY, *queryPrompt);
         CleanupStack::PopAndDestroy(queryPrompt);
       }
    else
       {
         HBufC* queryPrompt = StringLoader::LoadLC(R_SECUI_FINAL_PUK2_ATTEMPT);
         ret = iSecurityDlg->ExecuteLD(R_PUK2_REQUEST_QUERY, *queryPrompt);
         CleanupStack::PopAndDestroy(queryPrompt);   
       }
	
	iSecurityDlg = NULL;
    if(!ret  || (ret == ESecUiDeviceLocked))
        {
        #if defined(_DEBUG)
    	RDebug::Print(_L("(SECUI)CSecurityHandler::Puk2RequiredL() PUK QUERY CANCEL"));
    	#endif    
        // cancel "get security unblock code" request
        iPhone.AbortSecurityCode(blockCodeType);
		CleanupStack::PopAndDestroy(2); //wait, deviceLockStatusObserver
        return;
        }
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecurityHandler::Puk2RequiredL() NEW QUERY"));
    #endif
    // new pin2 code query
    iSecurityDlg = new (ELeave) CCodeQueryDialog (aNewPassword,SEC_C_PIN2_CODE_MIN_LENGTH,SEC_C_PIN2_CODE_MAX_LENGTH,ESecUiNone);
    deviceLockStatusObserver->SetAddress(iSecurityDlg);
    ret = iSecurityDlg->ExecuteLD(R_NEW_PIN2_CODE_QUERY);
    if(!ret  || (ret == ESecUiDeviceLocked))
        {
        #if defined(_DEBUG)
    	RDebug::Print(_L("(SECUI)CSecurityHandler::Puk2RequiredL() NEW QUERY CANCEL"));
    	#endif 
        // cancel "get security unblock code" request
        iPhone.AbortSecurityCode(blockCodeType);
        CleanupStack::PopAndDestroy(2); //wait, deviceLockStatusObserver
        return;
        }

     // verification code query
    iSecurityDlg = new (ELeave) CCodeQueryDialog (verifcationPassword,SEC_C_PIN_CODE_MIN_LENGTH,SEC_C_PIN_CODE_MAX_LENGTH,ESecUiNone);
    deviceLockStatusObserver->SetAddress(iSecurityDlg);
    ret = iSecurityDlg->ExecuteLD(R_VERIFY_NEW_PIN2_CODE_QUERY);
    if (!ret || (ret == ESecUiDeviceLocked))    
        {
        #if defined(_DEBUG)
    	RDebug::Print(_L("(SECUI)CSecurityHandler::Puk2RequiredL() VERIFY QUERY CANCEL"));
    	#endif 
        // cancel "get security unblock code" request
        iPhone.AbortSecurityCode(blockCodeType);
        CleanupStack::PopAndDestroy(2); //wait, deviceLockStatusObserver
        return;
        }
        
    while (aNewPassword.CompareF(verifcationPassword) != 0) 
        {
        // codes do not match -> note -> ask new pin and verification codes again  
        CSecuritySettings::ShowResultNoteL(R_CODES_DONT_MATCH, CAknNoteDialog::EErrorTone);
        
        verifcationPassword = _L("");
        aNewPassword = _L("");

        // new pin2 code query
        iSecurityDlg = new (ELeave) CCodeQueryDialog (aNewPassword,SEC_C_PIN2_CODE_MIN_LENGTH,SEC_C_PIN2_CODE_MAX_LENGTH,ESecUiNone);
        deviceLockStatusObserver->SetAddress(iSecurityDlg);
        deviceLockStatusObserver->StartObserver();
        
        ret = iSecurityDlg->ExecuteLD(R_NEW_PIN2_CODE_QUERY);
    
        if(!ret || (ret == ESecUiDeviceLocked))
            {
            // cancel "get security unblock code" request
            iPhone.AbortSecurityCode(blockCodeType);
            CleanupStack::PopAndDestroy(2); //wait, deviceLockStatusObserver
            return;
            }
                    
        // verification code query
        iSecurityDlg = new (ELeave) CCodeQueryDialog (verifcationPassword,SEC_C_PIN_CODE_MIN_LENGTH,SEC_C_PIN_CODE_MAX_LENGTH,ESecUiNone);
        deviceLockStatusObserver->SetAddress(iSecurityDlg);
        deviceLockStatusObserver->StartObserver();
        ret = iSecurityDlg->ExecuteLD(R_VERIFY_NEW_PIN2_CODE_QUERY);
        
        if (!ret || (ret == ESecUiDeviceLocked))    
            {
            // cancel "get security unblock code" request
            iPhone.AbortSecurityCode(blockCodeType);
            CleanupStack::PopAndDestroy(2); //wait, deviceLockStatusObserver
            return;
            }
        }            
    CleanupStack::PopAndDestroy(deviceLockStatusObserver);            
    // send code
    
    iPhone.VerifySecurityCode(wait->iStatus,blockCodeType,aNewPassword,aPassword);
    TInt res = wait->WaitForRequestL();
    CleanupStack::PopAndDestroy(wait);
    
    switch(res)
        {
        case KErrNone:
            // code approved -> note
            CSecuritySettings::ShowResultNoteL(R_PIN2_CODE_CHANGED_NOTE, CAknNoteDialog::EConfirmationTone);
            break;
        case KErrGsm0707IncorrectPassword:
        case KErrAccessDenied:
            // wrong PUK2 code -> note -> ask PUK2 code again        
            CSecuritySettings::ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
            Puk2RequiredL();
            break;
        case KErrGsmSSPasswordAttemptsViolation:
        case KErrLocked:
            // Pin2 features blocked permanently!
            CSecuritySettings::ShowResultNoteL(R_PIN2_REJECTED, CAknNoteDialog::EConfirmationTone);    
            break;    
        default:
            CSecuritySettings::ShowErrorNoteL(res);            
            Puk2RequiredL();
            break;
        }            
    }

//
// ----------------------------------------------------------
// CSecurityHandler::UPinRequiredL()
// Hendles UniversalPinRequired event
// ----------------------------------------------------------
//  
TInt CSecurityHandler::UPinRequiredL()
    {
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
    TBool wcdmaSupported(FeatureManager::FeatureSupported( KFeatureIdProtocolWcdma ));
    TBool upinSupported(FeatureManager::FeatureSupported( KFeatureIdUpin ));
    if(wcdmaSupported || upinSupported)
       {
        RMobilePhone::TMobilePassword password;
        RMobilePhone::TMobilePassword required_fourth;
        RMobilePhone::TMobilePhoneSecurityCodeInfoV5 codeInfo;
        RMobilePhone::TMobilePhoneSecurityCodeInfoV5Pckg codeInfoPkg(codeInfo);
        RMobilePhone::TMobilePhoneSecurityCode secCodeType = RMobilePhone::ESecurityUniversalPin;
        CWait* wait = CWait::NewL();
        CleanupStack::PushL(wait);
        TBool StartUp = ETrue; 
        TInt secUiOriginatedQuery(ESecurityUIsSecUIOriginatedUninitialized);
        TInt err = KErrNone;
        TInt res = KErrGeneral;
    
        StartUp = iStartup;
    
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecurityHandler::UPinRequiredL()"));
        #endif
    
        if(!StartUp)
        {
            // read a flag to see whether the query is SecUi originated. 
            err = RProperty::Get(KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery, secUiOriginatedQuery);
        }
        
        #if defined(_DEBUG)
        RDebug::Print(_L("CSecurityHandler::UPinRequiredL() Execute dlg"));
        #endif 
        if(StartUp || (secUiOriginatedQuery != ESecurityUIsSecUIOriginated) || (err != KErrNone))
        {
            iSecurityDlg = new (ELeave) CCodeQueryDialog (password,SEC_C_PIN_CODE_MIN_LENGTH,SEC_C_PIN_CODE_MAX_LENGTH,ESecUiCodeEtelReqest);
            if(AknLayoutUtils::PenEnabled())
                iSecurityDlg->SetEmergencyCallSupportForCBA( ETrue );
            else
                iSecurityDlg->SetEmergencyCallSupport(ETrue);
            #ifdef __COVER_DISPLAY
            iSecurityDlg->PublishDialogL(SecondaryDisplay::ECmdShowSecurityQuery, SecondaryDisplay::KCatStartup);
            CAknMediatorFacade* covercl = AknMediatorFacade(iSecurityDlg); // uses MOP, so control provided 
    		if (covercl) // returns null if __COVER_DISPLAY is not defined
        		{
        		// … -  add data that cover ui is interested in
        		covercl->BufStream().WriteInt32L(SecondaryDisplay::EShowUPIN); // adds int to additional data to be posted to cover ui
        		covercl->BufStream().CommitL(); // no more data to send so commit buf
         		}  
            #endif //__COVER_DISPLAY
            
            wait->SetRequestType(EMobilePhoneGetSecurityCodeInfo);
            iPhone.GetSecurityCodeInfo(wait->iStatus, secCodeType, codeInfoPkg);
            res = wait->WaitForRequestL();
            User::LeaveIfError(res);
            
            if(codeInfo.iRemainingEntryAttempts == KMaxNumberOfPINAttempts)
                res = iSecurityDlg->ExecuteLD(R_UPIN_REQUEST_QUERY);
            else if(codeInfo.iRemainingEntryAttempts > KLastRemainingInputAttempt)
                {
                    HBufC* queryPrompt = StringLoader::LoadLC(R_SECUI_REMAINING_UPIN_ATTEMPTS, codeInfo.iRemainingEntryAttempts);
                    res = iSecurityDlg->ExecuteLD(R_UPIN_REQUEST_QUERY, *queryPrompt);
                    CleanupStack::PopAndDestroy(queryPrompt);
                }
            else
                {
                    HBufC* queryPrompt = StringLoader::LoadLC(R_SECUI_FINAL_UPIN_ATTEMPT);
                    res = iSecurityDlg->ExecuteLD(R_UPIN_REQUEST_QUERY, *queryPrompt);
                    CleanupStack::PopAndDestroy(queryPrompt);   
                }
            
            
            if ((!res) || (res == ESecUiEmergencyCall) || (res == EAknSoftkeyEmergencyCall))
                { 
                CleanupStack::PopAndDestroy(wait);   
                return KErrCancel;
                }
        }
        else
        {
            iSecurityDlg = new (ELeave) CCodeQueryDialog (password,SEC_C_PIN_CODE_MIN_LENGTH,SEC_C_PIN_CODE_MAX_LENGTH,ESecUiNone);
            #ifdef __COVER_DISPLAY
            iSecurityDlg->PublishDialogL(SecondaryDisplay::ECmdShowSecurityQuery, SecondaryDisplay::KCatStartup);
            CAknMediatorFacade* covercl = AknMediatorFacade(iSecurityDlg); // uses MOP, so control provided 
    		if (covercl) // returns null if __COVER_DISPLAY is not defined
        		{
        		// … -  add data that cover ui is interested in
        		covercl->BufStream().WriteInt32L(SecondaryDisplay::EShowUPIN); // adds int to additional data to be posted to cover ui
        		covercl->BufStream().CommitL(); // no more data to send so commit buf
         		}  
            #endif //__COVER_DISPLAY
            
    		wait->SetRequestType(EMobilePhoneGetSecurityCodeInfo);
            iPhone.GetSecurityCodeInfo(wait->iStatus, secCodeType, codeInfoPkg);
            res = wait->WaitForRequestL();
            User::LeaveIfError(res);
            
            CSecUiLockObserver* deviceLockStatusObserver = CSecUiLockObserver::NewL(iSecurityDlg);
    		CleanupStack::PushL(deviceLockStatusObserver);
    		CSecUiLockObserver* queryStatusObserver = CSecUiLockObserver::NewL(iSecurityDlg, ESecUiRequestStateObserver);
            CleanupStack::PushL(queryStatusObserver);
            
            if(codeInfo.iRemainingEntryAttempts == KMaxNumberOfPINAttempts)
                res = iSecurityDlg->ExecuteLD(R_UPIN_QUERY);
            else if(codeInfo.iRemainingEntryAttempts > KLastRemainingInputAttempt)
                {
                    HBufC* queryPrompt = StringLoader::LoadLC(R_SECUI_REMAINING_UPIN_ATTEMPTS, codeInfo.iRemainingEntryAttempts);
                    res = iSecurityDlg->ExecuteLD(R_UPIN_QUERY, *queryPrompt);
                    CleanupStack::PopAndDestroy(queryPrompt);
                }
            else
                {
                    HBufC* queryPrompt = StringLoader::LoadLC(R_SECUI_FINAL_UPIN_ATTEMPT);
                    res = iSecurityDlg->ExecuteLD(R_UPIN_QUERY, *queryPrompt);
                    CleanupStack::PopAndDestroy(queryPrompt);   
                }
    		
    		CleanupStack::PopAndDestroy(2); //deviceLockStatusObserver, queryStatusObserver
    		iSecurityDlg = NULL;
            if( !res || (res == ESecUiDeviceLocked))
                {
                // cancel code request
                CleanupStack::PopAndDestroy(wait);
                iPhone.AbortSecurityCode(RMobilePhone::ESecurityUniversalPin);
                return KErrCancel;
                }      
        }
        
        #if defined(_DEBUG)
        RDebug::Print(_L("CSecurityNotifier::UPinRequiredL()VerifySecurityCode"));
        #endif
        iPhone.VerifySecurityCode(wait->iStatus,secCodeType, password, required_fourth);
        res = wait->WaitForRequestL();
        CleanupStack::PopAndDestroy(wait);
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecurityHandler::UPinRequiredL() VerifySecurityCode STATUS: %d"), res);
        #endif
        TInt returnValue = res;
        switch(res)
            {        
            case KErrNone:
                // code approved 
                #if defined(_DEBUG)
                RDebug::Print(_L("CSecurityHandler::UPinRequiredL()code approved "));
                #endif
                CSecuritySettings::ShowResultNoteL(R_CONFIRMATION_NOTE, CAknNoteDialog::EConfirmationTone);
                break;
            case KErrGsm0707IncorrectPassword:
            case KErrAccessDenied:
                // code was entered erroneously
                CSecuritySettings::ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
                if(StartUp)
                {
                returnValue = UPinRequiredL();
                }
                break;
            case KErrGsmSSPasswordAttemptsViolation:
            case KErrLocked:
                // code blocked; show error note and terminate.
                if(StartUp)
                    CSecuritySettings::ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone); 
                break;
            case KErrGsm0707SimWrong:
                // sim lock active
                break;
            default:
                CSecuritySettings::ShowErrorNoteL(res);
                if(StartUp)
                {
                returnValue = UPinRequiredL();
                }
                break;
            }
    
        return returnValue;
       }
    else
        return KErrNone;
    }
//
// ----------------------------------------------------------
// CSecurityHandler::UPukRequiredL()
// Handles UPukRequired event
// ----------------------------------------------------------
//
TInt CSecurityHandler::UPukRequiredL()
    {
    TBool wcdmaSupported(FeatureManager::FeatureSupported( KFeatureIdProtocolWcdma ));
    TBool upinSupported(FeatureManager::FeatureSupported( KFeatureIdUpin ));
    if(wcdmaSupported || upinSupported)
       {
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecurityHandler::UPukRequiredL()"));
        #endif            
        RMobilePhone::TMobilePassword aPassword;
        RMobilePhone::TMobilePassword aNewPassword;
        RMobilePhone::TMobilePhoneSecurityCodeInfoV5 codeInfo;
        RMobilePhone::TMobilePhoneSecurityCodeInfoV5Pckg codeInfoPkg(codeInfo);
    
        RMobilePhone::TMobilePhoneSecurityCode blockCodeType;
        blockCodeType = RMobilePhone::ESecurityUniversalPuk;
        CWait* wait = CWait::NewL();
        CleanupStack::PushL(wait);
        
        TBool StartUp(ETrue);
        StartUp = iStartup;
    
        TInt res(KErrNone);
        wait->SetRequestType(EMobilePhoneGetSecurityCodeInfo);
        iPhone.GetSecurityCodeInfo(wait->iStatus, blockCodeType, codeInfoPkg);
        res = wait->WaitForRequestL();
        User::LeaveIfError(res);
        //show last "Code Error" note for UPIN verify result so it won't be left under the PUK1 dialog
        if(!StartUp && (codeInfo.iRemainingEntryAttempts == KMaxNumberOfPUKAttempts))
            CSecuritySettings::ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
        
        // ask UPUK code
        iSecurityDlg = new (ELeave) CCodeQueryDialog (aPassword,SEC_C_PUK_CODE_MIN_LENGTH,SEC_C_PUK_CODE_MAX_LENGTH,ESecUiPukRequired);
        if(AknLayoutUtils::PenEnabled())
            iSecurityDlg->SetEmergencyCallSupportForCBA( ETrue );
        else
            iSecurityDlg->SetEmergencyCallSupport(ETrue);
        #ifdef __COVER_DISPLAY
        iSecurityDlg->PublishDialogL(SecondaryDisplay::ECmdShowSecurityQuery, SecondaryDisplay::KCatStartup);
        CAknMediatorFacade* covercl = AknMediatorFacade(iSecurityDlg); // uses MOP, so control provided 
    		if (covercl) // returns null if __COVER_DISPLAY is not defined
        		{
        		// … -  add data that cover ui is interested in
        		covercl->BufStream().WriteInt32L(SecondaryDisplay::EShowUPUK);// adds int to additional data to be posted to cover ui
        		covercl->BufStream().CommitL(); // no more data to send so commit buf
         		}  
        #endif //__COVER_DISPLAY
       
            
        if(codeInfo.iRemainingEntryAttempts == KMaxNumberOfPUKAttempts)
                res = iSecurityDlg->ExecuteLD(R_UPUK_REQUEST_QUERY);
        else if(codeInfo.iRemainingEntryAttempts > KLastRemainingInputAttempt)
           {
             HBufC* queryPrompt = StringLoader::LoadLC(R_SECUI_REMAINING_UPUK_ATTEMPTS, codeInfo.iRemainingEntryAttempts);
             res = iSecurityDlg->ExecuteLD(R_UPUK_REQUEST_QUERY, *queryPrompt);
             CleanupStack::PopAndDestroy(queryPrompt);
           }
        else
           {
             HBufC* queryPrompt = StringLoader::LoadLC(R_SECUI_FINAL_UPUK_ATTEMPT);
             res = iSecurityDlg->ExecuteLD(R_UPUK_REQUEST_QUERY, *queryPrompt);
             CleanupStack::PopAndDestroy(queryPrompt);   
           }
        
        if((!res) || (res == ESecUiEmergencyCall) || (res == EAknSoftkeyEmergencyCall))
            {
            CleanupStack::PopAndDestroy(wait);
            return KErrCancel;
            }
            
        RMobilePhone::TMobilePassword verifcationPassword;
        // new upin code query
        iSecurityDlg = new (ELeave) CCodeQueryDialog (aNewPassword,SEC_C_PIN_CODE_MIN_LENGTH,SEC_C_PIN_CODE_MAX_LENGTH,ESecUiPukRequired);
        if(AknLayoutUtils::PenEnabled())
            iSecurityDlg->SetEmergencyCallSupportForCBA( ETrue );
        else
            iSecurityDlg->SetEmergencyCallSupport(ETrue);
        res = iSecurityDlg->ExecuteLD(R_NEW_UPIN_CODE_REQUEST_QUERY);
        if((!res) || (res == ESecUiEmergencyCall) || (res == EAknSoftkeyEmergencyCall))
            {
            CleanupStack::PopAndDestroy(wait);    
            return KErrCancel;
            }
      
        // verification code query
        iSecurityDlg = new (ELeave) CCodeQueryDialog (verifcationPassword,SEC_C_PIN_CODE_MIN_LENGTH,SEC_C_PIN_CODE_MAX_LENGTH,ESecUiPukRequired);
        if(AknLayoutUtils::PenEnabled())
            iSecurityDlg->SetEmergencyCallSupportForCBA( ETrue );
        else
            iSecurityDlg->SetEmergencyCallSupport(ETrue);
        res = iSecurityDlg->ExecuteLD(R_VERIFY_NEW_UPIN_CODE_REQUEST_QUERY);
        if((!res) || (res == ESecUiEmergencyCall) || (res == EAknSoftkeyEmergencyCall))
            {
            CleanupStack::PopAndDestroy(wait);
            return KErrCancel;
            }
                                
        while (aNewPassword.CompareF(verifcationPassword) != 0) 
            {
            // codes do not match -> note -> ask new upin and verification codes again  
            CSecuritySettings::ShowResultNoteL(R_CODES_DONT_MATCH, CAknNoteDialog::EErrorTone);
            
            verifcationPassword = _L("");
            aNewPassword = _L("");
    
            // new upin code query
            iSecurityDlg = new (ELeave) CCodeQueryDialog (aNewPassword,SEC_C_PIN_CODE_MIN_LENGTH,SEC_C_PIN_CODE_MAX_LENGTH,ESecUiPukRequired);
            if(AknLayoutUtils::PenEnabled())
                iSecurityDlg->SetEmergencyCallSupportForCBA( ETrue );
            else
                iSecurityDlg->SetEmergencyCallSupport(ETrue);
            res = iSecurityDlg->ExecuteLD(R_NEW_UPIN_CODE_REQUEST_QUERY);
        	if((!res) || (res == ESecUiEmergencyCall) || (res == EAknSoftkeyEmergencyCall))
                {
                CleanupStack::PopAndDestroy(wait);
                return KErrCancel;
                }
                    
            // verification code query
            iSecurityDlg = new (ELeave) CCodeQueryDialog (verifcationPassword,SEC_C_PIN_CODE_MIN_LENGTH,SEC_C_PIN_CODE_MAX_LENGTH,ESecUiPukRequired);
            if(AknLayoutUtils::PenEnabled())
                iSecurityDlg->SetEmergencyCallSupportForCBA( ETrue );
            else
                iSecurityDlg->SetEmergencyCallSupport(ETrue);
            res = iSecurityDlg->ExecuteLD(R_VERIFY_NEW_UPIN_CODE_REQUEST_QUERY);
        	if((!res) || (res == ESecUiEmergencyCall) || (res == EAknSoftkeyEmergencyCall))
                {
                CleanupStack::PopAndDestroy(wait);
                return KErrCancel;
                }
            }            
            
        // send code
        iPhone.VerifySecurityCode(wait->iStatus,blockCodeType,aNewPassword,aPassword);
        res = wait->WaitForRequestL();
        CleanupStack::PopAndDestroy(wait);
        
        TInt returnValue = res;
        switch(res)
            {
            case KErrNone:
                // code approved -> note
                CSecuritySettings::ShowResultNoteL(R_UPIN_CODE_CHANGED_NOTE, CAknNoteDialog::EConfirmationTone);
                break;
            case KErrGsm0707IncorrectPassword:
            case KErrAccessDenied:
                // wrong PUK code -> note -> ask UPUK code again        
                CSecuritySettings::ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
                returnValue = UPukRequiredL();
                break;
            case KErrGsm0707SimWrong:
                // sim lock active
                break;
            case KErrGsmSSPasswordAttemptsViolation:
            case KErrLocked:
                // sim card rejected.
                break;
            default:
                CSecuritySettings::ShowErrorNoteL(res);        
                returnValue = UPukRequiredL();
                break;
            }   
    
        return returnValue;
       }
    else
        return KErrNone;
    }

//
// ----------------------------------------------------------
// CSecurityHandler::SimLockEventL()
// Shows "SIM restriction on" note
// ----------------------------------------------------------
//    
void CSecurityHandler::SimLockEventL()
    {
    #if defined(_DEBUG)
    RDebug::Print(_L("CSecurityHandler::SimLockEventL()"));
    #endif
    CSecuritySettings::ShowResultNoteL(R_SIM_ON, CAknNoteDialog::EConfirmationTone);    
    }
// ---------------------------------------------------------
// CSecurityHandler::RemoveSplashScreenL()
// Removes splash screen
// ---------------------------------------------------------
void CSecurityHandler::RemoveSplashScreenL() const
    {

    }

// ---------------------------------------------------------
// CSecurityHandler::ShowGenericErrorNoteL(TInt aStatus)
// Shows a generic error note
// ---------------------------------------------------------

void CSecurityHandler::ShowGenericErrorNoteL(TInt aStatus)
    {
       // Let's create TextResolver instance for error resolving...
       CTextResolver* textresolver = CTextResolver::NewLC(); 
       // Resolve the error
       TPtrC errorstring;
       errorstring.Set( textresolver->ResolveErrorString( aStatus ) );
       iNoteDlg = new (ELeave) CAknNoteDialog(REINTERPRET_CAST(CEikDialog**,&iNoteDlg));
       iNoteDlg->PrepareLC(R_CODE_ERROR);
       iNoteDlg->SetTextL((TDesC&)errorstring);
       iNoteDlg->RunDlgLD(CAknNoteDialog::ELongTimeout, CAknNoteDialog::EErrorTone);
       CleanupStack::PopAndDestroy(textresolver); 
    }
                    

// End of file
