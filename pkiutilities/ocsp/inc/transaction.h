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
// Header file for OCSP transaction object
// 
//

/**
 @file 
 @internalComponent 
*/

#ifndef __OCSP_TRANSACTION_H__
#define __OCSP_TRANSACTION_H__

#include <ocsp.h>
#include <e32base.h>
#include "ocsprequestandresponse.h"

class COCSPRequestEncoder;
class COCSPResponseDecoder;

/**
 * OCSP Transaction object, used to send the request to the server and get the
 * response back again.
 */

NONSHARABLE_CLASS(COCSPTransaction) : public CActive
	{
public:
	// Does not take ownership of the transport object
	static COCSPTransaction* NewL(const TDesC8& aURI, 
								  MOCSPTransport& aTransport, const TUint aRetryCount, const TInt aTimeout);

	~COCSPTransaction();

	/**
	 * Asynchronous interface to send the reqest.  Can complete with one of the
	 * status codes from OCSP::TStatus if there's a problem decoding the
	 * response.
	 */
	
	void SendRequest(COCSPRequest& aRequest, TRequestStatus& aStatus);
	
	void CancelRequest();

	/**
	 * Get the response and relinquish ownership of it.  Returns NULL unless
	 * SendRequest completed successfully.
	 */	

	COCSPResponse* TakeResponse();

private:
	// From CActive
	void RunL();
	void DoCancel();
	TInt RunError(TInt aError);

private:
	COCSPTransaction(MOCSPTransport& aTransport, const TUint aRetryCount, const TInt aTimeout);
	void ConstructL(const TDesC8& aURI);

	void CompleteClient(const TInt aStatus);

private:

	enum TState
		{
		ESendRequest,
		EFinished
		};

	MOCSPTransport& iTransport;
	HBufC8* iURI;

	TState iState;
	
	// Client TRequestStatus, for the asynchronous SendRequestL
	TRequestStatus* iClientRequestStatus;

	// Request object to send
	COCSPRequest* iRequest;

	// Request encoder.  We own this.
	COCSPRequestEncoder* iRequestEncoder;

	// Response object, NULL unless response data received OK
	COCSPResponseDecoder* iResponseDecoder;

	// Remaining number of times to send request on failed response (1 means no retry)
	TUint iRetryCount;

	// Timeout value (passed on to the transport layer)
	TInt iTimeout;
	};

#endif
