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

#include "result.h"
#include "logger.h"

CTOCSPResult* CTOCSPResult::NewLC()
	{
	CTOCSPResult* self = NewL();
	CleanupStack::PushL(self);
	return self;
	}


CTOCSPResult* CTOCSPResult::NewL()
	{
	CTOCSPResult* self = new (ELeave) CTOCSPResult;
	return self;
	}


CTOCSPResult::~CTOCSPResult()
	{
	iNames.ResetAndDestroy();
	iResults.Close();
	}


// Call this when you're starting a new test
void CTOCSPResult::NewTestL(const TDesC& aName)
	{
	CheckL();

	HBufC* name = aName.AllocLC();
	User::LeaveIfError(iNames.Append(name));
	CleanupStack::Pop(name);

	iTotal++;
	}


// Then call this to say if it passed
void CTOCSPResult::ResultL(const TBool aResult)
	{
	User::LeaveIfError(iResults.Append(aResult));

	if (aResult)
		{
		iPassed++;
		}
	else
		{
		iFailed++;
		}

	CheckL();
	}


void CTOCSPResult::CheckL() const
	{
	if (iTotal != iPassed + iFailed
		|| iTotal != iNames.Count()
		|| iTotal != iResults.Count())
		{
		User::Leave(KErrCorrupt);
		}
	}


void CTOCSPResult::LogSummaryL(CTOCSPLogger& aLog)
	{
	// Allow one less test result than test started,
	// assume this meanswe crashed out of last test
	if (iTotal == (iPassed + iFailed + 1))
		{
		User::LeaveIfError(iResults.Append(EFalse));
		++iFailed;
		}

	CheckL();

	aLog.LogL(_L("Run: "));
	aLog.LogL(iTotal);
	aLog.LogL(_L("\nPassed: "));
	aLog.LogL(iPassed);

	aLog.LogL(_L("\n"));
	aLog.LogL(iFailed);
	aLog.LogL(_L(" tests failed out of "));
	aLog.LogL(iTotal);
	aLog.LogL(_L("\n"));

	aLog.LogL(_L("Failed tests: "));
	TInt tally = 0;
	for (TInt i = 0; i < iTotal; ++i)
		{
		if (!iResults[i])
			{
			aLog.LogL(*iNames[i]);
			aLog.LogL(_L(" ("));
			aLog.LogL(i);
			aLog.LogL(_L(")"));
			
			if (++tally < iFailed)
				{
				aLog.LogL(_L(", "));
				}
			else
				{
				aLog.LogL(_L("\n"));
				}
			}
		}
	aLog.LogL(_L("\r\n</pre></body></html>\r\n"));	
	}
