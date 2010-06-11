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
#include <xqaiwrequest.h>
#include <xqappmgr.h>

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
	RDEBUG("ret", ret);
	return ret;
	}

// ---------------------------------------------------------------------------
// Ensures that the connection to the service is alive.
// ---------------------------------------------------------------------------
TInt RLockAccessExtension::EnsureConnected( )
	{
	TInt ret(KErrNone);
	/*
	this is the old method. Now we use QtHighway
	// we need CCoeEnv because of window group list
	const TInt KTimesToConnectServer( 2);
	const TInt KTimeoutBeforeRetrying( 50000);
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
	RDEBUG("ret", ret);
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
	RDEBUG("ret", ret);
	if ( ret == KErrNone )
		{
		// assign parameters to IPC argument
		// TIpcArgs args( aParam1, aParam2);
		// this is the old methd. Now we use QtHighway
		// ret = SendReceive( aMessage, args );
    RDEBUG("aMessage", aMessage);
    RDEBUG("aParam1", aParam1);
    RDEBUG("aParam2", aParam2);

		
			{	// old method. Not used any more. Kept as reference
			/*
	    XQServiceRequest* mServiceRequest;
      RDEBUG("XQServiceRequest", 0);
	    mServiceRequest = new XQServiceRequest("com.nokia.services.Autolock.Autolock","service(QString,QString,QString)");// use   , false    to make async
      RDEBUG("aMessage", 0);
	    QString label;
	    label = "" + QString("%1").arg(aMessage);
	    *mServiceRequest << QString(label);
      RDEBUG("aParam1", 0);
	    label = "" + QString("%1").arg(aParam1);
	    *mServiceRequest << QString(label);
      RDEBUG("aParam2", 0);
	    label = "" + QString("%1").arg(aParam2);
	    *mServiceRequest << QString(label);
	    int returnvalue;
      RDEBUG("send", 0);
	    bool ret = mServiceRequest->send(returnvalue);
      RDEBUG("ret", ret);
      RDEBUG("returnvalue", returnvalue);
      */
			}

			RDEBUG("args", 0);
			QList<QVariant> args;
	    args << QVariant(QString(QString::number(aMessage)));
	    args << QVariant(QString(QString::number(aParam1)));
	    args << QVariant(QString(QString::number(aParam2)));

	    XQApplicationManager mAppManager;
	    XQAiwRequest *request;
	    RDEBUG("create", 0);
			request = mAppManager.create("com.nokia.services.Autolock", "Autolock", "service(QString,QString,QString)", false);
			// also works with		create("Autolock", "service(QString,QString,QString)", false);
			if(request)
				{
	    	RDEBUG("got request", 0);
	    	}
	    else
	    	{
	 	    RDEBUG("not got request", 0);
	 	    RDebug::Printf( "%s %s (%u) not got request=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
	 	    return KErrAbort;
	 	  	}
				
			RDEBUG("setArguments", 0);
			request->setArguments(args);
			RDEBUG("args", 0);
	    int returnvalue=0;
	    QVariant var = QVariant(returnvalue);
			RDEBUG("send", 0);
			bool retSend = request->send(var);
			returnvalue = var.toInt();
			RDEBUG("retSend", retSend);
			RDEBUG("returnvalue", returnvalue);
	    int error = request->lastError();
			RDEBUG("error", error);
	    ret = returnvalue;
	
	    delete request;
		}
  RDEBUG("ret", ret);
	return ret;
	}

// End of File
