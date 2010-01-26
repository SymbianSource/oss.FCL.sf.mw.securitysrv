// Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// Takes a line from a script file representing a command and its parameters,
// interprets it and applies them to the engine
// 
//

/**
 @file 
 @internalTechnology
*/

#ifndef __TOCSP_COMMAND_H__
#define __TOCSP_COMMAND_H__

#include <e32base.h>
#include <ocsp.h>
#include "t_input.h"

class CTOCSPEngine;
class CTOCSPResult;
class CTOCSPLogger;
class RFs;

class CTOCSPCommand : public CActive
	{
public:
	static CTOCSPCommand* NewL(CTOCSPLogger& aLog,
							   CTOCSPResult& aResult,
							   TInt aTransaction = 0);
	~CTOCSPCommand();

	// Return False means abort, True means carry on as normal
	TBool ProcessCommand(const TDesC& aLine, TRequestStatus& aStatus);

    // Reset internal state after processing a whole script
	// Need to call this to make sure last test result is recorded
    void ResetL();

    // Get the index of the current transaction we're processing
    TInt CurrentTransaction() const;

	/** Set whether to print informational messages. */
	void SetVerbose(TBool aVerbose);

private:
	void RunL();
	void DoCancel();
    TInt RunError(TInt aError);

private:
	CTOCSPCommand(CTOCSPLogger& aLog,
				  CTOCSPResult& aResult,
				  TInt aTransaction);

	void ConstructL();

	TBool DoProcessCommandL(const TDesC& aLine);    
	TBool ProcessTestCommandL(const TDesC& command);

	// Command functions
	void UnknownCommandL(const TDesC& aCommand);
	void PrintCommandL(const TDesC& aMess);

	void StartCommandL();
	void EndCommandL();
	void SetURICommandL();


	void CertCommandL();
	void SetTransportCommandL();
	void SetTransportRetryCommandL();
	void SetTransportTimeoutCommandL();
	void SetNonceCommandL();

	void CancelCommandL();

	void ValidationDateCommandL();
	void DirectAuthorisationCommandL();
	void CADelegateAuthorisationCommandL();
	void CADirectAuthorisationCommandL();
	void AllAuthorisationSchemesCommandL();
	void SetMaxStatusAgeCommandL();
	void SetFilterParamsCommandL();
	void CheckCommand();
	void TestCancelCommand();
	void TestSummaryCommandL();
	void TestOutcomeCommandL();
	void TestTransportCommandL();
	void TestTransportRetryCommandL();

	void LogResponseCommandL();
	void LogRequestCommandL();

	void LogCommandL(const TDesC& line);
	
	void FinishedTestL();

	void WrongNumberOfArgumentsL();

	TTime ParseTimeL(TInt aStartToken);
	
	void SetResponderCertCheck();
	void AddCertToStoreL();
	void SetCheckCertsWithAiaOnly();
	void SetUseAIAL();
	
	void InvalidArgumentL(const TDesC& aCommand, const TDesC& aCommandOptions);
	void InvalidArgumentL(const TDesC& aCommand);
	void LogErrorL();
	
private:
	enum TState 
		{
		EEngineStart,
		EEngineEnd,
		EDirectAuthorisationCommand,
		ECheckCommand
		};

private:
	TState iState;
	TRequestStatus* iOriginalRequestStatus;

	CTOCSPResult& iResult;

	CTOCSPLogger& iLog;

	CTOCSPEngine* iEngine;

	RArray<TPtrC> iTokens;

	TInt iSkipping;

	TBool iInsideTest;
    TInt iTestIndex;
    TInt iTestToProcess;

	TBool iTestResult;

	TPtrC iCert;
	TPtrC iLabel;

	TBool iVerbose;
	TBool iTransportSet;

	// Contains result of cancellation (ETrue if revocation successfully cancelled)
	TBool iCheckCancelled;
	TInt iError;
	TBuf<128> iTmsId;

	};

/** Use direct authorisation without supplying an authorising cert. */
_LIT(KAuthCertNone, "AUTHCERTNONE");

#endif
