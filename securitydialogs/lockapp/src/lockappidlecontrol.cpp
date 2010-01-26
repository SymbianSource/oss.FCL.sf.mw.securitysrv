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


#include "lockappidlecontrol.h"
#include "lockappstatecontrolinterface.h"
#include "lockapputils.h"
#include "lockapplockednote.h" // keyguard notes
#include "lockappkeycapturecontroller.h"

#include <avkon.rsg> // general avkon resources

#include <aknnotpi.rsg> // keyguard spesific resources

#include <featmgr.h> // feature manager

// from keyguard ui spesification, 6 sec.
const TInt KAknOfferKeyguardTimeout = 6000000;

// Asterisk key code for the keylock
const TUint KStdKeyAsterisk = 42;
//const TUint KAknChineseAsterisk = 0xFF0A;

// ---------------------------------------------------------------------------
// Standard Symbian OS construction sequence
// ---------------------------------------------------------------------------
CLockAppIdleControl* CLockAppIdleControl::NewL(MLockAppStateControl& aStateControl )
    {
    CLockAppIdleControl* self = new (ELeave) CLockAppIdleControl( aStateControl );
    CleanupStack::PushL( self );
    self->ConstructL( );
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Constructor passes the reference of the main state control.
// ---------------------------------------------------------------------------
CLockAppIdleControl::CLockAppIdleControl( MLockAppStateControl& aStateControl ) :
    CLockAppBaseControl( aStateControl )
    {
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
CLockAppIdleControl::~CLockAppIdleControl( )
    {
    delete iOfferLockNote;
    }

// ---------------------------------------------------------------------------
// Idle UI constructor reserves localized resources, configures itself
// using CenRep and FeatureManager and reserves child controls.
// ---------------------------------------------------------------------------
void CLockAppIdleControl::ConstructL( )
    {
    INFO( "CLockAppIdleControl::ConstructL started" );
    CLockAppBaseControl::ConstructL( );

    iOfferLockNote = new (ELeave) CLockAppLockedNote(this);
    iOfferLockNote->ConstructSleepingNoteL( R_KEYLOCK_OFFER_LOCK_NOTE );
    iOfferLockNote->ButtonGroupContainer().ButtonGroup()->AsControl()->DrawableWindow()->SetOrdinalPosition( 0, 2 );
    
    // Setup key pattern matcher
    if ( !SetupKeyPatternsWithPolicyL( EPolicyActivateKeyguard ) )
        {
        INFO( "CLockAppIdleControl::ConstructL - No CenRep policy defined" );
        iKeyPattern->AddPattern( EStdKeyDevice0, KStdKeyAsterisk ); // LSK + *
        }
#ifdef __WINS__
    iKeyPattern->AddPattern( EStdKeyDevice0, EStdKeyNkpAsterisk ); // LSK + *
#endif
                                                                                                                                                                                                           
    INFO( "CLockAppIdleControl::ConstructL completed" );
    }

// ---------------------------------------------------------------------------
// Offers keylock.
// ---------------------------------------------------------------------------
void CLockAppIdleControl::OfferKeyLock( )
    {
    ShowNote( iOfferLockNote, (CAknNoteDialog::TTimeout)KAknOfferKeyguardTimeout, CAknNoteDialog::ENoTone );
    }

// ---------------------------------------------------------------------------
// Cancels offering keylock.
// ---------------------------------------------------------------------------
void CLockAppIdleControl::CancelOfferKeyLock( )
    {
    CancelNote( );
    }

// ---------------------------------------------------------------------------
// Activate control
// ---------------------------------------------------------------------------
void CLockAppIdleControl::HandleActivateEventL( TUint aEnvMask )
    {  
		RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    CLockAppBaseControl::HandleActivateEventL( aEnvMask );
    }

// ---------------------------------------------------------------------------
// DeActivate control
// ---------------------------------------------------------------------------
void CLockAppIdleControl::HandleDeActivateEventL( TUint aEnvMask )
    {
    CLockAppBaseControl::HandleDeActivateEventL( aEnvMask );
    CancelOfferKeyLock( );
    }

// ---------------------------------------------------------------------------
// Idle UI key events are handled trough here.
// ---------------------------------------------------------------------------
TKeyResponse CLockAppIdleControl::OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode /*aType*/ )
    {
		RDebug::Printf( "%s %s (%u) aKeyEvent.iCode=%x 1=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aKeyEvent.iCode, 1 );
    TKeyResponse keyResponse = EKeyWasNotConsumed;
    if ( iActive )
        {
        if ( iOfferLockNote->IsVisible( ) )
            {
            // if keylock has been offered
            switch ( aKeyEvent.iCode )
                {
                case EKeyOK:
                    iStateControl.EnableKeyguardL( ETrue );
                    keyResponse = EKeyWasConsumed;
                    break;
                case EKeyLeftArrow:
                case EKeyRightArrow:
                case EKeyUpArrow:
                case EKeyDownArrow:
                case EKeyApplication:
                    keyResponse = EKeyWasConsumed;
                    break;
                default:
                    break;
                }
            }
        else
            {
            // keys pressed normally in idle - however keys are not captured
            // this is where locking policy could be monitored to check keyguard activation
            /*if ( aType == EEventKeyDown )
                {
                switch ( iKeyPattern->HandleKeyEvent( aKeyEvent.iScanCode ) )
                    {
                    case EPatternNoMatch:
                    case EPatternPrimaryMatch:
                        break;
                    case EPatternSecondaryMatch:
                        iStateControl.EnableKeyguardL( ETrue );
                        keyResponse = EKeyWasConsumed;
                        break;
                    default:
                        break;
                    }
                }
            */
            }
        }
    return keyResponse;
    }

// ---------------------------------------------------------------------------
// Handle UI commands received from the child controls
// ---------------------------------------------------------------------------
void CLockAppIdleControl::ProcessCommandL(TInt aCommandId )
    {
    // handle command
		RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    switch ( aCommandId )
        {
        case EAknSoftkeyLock:
            {
            // user has accepted offer keyguard enquiry - lock keys
            iStateControl.EnableKeyguardL( ETrue );
            }
            break;
        case EAknSoftkeyExit:
        case KNoteCmdFocusLost:
            {
            // user has declined offer keyguard enquiry -  cancel offer
            CancelOfferKeyLock( );
            }
            break;
        default:
            break;
        }
    }

TInt CLockAppIdleControl::CountComponentControls( ) const
    {
    return 0;
    }

CCoeControl* CLockAppIdleControl::ComponentControl(TInt /*aIndex*/) const
    {
    return NULL;
    }

// ---------------------------------------------------------------------------
// Notification if layout changes.
// ---------------------------------------------------------------------------
void CLockAppIdleControl::HandleResourceChange(TInt aType )
    {
    CCoeControl::HandleResourceChange( aType );
    }
