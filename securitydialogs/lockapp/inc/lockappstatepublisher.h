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
* Description:  Publishes LockApp states to other applications
 *
*/


#ifndef __LOCKAPPSTATEPUBLISHER_H__
#define __LOCKAPPSTATEPUBLISHER_H__

// INCLUDES
#include "lockappstateobserver.h"
#include <e32property.h>

/**
 *  CLockAppStatePublisher class publishes lock state to external parties using P&S key.
 * 
 *  @lib    lockapp
 *  @since  5.0
 *  @author Joona Petrell
 *  @author Tamas Koteles
 **/
class CLockAppStatePublisher : public CBase, public MLockAppStateObserver
	{
	public:
		/**
		 * Two-phased constructor.
		 */
		static CLockAppStatePublisher* NewL( );

		/**
		 * C++ default constructor.
		 */
		CLockAppStatePublisher( );

		/**
		 * Destructor.
		 */
		~CLockAppStatePublisher( );
	private:

		/**
		 * Second constructor that can fail (leave).
		 */
		void ConstructL( );

	public:
		// from CLockAppStateObserver

		virtual void HandleLockStatusChangedL( TLockStatus aLockStatus );

	private:

		// stores locking state property
		RProperty iStatusProperty;

	};

#endif // __LOCKAPPSTATEPUBLISHER_H__
// End of File

