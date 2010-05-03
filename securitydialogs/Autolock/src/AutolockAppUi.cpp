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

//  LOCAL CONSTANTS AND MACROS  
#define KSysApUid TUid::Uid(0x100058F3)

const TInt KTriesToConnectServer( 2 );
const TInt KTimeBeforeRetryingServerConnection( 50000 );
const TInt PhoneIndex( 0 );

// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CAutolockAppUi::ConstructL()
// ?implementation_description
// ----------------------------------------------------------
//
void CAutolockAppUi::ConstructL()
    {
    	RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    		
	#if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::ConstructL"));
    #endif
    
   	RDebug::Printf( "%s %s (%u) EAutolockOff=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, EAutolockOff );
    RProperty::Set(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, EAutolockOff);

    BaseConstructL( EAknEnableSkin | EAknEnableMSK );
    
	}
// ----------------------------------------------------
// CAutolockAppUi::~CAutolockAppUi()
// Destructor
// Frees reserved resources
// ----------------------------------------------------
//
CAutolockAppUi::~CAutolockAppUi()
    {
    RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
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
  RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
	delete iModel;
	delete iPhoneObserver;
	delete iIncallBubble;
RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
	#ifdef RD_AUTO_KEYGUARD
	delete iKeyguardObserver;
#endif

    delete iEcsDetector;
    delete iEcsNote; // Ecs change
	RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
	  delete iWait;
    FeatureManager::UnInitializeLib();
    delete iGripStatusObserver;
    delete iFpsStatusObserver;
    #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockAppUi::~CAutolockAppUi() END"));
    #endif
RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
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
			TApaTask self(iCoeEnv->WsSession());
			self.SetWgId(iCoeEnv->RootWin().Identifier());
			self.BringToForeground();		
			}
		else
			{
			// unlock voice key while there is active call
			UnLockSideKey();
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
	if ( aKeyEvent.iCode == EKeyBell)
 		{
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
			RDebug::Printf( "%s %s (%u) ESecUiCmdUnlock is not longer handled by Autolock=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, ESecUiCmdUnlock );

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
   			#define KPhoneAppUid TUid::Uid(0x100058B3)

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
		#if defined(_DEBUG)
		RDebug::Printf( "%s %s (%u) searching for popupclock.exe =%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0x0 );
		#endif
		TApaTaskList taskList( CCoeEnv::Static()->WsSession() );
		const TUid KBigClockUid = { 0x2000FDC3 };
		TApaTask task( taskList.FindApp( KBigClockUid ) );
		if ( task.Exists() )
			{
			#if defined(_DEBUG)
			RDebug::Printf( "%s %s (%u) popupclock.exe is running. Not capturing EStdKeyApplication0=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, EStdKeyApplication0 );
			#endif
			}
		else
			{
			#if defined(_DEBUG)
			RDebug::Printf( "%s %s (%u) popupclock.exe not running. Not capturing EStdKeyApplication0=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, EStdKeyApplication0 );
			#endif
			iAppKey = groupWin.CaptureKeyUpAndDowns(EStdKeyApplication0, 0, 0); // Capture app key
			}
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
		iSideKey1 = groupWin.CaptureKey(EKeySide,0,0);
		iSideKey2 = groupWin.CaptureKeyUpAndDowns(EStdKeyDevice6, 0, 0);
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
			        if ( 1==1 || callState == EPSCTsyCallStateNone || callState == EPSCTsyCallStateUninitialized )
			        	{
	    	        TPointerEvent *pointer = aEvent.Pointer();
	    	          CAknView* view = View(KAutoLockViewId);
								  if(view)
								    {	
								        STATIC_CAST(CAutolockView*, view)->ScreenDeviceChanged();
								    	TRect aCallRect;
								        STATIC_CAST(CAutolockView*, view)->HandleCall(1, aCallRect);
   								    	if(aCallButtonRect.iBr.iX==0)
   								    		aCallButtonRect = TRect (aCallRect);
								    }
	    	        if(pointer->iType==TPointerEvent::EButton1Up)
	    	        	{
		    	        TPoint iPosition = pointer->iPosition;
						if(aCallButtonRect.iBr.iX<iPosition.iX && iPosition.iX<aCallButtonRect.iBr.iX+aCallButtonRect.iTl.iX && iPosition.iY<400 )
	    		        	{
	    		        		#define KPhoneAppUid1 TUid::Uid(0x100058B3)
	    		        		TApaTaskList tasklist( iCoeEnv->WsSession() );
                      TApaTask phonetask = tasklist.FindApp( KPhoneAppUid1 );
	    		        		if ( phonetask.Exists() )
                        {
										    TRawEvent event;
										    event.Set(TRawEvent::EKeyDown, EStdKeyNo);
										    iEikonEnv->WsSession().SimulateRawEvent(event);
                        					User::After(1000);
										    event.Set(TRawEvent::EKeyUp, EStdKeyNo);
										    iEikonEnv->WsSession().SimulateRawEvent(event);
                        }
	    		        	}
	    		        }
	    		      }
    		      }
    	        break;
    	    case EEventKeyUp:	// on touch devices, this happens only for the switch-key, which should turn on the lights.
    	    case EEventKey:
    	    case EEventKeyDown:
    		if(iLocked)
	    	        SendMessageToSysAp( EEikSecurityQueryLights );
	    	        break;
		    default:
		    	break;
    	}
    	
    	// All events are sent to base class.
    	CAknViewAppUi::HandleWsEventL( aEvent, aDestination );
        
        // part of emergency call handling when telephony+devicelock is active
        // this solution is meant only for 3.1 and 3.2
        // Emergency detector only handles key down events
        if ( iEmergencySupportReady )
        	iEcsDetector->HandleWsEventL( aEvent, aDestination);    	
    }

TBool CAutolockAppUi::DeviceLockQueryStatus()
    {
    return iDeviceLockQueryStatus;
    }

TBool CAutolockAppUi::DeviceLockStatus()
    {
    return iLocked;
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
								    	TRect aCallRect;
								        STATIC_CAST(CAutolockView*, view)->HandleCall(10, aCallRect);
												if(aCallButtonRect.iBr.iX==0)
   								    		aCallButtonRect = TRect (aCallRect);
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
