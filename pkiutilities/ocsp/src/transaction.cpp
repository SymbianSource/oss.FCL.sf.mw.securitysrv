// Copyright (c) 2001-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// Implementation for OCSP Transaction object.
// 
//

#include "transaction.h"
#include "requestencoder.h"
#include "responsedecoder.h"
#include "ocsp.h"
#include "panic.h"

#include <x509cert.h>
#include <x509certchain.h>

COCSPTransaction* COCSPTransaction::NewL(const TDesC8& aURI, 
										 MOCSPTransport& aTransport, const TUint aRetryCount, const TInt aTimeout)
	{
	COCSPTransaction* self = new (ELeave) COCSPTransaction(aTransport, aRetryCount, aTimeout);
	CleanupStack::PushL(self);
	self->ConstructL(aURI);
	CleanupStack::Pop(self);
	return self;
	}

COCSPTransaction::COCSPTransaction(MOCSPTransport& aTransport, const TUint aRetryCount, const TInt aTimeout) :
	CActive(EPriorityNormal),
	iTransport(aTransport),
	iRetryCount(aRetryCount),
	iTimeout(aTimeout)
	{
	CActiveScheduler::Add(this);
	}


void COCSPTransaction::ConstructL(const TDesC8& aURI)
	{
	iURI = aURI.AllocL();
	}


COCSPTransaction::~COCSPTransaction()
	{
	Cancel();
	delete iURI;
	delete iRequestEncoder;
	delete iResponseDecoder;
	}


// Create the request, then send it off
void COCSPTransaction::SendRequest(COCSPRequest& aRequest, TRequestStatus& aStatus)
	{
	__ASSERT_ALWAYS(!iRequest, Panic(KErrInUse));
	iClientRequestStatus = &aStatus;
	aStatus = KRequestPending;

	iRequest = &aRequest;
	iState = ESendRequest;
	SetActive();

	TRequestStatus* status = &iStatus;
	User::RequestComplete(status, KErrNone);
	}


// Safe regardless of whether we have a request outstanding
void COCSPTransaction::CancelRequest()
	{
	Cancel();
	}


COCSPResponse* COCSPTransaction::TakeResponse()
	{
	__ASSERT_ALWAYS(iResponseDecoder, Panic(KErrNotReady));
	return iResponseDecoder->TakeResponse();
	}


void COCSPTransaction::CompleteClient(const TInt aStatus)
	{
	User::RequestComplete(iClientRequestStatus, aStatus);
	}


void COCSPTransaction::RunL()
	{
	TInt err = iStatus.Int();
	switch (iState)
		{
		case ESendRequest:
			__ASSERT_ALWAYS(err == KErrNone, Panic(KErrCorrupt));
			iRequestEncoder = COCSPRequestEncoder::NewL(*iRequest);
			iTransport.SendRequest(*iURI, iRequestEncoder->Encoding(), iTimeout, iStatus);
			iState = EFinished;
			SetActive();			  
			break;

		case EFinished:
			{
			// Depending on the error retry the request
			// Retry if
			// 		- More attempts remaining AND 
			//		- error is OCSP::KErrTransportTimeout OR
			// 		- error is OCSP::KErrTransportFailure (HTTP response malformed) OR
			//		- invalid response received (OCSPReponse malformed) OR 
			//		- OCSPResponseStatus is OCSP::EServerInternalError OR
			//		- OCSPResponseStatus is OCSP::ETryLater
			//
			// We won't retry if 
			//		- error is OCSP::KErrInvalidURI OR
			//		- error is OCSP::KErrServerNotFound

			TBool retryNeeded = EFalse;
			TInt errDecode = KErrNone;
			if ((err == OCSP::KErrTransportTimeout) || (err == OCSP::KErrTransportFailure))
				{
				retryNeeded = ETrue;
				}
			else if (err == KErrNone)
				{
				TRAP(errDecode, iResponseDecoder = COCSPResponseDecoder::NewL(iTransport.GetResponse()));
				if ((errDecode == OCSP::EMalformedResponse) || 
						(errDecode == OCSP::EServerInternalError) || (errDecode == OCSP::ETryLater))
					{
					retryNeeded = ETrue;
					}
				}

			--iRetryCount;
			if (retryNeeded && iRetryCount > 0)
				{
				__ASSERT_ALWAYS(iResponseDecoder == NULL, Panic(KErrCorrupt));
				iTransport.SendRequest(*iURI, iRequestEncoder->Encoding(), iTimeout, iStatus);
				SetActive();
				}
			else
				{
				// If retry is not needed and there was an error (including +ve) then pass it up else complete normally
				if (err != KErrNone)
					{
					User::Leave(err);
					}
				if (errDecode != KErrNone)
					{
					User::Leave(errDecode);
					}
				CompleteClient(KErrNone);
				}
			}
			break;

		default:
			Panic(KErrCorrupt);
		}
	}


TInt COCSPTransaction::RunError(TInt aError)
	{
	CompleteClient(aError);
	return KErrNone;
	}


void COCSPTransaction::DoCancel()
	{
	iTransport.CancelRequest();
	if (iClientRequestStatus)
		{
		CompleteClient(KErrCancel);
		}
	}

