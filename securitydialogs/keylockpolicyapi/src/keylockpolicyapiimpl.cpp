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
* Description:  Keylock policy Implementation
 *
*/


#include "keylockpolicyapiimpl.h"
#include <LockDomainCRKeys.h>

#include <centralrepository.h>
#include "gendebug.h"

const TUint32 KModeShift = 16;
const TUint32 KTypeShift = 12;

// ---------------------------------------------------------------------------
// Standard two-phased construction
// ---------------------------------------------------------------------------
//
CKeyLockPolicyApiImpl* CKeyLockPolicyApiImpl::NewL( TLockPolicyType aType )
	{
	TraceDump( INFO_LEVEL, _L( "CKeyLockPolicyApiImpl::NewL()" ) );
	CKeyLockPolicyApiImpl *self = new ( ELeave ) CKeyLockPolicyApiImpl( aType );
	CleanupStack::PushL( self );
	self->ConstructL( );
	CleanupStack::Pop( self );
	return self;
	}

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CKeyLockPolicyApiImpl::~CKeyLockPolicyApiImpl( )
	{
	// notifier listeners are stopped
	if ( iAllowNotif )
		{
		iAllowNotif->StopListening( );
		}
	if ( iModeNotif )
		{
		iModeNotif->StopListening( );
		}

	// keys are removed from the arrays
	iPrimaryKeys.Reset( );
	iSecondaryKeys.Reset( );
	iWaitingKeys.Reset( );

	// timer between key presses is cancelled
	if ( iKeyLockTimer )
		{
		iKeyLockTimer->Cancel( );
		}

	// childs are destroyed
	delete iKeyLockTimer;
	delete iAllowNotif;
	delete iModeNotif;
	delete iRep;
	}

// ---------------------------------------------------------------------------
// Returns true if the policy initialization has been successful.
// ---------------------------------------------------------------------------
//
TBool CKeyLockPolicyApiImpl::HasConfiguration( )
	{
	return iInitialised;
	}

// ---------------------------------------------------------------------------
// Set the central repository key to allow keyguard
// ---------------------------------------------------------------------------
//
TInt CKeyLockPolicyApiImpl::EnableKeyguardFeature( )
	{
	TraceDump( INFO_LEVEL, _L( "ENTER: CKeyLockPolicyApiImpl::EnableKeyguardFeature()" ) );
	TInt err( KErrUnknown);

	if ( iRep )
		{
		err = iRep->Set( KLockKeyguardAllow, ETrue );
		}

	TraceDump( INFO_LEVEL, _L( "EXIT : CKeyLockPolicyApiImpl::EnableKeyguardFeature()" ) );
	return err;
	}

// ---------------------------------------------------------------------------
// Set the central repository key to disable keyguard
// ---------------------------------------------------------------------------
//
TInt CKeyLockPolicyApiImpl::DisableKeyguardFeature( )
	{
	TraceDump( INFO_LEVEL, _L( "ENTER: CKeyLockPolicyApiImpl::DisableKeyguardFeature()" ) );
	TInt err( KErrUnknown);

	if ( iRep )
		{
		err = iRep->Set( KLockKeyguardAllow, EFalse );
		}

	TraceDump( INFO_LEVEL, _L( "EXIT : CKeyLockPolicyApiImpl::DisableKeyguardFeature()" ) );
	return err;
	}

// ---------------------------------------------------------------------------
// Return true if keyguard feature is allowed
// ---------------------------------------------------------------------------
//
TBool CKeyLockPolicyApiImpl::KeyguardAllowed( )
	{
	TraceDump( INFO_LEVEL, _L( "ENTER: CKeyLockPolicyApiImpl::KeyguardAllowed()" ) );
	TBool ret( ETrue);

	TInt tmp( 0);
	CRepository* rep=  NULL;
	TRAPD( err, rep = CRepository::NewL( KCRUidLockConf ) )
	if ( err || !rep )
		{
		ret = ETrue;
		}
	else
		if ( rep && rep->Get( KLockKeyguardAllow, tmp )== KErrNone )
			{
			if ( tmp )
				{
				ret = ETrue;
				}
			else
				{
				ret = EFalse;
				}
			}
	delete rep;

	TraceDump( INFO_LEVEL, _L( "EXIT : CKeyLockPolicyApiImpl::KeyguardAllowed()" ) );
	return ret;
	}

// ---------------------------------------------------------------------------
// Add new key combination to the selected policy type+mode combination
// ---------------------------------------------------------------------------
//
TInt CKeyLockPolicyApiImpl::AddKeyCombination( TUint32 aPrimaryKey, TUint32 aSecondaryKey )
	{
	TraceDump( INFO_LEVEL, _L( "ENTER: CKeyLockPolicyApiImpl::AddKeyCombination()" ) );
	TInt err( KErrNone);

	// mode zero means no policies are defined
	// and the implementation needs the repository
	if ( iMode && iRep )
		{
		TInt putindex( 0);
		err = GetNewKeyCombinationIndex( putindex );
		err = SetKeyCombination( aPrimaryKey, aSecondaryKey, putindex );
		}
	else
		{
		err = KErrUnknown;
		}

	TraceDump( INFO_LEVEL, _L( "EXIT : CKeyLockPolicyApiImpl::AddKeyCombination()" ) );
	return err;
	}

// ---------------------------------------------------------------------------
// Removes a key set defined in the selected policy type+mode combination
// ---------------------------------------------------------------------------
//
TInt CKeyLockPolicyApiImpl::RemoveKeyCombination( TUint32 aPrimaryKey, TUint32 aSecondaryKey )
	{
	TraceDump( INFO_LEVEL, _L( "ENTER: CKeyLockPolicyApiImpl::RemoveKeyCombination()" ) );
	TInt err( KErrNone);

	// the implementation needs the repository
	if ( iMode && iRep )
		{
		TInt removeindex( 0);
		err = FindKeyCombinationIndex( aPrimaryKey, aSecondaryKey, removeindex );
		if ( err == KErrNone )
			{
			TInt lastindex( 0);
			TUint32 tmpprimary( 0), tmpsecondary( 0);

			// find out the index of the last key set
			err = GetNewKeyCombinationIndex( lastindex );
			lastindex--;

			// collect the keys of the last existing key set
			err = GetKeyCombination( lastindex, tmpprimary, tmpsecondary );
			if ( err == KErrNone )
				{
				err = SetKeyCombination( tmpprimary, tmpsecondary, removeindex );
				if ( err == KErrNone )
					{
					// delete the already copied last key set
					iRep->Delete( iMask | KLockFirstPrimaryKey + lastindex );
					iRep->Delete( iMask | KLockFirstSecondaryKey + lastindex );
					}
				}
			}
		else
			{
			// if err != KErrNone either keys were not found or
			// there are problems in the repository
			}
		}
	else
		{
		err = KErrUnknown;
		}

	TraceDump( INFO_LEVEL, _L( "EXIT : CKeyLockPolicyApiImpl::RemoveKeyCombination()" ) );
	return err;
	}

// ---------------------------------------------------------------------------
// Finds the index of the given combination.
// ---------------------------------------------------------------------------
//
TInt CKeyLockPolicyApiImpl::FindKeyCombinationIndex(TUint32 aPrimaryKey, TUint32 aSecondaryKey,
		TInt &aIndex )
	{
	TraceDump( INFO_LEVEL, _L( "ENTER: CKeyLockPolicyApiImpl::FindKeyCombinationIndex()" ) );
	TInt err( KErrNone);

	if ( iMode && iRep )
		{
		TInt index( 0), tmpprimary( 0), tmpsecondary( 0);

		// go trough the keys until you find the primary key and secondary key
		while (err == KErrNone && tmpprimary != aPrimaryKey && tmpsecondary != aSecondaryKey )
			{
			err = iRep->Get( iMask | KLockFirstPrimaryKey + index, tmpprimary );
			if ( err == KErrNone )
				{
				err = iRep->Get( iMask | KLockFirstSecondaryKey + index, tmpsecondary );
				}
			index++;
			}

		// if we found it for sure
		if ( tmpprimary == aPrimaryKey && tmpsecondary == aSecondaryKey )
			{
			aIndex = index - 1;
			err = KErrNone;
			}
		}
	else
		{
		err = KErrUnknown;
		}

	TraceDump( INFO_LEVEL, _L( "EXIT : CKeyLockPolicyApiImpl::FindKeyCombinationIndex()" ) );
	return err;
	}

// ---------------------------------------------------------------------------
// Finds the index for a new combination (last one + 1).
// ---------------------------------------------------------------------------
//
TInt CKeyLockPolicyApiImpl::GetNewKeyCombinationIndex(TInt &aIndex )
	{
	TraceDump( INFO_LEVEL, _L( "ENTER: CKeyLockPolicyApiImpl::GetNewKeyCombinationIndex()" ) );
	TInt err( KErrNone);

	if ( iMode && iRep )
		{
		TInt index( 0), tmpvar( 0);
		while ( iRep->Get( iMask | KLockFirstPrimaryKey + index, tmpvar )== KErrNone )
			{
			index++;
			};

		// set the return value
		aIndex = index;
		}
	else
		{
		err = KErrUnknown;
		}

	TraceDump( INFO_LEVEL, _L( "EXIT : CKeyLockPolicyApiImpl::GetNewKeyCombinationIndex()" ) );
	return err;
	}

// ---------------------------------------------------------------------------
// Sets a combination at the given index.
// ---------------------------------------------------------------------------
//
TInt CKeyLockPolicyApiImpl::SetKeyCombination(TUint32 aPrimaryKey, TUint32 aSecondaryKey,
		TInt aIndex )
	{
	TraceDump( INFO_LEVEL, _L( "ENTER: CKeyLockPolicyApiImpl::SetKeyCombination()" ) );
	TInt err( KErrNone);

	if ( iMode && iRep )
		{
		// set the last key set over the removed key combination
		err = iRep->Set( iMask | KLockFirstPrimaryKey + aIndex, (TInt) aPrimaryKey );
		if ( err == KErrNone )
			{
			err = iRep->Set( iMask | KLockFirstSecondaryKey + aIndex, (TInt) aSecondaryKey );
			}
		}
	else
		{
		err = KErrUnknown;
		}

	TraceDump( INFO_LEVEL, _L( "EXIT : CKeyLockPolicyApiImpl::SetKeyCombination()" ) );
	return err;
	}

// ---------------------------------------------------------------------------
// Returns a key combination from spesified index
// ---------------------------------------------------------------------------
//
TInt CKeyLockPolicyApiImpl::GetKeyCombination( TInt aIndex, TUint32 &aPrimaryKey,
		TUint32 &aSecondaryKey )
	{
	TraceDump( INFO_LEVEL, _L( "ENTER: CKeyLockPolicyApiImpl::GetKeyCombination()" ) );
	TInt err( KErrNone);

	// the implementation needs the repository
	if ( iMode && iRep )
		{
		TInt tmppri( 0), tmpsec( 0);

		// collect the primary key from the index
		err = iRep->Get( iMask | KLockFirstPrimaryKey + aIndex, tmppri );
		if ( err == KErrNone )
			{
			// devicelock query needs only one key
			if ( iType == EPolicyDevicelockQuery )
				{
				tmpsec = 0;
				}
			else
				{
				// collect the secondary key from the index
				err = iRep->Get( iMask | ( KLockFirstSecondaryKey + aIndex ), tmpsec );
				}

			if ( err == KErrNone )
				{
				TraceDump( INFO_LEVEL, ( _L( "CKeyLockPolicyApiImpl::GetKeyCombination(): key combination: 0x%x, 0x%x" ), tmppri, tmpsec ) );
				}
			else
				{
				// if err != KErrNone either the secondary key was
				// not found or there are problems in the repository
				}

			aPrimaryKey = TUint32( tmppri );
			aSecondaryKey = TUint32( tmpsec );
			}
		else
			{
			// if err != KErrNone either the primary key was
			// not found or there are problems in the repository
			}
		}
	else
		{
		err = KErrUnknown;
		}

	TraceDump( INFO_LEVEL, _L( "EXIT : CKeyLockPolicyApiImpl::GetKeyCombination()" ) );
	return err;
	}

// ---------------------------------------------------------------------------
// Clear all key combinations from the selected policy type+mode combination
// ---------------------------------------------------------------------------
//
TInt CKeyLockPolicyApiImpl::ClearKeycombinations( )
	{
	TraceDump( INFO_LEVEL, _L( "ENTER: CKeyLockPolicyApiImpl::ClearKeycombinations()" ) );
	TInt err( KErrNone);

	// the implementation needs the repository
	if ( iMode && iRep )
		{
		TInt index( 0), tmpvar( 0);
		// go trough the primary keys and delete them
		while ( iRep->Get( iMask | KLockFirstPrimaryKey + index, tmpvar )== KErrNone )
			{
			err = iRep->Delete( iMask | KLockFirstPrimaryKey + index );
			index++;
			}

		index = 0;
		// go trough the secondary keys and delete them
		while ( iRep->Get( iMask | KLockFirstSecondaryKey + index, tmpvar )== KErrNone )
			{
			err = iRep->Delete( iMask | KLockFirstSecondaryKey + index );
			index++;
			}
		}
	else
		{
		err = KErrUnknown;
		}

	TraceDump( INFO_LEVEL, _L( "EXIT : CKeyLockPolicyApiImpl::ClearKeycombinations()" ) );
	return err;
	}

// ---------------------------------------------------------------------------
// Receives keys and checks whether they match any primary+secondary key combination
// ---------------------------------------------------------------------------
//
TBool CKeyLockPolicyApiImpl::HandleKeyEventL( const TKeyEvent& aKeyEvent, const TEventCode aType )
	{
	TBool matchFound( EFalse);
	// keylock policy only handles key events of type EEventKeyDown
	TEventCode type( EEventKeyDown);

	if ( aType == type )
		{
		// configuration keys in general are not handled if the policy has not been not initialized
		// configuration keys for enabling are not handled if keyguard feature is disabled
		if ( iInitialised && ( iAllowed || iType != EPolicyActivateKeyguard ) )
			{

			// if primary key has already been pressed check if the key matches
			// the subsequent secondary key stored in the waiting keys array
			if ( iPrimaryKeyDown )
				{
				TraceDump( INFO_LEVEL, ( _L( "CKeyLockPolicyApiImpl::HandleKeyEventL() : iPrimaryKeyDown, Mode: 0x%x" ), iMode ) );
				if ( iWaitingKeys.Find( aKeyEvent.iScanCode )!= KErrNotFound )
					{
					TraceDump( INFO_LEVEL, _L( "CKeyLockPolicyApiImpl::HandleKeyEventL() : Secondary key Pressed" ) );
					matchFound = ETrue;
					}
				// cancel the timer after secondary key candidate has been pressed
				iKeyLockTimer->Cancel( );
				iWaitingKeys.Reset( );
				}

			// if the received key did not provide a match for secondary keys
			// the primary key match is investigated
			if ( !matchFound )
				{
				// check if the key matches with primary key any of the primary keys
				TInt prikeyIndex( iPrimaryKeys.Find( aKeyEvent.iScanCode ));

				if ( prikeyIndex != KErrNotFound )
					{
					TraceDump( INFO_LEVEL, _L( "CKeyLockPolicyApiImpl::HandleKeyEventL() : Primary key pressed" ) );

					// store the index of matched primary key
					iLastPrimaryKeyIndex = prikeyIndex;

					// devicelock query only needs the primary key to match
					if ( iType == EPolicyDevicelockQuery )
						{
						matchFound = ETrue;
						}
					else
						{
						// collect the possible secondary key candidates to the waiting keys array
						iWaitingKeys.Reset( );
						for (TInt x( 0); x < iPrimaryKeys.Count( ); x++ )
							{
							if ( iPrimaryKeys[x] == aKeyEvent.iScanCode && iSecondaryKeys.Count( )> x )
								{
								TraceDump( INFO_LEVEL, ( _L( "CKeyLockPolicyApiImpl::HandleKeyEventL() : Waiting for key: 0x%x" ), iSecondaryKeys[x] ) );
								iWaitingKeys.Append( iSecondaryKeys[x] );
								}
							}

						// start timer for primary key timeout
						iKeyLockTimer->Start( iTimeout, iTimeout, TCallBack(
								HandleKeyLockTimerTimeout, this ) );

						// primary key has been pressed
						iPrimaryKeyDown = ETrue;
						}
					}
				else
					{
					// if no match was found cancel timer and start over
					TraceDump( INFO_LEVEL, ( _L( "CKeyLockPolicyApiImpl::HandleKeyEventL() : Unknown key, not handled: 0x%x" ), aKeyEvent.iScanCode ) );

					iPrimaryKeyDown = EFalse;
					iKeyLockTimer->Cancel( );
					iLastPrimaryKeyIndex = 0xffffffff;
					}
				}
			}
		else
			{
			TraceDump( INFO_LEVEL, ( _L( "CKeyLockPolicyApiImpl::HandleKeyEventL() : not initialized or not allowed" ) ) );
			}
		}
	return matchFound;
	}

// ---------------------------------------------------------------------------
// Returns true if the last received key matched any of the primary keys
// stored in the policy
// ---------------------------------------------------------------------------
//
TBool CKeyLockPolicyApiImpl::PrimaryKeyPressed( )
	{
	return iPrimaryKeyDown;
	}

// ---------------------------------------------------------------------------
// Returns the last received key if it was a primary key
// ---------------------------------------------------------------------------
//
TUint CKeyLockPolicyApiImpl::GetLastPrimaryKey( )
	{
	if ( iLastPrimaryKeyIndex < iPrimaryKeys.Count( ) )
		{
		return iPrimaryKeys[iLastPrimaryKeyIndex];
		}
	else
		{
		return 0;
		}
	}

// ---------------------------------------------------------------------------
// MCenRepNotifyHandlerCallback. Changes in the observed central
// repository keys are communicated trough this method.
// ---------------------------------------------------------------------------
//
void CKeyLockPolicyApiImpl::HandleNotifyInt( TUint32 aId, TInt aNewValue )
	{
	switch ( aId )
		{
		case KLockKeyguardAllow:
            {
			// keyguard feature has been enabled/disabled
			TraceDump( INFO_LEVEL, _L( "CKeyLockPolicyApiImpl::HandleNotifyInt() : KLockKeyguardAllow" ) );
			aNewValue == 0 ? iAllowed = EFalse : iAllowed = ETrue;
            }
			break;
			
		case KLockPolicyMode:
            {
			// keylock policy mode has been changed
			TraceDump( INFO_LEVEL, _L( "CKeyLockPolicyApiImpl::HandleNotifyInt() : KLockPolicyMode" ) );
			// mask has changed
			iMask = iType << KTypeShift|aNewValue << KModeShift;
			// key configuration has changed
			iPrimaryKeys.Reset( );
			iSecondaryKeys.Reset( );
			TUint32 x(0), k1(0), k2(0);
			while ( GetKeyCombination( x, k1, k2 )== KErrNone )
				{
				iPrimaryKeys.Append( k1 );
				iSecondaryKeys.Append( k2 );
				x++;
				}
			// reset the memory
			iPrimaryKeyDown = EFalse;
			iLastPrimaryKeyIndex = 0xffffffff;
            }
			break;
		
		default:
			break;
		}
	}

// ---------------------------------------------------------------------------
// Default C++ constructor
// ---------------------------------------------------------------------------
//
CKeyLockPolicyApiImpl::CKeyLockPolicyApiImpl( TLockPolicyType aType ) :
	iMode( 1), iType( aType), iInitialised( EFalse)
	{
	iMask = aType << KTypeShift;
	}

// ---------------------------------------------------------------------------
// Keylock policy construction
// ---------------------------------------------------------------------------
//
void CKeyLockPolicyApiImpl::ConstructL( )
	{
	TraceDump( INFO_LEVEL, _L( "CKeyLockPolicyApiImpl::ConstructL() started" ) );
	TInt err( KErrNone);

	// Create a connection to the repository
	TRAP( err, iRep = CRepository::NewL( KCRUidLockConf ) );
	if ( err == KErrNone )
		{
		// get the lock policy mode
		TInt mode( NULL);
		err = iRep->Get( KLockPolicyMode, mode );
		if ( err == KErrNone && mode )
			{
			iMode = mode;

			// every key in the configuration should have the mask below
			iMask |= iMode << KModeShift;

			// get the timeout between key presses
			TInt timeout( NULL);
			err = iRep->Get( KLockTimerInterval, timeout );
			if ( err == KErrNone && timeout > NULL )
				{
				iTimeout = timeout;

				// collect supported key configuration
				iPrimaryKeys.Reset( );
				iSecondaryKeys.Reset( );
				TUint32 x(0), k1(0), k2(0);
				while ( GetKeyCombination( x, k1, k2 )== KErrNone )
					{
					iPrimaryKeys.Append( k1 );
					iSecondaryKeys.Append( k2 );
					x++;
					}
				if ( iPrimaryKeys.Count( )> 0 && iSecondaryKeys.Count( )> 0 )
					{
					TraceDump( INFO_LEVEL, _L( "CRepository::NewL(): Initialised" ) );
					iInitialised = ETrue;
					}

				iPrimaryKeyDown = EFalse;
				iLastPrimaryKeyIndex = 0xffffffff;

				// whether keyguard feature is enabled or disabled
				// by default we want keyguard to be enabled
				TInt allow( 1 );
				iRep->Get( KLockKeyguardAllow, allow );
				iAllowed = allow;

				// create childs
				iKeyLockTimer = CPeriodic::NewL( CActive::EPriorityUserInput );
				iAllowNotif = CCenRepNotifyHandler::NewL( *this, *iRep,
						CCenRepNotifyHandler::EIntKey, KLockKeyguardAllow );
				iAllowNotif->StartListeningL( );
				iModeNotif = CCenRepNotifyHandler::NewL( *this, *iRep,
						CCenRepNotifyHandler::EIntKey, KLockPolicyMode );
				iModeNotif->StartListeningL( );
				}
			else
				{
				TraceDump( INFO_LEVEL, _L( "No time out defined: FAILED!" ) );
				}
			}
		else
			{
			TraceDump( INFO_LEVEL, _L( "No mode has been set: FAILED!" ) );
			}
		}
	else
		{
		TraceDump( INFO_LEVEL, _L( "CRepository::NewL( KCRUidLockConf ) ) FAILED!!" ) );
		}

	TraceDump( INFO_LEVEL, _L( "CKeyLockPolicyApiImpl::ConstructL() finished" ) );
	}

// ---------------------------------------------------------------------------
// A call back to the keylock timer, the allowed time window for pressing
// the secondary key to get a match has passed.
// ---------------------------------------------------------------------------
//
TInt CKeyLockPolicyApiImpl::HandleKeyLockTimerTimeout( TAny* aSelf )
	{
	TraceDump( INFO_LEVEL, _L( "CKeyLockPolicyApiImpl::HandleKeyLockTimerTimeout()" ) );
	CKeyLockPolicyApiImpl *self = reinterpret_cast< CKeyLockPolicyApiImpl* >( aSelf );

	// reset the memory
	self->iPrimaryKeyDown = EFalse;
	self->iLastPrimaryKeyIndex = 0xffffffff;
	self->iKeyLockTimer->Cancel( );

	return KErrNone;
	}

// EOF
