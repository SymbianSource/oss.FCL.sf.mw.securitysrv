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
// Define methods for validating a response.
// 
//

#include "validator.h"
#include "ocsp.h"
#include "panic.h"
#include "transaction.h"
#include <x509cert.h>

// We allow a certain amount of leeway when checking times.  This specifies the
// default value.
const TInt KDefaultLeewaySeconds = 5 * 60; // 5 minutes

// The spec says we must check that the thisUpdate field is "sufficiently
// recent".  This specifies the default value for the maximum age we tolerate
// (in seconds).
const TInt KDefaultMaxStatusAge = 30 * 24 * 60 * 60; // 30 days

COCSPValidator* COCSPValidator::NewL( const COCSPParameters& aParameters)
	{
	COCSPValidator* self = new (ELeave) COCSPValidator(aParameters);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	CActiveScheduler::Add(self);
	return self;
	}


COCSPValidator::~COCSPValidator()
	{
	Cancel();

	iAuthorisationScheme.Close();
	iRequestIndex.Close();
	
	delete iValidationTime;
	delete iResponderCertRequest;
	delete iResponderCertResponse;
	delete iTransaction;
	}


COCSPValidator::COCSPValidator( const COCSPParameters& aParameters) :
	CActive(CActive::EPriorityStandard),
	iMaxStatusAge(KDefaultMaxStatusAge),
	iLeewaySeconds(KDefaultLeewaySeconds),
	iResponderCertCheck(EFalse),
	iUseNonce(ETrue),
	iParameters(&aParameters)
	{}

void COCSPValidator::ConstructL() 
	{
	for (TUint j = 0 ; j < iParameters->AuthSchemeCount() ; ++j)
		{
		User::LeaveIfError(iAuthorisationScheme.Append(&iParameters->AuthScheme(j)));
		}
	if (iParameters->ValidationTime())
		{
		iValidationTime = new (ELeave) TTime(*iParameters->ValidationTime());
		}
	if (iParameters->MaxStatusAge())
		{
		iMaxStatusAge = *iParameters->MaxStatusAge();
		}
	if (iParameters->TimeLeeway())
		{
		iLeewaySeconds = *iParameters->TimeLeeway();
		}
	iResponderCertCheck = iParameters->ReponderCertCheck();
	iUseNonce = iParameters->UseNonce();
	}

void COCSPValidator::Validate(const COCSPRequest& aRequest, COCSPResponse& aResponse,
									   TOCSPOutcome& aOutcome, TRequestStatus& aStatus)
	{
    TRAPD(err, DoValidateL(aRequest, aResponse, aOutcome, aStatus));
    
    if (err != KErrNone)
        {
        TRequestStatus* status = &aStatus;
        User::RequestComplete(status, err);
        }
	}

void COCSPValidator::DoValidateL(const COCSPRequest& aRequest, COCSPResponse& aResponse,
								 TOCSPOutcome& aOutcome, TRequestStatus& aStatus)
	{
	iRequest = &aRequest;
	iResponse = &aResponse;

	iValidationStatus = &aStatus;
	aStatus = KRequestPending;

	iOutcome = &aOutcome;
	iOutcome->iStatus = OCSP::EClientInternalError;
	// this has been set to EUnknown at client side, but still making sure that this 
	// value is being used.
	iOutcome->iResult = OCSP::EUnknown;
	
	if ( !IsResponseWellFormed())
		{
		User::RequestComplete(iValidationStatus, KErrNone);
		return;
		}
	
	// points to the current scheme being used for validation of the certificate
	// in question.
	iIndexScheme = -1;
	ProcessSchemeValidationL();
	}

TBool COCSPValidator::IsResponseWellFormed()
	{
	// Check the certificates in the response were indeed those we asked for
	// Make lookup table indexing request/response while we're at it

	TInt numResponseCerts = iResponse->CertCount();
	TInt numRequestCerts = iRequest->CertCount();

	if (numRequestCerts < numResponseCerts)
		{
		iOutcome->iStatus = OCSP::EMalformedResponse;
		return EFalse;
		}
	else if (numRequestCerts > numResponseCerts)
		{
		iOutcome->iStatus = OCSP::EMissingCertificates;
		return EFalse;
		}
	
	// Check each cert to verify that each request has a corresponding response present.
	// In process, set up array giving the position in the request of each cert in the response
	iRequestIndex.Reset();
		
	for (TInt requestIndex = 0; requestIndex < numRequestCerts; ++requestIndex)
		{
		// This is what we're after
		const COCSPCertID& requestCertID = iRequest->CertInfo(requestIndex).CertID();

		// This is where it is in the response
		TInt responseIndex = iResponse->Find(requestCertID);

		if (responseIndex < 0)
			{
			iOutcome->iStatus = OCSP::EMissingCertificates;
			return EFalse;
			}
		iRequestIndex.Append(responseIndex);
		}
	// All found
	return ETrue;
	}

void COCSPValidator::ProcessSchemeValidationL()
	{
	TInt count = iAuthorisationScheme.Count();
	__ASSERT_ALWAYS(count, Panic(KErrNoAuthorisationSchemes));
	if (++iIndexScheme < count)
		{
		iSchemeInUse = iAuthorisationScheme[iIndexScheme];
		TTime validationTime = ValidationTime();
		iSchemeInUse->ValidateL(iOutcome->iStatus, *iResponse, validationTime, iStatus, *iRequest);
		iState = EWaitingResponse;
		SetActive();
		}
	else
		{
		User::RequestComplete(iValidationStatus, KErrNone);
		}
	}

// Get status of least trusted cert
void COCSPValidator::FinalResponseValidationL()
	{
	// Do nonce last so can still trust rest of validation if nonce is missing.
	if(ValidateTimeL())
		{
		ValidateNonce();
		}
	
	if (iOutcome->iStatus == OCSP::EMissingNonce || 
		iOutcome->iStatus == OCSP::EValid )
		{
		iOutcome->iResult = CheckOCSPStatus(iResponse);
		}
	else
		{
		// If the response is not valid, result is always unknown
		iOutcome->iResult = OCSP::EUnknown;
		}
	
	if(iResponderCertCheck)
		{
		iResponderCert = iSchemeInUse->ResponderCert();
	 	
	 	if(iResponderCert != NULL)
	 		{
	 		iIssuerCert = &iRequest->CertInfo(0).Issuer();
	 		SendResponderCertL();
	 		}
	 	else
			{
			User::RequestComplete(iValidationStatus, KErrNone);
			}
		}
	else
		{
		User::RequestComplete(iValidationStatus, KErrNone);
		}
	}

TBool COCSPValidator::ValidateTimeL()
	{
	const TTime validationTime = ValidationTime();
	const TTime producedAt = iResponse->ProducedAt();
	
	// For each certificate request, do the following:
	// 1. Check thisUpdate
	// 2. Check producedAt 
	TInt numCerts = iRequest->CertCount();
	for (TInt requestIndex = 0; requestIndex < numCerts; ++requestIndex)
		{
		const COCSPResponseCertInfo& responseCertInfo = iResponse->CertInfo(iRequestIndex[requestIndex]);
		const TTime thisUpdate = responseCertInfo.ThisUpdate();
		const TTime* nextUpdate = responseCertInfo.NextUpdate();

		// Check validity interval of response includes validation time
		// and producedAt time (if different).  Give iLeewaySeconds second's lee-way.

		// 4.2.2.1 "Responses whose thisUpdate time is later than the local
		// system time SHOULD be considered unreliable"
		if (TimeIsBeforeL(validationTime, thisUpdate))
			{
			iOutcome->iStatus = OCSP::EThisUpdateTooLate;
			return EFalse;
			}

		// Check producedAt later than thisUpdate.  This is not mandated by the spec.
		if (TimeIsBeforeL(producedAt, thisUpdate))
			{
			iOutcome->iStatus = OCSP::EThisUpdateTooLate;
			return EFalse;
			}
		
		if (nextUpdate)
			{
			// 4.2.2.1 "Responses whose nextUpdate value is earlier than the
			// local system time value SHOULD be considered unreliable"
			// 3.2.6 "OCSP clients shall confirm that ... nextUpdate is greater
			// than the current time."
			if (TimeIsBeforeL(*nextUpdate, validationTime))
				{
				iOutcome->iStatus = OCSP::ENextUpdateTooEarly;
				return EFalse;
				}

			// Check nextUpdate later than producedAt.  This is not mandated by the spec.
			if (TimeIsBeforeL(*nextUpdate, producedAt))
				{
				iOutcome->iStatus = OCSP::ENextUpdateTooEarly;
				return EFalse;
				}
			}
		
		// 3.2.5 "OCSP clients SHALL confirm that ... thisUpdate is sufficiently
		// recent"
		if (iMaxStatusAge)
			{
			TTimeIntervalSeconds difference;

			User::LeaveIfError(validationTime.SecondsFrom(thisUpdate, difference));
			const TTimeIntervalSeconds maxUpdateAge(iMaxStatusAge + iLeewaySeconds);	
			if (difference > maxUpdateAge)
				{
				iOutcome->iStatus = OCSP::EThisUpdateTooEarly;
				return EFalse;
				}	
			}

		// Check certificate validity period against validation time.
		// 
		// Strictly speaking, the OCSP protcol is about checking revocation
		// rather then checking whether a certificate has just expired.
		// However, it's difficult to check this on a device when you don't have
		// an accurate value for the current time.  We do the check here for
		// completeness, and trust the time given to us by the ocsp server.  If
		// we are using a nonce, as we will be most of the time, we can
		// guarantee that the producedAt time is current.

		const CX509Certificate& cert = iRequest->CertInfo(requestIndex).Subject();
		const CValidityPeriod& validityPeriod = cert.ValidityPeriod();

		if (!validityPeriod.Valid(validationTime))
			{
			iOutcome->iStatus = OCSP::ECertificateNotValidAtValidationTime;
			return EFalse;
			}

		} // Continue with next cert

	// If we've got this far, we're fine
	return ETrue;
	}

TBool COCSPValidator::ValidateNonce()
	{
	const TDesC8* requestNonce = iRequest->Nonce();
	const TPtrC8* responseNonce = iResponse->DataElementEncoding(COCSPResponse::ENonce);
	
	if (requestNonce)
		{
		if (responseNonce)
			{
			if (*requestNonce == *responseNonce)
				{
				return ETrue;
				}
			else
				{
				iOutcome->iStatus = OCSP::ENonceMismatch;
				return EFalse;
				}
			}
		else
			{
			iOutcome->iStatus = OCSP::EMissingNonce;
			return EFalse;
			}
		}
	else
		{
		if (responseNonce)
			{
			// Shouldn't have a nonce!
			iOutcome->iStatus = OCSP::EMalformedResponse;
			return EFalse;
			}
		else
			{
			// No nonces - fine
			return ETrue;
			}
		}
	}

// Return true if first argument is iLeewaySeconds or more before the second
// argument.  Hence it is conservative, and should be always used "positively"
// to check for error conditions.
TBool COCSPValidator::TimeIsBeforeL(const TTime& aBefore, const TTime& aAfter)
	{
	TTimeIntervalSeconds difference;
	const TTimeIntervalSeconds leeway(iLeewaySeconds);	

	User::LeaveIfError(aAfter.SecondsFrom(aBefore, difference));
	return (difference > leeway);
	}

void COCSPValidator::RunL()
	{
	User::LeaveIfError(iStatus.Int());

	switch (iState)
		{
	case EWaitingResponse:
		CheckSchemeValidationL();
		break;
	case EValidating:
		ProcessSchemeValidationL();
		break;
	case EValidateResponderCert:
		ValidateResponderCertL();
		break;
	default:
		ASSERT(FALSE);
		}
	}

void COCSPValidator::DoCancel()
	{
	TInt count = iAuthorisationScheme.Count();
	__ASSERT_ALWAYS(count, Panic(KErrNoAuthorisationSchemes));
	if (iState == EWaitingResponse)
		{
		ASSERT(iSchemeInUse != NULL);
		iSchemeInUse->CancelValidate();
		}
	User::RequestComplete(iValidationStatus, KErrCancel);
	}

TInt COCSPValidator::RunError(TInt aError)
	{
	User::RequestComplete(iValidationStatus, aError);
	return KErrNone;
	}

void COCSPValidator::CheckSchemeValidationL()
	{
	// If any scheme says it's OK, we're happy, otherwise we'll return
	// with whatever the last scheme said.
	if (iOutcome->iStatus == OCSP::EValid)
		{
		FinalResponseValidationL();
		}
	else
		{
		iState = EValidating;
		// Fire off AO
		TRequestStatus* status = &iStatus;
		User::RequestComplete(status, KErrNone);
		SetActive();
		}
	}

TTime COCSPValidator::ValidationTime() const
	{
	__ASSERT_ALWAYS(iResponse, Panic(KErrNotReady));
	if (iValidationTime)
		{
		return *iValidationTime;
		}
	else
		{
		TTime gmt;
		
		// if secure time is not available then fall back to the insecure version.
		if(gmt.UniversalTimeSecure() == KErrNoSecureTime)
			{
			gmt.UniversalTime();
			}
		return gmt;
		}
	}

/**
 * For the response in question there can be more than one authentication scheme initialized.
 * We need to find out whether the schemes initialized contain at least delegate or direct auth scheme,
 * if any of them is present we can send the request for validation for responder certificate, as 
 * validation of responder certificate should only work for these 2 schemes.
 * 
 * If we get a valid scheme, following would be the sequence of operation:
 * 1. Retrieve the responder certificate and the issuer(should be the CA who issued the certificate
 * in question) who has issued the responder certificate.
 * 2. Check whether the responder certificate contains the id-pkix-ocsp-nocheck, if present there is no need for 
 * 	 sending it for OCSP check, if not present send it for OCSP check.
 
 * Send the responder certificate for OCSP checking. Here we would use the existing parameters
 * for creating the responder certificate request, as this check is an extension of the original 
 * certificate OCSP check.
 */
 void COCSPValidator::SendResponderCertL()
 	{
 	if( OCSPUtils::DoesCertHaveOCSPNoCheckExt(*iResponderCert))
 		{
 		User::RequestComplete(iValidationStatus, KErrNone);
 		return;
 		}
 	
 	iResponderCertRequest = COCSPRequest::NewL(iUseNonce);
 	iResponderCertRequest->AddCertificateL(*iResponderCert, *iIssuerCert);
 	 	
 	// Only add further requests if there is:
 	// a URI (either AIA URI or default URI) 
 	TDesC8* uri = NULL;
 	TRAPD(error, uri = OCSPUtils::ServerUriL(iResponderCertRequest->CertInfo(0).Subject(),iParameters));
 
	if(error == KErrArgument)
 		{
		iOutcome->iStatus = OCSP::ENoServerSpecified;
		TRequestStatus* status = &iStatus;
		User::RequestComplete(status, OCSP::ENoServerSpecified);
		iState = EValidateResponderCert;
	 	SetActive();
	 	return;
 		}
	
	User::LeaveIfError(error);
	CleanupStack::PushL(uri);
 	
 	// if state is valid it means that uri has been retrieved.
	__ASSERT_ALWAYS(uri != NULL, Panic(OCSP::EInvalidURI));
	MOCSPTransport& transport = *iParameters->Transport();
	delete iTransaction;
	iTransaction = NULL;
	iTransaction = COCSPTransaction::NewL(*uri, transport, iParameters->RetryCount(), iParameters->Timeout());
	iTransaction->SendRequest(*iResponderCertRequest, iStatus);
	CleanupStack::PopAndDestroy(uri);
	iState = EValidateResponderCert;
 	SetActive();
 	}

 /**
  * Receive the response for responder certificate OCSP check.
  * Leave if there is any problem with the received response.
  * If the response is well formed then send it for further validation.
  */
 void COCSPValidator::ValidateResponderCertL()
 	{
 	TInt status = iStatus.Int();
 	
 	if (status == KErrNone)
 		{
 		iResponderCertResponse = iTransaction->TakeResponse();
 		}
 	else if (status == OCSP::KErrTransportFailure)
 		{
 		User::Leave(OCSP::ETransportError);
 		}
 	else if (status == OCSP::KErrInvalidURI)
 		{
 		User::Leave(OCSP::EInvalidURI);
 		}
 	else
 		{
 		User::Leave(status);		   
 		}
	
 	iOutcome->iResult = CheckOCSPStatus(iResponderCertResponse);
 	if(iOutcome->iResult != OCSP::EGood )
 		{
 		// as the responder certificate is either revoked or unknown the final status returned
 		// should be unknown.
 		iOutcome->iResult = OCSP::EUnknown;
 		}
 	User::RequestComplete(iValidationStatus, KErrNone);
 	
 	}

 OCSP::TResult COCSPValidator::CheckOCSPStatus(const COCSPResponse* aResponse) const
	{
	OCSP::TResult result = OCSP::EGood;
	TInt numCerts = aResponse->CertCount();
	for (TInt index = 0; index < numCerts; ++index)
		{
		const COCSPResponseCertInfo& info = aResponse->CertInfo(index);
	
		OCSP::TResult certStatus = info.Status();
		result = certStatus > result? certStatus : result;
		}
	return result;
	}
