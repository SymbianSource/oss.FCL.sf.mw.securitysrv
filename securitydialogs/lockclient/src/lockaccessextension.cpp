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


#include "lockaccessextension.h"
#include <lockappclientserver.h>
#include <e32property.h> // P&S API
#include <apgtask.h> // TApaTask, TApaTaskList
#include <coemain.h> // CCoeEnv

#include <xqservicerequest.h>
#include <xqserviceutil.h>
#include <xqrequestinfo.h>
#include <QDebug>

// Constants
const TInt KTimesToConnectServer( 2);
const TInt KTimeoutBeforeRetrying( 50000);

// ---------------------------------------------------------------------------
// Gets server version, needed for connection
// ---------------------------------------------------------------------------
TVersion RLockAccessExtension::GetVersion( )
	{
	return TVersion( KLockAppServMajorVersion, KLockAppServMinorVersion, KLockAppServBuildVersion );
	}

// ---------------------------------------------------------------------------
// Connects client to lockapp application server
// ---------------------------------------------------------------------------
TInt RLockAccessExtension::TryConnect( RWsSession& aWsSession )
	{
	TInt ret(KErrNone);
	/*
	this is the old methd. Now we use QtHighway
	TApaTaskList list(aWsSession);
	// check that lockapp is running
	TApaTask task = list.FindApp( KLockAppUid );
	if ( task.Exists( ) )
		{
		if ( Handle( )== NULL )
			{
			// connect session to the server
			ret = CreateSession( KLockAppServerName, GetVersion( ) );
			}
		else
			{
			// not CreateSession because Handle( ) not NULL
			}
		}
	else
		{
		// LockApp task not found
		RDebug::Printf( "%s %s (%u) ???? LockApp task not found=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, KLockAppUid );
		ret = KErrNotReady;
		}
	*/
	qDebug() << "============= RLockAccessExtension::TryConnect";
	qDebug() << ret;
	return ret;
	}

// ---------------------------------------------------------------------------
// Ensures that the connection to the service is alive.
// ---------------------------------------------------------------------------
TInt RLockAccessExtension::EnsureConnected( )
	{
	TInt ret(KErrNone);
	/*
	this is the old methd. Now we use QtHighway
	// we need CCoeEnv because of window group list
	CCoeEnv* coeEnv = CCoeEnv::Static( );
	if ( coeEnv )
		{
		// All server connections are tried to be made KTriesToConnectServer times because
		// occasional fails on connections are possible at least on some servers
		TInt retry(0);
		while ( (ret = TryConnect( coeEnv->WsSession( ) ) ) != KErrNone && 
				(retry++) <= KTimesToConnectServer )
			{
			User::After( KTimeoutBeforeRetrying );
			}
		// the connection state gets returned
		}
	else
		{
		// No CCoeEnv
		ret = KErrNotSupported;
		}
	*/
	qDebug() << "============= RLockAccessExtension::EnsureConnected";
	qDebug() << ret;
	return ret;
	}

// ---------------------------------------------------------------------------
// Sends blind message to lockapp if the session is connected.
// ---------------------------------------------------------------------------
TInt RLockAccessExtension::SendMessage( TInt aMessage )
	{
	TInt ret = EnsureConnected( );
	if ( ret == KErrNone )
		{
		// ret = SendReceive( aMessage );
		ret = SendMessage( aMessage, -1, -1 );
		}
	return ret;
	}

// ---------------------------------------------------------------------------
// Sends blind message to lockapp if the session is connected.
// ---------------------------------------------------------------------------
TInt RLockAccessExtension::SendMessage( TInt aMessage, TInt aParam1 )
	{
	TInt ret = EnsureConnected( );
	if ( ret == KErrNone )
		{
		// assign parameters to IPC argument
		TIpcArgs args(aParam1);
		// ret = SendReceive( aMessage, args );
		ret = SendMessage( aMessage, aParam1, -1 );
		}
	return ret;
	}

// ---------------------------------------------------------------------------
// Sends blind message to lockapp if the session is connected.
// ---------------------------------------------------------------------------
TInt RLockAccessExtension::SendMessage( TInt aMessage, TInt aParam1, TInt aParam2 )
	{
	TInt ret = EnsureConnected( );
	if ( ret == KErrNone )
		{
		// assign parameters to IPC argument
		TIpcArgs args( aParam1, aParam2);
		// this is the old methd. Now we use QtHighway
		// ret = SendReceive( aMessage, args );
    qDebug() << "============= RLockAccessExtension::SendMessage 123.1";
    qDebug() << aMessage;
    qDebug() << aParam1;
    qDebug() << aParam2;
    XQServiceRequest* mServiceRequest;
    qDebug() << "============= RLockAccessExtension::SendMessage 2";
    mServiceRequest = new XQServiceRequest("com.nokia.services.AutolockSrv.AutolockSrv","dial(QString,bool)");// use   , false    to make async
    qDebug() << "============= RLockAccessExtension::SendMessage 2.1";
    qDebug() << mServiceRequest;
    QString label = "";
    label += QString("%1").arg(aMessage);
    *mServiceRequest << QString(label);
    qDebug() << "============= RLockAccessExtension::SendMessage 2.2";
    bool isSync = false;
    *mServiceRequest << isSync;
    qDebug() << "============= RLockAccessExtension::SendMessage 3";
    int returnvalue;
    bool ret = mServiceRequest->send(returnvalue);
    qDebug() << "============= RLockAccessExtension::SendMessage 4";
    qDebug() << ret;
    
		}
	return ret;
	}

// End of File
