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
* Description:  Base control for logical UI components
 *
*/


#ifndef __LOCKAPPBASECONTROL_H__
#define __LOCKAPPBASECONTROL_H__

// INCLUDES
#include <coecntrl.h>
#include "lockappstateobserver.h"
#include "lockapplockednote.h"
#include "lockappkeypattern.h"
#include <keylockpolicyapi.h>

// FORWARD DECLARATIONS
class MLockAppStateControl;
class CEikButtonGroupContainer;

/**
 *  CLockAppBaseControl class is the base control of lockapp state controls,
 *  provides common interface and some protected utility methods.
 *
 *  @lib    lockapp
 *  @since  5.0
 *  @author Joona Petrell
 *  @author Tamas Koteles
 */
class CLockAppBaseControl : public CCoeControl, public MLockAppStateObserver
	{
	public:

		/**
		 * Destructor.
		 */
		~CLockAppBaseControl( );

		/**
		 * From @c MLockAppStateObserver. Method handles the lock state changes.
		 *
		 * @param aStatus The new lock state
		 */
		virtual void HandleLockStatusChangedL( TLockStatus aStatus );

		/**
		 * Handles control activation.
		 * 
		 * @param aEnvMask environment bitmask
		 */
		virtual void HandleActivateEventL( TUint aEnvMask );

		/**
		 * Handles control deactivation.
		 * 
		 * @param aEnvMask environment bitmask
		 */
		virtual void HandleDeActivateEventL( TUint aEnvMask );

		/**
		 * Handle environment bitmask change.
		 * 
		 * @param aEnvMask environment bitmask
		 * @param aEventMask event bitmask
		 */
		virtual void HandleEnvironmentChange( TUint aEnvMask, TUint aEventMask );

	protected:

		/**
		 * Default Constructor.
		 * 
		 * @param aStateControl state control interface
		 */
		CLockAppBaseControl( MLockAppStateControl& aStateControl );

		/**
		 * 2nd phase constructor.
		 */
		void ConstructL( );

		/**
		 * Set up the control's keypattern matcher with the specified keylockpolicy.
		 * 
		 * @param aType keylock policy type (lock,unlock,devicelock) 
		 */
		TBool SetupKeyPatternsWithPolicyL( TLockPolicyType aType );

		/**
		 * Show a note. (cancels previous one if shown)
		 * 
		 * @param aNote    note to be shown
		 * @param aTimeout timeout for the note
		 * @param aTone    tone type
		 */
		void ShowNote( CLockAppLockedNote* aNote, const TInt aTimeout,
				const CAknNoteDialog::TTone aTone );

		/**
		 * Dismisses a note.
		 */
		void CancelNote( );

		/**
		 * Capture/Release primary keys.
		 */
		void CapturePrimaryKeys( const TBool aCapture );

		/**
		 * Show/Hide softkey cba.
		 */
		void ShowCba( const TBool aShow );
		
		/**
		 * Capture/Release pointer events.
		 */
		void SetPointerEventCapture( const TBool aEnable );

		/**
		 * Show/Hide keyguard indicator state.
		 */
		void SetKeyguardIndicatorStateL( const TBool aEnable );

	protected:

		// interface to parent state control
		MLockAppStateControl& iStateControl;

		// application's window group
		RWindowGroup& iWindowGroup;

		// control's currently shown note (not owned)
		CLockAppLockedNote* iCurrentNote;

		// control's Cba (owned)
		CEikButtonGroupContainer* iCba;

		// key pattern matching (owned)
		CLockAppKeyPattern* iKeyPattern;

		// if control is active
		TBool iActive;

	};

#endif // __LOCKAPPBASECONTROL_H__
