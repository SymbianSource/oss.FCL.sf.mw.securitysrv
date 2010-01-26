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
// Class to handle output from the test harness.  All output goes to the log file,
// but screen output can be more controlled.
// 
//

/**
 @file 
 @internalTechnology
*/

#ifndef __TOCSP_LOGGER_H__
#define __TOCSP_LOGGER_H__

#include <e32base.h>
#include <f32file.h>

/**
 * This class provides a logging functionality. It logs everything in a file
 * and optionally displays it on the console.
 */
class CTOCSPLogger : public CBase
	{
public:
	static CTOCSPLogger* NewLC(const TDesC& aLogFile);
	static CTOCSPLogger* NewL(const TDesC& aLogFile);
	~CTOCSPLogger();

	// Log to the file, and maybe echo to screen.
	// Sets echo mode from here on
	void LogL(const TDesC& aText, const TBool aEcho);
	void LogL(const TDesC8& aText, const TBool aEcho);
	void LogL(const TInt aInt, const TBool aEcho);
	
	// Log to file, and maybe to screen, depending on current echo mode
	void LogL(const TDesC& aText);
	void LogL(const TDesC8& aText);
	void LogL(const TInt aInt);
	void NewLineL();

	// Prompt user and wait - only if echo is on
	void PressAnyKeyL();

private:
	void ConstructL(const TDesC& aLogFile);

private:
	// We own the console
	CConsoleBase* iConsole;

	RFs iSession;
	RFile iFile;

	TBool iEcho;
	};

#endif // __TOCSP_LOGGER_H__
