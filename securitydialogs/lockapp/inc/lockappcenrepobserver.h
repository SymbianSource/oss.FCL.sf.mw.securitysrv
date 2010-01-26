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
* Description:  Central Repository key observer
 *
*/


#ifndef __LOCKAPPCENREPOBSERVER_H__
#define __LOCKAPPCENREPOBSERVER_H__

// INCLUDES
#include <e32std.h>
#include <cenrepnotifyhandler.h>
#include "lockappobserverinterface.h"

/**
 *  CLockAppCenRepObserver class is a Central Repository key observer
 *  which can be used to monitor and set a CenRep value.
 *
 *  @lib    lockapp
 *  @since  5.0
 *  @author Tamas Koteles
 */
class CLockAppCenRepObserver : public CBase, public MCenRepNotifyHandlerCallback
	{
	public:

		/**
		 * Two-phased constructor.
		 *
		 * @param aObserver   pointer to observer
		 * @param aCenRepUid  repository Uid
		 * @param aKeyId      repository key Id
		 * @return            the instance just created
		 */
		static CLockAppCenRepObserver* NewL( MLockAppObserverInterface* aObserver, TUid aCenRepUid,
				TUint32 aKeyId );

		/**
		 * Destructor.
		 */
		~CLockAppCenRepObserver( );

		/**
		 * Get key value from CenRep.
		 */
		TInt GetValue( TInt& aValue );

		/**
		 * Get the value from a different CenRep key.
		 */
		TInt GetKeyValue( TUint32 aKey, TInt& aValue );

		/**
		 * Set key value to CenRep.
		 */
		TInt SetValue( TInt aValue );

		/**
		 * Set the value of a different CenRep key.
		 */
		TInt SetKeyValue( TUint32 aKey, TInt aValue );

	public:

		/**
		 * From MCenRepNotifyHandlerCallback. Handles change event. Called by CenRep.
		 *
		 * @param aId the id of the changed setting
		 * @param aNewValue the new value of the changed setting
		 */
		void HandleNotifyInt( TUint32 aId, TInt aNewValue );

		void HandleNotifyError( TUint32 aId, TInt error, CCenRepNotifyHandler* aHandler );

		void HandleNotifyGeneric( TUint32 aId );

	protected:

		/**
		 * C++ default constructor.
		 *
		 * @param aObserver  pointer to observer
		 * @param aCenRepUid central repository Uid
		 * @param aKeyId     key Id
		 */
		CLockAppCenRepObserver( MLockAppObserverInterface* aObserver, TUid aCenRepUid,
				TUint32 aKeyId );

		/**
		 * Symbian OS constructor.
		 */
		void ConstructL( );

	protected:

		/**
		 * Observer's callback interface. Not owned.
		 */
		MLockAppObserverInterface* iObserver;

		/**
		 * CenRep value notifier.
		 */
		CCenRepNotifyHandler* iNotifyHandler;

		/**
		 * Access to central repository.
		 */
		CRepository* iRepository;

		/**
		 * Repository Uid.
		 */
		TUid iCenRepUid;

		/**
		 * Repository key Id.
		 */
		TUint32 iKeyId;

	};

#endif // __LOCKAPPCENREPOBSERVER_H__
// END OF FILE
