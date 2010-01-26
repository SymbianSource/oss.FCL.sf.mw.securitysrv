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
// ocsptransport.h
// Declare implementations of OCSP transport interface.  Links the OCSP module
// to the HTTP module.
// 
//

/**
 @file
 @internalTechnology
*/

#ifndef __OCSP_TRANSPORT_HTTP_H__
#define __OCSP_TRANSPORT_HTTP_H__

#include <e32std.h>
#include <ocsp.h>
#include <http.h>
#include <uri8.h>
#include <callbacktimer.h>
#include <http/rexplicithttpsession.h>
#include <ocsprequestandresponse.h>

// Forward declaration
class CTransportTimer;

/**
 * OCSP transport object that uses the default transport appropriate to the
 * request URI.  Currently, only HTTP POST is supported.
 */
class COCSPTransportDefault : public CBase, public MOCSPTransport
	{
public:
	/**
	 * Create a new instance of the transport.
	 * @leave OCSP::KErrInvalidURI If the URI is not supported.
	 */
	IMPORT_C static COCSPTransportDefault* NewL(TUint32& aIap);
	
	virtual ~COCSPTransportDefault();

private:
	COCSPTransportDefault(TUint32& aIap);

	// Methods from MOCSPTransport
	virtual void SendRequest(const TDesC8& aURI, 
							const TDesC8& aRequest,
							const TInt aTimeout,
							TRequestStatus& iStatus);
	virtual void CancelRequest();
	virtual TPtrC8 GetResponse() const;

private:
	// Use string pool for case-independent comparisons....
	enum TTransportScheme
		{
		ETransportSchemeNotSupported,
		ETransportSchemeHTTP
		};

	static TTransportScheme IdentifySchemeL(const TDesC8& aURI);

	void CreateTransportL(const TDesC8& aURI);

private:
	MOCSPTransport* iTransport;
	TTransportScheme iScheme;
	
	TUint32& iIap;
	};


enum THTTPTransportState
	{
	ETransportConnectingState,
	ETransportSendRequestState
	};


/**
 * OCSP transport object that send requests via HTTP GET or POST depending on size of the request
 */
class COCSPTransportHttp :	public CActive, 
							public MOCSPTransport,
							public MHTTPDataSupplier,
							public MHTTPTransactionCallback,
							public MTimerObserver
	{
public:
	/**
	 * Create a new instance of the HTTP POST transport.
	 */
	IMPORT_C static COCSPTransportHttp* NewL(const TDesC8& aUri, TUint32& aIap);
	
	~COCSPTransportHttp();

private:
	COCSPTransportHttp(TUint32& aIap);
	
	void ConstructL(const TDesC8& aUri);

	// Methods from MOCSPTransport
	void SendRequest(const TDesC8& aURI, 
					const TDesC8& aRequest,
					const TInt aTimeout,
					TRequestStatus& iStatus);
	void CancelRequest();
	TPtrC8 GetResponse() const;

	// From CActive
	void RunL();
	void DoCancel();
	TInt RunError(TInt aError);

	// Methods from MHTTPDataSupplier
	TBool GetNextDataPart(TPtrC8& aDataPart);
	void ReleaseData();
	TInt OverallDataSize();
	TInt Reset();

	// Methods from MHTTPTransactionCallback
	void MHFRunL(RHTTPTransaction aTransaction, const THTTPEvent& aEvent);
	TInt MHFRunError(TInt aError, RHTTPTransaction aTransaction, const THTTPEvent& aEvent);

	// Methods from MTransportTimer
	void TimerRun(TInt aError);

private:
	// Internal methods
	void InstallHttpFilterL();
	void DoSendRequestL(const TDesC8& aURI, 
						const TDesC8& aRequest);
	void AddHttpHeadersL();
	void ProcessHeadersL();
	void ProcessDataL();
	void CheckDataCompleteL() const;
	void SetIAPL();
	void Complete(TInt aError);
	void AbortTransaction(TInt aError);

private:
	TRequestStatus* iCallBack;

	// Data to send, from OCSP module
	TPtrC8 iOCSPRequest;

	TUriParser8 iURI;

	RExplicitHTTPSession iHTTPSession;
	RHTTPTransaction iHTTPTransaction;

	// request data
	HBufC8* iUri;
	HBufC8* iRequest;

	// The data in the response
	HBufC8* iResponseData;
	TInt iResponseLength;
	
	TUint32& iIap;
	THTTPTransportState iState;

	// ETrue if HTTP GET method support should be enabled (default is EFalse)
	TBool iUseHTTPGETMethod;

	// Timer class
	CCallbackTimer* iTimer;

	// Flag to indicate whether we were able to contact the responder
	TBool iServerFound;

	// Reponse timeout out value in Milliseconds
	TInt iTimeout;
	};


#endif // __OCSP_TRANSPORT_HTTP_H__
