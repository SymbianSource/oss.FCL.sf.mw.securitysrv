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
//

/**
 @file 
 @internalTechnology
*/
 
#ifndef __TOCSP_MAIN_H__
#define __TOCSP_MAIN_H__

#include <e32base.h>
#include <f32file.h>

#include "TEFparser.h"

/** UID for setting cert applicability. */

const TInt KTOCSP_UID = 0x1000A405;

class CTOCSPLogger;
class CTOCSPResult;
class CTOCSPScript;
class CCertUtils;

/**
 * Runs OCSP test script.
 */

class CTOCSPMain : public CActive
	{
public:	

	static CTOCSPMain* NewLC(const TDesC& aScriptFile, const TDesC& aLogFile);
	~CTOCSPMain();

	void RunNormalTestsL();
	void RunOOMTestsL();
	const TPtrC& getSectionData() { return iSectionData; };	
	
	TBool getScriptFlag() { return iScriptFlag; };

private:

	enum TState
		{
		ERunningNormalTest,
		ERunningOOMTest
		};

	CTOCSPMain(const TDesC& aScriptFile);
	void ConstructL(const TDesC& aLogFile);
	void RunTestsL(TState aState);

	void FinishedNormalTestL();
	void StartNextOOMTestL();
	void FinishedOOMTestL();
	TBool CheckOOMTestResultL();
	void ResetTestL();
	void ReportErrorL();

	// Implementation of CActive methods
	void RunL();
	TInt RunError(TInt aError);
	void DoCancel();

	TState			iState;
	RFs				iFs;
	CCertUtils*		iCertUtils;
	const TDesC&	iScriptFile;
	CTOCSPLogger*	iLog;
	TInt			iTransaction;
	TInt			iFailPoint;
	TInt			iError;
	CTOCSPResult*	iResult;
	CTOCSPScript*	iTest;
	TInt			iCompletions;
	
	TPtrC			iSectionData;
	TBool 			iScriptFlag;
	TBuf16<256> 	iTocspTestFile;
	};

#endif
