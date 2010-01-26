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
* Description:  Sleeping note with timeout and a reference flag that informs
 *                the parent when the note is visible
 *
*/


#include "lockapplockednote.h"

// ---------------------------------------------------------------------------
// Construction passes the parent implementing command observer interface
// and a reference to variable that is true if note with timeout is shown,
// false if not
// ---------------------------------------------------------------------------
CLockAppLockedNote::CLockAppLockedNote( MEikCommandObserver* aCommandObserver ) :
	CLockAppSleepingNote(aCommandObserver)
	{
	// no implementation required
	}

// ---------------------------------------------------------------------------
// In destruction set reference value (is note shown) to not true.
// ---------------------------------------------------------------------------
CLockAppLockedNote::~CLockAppLockedNote( )
	{
	}

// ---------------------------------------------------------------------------
// Method to cancel note from screen.
// ---------------------------------------------------------------------------
void CLockAppLockedNote::CancelNote( )
	{
	ExitSleepingDialog( );
	}

// ---------------------------------------------------------------------------
// Inform the command observer (parent) about the focus lost event.
// ---------------------------------------------------------------------------
void CLockAppLockedNote::FocusChanged( TDrawNow /*aDrawNow*/)
	{
	if ( !IsFocused( ) && iCommandObserver )
		{
		TRAP_IGNORE(iCommandObserver->ProcessCommandL(KNoteCmdFocusLost))
		}
	}
