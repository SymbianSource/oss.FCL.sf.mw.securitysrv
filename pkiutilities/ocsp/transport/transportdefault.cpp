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
// COCSPTransportDefault.  Looks at the URI and creates appropriate transport
// object to send the request.
// 
//

#include <ocsptransport.h>
#include "panic.h"
#include "ocsp.h"

#include <uri8.h>

_LIT8(KHttpString, "http");

EXPORT_C COCSPTransportDefault* COCSPTransportDefault::NewL(TUint32& aIap)
	{
	COCSPTransportDefault* self = new (ELeave) COCSPTransportDefault(aIap);
	return self;
	}

COCSPTransportDefault::COCSPTransportDefault(TUint32& aIap)
	: iIap(aIap)
	{
	}

COCSPTransportDefault::~COCSPTransportDefault()
	{
	delete iTransport;
	}

void COCSPTransportDefault::SendRequest(const TDesC8& aURI, 
										const TDesC8& aRequest,
										const TInt aTimeout,
									    TRequestStatus& iStatus)
	{
	TRAPD(err, CreateTransportL(aURI));
	if (err)
		{
		TRequestStatus* status = &iStatus;
		User::RequestComplete(status, err);
		}
	else
		{
		iTransport->SendRequest(aURI, aRequest, aTimeout, iStatus);
		}
	}

void COCSPTransportDefault::CancelRequest()
	{
	if (iTransport)
		{
		iTransport->CancelRequest();
		}
	}

TPtrC8 COCSPTransportDefault::GetResponse() const
	{
	ASSERT(iTransport);
	return iTransport->GetResponse();
	}

COCSPTransportDefault::TTransportScheme COCSPTransportDefault::IdentifySchemeL(const TDesC8& aURI)
	{
	TTransportScheme ret = ETransportSchemeNotSupported;
	
	TUriParser8 uri;
	TInt error = uri.Parse(aURI);
	if (error != KErrNone || !uri.IsPresent(EUriScheme))
		{
		return ret;
		}
	const TPtrC8 scheme = uri.Extract(EUriScheme);
			
	RStringPool stringPool;
	stringPool.OpenL();
	CleanupClosePushL(stringPool);

	RStringF schemeF = stringPool.OpenFStringL(scheme);
	CleanupClosePushL(schemeF);
	RStringF httpF = stringPool.OpenFStringL(KHttpString);
	CleanupClosePushL(httpF);

	if (schemeF == httpF)
		{
		ret = ETransportSchemeHTTP;
		}

	CleanupStack::PopAndDestroy(3); // close httpF, schemeF, stringPool

	return ret;
	}


void COCSPTransportDefault::CreateTransportL(const TDesC8& aURI)
	{
	TTransportScheme scheme = IdentifySchemeL(aURI);

	if (iTransport && iScheme == scheme)
		{
		return; // reuse existing transport
		}

	delete iTransport;
	iTransport = NULL;
	
	switch (scheme)
		{
		case ETransportSchemeHTTP:
			iTransport = COCSPTransportHttp::NewL(aURI, iIap);
			break;
			
		default:
			User::Leave(OCSP::KErrInvalidURI);
			break;
		}

	iScheme = scheme;
	}
