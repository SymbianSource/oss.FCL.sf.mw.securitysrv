// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
//

#include "ocsphttpfilter.h"
#include <e32cmn.h>
#include <e32std.h>
#include <e32property.h>
#include <http/rhttptransaction.h>
#include <http/rhttpheaders.h>
#include <http/thttphdrval.h>
#include <httpstringconstants.h>
#include "testfilterparameters.h"

// Filter name used to register the filter
_LIT8(KFilterName,"tocsphttpfilter");

// Filter log file read by the test code
_LIT(KFilterLogFileName, "\\tocsphttpfilter.log");

// HTTPMethod StartTimeInMicroseconds
_LIT8(KFilterLogFormat1, "%S %Ld");
// StopTimeInMicroseconds
_LIT8(KFilterLogFormat2, " %Ld\n"); 

// Canned responses
_LIT8(KCannedResponseInteralError, "\x30\x03\x0a\x01\x02");
_LIT8(KCannedResponseTryLater, "\x30\x03\x0a\x01\x03");
_LIT8(KCannedResponseCorruptOCSPData, "\x30\x03\x0a\x01\xf0");

// A non-existent ocsp responder. Redirect requests to this
// responder to simulate a missing response
_LIT8(KNonExistentServer, "http://42.042.042.042:0420/");

// HTTP response header for content-type (canned corrupted)
_LIT8(KOCSPContentTypeResponseCorrupted, "application/ocsp-reponse");

// Byte offset (+1) of Responder ID field of OCSPResponse
const TInt KOCSPResponderIDOffset = 42;

const TInt KTimeMilliToMicro = 1000;

COCSPHTTPFilter::COCSPHTTPFilter()
		: iLogLineCompleted(ETrue)
	{
	}

CEComFilter* COCSPHTTPFilter::InstallFilterL(TAny* aSession)
	{
	RHTTPSession* session = reinterpret_cast<RHTTPSession*>(aSession);
	COCSPHTTPFilter* filter = new (ELeave) COCSPHTTPFilter();
	CleanupStack::PushL(filter);
	filter->ConstructL(*session);
	CleanupStack::Pop(filter);
	return filter;
	}

void COCSPHTTPFilter::ConstructL(const RHTTPSession& aSession)
	{
	iStringPool = aSession.StringPool();
	iFilterName = iStringPool.OpenFStringL(KFilterName);
	// Register the filter for submit events
	aSession.FilterCollection().AddFilterL(*this, THTTPEvent::EAnyTransactionEvent, MHTTPFilter::EClientFilters, iFilterName);
	User::LeaveIfError(iFs.Connect());
	User::LeaveIfError(iLogFile.Replace(iFs, KFilterLogFileName, EFileShareAny|EFileWrite));
	}

COCSPHTTPFilter::~COCSPHTTPFilter()
	{
	iFilterName.Close();
	iLogFile.Close();
	iFs.Close();
	delete iCustomDataSupplier;
	}

void COCSPHTTPFilter::MHFLoad(RHTTPSession, THTTPFilterHandle)
	{
	++iLoadCount;
	}

void COCSPHTTPFilter::MHFUnload(RHTTPSession /*aSession*/, THTTPFilterHandle)
	{
	if (--iLoadCount)
		return;

	delete this;
	}

void COCSPHTTPFilter::MHFRunL(RHTTPTransaction aTransaction,
									const THTTPEvent& aEvent)
	{
	// Read test parameters
	TInt countDropResp, numDelayResp;
	TInt countCorruptHTTPDataHeader, countCorruptHTTPDataBodySizeLarge, countCorruptHTTPDataBodySizeSmall;
	TInt countCorruptOCSPData;
	TInt countInternalErrorResp, countTryLaterResp;
	TInt countSigValidateFailure;
	ReadTestParameters(numDelayResp, countDropResp,
			countCorruptHTTPDataHeader, countCorruptHTTPDataBodySizeLarge, countCorruptHTTPDataBodySizeSmall,
			countCorruptOCSPData,
			countInternalErrorResp, countTryLaterResp, 
			countSigValidateFailure);

	switch (aEvent.iStatus)
		{
	case THTTPEvent::ESubmit:
		// Start of the HTTP transaction
		iDataSupplied = EFalse;
		iLogLineCompleted = EFalse;
		LogTransactionStartL(aTransaction);
		
		if (countDropResp > 0)
			{
			// Drop the request
			--countDropResp;
			TUriParser8 uri;
			uri.Parse(KNonExistentServer);
			aTransaction.Request().SetURIL(uri);
			}
		break;
	case THTTPEvent::EGotResponseHeaders:
		{
		RHTTPResponse response = aTransaction.Response();
		RHTTPHeaders headers = response.GetHeaderCollection();
		RStringPool stringPool = aTransaction.Session().StringPool();

		// Modify http body size if we plan to modify the body (response data) later
		if (countInternalErrorResp > 0 || countTryLaterResp > 0 || countCorruptOCSPData > 0) // Common code since size is same for all canned responses
			{
			// Create the data supplier
			delete iCustomDataSupplier;
			iCustomDataSupplier = NULL;
			TPtrC8 ptr(KCannedResponseInteralError);
			RStringF contentLengthString = stringPool.StringF(HTTP::EContentLength, RHTTPSession::GetTable());
			if (countTryLaterResp > 0)
				{
				ptr.Set(KCannedResponseTryLater);
				}
			else if (countCorruptOCSPData > 0)
				{
				ptr.Set(KCannedResponseCorruptOCSPData);
				}
			iCustomDataSupplier = new (ELeave) TCustomDataSupplier(ptr, aTransaction);
			THTTPHdrVal contentLengthVal;
			TInt size = iCustomDataSupplier->OverallDataSize();
			contentLengthVal.SetInt(size);
			headers.RemoveFieldPart(contentLengthString, 0);
			headers.SetFieldL(contentLengthString, contentLengthVal);
			}
		// Create and keep a data supplier if we plan to corrupt the data leading to a signature validation failure
		if (countSigValidateFailure > 0)
			{
			// Create the data supplier
			delete iCustomDataSupplier;
			iCustomDataSupplier = NULL;
			TPtrC8 ptr(KNullDesC8);
			iCustomDataSupplier = new (ELeave) TCustomDataSupplier(ptr, aTransaction);
			}
		if (countCorruptHTTPDataHeader > 0)
			{
			// Corrupt the header
			RStringF ocspResponse = stringPool.OpenFStringL(KOCSPContentTypeResponseCorrupted);
			CleanupClosePushL(ocspResponse); 
			RStringF contentTypeString = stringPool.StringF(HTTP::EContentType, RHTTPSession::GetTable());
			THTTPHdrVal contentTypeVal;
			contentTypeVal.SetStrF(ocspResponse);
			headers.RemoveFieldPart(contentTypeString, 0);
			headers.SetFieldL(contentTypeString, contentTypeVal);
			CleanupStack::PopAndDestroy(&ocspResponse);
			--countCorruptHTTPDataHeader;
			}
		if (countCorruptHTTPDataBodySizeLarge > 0 || countCorruptHTTPDataBodySizeSmall > 0)
			{
			// Corrupt the body size recorded in the header
			RStringF contentLengthString = stringPool.StringF(HTTP::EContentLength, RHTTPSession::GetTable());
			THTTPHdrVal contentLengthVal;
			TInt err = headers.GetField(contentLengthString, 0, contentLengthVal);
			if ((err == KErrNone) && (contentLengthVal.Type() == THTTPHdrVal::KTIntVal))
				{
				TInt bodySize = contentLengthVal.Int();
				if (countCorruptHTTPDataBodySizeLarge > 0)
					{
					--bodySize;
					--countCorruptHTTPDataBodySizeLarge;
					}
				else
					{
					++bodySize;
					--countCorruptHTTPDataBodySizeSmall;
					}
				contentLengthVal.SetInt(bodySize);
				headers.RemoveFieldPart(contentLengthString, 0);
				headers.SetFieldL(contentLengthString, contentLengthVal);
				}
			}
		}
		break;
	case THTTPEvent::EGotResponseBodyData:
		if (iCustomDataSupplier && (countInternalErrorResp > 0 || countTryLaterResp > 0 || countCorruptOCSPData > 0 || countSigValidateFailure > 0))
			{
			aTransaction.Response().RemoveBody();
			aTransaction.Response().SetBody(*iCustomDataSupplier);
			// Make sure state machine goes on
			aTransaction.Response().SetStatusCode(THTTPEvent::EResponseComplete);
			}
		break;
	case THTTPEvent::ESucceeded:
	case THTTPEvent::EFailed:
		// Delay response (numDelayResp is in milliseconds)
		if (numDelayResp > 0)
			{
			User::After(numDelayResp * KTimeMilliToMicro);
			}
	// Deliberate fall through case
	case THTTPEvent::ECancel:
		if (!iLogLineCompleted)
			{
			// End of the HTTP transaction
			iLogLineCompleted = ETrue;
			LogTransactionEndL(aTransaction);
			}
		// Since EGotResponseBodyData event can happen more than once per transaction ensure to decrement
		// counters only once
		if (!iDataSupplied)
			{
			if (countInternalErrorResp > 0)
				{
				--countInternalErrorResp;
				}
			if (countTryLaterResp > 0)
				{
				--countTryLaterResp;
				}
			if (countCorruptOCSPData > 0)
				{
				--countCorruptOCSPData;
				}
			if (countSigValidateFailure > 0)
				{
				--countSigValidateFailure;
				}
			iDataSupplied = ETrue;
			}
		break;
	default:
		break;
		}

	// Write the parameters back to keep them persistent between retries
	WriteTestParameters(countDropResp, 
			countCorruptHTTPDataHeader, countCorruptHTTPDataBodySizeLarge, countCorruptHTTPDataBodySizeSmall, 
			countCorruptOCSPData, 
			countInternalErrorResp, countTryLaterResp,
			countSigValidateFailure);
	}

TInt COCSPHTTPFilter::MHFRunError(TInt /*aError*/,
										RHTTPTransaction aTransaction,
										const THTTPEvent& /*aEvent*/)
	{
	// If anything left, we've run out of memory or something
	// similarly catastrophic has gone wrong.
	aTransaction.Fail();
	return KErrNone;
	}

// Logs the transaction method used and the current (start) time
void COCSPHTTPFilter::LogTransactionStartL(const RHTTPTransaction& aTransaction)
	{
	// Get the transaction method being used (GET/POST) and log it
	RHTTPRequest request = aTransaction.Request();
	RStringF method = request.Method();
	TTime time;
	time.HomeTime();
	TInt64 intTime = time.Int64();
	RBuf8 logText;
	logText.CreateL(255);
	CleanupClosePushL(logText);
	logText.Format(KFilterLogFormat1, &method.DesC(), intTime);
	User::LeaveIfError(iLogFile.Write(logText));
	CleanupStack::PopAndDestroy(&logText);
	}

// Logs the current (end) time
void COCSPHTTPFilter::LogTransactionEndL(const RHTTPTransaction& /*aTransaction*/)
	{
	TTime time;
	time.HomeTime();
	TInt64 intTime = time.Int64();
	RBuf8 logText;
	logText.CreateL(255);
	CleanupClosePushL(logText);
	logText.Format(KFilterLogFormat2, intTime);
	User::LeaveIfError(iLogFile.Write(logText));
	CleanupStack::PopAndDestroy(&logText);
	}

// Read test parameters using Publish & Subscribe method
void COCSPHTTPFilter::ReadTestParameters(TInt& aNumDelayResp, TInt& aCountDropResp,
		TInt& aCountCorruptHTTPDataHeader, TInt& aCountCorruptHTTPDataBodySizeLarge, TInt& aCountCorruptHTTPDataBodySizeSmall,
		TInt& aCountCorruptOCSPData, 
		TInt& aCountInternalErrorResp, TInt& aCountTryLaterResp,
		TInt& aCountSigValidateFailure)
	{
	// Set default values which will be used if P&S doesn't exist
	aNumDelayResp = aCountDropResp = 
	aCountCorruptHTTPDataHeader = aCountCorruptHTTPDataBodySizeLarge = aCountCorruptHTTPDataBodySizeSmall = 
		aCountCorruptOCSPData = 
		aCountInternalErrorResp = aCountTryLaterResp = 
		aCountSigValidateFailure = 0;

	TUid categoryUid = TUid::Uid(KFilterParametersCategoryUID);
	RProperty::Get(categoryUid, KFilterParameterNumDelayResp, aNumDelayResp);
	RProperty::Get(categoryUid, KFilterParameterCountDropResp, aCountDropResp);
	RProperty::Get(categoryUid, KFilterParameterCountCorruptHTTPDataHeader, aCountCorruptHTTPDataHeader);
	RProperty::Get(categoryUid, KFilterParameterCountCorruptHTTPDataBodySizeLarge, aCountCorruptHTTPDataBodySizeLarge);
	RProperty::Get(categoryUid, KFilterParameterCountCorruptHTTPDataBodySizeSmall, aCountCorruptHTTPDataBodySizeSmall);
	RProperty::Get(categoryUid, KFilterParameterCountCorruptOCSPData, aCountCorruptOCSPData);
	RProperty::Get(categoryUid, KFilterParameterCountInternalErrorResp, aCountInternalErrorResp);
	RProperty::Get(categoryUid, KFilterParameterCountTryLaterResp, aCountTryLaterResp);
	RProperty::Get(categoryUid, KFilterParameterCountSigValidateFailure, aCountSigValidateFailure);
	}

// To maintain persistence between retry attempts store the updated counts back
void COCSPHTTPFilter::WriteTestParameters(TInt aCountDropResp,
		TInt aCountCorruptHTTPDataHeader, TInt aCountCorruptHTTPDataBodySizeLarge, TInt aCountCorruptHTTPDataBodySizeSmall,
		TInt aCountCorruptOCSPData, 
		TInt aCountInternalErrorResp, TInt aCountTryLaterResp,
		TInt aCountSigValidateFailure)
	{
	TUid categoryUid = TUid::Uid(KFilterParametersCategoryUID);
	RProperty::Set(categoryUid, KFilterParameterCountDropResp, aCountDropResp);
	RProperty::Set(categoryUid, KFilterParameterCountCorruptHTTPDataHeader, aCountCorruptHTTPDataHeader);
	RProperty::Set(categoryUid, KFilterParameterCountCorruptHTTPDataBodySizeLarge, aCountCorruptHTTPDataBodySizeLarge);
	RProperty::Set(categoryUid, KFilterParameterCountCorruptHTTPDataBodySizeSmall, aCountCorruptHTTPDataBodySizeSmall);
	RProperty::Set(categoryUid, KFilterParameterCountCorruptOCSPData, aCountCorruptOCSPData);
	RProperty::Set(categoryUid, KFilterParameterCountInternalErrorResp, aCountInternalErrorResp);
	RProperty::Set(categoryUid, KFilterParameterCountTryLaterResp, aCountTryLaterResp);
	RProperty::Set(categoryUid, KFilterParameterCountSigValidateFailure, aCountSigValidateFailure);
	}

TCustomDataSupplier::TCustomDataSupplier(TPtrC8& aData, RHTTPTransaction& aTransaction)
	{
	iData.Set(aData);
	iTransaction = &aTransaction;
	iDataSupplied = EFalse;
	iOriginalSupplier = aTransaction.Response().Body();
	}

// Methods from MHTTPDataSupplier
TBool TCustomDataSupplier::GetNextDataPart(TPtrC8& aDataPart)
	{
	// Check if we need to simulate a signature validation failure
	if (iData == KNullDesC8)
		{
		// Get the original data
		iOriginalSupplier->GetNextDataPart(aDataPart);
		iCorruptData.Create(aDataPart);
		// Corrupt it (byte is part of ResponderID field)
		--iCorruptData[KOCSPResponderIDOffset];
		// Send it along
		aDataPart.Set(iCorruptData);
		}
	else
		{
		// Consume the original data and pass back canned response
		iOriginalSupplier->GetNextDataPart(aDataPart);
		if (!iDataSupplied)
			{
			aDataPart.Set(iData);
			iDataSupplied = ETrue;
			}
		else
			{
			// no data
			aDataPart.Set(KNullDesC8);
			}
		}
	return ETrue;
	}

void TCustomDataSupplier::ReleaseData()
	{
	iCorruptData.Close();
	// Call original method to ensure it does it's work to let the state machine go on
	iOriginalSupplier->ReleaseData();
	// We are done supplying canned response so restore original supplier
	iTransaction->Response().SetBody(*iOriginalSupplier);
	}

TInt TCustomDataSupplier::OverallDataSize()
	{
	return iData.Length();
	}

TInt TCustomDataSupplier::Reset()
	{
	return KErrNotSupported;
	}
