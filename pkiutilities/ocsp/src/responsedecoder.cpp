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
// requestencoder.cpp
// Implementation for OCSP response decoder object.
// See RFC2560 S4.2.1 for response specification.
// 
//

#include "responsedecoder.h"
#include "oids.h"
#include <ocsp.h>
#include <asn1dec.h>
#include <x509cert.h>
#include "ocsprequestandresponse.h"

// Enum values in DER encoding of response status
enum
	{
	ESuccessfulEncoding = 0,
	EMalformedRequestEncoding = 1,
	EInternalErrorEncoding = 2,
	ETryLaterEncoding = 3,
	ESigRequiredEncoding = 5,
	EUnauthorisedEncoding = 6
	};


// Tag values in DER encoded response data
const TUint KResponseBytesTag = 0;
const TUint KCertificatesTag = 0;

/** Version tag number in ResponseData, RFC2560, S4.2.1. */
const TUint KOcspResponseVersionTag = 0;

/**
	Supported OCSP response Version, RFC2560, S4.2.1.
	This is the integer value in Version, which means response
	format version 1.
 */
const TUint KOcspResponseVersion1 = 0;

const TUint KResponderIDNameTag = 1;
const TUint KResponderIDKeyHashTag = 2;
const TUint KResponseExtensionsTag = 1;


COCSPResponseDecoder* COCSPResponseDecoder::NewL(const TDesC8& aEncoding)
	{
	COCSPResponseDecoder* self = new (ELeave) COCSPResponseDecoder;
	CleanupStack::PushL(self);
	self->ConstructL(aEncoding);
	CleanupStack::Pop(self);
	return self;
	}


COCSPResponseDecoder::COCSPResponseDecoder()
	{
	}


COCSPResponseDecoder::~COCSPResponseDecoder()
	{
	delete iResponse;
	}


void COCSPResponseDecoder::ConstructL(const TDesC8& aEncoding)
	{
	iResponse = new (ELeave) COCSPResponse;
	
	// Populate CSignedObject data members
	iResponse->iKeyFactory = new (ELeave) TX509KeyFactory; // Unconventional class name
	iResponse->iEncoding = aEncoding.AllocL();
	
	TRAPD(error, DecodeOCSPResponseL(aEncoding));
	if (error == KErrArgument || error == KErrNotSupported)
		{
		// These arise from problems parsing the data in X509 or ASN1
		error = OCSP::EMalformedResponse;
		}
	
	if (error != KErrNone)
		{
		// Errors and our status codes go back to the client
		delete iResponse;
		iResponse = NULL;
		User::Leave(error);
		}
	}


COCSPResponse* COCSPResponseDecoder::TakeResponse()
	{
	COCSPResponse* result = iResponse;
	iResponse = NULL;
	return result;
	}


CArrayPtr<TASN1DecGeneric>* COCSPResponseDecoder::DecodeSequenceLC(const TDesC8& aEncoding)
	{
	CArrayPtr<TASN1DecGeneric>* items = NULL;
	
	// Check we've got a sequence
	TASN1DecGeneric decGen(aEncoding);
	decGen.InitL();
	if (decGen.Tag() != EASN1Sequence)
		{
		User::Leave(KErrArgument);
		}
	else
		{
		// Decode the sequence		
		TASN1DecSequence decSeq;
		items = decSeq.DecodeDERLC(decGen);
		}
	return items;
	}


CArrayPtr<TASN1DecGeneric>* COCSPResponseDecoder::DecodeSequenceLC(const TDesC8& aEncoding,
															const TInt aMinTerms,
															const TInt aMaxTerms)
	{
	CArrayPtr<TASN1DecGeneric>* items = DecodeSequenceLC(aEncoding);
	TInt count = items->Count();
	if (count < aMinTerms || count > aMaxTerms)
		{
		User::Leave(KErrArgument);
		}

	return items;
	}


void COCSPResponseDecoder::DecodeOCSPResponseL(const TDesC8& aEncoding)
	{
	CArrayPtr<TASN1DecGeneric>* items = DecodeSequenceLC(aEncoding, 1, 2);
	
	// Use integer decoding for enumerated
	TASN1DecInteger decInt;
	TInt status = decInt.DecodeDERShortL(*items->At(0));
	if (status == ESuccessfulEncoding)
		{
		if (items->Count() != 2)
			{
			User::Leave(OCSP::EMalformedResponse);
			}
		
		// Check tag on second part is [0]
		// We ignore any other parts in the sequence after that
		TASN1DecGeneric& responseBytesDec = *items->At(1);
		if (responseBytesDec.Tag() != KResponseBytesTag)
			{
			User::Leave(OCSP::EMalformedResponse);
			}
		
		// It's OK, so decode the response bytes object therein
		DecodeResponseBytesL(responseBytesDec.GetContentDER());
		}
	else
		{
		if (items->Count() != 1)
			{
			User::Leave(KErrArgument);
			}
		
		switch (status)
			{
			case EMalformedRequestEncoding:
				User::Leave(OCSP::EMalformedRequest);
			case EInternalErrorEncoding:
				User::Leave(OCSP::EServerInternalError);
				break;
			case ETryLaterEncoding:
				User::Leave(OCSP::ETryLater);
				break;
			case ESigRequiredEncoding:
				User::Leave(OCSP::ESignatureRequired);
				break;
			case EUnauthorisedEncoding:
				User::Leave(OCSP::EClientUnauthorised);
				break;
			default:
				User::Leave(OCSP::EMalformedResponse);
			}
		}
		CleanupStack::PopAndDestroy(); // items
	}


void COCSPResponseDecoder::DecodeResponseBytesL(const TDesC8& aEncoding)
	{
	CArrayPtr<TASN1DecGeneric>* items = DecodeSequenceLC(aEncoding, 2, 2);

	TASN1DecObjectIdentifier decOid;
	HBufC* oid = decOid.DecodeDERL(*items->At(0));
	CleanupStack::PushL(oid);
	if (*oid != KOCSPOidBasic)
		{
		User::Leave(OCSP::EUnknownResponseType);
		}

	TASN1DecGeneric& response = *items->At(1);
	if (response.Tag() != EASN1OctetString)
		{
		User::Leave(OCSP::EMalformedResponse);
		}

	DecodeBasicOCSPResponseL(response.GetContentDER());

	CleanupStack::PopAndDestroy(2); // oid, items
	}


void COCSPResponseDecoder::DecodeBasicOCSPResponseL(const TDesC8& aEncoding)
	{
	CArrayPtr<TASN1DecGeneric>* items = DecodeSequenceLC(aEncoding, 3, 4);

	// First, the ResponseData object
	DecodeResponseDataL(items->At(0)->Encoding());

	// Continue, with the AlgorithmIdentifier
	iResponse->iSigningAlgorithm = CX509SigningAlgorithmIdentifier::NewL(items->At(1)->Encoding());
	
	// Now move on to the signature
	TASN1DecBitString encBS;
	iResponse->iSignature = encBS.ExtractOctetStringL(*items->At(2));
	
	// And finally, the certs (if they're there)
	if (items->Count() == 4)
		{
		// Check explicit tag [0]
		TASN1DecGeneric& certsDec = *items->At(3);
		if (certsDec.Tag() != KCertificatesTag)
			{
			User::Leave(OCSP::EMalformedResponse);
			}
		
		// It's OK, so decode the response bytes object therein
		DecodeCertificatesL(certsDec.GetContentDER());
		}

	CleanupStack::PopAndDestroy(); // Cleans up items
	}


void COCSPResponseDecoder::DecodeCertificatesL(const TDesC8& aEncoding)
	{
	TASN1DecGeneric dec(aEncoding);
	dec.InitL();
	if (dec.Tag() != EASN1Sequence)
		{
		User::Leave(OCSP::EMalformedResponse);
		}
	
	// Just stores a reference to the encoding
	iResponse->iSigningCerts.Set(dec.GetContentDER());
	}


void COCSPResponseDecoder::DecodeResponseDataL(const TDesC8& aEncoding)
	{
	// This is the signed data
	iResponse->iSignedData.Set(aEncoding);
	
	// version and responseExtensions are optional
	CArrayPtr<TASN1DecGeneric>* items = DecodeSequenceLC(aEncoding, 3, 5);
	
	TInt seqIndex = 0;		// index of currently-parsed item
	TASN1DecGeneric& item0 = *items->At(0);
	
	// If a version number is supplied, it must be the default version, v1,
	// which is encoded as an integer with value zero.  (The version should
	// not be sent at all if it has the default value - S11.5, ITU-Y X.690
	// (07/2002) ASN.1 encoding rules - but some servers can still send it.)
	if (item0.Class() == EContextSpecific && item0.Tag() == KOcspResponseVersionTag)
		{
		TASN1DecGeneric vDecGen(item0.GetContentDER());
		vDecGen.InitL();
		if (!(vDecGen.Class() == EUniversal && vDecGen.Tag() == EASN1Integer))
			{
			User::Leave(OCSP::EMalformedResponse);
			}
		
		TASN1DecInteger vDecInt;
		TInt version = vDecInt.DecodeDERShortL(vDecGen);
		if (version != KOcspResponseVersion1)
			{
			User::Leave(OCSP::EMalformedResponse);
			}
		++seqIndex;
		}
	
	TASN1DecGeneric& respIdDecGen = *items->At(seqIndex++);
	switch (respIdDecGen.Tag())
		{
		case KResponderIDNameTag:
			// Set to Name DER encoding
			iResponse->iResponderIDName.Set(respIdDecGen.GetContentDER());
			break;
		case KResponderIDKeyHashTag:
			{
			// Set to KeyHash to value within the octet string
			TASN1DecGeneric keyHashDecGen(respIdDecGen.GetContentDER());
			keyHashDecGen.InitL();
			iResponse->iResponderIDKeyHash.Set(keyHashDecGen.GetContentDER());
			break;
			}
		default:
			User::Leave(OCSP::EMalformedResponse);
		}
	
	
	// ProducedAt is a GeneralizedTime
	TASN1DecGeneralizedTime decGT;
	iResponse->iProducedAt = decGT.DecodeDERL(*items->At(seqIndex++));
	
	// Now the responses themselves
	DecodeResponsesL(items->At(seqIndex++)->Encoding());
	
	// Continue if extensions exist
	if (seqIndex < items->Count())
		{
		// Check tag on responseExtensions
		TASN1DecGeneric& extDecGen = *items->At(seqIndex++);
		if (extDecGen.Tag() != KResponseExtensionsTag)
			{
			User::Leave(OCSP::EMalformedResponse);
			}
		
		DecodeResponseExtensionsL(extDecGen.GetContentDER());
		}
	CleanupStack::PopAndDestroy(items);
	}


void COCSPResponseDecoder::DecodeResponseExtensionsL(const TDesC8& aEncoding)
	{
	CArrayPtr<TASN1DecGeneric>* items = DecodeSequenceLC(aEncoding);
	TInt count = items->Count();
	for (TInt index = 0; index < count; ++index)
		{
		DecodeResponseExtensionL(items->At(index)->Encoding());
		}
	
	CleanupStack::PopAndDestroy(); // items
	}


void COCSPResponseDecoder::DecodeResponseExtensionL(const TDesC8& aEncoding)
	{
	CArrayPtr<TASN1DecGeneric>* items = DecodeSequenceLC(aEncoding, 2, 3);
	
	// Get oid
	TASN1DecGeneric& oid = *items->At(0);
	if (oid.Tag() != EASN1ObjectIdentifier)
		{
		User::Leave(OCSP::EMalformedResponse);
		}
	
	TASN1DecObjectIdentifier oidDec;
	HBufC* oidVal = oidDec.DecodeDERL(oid);
	CleanupStack::PushL(oidVal);
	
	TBool critical = EFalse; // Default value of critical flag
	if (items->Count() == 3)
		{
		// The critical flag is specified - what does it say?
		TASN1DecBoolean decBool;
		critical = decBool.DecodeDERL(*items->At(1));
		}
	
	TASN1DecGeneric& extnVal = items->Count() == 3 ? *items->At(2) : *items->At(1);
	if (extnVal.Tag() != EASN1OctetString)
		{
		User::Leave(OCSP::EMalformedResponse);
		}
	
	// Check oid to decide what to do
	if (*oidVal == KOCSPOidNonce)
		{
		iResponse->iNonce.Set(extnVal.GetContentDER());
		}
	else if (*oidVal == KOCSPOidArchiveCutoff)
		{
		TASN1DecGeneralizedTime decGT;
		TInt pos = 0;
		iResponse->iArchiveCutoff = new (ELeave) TTime(decGT.DecodeDERL(extnVal.GetContentDER(), pos));
		}
	else if (critical)
		{
		// Didn't understand extension, and it was critical!  Erk!
		User::Leave(OCSP::EUnknownCriticalExtension);
		}
	
	CleanupStack::PopAndDestroy(2); // oidVal, items
	}


void COCSPResponseDecoder::DecodeResponsesL(const TDesC8& aEncoding)
	{
	CArrayPtr<TASN1DecGeneric>* items = DecodeSequenceLC(aEncoding);
	TInt count = items->Count();
	for (TInt index = 0; index < count; ++index)
		{
		DecodeSingleResponseL(items->At(index)->Encoding());
		}

	CleanupStack::PopAndDestroy(); // items
	}


void COCSPResponseDecoder::DecodeSingleResponseL(const TDesC8& aEncoding)
	{
	CArrayPtr<TASN1DecGeneric>* items = DecodeSequenceLC(aEncoding, 3, 5);

	COCSPResponseCertInfo* response = COCSPResponseCertInfo::NewLC(*items);
	User::LeaveIfError(iResponse->iCertInfos.Append(response));
	CleanupStack::Pop(response); // Now owned through iSingleResponses

	CleanupStack::PopAndDestroy(); // Clean up items
	}
