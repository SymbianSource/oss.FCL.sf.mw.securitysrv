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
* Description:  Lock App internal state control
 *
*/


#include "lockappstatecontrol.h"
#include "lockappstatepublisher.h"
#include "lockappkeyguardcontrol.h"
#include "lockapputils.h"
#include "lockappecsdetector.h"

const TInt KLockAppObserverListGranularity = 4;

// ---------------------------------------------------------------------------
// Standard C++ constructor
// ---------------------------------------------------------------------------
CLockAppObserverList::CLockAppObserverList( )
    {
    // no implementation required
    }

// ---------------------------------------------------------------------------
// Destroys the observer list and all existing observers.
// ---------------------------------------------------------------------------
CLockAppObserverList::~CLockAppObserverList( )
    {
    if ( iObserverList )
        {
        TInt count = iObserverList->Count( );
        for (TInt index=0; index<count; index++ )
            {
            delete (*iObserverList)[index];
            }
        iObserverList->Reset( );
        delete iObserverList;
        }
    }

// ---------------------------------------------------------------------------
// Adds lock state observer to the observer list.
// ---------------------------------------------------------------------------
void CLockAppObserverList::AddObserverL( MLockAppStateObserver* aObserver )
    {
    if ( aObserver )
        {
        TBool found(EFalse);
        // check that the observer has not been added before
        TInt count = iObserverList->Count( );
        for ( TInt index=0; index<count; index++ )
            {
            if ( ((*iObserverList)[index]) == aObserver )
                {
                found = ETrue;
                }
            }
        if ( !found )
            {
            iObserverList->AppendL( aObserver );
            }
        }
    }

// ---------------------------------------------------------------------------
// Removes lock state observer from the observer list.
// ---------------------------------------------------------------------------
void CLockAppObserverList::RemoveObserver( MLockAppStateObserver* aObserver )
    {
    if ( aObserver )
        {
        TInt count = iObserverList->Count( );
        for ( TInt index=0; index<count; index++ )
            {
            if ( ((*iObserverList)[index]) == aObserver )
                {
                iObserverList->Remove( index );
                return;
                }
            }
        }
    }

// ---------------------------------------------------------------------------
// Constructs the observer list.
// ---------------------------------------------------------------------------
void CLockAppObserverList::BaseConstructL( )
    {
    // create observer list
    iObserverList = new (ELeave) RPointerArray<MLockAppStateObserver>(KLockAppObserverListGranularity);
    }

// ---------------------------------------------------------------------------
// Not implemented, meant for derived classes
// ---------------------------------------------------------------------------
void CLockAppObserverList::HandleLockStatusChangedL( TLockStatus /*aLockStatus*/)
    {
    }

// ---------------------------------------------------------------------------
// Informs observers that the lock state has changed.
// ---------------------------------------------------------------------------
void CLockAppObserverList::PostStatusChangeL( TLockStatus aStatusChange )
    {
    // first inform the main control
    HandleLockStatusChangedL( aStatusChange );

    // for child observers
    TInt count = iObserverList->Count( );
    for ( TInt index=count-1; index>=0; index-- )
        {
        //TRAP_IGNORE
        ((*iObserverList)[index])->HandleLockStatusChangedL( aStatusChange );
        }
    }
