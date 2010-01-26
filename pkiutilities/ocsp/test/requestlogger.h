// Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
// A transport that logs the ocsp requests made to a file and then calls another
// transport to send the request.
// 
//

/**
 @file 
 @internalTechnology
*/

#ifndef __TOCSP_REQUEST_LOGGER_H__
#define __TOCSP_REQUEST_LOGGER_H__

#include <e32base.h>
#include <ocsp.h>
#include <s32file.h>
#include "ocsprequestandresponse.h"

class CTOCSPRequestLogger : public CBase, public MOCSPTransport
	{
public:

	/** Create a new request logger.  Takes ownership of the transport unless it leaves. */
	static CTOCSPRequestLogger* NewL(const TDesC& aRquesteFile, MOCSPTransport* aTransport);

private:

	// From MOCSPTransport
	~CTOCSPRequestLogger();
	void SendRequest(const TDesC8& aURI, const TDesC8& aRequest, const TInt aTimeout, TRequestStatus& aStatus);
	void CancelRequest();
	TPtrC8 GetResponse() const;

private:

	CTOCSPRequestLogger();
	void DoSendRequestL();
	void ConstructL(const TDesC& aResponseFile, MOCSPTransport* aTransport);
	void LogRequestL(const TDesC8& aURI, const TDesC8& aRequest);

private:

	RFs				 iFs;
	RFileWriteStream iWriteStream;
	TInt			 iTotalRequests;
	MOCSPTransport*	 iTransport;
	};

#endif // __TOCSP_TRANSPORT_H__
