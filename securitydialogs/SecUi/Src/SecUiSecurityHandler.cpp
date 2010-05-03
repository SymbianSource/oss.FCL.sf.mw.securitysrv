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
// #include <secondarydisplay/SecondaryDisplayStartupAPI.h>
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

#include "SecQueryUi.h"

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
// qtdone
EXPORT_C CSecurityHandler::CSecurityHandler(RMobilePhone& aPhone):
        iPhone(aPhone), iQueryCanceled(ETrue), iSecurityDlg(NULL), iNoteDlg(NULL) 
    {
    		RDEBUG( "0", 0 );

        TInt result = iCustomPhone.Open(aPhone);
        TRAP_IGNORE( FeatureManager::InitializeLibL() ); //Shouldn't this panic if FM does not initialise??
    }

//
// ----------------------------------------------------------
// CSecurityHandler::~CSecurityHandler()
// Destructor
// ----------------------------------------------------------
// qtdone
EXPORT_C CSecurityHandler::~CSecurityHandler()
    {
    	RDEBUG( "0", 0 );

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
// qtdone
EXPORT_C void CSecurityHandler::HandleEventL(
    RMobilePhone::TMobilePhoneSecurityEvent aEvent )
    {
    	RDEBUG( "0", 0 );

    TInt result = KErrNone;
    HandleEventL( aEvent, result );
    }

//
// ----------------------------------------------------------
// CSecurityHandler::HandleEventL()
// Handles different security events
// ----------------------------------------------------------
// qtdone
EXPORT_C void CSecurityHandler::HandleEventL(
    RMobilePhone::TMobilePhoneSecurityEvent aEvent,
    TBool aStartup, TInt& aResult )
    {
    	RDEBUG( "0", 0 );

    iStartup = aStartup;
    HandleEventL( aEvent, aResult );
    }
    
//
// ----------------------------------------------------------
// CSecurityHandler::HandleEventL()
// Handles different security events
// ----------------------------------------------------------
// qtdone
EXPORT_C void CSecurityHandler::HandleEventL(
    RMobilePhone::TMobilePhoneSecurityEvent aEvent, TInt& aResult )
    {
    	RDEBUG( "0", 0 );

    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
    TBool wcdmaSupported(FeatureManager::FeatureSupported( KFeatureIdProtocolWcdma ));
    TBool upinSupported(FeatureManager::FeatureSupported( KFeatureIdUpin ));
		RDEBUG( "aEvent", aEvent );
 
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
                Puk1RequiredL();
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
    RDEBUG( "aResult", aResult );
    }
//
// ----------------------------------------------------------
// CSecurityHandler::AskSecCodeL()
// For asking security code e.g in settings
// ----------------------------------------------------------
// qtdone
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
    TInt lAlphaSupported=0;
    TInt lCancelSupported=0;

    res = repository->Get(KSCPLockCodeDefaultLockCode , currentLockStatus);
    RDEBUG( "res", res );
    RDEBUG( "currentLockStatus", currentLockStatus );
    delete repository;
    if(res==0 && currentLockStatus==1)
        {
        // code is the default one; no need to request it.
        return ETrue;
        }
    /* end check for default code */

    
    iQueryCanceled = EFalse;
    RMobilePhone::TMobilePassword required_fourth;
        
    TInt ret = KErrNone;
    TInt status = KErrNone;
    
		RMobilePhone::TMobilePassword iSecUi_password;
    TInt queryAccepted = KErrCancel;

        while (queryAccepted!=KErrNone)
            {
		        RMobilePhone::TMobilePhoneSecurityCode secCodeType;
		        secCodeType = RMobilePhone::ESecurityCodePhonePassword;

						/* request PIN using QT */
						CSecQueryUi *iSecQueryUi;
						RDEBUG( "CSecQueryUi", 0 );
						iSecQueryUi = CSecQueryUi::NewL();
						lAlphaSupported = ESecUiAlphaSupported;
						lCancelSupported = ESecUiCancelSupported;
						TBuf<0x100> title;	title.Zero();	title.Append(_L("AskSecCodeL"));	title.Append(_L("#"));	title.AppendNum(-1);
						queryAccepted = iSecQueryUi->SecQueryDialog( title, iSecUi_password, SEC_C_SECURITY_CODE_MIN_LENGTH,SEC_C_SECURITY_CODE_MAX_LENGTH, lAlphaSupported | lCancelSupported | secCodeType /*aMode*/ );
						RDEBUG( "iSecUi_password", 0 );
						RDebug::Print( iSecUi_password );
						RDEBUG( "delete", 0 );
						delete iSecQueryUi;
						RDEBUG( "queryAccepted", queryAccepted );
						/* end request PIN using QT */
		        if (queryAccepted!=KErrNone)
		            {
		            ret = EFalse;
		            return ret;
		            }
		
		        CWait* wait = CWait::NewL();
        		RDEBUG( "VerifySecurityCode", 0 );
		        iPhone.VerifySecurityCode(wait->iStatus,secCodeType, iSecUi_password, required_fourth);
       			RDEBUG( "WaitForRequestL", 0 );
		        status = wait->WaitForRequestL();
      			RDEBUG( "status", status );
		        delete wait;
        		#ifdef __WINS__
		        if(status==KErrNotSupported )
		        	{
		        	RDEBUG( "status", status );
	        		status=KErrNone;
		        	}
		        #endif

						ret = ETrue;
		        queryAccepted = KErrCancel;	// because it's not yet validated
            switch(status)
                {        
                case KErrNone:
                    {
                    if(FeatureManager::FeatureSupported(KFeatureIdSapTerminalControlFw ) &&
    										!(FeatureManager::FeatureSupported(KFeatureIdSapDeviceLockEnhancements)))
    								{
   											RDEBUG( "calling RSCPClient", 0 );
                        RSCPClient scpClient;
                        User::LeaveIfError( scpClient.Connect() );
                        CleanupClosePushL( scpClient );

                        TSCPSecCode newCode;
                        newCode.Copy( iSecUi_password );
                        scpClient.StoreCode( newCode );
                       	RDEBUG( "called StoreCode", 1 );

                        CleanupStack::PopAndDestroy(); //scpClient
                       	queryAccepted = KErrNone;
                  	}
                            	
                    iQueryCanceled = ETrue;	// TODO
                    return ETrue;
                    }                    
                case KErrGsmSSPasswordAttemptsViolation:
                case KErrLocked:
                    {
                    // security code blocked! 
                    CSecuritySettings::ShowResultNoteL(R_SEC_BLOCKED, CAknNoteDialog::EErrorTone);
                    break;
                    }
                case KErrGsm0707IncorrectPassword:
                case KErrAccessDenied:
                    {    
                    // code was entered erroneusly
                    CSecuritySettings::ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
                    }    
                default:
                    {
                    CSecuritySettings::ShowResultNoteL(status, CAknNoteDialog::EErrorTone);
                    }
                }     
        } // while

    iQueryCanceled = ETrue;
    return ret;
    }
//
// ----------------------------------------------------------
// CSecurityHandler::CancelSecCodeQuery()    
// Cancels PIN2 and security code queries
// TODO is this used?
// ----------------------------------------------------------
// qtdone
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
// qtdone
EXPORT_C TBool CSecurityHandler::AskSecCodeInAutoLockL()
    {
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
    
			RDEBUG( "0", 0 );
			

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

			RDEBUG( "res", res );

    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecurityHandler::AskSecCodeInAutoLockL() autolock period:%d"), res);
    #endif
    if (res == KErrNone)
        {
        // disable autolock in Domestic OS side too if autolock period is 0.
					RDEBUG( "period", period );
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
									RDEBUG( "0", 0 );
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

									RDEBUG( "lockChange", lockChange );
                wait = CWait::NewL();
									RDEBUG( "0", 0 );
								// this also calls PassPhraseRequiredL ???
									RDEBUG( "SetLockSetting", 1 );
                iPhone.SetLockSetting(wait->iStatus,lockType,lockChange);
				        res = KErrNone;
        					RDEBUG( "WaitForRequestL", 0 );
                res = wait->WaitForRequestL();
									RDEBUG( "res", res );
                delete wait;
                #if defined(_DEBUG)
                RDebug::Print(_L("(SECUI)CSecurityHandler::AskSecCodeInAutoLockL() SetLockSetting RESULT:%d"), res);
                #endif
            }	// from   period == 0
        else
            {	// ask security code
                #if defined(_DEBUG)
            	RDebug::Print(_L("(SECUI)CSecurityHandler::AskSecCodeInAutoLockL() Ask sec code via notifier"));
            	#endif
									RDEBUG( "0", 0 );
                RNotifier codeQueryNotifier;
                User::LeaveIfError(codeQueryNotifier.Connect());
                CWait* wait = CWait::NewL();
                CleanupStack::PushL(wait);
                TInt queryResponse = 0;
                TPckg<TInt> response(queryResponse);
									RDEBUG( "0", 0 );
                TSecurityNotificationPckg params;
                params().iEvent = static_cast<TInt>(RMobilePhone::EPhonePasswordRequired);
                params().iStartup = EFalse;
                #if defined(_DEBUG)
    			RDebug::Print(_L("(SECUI)CSecurityHandler::AskSecCodeInAutoLockL() Start Notifier"));
    			#endif
									RDEBUG( "0", 0 );
									RDEBUG( "StartNotifierAndGetResponse", 0 );
                codeQueryNotifier.StartNotifierAndGetResponse(wait->iStatus, KSecurityNotifierUid,params, response);
                // this will eventually call PassPhraseRequiredL
									RDEBUG( "WaitForRequestL", 0 );
                res = wait->WaitForRequestL();
									RDEBUG( "WaitForRequestL", 1 );
									RDEBUG( "res", res );
                CleanupStack::PopAndDestroy(); // wait
            	if(res == KErrNone)
                	res = queryResponse;
            }	// from   else period == 0
         	RDEBUG( "0", 0 );
        }
    else
        {	// can't read repository for KSettingsAutoLockTime
									RDEBUG( "KERRSOMETHING:Call SetLockSetting", 0 );

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
									RDEBUG( "0", 0 );

        wait = CWait::NewL();
       		RDEBUG( "SetLockSetting", 0 );
        iPhone.SetLockSetting(wait->iStatus,lockType,lockChange);
					RDEBUG( "WaitForRequestL", 0 );
        res = wait->WaitForRequestL();
					RDEBUG( "WaitForRequestL", 1 );
        delete wait;
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecurityHandler::AskSecCodeInAutoLockL() KES: SetLockSetting RESULT:%d"), res);
        #endif
        }
        
			RDEBUG( "res", res );
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
            	RDEBUG( "KErrAccessDenied", KErrAccessDenied );
						return AskSecCodeInAutoLockL();
            }
        case KErrAbort:
        case KErrCancel:
            // user pressed "cancel"
            return EFalse;
        default:
            {
            	RDEBUG( "default", res );
            return AskSecCodeInAutoLockL();
            }
        }
    }
//
// ----------------------------------------------------------
// CSecurityHandler::PassPhraseRequired()    
// Handles PassPhraseRequired event
// ----------------------------------------------------------
// qtdone
TInt CSecurityHandler::PassPhraseRequiredL()
    {
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
    	RDEBUG( "0", 0 );
    TBool StartUp = iStartup;

    RMobilePhone::TMobilePassword iSecUi_password;
    RMobilePhone::TMobilePassword required_fourth;
	  TInt queryAccepted = KErrCancel;

    TInt autolockState=0;
    TInt lCancelSupported=0;
    TInt lEmergencySupported=0;
    TInt lAlphaSupported=0;
    
    TInt err( KErrGeneral );
    err = RProperty::Get(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, autolockState);
    	RDEBUG( "StartUp", StartUp );
    	RDEBUG( "err", err );
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
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecurityHandler::PassPhraseRequiredL() Dialog 1"));
        #endif
        // Security code at bootup: No "cancel" softkey; Emergency calls enabled.
						RMobilePhone::TMobilePhoneSecurityCode secCodeTypeToAsk = RMobilePhone::ESecurityCodePhonePassword;	// for starters
    					RDEBUG( "isConditionSatisfied", isConditionSatisfied );
						if (isConditionSatisfied)
				        {
				        	// starter or special TARM. NoCancel+Emergency
				        	lCancelSupported = ESecUiCancelNotSupported;
				        	lEmergencySupported = ESecUiEmergencySupported;
				      	}
	        	else if (autolockState > EAutolockOff)
	        		{
	        			// from unlock. Cancel+Emergency
				        	lCancelSupported = ESecUiCancelSupported;
				        	lEmergencySupported = ESecUiEmergencySupported;
	        		}
	        	else
	        		{
	        			// from settings. Cancel+NoEmergency
				        	lCancelSupported = ESecUiCancelSupported;
				        	lEmergencySupported = ESecUiEmergencyNotSupported;
	        		}
						lAlphaSupported = ESecUiAlphaSupported;
						
						/* request PIN using QT */
						CSecQueryUi *iSecQueryUi;
						iSecQueryUi = CSecQueryUi::NewL();
						TInt lType = lAlphaSupported | lCancelSupported | lEmergencySupported | secCodeTypeToAsk;
							RDEBUG( "lType", lType );
						queryAccepted = iSecQueryUi->SecQueryDialog( _L("PassPhraseRequiredL"), iSecUi_password, SEC_C_SECURITY_CODE_MIN_LENGTH,SEC_C_SECURITY_CODE_MAX_LENGTH, lType );
						RDEBUG( "iSecUi_password", 0 );
						RDebug::Print( iSecUi_password );
						RDEBUG( "queryAccepted", queryAccepted );
						delete iSecQueryUi;
						/* end request PIN using QT */


TBool wasCancelledOrEmergency = EFalse;		
	RDEBUG( "KFeatureIdSapDeviceLockEnhancements", KFeatureIdSapDeviceLockEnhancements );
if ( (queryAccepted==KErrAbort /* =emergency */) || (queryAccepted == KErrCancel))
    wasCancelledOrEmergency = ETrue;
	RDEBUG( "wasCancelledOrEmergency", wasCancelledOrEmergency );
			if (wasCancelledOrEmergency)
        {
		#if defined(_DEBUG)
		RDebug::Print(_L("(SECUI)CSecurityHandler::PassPhraseRequiredL() DIALOG ERROR"));
		#endif
        if (!StartUp)
            {
            	RDEBUG( "AbortSecurityCode", 0 );
            iPhone.AbortSecurityCode(RMobilePhone::ESecurityCodePhonePassword);
            	RDEBUG( "AbortSecurityCode", 1 );
            }
        return KErrCancel;
        }

    	RMobilePhone::TMobilePhoneSecurityCode secCodeType = RMobilePhone::ESecurityCodePhonePassword;
     	CWait* wait = NULL;
     	TInt status = KErrNone;;
        wait = CWait::NewL();
        	RDEBUG( "VerifySecurityCode", 0 );
        #ifndef __WINS__
        iPhone.VerifySecurityCode(wait->iStatus,secCodeType, iSecUi_password, required_fourth);
        	RDEBUG( "WaitForRequestL", 0 );
        status = wait->WaitForRequestL();
        #else
        status = KErrTimedOut;
        	RDEBUG( "WaitForRequestL not waint WINS", 0 );
        #endif
        	RDEBUG( "WaitForRequestL status", status );
				#ifdef __WINS__
		   	if (status == KErrTimedOut)
		   		{
		   		status = KErrNone;
		   		}
		    #endif
        delete wait;
    
    TInt returnValue = status;
        	RDEBUG( "tarmFlag", tarmFlag );
        	RDEBUG( "StartUp", StartUp );
    switch(status)
        {        
        case KErrNone:
            #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CSecurityHandler::PassPhraseRequiredL() KErrNone"));
            #endif
            // code approved 
            CSecuritySettings::ShowResultNoteL(R_CONFIRMATION_NOTE, CAknNoteDialog::EConfirmationTone);
            	RDEBUG( "R_CONFIRMATION_NOTE", R_CONFIRMATION_NOTE );
        if(FeatureManager::FeatureSupported(KFeatureIdSapTerminalControlFw))    
        {
        				RDEBUG( "KFeatureIdSapTerminalControlFw", KFeatureIdSapTerminalControlFw );
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
                	RDEBUG( "KFeatureIdSapDeviceLockEnhancements", KFeatureIdSapDeviceLockEnhancements );
    						RSCPClient scpClient;
    							RDEBUG( "scpClient.Connect", 0 );
                User::LeaveIfError( scpClient.Connect() );
    							RDEBUG( "scpClient.Connect", 1 );
                CleanupClosePushL( scpClient );
                TSCPSecCode newCode;
                newCode.Copy( iSecUi_password );
                scpClient.StoreCode( newCode );
    							RDEBUG( "scpClient.StoreCode", 1 );
                CleanupStack::PopAndDestroy(); //scpClient
                }

          }
        			RDEBUG( "StartUp", StartUp );
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
                                	RDEBUG( "iCustomPhone.DisablePhoneLock", 0 );
                                wait = CWait::NewL();
                                iCustomPhone.DisablePhoneLock(wait->iStatus,iSecUi_password);
                                wait->WaitForRequestL();
                                	RDEBUG( "iCustomPhone.DisablePhoneLock", 1 );
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

                        	RDEBUG( "iCustomPhone.DisablePhoneLock", 0 );
                        wait = CWait::NewL();
                        iCustomPhone.DisablePhoneLock(wait->iStatus,iSecUi_password);
                        wait->WaitForRequestL();
                        	RDEBUG( "iCustomPhone.DisablePhoneLock", 1 );
                        delete wait;
#endif // RD_REMOTELOCK
                        }
                    }
                else	// error getting repository
                    {
        						RDEBUG( "error getting repository", 0 );
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
                            	RDEBUG( "iCustomPhone.DisablePhoneLock", 0 );
                            wait = CWait::NewL();
                            iCustomPhone.DisablePhoneLock(wait->iStatus,iSecUi_password);
                            wait->WaitForRequestL();
                            	RDEBUG( "iCustomPhone.DisablePhoneLock", 1 );
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

                    // could not get the current autolock time... disable autolock in Domestic OS side.
                    	RDEBUG( "iCustomPhone.DisablePhoneLock", 0 );
                    wait = CWait::NewL();
                    iCustomPhone.DisablePhoneLock(wait->iStatus,iSecUi_password);
                    wait->WaitForRequestL();
                    	RDEBUG( "iCustomPhone.DisablePhoneLock", 1 );
                    delete wait;

#endif // RD_REMOTELOCK
                    }
                
                } // no Startup

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
        			RDEBUG( "KErrAccessDenied", KErrAccessDenied );
            	// TODO should this try again? It seems that it's not asked again.
            #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CSecurityHandler::PassPhraseRequiredL() KErrGsm0707IncorrectPassword"));
            #endif
            CSecuritySettings::ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
            break;
        default:
        			RDEBUG( "default", status );
            CSecuritySettings::ShowErrorNoteL(status);
            	// TODO should this try again? It seems that it's not asked again.
            break;
        }
			RDEBUG( "returnValue", returnValue );
	  return returnValue;
    }
//
// ----------------------------------------------------------
// CSecurityHandler::Pin1Required()    
// Handles Pin1Required event
// ----------------------------------------------------------
// qtdone
TInt CSecurityHandler::Pin1RequiredL()
    {
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
	RDEBUG( "0", 0 );

		RMobilePhone::TMobilePassword iSecUi_password;
		TInt lCancelSupported = ESecUiCancelNotSupported;
    TInt queryAccepted = KErrCancel;
    TInt lAlphaSupported=0;
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
	RDEBUG( "0", 0 );

    StartUp = iStartup;

	RDEBUG( "StartUp", StartUp );
    if(!StartUp)
    {
        // read a flag to see whether the query is SecUi originated. For example, from CSecuritySettings::ChangePinRequestParamsL
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

	RDEBUG( "StartUp", StartUp );
	RDEBUG( "secUiOriginatedQuery", secUiOriginatedQuery );
	RDEBUG( "ESecurityUIsSecUIOriginated", ESecurityUIsSecUIOriginated );
	RDEBUG( "err", err );
    if(StartUp || (secUiOriginatedQuery != ESecurityUIsSecUIOriginated) || (err != KErrNone))
    {	
					RDEBUG( "0", 0 );
					lCancelSupported = ESecUiCancelNotSupported;
		}
		else
			{
				lCancelSupported = ESecUiCancelSupported;
				// it will be RMobilePhone::ESecurityCodePin1 , equivalent to ESecUiNone
			}
        wait->SetRequestType(EMobilePhoneGetSecurityCodeInfo);
        	RDEBUG( "GetSecurityCodeInfo", 0 );
        iPhone.GetSecurityCodeInfo(wait->iStatus, secCodeType, codeInfoPkg);
        res = wait->WaitForRequestL();

        TInt attempts(codeInfo.iRemainingEntryAttempts);
        	RDEBUG( "attempts", attempts );

					RDEBUG( "res", res );
    		#ifdef __WINS__
						RDEBUG( "emulator can't read PIN attempts", res );
    			res=KErrNone;
    			codeInfo.iRemainingEntryAttempts=3;
 		    #endif

        User::LeaveIfError(res);
				/* request PIN using QT */
				CSecQueryUi *iSecQueryUi;
					RDEBUG( "CSecQueryUi", 0 );
				iSecQueryUi = CSecQueryUi::NewL();
					RDEBUG( "SecQueryDialog", 1 );
				// TODO ESecUiCodeEtelReqest/ESecUiNone might be useful
				// TODO also support Emergency
				lAlphaSupported = ESecUiAlphaNotSupported;
				TBuf<0x100> title;	title.Zero();	title.Append(_L("Pin1RequiredL"));	title.Append(_L("#"));	title.AppendNum(codeInfo.iRemainingEntryAttempts);
				TInt amode = lAlphaSupported | lCancelSupported | ESecUiEmergencySupported | secCodeType;
					RDEBUG( "amode", amode );
				queryAccepted = iSecQueryUi->SecQueryDialog( title, iSecUi_password, SEC_C_PIN_CODE_MIN_LENGTH, SEC_C_PIN_CODE_MAX_LENGTH, amode );
					RDEBUG( "iSecUi_password", 0 );
				RDebug::Print( iSecUi_password );
				delete iSecQueryUi;
					RDEBUG( "queryAccepted", queryAccepted );
        	// TODO handle emergency
				/* end request PIN using QT */

        if ( queryAccepted == KErrAbort )	// emergency call
            { 
            #if defined(_DEBUG)
            RDebug::Print(_L("CSecurityHandler::Pin1RequiredL() R_PIN_REQUEST_QUERY CANCEL!"));
            #endif
            CleanupStack::PopAndDestroy(wait);   // this is needed
            return KErrCancel;
            }
        if( lCancelSupported && (queryAccepted == KErrCancel) )
        	  {
        	  // cancel code request
        	  	RDEBUG( "AbortSecurityCode", 0 );
            iPhone.AbortSecurityCode(RMobilePhone::ESecurityCodePin1);
            	RDEBUG( "AbortSecurityCode", 1 );
            CleanupStack::PopAndDestroy(wait);   // this is needed
            return KErrCancel;
          }

			RDEBUG( "iSecUi_password", iSecUi_password );
		RDebug::Print( iSecUi_password );
			RDEBUG( "VerifySecurityCode", 0 );
		iPhone.VerifySecurityCode(wait->iStatus,secCodeType, iSecUi_password, required_fourth);
    	RDEBUG( "WaitForRequestL", 0 );
    res = wait->WaitForRequestL();
    	RDEBUG( "WaitForRequestL res", res );
    CleanupStack::PopAndDestroy(wait); 

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
            // TODO what if not during Startup? Probably it's Ok since the SIM would had also failed at StartUp
            if(StartUp)
                CSecuritySettings::ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone); 
            break;
        case KErrGsm0707SimWrong:
            // sim lock active
            // TODO no error? This is strange
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
// qtdone
TInt CSecurityHandler::Puk1RequiredL()
    {
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecurityHandler::Puk1RequiredL()"));
    #endif            
		TInt queryAccepted = KErrCancel;
    RMobilePhone::TMobilePassword iSecUi_password;
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
    
    TInt thisTry = 0;

	// If there was a problem (as there might be in case we're dropping off SIM Access Profile); try again a couple of times.
	while ( res != KErrNone && ( thisTry++ ) <= KTriesToConnectServer )
        {
        if(thisTry>0)
        	User::After( KTimeBeforeRetryingRequest );
        	RDEBUG( "GetSecurityCodeInfo", 0 );
        iPhone.GetSecurityCodeInfo(wait->iStatus, blockCodeType, codeInfoPkg);
		    	RDEBUG( "WaitForRequestL", 0 );
		    res = wait->WaitForRequestL();
		    	RDEBUG( "WaitForRequestL res", res );
        }
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecurityHandler::Puk1RequiredL(): Get Code info result: %d"), res);
    #endif
    //If there's still an error we're doomed. Bail out.
    User::LeaveIfError(res);
    
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecurityHandler::Puk1RequiredL(): Show last note"));
    #endif

    	RDEBUG( "StartUp", StartUp );
    	RDEBUG( "codeInfo.iRemainingEntryAttempts", codeInfo.iRemainingEntryAttempts );
    //show the last "Code Error" note of PIN verify result here so it won't be left under the PUK1 dialog
    if(!StartUp && (codeInfo.iRemainingEntryAttempts == KMaxNumberOfPUKAttempts))
        CSecuritySettings::ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
    
    // ask PUK code
				/* request PIN using QT */
				CSecQueryUi *iSecQueryUi;
					RDEBUG( "CSecQueryUi", 0 );
				iSecQueryUi = CSecQueryUi::NewL();
					RDEBUG( "SecQueryDialog", 1 );
				// TODO ESecUiCodeEtelReqest/ESecUiNone might be useful
				// TODO also support Emergency
				TBuf<0x100> title;	title.Zero();	title.Append(_L("Puk1RequiredL"));	title.Append(_L("#"));	title.AppendNum(codeInfo.iRemainingEntryAttempts);
				queryAccepted = iSecQueryUi->SecQueryDialog( title, iSecUi_password, SEC_C_PUK_CODE_MIN_LENGTH,SEC_C_PUK_CODE_MAX_LENGTH, ESecUiAlphaNotSupported | ESecUiCancelSupported | ESecUiPukRequired );
					RDEBUG( "iSecUi_password", 0 );
				RDebug::Print( iSecUi_password );
				delete iSecQueryUi;
					RDEBUG( "queryAccepted", queryAccepted );
    
    if( (queryAccepted == KErrAbort) || (queryAccepted==KErrCancel) )
        {
        CleanupStack::PopAndDestroy(wait);	// TODO this is needed ???
        return KErrCancel;
        }

				{
    		// new pin code query
				CSecQueryUi *iSecQueryUi;
					RDEBUG( "CSecQueryUi", 0 );
				iSecQueryUi = CSecQueryUi::NewL();
					RDEBUG( "SecQueryDialog", 1 );
				// TODO ESecUiCodeEtelReqest/ESecUiNone might be useful
				// TODO also support Emergency

				queryAccepted = iSecQueryUi->SecQueryDialog( _L("Puk1-New|Puk1-Verif"), aNewPassword, SEC_C_PUK_CODE_MIN_LENGTH,SEC_C_PUK_CODE_MAX_LENGTH, ESecUiAlphaNotSupported | ESecUiCancelSupported | ESecUiPukRequired );
					RDEBUG( "aNewPassword", 0 );
				RDebug::Print( aNewPassword );
				delete iSecQueryUi;
					RDEBUG( "queryAccepted", queryAccepted );
      	}

    if( (queryAccepted == KErrAbort) || (queryAccepted==KErrCancel) )
        {
        CleanupStack::PopAndDestroy(wait);    
        return KErrCancel;
        }
        
    // send code
    	RDEBUG( "VerifySecurityCode", 0 );
    iPhone.VerifySecurityCode(wait->iStatus,blockCodeType,aNewPassword,iSecUi_password);
    	RDEBUG( "WaitForRequestL", 0 );
    res = wait->WaitForRequestL();
    	RDEBUG( "WaitForRequestL res", res );
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
            // TODO no message ?
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
// qtdone
void CSecurityHandler::Pin2RequiredL()
    {
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/

    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecurityHandler::Pin2RequiredL() BEGIN"));
    #endif
		TInt queryAccepted = KErrCancel;
    RMobilePhone::TMobilePassword iSecUi_password;
    RMobilePhone::TMobilePassword required_fourth;
    RMobilePhone::TMobilePhoneSecurityCode secCodeType(RMobilePhone::ESecurityCodePin2);
    RMobilePhone::TMobilePhoneSecurityCodeInfoV5 codeInfo;
    RMobilePhone::TMobilePhoneSecurityCodeInfoV5Pckg codeInfoPkg(codeInfo);
    CWait* wait = CWait::NewL();
    CleanupStack::PushL(wait);
    
		wait->SetRequestType(EMobilePhoneGetSecurityCodeInfo);
			RDEBUG( "GetSecurityCodeInfo", 0 );
    iPhone.GetSecurityCodeInfo(wait->iStatus, secCodeType, codeInfoPkg);
    	RDEBUG( "WaitForRequestL", 0 );
    TInt ret = wait->WaitForRequestL();
    	RDEBUG( "WaitForRequestL ret", ret );

    User::LeaveIfError(ret);

    TInt attempts(codeInfo.iRemainingEntryAttempts);
    	RDEBUG( "attempts", attempts );

				/* request PIN using QT */
				CSecQueryUi *iSecQueryUi;
					RDEBUG( "CSecQueryUi", 0 );
				iSecQueryUi = CSecQueryUi::NewL();
					RDEBUG( "SecQueryDialog", 1 );
				// TODO ESecUiCodeEtelReqest/ESecUiNone might be useful	against KLastRemainingInputAttempt
				// TODO also support Emergency

				TBuf<0x100> title;	title.Zero();	title.Append(_L("Pin2RequiredL"));	title.Append(_L("#"));	title.AppendNum(codeInfo.iRemainingEntryAttempts);
				queryAccepted = iSecQueryUi->SecQueryDialog( title, iSecUi_password, SEC_C_PIN2_CODE_MIN_LENGTH, SEC_C_PIN2_CODE_MAX_LENGTH, ESecUiAlphaNotSupported | ESecUiCancelSupported | secCodeType );
					RDEBUG( "iSecUi_password", 0 );
				RDebug::Print( iSecUi_password );
					RDEBUG( "queryAccepted", queryAccepted );
				delete iSecQueryUi;

		// If failed or device became locked, any pending request should be cancelled.
    if ( queryAccepted!=KErrNone )
        {
        	RDEBUG( "AbortSecurityCode", 0 );
        iPhone.AbortSecurityCode(secCodeType);
        	RDEBUG( "AbortSecurityCode", 1 );
        CleanupStack::PopAndDestroy(wait);
        return;
        }

    	RDEBUG( "VerifySecurityCode", 0 );
    iPhone.VerifySecurityCode(wait->iStatus,secCodeType,iSecUi_password,required_fourth);
    	RDEBUG( "WaitForRequestL", 0 );
    TInt status = wait->WaitForRequestL();
    	RDEBUG( "WaitForRequestL status", status );
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
// qtdone
void CSecurityHandler::Puk2RequiredL()
    {    
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
		TInt queryAccepted = KErrCancel;
    RMobilePhone::TMobilePassword iSecUi_password;
    RMobilePhone::TMobilePassword aNewPassword;
    RMobilePhone::TMobilePassword verifcationPassword;
    RMobilePhone::TMobilePhoneSecurityCodeInfoV5 codeInfo;
    RMobilePhone::TMobilePhoneSecurityCodeInfoV5Pckg codeInfoPkg(codeInfo);
    
    RMobilePhone::TMobilePhoneSecurityCode secCodeType = RMobilePhone::ESecurityCodePuk2;
    CWait* wait = CWait::NewL();
    CleanupStack::PushL(wait);
    
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecurityHandler::Puk2RequiredL()"));
    #endif
    // ask PUK2
	
	TInt ret(KErrNone);
    wait->SetRequestType(EMobilePhoneGetSecurityCodeInfo);
    	RDEBUG( "GetSecurityCodeInfo", 0 );
    iPhone.GetSecurityCodeInfo(wait->iStatus, secCodeType, codeInfoPkg);
    	RDEBUG( "WaitForRequestL", 0 );
    ret = wait->WaitForRequestL();
    	RDEBUG( "WaitForRequestL ret", ret );
    User::LeaveIfError(ret);
        
				/* request PIN using QT */
				CSecQueryUi *iSecQueryUi;
					RDEBUG( "CSecQueryUi", 0 );
				iSecQueryUi = CSecQueryUi::NewL();
					RDEBUG( "SecQueryDialog", 1 );
				// TODO ESecUiCodeEtelReqest/ESecUiNone might be useful
				// TODO also support Emergency

				TBuf<0x100> title;	title.Zero();	title.Append(_L("Puk2RequiredL"));	title.Append(_L("#"));	title.AppendNum(codeInfo.iRemainingEntryAttempts);
				queryAccepted = iSecQueryUi->SecQueryDialog( title, iSecUi_password, SEC_C_PUK2_CODE_MIN_LENGTH,SEC_C_PUK2_CODE_MAX_LENGTH, ESecUiAlphaNotSupported | ESecUiCancelSupported | secCodeType /*aMode*/ );
					RDEBUG( "iSecUi_password", 0 );
				RDebug::Print( iSecUi_password );
				delete iSecQueryUi;
					RDEBUG( "queryAccepted", queryAccepted );

    if( queryAccepted!=KErrNone )
        {
        // cancel "get security unblock code" request
        	RDEBUG( "AbortSecurityCode", 0 );
        iPhone.AbortSecurityCode(secCodeType);
        	RDEBUG( "AbortSecurityCode", 1 );
				CleanupStack::PopAndDestroy(1); //wait
        return;
        }

				{
    		// new pin code query
				CSecQueryUi *iSecQueryUi;
					RDEBUG( "CSecQueryUi", 0 );
				iSecQueryUi = CSecQueryUi::NewL();
				// TODO ESecUiCodeEtelReqest/ESecUiNone might be useful
				// TODO also support Emergency

				queryAccepted = iSecQueryUi->SecQueryDialog( _L("Puk2-New|Puk2-Verif"), aNewPassword, SEC_C_PUK2_CODE_MIN_LENGTH,SEC_C_PUK2_CODE_MAX_LENGTH, ESecUiAlphaNotSupported | ESecUiCancelSupported | secCodeType );
					RDEBUG( "aNewPassword", 0 );
				RDebug::Print( aNewPassword );
				delete iSecQueryUi;
					RDEBUG( "queryAccepted", queryAccepted );
		    if( queryAccepted!=KErrNone )
		        {
		        // cancel "get security unblock code" request
		        	RDEBUG( "AbortSecurityCode", 0 );
		        iPhone.AbortSecurityCode(secCodeType);
		        	RDEBUG( "AbortSecurityCode", 1 );
						CleanupStack::PopAndDestroy(1); //wait
		        return;
		        }
      	}
    // send code
    // TODO the current code should be verified before
    	RDEBUG( "VerifySecurityCode", 0 );
    iPhone.VerifySecurityCode(wait->iStatus,secCodeType,aNewPassword,iSecUi_password);
    	RDEBUG( "WaitForRequestL", 0 );
    TInt res = wait->WaitForRequestL();
    	RDEBUG( "WaitForRequestL res", res );
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
// qtdone
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
				TInt queryAccepted = KErrCancel;
				TInt lCancelSupported = ESecUiCancelNotSupported;
        RMobilePhone::TMobilePassword iSecUi_password;
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
        	RDEBUG( "StartUp", StartUp );
    
            
            wait->SetRequestType(EMobilePhoneGetSecurityCodeInfo);
            	RDEBUG( "GetSecurityCodeInfo", 0 );
            iPhone.GetSecurityCodeInfo(wait->iStatus, secCodeType, codeInfoPkg);
            	RDEBUG( "WaitForRequestL", 0 );
            res = wait->WaitForRequestL();
            	RDEBUG( "WaitForRequestL res", res );
            User::LeaveIfError(res);

        if(!StartUp)
        {
            // read a flag to see whether the query is SecUi originated. 
            err = RProperty::Get(KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery, secUiOriginatedQuery);
        }

				/* request PIN using QT */
				CSecQueryUi *iSecQueryUi;
					RDEBUG( "CSecQueryUi", 0 );
				iSecQueryUi = CSecQueryUi::NewL();
				// TODO ESecUiCodeEtelReqest/ESecUiNone might be useful
				// TODO also support Emergency
				if(StartUp || (secUiOriginatedQuery != ESecurityUIsSecUIOriginated) || (err != KErrNone))
					lCancelSupported = ESecUiCancelNotSupported;
				else
					lCancelSupported = ESecUiCancelSupported;

				TBuf<0x100> title;	title.Zero();	title.Append(_L("UPin1RequiredL"));	title.Append(_L("#"));	title.AppendNum(codeInfo.iRemainingEntryAttempts);
				queryAccepted = iSecQueryUi->SecQueryDialog( title, iSecUi_password, SEC_C_PIN_CODE_MIN_LENGTH,SEC_C_PIN_CODE_MAX_LENGTH, ESecUiAlphaNotSupported | lCancelSupported | ESecUiCodeEtelReqest );
					RDEBUG( "iSecUi_password", 0 );
				RDebug::Print( iSecUi_password );
				delete iSecQueryUi;
					RDEBUG( "queryAccepted", queryAccepted );
        if ( queryAccepted!=KErrNone )
            { 
            CleanupStack::PopAndDestroy(wait);
           		RDEBUG( "AbortSecurityCode", 0 );
            iPhone.AbortSecurityCode(RMobilePhone::ESecurityUniversalPin);
            	RDEBUG( "AbortSecurityCode", 1 );
            	
            return KErrCancel;
            }
        	RDEBUG( "VerifySecurityCode", 0 );
        iPhone.VerifySecurityCode(wait->iStatus,secCodeType, iSecUi_password, required_fourth);
        	RDEBUG( "WaitForRequestL", 0 );
        res = wait->WaitForRequestL();
        	RDEBUG( "WaitForRequestL res", res );
        CleanupStack::PopAndDestroy(wait);

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
// qtdone
TInt CSecurityHandler::UPukRequiredL()
    {
    TBool wcdmaSupported(FeatureManager::FeatureSupported( KFeatureIdProtocolWcdma ));
    TBool upinSupported(FeatureManager::FeatureSupported( KFeatureIdUpin ));
    if(wcdmaSupported || upinSupported)
       {
        	RDEBUG( "0", 0 );
				TInt queryAccepted = KErrCancel;
        RMobilePhone::TMobilePassword iSecUi_password;
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
        	RDEBUG( "GetSecurityCodeInfo", 0 );
        iPhone.GetSecurityCodeInfo(wait->iStatus, blockCodeType, codeInfoPkg);
        	RDEBUG( "WaitForRequestL", 0 );
        res = wait->WaitForRequestL();
        	RDEBUG( "WaitForRequestL res", res );
        User::LeaveIfError(res);
        //show last "Code Error" note for UPIN verify result so it won't be left under the PUK1 dialog
        if(!StartUp && (codeInfo.iRemainingEntryAttempts == KMaxNumberOfPUKAttempts))
            CSecuritySettings::ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
        
        // ask UPUK code
				/* request PIN using QT */
				{
				CSecQueryUi *iSecQueryUi;
					RDEBUG( "CSecQueryUi", 0 );
				iSecQueryUi = CSecQueryUi::NewL();
				// TODO ESecUiCodeEtelReqest/ESecUiNone might be useful
				// TODO also support Emergency

				TBuf<0x100> title;	title.Zero();	title.Append(_L("Pin1RequiredL"));	title.Append(_L("#"));	title.AppendNum(codeInfo.iRemainingEntryAttempts);
				queryAccepted = iSecQueryUi->SecQueryDialog( title, iSecUi_password, SEC_C_PUK_CODE_MIN_LENGTH,SEC_C_PUK_CODE_MAX_LENGTH, ESecUiAlphaNotSupported | ESecUiCancelSupported | ESecUiPukRequired );
					RDEBUG( "iSecUi_password", 0 );
				RDebug::Print( iSecUi_password );
				delete iSecQueryUi;
					RDEBUG( "queryAccepted", queryAccepted );
       
        if( queryAccepted!=KErrNone )
            {
            CleanupStack::PopAndDestroy(wait);
            return KErrCancel;
            }
        }
      
      	{
				/* request PIN using QT */
				CSecQueryUi *iSecQueryUi;
					RDEBUG( "CSecQueryUi", 0 );
				iSecQueryUi = CSecQueryUi::NewL();
				// TODO ESecUiCodeEtelReqest/ESecUiNone might be useful
				// TODO also support Emergency

				queryAccepted = iSecQueryUi->SecQueryDialog( _L("UPuk-New|UPuk-Verif"), aNewPassword, SEC_C_PUK_CODE_MIN_LENGTH,SEC_C_PUK_CODE_MAX_LENGTH, ESecUiAlphaNotSupported | ESecUiCancelSupported | ESecUiPukRequired );
					RDEBUG( "aNewPassword", 0 );
				RDebug::Print( aNewPassword );
				delete iSecQueryUi;
					RDEBUG( "queryAccepted", queryAccepted );
        if( queryAccepted!=KErrNone )
            {
            CleanupStack::PopAndDestroy(wait);    
            return KErrCancel;
            }
				}

        // send code
        	RDEBUG( "VerifySecurityCode", 0 );
        iPhone.VerifySecurityCode(wait->iStatus,blockCodeType,aNewPassword,iSecUi_password);
        	RDEBUG( "WaitForRequestL", 0 );
        res = wait->WaitForRequestL();
        	RDEBUG( "WaitForRequestL res", res );
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
    else	// not wcdmaSupported || upinSupported
        return KErrNone;
    }

//
// ----------------------------------------------------------
// CSecurityHandler::SimLockEventL()
// Shows "SIM restriction on" note
// ----------------------------------------------------------
// qtdone
void CSecurityHandler::SimLockEventL()
    {
    	RDEBUG( "0", 0 );
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
// qtdone
void CSecurityHandler::ShowGenericErrorNoteL(TInt aStatus)
    {
       // Let's create TextResolver instance for error resolving...
       	RDEBUG( "aStatus", aStatus );
       	RDEBUG( "!!!!! this should never be called !!!!", 0 );
       	
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
