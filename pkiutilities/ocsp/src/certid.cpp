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
// Define certID object, as used in the OCSP spec.  Used both to code data
// for the request, and decoded data from the response.
// 
//

#include "certid.h"
#include "bigint.h"


#include <asn1dec.h>
#include <asn1enc.h>
#include <x509cert.h>

COCSPCertID* COCSPCertID::NewL(const CX509Certificate& aSubject, const CX509Certificate& aIssuer)
	{
	COCSPCertID* self = new (ELeave) COCSPCertID;
	CleanupStack::PushL(self);
	self->ConstructL(aSubject, aIssuer);
	CleanupStack::Pop(self);
	return self;
	}


void COCSPCertID::ConstructL(const CX509Certificate& aSubject, const CX509Certificate& aIssuer)
	{
	CSHA1* sha1 = CSHA1::NewL();
	CleanupStack::PushL(sha1);

	// Hash of DER encoding of IssuerName from subject cert (including tag and length)
	const TPtrC8* issuerNameDER = aSubject.DataElementEncoding(CX509Certificate::EIssuerName);
	if (!issuerNameDER)
		{
		User::Leave(KErrArgument);
		}

	iIssuerNameHash.Copy(sha1->Hash(*issuerNameDER));

	// We'll ignore the 'number of unused bits' octet, since this is what everyone does, even
	// though strictly speaking the OCSP spec says to include it.  This has been discussed on
	// the PKIX mailing list.
	sha1->Reset();
	TPtrC8 subjectPublicKeyContents(aIssuer.PublicKey().KeyData());
	iIssuerKeyHash.Copy(sha1->Hash(subjectPublicKeyContents));

	CleanupStack::PopAndDestroy(sha1);

	// Set serial number from subject
	iSerialNumber.Set(aSubject.SerialNumber());
	}


COCSPCertID* COCSPCertID::NewL(const TDesC8& aBinaryData)
	{
	COCSPCertID* self = new (ELeave) COCSPCertID();
	CleanupStack::PushL(self);
	self->ConstructL(aBinaryData);
	CleanupStack::Pop(self);
	return self;
	}


void COCSPCertID::ConstructL(const TDesC8& aBinaryData)
	{
	// Check the tag
	TASN1DecGeneric decGen(aBinaryData);
	decGen.InitL();
	if (decGen.Tag() != EASN1Sequence)
		{
		User::Leave(KErrArgument);
		}

	// Decode the sequence into 4 bits
	TASN1DecSequence decSeq;
	CArrayPtr<TASN1DecGeneric>* items = decSeq.DecodeDERLC(decGen, 4, 4);

	// First part - the hash algorithm - check for SHA1, no more needed
	CX509AlgorithmIdentifier* algID = CX509AlgorithmIdentifier::NewLC(items->At(0)->Encoding());
	if (algID->Algorithm() != ESHA1)
		{
		User::Leave(KErrArgument);
		}
	CleanupStack::PopAndDestroy(algID);

	// Next parts - issuerNameHash and issuerKeyHash
	TASN1DecOctetString decOS;

	HBufC8* temp = decOS.DecodeDERL(*items->At(1));
	iIssuerNameHash.Copy(*temp);
	delete temp;

	temp = decOS.DecodeDERL(*items->At(2));
	iIssuerKeyHash.Copy(*temp);
	delete temp;

	// Lastly, the certificate serial number - just copy a reference to the encoded data
	iSerialNumber.Set(items->At(3)->GetContentDER());

	CleanupStack::PopAndDestroy(); // items
	}


// Construct ASN1 encoding object for the CertID data
CASN1EncBase* COCSPCertID::EncoderLC() const
	{
	CASN1EncSequence* certID = CASN1EncSequence::NewLC();

	// AlgId is a sequence, containing oid and null (both specific to SHA1)
	CASN1EncSequence* algEnc = CASN1EncSequence::NewLC();
	CASN1EncObjectIdentifier* algOidEnc = CASN1EncObjectIdentifier::NewLC(KSHA1);
	algEnc->AddChildL(algOidEnc);
	CleanupStack::Pop(); // algOidEnc, now owned by algEnc
	CASN1EncNull* nullEnc = CASN1EncNull::NewLC();
	algEnc->AddChildL(nullEnc);
	CleanupStack::Pop(); // nullEnc, now owned by algEnc
	certID->AddChildL(algEnc);
	CleanupStack::Pop(); // algEnc, now owned by certID

	// issuerNameHash
	CASN1EncOctetString* name = CASN1EncOctetString::NewLC(iIssuerNameHash);
	certID->AddChildL(name);
	CleanupStack::Pop(); // name, now owned by certID

	// issuerKeyHash
	CASN1EncOctetString* issuer = CASN1EncOctetString::NewLC(iIssuerKeyHash);
	certID->AddChildL(issuer);
	CleanupStack::Pop(); // issuer, now owned by certID

	// serialNumber
	RInteger serialNumber = RInteger::NewL(iSerialNumber);
	CleanupStack::PushL(serialNumber);
	CASN1EncBigInt* snEnc = CASN1EncBigInt::NewLC(serialNumber);
	certID->AddChildL(snEnc);
	CleanupStack::Pop(); // snEnc, now owned by certID;
	CleanupStack::PopAndDestroy(); // serialNumber

	return certID;
	}


TBool COCSPCertID::operator==(const COCSPCertID& aRhs) const
	{
	if (iIssuerNameHash == aRhs.iIssuerNameHash
		&& iIssuerKeyHash == aRhs.iIssuerKeyHash
		&& iSerialNumber == aRhs.iSerialNumber)
		{
		return ETrue;
		}
	else
		{
		return EFalse;
		}
	}


TPtrC8 COCSPCertID::SerialNumber() const
	{
	return iSerialNumber;
	}
