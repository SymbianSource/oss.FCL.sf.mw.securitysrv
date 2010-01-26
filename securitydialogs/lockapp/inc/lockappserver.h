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
* Description:  Server implementation that responses to client API in
 *                lockclient
 *
*/


#ifndef __LOCKAPPSERVER_H__
#define __LOCKAPPSERVER_H__

// INCLUDES
#include <apaserverapp.h>

/**
 *  CLockAppServer class implements the application server of LockApp.
 *  The server is created with a fixed server name and offers only one service.
 *
 *  @lib    lockapp
 *  @since  5.0
 *  @author Joona Petrell
 *  @author Tamas Koteles
 */
class CLockAppServer : public CApaAppServer
	{
	public:

		/**
		 * Two-phased constructor.
		 */
		static CLockAppServer* NewL( const TDesC& aFixedServerName );

		/**
		 * Offers support for multiple services, but only one is supported/used by LockApp Server.
		 * 
		 * @param aServiceType uid used to identify different services.
		 */
		CApaAppServiceBase* CreateServiceL( TUid aServiceType ) const;

	private:

		/**
		 * C++ default constructor.
		 */
		CLockAppServer( );

		/**
		 * Establishes a new session between client and server.
		 * 
		 * @param aVersion for support of new server client-server API variations (not used).
		 * @param aMessage message that started the session.
		 */
		CSession2* NewSessionL( const TVersion& aVersion, const RMessage2& aMessage ) const;

	private:

		// how many sessions are created to the server
		TInt iSessionCount;

	};

#endif //__LOCKAPPSERVER_H__
