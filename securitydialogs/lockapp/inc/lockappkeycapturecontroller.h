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
* Description:  Key capture controller
 *
*/


#ifndef __LOCKAPPKEYCAPTURECONTROLLER_H__
#define __LOCKAPPKEYCAPTURECONTROLLER_H__

//  INCLUDES
#include <e32base.h>
#include <e32keys.h>

// enumerated type for captured key events
enum TPhoneKeyCaptureType
    {
    EKeyCaptureEvent,
    EKeyCaptureUpAndDownEvents,
    EKeyCaptureAllEvents
    };

// Key capture data structure
class TPhoneKeyCapture
    {
    public:
        TStdScanCode iKey; // primary identifier
        TKeyCode iKeyCode;
        TPhoneKeyCaptureType iType;
        TInt32 iHandle;
        TInt32 iHandleForUpAndDown;
    };

// FORWARD DECLARATIONS
class RWindowGroup;

/**
 *  CLockAppKeyCaptureController class is a simple key capture utily that
 *  provides a way to capture/release keys for the current window group.
 *
 *  @lib    lockapp
 *  @since  5.0
 *  @author Joona Petrell
 *  @author Tamas Koteles
 */
class CLockAppKeyCaptureController : public CBase
    {
    public:

        /**
         * Initialize the utility
         */
        static CLockAppKeyCaptureController* InitL( RWindowGroup& aWindowGroup );

        /**
         * Destroy the utility
         */
        static void Destroy( );

        /**
         * Set key to be captured
         */
        static void CaptureKey( TUint32 aKey, TUint32 aKeyCode, TPhoneKeyCaptureType aType );

        /**
         * Set key to be released
         */
        static void ReleaseKey( TUint32 aKey );

    private:

        /**
         * C++ default constructor.
         */
        CLockAppKeyCaptureController( RWindowGroup& aWindowGroup );

        /**
         * Destructor.
         */
        virtual ~CLockAppKeyCaptureController( );

    private:

        /**
         * Set key to be captured
         */
        void StartCapturingKey( TUint32 aKey, TUint32 aKeyCode, TPhoneKeyCaptureType aType );

        /**
         * Set key not to be captured
         */
        void StopCapturingKey( TUint32 aKey );

        /**
         * May be used to ask whether a key has been set to be captured or not
         * @param aKey is the iScanCode of the key
         * @return ETrue if the key is currently captured via this mechanism
         */
        TBool IsKeyCaptured( TUint32 aKey ) const;

        /**
         * Set key not to be captured
         * @param aKeyCapture is the key not to be captured
         */
        void StopKeyCapture( TPhoneKeyCapture aKeyCapture );

    private:

        /**
         * Private instance of the utility
         */
        static CLockAppKeyCaptureController* instance;

        /**
         * Array of keycodes currently captured which includes the window
         * server handles for the captured keys.
         */
        RArray<TPhoneKeyCapture> iCapturedKeys;

        /**
         * application's window group
         */
        RWindowGroup& iWindowGroup;

    };

#endif  // LOCKAPPKEYCAPTURECONTROLLER_H

// End of File
