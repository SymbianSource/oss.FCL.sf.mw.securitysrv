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
* Description:  Controls lock states (unlocked, keylock, devicelock)
 *
*/


#include "lockappstatecontrol.h"

// child controls
#include "lockappidlecontrol.h"
#include "lockappkeyguardcontrol.h"
#include "lockappdevicelockcontrol.h"

#include "lockapppubsubobserver.h"
#include "lockappcenrepobserver.h"

#include "lockappkeycapturecontroller.h"
#include "lockappstatepublisher.h"
#include "lockappecsdetector.h"
#include "lockapputils.h"
#include <featmgr.h>

#include <settingsinternalcrkeys.h>
// #include <ScreensaverInternalPSKeys.h>
#include <ctsydomainpskeys.h>
#include <activeidle2domainpskeys.h>

#include "GlobalWindowPriorities.h"

#include <avkon.rsg> // general avkon resources
#include <aknnotpi.rsg> // keyguard spesific resources
#include <aknsoundsystem.h>
#include <AknIncallBubbleNotify.h> // incall bubble
#include <bautils.h> // BaflUtils
#include <apgwgnam.h>

#include <keylockpolicyapi.h>

#include <hwrmdomainpskeys.h>
    TUid KUidFpsCategory = {0x1020507E};
const		TInt KFpsAuthenticationKey = 1;

// used avkon dialog resources
_LIT(KRESOURCEFILE, "z:\\resource\\aknnotpi.rsc");

// Visibility gate WG priority's relative offset from ECoeWinPriorityAlwaysAtFront
// Now equals -1000 to be an absolute 0.
const TInt KGlobalWindowPriority_LockVisibilityGate = -1000;

// ---------------------------------------------------------------------------
// Standard Symbian OS construction sequence
// ---------------------------------------------------------------------------
CLockAppStateControl* CLockAppStateControl::NewL( )
    {
    CLockAppStateControl* self = new (ELeave) CLockAppStateControl();
    CleanupStack::PushL( self );
    self->ConstructL( );
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destruction releases key captures.
// ---------------------------------------------------------------------------
CLockAppStateControl::~CLockAppStateControl( )
    {
    // localized resources are freed
    if ( iResourceFileOffset )
        {
        // We implicitely trust that eikon env exists (though in practice it does not make the
        // difference as this destructor is really never called...)
        iEikonEnv->DeleteResourceFile(iResourceFileOffset);
        }

    // PuSub observers
    if ( iPSScreenSaverObserver )
        {
        delete iPSScreenSaverObserver;
        iPSScreenSaverObserver = NULL;
        }

    if ( iPSTelephonyObserver )
        {
        delete iPSTelephonyObserver;
        iPSTelephonyObserver = NULL;
        }

    if ( iPSGripObserver )
        {
        delete iPSGripObserver;
        iPSGripObserver = NULL;
        }

    if ( iPSFPSObserver )
        {
        delete iPSFPSObserver;
        iPSFPSObserver = NULL;
        }

    CLockAppKeyCaptureController::Destroy( );

    if ( iIncallBubble )
        {
        delete iIncallBubble;
        iIncallBubble = NULL;
        }

    if ( iSoundsMuted )
        {
        MuteSounds( EFalse );
        }
    }

// ---------------------------------------------------------------------------
// The uninitialized status is ELockNoteActive.
// ---------------------------------------------------------------------------
CLockAppStateControl::CLockAppStateControl( ) :
    iLockStatus( ELockNotActive),
    iWGEventGate( iEikonEnv->RootWin()),
    iEnvState( 0 )
    {
    // no implementation required
    }

// ---------------------------------------------------------------------------
// All lock state observers are created here.
// ---------------------------------------------------------------------------
void CLockAppStateControl::ConstructL( )
    {
    INFO( "CLockAppStateControl::ConstructL started" );

    // creates observer list
    BaseConstructL( );

    // localized resources are loaded
    TFileName resourceFile;
    resourceFile.Append( KRESOURCEFILE );
    BaflUtils::NearestLanguageFile( iEikonEnv->FsSession(), resourceFile );
    iResourceFileOffset = iEikonEnv->AddResourceFileL(resourceFile);

    // fetch power key configuration from feature manager
    FeatureManager::InitializeLibL( );
    iFeatureNoPowerkey = FeatureManager::FeatureSupported( KFeatureIdNoPowerkey );
    FeatureManager::UnInitializeLib( );

    // allow autoforwarding for the application (stacked bringforwards)
    iEikonEnv->SetAutoForwarding( ETrue );

    CLockAppKeyCaptureController::InitL( iEikonEnv->RootWin() );

    // Create second windowgroup for visibility gate
    CreateVisibilityGateWgL( );

    INFO_1( "CLockAppStateControl - EventGate WgID = %d", iWGEventGate.WindowGroupId() );
    INFO_1( "CLockAppStateControl - VisibilityGate WgID = %d", iWGVisibilityGate.WindowGroupId() );

    //
    // Construct Controls
    //
    iIdle = CLockAppIdleControl::NewL( *this );
    AddObserverL( iIdle ); // ownership is transfered

    iCurrentControl = iIdle;
    iCurrentControl->HandleActivateEventL( iEnvState );

    iKeyguard = CLockAppKeyguardControl::NewL( *this );
    AddObserverL( iKeyguard ); // ownership is transfered
    
    RDebug::Printf( "%s %s (%u) iDevicelock=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iDevicelock );
    iDevicelock = CLockAppDevicelockControl::NewL( *this, iWGVisibilityGate ); 
    RDebug::Printf( "%s %s (%u) iDevicelock=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iDevicelock );
    iDevicelock->ConnectToPhoneL( iWGVisibilityGate );
    RDebug::Printf( "%s %s (%u) iDevicelock=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iDevicelock );
    AddObserverL( iDevicelock ); // ownership is transfered

    // Emergency support
    iLockEcsDetector = CLockAppEcsDetector::NewL( );
    AddObserverL( iLockEcsDetector ); // ownership is transfered

    // Lock state publisher
    CLockAppStatePublisher* lockPublisher = CLockAppStatePublisher::NewL( );
    AddObserverL( lockPublisher ); // ownership is transfered

    // PubSub observers
		/* This is not used any more because screensavers are removed now
    iPSScreenSaverObserver = CLockAppPubSubObserver::NewL( this, KPSUidScreenSaver, KScreenSaverOn );
    */
    iPSTelephonyObserver = CLockAppPubSubObserver::NewL( this, KPSUidCtsyCallInformation, KCTsyCallState );
    iPSGripObserver = CLockAppPubSubObserver::NewL( this, KPSUidHWRM, KHWRMGripStatus );

    iPSFPSObserver = CLockAppPubSubObserver::NewL( this, KUidFpsCategory, KFpsAuthenticationKey );

    // Call bubble
    iIncallBubble = CAknIncallBubble::NewL( );

    ActivateL( );

    INFO( "CLockAppStateControl::ConstructL completed" );
    }

// ---------------------------------------------------------------------------
// Create visibility gate window group
// ---------------------------------------------------------------------------
void CLockAppStateControl::CreateVisibilityGateWgL( )
    {
    INFO( "CLockAppStateControl - CreateVisibilityGateL" );

    iWGVisibilityGate = RWindowGroup( iCoeEnv->WsSession( ) );
    User::LeaveIfError( iWGVisibilityGate.Construct( (TUint32) &iWGVisibilityGate ) );

    /* hide this window group from the app switcher */
    iWGVisibilityGate.SetOrdinalPosition( 0, ECoeWinPriorityNeverAtFront );
    iWGVisibilityGate.EnableReceiptOfFocus( EFalse );

    /*CApaWindowGroupName* wn = CApaWindowGroupName::NewLC( ws );
    wn->SetHidden( ETrue );
    wn->SetWindowGroupName( iWGVisibilityGate );
    CleanupStack::PopAndDestroy( wn );*/
    }

// ---------------------------------------------------------------------------
// This method should be called to enable keyguard.
// ---------------------------------------------------------------------------
TInt CLockAppStateControl::EnableKeyguardL( TBool aWithNote )
    {
    INFO_1( "CLockAppStateControl::EnableKeyguardL aWithNote= %d", aWithNote );
    TInt err = CheckIfLegal( ELockAppEnableKeyguard );
    if ( err == KErrNone && !iKeyguard->ActivationAllowedL( ) )
        {
        err = KErrPermissionDenied;
        }
    if ( err == KErrNone )
        {
        PostStatusChangeL( EKeyguardActive );
        __ASSERT_DEBUG( iLockStatus == EKeyguardActive, DoPanic(ELockIllegalState));
        INFO( "CLockAppStateControl - Keyguard Enabled" );
        if ( aWithNote )
            {
            iKeyguard->DisplayKeysLockedNote( );
            }
        }
    return err;
    }

// ---------------------------------------------------------------------------
// This method should be called to disable keyguard.
// ---------------------------------------------------------------------------
TInt CLockAppStateControl::DisableKeyguardL( TBool aWithNote )
    {
    INFO_1( "CLockAppStateControl::DisableKeyguardL aWithNote= %d", aWithNote );
    TInt err = CheckIfLegal( ELockAppDisableKeyguard );
    if ( err == KErrNone && !iKeyguard->DeActivationAllowedL( ) )
        {
        err = KErrPermissionDenied;
        }
    if ( err == KErrNone )
        {
        PostStatusChangeL( ELockNotActive );
        __ASSERT_DEBUG( iLockStatus == ELockNotActive, DoPanic(ELockIllegalState));
        INFO( "CLockAppStateControl - Keyguard Disabled" );
        if ( aWithNote )
            {
            iKeyguard->DisplayKeysActiveNote( );
            }
        }
    return err;
    }

// ---------------------------------------------------------------------------
// This method should be called to enable device lock.
// ---------------------------------------------------------------------------
TInt CLockAppStateControl::EnableDevicelockL( TDevicelockReason aReason )
    {
    INFO_1( "CLockAppStateControl::EnableDevicelockL aReason= %d", aReason );
    RDebug::Printf( "%s %s (%u) iDevicelock=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iDevicelock );
    TInt err = CheckIfLegal( ELockAppEnableDevicelock );
    if ( err == KErrNone && !iDevicelock->ActivationAllowedL( aReason ) )
        {
        err = KErrPermissionDenied;
        }
    if ( err == KErrNone )
        {
        PostStatusChangeL( EDevicelockActive );
        __ASSERT_DEBUG( iLockStatus == EDevicelockActive, DoPanic(ELockIllegalState));
        iDevicelock->SetLockingReason( aReason );
        INFO( "CLockAppStateControl - Devicelock Enabled" );
        }
    return err;
    }

// ---------------------------------------------------------------------------
// This method should be called to disable device lock.
// ---------------------------------------------------------------------------
TInt CLockAppStateControl::DisableDevicelockL( )
    {
    INFO_1( "CLockAppStateControl::DisableDevicelockL 1= %d", 1 );
    TInt err = CheckIfLegal( ELockAppDisableDevicelock );
    if ( err == KErrNone && !iDevicelock->DeActivationAllowedL( ) )
        {
        err = KErrPermissionDenied;
        }
    if ( err == KErrNone )
        {
        PostStatusChangeL( ELockNotActive );
        __ASSERT_DEBUG( iLockStatus == ELockNotActive, DoPanic(ELockIllegalState));
        INFO( "CLockAppStateControl - Devicelock Disabled" );
        }
    return err;
    }

// ---------------------------------------------------------------------------
// Checks if offer keyguard is allowed and if it is, offers to lock keyguard.
// ---------------------------------------------------------------------------
TInt CLockAppStateControl::OfferKeyguard( )
    {
    INFO_1( "CLockAppStateControl::OfferKeyguard 1= %d", 1 );
    TInt err = CheckIfLegal( ELockAppOfferKeyguard );
    if ( err == KErrNone )
        {
        __ASSERT_DEBUG( iLockStatus == ELockNotActive, DoPanic(ELockIllegalState));
        INFO( "CLockAppStateControl - OfferKeyguard" );
        iIdle->OfferKeyLock( );
        }
    return err;
    }

// ---------------------------------------------------------------------------
// Shows keys locked note if keyguard is enabled.
// ---------------------------------------------------------------------------
TInt CLockAppStateControl::ShowKeysLockedNote( )
    {
    INFO_1( "CLockAppStateControl::ShowKeysLockedNote 1= %d", 1 );
    TInt err = CheckIfLegal( ELockAppShowKeysLockedNote );
    if ( err == KErrNone )
        {
        __ASSERT_DEBUG( iLockStatus == EKeyguardActive, DoPanic(ELockIllegalState));
        INFO( "CLockAppStateControl - ShowKeysLockedNote" );
        iKeyguard->DisplayLockedNote( );
        }
    return err;
    }

// ---------------------------------------------------------------------------
// A method for returning the lock status.
// ---------------------------------------------------------------------------
TLockStatus CLockAppStateControl::LockStatus( ) const
    {
    INFO_1( "CLockAppStateControl::LockStatus iLockStatus= %d", iLockStatus );
    return iLockStatus;
    }

// ---------------------------------------------------------------------------
// A method for returning the environment status bitmask.
// ---------------------------------------------------------------------------
TUint CLockAppStateControl::EnvironmentStatus( ) const
    {
    return iEnvState;
    }

// ---------------------------------------------------------------------------
// A method for executing different internal tests.
// ---------------------------------------------------------------------------
TInt CLockAppStateControl::ExecuteInternalTest( )
    {
#ifdef _DEBUG
    return iLockEcsDetector->TestEcsNote( );
#else
    return KErrNone;
#endif
    }

// ---------------------------------------------------------------------------
// Number of owned UI controls (idle, keyguard, devicelock)
// ---------------------------------------------------------------------------
TInt CLockAppStateControl::CountComponentControls( ) const
    {
    return 3;
    }

// ---------------------------------------------------------------------------
// Owned UI controls (idle, keyguard, devicelock)
// ---------------------------------------------------------------------------
CCoeControl* CLockAppStateControl::ComponentControl( TInt aIndex ) const
    {
    RDebug::Printf( "%s %s (%u) aIndex=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aIndex );
    switch ( aIndex )
        {
        case 0:
            return iIdle;
        case 1:
            return iKeyguard;
        case 2:
     		    RDebug::Printf( "%s %s (%u) iDevicelock=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iDevicelock );
            return iDevicelock;
        default:
            return NULL;
        }
    }

// ----------------------------------------------------------
// Handle window-server events
// ----------------------------------------------------------
void CLockAppStateControl::HandleWsEventL( const TWsEvent& aEvent, CCoeControl* /*aDestination*/)
    {
    TInt type = aEvent.Type( );
    	RDebug::Printf( "%s %s (%u) type=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, type );
    switch ( type )
        {
        case KAknFullOrPartialForegroundLost: // partial or full fg lost
        case KAknFullOrPartialForegroundGained: // partial or full fg gained
            {
    				RDebug::Printf( "%s %s (%u) type=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, type );
            RWsSession& ws = iCoeEnv->WsSession( );
            TFileName windowName;
            TInt fwgid = ws.GetFocusWindowGroup( );
            ws.GetWindowGroupNameFromIdentifier( fwgid, windowName );
            INFO_3( "CLockAppStateControl::HandleWsEventL type: %d - FocusWg: %d %S", type, fwgid, &windowName);
            }
            break;
        case EEventKeyUp:
        case EEventKey:
        case EEventKeyDown:
        		{
        		RDebug::Printf( "%s %s (%u) EEventKey*=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, EEventKeyDown );
        		TKeyEvent* aKeyEvent = aEvent.Key();
        		TKeyResponse response = OfferKeyEventL( *aKeyEvent, (TEventCode)type );
        		}
            break;
        default:
            break;
        }
    }

// ---------------------------------------------------------------------------
// From @c CAknAppUiBase. Overriden the parent method, because otherwise
// the main lock state control would not receive the call, because is is not
// window-owning control (=without window->not visible). The call is needed
// because the main state control owns window-owning child controls.
// ---------------------------------------------------------------------------
void CLockAppStateControl::HandleResourceChange( TInt aType )
    {
    // these would not normally get handleresource call since they do not own window
		RDebug::Printf( "%s %s (%u) aType=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aType );
	if(iIdle)
		iIdle->HandleResourceChange( aType );
	if(iKeyguard)
		iKeyguard->HandleResourceChange( aType );
	if(iDevicelock)
		iDevicelock->HandleResourceChange( aType );
    // call parent
    CCoeControl::HandleResourceChange( aType );
    }

// ---------------------------------------------------------------------------
// Update the environment state variable with the event.
// ---------------------------------------------------------------------------
TBool CLockAppStateControl::HandleEnvironmentChange( TUint aEventMask, TBool aEnable )
    {
    // check if there will be a change

    if ( IsBitFieldSet( iEnvState, aEventMask ) != aEnable )
        {
        // set the bit-field
        SetBitField( iEnvState, aEventMask, aEnable );
        // notify current control
        __ASSERT_DEBUG( iCurrentControl, DoPanic(ELockIllegalState));
        iCurrentControl->HandleEnvironmentChange( iEnvState, aEventMask );
        // return true because the environment changed
        return ETrue;
        }
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Handle all Central Repository observer callbacks.
// ---------------------------------------------------------------------------
void CLockAppStateControl::HandleCenRepNotify(TUid aCenRepUid, TUint32 aKeyId, TInt aValue )
    {
    	RDebug::Printf( "%s %s (%u) ???? Nothing done for aKeyId=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aKeyId );

    INFO_3( "CLockAppStateControl::HandleCenRepNotify %d %d = %d", aCenRepUid.iUid, aKeyId, aValue );
    }

// ---------------------------------------------------------------------------
// Handle all Publish & Subscribe observer callbacks.
// ---------------------------------------------------------------------------
void CLockAppStateControl::HandlePubSubNotify(TUid aPubSubUid, TUint aKeyId, TInt aValue )
    {
    INFO_3( "CLockAppStateControl::HandlePubSubNotify %x %x = %d", aPubSubUid.iUid, aKeyId, aValue );
		/* This is not used any more because screensavers are removed now
    INFO_3( "CLockAppStateControl::HandlePubSubNotify KPSUidScreenSaver=%x KPSUidCtsyCallInformation=%x KPSUidAiInformation=%x", KPSUidScreenSaver, KPSUidCtsyCallInformation, KPSUidAiInformation );
    INFO_3( "CLockAppStateControl::HandlePubSubNotify KPSUidHWRM=%x KHWRMGripStatus=%x KPSUidAiInformation=%x", KPSUidHWRM, KHWRMGripStatus, KPSUidAiInformation );
    if ( aPubSubUid == KPSUidScreenSaver )
        {
        switch ( aKeyId )
            {
            case KScreenSaverOn:
                {
                INFO_1("CLockAppStateControl::ScreenSaver - %d", (aValue > 0));
                HandleEnvironmentChange( KLockAppEnvScreenSaverOn, (aValue > 0) );
                }
                break;
            default:
                break;
            }
        }
    else if ( aPubSubUid == KPSUidCtsyCallInformation )
        {
        switch ( aKeyId )
            {
            case KCTsyCallState:
                {
                PrintCallState( aValue );
                HandleEnvironmentChange( KLockAppEnvPhonecallOngoing, (aValue > EPSCTsyCallStateNone) );
                }
                break;
            default:
                break;
            }
        }
    else if ( aPubSubUid == KPSUidAiInformation )	// ActiveIdle
            {
            switch ( aKeyId )
                {
                case KActiveIdleState:
                    {
                    INFO_1("CLockAppStateControl::ActiveIdle - %d", (aValue == EPSAiForeground));
                    HandleEnvironmentChange( KLockAppEnvIdleOnForeground, (aValue == EPSAiForeground) );
                    }
                    break;
                default:
                    break;
                }
            }
    else if ( aPubSubUid == KPSUidHWRM )
        {
        switch ( aKeyId )
            {
            case KHWRMGripStatus:
                {
                INFO_1("CLockAppStateControl::Grip - %d", aValue );
                HandleEnvironmentChange( KLockAppEnvGrip, (aValue == EPSHWRMGripOpen) );
                }
                break;
            default:
                break;
            }
        }
    else if ( aPubSubUid == KUidFpsCategory )
        {
        switch ( aKeyId )
            {
            case KFpsAuthenticationKey:
                {
                INFO_1("CLockAppStateControl::Grip - %d", aValue );
                #define EAuthenticationSuccess 1
                HandleEnvironmentChange( KLockAppEnvFPS, (aValue == EAuthenticationSuccess) );
                }
                break;
            default:
                break;
            }
        }
    */
    }

// ---------------------------------------------------------------------------
// All keyevents should go trough here.
// ---------------------------------------------------------------------------
TKeyResponse CLockAppStateControl::OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType )
    {
    	RDebug::Printf( "%s %s (%u) iDevicelock=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iDevicelock );
    	RDebug::Printf( "%s %s (%u) aKeyEvent.iCode=%x aType=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aKeyEvent.iCode, aType );
    TKeyResponse keyResponse = PreCheckKeyEvents( aKeyEvent, aType );
    if ( keyResponse == EKeyWasNotConsumed )
        {
        if ( aKeyEvent.iRepeats == 0 )
            {
            // handle only without the repeats
            INFO_3("KeyEvent %d, %d, %d", aType, aKeyEvent.iCode, aKeyEvent.iScanCode );
            __ASSERT_DEBUG( iCurrentControl, DoPanic(ELockIllegalState));
            keyResponse = iCurrentControl->OfferKeyEventL( aKeyEvent, aType );
            }
        else
            {
            // set consumed which are repeated
            keyResponse = EKeyWasConsumed;
            }
        }

    return keyResponse;
    }

// ---------------------------------------------------------------------------
// From @c CLockAppObserverList. The main state control is the first
// to handle the lock state changes, locks/unlocks the phone depending
// on the state change.
// ---------------------------------------------------------------------------
void CLockAppStateControl::HandleLockStatusChangedL( TLockStatus aLockStatus )
    {    
    __ASSERT_DEBUG( iCurrentControl, DoPanic(ELockIllegalState));
    INFO_1("CLockAppStateControl::HandleLockStatusChangedL - %d", aLockStatus);
    iCurrentControl->HandleDeActivateEventL( iEnvState );

    switch ( aLockStatus )
        {
        case ELockNotActive:
            {
            iCurrentControl = iIdle;
            MuteSounds( EFalse );
            BringForward( EFalse );
            }
            break;
        case EKeyguardActive:
            {
            iCurrentControl = iKeyguard;
            MuteSounds( ETrue );
            BringForward( ETrue );
            }
            break;
        case EDevicelockActive:
            {
       	    RDebug::Printf( "%s %s (%u) iDevicelock=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iDevicelock );
            iCurrentControl = iDevicelock;
            MuteSounds( ETrue );
            BringForward( ETrue );
            }
            break;
        default:
        		INFO_1("CLockAppStateControl::HandleLockStatusChangedL - ???? aLockStatus=%d", aLockStatus);
            DoPanic( ELockUnknownValue );
            break;
        }

    __ASSERT_DEBUG( iCurrentControl, DoPanic(ELockIllegalState));
    iCurrentControl->HandleActivateEventL( iEnvState );

    // finally we update the value (only place were changing the status is allowed)
    iLockStatus = aLockStatus;
    }

// ---------------------------------------------------------------------------
// Check if requested transition is valid.
// ---------------------------------------------------------------------------
TInt CLockAppStateControl::CheckIfLegal( TLockAppMessageReason aReason )
    {
    INFO_1("CLockAppStateControl::CheckIfLegal aReason=%d", aReason);
    INFO_1("CLockAppStateControl::CheckIfLegal iLockStatus=%d", iLockStatus);
    switch ( aReason )
        {
        case ELockAppEnableKeyguard:
            {
           	RDebug::Printf( "%s %s (%u) ELockAppEnableKeyguard iLockStatus=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iLockStatus );
            switch ( iLockStatus )
                {
                case ELockNotActive:
                    if ( !CKeyLockPolicyApi::KeyguardAllowed() )
                        return KErrPermissionDenied;
                    else
                        return KErrNone;
                case EKeyguardActive:
                    return KErrAlreadyExists;
                case EDevicelockActive:
                    return KErrPermissionDenied;
                }
            }
            break;
        case ELockAppDisableKeyguard:
            {
           	RDebug::Printf( "%s %s (%u) ELockAppDisableKeyguard iLockStatus=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iLockStatus );
            switch ( iLockStatus )
                {
                case ELockNotActive:
                    return KErrAlreadyExists;
                case EKeyguardActive:
                    return KErrNone;
                case EDevicelockActive:
                    return KErrPermissionDenied;
                }
            }
            break;
        case ELockAppEnableDevicelock:
            {
           	RDebug::Printf( "%s %s (%u) ELockAppEnableDevicelock iLockStatus=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iLockStatus );
            switch ( iLockStatus )
                {
                case ELockNotActive:
                    return KErrNone;
                case EKeyguardActive:
                    return KErrNone;
                case EDevicelockActive:
                    return KErrAlreadyExists;
                }
            }
            break;
        case ELockAppDisableDevicelock:
            {
           	RDebug::Printf( "%s %s (%u) ELockAppDisableDevicelock iLockStatus=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iLockStatus );
            switch ( iLockStatus )
                {
                case ELockNotActive:
                    return KErrAlreadyExists;
                case EKeyguardActive:
                    return KErrPermissionDenied;
                case EDevicelockActive:
                    return KErrNone;
                }
            }
            break;
        case ELockAppOfferKeyguard:
            {
           	RDebug::Printf( "%s %s (%u) ELockAppOfferKeyguard iLockStatus=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iLockStatus );
            switch ( iLockStatus )
                {
                case ELockNotActive:
                    return KErrNone;
                case EKeyguardActive:
                    return KErrPermissionDenied;
                case EDevicelockActive:
                    return KErrPermissionDenied;
                }
            }
            break;
        case ELockAppShowKeysLockedNote:
            {
           	RDebug::Printf( "%s %s (%u) ELockAppShowKeysLockedNote iLockStatus=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iLockStatus );
            switch ( iLockStatus )
                {
                case ELockNotActive:
                    return KErrPermissionDenied;
                case EKeyguardActive:
                    return KErrNone;
                case EDevicelockActive:
                    return KErrPermissionDenied;
                }
            }
            break;
        }
    return KErrPermissionDenied;
    }

// ---------------------------------------------------------------------------
// Prescreen key events for special cases before giving them to child controls
// ---------------------------------------------------------------------------
TKeyResponse CLockAppStateControl::PreCheckKeyEvents(const TKeyEvent& aKeyEvent, TEventCode aType )
    {
    	RDebug::Printf( "%s %s (%u) aKeyEvent.iCode=%x aType=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aKeyEvent.iCode, aType );
    TKeyResponse keyResponse = EKeyWasNotConsumed;
    // power key always turns the lights on
    CheckForPowerKeyLights( aKeyEvent, aType );
    if ( iLockEcsDetector->EcsNoteOnScreen( ) )
        {
        // the key events are fully handled by the emergency
        // detector when emergency note is shown
        keyResponse = EKeyWasConsumed;
        }
    else
        {
        if ( CheckForPhoneKeys( aKeyEvent, aType ) )
            {
            keyResponse = EKeyWasConsumed;
            }
        else if ( aKeyEvent.iCode == EKeyLeftShift || aKeyEvent.iCode == EKeyRightShift )
            {
            // eat these modifier events as they just cause problems elsewhere
            keyResponse = EKeyWasConsumed;
            }
        }
    return keyResponse;
    }

// ---------------------------------------------------------------------------
// Checks if power key has been pressed and inform the sysap accordingly.
// ---------------------------------------------------------------------------
void CLockAppStateControl::CheckForPowerKeyLights( const TKeyEvent& aKeyEvent, TEventCode aType )
    {
    // only handle power key when keys are locked
    	RDebug::Printf( "%s %s (%u) aKeyEvent.iCode=%x aType=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aKeyEvent.iCode, aType );
    	RDebug::Printf( "%s %s (%u) iLockStatus=%x ELockNotActive=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iLockStatus, ELockNotActive );
    if ( iLockStatus != ELockNotActive )
        {
    		RDebug::Printf( "%s %s (%u) iFeatureNoPowerkey=%x EStdKeyDevice2=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iFeatureNoPowerkey, EStdKeyDevice2 );
        if ( (aType == EEventKey && aKeyEvent.iRepeats == 0) || (aType == EEventKeyDown && aKeyEvent.iScanCode == EStdKeyDevice2) ) // Power key
            {
            // If power key pressed, tell SysAp about if
            if ( (aKeyEvent.iScanCode == EStdKeyDevice2 && aType == EEventKeyDown) || (iFeatureNoPowerkey && aKeyEvent.iCode == EKeyNo) )
                {
                // normal power key event
                RDebug::Printf( "%s %s (%u) EEikKeyLockPowerKeyPressed=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iFeatureNoPowerkey );
                SendMessageToSysAp( EEikKeyLockPowerKeyPressed );
                if ( iFeatureNoPowerkey )
                    {
                		RDebug::Printf( "%s %s (%u) EEikKeyLockLightsOnRequest=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, EEikKeyLockLightsOnRequest );
                    // end key as power key needs additional info (ugly solution)
                    SendMessageToSysAp( EEikKeyLockLightsOnRequest );
                    }
                }
            }
        }
    }

// ---------------------------------------------------------------------------
// Checks if green or red key has been pressed and forward it to phone
// in case there is a phone call ongoing.
// ---------------------------------------------------------------------------
TBool CLockAppStateControl::CheckForPhoneKeys( const TKeyEvent& aKeyEvent, TEventCode aType )
    {
    	RDebug::Printf( "%s %s (%u) aKeyEvent.iCode=%x aType=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aKeyEvent.iCode, aType );
#ifndef RD_GLOBALWG_PRIORITY_CHANGE
    // only handle phone keys when locked and phonecall ongoing
    	RDebug::Printf( "%s %s (%u) iLockStatus=%x ELockNotActive=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iLockStatus, ELockNotActive );
    if ( iLockStatus != ELockNotActive )
        {
        if ( aKeyEvent.iScanCode == EStdKeyYes || aKeyEvent.iScanCode == EStdKeyNo )
            {
            if ( IsBitFieldSet( iEnvState, KLockAppEnvPhonecallOngoing ) )	// TODO what about video-call
                {
                // send Green and Red keys to phoneapp's windowgroup
                INFO( "CLockAppStateControl::CheckForPhoneKeys - SendKeyToPhoneApp");
                SendKeyToPhoneApp( aKeyEvent, aType );
                return ETrue;
                }
            }
        }
#endif // !RD_GLOBALWG_PRIORITY_CHANGE
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Brings foreground/background the root window.
// ---------------------------------------------------------------------------
void CLockAppStateControl::BringForward(TBool aForeground )
    {
    	RDebug::Printf( "%s %s (%u) aForeground=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aForeground );
    if ( aForeground )
        {
        INFO_4( "CLockAppStateControl::BringForward(1) STA - EvWG(%d,%d) ViWG(%d,%d)",
                iWGEventGate.OrdinalPosition(), iWGEventGate.OrdinalPriority(),
                iWGVisibilityGate.OrdinalPosition(), iWGVisibilityGate.OrdinalPriority());
        // not sure if these cause flush in WindowServer
        iWGVisibilityGate.SetOrdinalPosition( 0, ECoeWinPriorityAlwaysAtFront + KGlobalWindowPriority_LockVisibilityGate );
        iWGEventGate.SetOrdinalPosition( 0, ECoeWinPriorityAlwaysAtFront + KGlobalWindowPriority_LockVisibilityGate );
        iEikonEnv->BringForwards( ETrue, ECoeWinPriorityAlwaysAtFront + KGlobalWindowPriority_KeyLock );
        /*if ( iIncallBubble )
            {
            iIncallBubble->SetIncallBubbleAllowedInIdleL( ETrue );
            }*/
        }
    else
        {
        INFO_4( "CLockAppStateControl::BringForward(0) STA - EvWG(%d,%d) ViWG(%d,%d)",
                iWGEventGate.OrdinalPosition(), iWGEventGate.OrdinalPriority(),
                iWGVisibilityGate.OrdinalPosition(), iWGVisibilityGate.OrdinalPriority());

        // not sure if these cause flush in WindowServer
        iWGVisibilityGate.SetOrdinalPosition( 0, ECoeWinPriorityNeverAtFront );
		iWGEventGate.SetOrdinalPosition( 0, ECoeWinPriorityNeverAtFront );
        iEikonEnv->BringForwards( EFalse );
        /*if ( iIncallBubble )
            {
            iIncallBubble->SetIncallBubbleAllowedInIdleL( EFalse );
            }*/
        }

    INFO_4( "CLockAppStateControl::BringForward END - EvWG(%d,%d) ViWG(%d,%d)",
            iWGEventGate.OrdinalPosition(), iWGEventGate.OrdinalPriority(),
            iWGVisibilityGate.OrdinalPosition(), iWGVisibilityGate.OrdinalPriority());
    }

// ---------------------------------------------------------------------------
// Mute key sounds when phone is locked.
// ---------------------------------------------------------------------------
void CLockAppStateControl::MuteSounds(TBool aMuteSounds )
    {
    	RDebug::Printf( "%s %s (%u) aMuteSounds=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aMuteSounds );
    if ( iSoundsMuted != aMuteSounds )
        {
        if ( aMuteSounds )
            {
            TRAPD(err, iAvkonAppUiBase->KeySounds()->PushContextL(R_AVKON_SILENT_SKEY_LIST))
            if ( !err )
                {
                iAvkonAppUiBase->KeySounds()->BringToForeground();
                iAvkonAppUiBase->KeySounds()->LockContext();
                iSoundsMuted = ETrue;
                }
            }
        else
            {
            iAvkonAppUiBase->KeySounds()->ReleaseContext();
            iAvkonAppUiBase->KeySounds()->PopContext();
            iSoundsMuted = EFalse;
            }
        }
    }

// ---------------------------------------------------------------------------
// Print out call state.
// ---------------------------------------------------------------------------
void CLockAppStateControl::PrintCallState( TInt aValue )
    {
    RDebug::Printf( "%s %s (%u) aValue=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aValue );
    switch ( aValue )
        {
        case EPSCTsyCallStateUninitialized:
            INFO("CLockAppStateControl::PrintCallState - Uninitialized")
            break;
        case EPSCTsyCallStateNone:
            INFO("CLockAppStateControl::PrintCallState - None")
            break;
        case EPSCTsyCallStateAlerting:
            INFO("CLockAppStateControl::PrintCallState - Alerting")
            break;
        case EPSCTsyCallStateRinging:
            INFO("CLockAppStateControl::PrintCallState - Ringing")
            break;
        case EPSCTsyCallStateDialling:
            INFO("CLockAppStateControl::PrintCallState - Dialling")
            break;
        case EPSCTsyCallStateAnswering:
            INFO("CLockAppStateControl::PrintCallState - Answering")
            break;
        case EPSCTsyCallStateDisconnecting:
            INFO("CLockAppStateControl::PrintCallState - Disconnecting")
            break;
        case EPSCTsyCallStateConnected:
            INFO("CLockAppStateControl::PrintCallState - Connected")
            break;
        case EPSCTsyCallStateHold:
            INFO("CLockAppStateControl::PrintCallState - Hold")
            break;
        default:
            break;
        }
    }

// END OF FILE
