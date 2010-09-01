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
* Description:  Key pattern matching component.
 *
*/


#include "lockapptrace.h"
#include "lockappkeypattern.h"

const TInt KDefaultTimeout = 2000000;

// ---------------------------------------------------------------------------
// Standard two-phased construction
// ---------------------------------------------------------------------------
CLockAppKeyPattern* CLockAppKeyPattern::NewL( )
    {
    CLockAppKeyPattern *self = new ( ELeave ) CLockAppKeyPattern();
    CleanupStack::PushL( self );
    self->ConstructL( );
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Default C++ constructor
// ---------------------------------------------------------------------------
CLockAppKeyPattern::CLockAppKeyPattern( ) :
    iKeyTimeOut( KDefaultTimeout ),
    iState( EPatternNoMatch ),
    iEnabled ( ETrue ),
    iHasPatterns( EFalse )
    {
    // no implementation required
    }

// ---------------------------------------------------------------------------
// Key pattern construction
// ---------------------------------------------------------------------------
void CLockAppKeyPattern::ConstructL( )
    {
    iKeyTimer = CPeriodic::NewL( CActive::EPriorityUserInput );
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
CLockAppKeyPattern::~CLockAppKeyPattern( )
    {
    ClearPatterns( );
    // timer between key presses is cancelled
    if ( iKeyTimer )
        {
        iKeyTimer->Cancel( );
        delete iKeyTimer;
        }
    }

// ---------------------------------------------------------------------------
// Gets the enabled state.
// ---------------------------------------------------------------------------
TBool CLockAppKeyPattern::IsEnabled( )
    {
    return iEnabled;
    }

// ---------------------------------------------------------------------------
// Sets the enabled state.
// ---------------------------------------------------------------------------
void CLockAppKeyPattern::SetEnabled(TBool aEnabled )
    {
    iEnabled = aEnabled;
    }

// ---------------------------------------------------------------------------
// Gets the primary key timeout.
// ---------------------------------------------------------------------------
TUint CLockAppKeyPattern::GetKeyTimeOut( )
    {
    return iKeyTimeOut;
    }

// ---------------------------------------------------------------------------
// Sets the primary key timeout.
// ---------------------------------------------------------------------------
void CLockAppKeyPattern::SetKeyTimeOut(TUint aTimeOut )
    {
    iKeyTimeOut = aTimeOut;
    }

// ---------------------------------------------------------------------------
// Add new key combination
// ---------------------------------------------------------------------------
TInt CLockAppKeyPattern::AddPattern( TUint32 aPrimaryKey, TUint32 aSecondaryKey )
    {
    TInt err(KErrNone);
    iPrimaryKeys.Append( aPrimaryKey );
    iSecondaryKeys.Append( aSecondaryKey );
    iHasPatterns = ETrue;
    return err;
    }

// ---------------------------------------------------------------------------
// Get key combination
// ---------------------------------------------------------------------------
TInt CLockAppKeyPattern::GetPattern(TInt aIndex, TUint32& aPrimaryKey, TUint32& aSecondaryKey )
    {
    TInt err( KErrNone);
    if ( iHasPatterns && aIndex < iPrimaryKeys.Count( )&& aIndex < iSecondaryKeys.Count( ) )
        {
        aPrimaryKey = iPrimaryKeys[aIndex];
        aSecondaryKey = iSecondaryKeys[aIndex];
        }
    else
        {
        err = KErrNotFound;
        }
    return err;
    }

// ---------------------------------------------------------------------------
// Returns true if the key pattern matcher has been initialized
// ---------------------------------------------------------------------------
TBool CLockAppKeyPattern::HasPatterns( )
    {
    return (iHasPatterns && iPrimaryKeys.Count() > 0 && iSecondaryKeys.Count() > 0);
    }

// ---------------------------------------------------------------------------
// Clear all key combinations
// ---------------------------------------------------------------------------
TInt CLockAppKeyPattern::ClearPatterns( )
    {
    TInt err( KErrNone);
    if ( iHasPatterns )
        {
        iPrimaryKeys.Reset( );
        iSecondaryKeys.Reset( );
        iWaitingKeys.Reset( );
        iHasPatterns = EFalse;
        // cancel the timer
        iKeyTimer->Cancel( );
        }
    return err;
    }

// ---------------------------------------------------------------------------
// Receives keys and checks whether they match any primary+secondary key combination
// ---------------------------------------------------------------------------
TPatternState CLockAppKeyPattern::HandleKeyEvent( TUint32 aKey )
    {  
    if ( iEnabled )
        {
        if ( iHasPatterns )
            {
            switch (iState)
                {
                case EPatternNoMatch:
                case EPatternSecondaryMatch:
                    {
                    if ( iPrimaryKeys.Find( aKey ) != KErrNotFound )
                        {
                        HandlePrimaryKeyEvent( aKey );
                        }
                    else
                        {
                        HandleOtherKeyEvent();
                        }
                    }
                    break;
                case EPatternPrimaryMatch:
                    {
                    if ( iWaitingKeys.Find( aKey ) != KErrNotFound )
                        {
                        HandleSecondaryKeyEvent();
                        }
                    else
                        {
                        if ( iPrimaryKeys.Find( aKey ) != KErrNotFound )
                            {
                            HandlePrimaryKeyEvent( aKey );
                            }
                        else
                            {
                            HandleOtherKeyEvent();
                            }
                        }
                    }
                    break;
                default:
                    break;
                }
            }
        else
            {
            return EPatternNotInitialized;
            }
        }
    else
        {
        return EPatternNotEnabled;
        }

    return iState;
    }

// ---------------------------------------------------------------------------
// Handles primary key events
// ---------------------------------------------------------------------------
void CLockAppKeyPattern::HandlePrimaryKeyEvent( TUint32 aKey )
    {
    // cancel the timer
    iKeyTimer->Cancel( );
    // clear the waiting keys
    iWaitingKeys.Reset( );
    // add those secondary keys to the waiting keys, which have the same primary key
    for (TInt i( 0); i < iPrimaryKeys.Count( ); i++ )
        {
        if ( iPrimaryKeys[i] == aKey && i < iSecondaryKeys.Count( ) )
            {
            iWaitingKeys.Append( iSecondaryKeys[i] );
            }
        }

    // start timer for primary key timeout
    iKeyTimer->Start( iKeyTimeOut, iKeyTimeOut, TCallBack( HandleKeyTimerTimeout, this ) );

    // set the state
    iState = EPatternPrimaryMatch;
    }

// ---------------------------------------------------------------------------
// Handles secondary key events
// ---------------------------------------------------------------------------
void CLockAppKeyPattern::HandleSecondaryKeyEvent( )
    {
    // cancel the timer
    iKeyTimer->Cancel( );
    // clear the waiting keys
    iWaitingKeys.Reset( );
    // set the state
    iState = EPatternSecondaryMatch;
    }

// ---------------------------------------------------------------------------
// Handles other key events
// ---------------------------------------------------------------------------
void CLockAppKeyPattern::HandleOtherKeyEvent( )
    {
    // cancel the timer
    iKeyTimer->Cancel( );
    // clear the waiting keys
    iWaitingKeys.Reset( );
    // set the state
    iState = EPatternNoMatch;
    }

// ---------------------------------------------------------------------------
// A call back to the key pattern timer, the allowed time window for pressing
// the secondary key to get a match has passed.
// ---------------------------------------------------------------------------
TInt CLockAppKeyPattern::HandleKeyTimerTimeout( TAny* aSelf )
    {
    CLockAppKeyPattern* self = reinterpret_cast< CLockAppKeyPattern* >( aSelf );
    // reset the state
    self->HandleOtherKeyEvent( );
    return KErrNone;
    }

// End of file
