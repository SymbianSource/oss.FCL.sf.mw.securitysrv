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
* Description:  Sleeping note with timeout
 *
*/


#include "lockappsleepingnote.h"
#include <avkon.rsg> // avkon animation resources
#include <aknnotpi.rsg> // keyguard note resources
#include <aknnotecontrol.h> // access to note control
#include "lockapputils.h"
#include <GlobalWindowPriorities.h>

// ---------------------------------------------------------------------------
// Pointer to parent (implements MEikCommandObserver) is given in construction.
// ---------------------------------------------------------------------------
CLockAppSleepingNote::CLockAppSleepingNote( MEikCommandObserver* aCommandObserver ) :
	iCommandObserver(aCommandObserver)
	{
	}

// ---------------------------------------------------------------------------
// Note construction on startup.
// Sleeping note owns a timer that controls the note timeout.
// ---------------------------------------------------------------------------
void CLockAppSleepingNote::ConstructSleepingNoteL( TInt aResourceId )
	{
	iResourceId = aResourceId;
	CAknNoteDialog::ConstructSleepingDialogL( aResourceId );
	// Construct now rather than in PreLayoutDynInit
	}

// ---------------------------------------------------------------------------
// In destruction set reference value (is note shown) to not true.
// ---------------------------------------------------------------------------
CLockAppSleepingNote::~CLockAppSleepingNote( )
	{
	}

// ---------------------------------------------------------------------------
// Show note with timeout.
// ---------------------------------------------------------------------------
TInt CLockAppSleepingNote::ShowNote( const TInt aTimeout, const TTone aTone )
	{
	ReportUserActivity( );
	iTimeoutInMicroseconds = aTimeout;
	iTone = aTone;
	return RouseSleepingDialog( );
	}

// ---------------------------------------------------------------------------
// Key events are handled here.
// ---------------------------------------------------------------------------
TKeyResponse CLockAppSleepingNote::OfferKeyEventL( const TKeyEvent& /*aKeyEvent*/, TEventCode /*aType*/)
	{
	//return CAknNoteDialog::OfferKeyEventL( aKeyEvent, aType );
	return EKeyWasConsumed;
	}

// ---------------------------------------------------------------------------
// Always called when note exists, the reason (command) for exit
// is delivered to parent.
// ---------------------------------------------------------------------------
TBool CLockAppSleepingNote::OkToExitL( TInt aCommand )
	{
	if ( iCommandObserver )
		{
		iCommandObserver->ProcessCommandL( aCommand );
		}
	return ETrue;
	}

// ---------------------------------------------------------------------------
// Always called when note exists, the reason (command) for exit
// is delivered to parent.
// ---------------------------------------------------------------------------
void CLockAppSleepingNote::HandleResourceChange( TInt aType )
	{
	if ( aType == KAknsMessageSkinChange )
		{
		TInt animationRes(0);
		switch ( iResourceId )
			{
			case R_KEYLOCK_NOTE_KEYLOCKED:
				animationRes = R_QGN_NOTE_INFO_ANIM;
				break;
			case R_KEYLOCK_NOTE_UNLOCK_CONFIRM:
				animationRes = R_QGN_NOTE_QUERY_ANIM;
				break;
			case R_KEYLOCK_NOTE_UNLOCK_ASTERISK:
				animationRes = R_QGN_NOTE_INFO_ANIM;
				break;
			case R_KEYLOCK_NOTE_LOCK_ON:
				animationRes = R_QGN_NOTE_KEYGUARD_LOCKED_ANIM;
				break;
			case R_KEYLOCK_NOTE_LOCK_OFF:
				animationRes = R_QGN_NOTE_KEYGUARD_OPEN_ANIM;
				break;
			case R_KEYLOCK_OFFER_LOCK_NOTE:
				animationRes = R_QGN_NOTE_QUERY_ANIM;
				break;
			default:
				break;
			} // end of switch

		if ( animationRes )
			{
			CAknNoteControl* ctrl = NoteControl( );
			if ( ctrl )
				{
				TRAP_IGNORE(ctrl->SetAnimationL( animationRes ));
				}
			}
		}
	CAknNoteDialog::HandleResourceChange( aType );
	}
