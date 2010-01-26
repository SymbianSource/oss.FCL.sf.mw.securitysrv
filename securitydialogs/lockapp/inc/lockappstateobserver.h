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
* Description:  Interface for controls that want to be informed about lock status
 *
*/


#ifndef __LOCKAPPSTATEOBSERVER_H__
#define __LOCKAPPSTATEOBSERVER_H__

// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include "lockapp.hrh"

/**
 *  CLockAppStateObserver class offers a lockstate observer interface. 
 *  All observer classes derived from MLockAppStateObserver should be added to 
 *  the lockapp observer list in order to get notifications about lock state changes.
 *
 *  @lib    lockapp
 *  @since  5.0
 *  @author Joona Petrell
 *  @author Tamas Koteles
 */
class MLockAppStateObserver
	{
	public:

		/**
		 * Lock status changes are handled trough HandleLockStatusChangedL method.
		 * Must be overriden by derived class for observing.
		 */
		virtual void HandleLockStatusChangedL( TLockStatus aLockStatus ) = 0;

	};

#endif // __LOCKAPPSTATEOBSERVER_H__
// End of File
