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
// Used in cert info objects for requests and responses.
// Declare certID object, representing the CertID data structure from
// the OCSP spec. Used in cert info objects for requests and responses.
// 
//

/**
 @file 
 @internalTechnology 
*/
 
#ifndef __OCSP_CERTID_H__
#define __OCSP_CERTID_H__

#include <hash.h>

class CASN1EncBase;
class CX509Certificate;

// CertID, used in requests and responses
NONSHARABLE_CLASS(COCSPCertID) : public CBase
	{
public:
	// Make from certificates - subject mustn't be deleted in our lifetime
	static COCSPCertID* NewL(const CX509Certificate& aSubject, const CX509Certificate& aIssuer);

	// Make from binary data - which mustn't be deleted in our lifetime
	static COCSPCertID* NewL(const TDesC8& aBinaryData);

	// Construct ASN1 encoding object for the CertID data
	CASN1EncBase* EncoderLC() const;

    TPtrC8 SerialNumber() const;

	TBool operator==(const COCSPCertID& aRhs) const;

private:
	void ConstructL(const CX509Certificate& aSubject, const CX509Certificate& aIssuer);
	void ConstructL(const TDesC8& aBinaryData);

private:
	TBuf8<SHA_HASH> iIssuerNameHash;
	TBuf8<SHA_HASH> iIssuerKeyHash;
	TPtrC8 iSerialNumber; // Refers to the input certificate or input binary data
	};

#endif
