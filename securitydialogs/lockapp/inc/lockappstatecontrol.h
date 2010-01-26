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
* Description:  Controls lock states (keyguard locked/unlocked, devicelock)
 *
*/


#ifndef __LOCKAPPSTATECONTROL_H__
#define __LOCKAPPSTATECONTROL_H__

// INCLUDES
#include "lockappobserverlist.h"
#include "lockappstatecontrolinterface.h"
#include "lockappobserverinterface.h"
#include <lockappclientserver.h>

// FORWARD DECLARATIONS
class CLockAppBaseControl;
class CLockAppIdleControl;
class CLockAppKeyguardControl;
class CLockAppDevicelockControl;
class CLockAppEcsDetector;
class CLockAppPubSubObserver;
class CAknIncallBubble;

/**
 *  CLockAppStateControl class is the main locking state control class. 
 *  Class is derived from CLockAppObserverList observer list class. 
 *  Does not own visible user interface, but directs window server events like 
 *  key events to window-owning child controls (keyguard ui, emergency support). 
 *  All lock state changes should be handled through this class.
 *
 *  @lib    lockapp
 *  @since  5.0
 *  @author Joona Petrell
 *  @author Tamas Koteles
 */
class CLockAppStateControl : public CLockAppObserverList, public MLockAppStateControl,
		public MLockAppObserverInterface
	{
	public:

		/**
		 * Two-phased constructor.
		 */
		static CLockAppStateControl* NewL( );

		/**
		 * Destructor.
		 */
		~CLockAppStateControl( );

	private:

		/**
		 * C++ default constructor.
		 */
		CLockAppStateControl( );

		/**
		 * Second phase constructor allowed to fail (leave).
		 */
		void ConstructL( );

	public:

		/**
		 * Activate keyguard lock.
		 *
		 * @param aWithNote if "keys locked" note is shown
		 * @return KErrAlreadyExists if already enabled
		 *         KErrPermissionDenied if devicelock is activated
		 *         KErrNone if succeeded
		 */
		TInt EnableKeyguardL( TBool aWithNote );

		/**
		 * Dectivate keyguard lock.
		 *
		 * @param aWithNote if "keys active" note is shown.
		 * @return KErrAlreadyExists if already disabled
		 *         KErrPermissionDenied if devicelock is activated
		 *         KErrNone if succeeded
		 */
		TInt DisableKeyguardL( TBool aWithNote );

		/**
		 * Activate devicelock.
		 *
		 * @param aReason the device locking reason.
		 * @return KErrAlreadyExists if already enabled.
		 *         KErrNone if succeeded
		 */
		TInt EnableDevicelockL( TDevicelockReason aReason = EDevicelockManual );

		/**
		 * Dectivate devicelock.
		 *
		 * @return KErrAlreadyExists if already disabled
		 *         KErrPermissionDenied if keyguard is activated
		 *         KErrNone if succeeded
		 */
		TInt DisableDevicelockL( );

		/**
		 * Offer to enable keyguard by showing "offerkeylock" note.
		 * @return KErrPermissionDenied if keyguard/devicelock already activated
		 *         KErrNone if succeeded
		 */
		TInt OfferKeyguard( );

		/**
		 * Offer to enable keyguard by showing "offerkeylock" note.
		 * 
		 * @return KErrNone if succeeded
		 *         KErrPermissionDenied if keyguard not enabled
		 */
		TInt ShowKeysLockedNote( );

		/**
		 * External access to lock state
		 *
		 * @return Current lock state
		 */
		TLockStatus LockStatus( ) const;

		/**
		 * External access to lock environment
		 *
		 * @return Current environment state mask
		 */
		TUint EnvironmentStatus( ) const;

		/**
		 * Only used for internal testing.
		 * @return error code
		 */
		TInt ExecuteInternalTest( );

	public:
		// From MLockAppObserverInterface

		/**
		 * Handle Central Repository observer callback.
		 */
		void HandleCenRepNotify( TUid aCenRepUid, TUint32 aKeyId, TInt aValue );

		/**
		 * Handle Publish & Subscribe observer callback.
		 */
		void HandlePubSubNotify( TUid aPubSubUid, TUint aKeyId, TInt aValue );

	public:
		// from CCoeControl

		TInt CountComponentControls( ) const;

		CCoeControl* ComponentControl( TInt aIndex ) const;

		TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType );

		void HandleWsEventL( const TWsEvent& aEvent, CCoeControl* aDestination );

		void HandleResourceChange( TInt aType );

	private:

		/**
		 * Creates the second "visibility gate" window group.
		 */
		void CreateVisibilityGateWgL( );

		/**
		 * Logs the telephony P&S call state.
		 */
		void PrintCallState( TInt aValue );

		/**
		 * Update the environment variable with the event.
		 * @return ETrue if the environment value has changed
		 *         EFalse otherwise
		 */
		TBool HandleEnvironmentChange( TUint aEventMask, TBool aEnable );

		/**
		 * Check if given transition is valid.
		 * @return KErrNone if transition is legal
		 *         KErrAlreadyExists if LockApp already is in requested state
		 *         KErrPermissionDenied if illegal transition
		 */
		TInt CheckIfLegal( TLockAppMessageReason aReason );

		/**
		 * Handle lock state change. Should only called from
		 * method PostStatusChangeL.
		 *
		 * @param aLockStatus The new lock state
		 */
		void HandleLockStatusChangedL( TLockStatus aLockStatus );

		void BringForward( TBool aForeground );

		/**
		 * Mute/Unmute key sounds when phone is locked/unlocked.
		 * 
		 * @param aMuteSounds mute switch
		 */
		void MuteSounds( TBool aMuteSounds );

		/**
		 * Prescreen key events for special cases before giving them to child controls.
		 */
		TKeyResponse PreCheckKeyEvents( const TKeyEvent& aKeyEvent, TEventCode aType );

		/**
		 * Power key needs to always activate lights.
		 */
		void CheckForPowerKeyLights( const TKeyEvent& aKeyEvent, TEventCode aType );

		/**
		 * Green and Red keys should be passed to Phone during phone call.
		 * @return ETrue if the keys have been forwarded
		 *         EFalse otherwise
		 */
		TBool CheckForPhoneKeys( const TKeyEvent& aKeyEvent, TEventCode aType );

	private:

		/**
		 * Internal lock state.
		 */
		TLockStatus iLockStatus;

		/**
		 * Current control;
		 */
		CLockAppBaseControl* iCurrentControl;

		/**
		 * Idle UI.
		 * Owned by superclass.
		 */
		CLockAppIdleControl* iIdle; // owned by superclass

		/**
		 * Keyguard UI.
		 * Owned by superclass.
		 */
		CLockAppKeyguardControl* iKeyguard; // owned by superclass

		/**
		 * Autolock UI
		 * Owned by superclass. 
		 */
		CLockAppDevicelockControl* iDevicelock;

		/**
		 * Emergency call detector with emergency note.
		 * Owned by superclass.
		 */
		CLockAppEcsDetector* iLockEcsDetector;

		/**
		 * PubSub observers
		 */
		CLockAppPubSubObserver* iPSScreenSaverObserver;
		CLockAppPubSubObserver* iPSTelephonyObserver;
		CLockAppPubSubObserver* iPSGripObserver;
		CLockAppPubSubObserver* iPSFPSObserver;

		/**
		 * Application's main window group - Event gate
		 */
		RWindowGroup& iWGEventGate;

		/**
		 * Visibility gate (owned)
		 */
		RWindowGroup iWGVisibilityGate;

		/**
		 * Incall Bubble. (owned)
		 */
		CAknIncallBubble* iIncallBubble;

	private:

		/**
		 *  feature manager keys
		 */
		TBool iFeatureNoPowerkey;

		/**
		 * Offset value used to free reserved localization resources
		 */
		TInt iResourceFileOffset;

		/**
		 * if sounds are muted
		 */
		TBool iSoundsMuted;

		/**
		 * Environment state descriptor bit-mask
		 */
		TUint iEnvState;

	};

#endif // __LOCKAPPSTATECONTROL_H__
// End of File
