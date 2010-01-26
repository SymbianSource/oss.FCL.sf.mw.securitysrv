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
* Description:  Keylock policy implementation
 *
*/


#ifndef ___CKEYLOCKPOLICYAPIIMPL__
#define ___CKEYLOCKPOLICYAPIIMPL__

#include <w32std.h>
#include <e32base.h>
#include <cenrepnotifyhandler.h>
#include "keylockpolicyapi.h"

/**
 * Keylock policy component supports three policy types:
 *
 * EPolicyActivateKeyguard keys used for keyguard locking
 * EPolicyDeactivateKeyguard keys used for keyguard unlocking
 * EPolicyDevicelockQuery keys used for showing devicelock query
 */

/**
 *  CKeyLockPolicyApi. Trough CKeyLockPolicyApi users can
 *
 *  @lib keylockpolicy.lib
 *  @since 3.2
 */
class CKeyLockPolicyApiImpl : public CBase, public MCenRepNotifyHandlerCallback
	{
public:

	/**
	 * Use this to create keylock policy
	 * @param TLockPolicyType aType the policy used
	 */
	static CKeyLockPolicyApiImpl* NewL( TLockPolicyType aType );

	/**
	 * Destructor.
	 */
	~CKeyLockPolicyApiImpl( );

	/**
	 * If no policy has been defined for the type it cannot be used.
	 * @return ETrue if has succesfully loaded policy
	 */
	TBool HasConfiguration( );

	/**
	 * Activates support for keyguard feature so  the phone keys cannot
	 * be locked with keyguard. Does not affect the devicelock.
	 * @return standard Symbian error code
	 */
	TInt EnableKeyguardFeature( );

	/**
	 * Disables support for keyguard feature so the phone keys cannot
	 * be locked with keyguard. Does not affect the devicelock.
	 * @return standard Symbian error code
	 */
	TInt DisableKeyguardFeature( );

	/**
	 * Whether keyguard feature is currently enabled or disabled.
	 * @return ETrue if keyguard can be locked.
	 */
	TBool static KeyguardAllowed( );

	/**
	 * Adds a key combination for the policy type defined in the construction.
	 *
	 * @param TUint scan code for primary key
	 * @param TUint scan code for secondary key
	 * @return standard Symbian error code
	 */
	TInt AddKeyCombination(TUint32 aPrimaryKey, TUint32 aSecondaryKey );

	/**
	 * Removes a key combination for the policy type defined in the construction.
	 *
	 * @param TUint scan code for primary key
	 * @param TUint scan code for secondary key
	 * @return standard Symbian error code
	 */
	TInt RemoveKeyCombination(TUint32 aPrimaryKey, TUint32 aSecondaryKey );

	/**
	 * Retrieve a key combination on the given index.
	 *
	 * @param TInt aIndex index on the list that stores key combinations
	 * @param TUint key code for primary key
	 * @param TUint key code for secondary key
	 * @return standard Symbian error code
	 */
	TInt GetKeyCombination(TInt aIndex, TUint32 &aPrimaryKey, TUint32 &aSecondaryKey );

	/**
	 * Clears all the defined key combinations from the policy.
	 *
	 * @return standard Symbian error code
	 */
	TInt ClearKeycombinations( );

	/**
	 * Handles Key events, returns ETrue the key events match any of the
	 * defined key combinations. Only uses scancodes with type EEventKeyDown.
	 *
	 * @param TKeyEvent aKeyEvent the actual key event
	 * @param TEventCode aType the type of the received key event
	 * @return ETrue if a match was found.
	 */
	TBool HandleKeyEventL(const TKeyEvent& aKeyEvent, const TEventCode aType );

	/**
	 * Returns true if primary button is pressed and note should be shown.
	 * @return boolean true if primary key is pressed.
	 */
	TBool PrimaryKeyPressed( );

	/**
	 * Returns Primary key scan code that has been pressed last.
	 * @return scan code value of the primary key if found, NULL if not
	 */
	TUint GetLastPrimaryKey( );

private:

	/**
	 * C++ default constructor (private so cannot be derived).
	 * @param TLockPolicyType aType the policy (TLockPolicyType) used
	 */
	CKeyLockPolicyApiImpl( TLockPolicyType aType );

	/**
	 * Second constructor initializes the policy
	 */
	void ConstructL( );

	TInt SetKeyCombination(TUint32 aPrimaryKey, TUint32 aSecondaryKey, TInt aIndex );

	TInt FindKeyCombinationIndex(TUint32 aPrimaryKey, TUint32 aSecondaryKey, TInt &aIndex );

	TInt GetNewKeyCombinationIndex(TInt &aIndex );

	/**
	 * From @c MCenRepNotifyHandlerCallback. Changes in the observed
	 * central repository keys are communicated trough this method.
	 * @param TUint32 aId central repository key
	 * @param TInt aNewValue the new value
	 */
	void HandleNotifyInt( TUint32 aId, TInt aNewValue );

	/**
	 * A call back to the keylock timer
	 * param TAny aSelf a pointer to the parent class
	 */
	static TInt HandleKeyLockTimerTimeout(TAny* aSelf );

private:

	// handle to central repository
	CRepository* iRep;

	// defined configuration
	RArray<TUint> iPrimaryKeys;
	RArray<TUint> iSecondaryKeys;
	RArray<TUint> iWaitingKeys;

	// defined timeout
	TUint iTimeout;

	// first key pressed
	TBool iPrimaryKeyDown;

	// index of last primary key in the policy
	TUint iLastPrimaryKeyIndex;

	// key configuration mask
	TUint iMask;

	// current policy mode
	TUint iMode;

	// initialized policy type
	TUint iType;

	// if the keyguard is allowed or not
	TUint iAllowed;

	// if the keylock policy exists
	TBool iInitialised;

	// timer used between primary and secondary keys
	CPeriodic* iKeyLockTimer;

	// notifies the changes in keyguard block
	CCenRepNotifyHandler* iAllowNotif;

	// notifies the changes in mode
	CCenRepNotifyHandler* iModeNotif;
	};

#endif // ___CKEYLOCKPOLICYAPIIMPL__
