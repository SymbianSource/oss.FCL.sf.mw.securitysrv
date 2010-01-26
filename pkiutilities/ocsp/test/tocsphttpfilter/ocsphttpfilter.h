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

/**
 @file
 @internalComponent
*/

#ifndef __OCSPHTTPFILTER_H__
#define __OCSPHTTPFILTER_H__

#include <http/mhttpfilter.h>
#include <http/rhttpsession.h>
#include <http/cecomfilter.h>
#include <http.h>

// Forward declarations
class TCustomDataSupplier;

/**
 * An http filter which logs the requests sent to the responder
 * @see MHTTPFilter
 */
class COCSPHTTPFilter : public CEComFilter, public MHTTPFilter
	{
public:
	/** Installs the user-agent filter
		@param  aSession the session to install the filter into.
		@return Pointer to the newly installed plugin
	*/
	static CEComFilter* InstallFilterL(TAny* aSession);
	~COCSPHTTPFilter();

	// Methods from MHTTPFilterBase
	virtual void MHFRunL(RHTTPTransaction aTransaction, const THTTPEvent& aEvent);
	virtual TInt MHFRunError(TInt aError, RHTTPTransaction aTransaction,
							 const THTTPEvent& aEvent);

	// Methods from MHTTPFilter
	virtual void MHFUnload(RHTTPSession aSession, THTTPFilterHandle aHandle);
	virtual void MHFLoad(RHTTPSession aSession, THTTPFilterHandle aHandle);

protected:
	COCSPHTTPFilter();
	void ConstructL(const RHTTPSession& aSession);

	// Log the transaction
	void LogTransactionStartL(const RHTTPTransaction& aTransaction);
	void LogTransactionEndL(const RHTTPTransaction& aTransaction);

	// Read test parameters using Publish & Subscribe method
	void ReadTestParameters(TInt& aNumDelayResp, TInt& aCountDropResp,
			TInt& aCountCorruptHTTPDataHeader, TInt& aCountCorruptHTTPDataBodySizeLarge, TInt& aCountCorruptHTTPDataBodySizeSmall,
			TInt& aCountCorruptOCSPData, 
			TInt& aCountInternalErrorResp, TInt& aCountTryLaterResp,
			TInt& aCountSigValidateFailure);

	// To maintain persistence between retry attempts store the updated counts back
	void WriteTestParameters(TInt aCountDropResp,
			TInt aCountCorruptHTTPDataHeader, TInt aCountCorruptHTTPDataBodySizeLarge, TInt aCountCorruptHTTPDataBodySizeSmall,
			TInt aCountCorruptOCSPData, 
			TInt aCountInternalErrorResp, TInt aCountTryLaterResp,
			TInt aCountSigValidateFailure);

protected:
 	RStringF     	iFilterName; // name of this filter (appears in filter lists)
	TInt         	iLoadCount;  // reference count - unloads filter when reaches 0
	RStringPool		iStringPool; // for the filter name
	RFs			 	iFs;
	RFile 		 	iLogFile;
	TBool		 	iLogLineCompleted; // To prevent double logging of end time
	TBool		 	iDataSupplied;

	// HTTP custom data supplier for returning canned responses
	TCustomDataSupplier* iCustomDataSupplier;
	};

/**
 * Custom HTTP body data supplier to send back canned responses
 * @see MHTTPDataSupplier
 */
class TCustomDataSupplier : public MHTTPDataSupplier
	{
public:
	TCustomDataSupplier(TPtrC8& aData, RHTTPTransaction& aTransaction);

public:
	// Methods from MHTTPDataSupplier
	TBool GetNextDataPart(TPtrC8& aDataPart);
	void ReleaseData();
	TInt OverallDataSize();
	TInt Reset();

private:
	TBool iDataSupplied;
	RHTTPTransaction* iTransaction;
	MHTTPDataSupplier* iOriginalSupplier;
	TPtrC8 iData;
	RBuf8 iCorruptData;
	};

#endif //__OCSPHTTPFILTER_H__

