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
* Description:  LockApp server session requested by lockclient
 *
*/


#ifndef __LOCKAPPSESSION_H__
#define __LOCKAPPSESSION_H__

// INCLUDES
#include <apaserverapp.h>
#include "lockapp.hrh"

// FORWARD DECLARATIONS
class MLockAppStateControl;

/**
 *  CLockAppSession class is the server side implementation of the Lockapp API. 
 *  The session class offers keyguard and devicelock API for clients wanting to 
 *  alter phone lock states.
 *
 *  @lib    lockapp
 *  @since  5.0
 *  @author Joona Petrell
 *  @author Tamas Koteles
 */
class CLockAppSession : public CApaAppServiceBase
	{
	public:

		/**
		 * C++ default constructor.
		 */
		CLockAppSession( );

	private:
		/**
		 * Destructor.
		 */
		~CLockAppSession( );

		/**
		 * From @c CApaAppServiceBase. Handles possible error in
		 * service. Method is empty.
		 * @param aMessage received message
		 * @param aError an error id
		 */
		void ServiceError( const RMessage2& aMessage, TInt aError );

		/**
		 * From @c CApaAppServiceBase. Receives messages.
		 * @param aMessage received message
		 */
		void ServiceL( const RMessage2& aMessage );

	private:

		/**
		 * Ask state control to activate keyguard lock.
		 *
		 * @param aWithNote if "keys locked" note is shown
		 * @return error code
		 */
		TInt DoEnableKeyguardL( TBool aWithNote );

		/**
		 * Ask state control to deacctivate keyguard lock.
		 *
		 * @param aWithNote if "keys active" note is shown.
		 * @return error code
		 */
		TInt DoDisableKeyguardL( TBool aWithNote );

		/**
		 * Ask state control to activate devicelock.
		 * @return error code
		 */
		TInt DoEnableDevicelockL( TDevicelockReason aReason );

		/**
		 * Ask state control to deactivate devicelock.
		 * @return error code
		 */
		TInt DoDisableDevicelockL( );

		/**
		 * Ask state control to offer to enable keyguard.
		 * by showing "offerkeylock" note.
		 * @return error code
		 */
		TInt DoOfferKeyguardL( );

		/**
		 * Ask state control to inform user that keys are locked.
		 * @return error code
		 */
		TInt DoShowKeysLockedNoteL( );

	private:

		/**
		 * Access to lock state control
		 * @return the main state control, from which lock states are changed
		 */
		MLockAppStateControl* StateControlL( );

		/**
		 * The main state control
		 * Not owned.
		 */
		MLockAppStateControl* iStateControl;
	};

#endif //__LOCKAPPSESSION_H__
