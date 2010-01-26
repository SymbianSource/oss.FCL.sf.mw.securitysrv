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
* Description:  Base control for logical UI components
 *
*/


#include "lockappbasecontrol.h"
#include "lockappkeycapturecontroller.h"
#include "lockapputils.h"

#include <eikenv.h>
#include <eikbtgpc.h>
#include <aknnotedialog.h>
#include <AknSmallIndicator.h> // CAknSmallIndicator
#include <AknUtils.h>

// ---------------------------------------------------------------------------
// Constructor.
// ---------------------------------------------------------------------------
CLockAppBaseControl::CLockAppBaseControl( MLockAppStateControl& aStateControl ) :
    iStateControl(aStateControl), iWindowGroup( iEikonEnv->RootWin())
    {
	RDebug::Printf( "%s %s (%u) 1=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 1 );
    }

// ---------------------------------------------------------------------------
// Second phase constructor.
// ---------------------------------------------------------------------------
void CLockAppBaseControl::ConstructL( )
    {
	RDebug::Printf( "%s %s (%u) 1=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 1 );
    iKeyPattern = CLockAppKeyPattern::NewL( );
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
CLockAppBaseControl::~CLockAppBaseControl( )
    {
    CancelNote( );
    if ( iActive )
        {
        TRAP_IGNORE(HandleDeActivateEventL( NULL ));
        }
    delete iCba;
    delete iKeyPattern;
    }

// ---------------------------------------------------------------------------
// Setup key pattern matcher.
// ---------------------------------------------------------------------------
TBool CLockAppBaseControl::SetupKeyPatternsWithPolicyL( TLockPolicyType aType )
    {
    INFO_1( "CLockAppBaseControl::SetupKeyPatternsL( type=%d )", aType );
    TBool ret(EFalse);
    __ASSERT_DEBUG( iKeyPattern, DoPanic(ELockIllegalState));
    if ( iKeyPattern )
        {
        CKeyLockPolicyApi* keylockPolicy = CKeyLockPolicyApi::NewL( aType );
                
        // if the keylockpolicy has keycombinations
        if ( keylockPolicy->HasConfiguration( ) )
            {
            // TESTING1!
            INFO( "CLockAppBaseControl::SetupKeyPatternsWithPolicyL - HasConfiguration!" );
            
            TUint32 index(0), k1(0), k2(0);
            while ( keylockPolicy->GetKeyCombination( index, k1, k2 )== KErrNone )
                {
                // TESTING2!
                INFO( "CLockAppBaseControl::SetupKeyPatternsWithPolicyL - key combination found!" );
                
                // add each combination to our pattern matcher
                iKeyPattern->AddPattern( k1, k2 );
                index++;
                }
                        
            ret = ETrue;
            INFO_1( "CLockAppBaseControl::SetupKeyPatternsL: added %d patterns", index );
            }
        delete keylockPolicy;
        }
    return ret;
    }

// ---------------------------------------------------------------------------
// Show note
// ---------------------------------------------------------------------------
void CLockAppBaseControl::ShowNote( CLockAppLockedNote* aNote, const TInt aTimeout,
        const CAknNoteDialog::TTone aTone )
    {
    if ( aNote )
        {
        // cancel the current note if any
        if ( iCurrentNote != aNote )
            {
            CancelNote( );
            iCurrentNote = aNote;
            }
        iCurrentNote->ShowNote( aTimeout, aTone );
        }
    }

// ---------------------------------------------------------------------------
// Cancel currently shown note
// ---------------------------------------------------------------------------
void CLockAppBaseControl::CancelNote( )
    {
    if ( iCurrentNote )
        {
        if ( iCurrentNote->IsVisible( ) )
            {
            iCurrentNote->CancelNote( );
            }
        iCurrentNote = NULL;
        }
    }

// ---------------------------------------------------------------------------
// Internal lock state has been changed.
// ---------------------------------------------------------------------------
void CLockAppBaseControl::HandleLockStatusChangedL( TLockStatus aStatus )
    {
    switch ( aStatus )
        {
        case ELockNotActive:
            break;
        case EKeyguardActive:
            break;
        case EDevicelockActive:
            break;
        default:
            DoPanic( ELockPanicGeneral );
            break;
        }
    }

// ---------------------------------------------------------------------------
// Handle activate event
// ---------------------------------------------------------------------------
void CLockAppBaseControl::HandleActivateEventL( TUint /*aEnvMask*/ )
    {
    __ASSERT_DEBUG( !iActive, DoPanic(ELockIllegalState));
    if ( !iActive )
        {
        iActive = ETrue;
        }
    }

// ---------------------------------------------------------------------------
// Handle de-activate event
// ---------------------------------------------------------------------------
void CLockAppBaseControl::HandleDeActivateEventL( TUint /*aEnvMask*/ )
    {
    __ASSERT_DEBUG( iActive, DoPanic(ELockIllegalState));
    if ( iActive )
        {
        iActive = EFalse;
        }
    }

// ---------------------------------------------------------------------------
// Capture/uncapture primary keys
// ---------------------------------------------------------------------------
void CLockAppBaseControl::CapturePrimaryKeys( const TBool aCapture )
    {   
    	RDebug::Printf( "%s %s (%u) aCapture=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aCapture );
    	RDebug::Printf( "%s %s (%u) iKeyPattern=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iKeyPattern );
    if ( iKeyPattern )
        {
        if ( aCapture )
            {
            // capture primary keys
            TUint32 index( 0), primaryKey( 0), secondaryKey( 0);
            while (iKeyPattern->GetPattern( index, primaryKey, secondaryKey )== KErrNone )
                {
                CLockAppKeyCaptureController::CaptureKey( primaryKey, 0, EKeyCaptureUpAndDownEvents );
                index++;
                }
            }
        else
            {
            // uncapture primary keys
            TUint32 index( 0), primaryKey( 0), secondaryKey( 0);
            while (iKeyPattern->GetPattern( index, primaryKey, secondaryKey )== KErrNone )
                {
                CLockAppKeyCaptureController::ReleaseKey( primaryKey );
                index++;
                }
            }
        }
    }

// ---------------------------------------------------------------------------
// Bring keyguard cba forwards/backwards.
// ---------------------------------------------------------------------------
void CLockAppBaseControl::ShowCba( const TBool aShow )
    {
    if ( iCba )
        {
        RDrawableWindow* cbaWindow = iCba->ButtonGroup()->AsControl()->DrawableWindow( );
        if ( aShow )
            {
            cbaWindow->SetNonFading( ETrue );
            cbaWindow->SetOrdinalPosition( 0 );
            iCba->MakeVisible( ETrue );
            }
        else
            {
            // hide the window
            iCba->MakeVisible( EFalse );
            }
        }
    }

// ---------------------------------------------------------------------------
// Method used for capturing/releasing pointer events when key lock is enabled.
// Capturing is done using button/cba group window.
// ---------------------------------------------------------------------------
void CLockAppBaseControl::SetPointerEventCapture(const TBool aEnable )
    {
#ifdef RD_SCALABLE_UI_V2
    if ( AknLayoutUtils::PenEnabled( )&& iCba )
        {
        // cba captures all pointer events
        RDrawableWindow* cbaWindow = iCba->ButtonGroup()->AsControl()->DrawableWindow( );
        if ( aEnable )
            {
            cbaWindow->SetPointerCapture( RWindowBase::TCaptureDragDrop );
            }
        else
            {
            cbaWindow->SetPointerCapture( RWindowBase::TCaptureDisabled );
            }
        cbaWindow->ClaimPointerGrab( aEnable );
        }
#endif // RD_SCALABLE_UI_V2
    }

// ---------------------------------------------------------------------------
// Set Keyguard indicator on Idle.
// ---------------------------------------------------------------------------
void CLockAppBaseControl::SetKeyguardIndicatorStateL( const TBool aEnable )
    {
    CAknSmallIndicator* theIndicator = CAknSmallIndicator::NewLC( TUid::Uid( EAknIndicatorKeyguard ) );
    if ( aEnable )
        {
        theIndicator->SetIndicatorStateL( EAknIndicatorStateOn );
        }
    else
        {
        theIndicator->SetIndicatorStateL( EAknIndicatorStateOff );
        }
    CleanupStack::PopAndDestroy( theIndicator );
    }

// ---------------------------------------------------------------------------
// Handle environment changes (Screensaver, Telephony, etc.)
// ---------------------------------------------------------------------------
void CLockAppBaseControl::HandleEnvironmentChange( TUint /*aEnvMask*/, TUint /*aEventMask*/ )
    {
    // no implementation needed - virtual function
    }

// END OF FILE
