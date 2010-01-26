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

#include "requestlogger.h"

#include <f32file.h>

CTOCSPRequestLogger* CTOCSPRequestLogger::NewL(const TDesC& aRequestFile, MOCSPTransport* aTransport)
	{
	CTOCSPRequestLogger* self = new (ELeave) CTOCSPRequestLogger;
	CleanupStack::PushL(self);
	self->ConstructL(aRequestFile, aTransport);
	CleanupStack::Pop(self);
	return self;
	}


CTOCSPRequestLogger::CTOCSPRequestLogger()	
	{
	}


void CTOCSPRequestLogger::ConstructL(const TDesC& aRequestFile, MOCSPTransport* aTransport)
	{
	User::LeaveIfError(iFs.Connect());

	TInt err = iFs.MkDirAll(aRequestFile);
	if (err != KErrAlreadyExists)
		{
		User::LeaveIfError(err);
		}

	User::LeaveIfError(iWriteStream.Replace(iFs, aRequestFile, EFileWrite | EFileShareExclusive));

	// Initially we write a request count of zero, this gets updates as each
	// request is logged
	iWriteStream.WriteUint32L(0);

	iTransport = aTransport;
	}


CTOCSPRequestLogger::~CTOCSPRequestLogger()
	{
	iWriteStream.Close();
	iFs.Close();
	delete iTransport;
	}


void CTOCSPRequestLogger::SendRequest(const TDesC8& aURI,
									  const TDesC8& aRequest, 
									  const TInt aTimeout,
									  TRequestStatus& aStatus)
	{
	TRAPD(err, LogRequestL(aURI, aRequest));

	if (err == KErrNone)
		{
		iTransport->SendRequest(aURI, aRequest, aTimeout, aStatus);
		}
	else
		{
		TRequestStatus* status = &aStatus;
		User::RequestComplete(status, err);
		}
	}

void CTOCSPRequestLogger::LogRequestL(const TDesC8& aURI,
									  const TDesC8& aRequest)
	{
	++iTotalRequests;

	MStreamBuf* sink = iWriteStream.Sink();

	sink->SeekL(MStreamBuf::EWrite, EStreamBeginning, 0);
	iWriteStream.WriteUint32L(iTotalRequests);

	sink->SeekL(MStreamBuf::EWrite, EStreamEnd, 0);
	iWriteStream.WriteUint32L(aURI.Length());
	iWriteStream.WriteL(aURI);
	iWriteStream.WriteUint32L(aRequest.Length());
	iWriteStream.WriteL(aRequest);

	iWriteStream.CommitL();
	}


void CTOCSPRequestLogger::CancelRequest()
	{
	iTransport->CancelRequest();
	}


TPtrC8 CTOCSPRequestLogger::GetResponse() const
	{
	return iTransport->GetResponse();
	}
