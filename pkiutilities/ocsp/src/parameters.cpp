// Copyright (c) 2003-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include "panic.h"
#include "log.h"
#include <ocsp.h>
#include <x509certchain.h>
#include <ocsppolicy.h>
#include "ocsprequestandresponse.h"

EXPORT_C COCSPParameters* COCSPParameters::NewL()
	{
	COCSPParameters* self = NewLC();
	CleanupStack::Pop(self);
	return self;
	}

EXPORT_C COCSPParameters* COCSPParameters::NewLC()
	{
	COCSPParameters* self = new (ELeave) COCSPParameters();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

COCSPParameters::COCSPParameters() :
	iUseNonce(ETrue),
	iUseAIA(ETrue),
	iGenerateResponseForMissingUri(ETrue),
	iResponderCertCheck(EFalse), // should be turned off by default
	iRetryCount(KTransportDefaultRequestRetryCount),
	iTimeout(KTransportDefaultRequestTimeout),
	iCheckCertsWithAiaOnly(EFalse) // should be turned off by default
	{}

void COCSPParameters::ConstructL()
	{
	DEBUG_PRINTF(_L8("Reading policy."));
	iDefaultURI = KNullDesC8().AllocL();
	
	COcspPolicy* ocspPolicy = COcspPolicy::NewL();
	iGenerateResponseForMissingUri = ocspPolicy->IsGenerateResponseForMissingUriEnabled();

	delete ocspPolicy;

	DEBUG_PRINTF2(_L8("Generate response when no AIA URI and no default OCSP URI: %d."), iGenerateResponseForMissingUri);
	}

COCSPParameters::~COCSPParameters()
	{
	iSubjectCerts.Close();
	iIssuerCerts.Close();
	delete iDefaultURI;
	delete iTransport;
	iAuthSchemes.ResetAndDestroy();
	delete iValidationTime;
	delete iMaxStatusAge;
	delete iTimeLeeway;
	}

EXPORT_C void COCSPParameters::AddCertificateL(const CX509Certificate& aSubject, const CX509Certificate& aIssuer)
	{
	User::LeaveIfError(iSubjectCerts.Append(&aSubject));
	User::LeaveIfError(iIssuerCerts.Append(&aIssuer));
	}

EXPORT_C void COCSPParameters::AddCertificatesL(const CX509CertChain& aChain)
	{
	TInt numCerts = aChain.Count();
	if (numCerts >= 2)
		{
		// Go through all but last cert (last = root)
		const CX509Certificate* issuerCert = &aChain.Cert(0);
		const CX509Certificate* subjectCert = NULL;
		for (TInt index = 1; index < numCerts; ++index)
			{
			subjectCert = issuerCert;
			issuerCert = &aChain.Cert(index);

			AddCertificateL(*subjectCert, *issuerCert);
			}
		}
	}

EXPORT_C void COCSPParameters::SetUseNonce(TBool aUseNonce)
	{
	iUseNonce = aUseNonce;
	}

EXPORT_C void COCSPParameters::SetURIL(const TDesC8& aURI, TBool aUseAIA)
	{
	delete iDefaultURI;
	iDefaultURI = NULL;
	iDefaultURI = aURI.AllocL();
	iUseAIA = aUseAIA;
	}

EXPORT_C void COCSPParameters::SetTransport(MOCSPTransport* aTransport)
	{
	delete iTransport;
	iTransport = aTransport;
	}

EXPORT_C void COCSPParameters::SetRetryCount(const TUint aRetryCount)
	{
	iRetryCount = aRetryCount;
	}

EXPORT_C void COCSPParameters::SetTimeout(const TInt aTimeout)
	{
	iTimeout = aTimeout;
	}

EXPORT_C void COCSPParameters::AddAuthorisationSchemeL(MOCSPAuthorisationScheme* aScheme)
	{
	__ASSERT_ALWAYS(aScheme, ::Panic(KErrArgument));
	User::LeaveIfError(iAuthSchemes.Append(aScheme));
	}

EXPORT_C void COCSPParameters::AddAllAuthorisationSchemesL(const TUid& aCertStoreUid, MCertStore& aCertStore)
/**
	This function adds all of the currently supported authorisation schemes
	to this object.  It is more convenient than having the client to allocate
	each scheme.

	This function allocates the authorisation schemes defined in RFC2560 S2.2 -
	direct authorisation, CA delegate, and CA direct.

	@param	aCertStoreUid	UID of trusted root certificates.  E.g.,
							KCertStoreUIDForSWInstallOCSPSigning.
	@param	aCertStore		Certificate store which contains the
							the trust anchors used to validate the
							response.	
	@pre No authorisation schemes should have been added to this object before
		this function is called.
	@see AddAuthorisationSchemeL
 */
	{
	__ASSERT_DEBUG(iAuthSchemes.Count() == 0, Panic(EAAASAlreadyHaveSchemes));

	COCSPDirectAuthorisationScheme* directScheme =
		COCSPDirectAuthorisationScheme::NewLC(aCertStoreUid, aCertStore);
	AddAuthorisationSchemeL(directScheme);
	CleanupStack::Pop(directScheme);

	COCSPDelegateAuthorisationScheme* caDelgScheme =
		COCSPDelegateAuthorisationScheme::NewLC(aCertStore);
	AddAuthorisationSchemeL(caDelgScheme);
	CleanupStack::Pop(caDelgScheme);

	COCSPCaDirectAuthorisationScheme* caDirectScheme = COCSPCaDirectAuthorisationScheme::NewLC();
	AddAuthorisationSchemeL(caDirectScheme);
	CleanupStack::Pop(caDirectScheme);
	}

EXPORT_C void COCSPParameters::SetValidationTimeL(const TTime& aValidationTime)
	{
	delete iValidationTime;
	iValidationTime = NULL;
	iValidationTime = new (ELeave) TTime(aValidationTime);
	}

EXPORT_C void COCSPParameters::SetMaxStatusAgeL(TUint aMaxAge)
	{
	delete iMaxStatusAge;
	iMaxStatusAge = NULL;
	iMaxStatusAge = new (ELeave) TUint(aMaxAge);
	}

EXPORT_C void COCSPParameters::SetTimeLeewayL(TUint aLeewaySeconds)
	{
	delete iTimeLeeway;
	iTimeLeeway = NULL;
	iTimeLeeway = new (ELeave) TUint(aLeewaySeconds);
	}

EXPORT_C void COCSPParameters::SetCheckCertsWithAiaOnly(const TBool aCheckCertsWithAiaOnly)
	{
	iCheckCertsWithAiaOnly = aCheckCertsWithAiaOnly;
	}

EXPORT_C void COCSPParameters::SetOCSPCheckForResponderCert(const TBool aResponderCertCheck)
	{
	iResponderCertCheck = aResponderCertCheck;
	}

EXPORT_C void COCSPParameters::SetUseAIA(const TBool aUseAIA)
	{
	iUseAIA = aUseAIA;
	}

TUint COCSPParameters::CertCount() const
	{
	return iSubjectCerts.Count();
	}

const CX509Certificate& COCSPParameters::SubjectCert(TUint aIndex) const
	{
	return *iSubjectCerts[aIndex];
	}

const CX509Certificate& COCSPParameters::IssuerCert(TUint aIndex) const
	{
	return *iIssuerCerts[aIndex];
	}

TBool COCSPParameters::UseNonce() const
	{
	return iUseNonce;
	}

const TDesC8& COCSPParameters::DefaultURI() const
	{
	return static_cast<TDesC8&>(*iDefaultURI);
	}

TBool COCSPParameters::UseAIA() const
	{
	return iUseAIA;
	}

MOCSPTransport* COCSPParameters::Transport() const
	{
	return iTransport;
	}

TUint COCSPParameters::AuthSchemeCount() const
	{
	return iAuthSchemes.Count();
	}

MOCSPAuthorisationScheme& COCSPParameters::AuthScheme(TUint aIndex) const
	{
	// Modified so when backported to typhoon this stil compiles, required because of the
	// RArrayPointer operator[] changes,
	return const_cast<MOCSPAuthorisationScheme&>(*iAuthSchemes[aIndex]);
	}

const TTime* COCSPParameters::ValidationTime() const
	{
	return iValidationTime;
	}

const TUint* COCSPParameters::MaxStatusAge() const
	{
	return iMaxStatusAge;
	}

const TUint* COCSPParameters::TimeLeeway() const
	{
	return iTimeLeeway;
	}

TBool COCSPParameters::GenerateResponseForMissingUri() const
	{
	return iGenerateResponseForMissingUri;
	}

TUint COCSPParameters::RetryCount() const
	{
	return iRetryCount;
	}

TInt COCSPParameters::Timeout() const
	{
	return iTimeout;
	}

TBool COCSPParameters::ReponderCertCheck() const
	{
	return iResponderCertCheck;
	}

TBool COCSPParameters::CheckCertsWithAiaOnly() const
	{
	return iCheckCertsWithAiaOnly;
	}

#ifdef _DEBUG

void COCSPParameters::Panic(COCSPParameters::TPanic aPanic)
/**
	Halt the current thread with the supplied panic code.
	The thread is halted with category "OCSPParam" and the supplied
	reason.

	@param	aPanic			Panic reason.
 */
	{
	_LIT(KPanicCat, "OCSPParam");
	User::Panic(KPanicCat, aPanic);
	}

#endif	// #ifdef _DEBUG
