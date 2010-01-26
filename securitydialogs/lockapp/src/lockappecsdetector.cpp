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


#include "lockappecsdetector.h"
#include "lockappecsnote.h"
#include "lockapputils.h"
#include <aknappui.h>
#include <avkon.rsg>

// ---------------------------------------------------------------------------
// Standard Symbian OS construction sequence
// ---------------------------------------------------------------------------
CLockAppEcsDetector* CLockAppEcsDetector::NewL( )
    {
    CLockAppEcsDetector* self = new (ELeave) CLockAppEcsDetector();
    CleanupStack::PushL( self );
    self->ConstructL( );
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Standard C++ constructor
// ---------------------------------------------------------------------------
CLockAppEcsDetector::CLockAppEcsDetector( )
    {
    }

// ---------------------------------------------------------------------------
// Constructs the emergency detector and the note.
// ---------------------------------------------------------------------------
void CLockAppEcsDetector::ConstructL( )
    {
    // emergency call support
    iEcsDetector = CAknEcsDetector::NewL( );
    iEcsDetector->SetObserver( this );

    // emergency call note
    iEcsNote = new (ELeave) CLockAppEcsNote();
    iEcsNote->ConstructSleepingNoteL( R_AVKON_EMERGENCY_CALL_NOTE );
    iEcsNote->ButtonGroupContainer().ButtonGroup()->AsControl()->DrawableWindow()->SetOrdinalPosition( 0, 2 );

    // needs to be called for ecs detector to work/receive events.
    iAvkonAppUi->EventMonitor()->Enable( ETrue );
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
CLockAppEcsDetector::~CLockAppEcsDetector( )
    {
    delete iEcsDetector;
    delete iEcsNote;
    }

// ---------------------------------------------------------------------------
// Detector starts/stops listening to key events if keys are locked/unlocked.
// The detector key event queue is always reseted when lock status is changed.
// ---------------------------------------------------------------------------
void CLockAppEcsDetector::HandleLockStatusChangedL( TLockStatus aLockStatus )
    {
    iEcsDetector->Reset( );
    switch ( aLockStatus )
        {
        case ELockNotActive:
            {
            iEcsDetector->CloseEventSource( );
            }
            break;
        case EKeyguardActive:
        case EDevicelockActive:
            {
            iEcsDetector->ConnectToEventSource( );
            }
            break;
        default:
            DoPanic( ELockUnknownValue );
            break;
        }
    }

// ---------------------------------------------------------------------------
// The main state controller needs to know if emergency call note is
// on the screen.
// ---------------------------------------------------------------------------
TBool CLockAppEcsDetector::EcsNoteOnScreen( ) const
    {
    return iEcsNote->iNoteOnScreen;
    }

// ---------------------------------------------------------------------------
// From @c MAknEcsObserver.. Handles changes in emergency the emergency number
// queue (i.e. if the user has dialed emergency numbers or not).
// ---------------------------------------------------------------------------
void CLockAppEcsDetector::HandleEcsEvent( CAknEcsDetector* aEcsDetector,
        CAknEcsDetector::TState aState )
    {
    switch ( aState )
        {
        case CAknEcsDetector::ECompleteMatchThenSendKey:
            // Do nothing since note will be removed on ECallAttempted event
            break;
            // user has successfully dialed emergency numbers
        case CAknEcsDetector::ECompleteMatch:
            iEcsNote->SetEmergencyNumber( aEcsDetector->CurrentMatch( ) );
            // Tell sysAp to switch lights on
            SendMessageToSysAp( EEikEcsQueryLights );
            iEcsNote->ShowNote( );
            break;
        case CAknEcsDetector::EPartialMatch:
            iEcsNote->SleepNote( );
            break;
        case CAknEcsDetector::ECallAttempted:
            iEcsNote->SleepNote( );
            break;
        case CAknEcsDetector::EEmpty:
            iEcsNote->SleepNote( );
            break;
        case CAknEcsDetector::ENoMatch:
            iEcsNote->SleepNote( );
            break;
        default:
            break;
        }
    }

// ---------------------------------------------------------------------------
// Tests the emergency note ui. Only used for testing purposes,
// Created, because emergency detector does not work in emulator.
// ---------------------------------------------------------------------------
TInt CLockAppEcsDetector::TestEcsNote( )
    {
#ifdef _DEBUG
    HandleEcsEvent( iEcsDetector, CAknEcsDetector::EEmpty );
    HandleEcsEvent( iEcsDetector, CAknEcsDetector::ENoMatch );
    HandleEcsEvent( iEcsDetector, CAknEcsDetector::EPartialMatch );
    HandleEcsEvent( iEcsDetector, CAknEcsDetector::ECompleteMatch );
    _LIT( NText, "112Test");
    iEcsNote->SetEmergencyNumber( NText );
    SendMessageToSysAp( EEikEcsQueryLights );
    iEcsNote->ShowNote( );
#endif
    return KErrNone;
    }
