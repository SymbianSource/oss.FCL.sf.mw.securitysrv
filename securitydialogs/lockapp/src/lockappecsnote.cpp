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


#include "lockappecsnote.h"
#include "lockapputils.h"
#include <AknUtils.h>
#include <aknlayoutscalable_avkon.cdl.h>
#include <aknappui.h>
#include <AknEcs.h> // for KAknEcsMaxMatchingLength

#include <aknglobalpopupprioritycontroller.h>
#include <GlobalWindowPriorities.h>

// ---------------------------------------------------------------------------
// Standard C++ constructor
// ---------------------------------------------------------------------------
CLockAppEcsNote::CLockAppEcsNote( ) :
	iNoteOnScreen(EFalse)
	{
	// no implementation required
	}

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
CLockAppEcsNote::~CLockAppEcsNote( )
    {
#ifdef _GLOBAL_PRIORITY_SUPPORTED
    AknGlobalPopupPriorityController::RemovePopupPriority(*this);
#endif
    }

// ---------------------------------------------------------------------------
// Emergency note is a sleeping note
// ---------------------------------------------------------------------------
void CLockAppEcsNote::ConstructSleepingNoteL(TInt aResourceId )
    {
    CAknNoteDialog::ConstructSleepingDialogL( aResourceId );
#ifdef _GLOBAL_PRIORITY_SUPPORTED
    // global ui component order is handled trhoug global popup priority controller
    AknGlobalPopupPriorityController::SetPopupPriorityL( *this, KGlobalWindowPriority_KeyLock );
#endif
    }

// ---------------------------------------------------------------------------
// Show note with time out.
// ---------------------------------------------------------------------------
TInt CLockAppEcsNote::ShowNote( )
    {
    ReportUserActivity( );
    iTimeoutInMicroseconds = CAknNoteDialog::EUndefinedTimeout;
    iTone = CAknNoteDialog::ENoTone;
    TInt error = KErrNone;
    if ( !iNoteOnScreen )
        {
        error = RouseSleepingDialog( );
        }
    iNoteOnScreen = ETrue;
    // return value not used
    return error;
    }

// ---------------------------------------------------------------------------
// Hide note from screen.
// ---------------------------------------------------------------------------
void CLockAppEcsNote::SleepNote( )
    {
    if ( iNoteOnScreen )
        {
        ExitSleepingDialog( ); // Causes flicker to other notes if called when note is not on screen
        }
    iNoteOnScreen = EFalse;
    }

// ---------------------------------------------------------------------------
// Emergency dialog consumes all key events it receives.
// ---------------------------------------------------------------------------
TKeyResponse CLockAppEcsNote::OfferKeyEventL(const TKeyEvent& /*aKeyEvent*/, TEventCode /*aType*/)
    {
    return EKeyWasConsumed;
    }

// ---------------------------------------------------------------------------
// Format the emergency number for the dialog text.
// ---------------------------------------------------------------------------
void CLockAppEcsNote::SetEmergencyNumber( const TDesC& aMatchedNumber )
    {
    TRect mainPaneRect;
    AknLayoutUtils::LayoutMetricsRect( AknLayoutUtils::EMainPane, mainPaneRect );
    TAknLayoutRect popupNoteWindow;
    AknLayoutUtils::TAknCbaLocation cbaLocation( AknLayoutUtils::CbaLocation( ));
    TInt variety( 0);
    if ( cbaLocation == AknLayoutUtils::EAknCbaLocationRight )
        {
        variety = 5;
        }
    else
        if ( cbaLocation == AknLayoutUtils::EAknCbaLocationLeft )
            {
            variety = 8;
            }
        else
            {
            variety = 2;
            }

    popupNoteWindow.LayoutRect( mainPaneRect, AknLayoutScalable_Avkon::popup_note_window( variety ) );
    TAknLayoutText textRect;
    textRect.LayoutText( popupNoteWindow.Rect( ), AknLayoutScalable_Avkon::popup_note_window_t5(2).LayoutLine( ) );

    // Size of a temporary buffer that contains new lines, spaces and
    // emergency number for a note.
    TBuf16<KAknEcsMaxMatchingLength+80> number;
    number.Append( '\n' );
    number.Append( '\n' );

    TInt spaceCharWidthInPixels = textRect.Font()->CharWidthInPixels( ' ' );
    if ( spaceCharWidthInPixels < 1 )
        {
        // Avoid divide by zero situation even the space char would have zero length.
        spaceCharWidthInPixels = 1;
        }

    TInt length = (textRect.TextRect().Width() - textRect.Font()->TextWidthInPixels(aMatchedNumber))/ spaceCharWidthInPixels;

    const TInt matchedNumberLength = aMatchedNumber.Length( );
    const TInt numberLength = number.Length( );
    const TInt numberMaxLength = number.MaxLength( );

    if ( numberLength + length + matchedNumberLength > numberMaxLength )
        {
        // To make sure that buffer overflow does not happen.
        length = numberMaxLength - numberLength - matchedNumberLength;
        }
    for (int i = 0; i < length; i++ )
        {
        number.Append( ' ' );
        }

    number.Append( aMatchedNumber );
    TRAP_IGNORE(SetTextL(number));
    }
