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
* Description:  Devicelock UI
 *
*/


#ifndef LOCKAPPDEVICELOCKCONTROL_H
#define LOCKAPPDEVICELOCKCONTROL_H

// INCLUDES
#include "lockappbasecontrol.h"
#include "lockappobserverinterface.h"
#include <etelmm.h>
#include <rmmcustomapi.h>
#include <LockDomainCRKeys.h>

// FORWARD DECLARATIONS
class CLockAppDevicelockContainer;
class CUserActivityManager;
class CLockAppPubSubObserver;
class CLockAppCenRepObserver;

/**
 *  CLockAppDevicelockControl represents the devicelock state in the state machine.
 *  Window-owning compound control that provides visible user interface,
 *  shows the lock bitmap by its child container, handles all events 
 *  and asks for security code if "unlock" is pressed.
 *
 *  @lib    lockapp
 *  @since  5.0
 *  @author Joona Petrell
 *  @author Tamas Koteles
 *  @see    CLockAppBaseControl
 *  @see    CLockAppDevicelockContainer
 *  @see    MLockAppStateControl
 *  @see    CUserActivityManager
 *  @see    CLockAppPubSubObserver
 *  @see    CLockAppCenRepObserver
 */
class CLockAppDevicelockControl : public CLockAppBaseControl, public MEikCommandObserver,
		public MLockAppObserverInterface
	{
	public:

		/**
		 * Two-phased constructor.
		 */
		static CLockAppDevicelockControl* NewL( MLockAppStateControl& aStateControl,
				RWindowGroup& aWg );

		/**
		 * Destructor.
		 */
		~CLockAppDevicelockControl( );
		
		/**
		 * Finalize the construction by connecting to Phone side.
		 */
		void ConnectToPhoneL( RWindowGroup& aWg );

		/**
		 * Is it allowed to activate control.
		 */
		TBool ActivationAllowedL( TDevicelockReason aReason );

		/**
		 * is it allowed to deactivate control.
		 */
		TBool DeActivationAllowedL( );

		virtual void HandleActivateEventL( TUint aEnvMask );

		virtual void HandleDeActivateEventL( TUint aEnvMask );

		virtual void HandleEnvironmentChange( TUint aEnvMask, TUint aEventMask );

		/**
		 * Set the reason for devicelock.
		 */
		void SetLockingReason( TDevicelockReason aReason );

	private:

		/**
		 * Constructor for performing 1st stage construction
		 */
		CLockAppDevicelockControl( MLockAppStateControl& aStateControl );

		/**
		 * 2nd stage construction
		 */
		void ConstructL( RWindowGroup& aWg );

		void DefinePubSubKeysL( );

		void HandleUnlockCommandL( );

	public:

		/**
		 * Handle Central Repository observer callback.
		 */
		void HandleCenRepNotify( TUid aCenRepUid, TUint32 aKeyId, TInt aValue );

		/**
		 * Handle Publish & Subscribe observer callback.
		 */
		void HandlePubSubNotify( TUid aPubSubUid, TUint aKeyId, TInt aValue );

	public:

		void HandleResourceChange( TInt aType );

		TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType );

	private:

		TInt CountComponentControls( ) const;

		CCoeControl* ComponentControl( TInt aIndex ) const;

	private:

		/*
		 * Checks whether we are booting from a Hidden Reset
		 */
		TBool IsHiddenReset( );

		/*
		 * Checks whether the pin is blocked.
		 */
		TBool IsPinBlocked( );

		/*
		 * Checks whether TARM admin flag is set (optionally unsets it).
		 */
		TBool TarmAdminFlag( TBool unSetFlag );

		TBool ETelActivationAllowed( );

		// Get autolock timeout (in seconds)
		TInt GetAutoLockTimeout( );

		// Starts monitoring user activity
		void StartActivityMonitoringL( );

		// Gets new autolock period and starts monitoring user activity
		void ResetInactivityTimeout( );

		// Stop monitoring user activity.
		void StopActivityMonitoring( );

		// Handles Active event. Called by ActivityManager
		static TInt HandleActiveEventL( TAny* aPtr );

		// Handles Inactive event. Called by ActivityManager
		static TInt HandleInactiveEventL( TAny* aPtr );

		// Set custom status pane visible/invisible
		void ShowStatusPane( const TBool aVisible );

	private:

		/**
		 * From @c MEikCommandObserver. Dialogs and CBA send
		 * UI commands to parent using method ProcessCommandL.
		 *
		 * @param aCommandId Command to be handled
		 */
		void ProcessCommandL( TInt aCommandId );

	private:

		/*****************************************************
		 *    S60 Customer / ETel
		 *    S60 ETel API
		 *****************************************************/

		RTelServer iTelServer;
		int iTelServerInitialized;
		RMobilePhone iPhone;
		int iPhoneInitialized;
		RMmCustomAPI iCustomPhone;
		int iCustomPhoneInitialized;

		/**
		 * Devicelock auto-locking timeout observer 
		 * (value in minutes)
		 */
		CLockAppCenRepObserver* iCRAutoLockTime;

		/**
		 * Devicelock status publisher.
		 * (Permamanent setting: On/Off)
		 */
		CLockAppCenRepObserver* iCRAutoLockStatus;

		/**
		 * Autolock state PubSub publisher.
		 * (Runtime setting)
		 */
		CLockAppPubSubObserver* iPSAutolockState; 

		/**
		 * User activity manager/observer
		 */
		CUserActivityManager* iActivityManager;

		/**
		 * Background image container control
		 */
		CLockAppDevicelockContainer* iContainer;

		/**
		 * Feature manager value for CDMA protocol
		 */
		TBool iFeatureProtocolCdma;

		/**
		 * Is security code query shown
		 */
		TBool iShowingSecCodeQuery;

	};

#endif // LOCKAPPDEVICELOCKCONTROL_H
