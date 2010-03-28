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
* Description:  Devicelock Background UI (window owning control)
 *
*/


#ifndef LOCKAPPDEVICELOCKCONTAINER_H
#define LOCKAPPDEVICELOCKCONTAINER_H

// INCLUDES
#include <coecntrl.h>
#include <eikimage.h>
#include <eiklabel.h>
#include <AknSkinnableClock.h>

// FORWARD DECLARATIONS
class CAknsLayeredBackgroundControlContext;

/**
 *  CLockAppDevicelockContainer class contains UI components 
 *  for the devicelock control.
 *
 *  @lib    lockapp
 *  @since  5.0
 *  @author Joona Petrell
 *  @author Tamas Koteles
 *  @see    CLockAppDevicelockControl
 */
class CLockAppDevicelockContainer : public CCoeControl, MCoeControlObserver
	{
	public:

		/**
		 * Two-phased constructor.
		 */
		static CLockAppDevicelockContainer* NewL( RWindowGroup& aWg );

		/**
		 * Destructor.
		 */
		~CLockAppDevicelockContainer( );

	public:

		/**
		 * From CCoeControl, MopSupplyObject
		 */
		TTypeUid::Ptr MopSupplyObject( TTypeUid aId );

	private:

		/**
		 * 2nd stage construction
		 */
		void ConstructL( RWindowGroup& aWg );

		TRect GetMainPaneRect( );

	private:

		void SizeChanged( );

		TInt CountComponentControls( ) const;

		CCoeControl* ComponentControl( TInt aIndex ) const;

		void Draw( const TRect& aRect ) const;

		void HandleControlEventL( CCoeControl* aControl, TCoeEvent aEventType );

	private:

		CEikImage* iEikBitmap;
		CFbsBitmap* iBitmap;
		CFbsBitmap* iMask;

		// Owned background context.
		CAknsLayeredBackgroundControlContext* iBgContext;

		// Clock for landscape.
		CAknSkinnableClock* iClock;
	};

#endif

// End of File
