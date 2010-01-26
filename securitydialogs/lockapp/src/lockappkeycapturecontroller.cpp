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
* Description:  Key capture utility
 *
*/


// INCLUDE FILES
#include "lockappkeycapturecontroller.h"
#include <w32std.h>

// ================= STATIC MEMBERS =========================

CLockAppKeyCaptureController* CLockAppKeyCaptureController::instance = NULL;


// ================= STATIC FUNCTIONS =======================

// ---------------------------------------------------------
// Initializes the key capture utility by creating an instance of it
// ---------------------------------------------------------
CLockAppKeyCaptureController* CLockAppKeyCaptureController::InitL( RWindowGroup& aWindowGroup )
    {
    if ( !instance )
        {
        instance = new ( ELeave ) CLockAppKeyCaptureController( aWindowGroup );
        }
    return instance;
    }

// ---------------------------------------------------------
// Destroys the key capture utility's instance
// ---------------------------------------------------------
void CLockAppKeyCaptureController::Destroy( )
    {
    if ( instance )
        {
        delete instance;
        instance = NULL;
        }
    }

// ---------------------------------------------------------
// Capture a key
// ---------------------------------------------------------
void CLockAppKeyCaptureController::CaptureKey( TUint32 aKey, TUint32 aKeyCode, TPhoneKeyCaptureType aType )
    {
    	RDebug::Printf( "%s %s (%u) aKey=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aKey );
    	RDebug::Printf( "%s %s (%u) aKeyCode=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aKeyCode );
    	RDebug::Printf( "%s %s (%u) aType=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aType );

    if ( instance )
        {
        instance->StartCapturingKey( aKey, aKeyCode, aType );
        }
    }

// ---------------------------------------------------------
// Release a key (previously captured)
// ---------------------------------------------------------
void CLockAppKeyCaptureController::ReleaseKey( TUint32 aKey )
    {
    	RDebug::Printf( "%s %s (%u) aKey=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aKey );

    if ( instance )
        {
        instance->StopCapturingKey( aKey );
        }
    }

// ================= INSTANCE FUNCTIONS ============================

// ---------------------------------------------------------
// Private default constructor
// ---------------------------------------------------------
CLockAppKeyCaptureController::CLockAppKeyCaptureController( RWindowGroup& aWindowGroup ) :
    iWindowGroup( aWindowGroup)
    {
    }

// ---------------------------------------------------------
// Private destructor
// ---------------------------------------------------------
CLockAppKeyCaptureController::~CLockAppKeyCaptureController( )
    {
    	RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );

    // should we have any captured keys, stop capturing now
    TInt capturedKeyCount = iCapturedKeys.Count( );
    for (TInt i = 0; i < capturedKeyCount; i++ )
        {
        StopKeyCapture( iCapturedKeys[i] );
        }
    iCapturedKeys.Close( );
    }

// ---------------------------------------------------------
// Starts capturing a key
// ---------------------------------------------------------
void CLockAppKeyCaptureController::StartCapturingKey( TUint32 aKey, TUint32 aKeyCode, TPhoneKeyCaptureType aType )
    {
    TInt32 handle = KErrNotFound;
		RDebug::Printf( "%s %s (%u) aKey=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aKey );
		RDebug::Printf( "%s %s (%u) aKeyCode=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aKeyCode );
		RDebug::Printf( "%s %s (%u) aType=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aType );

    if ( !IsKeyCaptured( aKey ) )
        {
        TPhoneKeyCapture keyCapture;
        keyCapture.iKey = (TStdScanCode) aKey;
        keyCapture.iKeyCode = (TKeyCode) aKeyCode;
        keyCapture.iHandle = 0; // set as initial value
        keyCapture.iHandleForUpAndDown = 0; // set as initial value

        switch( aType )
            {
            case EKeyCaptureEvent:
                keyCapture.iType = EKeyCaptureEvent;
                keyCapture.iHandle = iWindowGroup.CaptureKey( keyCapture.iKeyCode, 0, 0 );
                handle = keyCapture.iHandle;
                break;
            case EKeyCaptureUpAndDownEvents:
                keyCapture.iType = EKeyCaptureUpAndDownEvents;
                keyCapture.iHandleForUpAndDown = iWindowGroup.CaptureKeyUpAndDowns( keyCapture.iKey, 0, 0 );
                handle = keyCapture.iHandleForUpAndDown;
                break;
            default: // EKeyCaptureAllEvents
                {
                keyCapture.iType = EKeyCaptureAllEvents;
                keyCapture.iHandle = iWindowGroup.CaptureKey( keyCapture.iKeyCode, 0, 0 );
                if ( keyCapture.iHandle >= 0 )
                    {
                    keyCapture.iHandleForUpAndDown = iWindowGroup.CaptureKeyUpAndDowns( keyCapture.iKey, 0, 0 );
                    if ( keyCapture.iHandleForUpAndDown < 0 )
                        {
                        iWindowGroup.CancelCaptureKey( keyCapture.iHandle );
                        }
                    handle = keyCapture.iHandleForUpAndDown;
                    }
                break;
                }
            }

        RDebug::Printf( "CLockAppKeyCaptureController::StartCapturingKey - handle: 0x%08x", handle );

        if ( handle >= 0 )
            {
            if ( iCapturedKeys.Append( keyCapture )!= KErrNone )
                {
                StopKeyCapture( keyCapture );
                }
            }
        }
    }

// ---------------------------------------------------------
// Stops capturing a key
// ---------------------------------------------------------
void CLockAppKeyCaptureController::StopCapturingKey( TUint32 aKey )
    {
		RDebug::Printf( "%s %s (%u) aKey=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aKey );
    const TInt capturedKeyCount = iCapturedKeys.Count( );
    TBool foundKey = EFalse;
    for (TInt i = 0; ( i < capturedKeyCount ) && !foundKey; i++ )
        {
        if ( iCapturedKeys[i].iKey == aKey )
            {
            foundKey = ETrue;
            StopKeyCapture( iCapturedKeys[i] );
            iCapturedKeys.Remove( i );
            }
        }
    }

// ---------------------------------------------------------
// May be used to ask whether a key has been set to be captured
// ---------------------------------------------------------
TBool CLockAppKeyCaptureController::IsKeyCaptured( TUint32 aKey ) const
    {
		RDebug::Printf( "%s %s (%u) aKey=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aKey );
    const TInt capturedKeyCount = iCapturedKeys.Count( );
    TBool isCaptured = EFalse;
    for (TInt i = 0; ( i < capturedKeyCount ) && !isCaptured; i++ )
        {
        isCaptured = iCapturedKeys[i].iKey == aKey;
        }
    return isCaptured;
    }

// ---------------------------------------------------------
// Stops capturing a key
// ---------------------------------------------------------
void CLockAppKeyCaptureController::StopKeyCapture( TPhoneKeyCapture aKeyCapture )
    {
		RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    switch ( aKeyCapture.iType )
        {
        case EKeyCaptureEvent:
            iWindowGroup.CancelCaptureKey( aKeyCapture.iHandle );
            break;
        case EKeyCaptureUpAndDownEvents:
            iWindowGroup.CancelCaptureKeyUpAndDowns( aKeyCapture.iHandleForUpAndDown );
            break;
        default: // EPhoneKeyCaptureAllEvents
            {
            iWindowGroup.CancelCaptureKey( aKeyCapture.iHandle );
            iWindowGroup.CancelCaptureKeyUpAndDowns( aKeyCapture.iHandleForUpAndDown );
            break;
            }
        }
    }

// end of file
