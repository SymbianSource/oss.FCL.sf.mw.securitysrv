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
* Description:  Server implementation that responds to client API in lockclient
 *
*/


#include "lockappserver.h"
#include "lockappsession.h"
#include <lockappclientserver.h>
#include <coemain.h>

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
CLockAppServer* CLockAppServer::NewL( const TDesC& aFixedServerName )
	{

	CLockAppServer* self = new (ELeave) CLockAppServer();
	CleanupStack::PushL( self );
	self->SetPriority( EActivePriorityIpcEventsHigh );
	self->ConstructL( aFixedServerName );
	CleanupStack::Pop( self );
	return self;
	}

// ---------------------------------------------------------------------------
// Constructor.
// ---------------------------------------------------------------------------
CLockAppServer::CLockAppServer( )
	{

	// no implementation required
	}

// ---------------------------------------------------------------------------
// From @c CApaAppServer. Creates LockApp Session, only one type of
// service is supported by the server.
// ---------------------------------------------------------------------------
CApaAppServiceBase* CLockAppServer::CreateServiceL( TUid aServiceType ) const
	{

	// only one service is offered through server
	if ( aServiceType == KLockAppServiceUid )
		{
		return new (ELeave) CLockAppSession();
		}
	else
		{
		return CApaAppServer::CreateServiceL( aServiceType );
		}
	}

// ---------------------------------------------------------------------------
// From @c CApaAppServer. Creates a new client session, version
// numbering not supported.
// ---------------------------------------------------------------------------
CSession2* CLockAppServer::NewSessionL( const TVersion&, const RMessage2& ) const
	{

	return new (ELeave) CLockAppSession();
	}
