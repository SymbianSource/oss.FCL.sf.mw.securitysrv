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
* Description:  Keyguard UI (window-owning compound control)
 *
*/


#ifndef __LOCKAPPKEYGUARDCONTROL_H__
#define __LOCKAPPKEYGUARDCONTROL_H__

// INCLUDES
#include "lockappbasecontrol.h"
#include "lockappobserverinterface.h"
#include <LockDomainCRKeys.h>

// FORWARD DECLARATIONS
class CLockAppPubSubObserver;
class CLockAppCenRepObserver;
class CUserActivityManager;

/**
 *  CLockAppKeyguardControl class represents the keyguard state in the state machine. 
 *  Window-owning compound control that provides visible keyguard user interface. 
 *  Owns all keyguard dialogs and commands received from the child controls like 
 *  dialogs and cba.
 *
 *  @lib    lockapp
 *  @since  5.0
 *  @author Joona Petrell
 *  @author Tamas Koteles
 *  @see    CLockAppBaseControl
 *  @see    MLockAppStateControl
 *  @see    CUserActivityManager
 *  @see    CLockAppPubSubObserver
 *  @see    CLockAppCenRepObserver
 */
class CLockAppKeyguardControl : public CLockAppBaseControl, public MEikCommandObserver,
        public MLockAppObserverInterface
    {
    public:

        /**
         * Two-phased constructor.
         *
         * @param aStateControl reference to the main state control
         */
        static CLockAppKeyguardControl* NewL( MLockAppStateControl& aStateControl );

        /**
         * Destructor.
         */
        ~CLockAppKeyguardControl( );

        TBool ActivationAllowedL( );

        TBool DeActivationAllowedL( );

        virtual void HandleActivateEventL( TUint aEnvMask );

        virtual void HandleDeActivateEventL( TUint aEnvMask );

        virtual void HandleEnvironmentChange( TUint aEnvMask, TUint aEventMask );

    private:

        /**
         * C++ default constructor.
         *
         * @param aStateControl reference to the main state control
         */
        CLockAppKeyguardControl( MLockAppStateControl& aStateControl );

        /**
         * Second constructor that can fail (leave).
         */
        void ConstructL( );

    public:

        void DisplayLockedNote( );

        void DisplayKeysLockedNote( );

        void DisplayKeysActiveNote( );

    public:

        void HandleResourceChange( TInt aType );

        TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType );

    public:

        /**
         * Handle Central Repository observer callback.
         */
        void HandleCenRepNotify( TUid aCenRepUid, TUint32 aKeyId, TInt aValue );

        /**
         * Handle Publish & Subscribe observer callback.
         */
        void HandlePubSubNotify( TUid aPubSubUid, TUint aKeyId, TInt aValue );

    private:

        TInt CountComponentControls( ) const;

        CCoeControl* ComponentControl( TInt aIndex ) const;

    private:

        /**
         * UI commands to parent using method ProcessCommandL.
         *
         * @param aCommandId Command to be handled
         */
        void ProcessCommandL( TInt aCommandId );

    private:

        void DisplayConfirmationNote( );

        // Is allowed to auto lock keys
        TBool AutoActivationAllowedL( );

        // Get auto keyguard timeout
        TInt GetAutoKeyguardTimeout( );

        // Starts monitoring user activity
        void StartActivityMonitoringL( );

        // Gets new autolock period and starts monitoring user activity
        void ResetInactivityTimeout( );

        // Stop monitoring user activity.
        void StopActivityMonitoring( );

        // Handles Active event. Called by ActivityManager
        static TInt HandleActiveEventL( TAny* aPtr );

        // Handles Inactive event. Called by ActivityManager
        static TInt HandleInactiveEventL( TAny* aPtr );

    private:

        /**
         * "Now press *" confirmation note.
         */
        CLockAppLockedNote* iConfirmationNote;

        /**
         * "Keys are locked. Press Unlock" note.
         */
        CLockAppLockedNote* iLockedNote;

        /**
         * "Keys locked" note
         */
        CLockAppLockedNote* iKeypadLockedNote;

        /**
         * "Keys activated" note
         */
        CLockAppLockedNote* iKeypadUnlockedNote;

        /**
         * CenRep observers
         */
        CLockAppCenRepObserver* iCRAutoKeyguardTime;
        CLockAppCenRepObserver* iCRPersistentKeyguardStatus;

        /**
         * PubSub observers
         */
        CLockAppPubSubObserver* iPSStartupObserver;

        /**
         * User activity manager
         */
        CUserActivityManager* iActivityManager;

        /**
         * Hardware support for keyguard
         */
        TLockHardware iHardwareSupport;

        /**
         * Flags if we had already normal state
         */
        TBool iAlreadyNormalState;

    };

#endif // __LOCKAPPKEYGUARDCONTROL_H__

// End of File
