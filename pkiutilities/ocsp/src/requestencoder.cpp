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
// Implementation for OCSP request encoder object
// 
//

#include "requestencoder.h"
#include "certid.h"
#include "oids.h"
#include "ocsp.h"
#include "panic.h"
#include <asn1enc.h>

COCSPRequestEncoder* COCSPRequestEncoder::NewL(const COCSPRequest& aRequest)
	{
	COCSPRequestEncoder* self = new (ELeave) COCSPRequestEncoder();
	CleanupStack::PushL(self);
	self->ConstructL(aRequest);
	CleanupStack::Pop(self);
	return self;
	}


COCSPRequestEncoder::COCSPRequestEncoder()
	{
	}


COCSPRequestEncoder::~COCSPRequestEncoder()
	{
	delete iEncoding;
	}


const TDesC8& COCSPRequestEncoder::Encoding() const
	{
	return *iEncoding;
	}


void COCSPRequestEncoder::ConstructL(const COCSPRequest& aRequest)
	{
	// Get encoder
	CASN1EncBase* encoder = MakeOCSPRequestEncLC(aRequest);
	
	// Make buffer
	TUint length = encoder->LengthDER();
	iEncoding = HBufC8::NewMaxL(length);
	TPtr8 buf = iEncoding->Des();
	
	// Write data
	TUint position = 0;
	encoder->WriteDERL(buf, position);
	
	CleanupStack::PopAndDestroy(encoder);
	}


// Static helper method - encapsulates a commonly-dupliciated peice of code
void COCSPRequestEncoder::AddAndPopChildL(CASN1EncContainer& aParent, CASN1EncBase* aChild)
	{
	if (aChild)
		{
		aParent.AddChildL(aChild);
		CleanupStack::Pop(aChild); // Now owned by aParent
		}
	}


CASN1EncBase* COCSPRequestEncoder::MakeOCSPRequestEncLC(const COCSPRequest& aRequest)
	{
	CASN1EncSequence* ocspRequest = CASN1EncSequence::NewLC();

	CASN1EncBase* tbsRequest = MakeTBSRequestEncLC(aRequest);
	AddAndPopChildL(*ocspRequest, tbsRequest);

	return ocspRequest;
	}


CASN1EncBase* COCSPRequestEncoder::MakeTBSRequestEncLC(const COCSPRequest& aRequest)
	{
	CASN1EncSequence* tbsRequest = CASN1EncSequence::NewLC();

	// version - int 0, explicitly tagged as 0
	// Excluded, since this is the default value, and we're using DER

	// Skip GeneralName - we don't support that, and it is optional

	// Cert requestList
	CASN1EncBase* requestList = MakeRequestListEncLC(aRequest);
	AddAndPopChildL(*tbsRequest, requestList);

	// Global request extensions - optional, explicitly tagged [2]
	CASN1EncBase* extensions = MakeRequestExtensionsEncLC(aRequest);
	if (extensions)
		{
		CleanupStack::Pop(extensions); // Will be owned by explicit tag
		CASN1EncExplicitTag* taggedExtensions = CASN1EncExplicitTag::NewLC(extensions, 2);
		AddAndPopChildL(*tbsRequest, taggedExtensions);
		}

	return tbsRequest;
	}


CASN1EncBase* COCSPRequestEncoder::MakeRequestListEncLC(const COCSPRequest& aRequest)
	{
	CASN1EncSequence* requestList = CASN1EncSequence::NewLC();

	// Request objects - one for each cert
	TUint numCerts = aRequest.CertCount();
	ASSERT(numCerts > 0);

	for (TUint index = 0; index < numCerts; ++index)
		{
		const COCSPCertID& certId = aRequest.CertInfo(index).CertID();
		CASN1EncBase* certEnc = MakeCertRequestEncLC(certId);
		AddAndPopChildL(*requestList, certEnc);
		}

	return requestList;
	}


// Make encoder for the request object for a particular cert
CASN1EncBase* COCSPRequestEncoder::MakeCertRequestEncLC(const COCSPCertID& aCertId)
	{
	CASN1EncSequence* request = CASN1EncSequence::NewLC();

	// CertID
	CASN1EncBase* certIdEnc = aCertId.EncoderLC();
	AddAndPopChildL(*request, certIdEnc);

	return request;
	}


CASN1EncBase* COCSPRequestEncoder::MakeRequestExtensionsEncLC(const COCSPRequest& aRequest)
	{
	CASN1EncSequence* extns = CASN1EncSequence::NewLC();

	// Nonce extension (if present)
	CASN1EncBase* nonceExtn = MakeNonceExtnEncLC(aRequest);
	AddAndPopChildL(*extns, nonceExtn);

	// Response type extentions
	CASN1EncBase* responseTypeExtn = MakeResponseTypeExtnEncLC();
	AddAndPopChildL(*extns, responseTypeExtn);

	return extns;
	}


// Response type.  Is a sequence, with OID, default bool, and octet string
// for contents.  In this case the octet string contains a sequence
// of OIDs specifying the acceptable response types.
CASN1EncBase* COCSPRequestEncoder::MakeResponseTypeExtnEncLC()
	{
	CASN1EncSequence* extn = CASN1EncSequence::NewLC();

	// OID
	CASN1EncObjectIdentifier* oid = CASN1EncObjectIdentifier::NewLC(KOCSPOidResponse);
	AddAndPopChildL(*extn, oid);

	// Skip Critical flag since we want default value FALSE and this is DER

	// Acceptable responses - takes a bit of work, so bear with it...

	// First, make sequence containing the OID for our response type
	CASN1EncSequence* responses = CASN1EncSequence::NewLC();
	CASN1EncObjectIdentifier* basic = CASN1EncObjectIdentifier::NewLC(KOCSPOidBasic);
	AddAndPopChildL(*responses, basic);

	// Second, encode this into a descriptor
	TBuf8<KOCSPMaxOidEncodingLength> buf(responses->LengthDER());
	TUint pos = 0;
	responses->WriteDERL(buf, pos);
	CleanupStack::PopAndDestroy(responses); // Finished with that now

	// Third, make Octet string encoder from this data + add to extn
	CASN1EncOctetString* value = CASN1EncOctetString::NewLC(buf);
	AddAndPopChildL(*extn, value);

	return extn;
	}


CASN1EncBase* COCSPRequestEncoder::MakeNonceExtnEncLC(const COCSPRequest& aRequest)
	{
	CASN1EncSequence* extn = NULL;

	if (aRequest.Nonce())
		{
		extn = CASN1EncSequence::NewLC();

		// OID
		CASN1EncObjectIdentifier* oid = CASN1EncObjectIdentifier::NewLC(KOCSPOidNonce);
		AddAndPopChildL(*extn, oid);
		
		// Skip Critical flag since we want default value FALSE and this is DER
		
		// Place nonce into extension
		CASN1EncOctetString* value = CASN1EncOctetString::NewLC(*aRequest.Nonce());
		AddAndPopChildL(*extn, value);
		}
		
	return extn;
	}
