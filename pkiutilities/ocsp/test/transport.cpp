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
// Test transport object.  May be used in place of the default transport used by
// the ocsp module.  May later be extended to act as a dummy server producing
// a variety of test responses.
// 
//

#include "transport.h"

#include <f32file.h>

CTOCSPTransport* CTOCSPTransport::NewL(const TDesC& aResponseFile, const TDesC* aRequestFile)
	{
	CTOCSPTransport* self = new (ELeave) CTOCSPTransport;
	CleanupStack::PushL(self);
	self->ConstructL(aResponseFile, aRequestFile);
	CleanupStack::Pop(self);
	return self;
	}


void CTOCSPTransport::ConstructL(const TDesC& aResponseFile, const TDesC* aRequestFile)
	{
	User::LeaveIfError(iFs.Connect());
	User::LeaveIfError(iResponseStream.Open(iFs, aResponseFile, EFileShareReadersOnly));
	iTransactions = iResponseStream.ReadUint32L();

	if (aRequestFile)
		{
		iRequestStream = new (ELeave) RFileReadStream;
		User::LeaveIfError(iRequestStream->Open(iFs, *aRequestFile, EFileShareReadersOnly));
		TInt requests = iRequestStream->ReadUint32L();
		__ASSERT_ALWAYS(requests == iTransactions, User::Leave(KErrCorrupt));	
		} 		
	}


CTOCSPTransport::~CTOCSPTransport()
	{
	iResponseStream.Close();
	
	if (iRequestStream)
		{
		iRequestStream->Close();
		delete iRequestStream;
		}
	
	iFs.Close();
	delete iResponseData;
	}


void CTOCSPTransport::SendRequest(const TDesC8& aURI,
								  const TDesC8& aRequest, 
								  const TInt,				// Timeout not used
								  TRequestStatus& aStatus)
	{
	iCallBack = &aStatus;
	aStatus = KRequestPending;

	TRAPD(error, DoSendRequestL(aURI, aRequest));
	User::RequestComplete(iCallBack, error);
	}


void CTOCSPTransport::DoSendRequestL(const TDesC8& aURI,
									 const TDesC8& aRequest)
	{	
	__ASSERT_ALWAYS(iIndex++ < iTransactions, User::Leave(TOCSP::ETooManyTransactions));

	CheckRequestL(aURI, aRequest);

	delete iResponseData;
	iResponseData = NULL;

	iResponseData = ReadDataLC(iResponseStream);
	CleanupStack::Pop(iResponseData);
	}


void CTOCSPTransport::CheckRequestL(const TDesC8& aURI,
									const TDesC8& aRequest)
	{
	if (!iRequestStream)
		{
		return;
		}

	HBufC8* expectURI = ReadDataLC(*iRequestStream);
	if (aURI != *expectURI)
		{
		User::Leave(TOCSP::EURIMismatch);
		}
	CleanupStack::PopAndDestroy(expectURI);
	
	HBufC8* expectRequest = ReadDataLC(*iRequestStream);
	if (aRequest != *expectRequest)
		{
		User::Leave(TOCSP::ERequestMismatch);
		}
	CleanupStack::PopAndDestroy(expectRequest);
	}

HBufC8* CTOCSPTransport::ReadDataLC(RFileReadStream& aStream)
	{
	TUint32 len = aStream.ReadUint32L();
	HBufC8* data = HBufC8::NewLC(len);
	TPtr8 dataDes = data->Des();
	aStream.ReadL(dataDes, len);
	return data;
	}

void CTOCSPTransport::CancelRequest()
	{
	// If its not been done already, complete the active object
	if (iCallBack)
		{
		User::RequestComplete(iCallBack, KErrCancel);
		}
	}


TPtrC8 CTOCSPTransport::GetResponse() const
	{
	__ASSERT_ALWAYS(iResponseData, User::Panic(_L("TOCSP"), KErrNotReady));
	return *iResponseData;
	}
