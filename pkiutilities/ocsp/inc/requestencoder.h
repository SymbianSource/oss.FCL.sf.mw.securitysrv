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
// Header specifying class that encodes OCSP requests
// 
//

/**
 @file 
 @internalComponent 
*/

#ifndef __OCSP_REQUESTENCODER_H__
#define __OCSP_REQUESTENCODER_H__

#include <ocsp.h>
#include "ocsprequestandresponse.h"
/**
 * Constructs the ASN1 DER respresentation of an OCSP request.
 */

NONSHARABLE_CLASS(COCSPRequestEncoder) : public CBase
	{
public:

	~COCSPRequestEncoder();

	/**
	 * Create a new request encoder object.
	 */
	
	static COCSPRequestEncoder* NewL(const COCSPRequest& aRequest);

	/**
	 * Get the encoding of the request.
	 * The data is owned by this object.
	 */

	const TDesC8& Encoding() const;

private:

	COCSPRequestEncoder();
	void ConstructL(const COCSPRequest& aRequest);
	
	CASN1EncBase* MakeOCSPRequestEncLC(const COCSPRequest& aRequest);
 	CASN1EncBase* MakeTBSRequestEncLC(const COCSPRequest& aRequest);
	CASN1EncBase* MakeRequestListEncLC(const COCSPRequest& aRequest);
	CASN1EncBase* MakeCertRequestEncLC(const COCSPCertID& aCertId);
	CASN1EncBase* MakeRequestExtensionsEncLC(const COCSPRequest& aRequest);
	CASN1EncBase* MakeResponseTypeExtnEncLC();
	CASN1EncBase* MakeNonceExtnEncLC(const COCSPRequest& aRequest);

	static void AddAndPopChildL(CASN1EncContainer& aParent, CASN1EncBase* aChild);

private:

	HBufC8* iEncoding;
	};

#endif
