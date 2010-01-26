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
* Description:  Emergency number dialog
 *
*/


#ifndef __LOCKAPPECSNOTE_H__
#define __LOCKAPPECSNOTE_H__

// INCLUDES
#include <aknnotedialog.h>

/**
 *  CLockAppEcsNote class is derived from Avkon note implementation;
 *  shows emergency numbers on a dialog once the user has dialed
 *  the numbers.
 *
 *  @lib    lockapp
 *  @since  5.0
 *  @author Joona Petrell
 *  @author Tamas Koteles
 */
class CLockAppEcsNote : public CAknNoteDialog
    {
    public:

        /**
         * C++ constructor.
         */
        CLockAppEcsNote( );

        /**
         * Destructor.
         */
        ~CLockAppEcsNote( );

    public:

        /**
         * Constructs sleeping note
         */
        void ConstructSleepingNoteL( TInt aResourceId );

        /**
         * Shows sleeping note
         */
        TInt ShowNote( );

        /**
         * Hides sleeping note
         */
        void SleepNote( );

        /**
         * Consume key events
         */
        TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType );

        /**
         * API to set the emergency number to be displayed
         *
         * aMatchedNumber    text to display (e.g. "112" )
         */
        void SetEmergencyNumber( const TDesC& aMatchedNumber );

    public:

        TBool iNoteOnScreen;

    };

#endif // __LOCKAPPECSNOTE_H__
// End of File
