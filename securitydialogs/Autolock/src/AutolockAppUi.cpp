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

#include <apgtask.h>
#include <etelmm.h>
#include <AknEcs.h>
#include <aknnotedialog.h>
#include <aknkeylock.h>
#include <AknLayoutDef.h>
#include <AknLayout.lag>
#include <bldvariant.hrh>
#include <aknappui.h> 
#include <centralrepository.h>
#include <settingsinternalcrkeys.h>
#include <secuisecurityhandler.h>
#include <secui.h>
#include <featmgr.h>
#include <Autolock.rsg>
#include <mmtsy_names.h>
#include <e32property.h>
#include <PSVariables.h>   // Property values
#include <coreapplicationuisdomainpskeys.h>
#include <startupdomainpskeys.h>
#include <ctsydomainpskeys.h>
#include <securityuisprivatepskeys.h>
#include <AknSgcc.h>

#include "AutolockAppUiPS.h"
#include "AutoLockValueObserverPS.h"
#include "AutoLockModelPS.h"
#include "AutolockApp.h"
#include "autolock.hrh"
#include "AutolockView.h"

#include <SCPClient.h>
#include <AknSoftNotifier.h>

// sysap uid and message enums defined in eikon.hrh
// for sending messages to sysap
#include <eikon.hrh>
#include <apgwgnam.h>
#include <aknlayoutscalable_avkon.cdl.h>

#include <AknCapServerDefs.h>
#include <apgtask.h>

//  LOCAL CONSTANTS AND MACROS  
#define KSysApUid TUid::Uid(0x100058F3)
#define KPhoneAppUid TUid::Uid(0x100058B3)

const TInt KTriesToConnectServer( 2 );
const TInt KTimeBeforeRetryingServerConnection( 50000 );
const TInt PhoneIndex( 0 );
const TInt KCancelKeyScanCode( EStdKeyDevice1 ); // 165


// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CAutolockAppUi::ConstructL()
// ?implementation_description
// ----------------------------------------------------------
//
void CAutolockAppUi::ConstructL()
    {
	#if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::ConstructL"));
    #endif
    
    BaseConstructL( EAknEnableSkin | EAknEnableMSK | EAknDisableAnimationBackground );
    
    //Disable priority control so that Autolock process priority isn't set to "background" by 
	//window server when it is not active.
	iEikonEnv->WsSession().ComputeMode( RWsSession::EPriorityControlDisabled ); 
	RThread().SetProcessPriority( EPriorityHigh );

    FeatureManager::InitializeLibL();

	RTelServer::TPhoneInfo PhoneInfo;
	// prevent autolock shutdown
	iEikonEnv->SetSystem( ETrue ); 

	iSideKey1 = 0;
	iSideKey2 = 0;
	iAppKey = 0;

	aCallButtonRect = TRect (0,0,0,0);
	iGotEventDownDuringCall = -1;
	//connect to ETel

	TInt err( KErrGeneral );
    TInt thisTry( 0 );
    
	/*All server connections are tried to be made KTiesToConnectServer times because occasional
    fails on connections are possible at least on some servers*/
	#if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::ConstructL() connect to etel server"));
    #endif
	// connect etel server
	while ( ( err = iServer.Connect() ) != KErrNone && ( thisTry++ ) <= KTriesToConnectServer )
        {
        User::After( KTimeBeforeRetryingServerConnection );
        }
    User::LeaveIfError( err );

	#if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::ConstructL() load tsy"));
    #endif
    // load tsy
    err = iServer.LoadPhoneModule( KMmTsyModuleName );
	#if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::ConstructL() load tsy ERROR: %d"), err);
    #endif
    if ( err != KErrAlreadyExists )
        {
        // may return also KErrAlreadyExists if some other
        // is already loaded the tsy module. And that is
        // not an error.
        User::LeaveIfError( err );
        }
	
	thisTry = 0;
	#if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::ConstructL() open phone"));
    #endif
	//open phone
	User::LeaveIfError(iServer.SetExtendedErrorGranularity(RTelServer::EErrorExtended));
	User::LeaveIfError(iServer.GetPhoneInfo(PhoneIndex, PhoneInfo));
	User::LeaveIfError(iPhone.Open(iServer,PhoneInfo.iName));
    User::LeaveIfError(iCustomPhone.Open(iPhone));
 	#if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::ConstructL() phone opened"));
    #endif

  TBool systemLocked = EFalse;
	TBool phoneLocked = EFalse;

    iWait = NULL;
    iWait = CWait::NewL();

	#ifndef __WINS__


	/*****************************************************
	*	Series 60 Customer / ETEL
	*	Series 60 ETEL API
	*****************************************************/

	// set autolock period to 0, if lock is disabled in DOS side
	#if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::ConstructL() set autolock period to 0"));
    #endif
	RMobilePhone::TMobilePhoneLock lockType = RMobilePhone::ELockPhoneDevice;
	RMobilePhone::TMobilePhoneLockInfoV1 lockInfo;
	RMobilePhone::TMobilePhoneLockInfoV1Pckg lockInfoPkg(lockInfo);
    
	iWait->SetRequestType(EMobilePhoneGetLockInfo);
	iPhone.GetLockInfo(iWait->iStatus, lockType, lockInfoPkg);
	TInt res = iWait->WaitForRequestL();
	User::LeaveIfError(res);
	    
  // Eventhough we might lock the device on boot-up (systemLocked == ETrue), we
  // want to hide the app until the handshake is done. StartUp application will
  // active the app when it is finished.   
  TApaTask self(iCoeEnv->WsSession());
  self.SetWgId(iCoeEnv->RootWin().Identifier());
  self.SendToBackground();
  // flush
  iCoeEnv->WsSession().Flush();	    
		    
    TInt lockValue = 0;
    CRepository* repository = CRepository::NewL(KCRUidSecuritySettings);
    TInt cRresult = repository->Get(KSettingsAutolockStatus, lockValue);
    TBool hiddenReset = HiddenReset();
    #if defined(_DEBUG)
    if(hiddenReset)
        RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::ConstructL() Hidden reset"));
    RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::ConstructL() CR get result: %d"), cRresult);
    RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::ConstructL() CR lock value: %d"), lockValue);
    RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::ConstructL() CR lockInfo.iSetting: %d"), lockInfo.iSetting);    
    #endif
	  if (lockInfo.iSetting == RMobilePhone::ELockSetDisabled)
		{
        repository->Set(KSettingsAutoLockTime, 0);
        if ( FeatureManager::FeatureSupported( KFeatureIdProtocolCdma ) )
            {
            repository->Set(KSettingsLockOnPowerUp, 0);
            }
        }
    // In CDMA, the system can stay locked on after the boot-up sequence.
    else if ( FeatureManager::FeatureSupported( KFeatureIdProtocolCdma ) || (hiddenReset && (lockValue == 1)))
        {
        #if defined(_DEBUG)
        RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::ConstructL() Hidden reset when locked"));
        #endif
        phoneLocked = systemLocked = ETrue;
        }
    else if (lockInfo.iSetting == RMobilePhone::ELockSetEnabled && !hiddenReset) {       
        #if defined(_DEBUG)
        RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::ConstructL() Set phone locked"));
        #endif
        phoneLocked = ETrue;
        }             
 	
     
    delete repository;
	#endif   //__WINS__

	#if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::ConstructL() Enable emergency call support"));
    #endif
	
	#if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::ConstructL() Autolock view"));
    #endif
    


    // -------------------------------------------------------------------------------------------------------------
    // part of emergency call handling when telephony+devicelock is active
    // this solution is meant only for 3.1 and 3.2

    iEcsNote = new (ELeave) CEcsNote();
    iEcsNote->ConstructSleepingNoteL(R_AVKON_EMERGENCY_CALL_NOTE);
    iEcsNote->ButtonGroupContainer().ButtonGroup()->AsControl()->DrawableWindow()->SetOrdinalPosition(0,2);
    
    if (AknLayoutUtils::PenEnabled())	// on touch devices, if Autolock is activated from IdleScreen in landscape, the buttons need to be drawn.
	{
	iEcsNote->ButtonGroupContainer().SetCommandL( 0, _L("") );	// as keyboard is locked, these buttons do nothing. Better to hide their labels.
  	iEcsNote->ButtonGroupContainer().SetCommandL( EAknSoftkeyCancel, _L("") );
	iEcsNote->ButtonGroupContainer().ButtonGroup()->AsControl()->MakeVisible(ETrue);
  	}

	// Emergency call support
    iEcsDetector = CAknEcsDetector::NewL();
    iEcsDetector->SetObserver( this );
	iEmergencySupportReady = ETrue;
    // -------------------------------------------------------------------------------------------------------------
        

	// Autolock view	
	CAutolockView* lockView = new(ELeave) CAutolockView;
    CleanupStack::PushL(lockView);
    lockView->ConstructL();
    CleanupStack::Pop();	// lockView
    AddViewL(lockView);    // transfer ownership to CAknViewAppUi
	SetDefaultViewL(*lockView);

	// start autolock timer
	iModel = CAutoLockModel::NewL(this, phoneLocked);	

	// phone event observer
	iPhoneObserver = CValueObserver::NewL(this);
	//call bubble
	iIncallBubble = CAknIncallBubble::NewL();

	//Autokeyguard Period observer
	#ifdef RD_AUTO_KEYGUARD
	iKeyguardObserver = CAutoKeyguardObserver::NewL(this);
	#else //!RD_AUTO_KEYGUARD
	iKeyguardObserver = NULL;
	#endif //RD_AUTO_KEYGUARD
    // Create the write policy. Also processes with write device data can write the value.
    TSecurityPolicy writePolicy( ECapabilityWriteDeviceData ); 
	// Create the read policy. Also processes with read device data can read the value.	
	TSecurityPolicy readPolicy( ECapabilityReadDeviceData ); 
	
	TInt tRet = RProperty::Define( KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery, RProperty::EInt, readPolicy, writePolicy );
        
    if ( tRet != KErrNone )
        {
        #if defined(_DEBUG)
        RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::ConstructL():\
            FAILED to define the SECUI query Flag: %d"), tRet);
        #endif
        }	
    
    tRet = RProperty::Define( KPSUidSecurityUIs, KSecurityUIsQueryRequestCancel, RProperty::EInt, readPolicy, writePolicy );
    if ( tRet != KErrNone )
        {
        #if defined(_DEBUG)
        RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::ConstructL():\
            FAILED to define the SECUI query request state Flag: %d"), tRet);
        #endif
        }

if(FeatureManager::FeatureSupported(KFeatureIdSapTerminalControlFw ))  
{

    // Define the TARM admin flag.
    
    tRet = RProperty::Define( KSCPSIDAutolock, SCP_TARM_ADMIN_FLAG_UID, RProperty::EInt,
        readPolicy, writePolicy );    
    if ( tRet != KErrNone )
        {
        #if defined(_DEBUG)
        RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::ConstructL():\
            FAILED to define the TARM Admin Flag"));
        #endif
        }
        
    // The following sequence is used to validate the configuration on SCP server.
    // This is needed on the first boot (initial or RFS) or if the C-drive has been formatted
    // (3-button format) and Autolock is not active.

    RSCPClient scpClient;
    if ( scpClient.Connect() == KErrNone )
        {
        TInt confStatus = scpClient.CheckConfiguration( KSCPInitial );
        
        if ( confStatus == KErrAccessDenied )
            {
            #ifndef __WINS__            
            if ( ( lockInfo.iSetting == RMobilePhone::ELockSetDisabled ) )    
            #else // __WINS__                    
            if ( 1 ) // DOS lock is never active in WINS            
            #endif // __WINS__     
		        {					
		        // DOS lock is not active. Note that if DOS is locked, checking the code here will
		        // mess up the query sequence. On initial startup DOS is not locked.		                
                
                TInt finalConfStatus = scpClient.CheckConfiguration( KSCPComplete );
                
                if ( finalConfStatus == KErrAccessDenied )
                    {                
                    #ifdef __WINS__   
                    #if defined(_DEBUG)
                    RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::ConstructL():\
                        DOS validation FAILED in WINS, something wrong!"));
                    #endif                                  
                    #else // !__WINS__                                            

                    // The SCP server is out of sync and Autolock is not active. (c-drive formatted)
                    // We must ask the security code. ( Note that it is very rare that this is executed )
	                #if defined(_DEBUG)
                    RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::ConstructL():\
                        Lock setting disabled, calling setlocksetting"));
                    #endif
                
                    // Wait here until the startup is complete
                
                    TInt tarmErr = KErrNone;
                    while ( tarmErr == KErrNone )
                        {                              
                        TInt sysState=0;
                            tarmErr = RProperty::Get(KPSUidStartup, KPSGlobalSystemState, sysState);

                            if ((sysState == ESwStateNormalRfOn) || (sysState == ESwStateNormalRfOff) 
                                 || (sysState == ESwStateNormalBTSap))
                                {
                                break;
                                }                                        
                        User::After(500000);
                        }
                
                    // Just change the lock setting again to disabled to request the security code.
                    // Set the TARM flag so SecUi knows it should display the "login" query.
	                TInt tarmFlag=0;
	                tRet = RProperty::Get( KSCPSIDAutolock, SCP_TARM_ADMIN_FLAG_UID, tarmFlag );
	                if ( tRet == KErrNone )
    	                {
	                    tarmFlag |= KSCPFlagResyncQuery;
	                    tRet = RProperty::Set( KSCPSIDAutolock, SCP_TARM_ADMIN_FLAG_UID, tarmFlag );
	                    }
	            
	                if ( tRet != KErrNone )
                        {
                        #if defined(_DEBUG)
                        RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::ConstructL():\
                            FAILED to set TARM Admin Flag"));
                        #endif
                        }
	            
    	            RMobilePhone::TMobilePhoneLockSetting lockChange;
	                lockChange = RMobilePhone::ELockSetDisabled;
	                iWait->SetRequestType(EMobilePhoneSetLockSetting);
                    iPhone.SetLockSetting(iWait->iStatus, lockType, lockChange);
                
                    res = iWait->WaitForRequestL();
                    #endif // __WINS__                                 
                    }
                }                        
                                   
            } // if ( confStatus == KErrAccessDenied )
            
        scpClient.Close();               
        }
      
}

    // Eventhough we might lock the device on boot-up (systemLocked == ETrue), we
    // want to hide the app until the handshake is done. StartUp application will
    // active the app when it is finished.
    if( !systemLocked )
        {// app to background
        #if defined(_DEBUG)
        RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::ConstructL() app to background"));
        #endif
        TApaTask self(iCoeEnv->WsSession());
        self.SetWgId(iCoeEnv->RootWin().Identifier());
        self.SendToBackground();
        // flush
        iCoeEnv->WsSession().Flush();      
        }
    else
        {
        #if defined(_DEBUG)
        RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::ConstructL() LOCK SYSTEM"));
        #endif
        TInt lockState = 0;
        
        #ifdef RD_REMOTELOCK
	    lockState = EManualLocked; 	    
	    #else //!RD_REMOTELOCK	    
	    lockState = EAutolockOn; 	    
	    #endif//RD_REMOTELOCK
        iModel->LockSystemL(lockState);  
        }
        
    iGripStatusObserver = CAutolockGripStatusObserver::NewL( this, iEikonEnv->WsSession() ); 
    iFpsStatusObserver = CAutolockFpsStatusObserver::NewL( this, iEikonEnv->WsSession() ); 
    iDeviceLockQueryStatus = EFalse;
    #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::ConstructL()  END"));
    #endif
	}
// ----------------------------------------------------
// CAutolockAppUi::~CAutolockAppUi()
// Destructor
// Frees reserved resources
// ----------------------------------------------------
//
CAutolockAppUi::~CAutolockAppUi()
    {
    #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::~CAutolockAppUi() BEGIN"));
    #endif

    if(iWait)
        {
        // Cancel active requests
        if(iWait->IsActive())
            {
            #if defined(_DEBUG)
	        RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::~CAutolockAppUi() CANCEL REQ"));
	        #endif
            iPhone.CancelAsyncRequest(iWait->GetRequestType());
            }
        }
    //close ETel connections
    if (iCustomPhone.SubSessionHandle())
        iCustomPhone.Close();
    
    if (iPhone.SubSessionHandle())
    	iPhone.Close();
    
    if (iServer.Handle())
        {           
        iServer.UnloadPhoneModule(KMmTsyModuleName);
        iServer.Close();
        }
	delete iModel;
	delete iPhoneObserver;
	delete iIncallBubble;
#ifdef RD_AUTO_KEYGUARD
	delete iKeyguardObserver;
#endif

    delete iEcsDetector;
    delete iEcsNote; // Ecs change
    delete iWait;
    FeatureManager::UnInitializeLib();
    delete iGripStatusObserver;
    delete iFpsStatusObserver;
    #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::~CAutolockAppUi() END"));
    #endif
	}
// ----------------------------------------------------
// CAutolockAppUi::HandleForegroundEventL()
// Handles foreground event.
// ----------------------------------------------------
//
void CAutolockAppUi::HandleForegroundEventL(TBool aForeground)
	{
	#if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::HandleForegroundEventL()"));
    #endif
	if (aForeground )
		{
		if (iLocked)
			{
			// lock voice key 
			LockSideKeyL();	
			CAknView* view = View(KAutoLockViewId);
			if(view)
				{	
		  		TRect aInitialRect;
				STATIC_CAST(CAutolockView*, view)->HandleCall(0x15, aInitialRect);
				}
		
			}
		else
			{
			// app back to background
			TApaTask self(iCoeEnv->WsSession());
			self.SetWgId(iCoeEnv->RootWin().Identifier());
			self.SendToBackground();
			}
		}

	if (!aForeground && iLocked)
		{
        TInt callState=0;
        TInt simStatus=0;
        RProperty::Get(KPSUidCtsyCallInformation, KCTsyCallState, callState );
        RProperty::Get(KPSUidStartup, KPSSimStatus, simStatus);

        if (callState == EPSCTsyCallStateNone && simStatus != ESimNotPresent)
			{	
			// try put autolock back to foreground
			CAknView* view = View(KAutoLockViewId);
			if(view)
				{	
				TRect aInitialRect;
				STATIC_CAST(CAutolockView*, view)->HandleCall(0x19, aInitialRect);
				}
			TApaTask self(iCoeEnv->WsSession());
			self.SetWgId(iCoeEnv->RootWin().Identifier());
			self.BringToForeground();		
			}
		else
			{
			// unlock voice key while there is active call
			UnLockSideKey();
			CAknView* view = View(KAutoLockViewId);
			if(view)
				{	
			  	TRect aInitialRect;
				STATIC_CAST(CAutolockView*, view)->HandleCall(0x16, aInitialRect);
				}
			}
		}

	CAknAppUi::HandleForegroundEventL(aForeground);
	
	}
	

// ----------------------------------------------------
// CAutolockAppUi::HandleMessageL
//
// Handles the TARM command to unlock the phone.
// ----------------------------------------------------
//	
MCoeMessageObserver::TMessageResponse CAutolockAppUi::HandleMessageL( 
                                TUint32 aClientHandleOfTargetWindowGroup, 
                                TUid aMessageUid, 
                                const TDesC8& aMessageParameters 
                               )
    {
    	
    	if(!FeatureManager::FeatureSupported(KFeatureIdSapTerminalControlFw ))  
    	{
    		User::Leave(KErrNotSupported);
    	}
    #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::HandleMessageL()"));
    #endif
    
    MCoeMessageObserver::TMessageResponse messageResponse( EMessageHandled );
    
    if ( aMessageUid.iUid == SCP_CMDUID_UNLOCK )
        {        
        // For security reasons we must check from the SCP server did this 
        // command originate from it.                                
        RSCPClient scpClient;
        if ( scpClient.Connect() == KErrNone )
            {
            CleanupClosePushL( scpClient );
        
            if ( scpClient.QueryAdminCmd( ESCPCommandUnlockPhone ) )
                {
                // Switch autolock to BG
                #if defined(_DEBUG)
                RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::HandleMessageL():\
                    Admin command received, unlocking"));
                #endif            
            
                if ( !iLocked )
                    {
                    #if defined(_DEBUG)
                    RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::HandleMessageL():\
                        Ignoring Unlock message - not locked."));
                    #endif            
                    }   
                else
                    {
                    iLocked = EFalse;
                  DisableWGListChangeEventListening();  
	                UnLockKeys();
	                iModel->SetLockedL(EFalse);
			// Disable keyguard after remote unlock
	                RAknKeyLock iKeyLock;
	                TInt tempResult = iKeyLock.Connect();
	                iKeyLock.DisableKeyLock();
	                // end Disable keyguard after remote unlock
	                SwitchToPreviousAppL();
                    }   
                }           
            else
                {
                #if defined(_DEBUG)
                RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::HandleMessageL():\
                    Unauthorized attempt to unlock"));
                #endif
                }
        
            CleanupStack::PopAndDestroy(); // scpClient                               
            }
        else
            {          
            #if defined(_DEBUG)
            RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::HandleMessageL():\
                Failed to connect to SCP, ignoring unlock-message."));
            #endif            
            
            }
        }               
    else // aMessageUid.iUid != SCP_CMDUID_UNLOCK 
        {
        messageResponse = CAknAppUi::HandleMessageL( aClientHandleOfTargetWindowGroup, 
                                                     aMessageUid,
                                                     aMessageParameters
                                                   );
        }
    
    return messageResponse;
    }
    	 	
 	
// ------------------------------------------------------------------------------
// CAutolockAppUi::::DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane)
//  This function is called by the UIKON framework just before it displays
//  a menu pane. Its default implementation is empty, and by overriding it,
//  the application can set the state of menu items dynamically according
//  to the state of application data.
// ------------------------------------------------------------------------------
//
void CAutolockAppUi::DynInitMenuPaneL(
    TInt /*aResourceId*/,CEikMenuPane* /*aMenuPane*/)
    {
    }

void CAutolockAppUi::HandlePointerEventL(const TPointerEvent& aPointerEvent)
    {
    RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
		}
// ----------------------------------------------------
// CAutolockAppUi::HandleKeyEventL(
//     const TKeyEvent& aKeyEvent,TEventCode /*aType*/)
// ----------------------------------------------------
//
TKeyResponse CAutolockAppUi::HandleKeyEventL(
    const TKeyEvent& aKeyEvent,TEventCode aType)
    {
        
    if ( aKeyEvent.iCode == EKeyBell || (aType == EEventKeyUp && aKeyEvent.iScanCode == EStdKeyDeviceF)  || (aKeyEvent.iCode == EKeyDeviceF) )
		{
		if(iLocked)    
 		    HandleCommandL(ESecUiCmdUnlock);
 		return EKeyWasConsumed; 
 		}

    TBool featureNoPowerkey = FeatureManager::FeatureSupported( KFeatureIdNoPowerkey );
    // If power key pressed, tell SysAp about if
    if( (aKeyEvent.iScanCode == EStdKeyDevice2 && aType == EEventKeyDown )
        || (aType == EEventKey && featureNoPowerkey && aKeyEvent.iCode == EKeyNo))
        {
        SendMessageToSysAp(EEikKeyLockPowerKeyPressed);
        if ( featureNoPowerkey )
            {
            SendMessageToSysAp(EEikKeyLockLightsOnRequest);              
            }
        }
    return EKeyWasNotConsumed;
    }

void CAutolockAppUi::SendMessageToSysAp(TInt aMessage)
    {
    RWsSession& ws = iEikonEnv->WsSession();
    TInt wgId=0;
    CApaWindowGroupName::FindByAppUid(KSysApUid, ws, wgId);
    if (wgId)
        {
        TWsEvent event;
        event.SetType(aMessage);
        event.SetTimeNow();
        ws.SendEventToWindowGroup(wgId, event);
        }
    }

// ----------------------------------------------------
// HideSoftNotification()
// dismiss all the pending notes just before asking the unlocking code
// ----------------------------------------------------
//
void HideSoftNotification()
    {
    #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)HideSoftNotification"));
    #endif
    CAknSoftNotifier *softNotifier = NULL;
    TRAPD (err, {
            softNotifier = CAknSoftNotifier::NewL();
            softNotifier->AddNotificationL(ESetIdleState, 0);
    };)
    delete softNotifier;
    }
// CAutolockAppUi::HandleCommandL(TInt aCommand)
// ----------------------------------------------------
//
void CAutolockAppUi::HandleCommandL(TInt aCommand)
    {
	#if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::HandleCommandL()"));
    #endif
	switch ( aCommand )
        {
        case EEikCmdExit:
			{
			#if defined(_DEBUG)
			RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::HandleCommandL() EEikCmdExit"));
			#endif
            Exit();
            break;
			}
        case ESecUiCmdUnlock:
			{
			#if defined(_DEBUG)
			RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::HandleCommandL() ESecUiCmdUnlock"));
			#endif
			// stop observing emergency call event
			iEmergencySupportReady = EFalse;
			iEcsDetector->Reset(); // Ecs queue is cleared; keys up til here are forgotten
			// ask secuity code
			CSecurityHandler* handler = new (ELeave) CSecurityHandler(iPhone);
			CleanupStack::PushL(handler);
			TSecUi::InitializeLibL();	
	        
	        // Put the lights on when security query is shown
	        SendMessageToSysAp( EEikSecurityQueryLights );
	        HideSoftNotification();	// dismiss all the pending notes just before asking the unlocking code
	        
            TRAPD(err,
			{
			iDeviceLockQueryStatus = ETrue;
			if(handler->AskSecCodeInAutoLockL())
				{		
				iLocked = EFalse;
				DisableWGListChangeEventListening();
				iDeviceLockQueryStatus = EFalse;
				UnLockKeys();
				iModel->SetLockedL(EFalse);
				SwitchToPreviousAppL();
				}
            else
				{  // make sure that we will be topmost still
				    iDeviceLockQueryStatus = EFalse;
                    TInt callState;
                    RProperty::Get( KPSUidCtsyCallInformation, KCTsyCallState, callState );
                if ( callState == EPSCTsyCallStateNone &&
                     !FeatureManager::FeatureSupported( KFeatureIdProtocolCdma ) )
                    {
				    TApaTask self(CCoeEnv::Static()->WsSession());
				    self.SetWgId(CCoeEnv::Static()->RootWin().Identifier());
				    self.BringToForeground();
                    TBool featureNoPowerkey = FeatureManager::FeatureSupported( KFeatureIdNoPowerkey );
                    if ( featureNoPowerkey )
                        {//set lights on in case user pressed "red button". If he pressed cancel the lights are on anyway so it doesn't matter.
                        SendMessageToSysAp(EEikKeyLockLightsOnRequest);              
                        }
				    // we don't want enable lock if call in progress    
                    RProperty::Get( KPSUidCtsyCallInformation, KCTsyCallState, callState );
                    TInt keyguardDisableState(ECoreAppUIsDisableKeyguardUninitialized);
                    //If there is alarm on the keyguard status is set to disabled. In that case don't enable keyguard as it will be done by SysAp 
                    //after the alarm has been disabled/snoozed. Otherwise the alarm CBA is left under keyguard CBA.
                    RProperty::Get( KPSUidCoreApplicationUIs, KCoreAppUIsDisableKeyguard, keyguardDisableState );
                    if ((callState == EPSCTsyCallStateNone) && (keyguardDisableState != ECoreAppUIsDisableKeyguard))
                        {   
				    	RAknKeyLock keylock;
					    if ( keylock.Connect() == KErrNone )
						    {
						    keylock.EnableAutoLockEmulation();
						    keylock.Close();
						    }
					    }
                    }
                }
			};)

			// start observing emergency call event
			iEmergencySupportReady = ETrue;
			CleanupStack::PopAndDestroy(handler); // handler
			TSecUi::UnInitializeLib();  // secui 		
			
			User::LeaveIfError(err);

			break;
			}
        default:
            break;      
        } 
    }

// ----------------------------------------------------
// CAutolockAppUi::SwitchToPreviousAppL()
// Activates previous app 
//----------------------------------------------------
//
void CAutolockAppUi::SwitchToPreviousAppL()
	{
	#if defined(_DEBUG)
	RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::SwitchToPreviousAppL()"));
	#endif
	iEcsDetector->Reset(); // Ecs queue is cleared; keys up til here are forgotten
	// stop observing phone events
	iPhoneObserver->Stop();
	CAknView* view = View(KAutoLockViewId);
	if(view)
	  {
	  STATIC_CAST(CAutolockView*, view)->MakeVisible(EFalse);
	  }
	else
	  {
	  #if defined(_DEBUG)
	  RDebug::Printf( "%s %s (%u) no view ????=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, view );
	  #endif
	  }
	// app to background
	TApaTask self(iCoeEnv->WsSession());
	self.SetWgId(iCoeEnv->RootWin().Identifier());
	self.SendToBackground();
	// flush
	iCoeEnv->WsSession().Flush();
	}
// ----------------------------------------------------
// CAutolockAppUi::BringAppToForegroundL()
// Activates autolock app
//----------------------------------------------------
//
void CAutolockAppUi::BringAppToForegroundL()
	{
	#if defined(_DEBUG)
	RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::BringAppToForegroundL()"));
	#endif
	TBool tarmAdminFlag(EFalse);
	TBool remotelockState(EFalse);
if(FeatureManager::FeatureSupported(KFeatureIdSapTerminalControlFw ))
{
    tarmAdminFlag = TarmState();
}

#ifdef RD_REMOTELOCK
        TInt autolockState;
        RProperty::Get(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, autolockState);
        remotelockState = (autolockState == ERemoteLocked);           
#endif//RD_REMOTELOCK
	// If TARM admin flag is set, bring Autolock to foreground regardless of call state.
	if(!tarmAdminFlag)
	{   //If phone has been remote locked bring Autolock to foreground regardless of call state.
	    if(!remotelockState)
	    {   // check if there is active call
    	    TInt callState;
    	    RProperty::Get( KPSUidCtsyCallInformation, KCTsyCallState, callState );
            if (callState != EPSCTsyCallStateNone )
                {
    		    iModel->SetLockedL(EFalse);
    		    iModel->ResetInactivityTimeout();
    		    UnLockKeys();
    		    return;
    		    }   
	    }
	    
	}
         		

	#ifndef __WINS__

	#ifndef __NO_DOS__

		/*****************************************************
		*	Series 60 Customer / ETEL
		*	Series 60 ETEL API
		*****************************************************/

	// check that device locked in DOS side too
	RMobilePhone::TMobilePhoneLock lockType = RMobilePhone::ELockPhoneDevice;
	RMobilePhone::TMobilePhoneLockInfoV1 lockInfo;
	RMobilePhone::TMobilePhoneLockInfoV1Pckg lockInfoPkg(lockInfo);

	#if defined(_DEBUG)
	RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::BringAppToForegroundL() GetLockInfo"));
	#endif
	iWait->SetRequestType(EMobilePhoneGetLockInfo);
	iPhone.GetLockInfo(iWait->iStatus, lockType, lockInfoPkg);
	TInt res = iWait->WaitForRequestL();
	User::LeaveIfError(res);
	if (iWait->iStatus == KErrNone)
		{
		// if not don't lock ui either
		#if defined(_DEBUG)
		RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::BringAppToForegroundL() KErrNone"));
		#endif
		if (lockInfo.iSetting == RMobilePhone::ELockSetDisabled)
			{
			iModel->SetLockedL(EFalse);
			UnLockKeys();
			return;
			}
		}
	#endif

	#endif

	// in case Telephone-app is topmost, then the user is confused because he sees but can't use it.
	// therefore it's required to hide it, by sending to background. ETMA-7M8A2Y 20090105
	if ( AknLayoutUtils::PenEnabled() )
     	{ 		
   			TApaTaskList apaTaskList(CCoeEnv::Static()->WsSession());

   			TApaTask apaTask = apaTaskList.FindApp(KPhoneAppUid);
   			if (apaTask.Exists())
       	{
			 		#if defined(_DEBUG)
			 			RDebug::Print(_L("(AUTOLOCK)CAknKeyLockControl::EnableKeylock() Bring phone to background"));
			 		#endif
       		// Bring phone to background
       		apaTask.SendToBackground();
       	}
			}	
	
	iPhoneObserver->Start();
	iLocked = ETrue;
	// app to foreground	
	TApaTask self(iCoeEnv->WsSession());
	self.SetWgId(iCoeEnv->RootWin().Identifier());
	self.BringToForeground();
	ActivateLocalViewL(KAutoLockViewId);
	CAknView* view = View(KAutoLockViewId);
	if(view)
	  {
	  TRect aInitialRect;
      STATIC_CAST(CAutolockView*, view)->HandleCall(0x17, aInitialRect);
	  STATIC_CAST(CAutolockView*, view)->MakeVisible(ETrue);
	  }
	else
	  {
	  #if defined(_DEBUG)
	  RDebug::Printf( "%s %s (%u) no view ????=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, view );
	  #endif
	  }
	}

// ----------------------------------------------------
// CAutolockAppUi::LockKeysL()
// locks keys 
//----------------------------------------------------
//
void CAutolockAppUi::LockKeysL()
	{
	#if defined(_DEBUG)
	RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::LockKeysL()"));
	#endif
	// capture appkey & volume key
	if (!iAppKey)
		{
		RWindowGroup& groupWin=iCoeEnv->RootWin();
		iAppKey = groupWin.CaptureKeyUpAndDowns(EStdKeyApplication0, 0, 0); // Capture app key
		}
	LockSideKeyL();
	}
// ----------------------------------------------------
// CAutolockAppUi::UnLockKeys()
// unlocks keys 
//----------------------------------------------------
//
void CAutolockAppUi::UnLockKeys()
	{
	#if defined(_DEBUG)
	RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::UnLockKeysL()"));
	#endif
	// uncapture appkey
	if (iAppKey)
		{
		RWindowGroup& groupWin=iCoeEnv->RootWin();
		groupWin.CancelCaptureKeyUpAndDowns(iAppKey);
		iAppKey = 0;
		}
	UnLockSideKey();
	}
// ----------------------------------------------------
// CAutolockAppUi::LockSideKeyL()
// unlocks side-key 
//----------------------------------------------------
//
void CAutolockAppUi::LockSideKeyL()
	{
	#if defined(_DEBUG)
	RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::LockSideKeyL()"));
	#endif
	if (!iSideKey1)
		{
		RWindowGroup& groupWin=iCoeEnv->RootWin();
		iSideKey1 = groupWin.CaptureKey(EKeyDeviceF,0,0);	// EKeySide -> EKeyDeviceF
		#if defined(_DEBUG)
		RDebug::Printf( "%s %s (%u) capturing EStdKeyDeviceF=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, EStdKeyDeviceF );
		#endif
		iSideKey2 = groupWin.CaptureKeyUpAndDowns(EStdKeyDeviceF, 0, 0);	// EStdKeyDevice6 -> EStdKeyDeviceF
		}
	}

// -------------------------------------------------------------------------------------------------------------
// part of emergency call handling when telephony+devicelock is active
// this solution is meant only for 3.1 and 3.2
void CAutolockAppUi::HandleEcsEvent( CAknEcsDetector* aEcsDetector, CAknEcsDetector::TState aState )
    {
    switch ( aState )
        {
        case CAknEcsDetector::ECompleteMatchThenSendKey:
            // Do nothing since note will be removed on ECallAttempted event
            break;
        case CAknEcsDetector::ECompleteMatch:
            iEcsNote->SetEmergencyNumber( aEcsDetector->CurrentMatch() );
            
            // Tell sysAp to switch lights on
            SendMessageToSysAp( EEikEcsQueryLights );
            
            iEcsNote->ShowNote();
            iEcsNote->iNoteOnScreen =ETrue;
            break;
        case CAknEcsDetector::EPartialMatch:
            iEcsNote->SleepNote();
            break;
        case CAknEcsDetector::ECallAttempted:
            iEcsNote->SleepNote();
            break;
        case CAknEcsDetector::EEmpty:
            iEcsNote->SleepNote();
            break;
        case CAknEcsDetector::ENoMatch:
            iEcsNote->SleepNote();
            break;
        default:
            break;
        }
    }

CEcsNote::CEcsNote() : iNoteOnScreen( EFalse )
    {}
    
void CEcsNote::ConstructSleepingNoteL(TInt aResourceId)
    {
    CAknNoteDialog::ConstructSleepingDialogL(aResourceId);
    }
    
TInt CEcsNote::ShowNote()
    {
    ReportUserActivity();
    iTimeoutInMicroseconds = CAknNoteDialog::EUndefinedTimeout;
    iTone = CAknNoteDialog::ENoTone;    
    if (!iNoteOnScreen)
        {  
        return RouseSleepingDialog();
        }
        // return value not used
    else
        // return value not used
        return NULL;
    }

void CEcsNote::SleepNote()
    {
    if (iNoteOnScreen)
        ExitSleepingDialog(); // Causes flicker to other notes if called when note is not on screen
    iNoteOnScreen = EFalse;
    }

TKeyResponse CEcsNote::OfferKeyEventL(const TKeyEvent& /*aKeyEvent*/, TEventCode /*aType*/)
    {
    return EKeyWasConsumed;
    }

void CEcsNote::SetEmergencyNumber( const TDesC& aMatchedNumber )
    {
    TRect screen(iAvkonAppUi->ApplicationRect());
    TAknLayoutRect mainPane;
    mainPane.LayoutRect(screen, AKN_LAYOUT_WINDOW_main_pane(screen, 0, 1, 1));
    TAknLayoutRect popupNoteWindow;
    AknLayoutUtils::TAknCbaLocation cbaLocation( AknLayoutUtils::CbaLocation() );
    TInt variety( 0 );
    if ( cbaLocation == AknLayoutUtils::EAknCbaLocationRight )
        {
        variety = 5;
        }
    else if ( cbaLocation == AknLayoutUtils::EAknCbaLocationLeft )
        {
        variety = 8;
        }
    else
        {
        variety = 2;
        }

    popupNoteWindow.LayoutRect(mainPane.Rect(), AknLayoutScalable_Avkon::popup_note_window( variety ));
    TAknLayoutText textRect;
    textRect.LayoutText(popupNoteWindow.Rect(), AKN_LAYOUT_TEXT_Note_pop_up_window_texts_Line_1(4));

    // Size of a temporary buffer that contains new lines, spaces and 
    // emergency number for a note.
    TBuf16<KAknEcsMaxMatchingLength+80> number;
    number.Append('\n');
    number.Append('\n');

    TInt spaceCharWidthInPixels = textRect.Font()->CharWidthInPixels(' ');
    if (spaceCharWidthInPixels < 1)
        {
        // Avoid divide by zero situation even the space char would have zero length.
        spaceCharWidthInPixels = 1;
        }
    
    TInt length = (textRect.TextRect().Width() - textRect.Font()->TextWidthInPixels(aMatchedNumber))
                    / spaceCharWidthInPixels;

    const TInt matchedNumberLength = aMatchedNumber.Length();
    const TInt numberLength = number.Length();
    const TInt numberMaxLength = number.MaxLength();
    
    if ( numberLength + length + matchedNumberLength > numberMaxLength)
        {
        // To make sure that buffer overflow does not happen.
        length = numberMaxLength - numberLength - matchedNumberLength;
        }
    for (int i = 0; i < length ; i++)
        {
        number.Append(' ');
        }

    number.Append(aMatchedNumber);
    TRAP_IGNORE(SetTextL(number));

    }
        
// -------------------------------------------------------------------------------------------------------------
	
// ----------------------------------------------------
// CAutolockAppUi::UnLockSideKey()
// unlocks side-key 
//----------------------------------------------------
//
void CAutolockAppUi::UnLockSideKey()
	{
	#if defined(_DEBUG)
	RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::UnLockSideKeyL()"));
	#endif
	if (iSideKey1)
		{
		RWindowGroup& groupWin=iCoeEnv->RootWin();
		groupWin.CancelCaptureKeyUpAndDowns(iSideKey2);
		groupWin.CancelCaptureKey(iSideKey1);
		iSideKey1 = 0;
		iSideKey2 = 0;
		}
	}
	

//
// ---------------------------------------------------------
// CAutolockAppUi::HandleScreenDeviceChangedL()
// Handles screen layout changes, called by CCoeAppUi
// ---------------------------------------------------------
//
void CAutolockAppUi::HandleScreenDeviceChangedL()
{
  CAknAppUiBase::HandleScreenDeviceChangedL();
   //get autolock view from CAknViewAppUi
  CAknView* view = View(KAutoLockViewId);
  if(view)
    {
        STATIC_CAST(CAutolockView*, view)->ScreenDeviceChanged();
    }
  
}

//
// ----------------------------------------------------------
// CAutolockAppUi::IsPinBlocked()
// Checks whether PIN1/UPIN is blocked
// ----------------------------------------------------------
//
TBool CAutolockAppUi::IsPinBlocked()
{
	RMmCustomAPI::TSecurityCodeType secCodeType;
	TBool wcdmaSupported(FeatureManager::FeatureSupported( KFeatureIdProtocolWcdma ));
	TBool upinSupported(FeatureManager::FeatureSupported( KFeatureIdUpin ));

	if(wcdmaSupported || upinSupported)
	  {
    	RMobilePhone::TMobilePhoneSecurityCode activePin;
    	iCustomPhone.GetActivePin(activePin);
    	if(activePin == RMobilePhone::ESecurityUniversalPin)
    		secCodeType = RMmCustomAPI::ESecurityUniversalPin;
    	else
    		secCodeType = RMmCustomAPI::ESecurityCodePin1;
	  }
	else 
	    secCodeType = RMmCustomAPI::ESecurityCodePin1;
	
	TBool isBlocked = EFalse;
    iCustomPhone.IsBlocked(secCodeType,isBlocked);
    #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::IsPinBlocked result: %d"), isBlocked);
    #endif
	return isBlocked;
}

//
// ----------------------------------------------------------
// CAutolockAppUi::HandleWsEventL()
// 
// ----------------------------------------------------------
//
void CAutolockAppUi::HandleWsEventL( const TWsEvent& aEvent,CCoeControl* aDestination )
    {
    	const TInt type = aEvent.Type();
		#if defined(_DEBUG)
	    RDebug::Printf( "%s %s (%u) type=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, type );
	    #endif

    	switch ( type )
    	{
    		case KAknFullOrPartialForegroundLost: // partial or full fg lost
    			if( iIncallBubble )
                    {
                    #if defined(_DEBUG)
    				RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::HandleWsEventL: DISABLE call bubble"));
    				#endif                    
                    iIncallBubble->SetIncallBubbleAllowedInIdleL( EFalse );
                    }
				break;
		    case KAknFullOrPartialForegroundGained: // partial or full fg gained
		    	if( iIncallBubble )
		           	{
		             	#if defined(_DEBUG)
		    			RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::HandleWsEventL: ENABLE call bubble"));
		    			#endif
   						CAknView* view = View(KAutoLockViewId);
							if(view)
								{	
								TRect aInitialRect;
								STATIC_CAST(CAutolockView*, view)->HandleCall(0x1A, aInitialRect);
								}

		                iIncallBubble->SetIncallBubbleAllowedInIdleL( ETrue );
		            }
		    	break;	
    	    case EEventWindowGroupListChanged:
    	        HandleWindowGroupListChange();
    	        break;
    	    case EEventPointer:
    	    case EEventDragDrop:
    	    		{
    	    		TInt callState = 0;
    	    		RProperty::Get( KPSUidCtsyCallInformation, KCTsyCallState, callState );
	    	        TPointerEvent *pointer = aEvent.Pointer();
	    	          CAknView* view = View(KAutoLockViewId);
								  if(view)
								    {	
								        STATIC_CAST(CAutolockView*, view)->ScreenDeviceChanged();
								    	TRect aInitialRect;
								        STATIC_CAST(CAutolockView*, view)->HandleCall(0x1, aInitialRect);
   								    	if(aCallButtonRect.iBr.iX==0)	// initialize if not done already
   								    		aCallButtonRect = TRect (aInitialRect);
								    }
			        if ( callState != EPSCTsyCallStateNone && callState != EPSCTsyCallStateUninitialized )
			        	{
	    	        if(pointer->iType==TPointerEvent::EButton1Down)
	    	        	{
		    	        TPoint iPosition = pointer->iPosition;
									if(aCallButtonRect.iBr.iX<iPosition.iX && iPosition.iX<aCallButtonRect.iBr.iX+aCallButtonRect.iTl.iX && iPosition.iY>400 )
	    		        	{
	    		        	iGotEventDownDuringCall=1;
	    		        	}
	    		        }
	    	        if(pointer->iType==TPointerEvent::EButton1Up)
	    	        	{
		    	        TPoint iPosition = pointer->iPosition;
		    	        // touching at any point inside and below the BigRedButton. This is to handle the case where another dialog overlaps
						if(iGotEventDownDuringCall==1 && aCallButtonRect.iBr.iX<iPosition.iX && iPosition.iX<aCallButtonRect.iBr.iX+aCallButtonRect.iTl.iX && iPosition.iY>400 )
	    		        	{
	    		        		TApaTaskList tasklist( iCoeEnv->WsSession() );
                      TApaTask phonetask = tasklist.FindApp( KPhoneAppUid );
	    		        		if ( phonetask.Exists() )
                        {	// send End key to Telephony to end the call
										    TRawEvent event;
										    event.Set(TRawEvent::EKeyDown, EStdKeyNo);
										    iEikonEnv->WsSession().SimulateRawEvent(event);
                        					User::After(1000);
										    event.Set(TRawEvent::EKeyUp, EStdKeyNo);
										    iEikonEnv->WsSession().SimulateRawEvent(event);
                        }
	    		        	}
	    		        iGotEventDownDuringCall=0;	// even if outside
	    		        }
	    		      }
    		      }
    	        break;
    	    case EEventKeyUp:	// on touch devices, this happens only for the switch-key, which should turn on the lights.
    	    case EEventKey:
    	    case EEventKeyDown:
		    		if(iLocked)
		    				{	// need to capture the switch-key for the case activeCall because Autolock stays on top, even over Akn
		    	    	TKeyEvent *key = aEvent.Key();
		    	    	#if defined(_DEBUG)
		    	    	RDebug::Printf( "%s %s (%u) key->iCode=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, key->iCode );
		    	    	RDebug::Printf( "%s %s (%u) key->iScanCode=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, key->iScanCode );
		    	    	#endif
		    	   		if ( (key->iScanCode == EStdKeyDeviceF) || (key->iCode == EKeyDeviceF) )
		    	   			{
		    	   			#if defined(_DEBUG)
		    	   			RDebug::Printf( "%s %s (%u) good key=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 1 );
		    	   			#endif
			    				RWsSession& ws = iEikonEnv->WsSession();
				        	TApaTaskList tasklist( ws );
				        	TApaTask capserver = tasklist.FindApp( KAknCapServerUid );
				        	if( capserver.Exists() )
				        	    {
		    	   					#if defined(_DEBUG)
				        	    RDebug::Printf( "%s %s (%u) found KAknCapServerUid=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, KAknCapServerUid );
			        	    	#endif
				        	    capserver.SendKey( *key );
				        	    }
		   	        	SendMessageToSysAp( EEikSecurityQueryLights );
									}
								}
    	    	break;
		    default:
		    	iGotEventDownDuringCall=0;	// any other event invalidates the Press inside the BigRedButton
		    	break;
    	}
    	
    	// All events are sent to base class.
    	CAknViewAppUi::HandleWsEventL( aEvent, aDestination );
        
        // part of emergency call handling when telephony+devicelock is active
        // this solution is meant only for 3.1 and 3.2
        // Emergency detector only handles key down events
        if ( iEmergencySupportReady )
        	iEcsDetector->HandleWsEventL( aEvent, aDestination);    	
	#if defined(_DEBUG)
    RDebug::Printf( "%s %s (%u) 0=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    #endif
    }

TBool CAutolockAppUi::DeviceLockQueryStatus()
    {
    return iDeviceLockQueryStatus;
    }

TBool CAutolockAppUi::DeviceLockStatus()
    {
    return iLocked;
    }

void CAutolockAppUi::CancelDeviceLockQuery()
    {
	#if defined(_DEBUG)
    RDebug::Printf( "%s %s (%u) 0=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    #endif
    TRawEvent rawEvent;
    rawEvent.Set( TRawEvent::EKeyDown, KCancelKeyScanCode );
    iCoeEnv->WsSession().SimulateRawEvent( rawEvent );          
    rawEvent.Set( TRawEvent::EKeyUp, KCancelKeyScanCode );
    iCoeEnv->WsSession().SimulateRawEvent( rawEvent );          
    }

TBool CAutolockAppUi::DeviceFpsLock(TInt iStatus)
    {
		if(iStatus)
			HandleCommandL(ESecUiCmdUnlock);
		else
			iModel->SetLockedL(ETimerLocked);
    return ETrue;
    }
void CAutolockAppUi::HandleWindowGroupListChange()
    {
	#if defined(_DEBUG)
    RDebug::Printf( "%s %s (%u) iLocked=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iLocked );
    #endif
    if ( !iLocked )
        {
        // System is not locked, make sure app is not on the foreground.
        if ( !iCoeEnv->RootWin().OrdinalPosition() )
            {
            CAknSgcClient::MoveApp(
                iCoeEnv->RootWin().Identifier(), 
                ESgcMoveAppToBackground );
            }
        }
    else
        {
		            CAknView* view = View(KAutoLockViewId);
								  if(view)
								    {	
								    	TRect aInitialRect;
								        STATIC_CAST(CAutolockView*, view)->HandleCall(0x10, aInitialRect);
												if(aCallButtonRect.iBr.iX==0)	// initialize if not done already
   								    		aCallButtonRect = TRect (aInitialRect);
								    }
        // So now system is locked. When call is not ongoing, autolock should
        // be on the foreground.
        TInt callState = 0;
        RProperty::Get( KPSUidCtsyCallInformation, KCTsyCallState, callState );
        if ( callState == EPSCTsyCallStateNone || 
             callState == EPSCTsyCallStateUninitialized )
            {
            // No calls ongoing.
            if ( iCoeEnv->RootWin().OrdinalPosition() > 0 ) 
                {
                // Not on foreground
                CAknSgcClient::MoveApp(
                    iCoeEnv->RootWin().Identifier(), 
                    ESgcMoveAppToForeground );
                }
            }
        }
    }

TBool CAutolockAppUi::TarmState()
{
TBool ret(EFalse);
if(FeatureManager::FeatureSupported(KFeatureIdSapTerminalControlFw ))
{
    // Get the TARM admin flag value
    TInt tarmFlag;
	TInt tRet = RProperty::Get( KSCPSIDAutolock, SCP_TARM_ADMIN_FLAG_UID, tarmFlag );
    
    if ( tRet != KErrNone )
        {
            #if defined(_DEBUG)
            RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::BringAppToForegroundL(): Warning: failed to get TARM Admin Flag state"));
            #endif
        }
    else
        {
            #if defined(_DEBUG)
            RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::BringAppToForegroundL(): TARM flag: %d"),
                tarmFlag );
            #endif
        }
        
    // Unset the admin flag if set
    if ( tarmFlag & KSCPFlagAdminLock )
        {
        ret = ETrue;
        TInt tRet = RProperty::Get( KSCPSIDAutolock, SCP_TARM_ADMIN_FLAG_UID, tarmFlag );
        
        if ( tRet == KErrNone )
            {
            tarmFlag &= ~KSCPFlagAdminLock;
            tRet = RProperty::Set( KSCPSIDAutolock, SCP_TARM_ADMIN_FLAG_UID, tarmFlag );
            }

        if ( tRet != KErrNone )
            {
            #if defined(_DEBUG)
            RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::BringAppToForegroundL(): FAILED to unset TARM Admin Flag"));
            #endif
            }                    
        }  

}
	#if defined(_DEBUG)
    RDebug::Printf( "%s %s (%u) ret=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, ret );
    #endif
return ret;
}

// ---------------------------------------------------------
// CAutolockAppUi::HiddenReset()
// ---------------------------------------------------------
TBool CAutolockAppUi::HiddenReset()
    {
    #if defined(_DEBUG)
	RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::HiddenReset() begin"));
	#endif	    				    			

    TBool ret_val ( EFalse );
	TInt startupReason(ENormalStartup);
	TInt errorCode(KErrNone);
	
	RProperty::Get(KPSUidStartup, KPSStartupReason, startupReason);
	
	if ( errorCode != KErrNone )
		{
		    #if defined(_DEBUG)
	        RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::HiddenReset() error getting startup reason"));
	        #endif
	    }
    ret_val = (startupReason != ENormalStartup);
    #if defined(_DEBUG)
	RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::HiddenReset() END: %d"), ret_val);
	#endif
    return ret_val;
    }
void CAutolockAppUi::EnableWGListChangeEventListening()
{
	#if defined(_DEBUG)
	RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::EnableWGListChangeEventListening()"));
	#endif
    RWindowGroup& rootWin = iCoeEnv->RootWin();
    rootWin.EnableGroupListChangeEvents();
    rootWin.EnableScreenChangeEvents();
}
void CAutolockAppUi::DisableWGListChangeEventListening()
{
	#if defined(_DEBUG)
	RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::DisableWGListChangeEventListening()"));
	#endif
    RWindowGroup& rootWin = iCoeEnv->RootWin();
    rootWin.DisableGroupListChangeEvents();
    rootWin.DisableScreenChangeEvents();
}
// End of File  
