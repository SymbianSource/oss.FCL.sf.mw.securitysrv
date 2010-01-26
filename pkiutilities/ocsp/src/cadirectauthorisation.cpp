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
// Define methods for authorisation scheme based on the CA who issued the certificate
// issues and signs the response.
// 
//

#include "ocsp.h"
#include "panic.h"
#include <pkixcertchain.h>
#include <asn1dec.h>
#include <x509keys.h>
#include <x509cert.h>
#include "ocsprequestandresponse.h"


EXPORT_C COCSPCaDirectAuthorisationScheme* COCSPCaDirectAuthorisationScheme::NewLC()
	{
	COCSPCaDirectAuthorisationScheme* self = new(ELeave) COCSPCaDirectAuthorisationScheme();
	CleanupStack::PushL(self);
	return self;
	}

/**
	Validate the response if it is signed by the CA.  The response can
	optionally contain a copy of the CA's certificate.
*/
void COCSPCaDirectAuthorisationScheme::ValidateL(OCSP::TStatus& aOCSPStatus,
	COCSPResponse& aResponse, const TTime /* aValidationTime */, TRequestStatus& aStatus,
	const COCSPRequest& aRequest)
	{
	TRequestStatus* validatorReqStatus = &aStatus;
	aOCSPStatus = OCSP::EResponseSignatureValidationFailure;

	// the OCSP requests are constructed in
	// COCSPClient::ConstructL that exactly one certificate
	// and its signer are sent in each request.
	// (RFC 2560 S4.1.1 allows multiple pairs)
	
	// check the response is actually signed by the CA
	// this effectively tests the same condition.
	// By assuming there is only one request / response pair,
	// there is no need to iterate through each response, which
	// simplifies this scheme's implementation.

	if (DoValidateL(aRequest, aResponse ))
		{
		aOCSPStatus = OCSP::EValid;
		}
	User::RequestComplete(validatorReqStatus, KErrNone);
	};

/**
	Checks if the response is signed by the CA.
	If the response has a certificate chain, it must contain
	exactly the CA cert.  It is acceptable for the response
	to have no certificate chain.

	The responder ID in the certificate must match the CA cert,
	and the whole response must be signed by the CA cert's signer.
*/
TBool COCSPCaDirectAuthorisationScheme::DoValidateL(
	const COCSPRequest& aRequest, COCSPResponse& aResponse)
{
	// check the response contains either zero or one certificates.
	// If the response contains a certificate, then it must be the
	// same as the CA.

	// get the intermediate which signed the EE
	const CX509Certificate& caCert = aRequest.CertInfo(0).Issuer();
	
	// Retrieves the chain from the response of the responder certificate
	// whic signed the response.
	const TPtrC8* certChainData = aResponse.DataElementEncoding(COCSPResponse::ECertificateChain);
	if (certChainData != 0)
		{
		if (! CertChainMatchesCertL(*certChainData, caCert))
			return EFalse;
		}

	return
			OCSPUtils::DoesResponderIdMatchCertL(aResponse, caCert)
		&&	OCSPUtils::IsResponseSignedByCertL(&aResponse, caCert);
}

/**
	Checks whether the encoded cert chain contains exactly
	one cert which matches the supplied cert.

	This is used to verify that, when a cert chain is sent with
	the response, it contains exactly the CA cert.

	@param	aCertChainData	DER-encoded certificate chain data
							extracted from response.
	@param	aCert			Certificate to look for.
	@return					ETrue if the certificate chain contains
							exactly one response which matches
							aCert; EFalse otherwise.
 */
TBool COCSPCaDirectAuthorisationScheme::CertChainMatchesCertL(
	const TDesC8& aCertChainData, const CX509Certificate& aCert)
	{
	// here we cannot assume that the response contains only the CA certificate,
	// which signed the response as that can be a subordinate CA, so the chain
	// can be like RCA(root CA) -> ICA(intermediate CA) -> SCA (sub-ordinate CA)
	// we need to retrieve the SCA for this verification.
	CX509Certificate* respCert = OCSPUtils::GetResponderCertLC(aCertChainData);
	TBool match = aCert.IsEqualL(*respCert);
	CleanupStack::PopAndDestroy(respCert);
	return match;
	}

/**
	This is a no-op because this implementation is not an
	active object.
 */
void COCSPCaDirectAuthorisationScheme::CancelValidate()
	{}

const CX509Certificate* COCSPCaDirectAuthorisationScheme::ResponderCert() const	
	{
	return NULL;
	}
