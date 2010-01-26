// Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// Test transport object.  May be used in place of the automatically-constructed
// transport object from OCSP.DLL
// Allows a file to be specified, the contents of which will be returned as the response.
// 
//

/**
 @file 
 @internalTechnology
*/

#ifndef __TOCSP_TRANSPORT_H__
#define __TOCSP_TRANSPORT_H__

#include <e32base.h>
#include <ocsp.h>
#include <s32file.h>

#include "ocsprequestandresponse.h"
namespace TOCSP
	{
	// Some test code specific status codes
	
	enum TStatus
		{
		ETooManyTransactions = 200, // Client made more requests than we had responses for
		EURIMismatch,				// Request URI didn't match expected
		ERequestMismatch			// Request data didn't match expectedo		
		};
	}

class CTOCSPTransport : public CBase, public MOCSPTransport
	{
public:

	/**
	 * Create a test transport object.
	 * @param aResponseFile The filename of the response to return.
	 * @param aRequestFile The name of the request file to compare the request
	 * data against, or null.
	 */
	static CTOCSPTransport* NewL(const TDesC& aResponseFile, const TDesC* aRequestFile);

private:
	// From MOCSPTransport
	~CTOCSPTransport();
	void SendRequest(const TDesC8& aURI, const TDesC8& aRequest, const TInt aTimeout, TRequestStatus& aStatus);
	void CancelRequest();
	TPtrC8 GetResponse() const;

private:
	void DoSendRequestL(const TDesC8& aURI, const TDesC8& aRequest);
	void CheckRequestL(const TDesC8& aURI, const TDesC8& aRequest);
	void ConstructL(const TDesC& aResponseFile, const TDesC* aRequestFile);
	HBufC8* ReadDataLC(RFileReadStream& aStream);

private:
	RFs				 iFs;
	RFileReadStream  iResponseStream;
	RFileReadStream* iRequestStream;
	TInt			 iTransactions;	
	TInt			 iIndex;
	HBufC8*			 iResponseData;
	TRequestStatus*  iCallBack;
	};

#endif // __TOCSP_TRANSPORT_H__
