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
//

#include <asn1dec.h>
#include <x509certext.h>
#include "oids.h"
#include "pkixcertchain.h"
#include "ocsp.h"
#include <mcertstore.h>
#include <ccertattributefilter.h>
#include "ocsprequestandresponse.h"

const TInt CERT_SIZE = 1000;

EXPORT_C COCSPDelegateAuthorisationScheme* COCSPDelegateAuthorisationScheme::NewLC(
	MCertStore& aCertStore)
/**
	Factory function allocates new instance of
	COCSPDelegateAuthorisationScheme.
	
	@param	aCertStore		Cert store interface.  This is used to
							construct a certificate chain builder with
							CPKIXCertChainBase::NewL().  Certificates from
							the store are not used for validation if the response 
							contains the responder certificate. If it does not contain 
							the responder cert then validation would be done from store.
	@return					New instance of COCSPDelegateAuthorisationScheme.
							Leaves if cannot successfully construct.  The new
							object is placed on the cleanup stack.
 */
	{
	COCSPDelegateAuthorisationScheme* scheme =
		new(ELeave) COCSPDelegateAuthorisationScheme(aCertStore);
	CleanupStack::PushL(scheme);
	scheme->ConstructL();
	
	return scheme;
	}

COCSPDelegateAuthorisationScheme::COCSPDelegateAuthorisationScheme(
	MCertStore& aCertStore)
/**
	Initializes the CActive base class object and adds
	this object to the active scheduler.
	
	@param	aCertStore		Cert store interface.  This is only used to
							construct a certificate chain builder with
							CPKIXCertChainBase::NewL().  Certificates from
							the store are not used for validation if the response 
							contains the responder certificate. If it does not contain 
							the responder cert then validation would be done from store.
 */
:	CActive(EPriorityStandard),
	iCertStore(aCertStore)
	{
	CActiveScheduler::Add(this);
	}

void COCSPDelegateAuthorisationScheme::ConstructL()
/**
	Strictly, none of the resources owned by this object
	are required for all of its lifetime need to be preserved
	between validations.
	
	Some of the resources, such as the CPKIXCertChain
	instance have to be reallocated for every validation.
	
	The resources which can be allocated for the lifetime
	of this object are allocated here.  This improves performance
	and simplifies the validation process, at a RAM cost.
 */
	{
	// iRespSignCertChainBase is the array of intermediate
	// certificates which CPKIXCertChain will use to
	// chain (T->I) to (I->E).  Because only immediate
	// delegates are supported, there will only be one
	// intermediate (or trusted root from CPKIXCertChain's
	// point-of-view.)  Therefore the array can be
	// sized here.
	
	// RPointerArray<> doesn't have a Reserve() function.
	const CX509Certificate* nullCert = 0;
	User::LeaveIfError(iRespSignIntCert.Append(nullCert));
	
	iCertFilter = CCertAttributeFilter::NewL();
	iCertFilter->SetOwnerType(ECACertificate);
	iCertFilter->SetFormat(EX509Certificate);

	}

COCSPDelegateAuthorisationScheme::~COCSPDelegateAuthorisationScheme()
/**
	Cancels any outstanding validation and frees all resources
	owned by this object.
 */
	{
	delete iCertFilter;
	iCertStoreEntries.Close();
	iRespSignIntCert.Close();
	
	delete iEncodedCert;
	delete iResponseCert;
	
	delete iPKIXResultBase;
	delete iRespSignCertChainBase;
	
	}

/**
	Implement MOCSPAuthorisationScheme.
	
	Validate the response if it is signed by an	immediate delegate of the intermediate entity.
	I.e. if the request has the form
		(T->I) (I->E)
	
	where T is trusted (at least for the purposes of this validation) and I is an intermediate, 
	the response can be signed by R if (I->R).
	
	I is the CA, and can be equal to T, i.e. the certificate which is being tested for 
	revocation can be signed by a root certificate.
	
	R must be immediately signed by I, and must have id-kp-OCSPSigning in its extended key usage.
	(RFC 2560 S4.2.2.2)
 */
void COCSPDelegateAuthorisationScheme::ValidateL(
	OCSP::TStatus& aOCSPStatus, COCSPResponse& aResponse,
	const TTime aValidationTime, TRequestStatus& aStatus,
	const COCSPRequest& aRequest)
	{
	// store the client status, so that it can be used for request
	// completion later.
	iClientStatus = &aStatus;
	aStatus = KRequestPending;
	
	// By assuming there is only one request / response pair,
	// there is no need to iterate through each response, which
	// simplifies this scheme's implementation.
	// the OCSP requests are constructed in
	// COCSPClient::ConstructL() such that exactly one certificate
	// and its signer are sent in each request.
	// (RFC 2560 S4.1.1 allows multiple pairs)
	
	iResponse = &aResponse;
	iRequest = &aRequest;
	
	iValidationTime = aValidationTime;
	
	// This default value is changed once the whole validation 
	// process has completed successfully else the default value 
	// is returned on failure.
	aOCSPStatus = OCSP::EResponseSignatureValidationFailure;
	iOCSPStatus = &aOCSPStatus;
	
	// set this to false before starting the scheme validation
	// for multiple certificates are being validated through 
	// this scheme.
	iValidateFromResponse = EFalse;
	
	// This authentication scheme supports 2 ways of validation:
	// 1. If the response contains responder cert then validation 
	// would be performed against the responder cert included in response. 
	// This validation would also check whether the responder cert has been 
	// issued by the CA cert which issued the certificate in question.
	// 2. If the response does not contain responder cert, search for it 
	// in the store based on the ResponderId which is included in the 
	// response. If the responder cert is found in the store which has signed
	// the response then authorize the response, in both the cases we check whether 
	// responder cert has been issued by the CA which issued the cert in question, 
	// and the CA should be present in the store.
	const TPtrC8* certChainData = aResponse.DataElementEncoding(COCSPResponse::ECertificateChain);
	if (certChainData == 0)		// no signing certs
		{
		ValidateFromRootsL();
		}
	else
		{
		iValidateFromResponse = ETrue;
		ValidateDelegateCertL(*certChainData, iValidationTime);
		}
	}

/**
	Initialize this object to validate the certificate which was sent with the 
	response against the CA which was used	to sign the certificate in question.
	
	@param	aResponseCertChain	DER-encoded cert chain that	was either sent with the response.
								or was retrieved from the store as a single certificate.
	@param 	aValidationTime		Time to be used for chain validation of the delegate certificate.
	@post If successful, asynchronous validation will be set up.
 */

void COCSPDelegateAuthorisationScheme::ValidateDelegateCertL(
	const TDesC8& aResponseCertChain, const TTime aValidationTime)
	{
	// the response received can contain a chain of certificate, we need to extract the 
	// responder certificate from the chain for further processing. If the certificate has been
	// retrieved from store then there would be no chain but the delegate certificate would be
	// retrieved.
	CX509Certificate* decodedResponseCert = OCSPUtils::GetResponderCertLC(aResponseCertChain);
	CleanupStack::Pop(decodedResponseCert);
	
	delete iResponseCert;
	iResponseCert = NULL;
	iResponseCert = decodedResponseCert;
	
	// First check the responder certificate in accordance to RFC 2560 for the following:
	// 1. Does it contain extension id-kp-OCSPSigning
	// 2. The responder id in the response matches the response certificate.
	// 3. The response is signed by the responder certificate
	if( OCSPUtils::DoesCertHaveOCSPSigningExtL(*iResponseCert) 
		&&	OCSPUtils::DoesResponderIdMatchCertL(*iResponse, *iResponseCert)
		&&	OCSPUtils::IsResponseSignedByCertL(iResponse, *iResponseCert) )
		{
		// construct a certificate chain containing the X -> R with T -> I as the intermediate.
		delete iPKIXResultBase;
		iPKIXResultBase = NULL;
		iPKIXResultBase = CPKIXValidationResultBase::NewL();
		
		// get the intermediate which signed the EE
		const CX509Certificate& caCert = iRequest->CertInfo(0).Issuer();
		
		// use the intermediate cert as the trusted root for
		// the purpose of building the chain.  ("Intermediate"
		// in this context is the certificate which signed the EE.)
		iRespSignIntCert[0] = CONST_CAST(CX509Certificate*,&caCert);
		
		delete iRespSignCertChainBase;
		iRespSignCertChainBase = NULL;
		iRespSignCertChainBase = CPKIXCertChainBase::NewL(iCertStore, iResponseCert->Encoding(), iRespSignIntCert);
		
		// attempt to validate the chain.  I.e. test that X = I.
		iRespSignCertChainBase->ValidateL(*iPKIXResultBase, aValidationTime, iStatus);
		
		iState = EOnChainValidation;
		SetActive();
		}
	else
		{
		if(iValidateFromResponse)
			{
			User::RequestComplete(iClientStatus, KErrNone);
			return;
			}
		// this means that we are trying to retrieve the responder certificate from the store
		// and perform validation against it. As this certificate is not the valid responder
		// hence set the state to retrieve the next certificate from the store.
		else
			{
			iState = ERetrieveNext;
			TRequestStatus* status = &iStatus;
			User::RequestComplete(status,KErrNone);
			SetActive();
			return;
			}
		}
	}

void COCSPDelegateAuthorisationScheme::CancelValidate()
/**
	Implement MOCSPAuthorisationScheme.  This is an active
	object, and this function just calls Cancel().  See
	DoCancel() for information about the cancellation process.
	
	@see DoCancel
 */
	{
	ASSERT(iRespSignCertChainBase != 0);
	iRespSignCertChainBase->CancelValidate();
	}

void COCSPDelegateAuthorisationScheme::RunL()
	{
	User::LeaveIfError(iStatus.Int());
	switch(iState)
		{
		//Response validation after chain building.
		case EOnChainValidation:
			OnChainValidationL();
			break;
		
		// state used to allocate sufficient memory for retrieving the next certificate
		case ERetrieveNext:
			OnRetrieveNextL();
			break;

		// state used to retrieve the next certificate
		case ERetrievingEntry:
			OnRetrievingEntryL();
			break;
		}
	}

void COCSPDelegateAuthorisationScheme::OnChainValidationL()
	{	
	TValidationError error = iPKIXResultBase->Error().iReason;
	if (error != EValidatedOK)
		{
		User::Leave(error);
		}
	*iOCSPStatus = OCSP::EValid;
	User::RequestComplete(iClientStatus, KErrNone);
	}

void COCSPDelegateAuthorisationScheme::DoCancel()
/**
	If a validation request is outstanding, then it is cancelled.
	This objects client, i.e. the owner of the TRequestStatus which
	was passed to Validate(), is completed with KErrCancel.
 */
	{
	// object should be waiting for the chain builder to complete.  
	// Therefore there must be a valid client request status pointer 
	// and an instance of CPKIXCertChain.
	
	iRespSignCertChainBase->CancelValidate();
	
	if (iClientStatus)
		{		
		User::RequestComplete(iClientStatus, KErrCancel);
		}
	}

TInt COCSPDelegateAuthorisationScheme::RunError(TInt aError)
	{
	User::RequestComplete(iClientStatus, aError);
	return KErrNone;
	}

/**
 * Initiates request to retrieve the responder certificate from store.
 */
void COCSPDelegateAuthorisationScheme::ValidateFromRootsL()
	{
	iCertCount = -1;
	
	iCertStoreEntries.Close();
	iCertStore.List(iCertStoreEntries, *iCertFilter, iStatus);
	
	delete iEncodedCert;
	iEncodedCert = NULL;
	iEncodedCert = HBufC8::NewL(CERT_SIZE);
	
	iState = ERetrieveNext;
	SetActive();
	}

/**
 * For list of certificate entries in the store retrieve each certificate.
 */
void COCSPDelegateAuthorisationScheme::OnRetrieveNextL()
	{
	if(++iCertCount < iCertStoreEntries.Count() )
		{
		iState = ERetrievingEntry;
		TInt size = iCertStoreEntries[iCertCount]->Size();
		if( size > iEncodedCert->Des().MaxLength() )
			{
			delete iEncodedCert;
			iEncodedCert = NULL;
			iEncodedCert = HBufC8::NewL(size);
			}
		TPtr8 encodedCertDesc = iEncodedCert->Des();
		iCertStore.Retrieve(*iCertStoreEntries[iCertCount],	encodedCertDesc,iStatus);
		SetActive();
		}
	else
		{
		User::RequestComplete(iClientStatus,OCSP::EResponseSignatureValidationFailure);	
		}
	}

/**
 * Once the certificate has been retrieved from the store, it should be sent for chain validation.
 * This intermediate state is required so that we can retrieve the certificate from the store.
 */
void COCSPDelegateAuthorisationScheme::OnRetrievingEntryL()
	{
	ValidateDelegateCertL(*iEncodedCert, iValidationTime);
	}

/**
 * Returns the responder certificate.
 */
const CX509Certificate* COCSPDelegateAuthorisationScheme::ResponderCert() const	
	{
	return iResponseCert;
	}
