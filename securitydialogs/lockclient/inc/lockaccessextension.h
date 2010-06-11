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
* Description:  Extension to lockapp clients.
 *
*/


#ifndef __LOCKACCESSEXTENSION_H__
#define __LOCKACCESSEXTENSION_H__

#include <e32base.h>

#if defined(_DEBUG)
#define RDEBUG( x, y ) RDebug::Printf( "%s %s (%u) %s=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, x, y );
#else
#define RDEBUG( x, y )
#endif

class RWsSession;

/**
 *  RLockAccessExtension class.
 *  Extension class used to access lockapp.
 *
 *  @lib    lockclient
 *  @since  5.0
 *  @author Tamas Koteles
 */
class RLockAccessExtension : public RSessionBase
	{
	public:

		/**
		 * From @c RSessionBase. Sends a message.
		 * @return KErrNone if succeeded or already connected.
		 *         KErrNotReady if the server is not yet fully initialized.
		 *         otherwise standard RSessionBase error values.
		 */
		TInt SendMessage( TInt aMessage );

		/**
		 * From @c RSessionBase. Sends a message with a parameter.
		 * @return KErrNone if succeeded or already connected.
		 *         KErrNotReady if the server is not yet fully initialized.
		 *         otherwise standard RSessionBase error values.
		 */
		TInt SendMessage( TInt aMessage, TInt aParam1 );

		/**
		 * From @c RSessionBase. Sends a message with two parameter.
		 * @return KErrNone if succeeded or already connected.
		 *         KErrNotReady if the server is not yet fully initialized.
		 *         otherwise standard RSessionBase error values.
		 */
		TInt SendMessage( TInt aMessage, TInt aParam1, TInt aParam2 );

	private:

		/**
		 * Returns the server's version.
		 */
		TVersion GetVersion( );

		/**
		 * Tries to create session to service.
		 * Must be called before accessing other methods in the API.
		 *
		 * @return KErrNone if succeeded or already connected.
		 *         KErrNotReady if the server is not yet fully initialized.
		 *         KErrNotSupported if the CoeEnv is not available
		 *         otherwise standard RSessionBase error values.
		 */
		TInt TryConnect( RWsSession& aWsSession );

		/**
		 * Ensures that the connection to the service is alive.
		 * Tries to connect to service if not connected.
		 *
		 * @return KErrNone if succeeded or already connected.
		 *         KErrNotReady if the server is not yet fully initialized.
		 *         KErrNotSupported if the CoeEnv is not available
		 *         otherwise standard RSessionBase error values.
		 */
		TInt EnsureConnected( );

	};

#endif // __LOCKACCESSEXTENSION_H__
