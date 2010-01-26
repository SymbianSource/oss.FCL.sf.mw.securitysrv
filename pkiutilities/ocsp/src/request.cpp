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
// Object encapsulating the OCSP request - uses ASN1 encoding
// 
//

#include "ocsp.h"
#include "panic.h"
#include "oids.h"
#include "certid.h"

#include <bigint.h>
#include <x509cert.h>
#include <x509certchain.h>
#include <random.h>
#include "ocsprequestandresponse.h"

// COCSPRequest methods
COCSPRequest* COCSPRequest::NewLC(TBool aUseNonce)
	{
	COCSPRequest* self = new (ELeave) COCSPRequest;
	CleanupStack::PushL(self);
	self->ConstructL(aUseNonce);
	return self;
	}

COCSPRequest* COCSPRequest::NewL(TBool aUseNonce)
	{
	COCSPRequest* self = COCSPRequest::NewLC(aUseNonce);
	CleanupStack::Pop(self);
	return self;
	}

COCSPRequest::COCSPRequest()
	{}

void COCSPRequest::ConstructL(TBool aUseNonce)
	{
	if (aUseNonce)
		{
		// Make nonce data
		iNonce = HBufC8::NewL(KOCSPNonceBytes);
		TPtr8 des = iNonce->Des();
		des.SetLength(KOCSPNonceBytes);
		TRandom::RandomL(des);
		}
	}


COCSPRequest::~COCSPRequest()
	{
	iCertInfos.ResetAndDestroy();
	delete iNonce;
	}


void COCSPRequest::AddCertificateL(const CX509Certificate& aSubject,
								   const CX509Certificate& aIssuer)
	{
	COCSPRequestCertInfo* certInfo = COCSPRequestCertInfo::NewLC(aSubject, aIssuer);
	User::LeaveIfError(iCertInfos.Append(certInfo));
	CleanupStack::Pop(certInfo);
	}


EXPORT_C const TDesC8* COCSPRequest::Nonce() const
	{
	return iNonce;
	}


EXPORT_C TInt COCSPRequest::CertCount() const
	{
	return iCertInfos.Count();
	}


EXPORT_C const COCSPRequestCertInfo& COCSPRequest::CertInfo(TUint aIndex) const
	{
	return *iCertInfos[aIndex];
	}


// COCSPRequestCertInfo methods


COCSPRequestCertInfo* COCSPRequestCertInfo::NewLC(const CX509Certificate& aSubject,
												  const CX509Certificate& aIssuer)
	{
	COCSPRequestCertInfo* self = new (ELeave) COCSPRequestCertInfo(aSubject, aIssuer);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}


COCSPRequestCertInfo::COCSPRequestCertInfo(const CX509Certificate& aSubject, const CX509Certificate& aIssuer) :
	iSubject(aSubject),
	iIssuer(aIssuer)
	{
	}


void COCSPRequestCertInfo::ConstructL()
	{
	iCertID = COCSPCertID::NewL(iSubject, iIssuer);
	}


COCSPRequestCertInfo::~COCSPRequestCertInfo()
	{
	delete iCertID;
	}


EXPORT_C const CX509Certificate& COCSPRequestCertInfo::Subject() const
	{
	return iSubject;
	}


EXPORT_C const CX509Certificate& COCSPRequestCertInfo::Issuer() const
	{
	return iIssuer;
	}


const COCSPCertID& COCSPRequestCertInfo::CertID() const
	{
	return *iCertID;
	}
