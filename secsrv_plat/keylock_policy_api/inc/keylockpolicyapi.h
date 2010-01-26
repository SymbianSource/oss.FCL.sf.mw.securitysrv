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
* Description:  Keylock policy component offers a configuration support for keyguard
*
*/


#ifndef ___CKEYLOCKPOLICYAPI__
#define ___CKEYLOCKPOLICYAPI__

#include <w32std.h>
#include <e32base.h>

/**
 * Keylock policy component supports three policy types:
 *
 * EPolicyActivateKeyguard keys used for keyguard locking
 * EPolicyDeactivateKeyguard keys used for keyguard unlocking
 * EPolicyDevicelockQuery keys used for showing devicelock query
 */
enum TLockPolicyType
	{
	EPolicyActivateKeyguard = 1,
	EPolicyDeactivateKeyguard,
	EPolicyDevicelockQuery,
	};

// FORWARD DECLARATIONS
class CKeyLockPolicyApiImpl;

/**
 *  CKeyLockPolicyApi. Trough CKeyLockPolicyApi users can
 *
 *  @lib keylockpolicy.lib
 *  @since 3.2
 */
class CKeyLockPolicyApi : public CBase
	{
public:

	/**
	 * Use this to create keylock policy
	 * @param TLockPolicyType aType the policy used
	 */
	IMPORT_C static CKeyLockPolicyApi* NewL( TLockPolicyType aType );

	/**
	 * Destructor.
	 */
	IMPORT_C ~CKeyLockPolicyApi();

	/**
	 * If no policy has been defined for the type it cannot be used.
	 * @return ETrue if has succesfully loaded policy
	 */
	IMPORT_C TBool HasConfiguration();

	/**
	 * Activates support for keyguard feature so  the phone keys cannot
	 * be locked with keyguard. Does not affect the devicelock.
	 * @return standard Symbian error code
	 */
	IMPORT_C TInt EnableKeyguardFeature();

	/**
	 * Disables support for keyguard feature so the phone keys cannot
	 * be locked with keyguard. Does not affect the devicelock.
	 * @return standard Symbian error code
	 */
	IMPORT_C TInt DisableKeyguardFeature();

	/**
	 * Whether keyguard feature is currently enabled or disabled.
	 * @return ETrue if keyguard can be locked.
	 */
	IMPORT_C TBool static KeyguardAllowed();

	/**
	 * Adds a key combination for the policy type defined in the construction.
	 *
	 * @param TUint scan code for primary key
	 * @param TUint scan code for secondary key
	 * @return standard Symbian error code
	 */
	IMPORT_C TInt AddKeyCombination(TUint32 aPrimaryKey, TUint32 aSecondaryKey);

	/**
	 * Removes a key combination for the policy type defined in the construction.
	 *
	 * @param TUint scan code for primary key
	 * @param TUint scan code for secondary key
	 * @return standard Symbian error code
	 */
	IMPORT_C TInt RemoveKeyCombination(TUint32 aPrimaryKey, TUint32 aSecondaryKey);

	/**
	 * Retrieve a key combination on the given index.
	 *
	 * @param TInt aIndex index on the list that stores key combinations
	 * @param TUint key code for primary key
	 * @param TUint key code for secondary key
	 * @return standard Symbian error code
	 */
	IMPORT_C TInt GetKeyCombination(TInt aIndex, TUint32 &aPrimaryKey, TUint32 &aSecondaryKey);

	/**
	 * Clears all the defined key combinations from the policy.
	 *
	 * @return standard Symbian error code
	 */
	IMPORT_C TInt ClearKeycombinations();

	/**
	 * Handles Key events, returns ETrue the key events match any of the
	 * defined key combinations. Only uses scancodes with type EEventKeyDown.
	 *
	 * @param TKeyEvent aKeyEvent the actual key event
	 * @param TEventCode aType the type of the received key event
	 * @return ETrue if a match was found.
	 */
	IMPORT_C TBool HandleKeyEventL(const TKeyEvent& aKeyEvent, const TEventCode aType );

	/**
	 * Returns true if primary button is pressed and note should be shown.
	 * @return boolean true if primary key is pressed.
	 */
	IMPORT_C TBool PrimaryKeyPressed();

	/**
	 * Returns Primary key scan code that has been pressed last.
	 * @return scan code value of the primary key if found, NULL if not
	 */
	IMPORT_C TUint GetLastPrimaryKey();

private:

	/**
	 * C++ default constructor (private so cannot be derived).
	 */
	CKeyLockPolicyApi( );

	/**
	 * Second constructor initializes the policy
	 * @param TLockPolicyType aType the policy (TLockPolicyType) used
	 */
	void ConstructL( TLockPolicyType aType );

private:

	/**
	 * Actual implemetation.
	 */
	CKeyLockPolicyApiImpl* iKeylockpolicyImplementation;

	};

#endif // ___CKEYLOCKPOLICYAPI__

