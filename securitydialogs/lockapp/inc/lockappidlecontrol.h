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
* Description:  Idle (unlocked) UI
 *
*/


#ifndef LOCKAPPIDLECONTROL_H
#define LOCKAPPIDLECONTROL_H

// INCLUDES
#include <eiknotapi.h>
#include <eikbtgpc.h>
#include <AknNotifyStd.h>
#include <aknnotedialog.h>
#include <AknNotifierControllerPlugin.h>
#include <AknQueryDialog.h>   // phone query
#include "lockappbasecontrol.h"

/**
 *  CLockAppIdleControl class represents the unlocked state in the locking state machine.
 * 
 *  @lib    lockapp
 *  @since  5.0
 *  @author Joona Petrell
 *  @author Tamas Koteles
 *  @see    CLockAppBaseControl
 *  @see    MLockAppStateControl
 */
class CLockAppIdleControl : public CLockAppBaseControl, public MEikCommandObserver
    {
    public:

        /**
         * Two-phased constructor.
         */
        static CLockAppIdleControl* NewL( MLockAppStateControl& aStateControl );

        /**
         * Destructor.
         */
        ~CLockAppIdleControl( );

    private:

        /**
         * Constructor for performing 1st stage construction
         */
        CLockAppIdleControl( MLockAppStateControl& aStateControl );

        /**
         * EPOC default constructor for performing 2nd stage construction
         */
        void ConstructL( );

    public:

        void OfferKeyLock( );

        void CancelOfferKeyLock( );

        void HandleActivateEventL( TUint aEnvMask );

        void HandleDeActivateEventL( TUint aEnvMask );

    public:

        void HandleResourceChange( TInt aType );

        TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType );

    private:

        TInt CountComponentControls( ) const;

        CCoeControl* ComponentControl( TInt aIndex ) const;

    private:
        /**
         * From @c MEikCommandObserver. Dialogs and CBA send
         * UI commands to parent using method ProcessCommandL.
         *
         * @param aCommandId Command to be handled
         */
        void ProcessCommandL( TInt aCommandId );

    private:

        // "offer to lock keys" note
        CLockAppLockedNote* iOfferLockNote;

    };

#endif // LOCKAPPIDLECONTROL_H
