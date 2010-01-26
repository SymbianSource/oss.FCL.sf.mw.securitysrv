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


#include <aknnotedialog.h>

#ifndef __LOCKAPPSLEEPINGNOTE_H__
#define __LOCKAPPSLEEPINGNOTE_H__

/**
 *  CLockAppSleepingNote class implement notes that are not destroyed between use. 
 *  Note is shown with timeout and internal changes are reported to parent 
 *  trough command observer interface.
 *
 *  @lib    lockapp
 *  @since  5.0
 *  @author Joona Petrell
 *  @author Tamas Koteles
 */
class CLockAppSleepingNote : public CAknNoteDialog
	{
	public:

		/**
		 * C++ default constructor.
		 *
		 * @param aCommandObserver pointer to parent
		 *                         implementing observer interface
		 */
		CLockAppSleepingNote( MEikCommandObserver* aCommandObserver = NULL );

		/**
		 * Sleeping note is constructed from a resource
		 */
		void ConstructSleepingNoteL( TInt aResourceId );

		/**
		 * Destructor.
		 */
		~CLockAppSleepingNote( );

		/**
		 * Show sleeping note with given timeout.
		 * @param aTimeout how long note will be displayed.
		 * @param aTone tone to be played.
		 */
		TInt ShowNote( const TInt aTimeout, const TTone aTone );

		/**
		 * Always called when note is dismissed
		 *
		 * @param aCommand reason for exit.
		 */
		TBool OkToExitL( TInt aCommand );

		/**
		 * Handles key events (from CoeControl)
		 */
		TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType );

	protected:

		void HandleResourceChange( TInt aType );

		// pointer to keyguard UI
		MEikCommandObserver* iCommandObserver;

		// resource id is reserved for animation skin change
		TInt iResourceId;

	};

#endif // __LOCKAPPSLEEPINGNOTE_H__
