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


#include "keylockpolicyapi.h"
#include "keylockpolicyapiimpl.h"

// ---------------------------------------------------------------------------
// Standard two-phased construction
// ---------------------------------------------------------------------------
//
EXPORT_C CKeyLockPolicyApi* CKeyLockPolicyApi::NewL( TLockPolicyType aType )
	{
	CKeyLockPolicyApi *self = new ( ELeave ) CKeyLockPolicyApi( );
	CleanupStack::PushL( self );
	self->ConstructL( aType );
	CleanupStack::Pop( self );
	return self;
	}

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
EXPORT_C CKeyLockPolicyApi::~CKeyLockPolicyApi( )
	{
	delete iKeylockpolicyImplementation;
	iKeylockpolicyImplementation = NULL;
	}

// ---------------------------------------------------------------------------
// Returns true if the policy initialization has been successful.
// ---------------------------------------------------------------------------
//
EXPORT_C TBool CKeyLockPolicyApi::HasConfiguration( )
	{
	return iKeylockpolicyImplementation->HasConfiguration( );
	}

// ---------------------------------------------------------------------------
// Set the central repository key to allow keyguard
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CKeyLockPolicyApi::EnableKeyguardFeature( )
	{
	return iKeylockpolicyImplementation->EnableKeyguardFeature( );
	}

// ---------------------------------------------------------------------------
// Set the central repository key to disable keyguard
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CKeyLockPolicyApi::DisableKeyguardFeature( )
	{
	return iKeylockpolicyImplementation->DisableKeyguardFeature( );
	}

// ---------------------------------------------------------------------------
// Return true if keyguard feature is allowed
// ---------------------------------------------------------------------------
//
EXPORT_C TBool CKeyLockPolicyApi::KeyguardAllowed( )
	{
	return CKeyLockPolicyApiImpl::KeyguardAllowed( );
	}

// ---------------------------------------------------------------------------
// Add new key combination to the selected policy type+mode combination
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CKeyLockPolicyApi::AddKeyCombination( TUint32 aPrimaryKey, TUint32 aSecondaryKey )
	{
	return iKeylockpolicyImplementation->AddKeyCombination(aPrimaryKey, aSecondaryKey);
	}

// ---------------------------------------------------------------------------
// Removes a key set defined in the selected policy type+mode combination
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CKeyLockPolicyApi::RemoveKeyCombination( TUint32 aPrimaryKey, TUint32 aSecondaryKey )
	{
	return iKeylockpolicyImplementation->RemoveKeyCombination(aPrimaryKey, aSecondaryKey);
	}

// ---------------------------------------------------------------------------
// Receives a key set from spesified index
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CKeyLockPolicyApi::GetKeyCombination( TInt aIndex, TUint32 &aPrimaryKey, TUint32 &aSecondaryKey )
	{
	return iKeylockpolicyImplementation->GetKeyCombination(aIndex, aPrimaryKey, aSecondaryKey);
	}

// ---------------------------------------------------------------------------
// Clear all key combinations from the selected policy type+mode combination
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CKeyLockPolicyApi::ClearKeycombinations( )
	{
	return iKeylockpolicyImplementation->ClearKeycombinations( );
	}

// ---------------------------------------------------------------------------
// Receives keys and checks whether they match any primary+secondary key combination
// ---------------------------------------------------------------------------
//
EXPORT_C TBool CKeyLockPolicyApi::HandleKeyEventL( const TKeyEvent& aKeyEvent, const TEventCode aType )
	{
	return iKeylockpolicyImplementation->HandleKeyEventL(aKeyEvent, aType);
	}

// ---------------------------------------------------------------------------
// Returns true if the last received key matched any of the primary keys
// stored in the policy
// ---------------------------------------------------------------------------
//
EXPORT_C TBool CKeyLockPolicyApi::PrimaryKeyPressed( )
	{
	return iKeylockpolicyImplementation->PrimaryKeyPressed( );
	}

// ---------------------------------------------------------------------------
// Returns the last received key if it was a primary key
// ---------------------------------------------------------------------------
//
EXPORT_C TUint CKeyLockPolicyApi::GetLastPrimaryKey( )
	{
	return iKeylockpolicyImplementation->GetLastPrimaryKey( );
	}

// ---------------------------------------------------------------------------
// Default C++ constructor
// ---------------------------------------------------------------------------
//
CKeyLockPolicyApi::CKeyLockPolicyApi( )
	{
	}

// ---------------------------------------------------------------------------
// Keylock policy construction
// ---------------------------------------------------------------------------
//
void CKeyLockPolicyApi::ConstructL( TLockPolicyType aType )
	{
	iKeylockpolicyImplementation = CKeyLockPolicyApiImpl::NewL( aType );
	}

// EOF
