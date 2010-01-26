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
// Reads lines from script using idle tick for each.  Parse into tokens, pass to command object.
// 
//

#include "script.h"
#include "command.h"

#include <e32cons.h>
#include <unifiedcertstore.h>

CTOCSPScript* CTOCSPScript::NewLC(CTOCSPLogger& aLog,
								  CTOCSPResult& aResult, 
								  const TDesC& aInputFileName,
								  TInt aTransaction)
	{
	CTOCSPScript* self = new (ELeave) CTOCSPScript(aLog);
	CleanupStack::PushL(self);
	self->ConstructL(aInputFileName, aResult, aTransaction);
	return self;
	}


CTOCSPScript* CTOCSPScript::NewL(CTOCSPLogger& aLog,
								 CTOCSPResult& aResult, 
								 const TDesC& aInputFileName,
								 TInt aTransaction)
	{
	CTOCSPScript* self = NewLC(aLog, aResult, aInputFileName, aTransaction);
	CleanupStack::Pop(self);
	return self;
	}
//********************************************************************
CTOCSPScript* CTOCSPScript::NewL(CTOCSPLogger& aLog,
								 CTOCSPResult& aResult, 
								 TInt aTransaction)
	{
	CTOCSPScript* self = NewLC(aLog, aResult, aTransaction);
	CleanupStack::Pop(self);
	return self;
	}

CTOCSPScript* CTOCSPScript::NewLC(CTOCSPLogger& aLog,
								  CTOCSPResult& aResult, 
								  TInt aTransaction)
	{
	CTOCSPScript* self = new (ELeave) CTOCSPScript(aLog);
	CleanupStack::PushL(self);
	self->ConstructL(aResult, aTransaction);
	return self;
	}

void CTOCSPScript::ConstructL(CTOCSPResult& aResult, TInt aTransaction)
	{
	CActiveScheduler::Add(this);

	iCommand = CTOCSPCommand::NewL(iLog, aResult, aTransaction);
	
	}

//*********************************************************************

CTOCSPScript::CTOCSPScript(CTOCSPLogger& aLog) :
	CActive(EPriorityNormal),
	iLog(aLog)
	{
	}


void CTOCSPScript::ConstructL(const TDesC& aInputFileName, CTOCSPResult& aResult, TInt aTransaction)
	{
	CActiveScheduler::Add(this);

	iCommand = CTOCSPCommand::NewL(iLog, aResult, aTransaction);

	User::LeaveIfError(iFs.Connect());
	TInt err = iFile.Open(iFs, aInputFileName, EFileRead | EFileShareAny);
	if (err != KErrNone)
		{
		iLog.LogL(_L("Error while opening "));
		iLog.LogL(aInputFileName);
		iLog.NewLineL();
		User::Leave(err);
		}
	iLineReader.Set(iFile);
	}


CTOCSPScript::~CTOCSPScript()
	{
	Cancel();
	iFile.Close();
	iFs.Close();

	delete iCommand;
	}


void CTOCSPScript::RunTestL(TRequestStatus& aStatus)
	{
	iClientRequestStatus = &aStatus;
	aStatus = KRequestPending;
	ReadLineL();
	}


TInt CTOCSPScript::TransactionCount()
    {
    return iCommand->CurrentTransaction();
    }


void CTOCSPScript::RunL()
	{
	User::LeaveIfError(iStatus.Int());
	ReadLineL();
	}


void CTOCSPScript::DoCancel()
	{
    iCommand->Cancel();
	}


TInt CTOCSPScript::RunError(TInt aError)
    {
    User::RequestComplete(iClientRequestStatus, aError);
    return KErrNone;
    }



// void CTOCSPScript::ReadLineL()f-
//reads in from the TDes  
void CTOCSPScript::ReadLineL()
	{
	TInt error = iLineReader.Read(iLine);

    if (error == KErrNone)
        {
		++iLinesProcessed;
        TBool carryOn = iCommand->ProcessCommand(iLine, iStatus);

        if (carryOn)
			{
			SetActive();
			}
		else
            {
			User::RequestComplete(iClientRequestStatus, KErrNone);
            }
        }
    else if (error == KErrEof)
        {
		User::RequestComplete(iClientRequestStatus, KErrNone);
        }
    else
        {
		User::Leave(error);
		}
	}


void CTOCSPScript::SetVerbose(TBool aVerbose)
	{
	iCommand->SetVerbose(aVerbose);
	}


TInt CTOCSPScript::LinesProcessed() const
	{
	return iLinesProcessed;
	}

const TDesC& CTOCSPScript::LastLine() const
	{
	return iLine;
	}
