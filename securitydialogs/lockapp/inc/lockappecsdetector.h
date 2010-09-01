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
* Description:  Provides emergency call support for keyguard/devicelock
 *
*/


#ifndef __LOCKAPPSTATEDETECTOR_H__
#define __LOCKAPPSTATEDETECTOR_H__

// INCLUDES
#include "lockappstateobserver.h"
#include <AknEcs.h> // MAknEcsObserver and CAknEcsDetector

// FORWARD DECLARATIONS
class CLockAppEcsNote;

/**
 *  CLockAppEcsDetector class owns emergency note and emergency detector.
 *  Taps directly to AppUi windowserver event source for receiving key events.
 *  If user presses emergency numbers defined in SIM, emergency note is shown.
 *  Only works when keyguard or devicelock is activated.
 *
 *  @lib    lockapp
 *  @since  5.0
 *  @author Joona Petrell
 *  @author Tamas Koteles
 */
class CLockAppEcsDetector : public CBase, public MLockAppStateObserver, public MAknEcsObserver
    {
    public:

        /**
         * Two-phased constructor.
         */
        static CLockAppEcsDetector* NewL( );

        /**
         * Destructor
         */
        ~CLockAppEcsDetector( );

    private:

        /**
         * C++ default constructor.
         */
        CLockAppEcsDetector( );

        /**
         * Second constructor that can fail (leave).
         */
        void ConstructL( );

    public:

        /**
         * From @c CLockAppStateObserver. Method handles the lock state changes.
         *
         * @param aLockStatus The new lock state
         */
        virtual void HandleLockStatusChangedL( TLockStatus aLockStatus );

    private:

        /**
         * From @c MAknEcsObserver. Handles changes in emergency call detector.
         * @param aEcsDetector a pointer to ecsdetector component
         * @param aState the new emergency detector state.
         */
        void HandleEcsEvent( CAknEcsDetector* aEcsDetector, CAknEcsDetector::TState aState );

    public:

        /**
         * Emergency note is visible.
         *
         * @return true if emergency note is on the screen.
         */
        TBool EcsNoteOnScreen( ) const;

        /**
         * Internal Method only used for testing since emergency number
         * detection does not work in emulator.
         *
         * @return KErrNone if there were no problems
         */
        TInt TestEcsNote( );

    private:

        /**
         * Receives emergency number key events and handles emergency dialing.
         * Own.
         */
        CAknEcsDetector* iEcsDetector;

        /**
         * Emergency note to be shown when user has dialed emergency number.
         * Own.
         */
        CLockAppEcsNote* iEcsNote;

    };

#endif // __LOCKAPPSTATEDETECTOR_H__
// End of File

