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
// Declare CTOCSPScript.  This opens a file, starts a scheduler and runs a CIdle object.
// Each tick takes a line from the file, parses it into tokens and gives these to 
// the command processor.
// 
//

/**
 @file 
 @internalTechnology
*/

#ifndef __TOCSP_SCRIPT_H__
#define __TOCSP_SCRIPT_H__

#include <e32base.h>
#include <f32file.h>

#include "logger.h"
#include "TEFparser.h"
#include "main.h"

class CTOCSPCommand;
class CTOCSPResult;

// At most this many chars per line in input file, thanks.
const TInt KMaxInputFileLineLength = 256;

/**
 * This class provides a convenient interface to the test script.
 */
class CTOCSPScript : public CActive
	{
public:
	/**
	 * This function creates a new CTOCSPScript instance and pushes it on the
	 * cleanup stack.
	 * @param aLog the logger which writes the log into the log file and the cosole.
	 * @param aResult pass/fail result data on the tests
	 * @param aInputFileName The name of the test script file we want to access.
	 * @return A new instance of CTOCSPScript.
	 */
	static CTOCSPScript* NewLC(CTOCSPLogger& aLog, 
							   CTOCSPResult& aResult, 
							   const TDesC& aInputFileName,
							   TInt aTransaction = 0);

	static CTOCSPScript* NewL(CTOCSPLogger& aLog, 
							  CTOCSPResult& aResult, 
							  const TDesC& aInputFileName,
							  TInt aTransaction = 0);
//   script file parser
//***********************************************************
	static CTOCSPScript* NewL(CTOCSPLogger& aLog, 
							  CTOCSPResult& aResult,
							  TInt aTransaction = 0);
	
	static CTOCSPScript* NewLC(CTOCSPLogger& aLog, 
							   CTOCSPResult& aResult,
							   TInt aTransaction = 0);
//***********************************************************

	
	~CTOCSPScript();

	void RunTestL(TRequestStatus& aStatus);

    // Get the number of transactions performed
    TInt TransactionCount();

	/** Set whether to print informational messages. */
	void SetVerbose(TBool aVerbose);	

	/** Get the number of lines we have processed. */
	TInt LinesProcessed() const;

	/** Get the last line processed. */
	const TDesC& LastLine() const;

	void RunL();
	void DoCancel();
    TInt RunError(TInt aError);

private:
	CTOCSPScript(CTOCSPLogger& aLog);
	void ConstructL(const TDesC& aInputFileName, CTOCSPResult& aResult, TInt aTransaction);
	
	void ConstructL(CTOCSPResult& aResult, TInt aTransaction);

	void ReadLineL();
	
private:
	CTOCSPLogger& iLog;
	
	CTOCSPCommand* iCommand;

	TInt iError; // error code to return to client

	// File server stuff
	RFs iFs;
	RFile iFile;
	TFileText iLineReader;
	TInt iLinesProcessed;
	TBuf<KMaxInputFileLineLength> iLine;
	TRequestStatus* iClientRequestStatus;
	TInt iStartPos;
	};

#endif
