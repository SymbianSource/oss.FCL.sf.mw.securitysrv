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
#include "../../Autolock/PubSub/securityuisprivatepskeys.h"
#include <apgcli.h>
#include <apacmdln.h>

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
	(void)aWsSession;
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

	// Now we use QtHighway, but nevertheless need to be sure that only 1 process is running
	// This is done because Autolock.exe should start at the beginning, but it might not be ready yet.
	// As Qthighway will start it, it's better to give time for the first one to prepare itself.
	TInt err = KErrNone;
	err = err;
	TInt numAttempts = 0;
	TInt numberOfInstances = 0;
	do
		{
		numberOfInstances=0;
		TFullName processName;
		TFindThread find(_L("*utolock*"));	// first letter can can be uppercase or lowercase
		while( find.Next( processName ) == KErrNone )
		    {
				// Autolock[100059b5]0002::Autolock		in device
				// autolock.exe[100059b5]0002::Main		in emulator
				RDEBUG("found process", 1);
        numberOfInstances++;
		    }	// end while
		RDEBUG("numberOfInstances", numberOfInstances);
		if(numberOfInstances<=0)
			{
			RDEBUG("Autolock.exe not running already. Starting.", 0 );
	    RApaLsSession ls;
	    User::LeaveIfError(ls.Connect());
	    CleanupClosePushL(ls);
	    RDEBUG("commandLine", 0); 
	    CApaCommandLine* commandLine = CApaCommandLine::NewLC();
	    commandLine->SetExecutableNameL(_L("autolock.exe"));
	    commandLine->SetCommandL(EApaCommandRun);
	    // Try to launch the application.
	    RDEBUG("StartApp", 0); 
	    TInt err = ls.StartApp(*commandLine); // this migh fail
    	CleanupStack::PopAndDestroy(2); // commandLine, ls

			RDEBUG("Autolock.exe launched. Waiting a bit. err", err );
			User::After(1000*1000);
			RDEBUG("re-verifying Autolock.exe process.", 1 );
			}
		} while (numAttempts++ <3 && numberOfInstances<=0);
		


	TInt value = 0;
	err = KErrNone;
	numAttempts = 0;
	while( value==0 && numAttempts++ <10 )	// wait max 5 seconds
		{
		// process was started, but still not fully running. Give a bit more time
		err = RProperty::Get(KPSUidSecurityUIs, KSecurityUIsLockInitiatorUID, value);
		RDEBUG("err", err);
		RDEBUG("value", value);
		if(value==0 || value==-1)
			{
			RDEBUG("Autolock.exe has started but it's not fully running", value);
			User::After(5*100*1000);	// half a second
			}
		}
	RDEBUG("numAttempts", numAttempts);
	/*
	this is the old method. 
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
	RDEBUG("0", 0);
	TInt ret = KErrNone;
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
	RDEBUG("0", 0);
	TInt ret = KErrNone;
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
	RDEBUG("0", 0);
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
