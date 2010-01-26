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
* Description:  Interface for CenRep and PubSub observers
 *
*/


#ifndef __LOCKAPPOBSERVERINTERFACE_H__
#define __LOCKAPPOBSERVERINTERFACE_H__

// INCLUDES
#include "lockapp.hrh"

/**
 *  MLockAppObserverInterface class offers a CenRep/PubSub observer interface.
 *  Observer classes should implement this interface to get callback notifications
 *  on value changes.
 *
 *  @lib    lockapp
 *  @since  5.0
 *  @author Tamas Koteles
 */
class MLockAppObserverInterface
	{
	public:

		/** 
		 * Handle Central Repository observer callback.
		 *
		 * @param aCenRepUid  Central Repository category id
		 * @param aKeyId      Central Repository key id
		 * @param aValue      Central Repository key's new value
		 */
		virtual void HandleCenRepNotify( TUid aCenRepUid, TUint32 aKeyId, TInt aValue ) = 0;

		/** 
		 * Handle Publish & Subscribe observer callback.
		 *
		 * @param aPubSubUid  Publish & Subscribe category id
		 * @param aKeyId      Publish & Subscribe key id
		 * @param aValue      Publish & Subscribe key's new value
		 */
		virtual void HandlePubSubNotify( TUid aPubSubUid, TUint aKeyId, TInt aValue ) = 0;

	};

#endif // __LOCKAPPOBSERVERINTERFACE_H__
// End of File
