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
// Object encapsulating the OCSP response - uses ASN1 encoding
// 
//


#include "ocsp.h"
#include "oids.h"
#include "panic.h"
#include "certid.h"

#include <asn1dec.h>
#include <x509cert.h>
#include <x500dn.h>
#include "ocsprequestandresponse.h"

COCSPResponse::COCSPResponse()
	{
	}


COCSPResponse::~COCSPResponse()
	{
	delete iArchiveCutoff;
	iCertInfos.ResetAndDestroy();
	}


EXPORT_C TInt COCSPResponse::CertCount() const
	{
	return iCertInfos.Count();
	}


EXPORT_C const COCSPResponseCertInfo& COCSPResponse::CertInfo(TUint aIndex) const
	{
	return *iCertInfos[aIndex];
	}


const TPtrC8 COCSPResponse::SignedDataL() const
	{
	return iSignedData;	
	}


const TPtrC8* COCSPResponse::DataElementEncoding(const TUint aIndex) const
	{
	const TPtrC8* result = NULL;
	
	switch (aIndex)
		{
		case ECertificateChain:
			result = iSigningCerts.Length() ? &iSigningCerts : NULL;
			break;
		case ENonce:
			result = iNonce.Length() ? &iNonce : NULL;
			break;
		case EResponderIDName:
			result = iResponderIDName.Length() ? &iResponderIDName : NULL;
			break;
		case EResponderIDKeyHash:
			result = iResponderIDKeyHash.Length() ? &iResponderIDKeyHash : NULL;
			break;
		default:
			Panic(KErrArgument);	
		}

	return result;
	}


void COCSPResponse::InternalizeL(RReadStream& /* aStream */)
	{
	Panic(KErrNotSupported);	
	}


EXPORT_C TTime COCSPResponse::ProducedAt() const
	{
	return iProducedAt;
	}


EXPORT_C const TTime* COCSPResponse::ArchiveCutoff() const
	{
	return iArchiveCutoff;
	}


TInt COCSPResponse::Find(const COCSPCertID& aCertID) const
	{
	TInt count = CertCount();
	for (TInt index = 0; index < count; ++index)
		{
		const COCSPCertID& responseCertID = CertInfo(index).CertID();
		if (responseCertID == aCertID)
			{
			return index;
			}
		}
	
	return KErrNotFound;
	}
