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
* Description:  Controls observers and publishes state changes
 *
*/


#ifndef __LOCKAPPOBSERVERLIST_H__
#define __LOCKAPPOBSERVERLIST_H__

// INCLUDES
#include <coecntrl.h>
#include "lockapp.hrh"

// FORWARD DECLARATIONS
class MLockAppStateObserver;

/**
 *  CLockAppObserverList class publishes lock state changes to all implemented state 
 *  observers. The list owns all childs and it is responsible for deleting them.
 *
 *  @lib    lockapp
 *  @since  5.0
 *  @author Joona Petrell
 *  @author Tamas Koteles
 */
class CLockAppObserverList : public CCoeControl
    {
    public:

        /**
         * C++ default constructor.
         */
        CLockAppObserverList( );

        /**
         * Destructor.
         */
        virtual ~CLockAppObserverList( );

    public:

        /**
         * Add new state observer.
         * @param aObserver lock state observer
         */
        void AddObserverL( MLockAppStateObserver* aObserver );

        /**
         * Remove lock state observer.
         * @param aObserver lock state observer
         */
        void RemoveObserver( MLockAppStateObserver* aObserver );

    protected:

        /**
         * Has to be called by the derived class in the construction.
         */
        void BaseConstructL( );

        /**
         * Informs all observers about the status change.
         *
         * @param aStatusChange the new lock state.
         */
        void PostStatusChangeL( TLockStatus aStatusChange );

        /**
         * Class that derives from observer list will have first
         * notification about the lock status chane.
         *
         * @param aLockStatus the new lock state.
         */
        virtual void HandleLockStatusChangedL( TLockStatus aLockStatus );

    private:

        /**
         * Dynamic list storing observers. Both the list and observers are owned.
         * Own.
         */
        RPointerArray<MLockAppStateObserver>* iObserverList;

    };

#endif // __LOCKAPPOBSERVERLIST_H__
// End of File
