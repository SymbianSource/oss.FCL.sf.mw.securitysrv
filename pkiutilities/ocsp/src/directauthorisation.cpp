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
// Define methods for authorisation scheme based on locally configured
// authoritive root certs
// 
//

#include "ocsp.h"
#include "panic.h"
#include <pkixcertchain.h>
#include <asn1dec.h>
#include <x509keys.h>
#include <x509cert.h>
#include <mcertstore.h>
#include <ccertattributefilter.h>
#include "ocsprequestandresponse.h"

EXPORT_C COCSPDirectAuthorisationScheme* COCSPDirectAuthorisationScheme::NewLC(const TUid& aCertStoreUid, MCertStore& aCertStore)
	{
	COCSPDirectAuthorisationScheme* self = new(ELeave) COCSPDirectAuthorisationScheme(aCertStoreUid, aCertStore);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

COCSPDirectAuthorisationScheme::COCSPDirectAuthorisationScheme(const TUid& aCertStoreUid, MCertStore& aCertStore) :
	CActive(CActive::EPriorityStandard),
	iCertStoreUid(aCertStoreUid),
	iCertStore(aCertStore)
	{
	CActiveScheduler::Add(this);
	}


COCSPDirectAuthorisationScheme::~COCSPDirectAuthorisationScheme()
	{
	Cancel();
	delete iCertChain;
	delete iPKIXResult; 
	delete iCertFilter;
	delete iEncodedCertBuf;
	delete iEncodedCertPtr;
	iFs.Close();
	iCertStoreEntries.Close();
	delete iResponseCert;
	}


void COCSPDirectAuthorisationScheme::ConstructL()
	{
	iPKIXResult = CPKIXValidationResultBase::NewL();
	
	User::LeaveIfError(iFs.Connect());
	
	iCertFilter = CCertAttributeFilter::NewL();
	iCertFilter->SetOwnerType(ECACertificate);
	iCertFilter->SetUid(iCertStoreUid);
	iCertFilter->SetFormat(EX509Certificate);
	}

/**
 * Starts the process of validating the response.
 * The initial value of the OCSP Status is set to signature validation failure.
 */
void COCSPDirectAuthorisationScheme::ValidateL(OCSP::TStatus& aOCSPStatus,
	COCSPResponse& aResponse, const TTime aValidationTime, TRequestStatus& aStatus,
	const COCSPRequest& aRequest)
	{
	iValidationTime = aValidationTime;

	iClientStatus = &aStatus;
	*iClientStatus = KRequestPending;

	iResponse = &aResponse;
	iRequest = &aRequest;
	
	iOCSPStatus = &aOCSPStatus;
	*iOCSPStatus = OCSP::EResponseSignatureValidationFailure;
	
	// this object needs to be deleted if the same COCSPDirectAuthorisationScheme
	// object is used for validating more than one certificate via COCSPValidator
	delete iCertChain;
	iCertChain = NULL;
		
	StartValidateL();
	}


void COCSPDirectAuthorisationScheme::ValidateFromRoots()
//
// Certs for the chain were not included in the response.
// See if any of the root certs for the given Uid can do the job.
//
	{
	iCertStore.List(iCertStoreEntries, *iCertFilter, iStatus);
	iState = EListCertEntries;
	}

/**
 * If response contains responder's certificate, chain validation would be done till the CA 
 * certificate which should be located in the store.
 */
void COCSPDirectAuthorisationScheme::ValidateCertChainL()
	{
	__ASSERT_DEBUG(iCertChain, Panic(KErrCorrupt));

	iCertChain->ValidateL(*iPKIXResult, iValidationTime, iStatus);
	iState = EValidateCertChain;
	}

TBool COCSPDirectAuthorisationScheme::ValidateSignatureL()
	{
	__ASSERT_DEBUG(iCertChain, Panic(KErrCorrupt));

	// Validate against first cert in chain - must exist
	const CX509Certificate& eeCert = iCertChain->Cert(0);

	return	OCSPUtils::DoesResponderIdMatchCertL(*iResponse, eeCert)
		&&	OCSPUtils::IsResponseSignedByCertL(iResponse, eeCert);
	}

void COCSPDirectAuthorisationScheme::OnValidateCertChainL()
// Called when cert chain validation completes
	{
	if (iPKIXResult->Error().iReason == EValidatedOK)
		{
		if (ValidateSignatureL())
			{
			delete iResponseCert;
			iResponseCert = NULL;
			iResponseCert = CX509Certificate::NewL(iCertChain->Cert(0));
			*iOCSPStatus = OCSP::EValid;
			}
		}
	User::RequestComplete(iClientStatus, KErrNone);
	}
		

void COCSPDirectAuthorisationScheme::OnListCertEntries()
// Called when listing cert store entries completes
	{
	// trigger transitory state to start getting entries
	iCurEntry = -1;
	iState = ERetrieveNext;
	TRequestStatus* status = &iStatus;
	User::RequestComplete(status, KErrNone);
	SetActive();
	}


void COCSPDirectAuthorisationScheme::OnRetrieveNextL()
// Transitory state to retrieve next entry triggered
	{
	delete iEncodedCertBuf;
	iEncodedCertBuf = 0;
	delete iEncodedCertPtr;
	iEncodedCertPtr = 0;
	if (++iCurEntry < iCertStoreEntries.Count())
	// still more entries, get next one
		{
		CCTCertInfo* certInfo = iCertStoreEntries[iCurEntry];
		ASSERT(certInfo);

		iEncodedCertBuf = HBufC8::NewL(certInfo->Size());
		iEncodedCertPtr = new(ELeave) TPtr8(iEncodedCertBuf->Des());

		iCertStore.Retrieve(*certInfo, *iEncodedCertPtr, iStatus);
		
		iState = ERetrievingEntry;
		
		SetActive();
		}
	else
	// no more entries, bad news
		{
		User::RequestComplete(iClientStatus, KErrNone);
		}
	}

void COCSPDirectAuthorisationScheme::OnRetrievingEntryL()
// Called when retrieving an entry completes.
	{
	CX509Certificate* cert = CX509Certificate::NewLC(*iEncodedCertPtr);
	
	if (	OCSPUtils::DoesResponderIdMatchCertL(*iResponse, *cert)
		&&	OCSPUtils::IsResponseSignedByCertL(iResponse, *cert)
		&&	cert->ValidityPeriod().Valid(iValidationTime))
	// found a good one, job finished
		{
		delete iResponseCert;
		iResponseCert = NULL;
		CleanupStack::Pop(cert);
		iResponseCert = cert;
		*iOCSPStatus = OCSP::EValid;
		User::RequestComplete(iClientStatus, KErrNone);
		}
	else
	// wasn't good, try next one
		{
		iState = ERetrieveNext;
		TRequestStatus* status = &iStatus;
		User::RequestComplete(status, KErrNone);
		SetActive();
		CleanupStack::PopAndDestroy(cert);
		}
	
	}

void COCSPDirectAuthorisationScheme::RunL()
//
// Handles transitions from one state to the next
//
	{
	User::LeaveIfError(iStatus.Int());

	switch (iState)
		{
	case EValidateCertChain:
		OnValidateCertChainL();
		break;
	case EListCertEntries:
		OnListCertEntries();
		break;
	case ERetrieveNext:
		OnRetrieveNextL();
		break;
	case ERetrievingEntry:
		OnRetrievingEntryL();
		break;
	
	default:
		ASSERT(FALSE);
		}
	}

void COCSPDirectAuthorisationScheme::DoCancel()
//
// Cancel
//
	{
	switch (iState)
		{
	case EValidateCertChain:
		ASSERT(iCertChain);
		iCertChain->CancelValidate();
		break;
	case EListCertEntries:
		iCertStore.CancelList();
		break;
	case ERetrieveNext:
		// nothing to do, this is a transitory state
		break;
	case ERetrievingEntry:
		iCertStore.CancelRetrieve();
		break;
	default:
		ASSERT(FALSE);
		}
	if (iClientStatus)
		{
		User::RequestComplete(iClientStatus, KErrCancel);
		}
	}


TInt COCSPDirectAuthorisationScheme::RunError(TInt aError)
	{
	if(aError == KErrArgument)
		{
		User::RequestComplete(iClientStatus, KErrNone);
		}
	else
		{
		User::RequestComplete(iClientStatus, aError);
		}
	return KErrNone;
	}

/**
 * If the response contains the certificate which signed the response, create the certificate chain
 * till the responder certificate. Otherwise validate directly from root certificates contained 
 * in the store initialized by the client.
 */
void COCSPDirectAuthorisationScheme::StartValidateL()
	{
	const TPtrC8* certChainData = iResponse->DataElementEncoding(COCSPResponse::ECertificateChain);
	
	// Cert chain data was included in response
	if (certChainData)
		{
		iCertChain = CPKIXCertChainBase::NewL(iCertStore, *certChainData, iCertStoreUid);
		ValidateCertChainL();
		}
	else
		{
		// No cert chain in response - validate directly from root certs
		ValidateFromRoots();
		}
	SetActive();
	}

void COCSPDirectAuthorisationScheme::CancelValidate()
	{
	CActive::Cancel();
	}

const CX509Certificate* COCSPDirectAuthorisationScheme::ResponderCert() const	
	{
	return iResponseCert;
	}
