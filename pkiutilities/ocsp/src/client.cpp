// Copyright (c) 2002-2009 Nokia Corporation and/or its subsidiary(-ies).
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
//

#include "transaction.h"
#include "validator.h"
#include "panic.h"
#include <ocsp.h>
#include <x509certext.h>
#include <x509certchain.h>
#include <x509cert.h>
#include <asn1dec.h>

EXPORT_C COCSPClient* COCSPClient::NewL(const COCSPParameters* aParams)
	{
	COCSPClient* self = new (ELeave) COCSPClient();
	CleanupStack::PushL(self);
	self->ConstructL(aParams);
	CleanupStack::Pop(self);
	return self;
	}

COCSPClient::COCSPClient() :
	CActive(EPriorityNormal)
	{
	CActiveScheduler::Add(this);
	}

void COCSPClient::ConstructL(const COCSPParameters* aParams)
	{
	__ASSERT_ALWAYS(aParams, Panic(KErrArgument));

	// Caller must supply transport
	if (!aParams->Transport())
		{
		User::Leave(KErrArgument);
		}

	// Range check request timeout and retry values to avoid later panics
	if (aParams->Timeout() < KTransportDefaultRequestTimeout)
		{
		User::Leave(KErrArgument);
		}
	if (aParams->RetryCount() < KTransportDefaultRequestRetryCount)
		{
		User::Leave(KErrArgument);
		}

	iParams = aParams; // Take ownership
	
	// Make a request for every certificate
	for (TUint i = 0 ; i < aParams->CertCount() ; ++i)
		{
		// Following are the condition in which the request would not be 
		// added for ocsp check:
		// If CheckCertsWithAiaOnly is enabled and certificate does 
		// not contain AIA extension.
		//					Or
		// CheckCertsWithAiaOnly is disabled and 
		// GenerateResponseFromMissingURI is disabled and 
		// AIA is absent and
		// Global OCSP URL is absent
		
		if (! (	(	aParams->CheckCertsWithAiaOnly() && 
					!OCSPUtils::IsAIAForOCSPPresentL( aParams->SubjectCert(i) )
				) || 
				(	!aParams->CheckCertsWithAiaOnly() 
					&& !aParams->GenerateResponseForMissingUri() 
					&& !OCSPUtils::IsAIAForOCSPPresentL(aParams->SubjectCert(i)) 
					&& aParams->DefaultURI() == KNullDesC8()
				)
			   )
			)
			{
			COCSPRequest* request = COCSPRequest::NewLC(aParams->UseNonce());
			request->AddCertificateL(aParams->SubjectCert(i), aParams->IssuerCert(i));
			User::LeaveIfError(iRequests.Append(request));
			CleanupStack::Pop(request);
			}
			
		}
	iValidator = COCSPValidator::NewL(*aParams);
	}


COCSPClient::~COCSPClient()
	{
	Cancel();
	delete iParams;
	delete iURI;
	delete iTransport;
	delete iTransaction;
	delete iValidator;
	iRequests.ResetAndDestroy();
	iResponses.ResetAndDestroy();
	iOutcomes.Close();
	}

EXPORT_C OCSP::TResult COCSPClient::SummaryResult(void) const
	{
	__ASSERT_ALWAYS(iState == EHaveResult, Panic(KErrNotReady));
	return iSummaryResult;
	}

EXPORT_C TInt COCSPClient::TransactionCount(void) const
	{
	__ASSERT_ALWAYS(iState == EHaveResult, Panic(KErrNotReady));
	return iResponses.Count();
	}

EXPORT_C const COCSPRequest& COCSPClient::Request(TInt aIndex) const
	{
	__ASSERT_ALWAYS(iState == EHaveResult, Panic(KErrNotReady));
	__ASSERT_ALWAYS(aIndex >= 0 && aIndex < iRequests.Count(), Panic(KErrNotFound));
	return *(iRequests[aIndex]);
	}

EXPORT_C const TOCSPOutcome& COCSPClient::Outcome(TInt aIndex) const
	{
	__ASSERT_ALWAYS(iState == EHaveResult, Panic(KErrNotReady));
	__ASSERT_ALWAYS(aIndex >= 0 && aIndex < iOutcomes.Count(), Panic(KErrNotFound));
	return iOutcomes[aIndex];
	}

EXPORT_C const COCSPResponse* COCSPClient::Response(TInt aIndex) const
	{
	__ASSERT_ALWAYS(iState == EHaveResult, Panic(KErrNotReady));
	__ASSERT_ALWAYS(aIndex >= 0 && aIndex < iResponses.Count(), Panic(KErrNotFound));
	return iResponses[aIndex];
	}

EXPORT_C void COCSPClient::Check(TRequestStatus& aStatus)
	{
	__ASSERT_ALWAYS(iState == EInitial, Panic(KErrInUse));

	iClientStatus = &aStatus;
	TInt err = KErrNone;

	if (iRequests.Count() == 0)
		{
		err = OCSP::KErrNoCertificates;
		}

	if (err == KErrNone)
		{
		aStatus = KRequestPending;
		DoCheck();
		}

	if (err != KErrNone)
		{
		User::RequestComplete(iClientStatus, err);
		iState = EError;
		}
	}

EXPORT_C void COCSPClient::CancelCheck()
	{
	Cancel();
	}

EXPORT_C TBool COCSPClient::CertsAvailableForOCSPCheck()
	{
	return iRequests.Count();
	}

TInt COCSPClient::RunError(TInt aError)
	{
	User::RequestComplete(iClientStatus, aError);
	iState = EError;
	return KErrNone;
	}

void COCSPClient::DoCancel()
	{
	if (iTransaction)
		{
		iTransaction->CancelRequest();
		}
	
	iValidator->Cancel();
	if (iClientStatus)
		{
		User::RequestComplete(iClientStatus, KErrCancel);
		}
	}	

void COCSPClient::DoCheck()
	{
	iSummaryResult = OCSP::EGood;
	SendRequest();
	}

void COCSPClient::RunL()
	{
	switch (iState)
		{
		case ESendingRequest:
			HandleResponseReceivedL();
			break;
			
		case EValidatingResponse:
			User::LeaveIfError(iStatus.Int());
			HandleResponseValidatedL();
			break;
						
		default:
			Panic(KErrCorrupt);
			break;
		}
	}

/**
 * Receive the response, if the response was received correctly, 
 * perform validation based on the scheme in use.
 */
void COCSPClient::HandleResponseReceivedL()
	{
	TInt status = iStatus.Int();
	
	if (status == KErrNone)
		{
		COCSPResponse* response = iTransaction->TakeResponse();
		CleanupStack::PushL(response);
		User::LeaveIfError(iResponses.Append(response));
		CleanupStack::Pop(response);
		ValidateResponseL();
		}
	else if (status > 0)
		{
		HandleTransactionErrorL(static_cast<OCSP::TStatus>(status));
		}
	else if ((status == OCSP::KErrTransportFailure) || (status == OCSP::KErrServerNotFound))
		{
		HandleTransactionErrorL(OCSP::ETransportError);
		}
	else if (status == OCSP::KErrInvalidURI)
		{
		HandleTransactionErrorL(OCSP::EInvalidURI);
		}
	else if (status == OCSP::KErrTransportTimeout)
		{
		HandleTransactionErrorL(OCSP::ETimeOut);
		}
	else
		{
		User::Leave(status);		   
		}
	}

/**
 * Called when there's an error getting a response, and it's one of our non-fatal errors.
 * We record the error and continue checking.
 */

void COCSPClient::HandleTransactionErrorL(OCSP::TStatus aStatus)
	{
	User::LeaveIfError(iOutcomes.Append(TOCSPOutcome(aStatus, OCSP::EUnknown)));
	User::LeaveIfError(iResponses.Append(NULL));
	HandleResponseValidatedL();
	}

/**
 * Following is the sequence followed in this method:
 * 1. Check the result for validation of the current response and update the Summary result accordingly.
 * 2. If delegate certificate has to be checked further initiate the same.
 * 3. if all request have not been processed then start validation for the next request.
 * 4. If all request have been processed complete the original client request.
 */
void COCSPClient::HandleResponseValidatedL()
	{
	TInt index = iOutcomes.Count() - 1;
	const TOCSPOutcome& outcome = iOutcomes[index];

	if (outcome.iResult > iSummaryResult)
		{
		iSummaryResult = outcome.iResult;
		}
	
	if (iResponses.Count() < iRequests.Count())
		{
		SendRequest();
		}
	else
		{
		iState = EHaveResult;
		User::RequestComplete(iClientStatus, KErrNone);	
		}
	}

void COCSPClient::SendRequest()
	{
	TRAPD(error, DoSendRequestL());

	// Handle errors in RunL
	if (error != KErrNone)
		{
		TRequestStatus* status = &iStatus;
		User::RequestComplete(status, error);
		}

	iState = ESendingRequest;
	SetActive();
	}

void COCSPClient::DoSendRequestL()
	{
	// Determine the next request to send by the number of responses received
	TInt index = iResponses.Count();
	COCSPRequest& request = *(iRequests[index]);
	
	TDesC8* uri = NULL;
	TRAPD(error, uri = OCSPUtils::ServerUriL(request.CertInfo(0).Subject(),iParams));
	
	if(error == KErrArgument)
		{
		TRequestStatus* status = &iStatus;
		User::RequestComplete(status, OCSP::ENoServerSpecified);
		return;
		}
	
	User::LeaveIfError(error);
	CleanupStack::PushL(uri);
	
	// if state is valid it means that uri has been retrieved.
	__ASSERT_ALWAYS(uri != NULL, Panic(OCSP::EInvalidURI));
	MOCSPTransport& transport = *iParams->Transport();
	
	delete iTransaction;
	iTransaction = NULL;
	iTransaction = COCSPTransaction::NewL(*uri, transport, iParams->RetryCount(), iParams->Timeout());
	iTransaction->SendRequest(request, iStatus);
	CleanupStack::PopAndDestroy(uri);		
	}

/**
 * Each response received has to undergo validation based on RFC 2560 guidelines.
 */
void COCSPClient::ValidateResponseL()
	{
	TInt index = iResponses.Count() - 1;

	User::LeaveIfError(iOutcomes.Append(TOCSPOutcome()));
	__ASSERT_ALWAYS(iOutcomes.Count() == iResponses.Count(), Panic(KErrCorrupt));

	iState = EValidatingResponse;
	iValidator->Validate(*(iRequests[index]), *(iResponses[index]), iOutcomes[index], iStatus);
	SetActive();
	}
