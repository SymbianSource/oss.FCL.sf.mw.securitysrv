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
// Implement the transport class for using HTTP GET and POST.
// Links the OCSP module to the HTTP module.
// 
//

#include <http/cecomfilter.h>
#include <escapeutils.h>
#include "securitypolicy.h"
#include "ocsppolicy.h"
#include "ocsptransport.h"
#include "panic.h"
#include <tconvbase64.h>
#include "callbacktimer.h"

_LIT8(KOCSPContentTypeRequest, "application/ocsp-request");
_LIT8(KOCSPContentTypeResponse, "application/ocsp-response");
_LIT8(KSlash, "/");

// Min request size for the use of POST method (else use GET method)
const TInt KMinReqSizeForHTTPPOST = 255;

const TInt KMillSecsToMicroSecs = 1000;

// Default value of timeout means it's disabled
const TInt KTimeoutDisabledValue = KTransportDefaultRequestTimeout;

EXPORT_C COCSPTransportHttp* COCSPTransportHttp::NewL(const TDesC8& aUri, TUint32& aIap)
	{
	COCSPTransportHttp* self = new (ELeave) COCSPTransportHttp(aIap);
	CleanupStack::PushL(self);
	self->ConstructL(aUri);
	CleanupStack::Pop(self);
	return self;
	}

COCSPTransportHttp::COCSPTransportHttp(TUint32& aIap)
	: CActive(CActive::EPriorityStandard), iIap(aIap), 
	  iTimeout(KTimeoutDisabledValue)
	{
	CActiveScheduler::Add(this);
	}
	
void COCSPTransportHttp::ConstructL(const TDesC8& aUri)
	{
	// Create the timer object
	iTimer = CCallbackTimer::NewL(*this);

	TUriParser8	uri;
	uri.Parse(aUri);
	iHTTPSession.OpenL(uri, iIap, EFalse);
	
	// Install the HTTP filter (filter UID is specified in swipolicy.ini file)
	InstallHttpFilterL();
	}

COCSPTransportHttp::~COCSPTransportHttp()
	{
	Deque();
	iHTTPTransaction.Close();
	iHTTPSession.Close();

	delete iUri;
	delete iRequest;
	delete iResponseData;
	delete iTimer;
	}

// From MOCSPTransport
void COCSPTransportHttp::SendRequest(const TDesC8& aURI, 
										 const TDesC8& aRequest,
										 const TInt aTimeout,
										 TRequestStatus& aStatus)
	{
	iCallBack = &aStatus;
	aStatus = KRequestPending;
	iServerFound = ETrue;

	delete iUri;
	delete iRequest;

	iTimeout = aTimeout;
	iUri = aURI.Alloc();
	iRequest = aRequest.Alloc();

	// Check for out of memory
	if (iUri == NULL || iRequest == NULL)
		{
		Complete(KErrNoMemory);
		}

	// Take action depending on current state
	if (iState == ETransportConnectingState)
		{
		// Establish a connection
		// Note: Timeout and retries don't apply to this operation
		iState = ETransportConnectingState;
		iHTTPSession.StartConnection(iStatus);
		SetActive();
		}
	else
		{
		// We've connected already, send the request.
		iStatus = KRequestPending;
		TRAPD(err, DoSendRequestL(*iUri, *iRequest));
		if (err == KErrNone)
			{
			// Start the timer (if enabled)
			if (iTimeout > KTimeoutDisabledValue)
				{
				iTimer->After(iTimeout * KMillSecsToMicroSecs);
				}
			SetActive();
			}
		else
			{
			Complete(err);
			}
		}
	}

void COCSPTransportHttp::InstallHttpFilterL()
	{
	// Install filters (if any)
	Swi::RSecPolHandle secPol;
	secPol.OpenLC();
	TUint32 filterId = secPol().OcspHttpHeaderFilter();
	if (filterId != 0)
		{
		TUid headerFilterUid = {filterId};
		CEComFilter::InstallFilterL(iHTTPSession.HTTPSession(), headerFilterUid);
		}
	CleanupStack::PopAndDestroy(&secPol);
	}

void COCSPTransportHttp::DoSendRequestL(const TDesC8& aURI, 
											const TDesC8& aRequest)
	{
	delete iResponseData;
	iResponseData = NULL;
	iOCSPRequest.Set(aRequest);

	// Close previous transaction, if any
	iHTTPTransaction.Close();

	// If the size of the DER encoded request is less than 255 bytes use the GET method (if enabled)
	// else use the POST method
	RBuf8 url8;
	CleanupClosePushL(url8);
	TInt reqSize = aRequest.Length();

	COcspPolicy* ocspPolicy = COcspPolicy::NewL();
	TBool useHTTPGETMethod = ocspPolicy->IsHttpGETMethodEnabled();

	delete ocspPolicy;

	if (useHTTPGETMethod && reqSize < KMinReqSizeForHTTPPOST)
		{
		// Set the OCSP request as part of the url after Base64 and URL encoding
		
		TBase64 base64;
  		
		TInt destLen = ((iOCSPRequest.Length() - 1 ) / 3 + 1) * 4;
 		HBufC8* encodedBuf = HBufC8::NewMaxLC(destLen); // to get the decoded string
   		TPtr8 encodedPtr = encodedBuf->Des();
 		
 		base64.Encode(iOCSPRequest, encodedPtr);
		 				
 		HBufC8* escEncodedReq = EscapeUtils::EscapeEncodeL(encodedPtr, EscapeUtils::EEscapeUrlEncoded);

		CleanupStack::PushL(escEncodedReq);
		url8.CreateL(aURI, aURI.Length() + KSlash().Length() + escEncodedReq->Length());
		// Append a slash only if it's already not present
		if ((aURI.Length() > 0) && (aURI.Right(1) != KSlash))
			{
			url8.Append(KSlash);
			}
		url8.Append(*escEncodedReq);
		CleanupStack::PopAndDestroy(2, encodedBuf);

		if (iURI.Parse(url8))
			{
			User::Leave(OCSP::KErrInvalidURI);
			}

		// Create HTTP transaction, method = GET (default)
		iHTTPTransaction = iHTTPSession().OpenTransactionL(iURI, *this);

		// Add the appropriate Http headers
		AddHttpHeadersL();
		}
	else
		{
		// Use the passed in URL as is
		if (iURI.Parse(aURI))
			{
			User::Leave(OCSP::KErrInvalidURI);
			}

		// Create HTTP transaction, method = POST
		RStringPool stringPool = iHTTPSession().StringPool();
		iHTTPTransaction = iHTTPSession().OpenTransactionL(iURI, *this, stringPool.StringF(HTTP::EPOST,RHTTPSession::GetTable()));

		// Add the appropriate Http headers
		AddHttpHeadersL();

		// Setup the data supplier
		iHTTPTransaction.Request().SetBody(*this);
		}

	// Finally submit the transaction
	iHTTPTransaction.SubmitL();
	CleanupStack::PopAndDestroy(&url8);
	}

void COCSPTransportHttp::AddHttpHeadersL()
	{
	// Get request + it's headers
	RStringPool stringPool = iHTTPSession().StringPool();
	RHTTPRequest request = iHTTPTransaction.Request();
	RHTTPHeaders headers = request.GetHeaderCollection();

	// Host header: done by HTTP module
	// Content-Length header: done by HTTP module

	// Connection:close header:
	headers.SetFieldL(stringPool.StringF(HTTP::EConnection, RHTTPSession::GetTable()), stringPool.StringF(HTTP::EClose, RHTTPSession::GetTable()));

	// Content-Type header:
	RStringF ocspRequest = stringPool.OpenFStringL(KOCSPContentTypeRequest);
	CleanupClosePushL(ocspRequest);
	THTTPHdrVal contentTypeVal(ocspRequest);
	headers.SetFieldL(stringPool.StringF(HTTP::EContentType,RHTTPSession::GetTable()), contentTypeVal);
	CleanupStack::PopAndDestroy(&ocspRequest);
	}

// From MOCSPTransport
void COCSPTransportHttp::CancelRequest()
	{
	Cancel();
	}


// From MOCSPTransport
TPtrC8 COCSPTransportHttp::GetResponse() const
	{
	__ASSERT_ALWAYS(iResponseData, Panic(KErrNotReady));

	return iResponseData->Des();
	}


// From MHTTPDataSupplier
TBool COCSPTransportHttp::GetNextDataPart(TPtrC8& aDataPart)
	{
	aDataPart.Set(iOCSPRequest);
	return ETrue;  // This is the last chunk
	}


// From MHTTPDataSupplier
void COCSPTransportHttp::ReleaseData()
	{
	// Nothing to do - from the OCSP side, ownership of the outgoing data is sorted already
	}


// From MHTTPDataSupplier
TInt COCSPTransportHttp::OverallDataSize()
	{
	return iOCSPRequest.Length();
	}


// From MHTTPDataSupplier
TInt COCSPTransportHttp::Reset()
	{
	return KErrNotSupported;
	}

// From MHTTPTransactionCallback
void COCSPTransportHttp::MHFRunL(RHTTPTransaction aTransaction, const THTTPEvent& aEvent)
	{
	__ASSERT_ALWAYS(aTransaction == iHTTPTransaction, Panic(KErrArgument));

	// Either ESucceeded or EFailed *will* happen
	switch (aEvent.iStatus)
		{
		case THTTPEvent::EGotResponseHeaders:
			ProcessHeadersL();
			break;
		case THTTPEvent::EGotResponseBodyData:
			ProcessDataL();
			break;
		case THTTPEvent::EResponseComplete:
			CheckDataCompleteL();
			break;
		case THTTPEvent::EFailed:
			{
			// Stop the timer
			iTimer->Cancel();
			TRequestStatus* reqstatus = &iStatus;
			if (iServerFound)
				{
				User::RequestComplete(reqstatus, OCSP::KErrTransportFailure);
				}
			else
				{
				User::RequestComplete(reqstatus, OCSP::KErrServerNotFound);
				}
			}
			break;
		case THTTPEvent::ESucceeded:
			{
			// Stop the timer
			iTimer->Cancel();
			CheckDataCompleteL();
			SetIAPL();
			TRequestStatus* status = &iStatus;
			User::RequestComplete(status, KErrNone);
			}
			break;
		case KErrTimedOut:
			// HTTP timed out - this really means it couldn't find the server mentioned in the url
			// Remember this error so we pass it up correctly when we receive "THTTPEvent::EFailed" event
			iServerFound = EFalse;
			break;
		default:
			// Some other event (or error).
			// Do nothing - HTTP module will cause EFailed or ESucceeded to occur
			break;
		}
	}
	
void COCSPTransportHttp::SetIAPL()
	{
	const RStringPool strPool = iHTTPSession().StringPool();
	RHTTPConnectionInfo connectionInfo = iHTTPSession().ConnectionInfo();
	THTTPHdrVal hdrVal;
	if (connectionInfo.Property(strPool.StringF(HTTP::EHttpSocketConnection, RHTTPSession::GetTable()),
			hdrVal)) 
		{
		RConnection* connection =  reinterpret_cast<RConnection*>(hdrVal.Int());
		_LIT(KIapId, "IAP\\Id");
		User::LeaveIfError(connection->GetIntSetting(KIapId, iIap));
		}	
	}


// From MHTTPTransactionCallback
TInt COCSPTransportHttp::MHFRunError(TInt aError, RHTTPTransaction aTransaction, const THTTPEvent& /*aEvent*/)
	{
	__ASSERT_ALWAYS(aTransaction == iHTTPTransaction, Panic(KErrArgument));

	// Process error (otherwise, we get a panic)
	AbortTransaction(aError);
	return KErrNone;
	}

// Methods from MCallbackTimer
void COCSPTransportHttp::TimerRun(TInt aError)
	{
	if (aError == KErrNone)
		{
		// Handle the timeout
		iHTTPTransaction.Cancel();
		TRequestStatus* status = &iStatus;
		User::RequestComplete(status, OCSP::KErrTransportTimeout);
		}
	else
		{
		// Propagate the error upward
		TRequestStatus* status = &iStatus;
		User::RequestComplete(status, aError);
		}
	}

// The headers have been received - check them out
void COCSPTransportHttp::ProcessHeadersL()
	{
	RHTTPResponse response = iHTTPTransaction.Response();
	RHTTPHeaders headers = response.GetHeaderCollection();
	RStringPool stringPool = iHTTPSession().StringPool();

	// Check content type
	RStringF ocspResponse = stringPool.OpenFStringL(KOCSPContentTypeResponse);
	CleanupClosePushL(ocspResponse);
	RStringF contentTypeString = stringPool.StringF(HTTP::EContentType,RHTTPSession::GetTable());
	THTTPHdrVal contentTypeVal;
	TInt error = headers.GetField(contentTypeString, 0, contentTypeVal);
	if (error != KErrNone 
		|| contentTypeVal.Type() != THTTPHdrVal::KStrFVal
		|| contentTypeVal.StrF() != ocspResponse)
		{
		User::Leave(OCSP::KErrTransportFailure);
		}
	CleanupStack::PopAndDestroy(); // close ocspResponse

	// Check content length - make descriptor ready to recieve data
	RStringF contentLengthString = stringPool.StringF(HTTP::EContentLength,RHTTPSession::GetTable());
	THTTPHdrVal contentLengthVal;
	error = headers.GetField(contentLengthString, 0, contentLengthVal);
	if (error == KErrNone
		&& contentLengthVal.Type() == THTTPHdrVal::KTIntVal)
		{
		// Make descriptor ready to receive response data
		__ASSERT_ALWAYS(iResponseData == NULL, Panic(KErrAlreadyExists));
		iResponseLength = contentLengthVal.Int();
		iResponseData = HBufC8::NewL(iResponseLength);
		}
	else
		{
		// No Contents-Length field in headers, or wrong data type
		User::Leave(OCSP::KErrTransportFailure);
		}
	}

void COCSPTransportHttp::ProcessDataL()
	{
	if (iResponseData)
		{
		// Some data has come in - copy it into our descriptor
		MHTTPDataSupplier* body = iHTTPTransaction.Response().Body();
		if (!body)
			{
			User::Leave(OCSP::KErrTransportFailure);
			}

		TPtrC8 dataChunk;
		TBool finished = body->GetNextDataPart(dataChunk);
		if (iResponseLength - iResponseData->Length() >= dataChunk.Length())
			{
			iResponseData->Des().Append(dataChunk);			
			body->ReleaseData();
			}
		else
			{
			// Data is longer than Contents-Length header said it would be - error
			body->ReleaseData();
			User::Leave(OCSP::KErrTransportFailure);
			}

		if (finished)
			{
			CheckDataCompleteL();
			}
		}		
	}

void COCSPTransportHttp::CheckDataCompleteL() const
	{
	if (iResponseLength != iResponseData->Length())
		{
		User::Leave(OCSP::KErrTransportFailure);
		}
	}

void COCSPTransportHttp::Complete(TInt aError)
	{
	if (iCallBack)
		{
		// If something went wrong, remove the dodgy response data
		if (aError != KErrNone)
			{
			delete iResponseData;
			iResponseData = NULL;
			}

		// Sets iCallBack back to NULL, so we can only do this once
		User::RequestComplete(iCallBack, aError);
		}
	}

void COCSPTransportHttp::RunL()
	{
	User::LeaveIfError(iStatus.Int());
	switch(iState)
		{
	case ETransportConnectingState:
		iState = ETransportSendRequestState;
		iStatus = KRequestPending;
		DoSendRequestL(*iUri, *iRequest);
		// Start the timer (if enabled)
		if (iTimeout > KTimeoutDisabledValue)
			{
			iTimer->After(iTimeout * KMillSecsToMicroSecs);
			}
		SetActive();
		break;

	case ETransportSendRequestState:
		// All OK
		Complete(KErrNone);
		break;
	default:
		User::Leave(KErrNotSupported);
		break;
		};
	}

TInt COCSPTransportHttp::RunError(TInt aError)
	{
	// If we failed in the transport connection state, return a
	// transport error. Otherwise, just propogate the error recieved.
	if (iState == ETransportConnectingState)
		{
		Complete(OCSP::KErrTransportFailure);
		}
	else
		{
		Complete(aError);
		}
	return KErrNone;
	}

void COCSPTransportHttp::DoCancel()
	{
	AbortTransaction(KErrCancel);
	Complete(KErrCancel);
	}

void COCSPTransportHttp::AbortTransaction(TInt aError)
	{
	switch(iState)
		{
		case ETransportConnectingState:
		iHTTPSession.CancelStart();
		break;

		case ETransportSendRequestState:
		iTimer->Cancel();
		iHTTPTransaction.Cancel();

		//DEF101099 Fix - In a very rare situation after starting invocation of this DoCancel(), the asynchronous request
		//is getting completed normally. In that case(iStatus!=KRequestPending) no need call this request complete codes,
		//otherwise which leads to a "Stray Signal" and ending up in a Panic - E32USER-CBase 46.
		if(iStatus == KRequestPending)
			{
			// Thanks to HTTP not exporting a callback for "DoCancel()" we complete the request here
			TRequestStatus* status = &iStatus;
			User::RequestComplete(status, aError);
			}	
		break;
		}
	}

