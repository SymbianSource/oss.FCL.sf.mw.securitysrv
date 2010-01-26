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

#include "logger.h"

#include <e32cons.h>


CTOCSPLogger* CTOCSPLogger::NewLC(const TDesC& aLogFile)
	{
	CTOCSPLogger* self = new (ELeave) CTOCSPLogger;
	CleanupStack::PushL(self);
	self->ConstructL(aLogFile);
	return self;
	}


CTOCSPLogger* CTOCSPLogger::NewL(const TDesC& aLogFile)
	{
	CTOCSPLogger* self = CTOCSPLogger::NewLC(aLogFile);
	CleanupStack::Pop(self);
	return self;
	}


CTOCSPLogger::~CTOCSPLogger()
	{
	iFile.Close();
	iSession.Close();

	delete iConsole;
	}


void CTOCSPLogger::ConstructL(const TDesC& aLogFile)
	{
	iConsole = Console::NewL(_L("OCSP Test"), TSize(KDefaultConsWidth,KDefaultConsHeight));

	User::LeaveIfError(iSession.Connect());

	TInt err = iSession.MkDirAll(aLogFile);
	if (err != KErrAlreadyExists)
		{
		User::LeaveIfError(err);
		}
	User::LeaveIfError(iFile.Replace(iSession, aLogFile, EFileShareExclusive));
	}


void CTOCSPLogger::LogL(const TDesC& aText, const TBool aEcho)
	{
	iEcho = aEcho;
	LogL(aText);
	}


void CTOCSPLogger::LogL(const TDesC8& aText, const TBool aEcho)
	{
	iEcho = aEcho;
	LogL(aText);
	}


void CTOCSPLogger::LogL(const TInt aInt, const TBool aEcho)
	{
	iEcho = aEcho;
	LogL(aInt);
	}


void CTOCSPLogger::LogL(const TDesC& aText)
	{
	if (iEcho)
		{
		iConsole->Write(aText);
		}

	// Have to convert to 8-bit for file

	TInt textLen = aText.Length();
	
	HBufC8* buf8 = NULL;
	TRAPD(err, buf8=HBufC8::NewL(textLen));
	if( KErrNoMemory == err) 
	{
		User::CompressAllHeaps();
			
		TRAPD(err2, buf8=HBufC8::NewL(textLen));
		if( KErrNone != err2) 
		{
			User::Leave(err2);	
		}
		
	}
	CleanupStack::PushL(buf8);

	TPtr8 pBuf8(buf8->Des());
	pBuf8.Copy(aText);
		
	User::LeaveIfError(iFile.Write(*buf8));
	CleanupStack::PopAndDestroy(buf8);
	}

void CTOCSPLogger::LogL(const TDesC8& aText)
	{
	if (iEcho)
		{
		// Convert to 16 bit descriptor to write to console
		RBuf buf16;
		buf16.CreateL(aText.Length());
		CleanupClosePushL(buf16);
		buf16.Copy(aText);
		iConsole->Write(buf16);
		CleanupStack::PopAndDestroy(&buf16);
		}

	User::LeaveIfError(iFile.Write(aText));
	}


void CTOCSPLogger::LogL(const TInt aInt)
	{
	TBuf<12> buf;
	buf.Num(aInt);
	LogL(buf);
	}


void CTOCSPLogger::NewLineL()
	{
	LogL(_L("\n"));
	}


void CTOCSPLogger::PressAnyKeyL()
	{
	LogL(_L("[Press any key]\n"));

	if (iEcho)
		{
		iConsole->Getch();
		}
	}
