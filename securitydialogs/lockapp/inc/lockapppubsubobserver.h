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
* Description:  Publish & Subscribe key observer
 *
*/


#ifndef __LOCKAPPPUBSUBOBSERVER_H__
#define __LOCKAPPPUBSUBOBSERVER_H__

// INCLUDES
#include <w32std.h>
#include <e32svr.h>
#include <e32property.h>
#include "lockappobserverinterface.h"

/**
 *  CLockAppPubSubObserver class implements a Publish & Subscribe key observer.
 *  It can be used to monitor and also to set a PubSub key.
 *
 *  @lib    lockapp
 *  @since  5.0
 *  @author Tamas Koteles
 */
class CLockAppPubSubObserver : public CActive
	{
	public:

		/**
		 * Two-phased constructor.
		 *
		 * @param aObserver   pointer to observer
		 * @param aPubSubUid  publish & subscribe Uid
		 * @param aKeyId      publish & subscribe key id
		 * @return            the instance just created.
		 */
		static CLockAppPubSubObserver* NewL( MLockAppObserverInterface* aObserver, TUid aPubSubUid,
				TUint32 aKeyId );

		/**
		 * Destructor.
		 */
		~CLockAppPubSubObserver( );

		/**
		 * Start obsering PubSub key
		 */
		TInt Start( );

		/**
		 * Stop observing PubSub key
		 */
		void Stop( );

		/**
		 * Get key value from P&S.
		 */
		TInt GetKeyValue( TInt& aValue );

		/**
		 * Set key value to P&S.
		 */
		TInt SetKeyValue( TInt aValue );

	protected:

		/**
		 * C++ default constructor.
		 *
		 * @param aObserver observer
		 * @param aPubSubUid publish & subscribe Uid
		 * @param aKeyId key id
		 */
		CLockAppPubSubObserver( MLockAppObserverInterface* aObserver, TUid aPubSubUid,
				TUint32 aKeyId );

		/**
		 * Symbian OS constructor.
		 */
		void ConstructL( );

	private:

		void RunL( );

		void DoCancel( );

	protected:

		/**
		 * Observer's callback interface. Not owned.
		 */
		MLockAppObserverInterface* iObserver;

		/**
		 * Access to Publish & Subscribe.
		 */
		RProperty iProperty;

		/**
		 * Publish & Subscribe Uid.
		 */
		TUid iPubSubUid;

		/**
		 * Publish & Subscribe key Id.
		 */
		TUint32 iKeyId;

		/**
		 * Publish & Subscribe key's value.
		 */
		TInt iValue;

	};

#endif // __LOCKAPPPUBSUBOBSERVER_H__
// END OF FILE
