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
* Description:  Interface for controls that want to modify lock status
 *
*/


#ifndef __LOCKAPPSTATECONTROLINTERFACE_H__
#define __LOCKAPPSTATECONTROLINTERFACE_H__

// INCLUDES
#include "lockapp.hrh"

/**
 *  MLockAppStateControl class offers safe interface methods for lockapp controls.
 *  All internal child controls should use this interface for modifying
 *  the phone lock state.
 *
 *  @lib    lockapp
 *  @since  5.0
 *  @author Joona Petrell
 *  @author Tamas Koteles
 */
class MLockAppStateControl
	{
	public:

		/** 
		 * Activate keyguard lock.
		 *
		 * @param aWithNote if "keys locked" note is shown
		 * @return KErrAlreadyExists if already enabled
		 *         KErrPermissionDenied if devicelock is activated
		 *         KErrNone if succeeded
		 */
		virtual TInt EnableKeyguardL( TBool aWithNote ) = 0;

		/**
		 * Dectivate keyguard lock.
		 *
		 * @param aWithNote if "keys active" note is shown.
		 * @return KErrAlreadyExists if already disabled
		 *         KErrPermissionDenied if devicelock is activated
		 *         KErrNone if succeeded
		 */
		virtual TInt DisableKeyguardL( TBool aWithNote ) = 0;

		/**
		 * Activate devicelock.
		 * 
		 * @param aReason the device locking reason.
		 * @return KErrAlreadyExists if already enabled.
		 *         KErrNone if succeeded
		 */
		virtual TInt EnableDevicelockL( TDevicelockReason aReason = EDevicelockManual ) = 0;

		/**
		 * Dectivate devicelock.
		 *
		 * @return KErrAlreadyExists if already disabled
		 *         KErrPermissionDenied if keyguard is activated
		 *         KErrNone if succeeded
		 */
		virtual TInt DisableDevicelockL( ) = 0;

		/**
		 * Offer to enable keyguard by showing "offerkeylock" note.
		 * 
		 * @return KErrPermissionDenied if keyguard/devicelock already activated
		 *         KErrNone if succeeded
		 */
		virtual TInt OfferKeyguard( ) = 0;

		/**
		 * Offer to enable keyguard by showing "offerkeylock" note.
		 * 
		 * @return KErrNone if succeeded
		 *         KErrPermissionDenied if keyguard is not enabled
		 */
		virtual TInt ShowKeysLockedNote( ) = 0;

		/**
		 * External access to lock state
		 *
		 * @return Current lock state
		 */
		virtual TLockStatus LockStatus( ) const = 0;

		/**
		 * External access to lock environment
		 *
		 * @return Current environment state
		 */
		virtual TUint EnvironmentStatus( ) const = 0;

		/**
		 * Used for internal testing only, disabled normally
		 * 
		 * @return error code
		 */
		virtual TInt ExecuteInternalTest( ) = 0;

	};

#endif // __LOCKAPPSTATECONTROLINTERFACE_H__
// End of File
