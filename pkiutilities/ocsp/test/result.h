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
// Class to handle the acumulation of pass/fail data on the tests
// 
//

/**
 @file 
 @internalTechnology
*/

#ifndef __TOCSP_RESULT_H__
#define __TOCSP_RESULT_H__

#include "e32base.h"

class CTOCSPLogger;

class CTOCSPResult : public CBase
	{
public:
	static CTOCSPResult* NewLC();
	static CTOCSPResult* NewL();
	~CTOCSPResult();

	// Call this when you're starting a new test
	void NewTestL(const TDesC& aName);

	// Then call this to say if it passed
	void ResultL(const TBool aResult);

	// Print text report on conclusions
	void LogSummaryL(CTOCSPLogger& aLog);

private:
	void CheckL() const;

private:
	RPointerArray<HBufC> iNames;
	RArray<TBool> iResults;

	TInt iTotal;
	TInt iPassed;
	TInt iFailed;
	};

#endif // __TOCSP_RESULT_H__
