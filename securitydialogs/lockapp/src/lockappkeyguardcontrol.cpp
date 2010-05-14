/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Keyguard UI
 *
*/


#include "lockappkeyguardcontrol.h"
#include "lockappstatecontrolinterface.h"
#include "lockappcenrepobserver.h"
#include "lockapppubsubobserver.h"
#include "lockapputils.h"
#include "lockapplockednote.h" // keyguard notes
#include "lockappkeycapturecontroller.h"

#include <avkon.rsg> // general avkon resources
#include <aknnotpi.rsg> // keyguard spesific resources
#include <AknUtils.h>
#include <activitymanager.h>
// this is not needed
// #include <SecondaryDisplay/AknSecondaryDisplayDefs.h> // publishing keyguard notes to secondary display
#include <featmgr.h> // feature manager
#include <eikcba.h> // keyguard soft buttons
#include <eikspane.h>

#include "AutolockPrivateCRKeys.h"
#include <settingsinternalcrkeys.h>
// #include <ScreensaverInternalPSKeys.h>
#include <hwrmdomainpskeys.h>
// #include <activeidle2domainpskeys.h>
//#include <CoreApplicationUIsPrivateCRKeys.h> TODO remove
#include <coreapplicationuisdomainpskeys.h>
#include <ctsydomainpskeys.h>
#include <startupdomainpskeys.h>

// Asterisk key's scan code for the keylock
const TUint KStdKeyAsterisk = 42;
//const TUint KAknChineseAsterisk = 0xFF0A;

// timeout defined in keyguard ui specification custom value
// since avkon note TTimeout values don't support 1 second timeout
const TInt KKeyLockCustomShortTimeout = 1000000;

// Auto Keyguard Off value
const TInt KAutoKeyguardOff( 60000 );

// Flip open value
const TInt KFlipOpen = 1;

// Screensaver started fron idle status value
const TInt KSsStartedFromIdle = 1;

// ---------------------------------------------------------------------------
// Standard Symbian OS construction sequence
// ---------------------------------------------------------------------------
CLockAppKeyguardControl* CLockAppKeyguardControl::NewL(MLockAppStateControl& aStateControl )
    {
    CLockAppKeyguardControl* self = new (ELeave) CLockAppKeyguardControl( aStateControl );
    CleanupStack::PushL( self );
    self->ConstructL( );
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Notes, cba and localized resources are freed in Keyguard UI destruction.
// ---------------------------------------------------------------------------
CLockAppKeyguardControl::~CLockAppKeyguardControl( )
    {
#ifdef RD_AUTO_KEYGUARD
    if ( iActivityManager )
        {
        iActivityManager->Cancel( );
        delete iActivityManager;
        iActivityManager = NULL;
        }
    // CenRep observers
    if ( iCRAutoKeyguardTime )
        {
        delete iCRAutoKeyguardTime;
        iCRAutoKeyguardTime = NULL;
        }
#endif //RD_AUTO_KEYGUARD
    if ( iCRPersistentKeyguardStatus )
        {
        delete iCRPersistentKeyguardStatus;
        iCRPersistentKeyguardStatus = NULL;
        }
    if ( iPSStartupObserver )
        {
        delete iPSStartupObserver;
        iPSStartupObserver = NULL;
        }
    // child notes
    delete iConfirmationNote;
    delete iLockedNote;
    delete iKeypadUnlockedNote;
    delete iKeypadLockedNote;
    }

// ---------------------------------------------------------------------------
// Constructor passes the reference to the main state control.
// ---------------------------------------------------------------------------
CLockAppKeyguardControl::CLockAppKeyguardControl(MLockAppStateControl& aStateControl ) :
    CLockAppBaseControl(aStateControl)
    {
    }

// ---------------------------------------------------------------------------
// Keyguard UI constructor reserves localized resources, configures itself
// using CenRep and FeatureManager and reserves child controls.
// ---------------------------------------------------------------------------
void CLockAppKeyguardControl::ConstructL( )
    {
    INFO( "CLockAppKeyguardControl::ConstructL started" );
    CLockAppBaseControl::ConstructL( );

    // feature manager is used for determining if the phone is a slider
    TBool aFeatureKeypadNoSlider(ETrue);
    FeatureManager::InitializeLibL( );
    aFeatureKeypadNoSlider = FeatureManager::FeatureSupported( KFeatureIdKeypadNoSlider );
    INFO_1("CLockAppKeyguardControl::ConstructL - aFeatureKeypadNoSlider: %d", aFeatureKeypadNoSlider);
    FeatureManager::UnInitializeLib( );

    // keyguard hardware switch support
    CRepository* repository = CRepository::NewLC( KCRUidLockConf );
    TInt hardwareSupport(0);
    repository->Get( KKeyguardHardwareConf, hardwareSupport );
    CleanupStack::PopAndDestroy( repository );
    iHardwareSupport = TLockHardware( hardwareSupport );
    iHardwareSupport = TLockHardware( 0 );

    TBool touchEnabled( AknLayoutUtils::PenEnabled() );
    
    // Cba control
    iCba = CEikButtonGroupContainer::NewL( CEikButtonGroupContainer::ECba,
                                           CEikButtonGroupContainer::EHorizontal,
                                           this,
                                           touchEnabled ? R_AVKON_SOFTKEYS_EMPTY :
                                           R_KEYLOCK_SOFTKEYS_UNLOCK_EMPTY );
    TRect screenRect;
    AknLayoutUtils::LayoutMetricsRect( AknLayoutUtils::EScreen, screenRect );
    iCba->SetBoundingRect( screenRect );
    iCba->MakeVisible( EFalse );

    // Construct Keyguard Notes
    iKeypadLockedNote = new (ELeave) CLockAppLockedNote();
    iKeypadLockedNote->ConstructSleepingNoteL( touchEnabled ? R_KEYLOCK_NOTE_DISPLAY_LOCK_ON_TOUCH :
                                               R_KEYLOCK_NOTE_LOCK_ON );
    iKeypadLockedNote->ButtonGroupContainer().ButtonGroup()->AsControl()->DrawableWindow()->SetOrdinalPosition( 0, 2 );
		// this is not needed
    // iKeypadLockedNote->PublishDialogL( EAknKeysLockedNote );

    iKeypadUnlockedNote = new (ELeave) CLockAppLockedNote();
    iKeypadUnlockedNote->ConstructSleepingNoteL( touchEnabled ? R_KEYLOCK_NOTE_DISPLAY_LOCK_OFF_TOUCH :
                                                 R_KEYLOCK_NOTE_LOCK_OFF );
    iKeypadUnlockedNote->ButtonGroupContainer().ButtonGroup()->AsControl()->DrawableWindow()->SetOrdinalPosition( 0, 2 );
		// this is not needed
    // iKeypadUnlockedNote->PublishDialogL( EAknKeysReleasedNote );

    iLockedNote = new (ELeave) CLockAppLockedNote();
    if ( touchEnabled )
        {
        iLockedNote->ConstructSleepingNoteL( R_KEYLOCK_NOTE_DISPLAY_LOCKED_TOUCH );
        }
    else
        {
        switch ( iHardwareSupport )
            {
            case EKeyguardOnePositionSwitch:
            case EKeyguardTwoPositionSwitch:
                {
                iLockedNote->ConstructSleepingNoteL( R_KEYLOCK_NOTE_KEYLOCKED_SWITCH );
                break;
                }
            case EKeyguardDefaultHardware:
            default:
                {
                iLockedNote->ConstructSleepingNoteL( R_KEYLOCK_NOTE_KEYLOCKED );
                break;
                }
            }
        }
    iLockedNote->ButtonGroupContainer().ButtonGroup()->AsControl()->DrawableWindow()->SetOrdinalPosition( 0, 2 );

    // These are created only if touch is not enabled, since
    // currently in touch devices the keylock state is controlled
    // with switch.
    if ( !touchEnabled )
        {
        //Note showing "Now Press *" - when user has pressed "Unlock" in locked state.
        if ( aFeatureKeypadNoSlider )
            {
            // for normal phones
            iConfirmationNote = new (ELeave) CLockAppLockedNote();
            iConfirmationNote->ConstructSleepingNoteL( R_KEYLOCK_NOTE_UNLOCK_ASTERISK );
            }
        else
            {
            // for special slider phones
            iConfirmationNote = new (ELeave) CLockAppLockedNote();
            iConfirmationNote->ConstructSleepingNoteL( R_KEYLOCK_NOTE_UNLOCK_CONFIRM );
            }
        iConfirmationNote->ButtonGroupContainer().ButtonGroup()->AsControl()->DrawableWindow()->SetOrdinalPosition( 0, 2 );
        
        }
    
    
    // CenRep observers
    // TODO create private CR key for storing persistent keyguard state (currently in Sysap)
    // iCRPersistentKeyguardStatus = CLockAppCenRepObserver::NewL ( this, KCRUidCoreApplicationUIsSysAp, KSysApKeyguardActive );

#ifdef RD_AUTO_KEYGUARD
    iCRAutoKeyguardTime = CLockAppCenRepObserver::NewL ( this, KCRUidSecuritySettings, KSettingsAutomaticKeyguardTime );
    // Activity manager
    iActivityManager = CUserActivityManager::NewL( CActive::EPriorityStandard );
    StartActivityMonitoringL( );
#endif //RD_AUTO_KEYGUARD

    // PubSub observers
    iPSStartupObserver = CLockAppPubSubObserver::NewL( this, KPSUidStartup, KPSGlobalSystemState );

    // Setup key pattern matcher
    if ( !SetupKeyPatternsWithPolicyL( EPolicyDeactivateKeyguard ) )
        {
        INFO( "CLockAppKeyguardControl::ConstructL - No CenRep policy defined" );
        if ( aFeatureKeypadNoSlider )
            {
            iKeyPattern->AddPattern( EStdKeyDevice0, KStdKeyAsterisk ); // LSK + *
            }
        else
            {
            iKeyPattern->AddPattern( EStdKeyDevice0, EStdKeyDevice1 ); // LSK + RSK
            }
        }
#ifdef __WINS__
    // In Emulator add the LSK+* pattern also.
    iKeyPattern->AddPattern( EStdKeyDevice0, EStdKeyNkpAsterisk ); // LSK + *
#endif

    INFO( "CLockAppKeyguardControl::ConstructL completed" );
    }

// ---------------------------------------------------------------------------
// Check weather its allowed to automatically lock the keys
// ---------------------------------------------------------------------------
TBool CLockAppKeyguardControl::AutoActivationAllowedL( )
    {
#ifdef RD_AUTO_KEYGUARD
    TInt value = 0;
    TBool flipOpen(EFalse);
    RProperty::Get( KPSUidHWRM, KHWRMFlipStatus, value );
    flipOpen = (value = KFlipOpen);
    INFO_1("CLockAppKeyguardControl::AutoActivationAllowedL - flipOpen: %d", flipOpen);
    if ( flipOpen )
        {
        CRepository* repository = CRepository::NewL( KCRUidAutolockConf );
        repository->Get( KAutoKeyLockConf, value );
        delete repository;
        if ( value & KAutoKeylockFeatureIdFlipOpenDisabled )
            {
            INFO( "CLockAppKeyguardControl::AutoActivationAllowedL : False because flipOpenDisabled" );
            return EFalse;
            }
        }

    TInt lightStatus=EForcedLightsUninitialized; 
    RProperty::Get(KPSUidCoreApplicationUIs,KLightsVTForcedLightsOn,lightStatus ); 
    INFO_1("CLockAppKeyguardControl::AutoActivationAllowedL - lightStatus: %d", lightStatus);
    if ( lightStatus == EForcedLightsOn )
	{
	INFO( "CLockAppKeyguardControl::AutoActivationAllowedL : False because EForcedLightsOn" );
	return EFalse;
	}

    TBool keysLocked(EFalse);
    TBool idle(EFalse);
    TBool ongoingCall(EFalse);
    TBool screenSaverOn(EFalse);
    TBool screenSaverStertedFromIdle(EFalse);

    keysLocked = (iStateControl.LockStatus() != ELockNotActive);
    INFO_1("CLockAppKeyguardControl::AutoActivationAllowedL - keysLocked: %d", keysLocked);
    value = 0;
    RProperty::Get( KPSUidCtsyCallInformation, KCTsyCallState, value );
    ongoingCall = (value > EPSCTsyCallStateNone);
    INFO_2("CLockAppKeyguardControl::AutoActivationAllowedL - ongoingCall: %d %d", value, ongoingCall);
    value = 0;
    // RProperty::Get( KPSUidAiInformation, KActiveIdleState, value );
    // idle = (value == EPSAiForeground);
    idle = ETrue; // don't care about idle state. Phone should autolock on any UI, not only HomeSreeen.
    INFO_2("CLockAppKeyguardControl::AutoActivationAllowedL - idle: %d %d", value, idle);
    value = 0;
		/* This is not used any more because screensavers are removed now
    RProperty::Get( KPSUidScreenSaver, KScreenSaverOn, value );
    screenSaverOn = (value > 0);
    */
    screenSaverOn = ETrue;
    
    INFO_2("CLockAppKeyguardControl::AutoActivationAllowedL - screenSaverOn: %d %d", value, screenSaverOn);
    value = 0;
		/* This is not used any more because screensavers are removed now
    RProperty::Get( KPSUidScreenSaver, KScreenSaverActivatedFromIdle, value );
    screenSaverStertedFromIdle = (value == KSsStartedFromIdle);
    */
    screenSaverStertedFromIdle = ETrue;
    
    INFO_2("CLockAppKeyguardControl::AutoActivationAllowedL - screenSaverStertedFromIdle: %d %d", value, screenSaverStertedFromIdle);

    // If a call is ongoing or idle doesnt have foreground and
    // screensaver is not started from idle -> dont lock keys
    if ( keysLocked || ongoingCall || (!idle && !(screenSaverOn && screenSaverStertedFromIdle)) )
        {
        INFO("CLockAppKeyguardControl::AutoActivationAllowedL : False");
        return EFalse;
        }
#endif //RD_AUTO_KEYGUARD
    INFO("CLockAppKeyguardControl::AutoActivationAllowedL : True");
    return ETrue;
    }

// ---------------------------------------------------------------------------
// Check weather its allowed to activate the control
// ---------------------------------------------------------------------------
TBool CLockAppKeyguardControl::ActivationAllowedL( )
    {
    return ETrue;
    }

// ---------------------------------------------------------------------------
// Check weather its allowed to deactivate the control
// ---------------------------------------------------------------------------
TBool CLockAppKeyguardControl::DeActivationAllowedL( )
    {
    return ETrue;
    }

// ---------------------------------------------------------------------------
// Activate control
// ---------------------------------------------------------------------------
void CLockAppKeyguardControl::HandleActivateEventL( TUint aEnvMask )
    {    
    INFO_1("CLockAppKeyguardControl::HandleActivateEventL - aEnvMask: %x", aEnvMask);
    
    CLockAppBaseControl::HandleActivateEventL( aEnvMask );

    if ( IsBitFieldSet( aEnvMask, KLockAppEnvScreenSaverOn ) )
        {
        // if screensaver is on - capture primary keys
        CapturePrimaryKeys( ETrue );
        }

    // capture keys
    CLockAppKeyCaptureController::CaptureKey( EStdKeyApplication0, EKeyApplication0, EKeyCaptureAllEvents ); // App key
    CLockAppKeyCaptureController::CaptureKey( EStdKeyDevice2, EKeyDevice2, EKeyCaptureAllEvents ); // Power key (for lights)
    CLockAppKeyCaptureController::CaptureKey( EStdKeyDevice6, EKeyDevice6, EKeyCaptureAllEvents ); // Voice key (for lights)
    CLockAppKeyCaptureController::CaptureKey( EStdKeyNo, EKeyNo, EKeyCaptureAllEvents ); // End key (for Rosetta lights)

    SetPointerEventCapture( ETrue );
    SetKeyguardIndicatorStateL( ETrue );
    ShowCba( ETrue );
    // close task-list in case it is displayed : fast-swap window
    iEikonEnv->DismissTaskList( );
    if ( iCba )
        {
        TBool areWeInIdleState = CEikStatusPaneBase::Current()->PaneCapabilities(TUid::Uid(EEikStatusPaneUidClock)).IsInCurrentLayout( );
        if ( areWeInIdleState )
            {
            (static_cast<CEikCba*>(iCba->ButtonGroup()))->SetSkinBackgroundId( KAknsIIDQsnBgAreaControlIdle );
            }
        else
            {
            (static_cast<CEikCba*>(iCba->ButtonGroup()))->SetSkinBackgroundId( KAknsIIDQsnBgAreaControl );
            }
        }

    if ( iCRPersistentKeyguardStatus )
        {
        iCRPersistentKeyguardStatus->SetValue( 1 );
        }
    }

// ---------------------------------------------------------------------------
// DeActivate control
// ---------------------------------------------------------------------------
void CLockAppKeyguardControl::HandleDeActivateEventL( TUint aEnvMask )
    {
    INFO_1("CLockAppKeyguardControl::HandleDeActivateEventL - aEnvMask: 0x%x", aEnvMask);
    
    CLockAppBaseControl::HandleDeActivateEventL( aEnvMask );

    if ( IsBitFieldSet( aEnvMask, KLockAppEnvScreenSaverOn ) )
        {
        // if screensaver is on - uncapture primary keys
        CapturePrimaryKeys( EFalse );
        }

    // phonecall or not - uncapture keys anyway
    CLockAppKeyCaptureController::ReleaseKey( EStdKeyApplication0 );
    CLockAppKeyCaptureController::ReleaseKey( EStdKeyDevice2 );
    CLockAppKeyCaptureController::ReleaseKey( EStdKeyDevice6 );
    CLockAppKeyCaptureController::ReleaseKey( EStdKeyNo );

    SetPointerEventCapture( EFalse );
    SetKeyguardIndicatorStateL( EFalse );
    ShowCba( EFalse );

    if ( iCRPersistentKeyguardStatus )
        {
        iCRPersistentKeyguardStatus->SetValue( 0 );
        }
    }

// ---------------------------------------------------------------------------
// Handle environment changes (Screensaver, Telephony, etc.)
// ---------------------------------------------------------------------------
void CLockAppKeyguardControl::HandleEnvironmentChange( TUint aEnvMask, TUint aEventMask )
    {
    if ( IsBitFieldSet( aEventMask, KLockAppEnvScreenSaverOn ) )
        {
        // screen saver state changed
        CapturePrimaryKeys( IsBitFieldSet( aEnvMask, KLockAppEnvScreenSaverOn ) );
        }
    if ( IsBitFieldSet( aEventMask, KLockAppEnvFPS ) )
        {
    		// iStateControl.DisableKeyguardL( ETrue );
    		// iStateControl.LockStatus() != ELockNotActive		// no need to check, because keyguard will catch fingerprint only if not locked
    		iStateControl.EnableDevicelockL( EDevicelockManual );
        }
    }

// ---------------------------------------------------------------------------
// Inform the user that keys are locked (i.e. "Please press LSK+'*' to unlock).
// ---------------------------------------------------------------------------
void CLockAppKeyguardControl::DisplayLockedNote( )
    {
    ShowNote( iLockedNote, CAknNoteDialog::ELongTimeout, CAknNoteDialog::ENoTone );
    }

// ---------------------------------------------------------------------------
// Handle all Central Repository observer callbacks.
// ---------------------------------------------------------------------------
void CLockAppKeyguardControl::HandleCenRepNotify(TUid aCenRepUid, TUint32 aKeyId, TInt aValue )
    {
    if ( aCenRepUid == KCRUidSecuritySettings )
        {
        switch ( aKeyId )
            {
            case KSettingsAutomaticKeyguardTime:
                {
                INFO_1( "CLockAppKeyguardControl::HandleCenRepNotify - KSettingsAutomaticKeyguardTime = %d", aValue );
                ResetInactivityTimeout( );
                }
                break;
            default:
                break;
            }
        }
    }

// ---------------------------------------------------------------------------
// Handle all Publish & Subscribe observer callbacks.
// ---------------------------------------------------------------------------
void CLockAppKeyguardControl::HandlePubSubNotify(TUid aPubSubUid, TUint aKeyId, TInt aValue )
    {
    if ( aPubSubUid == KPSUidStartup )
        {
        switch ( aKeyId )
            {
            case KPSGlobalSystemState:
                {
                // In case of unexpected reset (e.g. hidden boot) the keylock must be enabled silently.
                if ( !iAlreadyNormalState && (aValue == ESwStateNormalRfOn || aValue == ESwStateNormalRfOff) )
                    {
                    iAlreadyNormalState = ETrue;
                    TInt keylockWasEnabled = 0;
                    if ( iCRPersistentKeyguardStatus )
                        {
                        iCRPersistentKeyguardStatus->GetValue( keylockWasEnabled );
                        if ( keylockWasEnabled )
                            {
                            TRAPD(err, iStateControl.EnableKeyguardL( EFalse ));
                            ERROR(err, "CLockAppKeyguardControl::HandlePubSubNotify - EnableKeyguardL");
                            }
                        }
                    }
                }
                break;
            default:
                break;
            }
        }
    INFO_3( "CLockAppKeyguardControl::HandlePubSubNotify %x %x = %x", aPubSubUid.iUid, aKeyId, aValue );
    }

// ---------------------------------------------------------------------------
// Show that keys are locked.
// ---------------------------------------------------------------------------
void CLockAppKeyguardControl::DisplayKeysLockedNote( )
    {
    ShowNote( iKeypadLockedNote, KKeyLockCustomShortTimeout, CAknNoteDialog::ENoTone );
    }

// ---------------------------------------------------------------------------
// Show that keys are unlocked.
// ---------------------------------------------------------------------------
void CLockAppKeyguardControl::DisplayKeysActiveNote( )
    {
    ShowNote( iKeypadUnlockedNote, KKeyLockCustomShortTimeout, CAknNoteDialog::ENoTone );
    }

// ---------------------------------------------------------------------------
// Show confirmation note when user has pressed "Unlock".
// ---------------------------------------------------------------------------
void CLockAppKeyguardControl::DisplayConfirmationNote( )
    {
    ShowNote( iConfirmationNote, CAknNoteDialog::EShortTimeout, CAknNoteDialog::EConfirmationTone );
    // inform sysap to put lights on left soft key press
    SendMessageToSysAp( EEikKeyLockLightsOnRequest );
    }

// ---------------------------------------------------------------------------
// Keyguard UI key events are handled trough here.
// ---------------------------------------------------------------------------
TKeyResponse CLockAppKeyguardControl::OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType )
    {
    RDebug::Printf( "%s %s (%u) aKeyEvent.iCode=%x aType=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aKeyEvent.iCode, aType );
    RDebug::Printf( "%s %s (%u) iActive=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iActive );
    if ( iActive )
        {
        		if(AknLayoutUtils::PenEnabled())
        			{
        			if(aKeyEvent.iCode == EKeyDeviceF)	// any Type
        				{
        				iStateControl.DisableKeyguardL( ETrue );
        				}
        			}
        if ( aType == EEventKeyDown )
            {
            switch ( iKeyPattern->HandleKeyEvent( aKeyEvent.iScanCode ) )
                {
                case EPatternNoMatch:
                    DisplayLockedNote( );
                    break;
                case EPatternPrimaryMatch:
                    DisplayConfirmationNote( );
                    break;
                case EPatternSecondaryMatch:
                    iStateControl.DisableKeyguardL( ETrue );
                    break;
                default:
        						RDebug::Printf( "%s %s (%u) default=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 1 );
                    break;
                }
            }
        }
    return EKeyWasConsumed;
    }

// ---------------------------------------------------------------------------
// Get autokeyguard timeout (in seconds)
// @return 0 means Off
// ---------------------------------------------------------------------------
TInt CLockAppKeyguardControl::GetAutoKeyguardTimeout( )
    {
    TInt timeoutInSeconds( 0 );
#ifdef RD_AUTO_KEYGUARD
    iCRAutoKeyguardTime->GetValue( timeoutInSeconds );
#endif //RD_AUTO_KEYGUARD
    return timeoutInSeconds;
    }

// ----------------------------------------------------------
// Starts monitoring user activity
// ----------------------------------------------------------
void CLockAppKeyguardControl::StartActivityMonitoringL( )
    {
#ifdef RD_AUTO_KEYGUARD
    __ASSERT_DEBUG( iActivityManager, DoPanic(ELockIllegalState));
    if ( iActivityManager && !iActivityManager->IsActive() )
        {
        TInt value = GetAutoKeyguardTimeout( );
        INFO_1( "CLockAppKeyguardControl::StartActivityMonitoringL - %d sec", value);
        if ( value )
            {
            iActivityManager->Start( value,
                    TCallBack( HandleInactiveEventL, this ),
                    TCallBack( HandleActiveEventL, this ) );
            }
        else
            {
            iActivityManager->Start( KAutoKeyguardOff,
                    TCallBack( HandleInactiveEventL, this ),
                    TCallBack( HandleActiveEventL, this ) );
            }
        }
#endif //RD_AUTO_KEYGUARD
    }

// ----------------------------------------------------------
// Gets keyguard period and starts monitoring user activity
// ----------------------------------------------------------
void CLockAppKeyguardControl::ResetInactivityTimeout( )
    {
#ifdef RD_AUTO_KEYGUARD
    __ASSERT_DEBUG( iActivityManager, DoPanic(ELockIllegalState));
    if ( iActivityManager )
        {
        TInt value = GetAutoKeyguardTimeout( );
        INFO_1( "CLockAppKeyguardControl::ResetInactivityTimeout - %d sec", value);
        if ( value )
            {
            iActivityManager->SetInactivityTimeout( value );
            }
        else
            {
            iActivityManager->SetInactivityTimeout( KAutoKeyguardOff );
            }
        }
#endif //RD_AUTO_KEYGUARD
    }

// ----------------------------------------------------------
// Stop monitoring user activity.
// ----------------------------------------------------------
void CLockAppKeyguardControl::StopActivityMonitoring( )
    {
#ifdef RD_AUTO_KEYGUARD
    if ( iActivityManager )
        {
        iActivityManager->Cancel( );
        }
#endif //RD_AUTO_KEYGUARD
    }

// ----------------------------------------------------------
// Handle Active event. Called by ActivityManager
// ----------------------------------------------------------
TInt CLockAppKeyguardControl::HandleActiveEventL(TAny* /*aPtr*/)
    {
    return KErrNone;
    }

// ----------------------------------------------------------
// Handles InActive event. Called by ActivityManager
// ----------------------------------------------------------
TInt CLockAppKeyguardControl::HandleInactiveEventL(TAny* aPtr )
    {
#ifdef RD_AUTO_KEYGUARD
    CLockAppKeyguardControl* keyguard = STATIC_CAST(CLockAppKeyguardControl*, aPtr);
    if ( keyguard->GetAutoKeyguardTimeout( ) && keyguard->AutoActivationAllowedL( ) )
        {
        keyguard->iStateControl.EnableKeyguardL( EFalse );
        }
#endif //RD_AUTO_KEYGUARD
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Handle UI commands received from the child controls
// ---------------------------------------------------------------------------
void CLockAppKeyguardControl::ProcessCommandL( TInt aCommandId )
    {
    INFO_1("CLockAppKeyguardControl::ProcessCommandL : %d ", aCommandId );
    }

TInt CLockAppKeyguardControl::CountComponentControls( ) const
    {
    return 1;
    }

CCoeControl* CLockAppKeyguardControl::ComponentControl(TInt /*aIndex*/) const
    {
    return iCba;
    }

// ---------------------------------------------------------------------------
// Notification if layout changes.
// ---------------------------------------------------------------------------
void CLockAppKeyguardControl::HandleResourceChange(TInt aType )
    {
    if ( aType == KEikDynamicLayoutVariantSwitch && iCba )
        {
        TRect screenRect;
        AknLayoutUtils::LayoutMetricsRect( AknLayoutUtils::EScreen, screenRect );
        iCba->SetBoundingRect( screenRect );
        }
    CCoeControl::HandleResourceChange( aType );
    }

// END OF FILE
