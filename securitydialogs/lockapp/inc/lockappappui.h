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
* Description:  LockApp Application UI class
 *
*/


#ifndef __LOCKAPP_APPUI_H__
#define __LOCKAPP_APPUI_H__

// INCLUDES
#include <aknappui.h>
#include "lockapp.hrh"

// FORWARD DECLARATIONS
class CLockAppAppView;
class MLockAppStateControl;
class CLockAppStateControl;
class CLockAppServer;

/**
 *  CLockAppApplication class is the central user interface class in Avkon.
 *  Owns the LockApp state control and the server component.
 *
 *  @lib    lockapp
 *  @since  5.0
 *  @author Joona Petrell
 *  @author Tamas Koteles
 */
class CLockAppAppUi : public CAknAppUi
	{
	public:

		/**
		 * Second constructor that can fail (leave).
		 */
		void ConstructL( );

		/**
		 * C++ default constructor.
		 */
		CLockAppAppUi( );

		/**
		 * Destructor.
		 */
		~CLockAppAppUi( );

	public:

		/** 
		 * @see CAknAppUi::HandleResourceChangeL(TInt aType) 
		 */
		void HandleResourceChangeL( TInt aType );

	public:

		/**
		 * Access to main lock state control (used by the server).
		 *
		 * @return pointer to state control
		 */
		MLockAppStateControl* StateControl( );

	protected:

#ifdef __SAP_TERMINAL_CONTROL_FW

		/**
		 * From CEikAppUi, handles the TARM unlock message, other messages
		 * are propagated to the superclass handler.
		 * 
		 * @param aClientHandleOfTargetWindowGroup The window group that the message was sent to.
		 * @param aMessageUid The message UID.
		 * @param aMessageParameters The message parameters
		 * @return TMessageResponse EMessageHandled if the message was the TARM unlock message,
		 * otherwise the return value from the superclass handler.
		 */
		MCoeMessageObserver::TMessageResponse HandleMessageL(
				TUint32 aClientHandleOfTargetWindowGroup,
				TUid aMessageUid,
				const TDesC8& aMessageParameters );

#endif // __SAP_TERMINAL_CONTROL_FW

		/** 
		 * @see CAknAppUi::HandleWsEventL(const TWsEvent& aEvent,CCoeControl* aDestination) 
		 */
		void HandleWsEventL( const TWsEvent& aEvent, CCoeControl* aDestination );

	private:

		/**
		 * Main control of the LockApp.
		 * Owned.
		 */
		CLockAppStateControl* iStateControl;

		/**
		 * Main server of the LockApp.
		 * Own.
		 */
		CLockAppServer* iLockServer;
	};

#endif // __LOCKAPP_APPUI_H__
