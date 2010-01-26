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
//

#include "command.h"
#include "engine.h"
#include "panic.h"
#include "logger.h"
#include "result.h"

#include "TEFparser.h"
#include <f32file.h>
#include <utf.h>

// Define the commands the user can use in the input script

// *****************  General  ************************

// Comment prefix for input file
_LIT(KTOCSPCommandComment,		"REM");

// Print remaining text to console
_LIT(KTOCSPCommandPrint,		"PRINT");

// Bail out at this point
_LIT(KTOCSPCommandAbort,		"ABORT");

// NOTE: Skips CAN be nested
// Skip everything unil KTOCSPCommandEndSkip
_LIT(KTOCSPCommandSkip,			"SKIP");

// Stop skipping - does nothing if not skipping
_LIT(KTOCSPCommandEndSkip,		"ENDSKIP");

// ************ Configuring the request ***************

// Start a new OCSP request/response transaction
// Has exactly one parameter - the name of the test (which must not include spaces)
_LIT(KTOCSPCommandStart,		"START");

// Set the URI of the server
_LIT(KTOCSPCommandSetURI,		"SETURI");


// Parameter is a file from which to read a cert
_LIT(KTOCSPCommandCert,			"REQUESTCERT");

// Parameter is ON or OFF
_LIT(KTOCSPCommandNonce,        "NONCE");

// ************ Handling the communication *************

// Set transport scheme for communication
// Parameter DEFAULT means let the OCSP module deal with the transport
// Parameter TEST means use the test harness server
_LIT(KTOCSPCommandSetTransport,	"TRANSPORT");

// ******* Configuring + performing the validation ********

// Set transport retry count
// Takes one parameter, the retry count (1 means no retry)
// NOTE: This command must appear after the TRANSPORT command
_LIT(KTOCSPCommandSetTransportRetry, "TRANSPORTRETRY");

// Set transport timeout
// Takes one parameter, the timeout value in milliseconds
// NOTE: This command must appear after the TRANSPORT command
_LIT(KTOCSPCommandSetTransportTimeout, "TRANSPORTTIMEOUT");

// Set validation date, parameters are YYYY MM DD HH MM - optional
_LIT(KTOCSPCommandValidationDate, "VALIDATIONDATE");

// Parameter is a file containing a cert to be set as valid for authorisation of
// the response.  Can call more than once.
_LIT(KTOCSPCommandDirectAuthorisation, "AUTHORISATIONCERT");

// No parameters - tells test client to use ca delegate authorisation.
// (See RFC2560 S2.2, S4.2.2.2)
_LIT(KTOCSPCommandCADelegateAuthorisation, "CADELEGATE");

// No parameters - tells test client to use direct signing by ca.
// see (RFC 2560 S2.2)
_LIT(KTOCSPCommandCADirectAuthorisation, "CADIRECT");

// No parameters - tells test client to use all schemes
// see (RFC 2560 S2.2)
_LIT(KTOCSPCommandAllAuthorisationSchemes, "ALLSCHEMES");

// Set the maximum allowable age of the status returned.  Parameter is in
// seconds, or "OFF" to disable checking.
_LIT(KTOCSPCommandSetMaxStatusAge, "SETMAXSTATUSAGE");

// Set http filter parameters
// The parameters to FILTERPARAMS command are:
// numDelayResp countDropResp countCorruptHTTPDataHeader countCorruptHTTPDataBodySizeLarge countCorruptHTTPDataBodySizeSmall countCorruptOCSPData countInternalErrorResp countTryLaterResp
// where:
// 		numDelayResp - Delays response by specified number of milliseconds
//		countDropResp - Drops specified number of responses
//		countCorruptHTTPDataHeader - Corrupts specified number of responses (content-type in header is corrupted)
//		countCorruptHTTPDataBodySizeLarge - Corrupts specified number of responses (body is of larger size than expected)
//		countCorruptHTTPDataBodySizeSmall - Corrupts specified number of responses (body is of smaller size than expected)
//		countCorruptOCSPData - Corrupts specified number of responses (OCSPResponse data is corrupted)
//		countInternalErrorResp - Returns an "internalError" response for specified number of requests
//		countTryLaterResp - Returns a "tryLater" response for specified number of requests
//		countSigValidateFailure - Causes a signature validation failure by corrupting the Responder ID
// NOTE: This command must appear after the TRANSPORT command
_LIT(KTOCSPCommandSetFilterParams, "FILTERPARAMS");


// Cancel the OCSP check after its issued
// The parameter is the time in milliseconds to wait before issuing the cancel
// Note that this command must appear before the CHECK command
_LIT(KTOCSPCommandCancel, "CANCEL");

// Run the OCSP check
_LIT(KTOCSPCommandCheck, "CHECK");

// Tests that the revocation check was cancelled
_LIT(KTOCSPCommandTestCancel, "TESTCANCEL");

// Tests that the summary result is as expected.  Takes one parameter, the
// expected result.
_LIT(KTOCSPCommandTestSummary, "TESTSUMMARY");

// Tests that the outcome for a certificate is as expected.  Takes three
// paramters, the certificate index, the expected status and the expected result
_LIT(KTOCSPCommandTestOutcome, "TESTOUTCOME");

// Tests that the outcome for the tranport is as expected. Takes four
// paramters, the retry count number (0 means first attempt), the expected HTTP method, 
// and the range of time in milliseconds (min max) within which each request should complete
// NOTE: The usage of this command assumes only one certificate is being
// checked for revocation per test case! (also means CHECKRESPONDERCERT command cannot be used with this)
_LIT(KTOCSPCommandTestTransport, "TESTTRANSPORT");

// Tests that the outcome for the retry attempts is as expected. 
// Takes one paramter, the retry count (1 means single attempt ie. no retry)
// NOTE: The usage of this command assumes only one certificate is being
// checked for revocation per test case! (also means CHECKRESPONDERCERT command cannot be used with this)
_LIT(KTOCSPCommandTestTransportRetry, "TESTTRANSPORTRETRY");

// Log the reponses to a file.  The filename is the only parameter.
// This command must be placed after the CHECK command
_LIT(KTOCSPCommandLogResponse, "LOGRESPONSE");

// Log the requests to a file.  The filename is the only parameter.
// This command must be placed before the TRANSPORT command
_LIT(KTOCSPCommandLogRequest, "LOGREQUEST");

// End a test
_LIT(KTOCSPCommandEnd, "END");

// parameter for setting whether the ocsp status check 
// for responder certificate should be done or not.
_LIT(KTOCSPCheckResponderCert, "CHECKRESPONDERCERT");

_LIT(KTOCSPAddCertToStore, "ADDCERTTOSTORE");

_LIT(KTOCSPUseAIA, "USEAIA");

_LIT(KTOCSPCheckCertsWithAiaOnly, "CHECKCERTSWITHAIAONLY");

_LIT(KTOCSPExpectedError, "EXPECTEDERROR");

// ************* Parameter definitions *****************

// Default transport (currently only HTTP)
_LIT(KTOCSPDefaultTransport,		"DEFAULT");

_LIT(KTOCSPTestTransport,			"TEST");

_LIT(KTOCSPOcspSupportTransport,	"OCSPSUPPORT");

// Generic parameter for toggle of settings.
_LIT(KTOCSPOn,  "ON");
_LIT(KTOCSPOff, "OFF");

_LIT(KTOCSPEnable,  "ENABLE");
_LIT(KTOCSPDisable, "DISABLE");
_LIT(KTOCSPEnableDisable,"ENABLE|DISABLE");


CTOCSPCommand* CTOCSPCommand::NewL(CTOCSPLogger& aLog,
								   CTOCSPResult& aResult,	
								   TInt aTransaction)
	{
	CTOCSPCommand* self = new (ELeave) CTOCSPCommand(aLog, aResult, aTransaction);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

 
CTOCSPCommand::CTOCSPCommand(CTOCSPLogger& aLog,
							 CTOCSPResult& aResult,
							 TInt aTransaction) :
	CActive(EPriorityNormal),
	iResult(aResult),
	iLog(aLog),
	iTestToProcess(aTransaction)
	{
	CActiveScheduler::Add(this);
	}


void CTOCSPCommand::ConstructL()
	{
	iEngine = CTOCSPEngine::NewL(iLog);
	}


CTOCSPCommand::~CTOCSPCommand()
	{
    Cancel();
	delete iEngine;
	iTokens.Close();
	}


void CTOCSPCommand::ResetL()
    {
	FinishedTestL();
    iTestIndex = 0;
    iTestToProcess = 0;
    }


TInt CTOCSPCommand::CurrentTransaction() const
    {
    return iTestIndex;
    }


TBool CTOCSPCommand::ProcessCommand(const TDesC& aLine, TRequestStatus& aStatus)
	{
	iOriginalRequestStatus = &aStatus;

    TBool result = ETrue;
    TRAPD(err, result = DoProcessCommandL(aLine));
    if (err != KErrNone)
        {
        Cancel();
        User::RequestComplete(iOriginalRequestStatus, err);
        }

    return result;
    }


TBool CTOCSPCommand::DoProcessCommandL(const TDesC& aLine)
	{
	*iOriginalRequestStatus = KRequestPending;

	if (!iVerbose)
		{
		iLog.LogL(_L("."), ETrue);
		}

	TLex lex(aLine);

    // Empty line
    if (lex.Eos())
        {
        User::RequestComplete(iOriginalRequestStatus, KErrNone);
        return ETrue;
        }

    TPtrC command = lex.NextToken();

    // Handle skipping commands
	if (command == KTOCSPCommandSkip)
		{
		++iSkipping;
		}
	else if (command == KTOCSPCommandEndSkip)
		{
        --iSkipping;
		if (iSkipping < 0)
			{
            User::RequestComplete(iOriginalRequestStatus, KErrCorrupt);
			return ETrue;
			}
		}

	LogCommandL(aLine);

	if (iSkipping)
		{
		User::RequestComplete(iOriginalRequestStatus, KErrNone);
        return ETrue;
		}

	if (command == KTOCSPCommandSkip || command == KTOCSPCommandEndSkip)
		{
		User::RequestComplete(iOriginalRequestStatus, KErrNone);
        return ETrue;
		}

	if (command == KTOCSPCommandStart)
		{
		++iTestIndex;
		}

    // Don't process any tests except the ones we're interested in
    if (iTestToProcess != 0 && iTestToProcess != iTestIndex)
        {   
		User::RequestComplete(iOriginalRequestStatus, KErrNone);
        return ETrue;
        }

	// Handle comment and print commands
	if (command == KTOCSPCommandComment)
		{
		// Remark - nothing to do except move on to next line
		User::RequestComplete(iOriginalRequestStatus, KErrNone);
        return ETrue;
		}
	else if (command == KTOCSPCommandPrint)
		{
		lex.SkipSpace();
		PrintCommandL(lex.Remainder());
        return ETrue;
		}

    // Now split the rest of the command into tokens
	iTokens.Reset();
	while (!lex.Eos())
		{
		User::LeaveIfError(iTokens.Append(lex.NextToken()));
		}
    
    // Check commands that don't have to occur in tests
	if (command == KTOCSPCommandAbort)
		{
		User::RequestComplete(iOriginalRequestStatus, KErrNone);
		return EFalse;
		}
	else if (command == KTOCSPCommandStart)
        {
		StartCommandL();
		}
	else if (command == KTOCSPCommandEnd)
        {
		EndCommandL();
		}
	else
		{
		return ProcessTestCommandL(command);
		}

	return ETrue;
	}


TBool CTOCSPCommand::ProcessTestCommandL(const TDesC& command)
	{
	if (!iInsideTest)
		{
		iLog.LogL(_L("Command cannot occur outside test: "));
		iLog.LogL(command);
		iLog.NewLineL();
		User::Leave(KErrCorrupt);
		}

    // Check the other commands that can only occur in tests
	if (command == KTOCSPCommandSetURI)
		{
		SetURICommandL();
		}
	else if (command == KTOCSPCommandSetTransport)
		{
		SetTransportCommandL();
		}
	else if (command == KTOCSPCommandSetTransportRetry)
		{
		SetTransportRetryCommandL();
		}
	else if (command == KTOCSPCommandSetTransportTimeout)
		{
		SetTransportTimeoutCommandL();
		}
	else if (command == KTOCSPCommandCert)
		{
		CertCommandL();
		}
	else if (command == KTOCSPCommandValidationDate)
		{
		ValidationDateCommandL();
		}
	else if (command == KTOCSPCommandDirectAuthorisation)
		{
		DirectAuthorisationCommandL();
		}
	else if (command == KTOCSPCommandCADelegateAuthorisation)
		{
		CADelegateAuthorisationCommandL();
		}
	else if (command == KTOCSPCommandCADirectAuthorisation)
		{
		CADirectAuthorisationCommandL();
		}
	else if (command == KTOCSPCommandAllAuthorisationSchemes)
		{
		AllAuthorisationSchemesCommandL();
		}
	else if (command == KTOCSPCommandSetMaxStatusAge)
		{
		SetMaxStatusAgeCommandL();
		}
	else if (command == KTOCSPCommandSetFilterParams)
		{
		SetFilterParamsCommandL();
		}
	else if (command == KTOCSPCommandCancel)
		{
		CancelCommandL();
		}
	else if (command == KTOCSPCommandCheck)
		{
		CheckCommand();
		}
	else if (command == KTOCSPCommandTestCancel)
		{
		TestCancelCommand();
		}
	else if (command == KTOCSPCommandTestSummary)
		{
		TestSummaryCommandL();
		}
	else if (command == KTOCSPCommandTestOutcome)
		{
		TestOutcomeCommandL();
		}
	else if (command == KTOCSPCommandTestTransport)
		{
		TestTransportCommandL();
		}
	else if (command == KTOCSPCommandTestTransportRetry)
		{
		TestTransportRetryCommandL();
		}
	else if (command == KTOCSPCommandNonce)
		{
		SetNonceCommandL();
		}
	else if (command == KTOCSPCommandLogResponse)
		{
		LogResponseCommandL();
		}
	else if (command == KTOCSPCommandLogRequest)
		{
		LogRequestCommandL();
		}
	else if (command == KTOCSPCheckResponderCert)
		{
		SetResponderCertCheck();
		}
	else if(command == KTOCSPAddCertToStore)
		{
		AddCertToStoreL();
		}
	else if(command ==  KTOCSPCheckCertsWithAiaOnly )
		{
		SetCheckCertsWithAiaOnly();
		}
	else if(command  == KTOCSPUseAIA)
		{
		SetUseAIAL();
		}
	else if(command == KTOCSPExpectedError)
		{
		LogErrorL();
		}
	else
		{
		UnknownCommandL(command);
		}

	return ETrue;
	}

void CTOCSPCommand::LogErrorL()
	{
	if (iTokens.Count() != 1)
		{
		WrongNumberOfArgumentsL();
		}

	TLex expectedErrorLex(iTokens[0]);
	TInt expectedError = 0;
	User::LeaveIfError(expectedErrorLex.Val(expectedError));

	iTestResult = (expectedError == iError)?ETrue:EFalse;

	User::RequestComplete(iOriginalRequestStatus, KErrNone);

	}

void CTOCSPCommand::RunL()
	{
	TInt err = iStatus.Int();

	// The engine does not get a chance to process the cancel notification so we handle it here
	if (err == KErrCancel)
		{
		// Cancel expected
		iCheckCancelled = ETrue;
		}
	else
		{
		User::LeaveIfError(err);
		}

	switch (iState)
		{
		case EEngineStart:
		case EEngineEnd:
		case EDirectAuthorisationCommand:
		case ECheckCommand:			
			User::RequestComplete(iOriginalRequestStatus, KErrNone);
			break;

		default:
            User::Panic(_L("TOCSP"), 1);
			break;
		}
	}

TInt CTOCSPCommand::RunError(TInt aError)
    {
    iError = aError;
    User::RequestComplete(iOriginalRequestStatus, KErrNone);
    return KErrNone;
    }

void CTOCSPCommand::DoCancel()
	{
	switch (iState)
		{
		case EEngineStart:
		case EEngineEnd:
		case EDirectAuthorisationCommand:
		case ECheckCommand:
			iEngine->Cancel();
			break;
			
		default:
            User::Panic(_L("TOCSP"), 1);
			break;
		}
	}

void CTOCSPCommand::LogCommandL(const TDesC& aLine)
	{
	if (iVerbose)
		{
		// To file only
		if (iSkipping)
			{
			iLog.LogL(_L("Skipped: "), EFalse);
			}
		else
			{
			iLog.LogL(_L("Input: "), EFalse);
			}

		iLog.LogL(aLine, EFalse);		
		
		TInt match;
		_LIT(KMatch,"*START*");
		match = aLine.Match(KMatch);
		if (match==0)
			{
		iTmsId.Copy(TEFparser::ParseNthElement(aLine,2));
		
		TBuf<64> timeBuf;
		TTime time;
		time.UniversalTime();
		TDateTime dateTime = time.DateTime();
		_LIT(KDateFormat,"%02d:%02d:%02d:%03d "); 
		timeBuf.AppendFormat(KDateFormat,dateTime.Hour(),dateTime.Minute(),dateTime.Second(),(dateTime.MicroSecond()/1000));
		
		iLog.LogL(_L("\r\n"));
		iLog.LogL(timeBuf);	
		iLog.LogL(_L("Command = START_TESTCASE "));
		iLog.LogL(iTmsId);
			}	        
		iLog.LogL(_L("\n"));
		}
	}


void CTOCSPCommand::UnknownCommandL(const TDesC& aCommand)
	{
	// Log to screen and file
	iLog.LogL(_L("Unrecognised command \""), ETrue);
	iLog.LogL(aCommand);
	iLog.LogL(_L("\" - aborting\n"));
	
	User::RequestComplete(iOriginalRequestStatus, KErrCorrupt);
	}


void CTOCSPCommand::PrintCommandL(const TDesC& aMess)
	{
	if (iVerbose)
		{
		// To screen and file
		iLog.LogL(aMess, ETrue);
		iLog.LogL(_L("\n"), ETrue);
		}

	User::RequestComplete(iOriginalRequestStatus, KErrNone);
	}


void CTOCSPCommand::StartCommandL()
	{
	if (iInsideTest)
		{
		if (iVerbose)
			{
			iLog.LogL(_L("Misplaced START command: not inside test\n"));
			}
		User::Leave(KErrCorrupt);
		}

	if (iTokens.Count() != 1)
		{
		WrongNumberOfArgumentsL();
		}

	iInsideTest = ETrue;
	iTestResult = ETrue;
	iTransportSet = EFalse;

	iResult.NewTestL(iTokens[0]);

	if (iVerbose)
		{
		iLog.LogL(_L("Test: "));
		iLog.LogL(iTokens[0]);
		iLog.NewLineL();
		}

	iEngine->StartL(iStatus);
	iState = EEngineStart;
	SetActive();
	}


void CTOCSPCommand::EndCommandL()
	{
	if (!iInsideTest)
		{
		if (iVerbose)
			{
			iLog.LogL(_L("Misplaced END command: not inside test\n"));
			}
		User::Leave(KErrCorrupt);
		}

	iInsideTest = EFalse;
	iCheckCancelled = EFalse;
	FinishedTestL();

	if (iVerbose)
		{
		iLog.NewLineL();
		}

	iEngine->EndL(iStatus);
	iState = EEngineEnd;
	SetActive();
	}


void CTOCSPCommand::SetURICommandL()
	{
	TInt tokenCount = iTokens.Count();
	
	if (tokenCount < 1 || tokenCount > 3) 
		{
		WrongNumberOfArgumentsL();
		}
	
	// Need to convert from unicode (as in file) to 8-bit (as used by OCSP)
	TBuf8<256> uri8;
	uri8.Copy(iTokens[0]);
	_LIT8(KNull,"NULL");
	if(uri8.Compare(KNull) == 0)
		{
		uri8.FillZ();
		}
	if(tokenCount == 3 )
		{
		if(iTokens[1].Compare(KTOCSPUseAIA) != 0 )
			{
			InvalidArgumentL(KTOCSPUseAIA);
			}
		if(iTokens[2].Compare(KTOCSPEnable) == 0)
			{
			iEngine->SetURIL(uri8, ETrue);
			}
		else if(iTokens[2].Compare(KTOCSPDisable) == 0)
			{
			iEngine->SetURIL(uri8, EFalse);
			}
		else
			{
			InvalidArgumentL(KTOCSPUseAIA,KTOCSPEnableDisable);
			}
		}
	else
		{
		// default value
		iEngine->SetURIL(uri8, ETrue);
		}
	User::RequestComplete(iOriginalRequestStatus, KErrNone);
	}

void CTOCSPCommand::SetTransportCommandL()
	{
	if (iTokens.Count() == 0)
		{
		WrongNumberOfArgumentsL();
		}

	TPtrC parameter = iTokens[0];
	if (parameter == KTOCSPDefaultTransport)
		{
		if (iTokens.Count() != 1)
			{
			WrongNumberOfArgumentsL();
			}
		iEngine->SetDefaultTransportL();
		}
	else if (parameter == KTOCSPTestTransport)
		{
		if (iTokens.Count() < 2 || iTokens.Count() > 3)
			{
			WrongNumberOfArgumentsL();
			}

		iEngine->SetTestTransportL(iTokens[1], iTokens.Count() == 3 ? &iTokens[2] : NULL);
		}
	else if (parameter == KTOCSPOcspSupportTransport)
		{
		if (iTokens.Count() != 1)
			{
			WrongNumberOfArgumentsL();
			}

		iEngine->SetOcspSupportTransportL();
		}
	iTransportSet = ETrue;
	User::RequestComplete(iOriginalRequestStatus, KErrNone);
	}

void CTOCSPCommand::SetTransportRetryCommandL()
	{
	if (!iTransportSet)
		{
		if (iVerbose)
			{
			iLog.LogL(_L("Command sequence error: TRANSPORTRETRY must appear after TRANSPORT command\n"));
			}
		User::Leave(KErrCorrupt);
		}

	if (iTokens.Count() != 1)
		{
		WrongNumberOfArgumentsL();
		}

	TInt retryCount = 0;
	TLex lex(iTokens[0]);
	if ((lex.Val(retryCount) != KErrNone) || (retryCount < 0))
		{
		if (iVerbose)
			{
			iLog.LogL(_L("Invalid retry count in TRANSPORTRETRY command\n"));
			}
		User::Leave(KErrArgument);
		}

	iEngine->SetTransportRetryCount(static_cast<TUint>(retryCount));

	User::RequestComplete(iOriginalRequestStatus, KErrNone);
	}

void CTOCSPCommand::SetTransportTimeoutCommandL()
	{
	if (!iTransportSet)
		{
		if (iVerbose)
			{
			iLog.LogL(_L("Command sequence error: TRANSPORTTIMEOUT must appear after TRANSPORT command\n"));
			}
		User::Leave(KErrCorrupt);
		}

	if (iTokens.Count() != 1)
		{
		WrongNumberOfArgumentsL();
		}

	TInt timeout = 0;
	TLex lex(iTokens[0]);
	if ((lex.Val(timeout) != KErrNone) || (timeout < -1))
		{
		if (iVerbose)
			{
			iLog.LogL(_L("Invalid timeout in TRANSPORTTIMEOUT command\n"));
			}
		User::Leave(KErrArgument);
		}

	iEngine->SetTransportTimeout(timeout);

	User::RequestComplete(iOriginalRequestStatus, KErrNone);
	}

void CTOCSPCommand::SetNonceCommandL()
	{
	if (iTokens.Count() != 1)
		{
		WrongNumberOfArgumentsL();
		}

	TPtrC param = iTokens[0];
	if (param == KTOCSPOn)
		{
		iEngine->SetNonce(ETrue);
		}
	else if (param == KTOCSPOff)
		{
		iEngine->SetNonce(EFalse);
		}
	else
		{
		User::Leave(KErrArgument);
		}

	User::RequestComplete(iOriginalRequestStatus, KErrNone);
	}


void CTOCSPCommand::CertCommandL()
	{
	if (iTokens.Count() != 2)
		{
		WrongNumberOfArgumentsL();
		}

	TPtrC subjectCert = iTokens[0];
	TPtrC issuerCert = iTokens[1];

	// Open file server session
	RFs session;
	User::LeaveIfError(session.Connect());
	CleanupClosePushL(session);

	HBufC8* subjectData = 0;
	TRAPD(err, subjectData = Input::ReadFileL(subjectCert, session));
	CleanupStack::PushL(subjectData);
	if (err != KErrNone)
		{
		if (err != KErrNoMemory && iVerbose)
			{
			iLog.LogL(_L("Error opening "));
			iLog.LogL(subjectCert);
			}
		User::Leave(err);
		}

	HBufC8* issuerData = 0;
	TRAP(err, issuerData = Input::ReadFileL(issuerCert, session));
	CleanupStack::PushL(issuerData);
	if (err != KErrNone)
		{
		if (err != KErrNoMemory && iVerbose)
			{
			iLog.LogL(_L("Error opening "));
			iLog.LogL(issuerCert);
			}
		User::Leave(err);
		}

	// Engine will actually read + own the cert
	iEngine->AddCertL(*subjectData, *issuerData);

	CleanupStack::PopAndDestroy(3); // issuerData, subjectData, close session

	User::RequestComplete(iOriginalRequestStatus, KErrNone);
	}


void CTOCSPCommand::ValidationDateCommandL()
	{
	if (iTokens.Count() != 5)
		{
		WrongNumberOfArgumentsL();
		}

	iEngine->SetValidationTimeL(ParseTimeL(0));

	User::RequestComplete(iOriginalRequestStatus, KErrNone);
	}

/**
 * Parse a series of input tokens representing a time - format is:
 * YEAR MONTH DAY HOUR MINUTE
 */

TTime CTOCSPCommand::ParseTimeL(TInt aStartToken)
	{
	TLex lexer;
	TInt year, month, day, hour, minute;

	lexer.Assign(iTokens[aStartToken++]);
	User::LeaveIfError(lexer.Val(year));

	lexer.Assign(iTokens[aStartToken++]);
	User::LeaveIfError(lexer.Val(month));

	lexer.Assign(iTokens[aStartToken++]);
	User::LeaveIfError(lexer.Val(day));

	lexer.Assign(iTokens[aStartToken++]);
	User::LeaveIfError(lexer.Val(hour));

	lexer.Assign(iTokens[aStartToken]);
	User::LeaveIfError(lexer.Val(minute));

	// Internal month and day are 0-based
	TDateTime dateTime(year, TMonth(--month), --day, hour, minute, 0, 0);

	return dateTime;
	}


void CTOCSPCommand::SetMaxStatusAgeCommandL()
	{
	if (iTokens.Count() != 1)
		{
		WrongNumberOfArgumentsL();
		}

	if (iTokens[0] == KTOCSPOff)
		{
		iEngine->SetMaxStatusAgeL(0);
		}
	else
		{
		TLex lexer;
		lexer.Assign(iTokens[0]);
		TUint seconds;
		User::LeaveIfError(lexer.Val(seconds));
		iEngine->SetMaxStatusAgeL(seconds);
		}

	User::RequestComplete(iOriginalRequestStatus, KErrNone);
	}

void CTOCSPCommand::SetFilterParamsCommandL()
	{
	if (iTokens.Count() != 9)
		{
		WrongNumberOfArgumentsL();
		}
	TInt numDelayResp = 0, countDropResp = 0;
	TInt countCorruptHTTPDataHeader = 0, countCorruptHTTPDataBodySizeLarge = 0, countCorruptHTTPDataBodySizeSmall = 0;
	TInt countCorruptOCSPData = 0;
	TInt countInternalErrorResp = 0, countTryLaterResp = 0;
	TInt countSigValidateFailure = 0;

	TLex lex(iTokens[0]);
	if ((lex.Val(numDelayResp) != KErrNone) || (numDelayResp < -1))
		{
		if (iVerbose)
			{
			iLog.LogL(_L("Invalid numDelayResp in FILTERPARAMS command\n"));
			}
		User::Leave(KErrArgument);
		}

	lex = iTokens[1];
	if ((lex.Val(countDropResp) != KErrNone) || (countDropResp < -1))
		{
		if (iVerbose)
			{
			iLog.LogL(_L("Invalid countDropResp in FILTERPARAMS command\n"));
			}
		User::Leave(KErrArgument);
		}

	lex = iTokens[2];
	if ((lex.Val(countCorruptHTTPDataHeader) != KErrNone) || (countCorruptHTTPDataHeader < -1))
		{
		if (iVerbose)
			{
			iLog.LogL(_L("Invalid countCorruptHTTPDataHeader in FILTERPARAMS command\n"));
			}
		User::Leave(KErrArgument);
		}

	lex = iTokens[3];
	if ((lex.Val(countCorruptHTTPDataBodySizeLarge) != KErrNone) || (countCorruptHTTPDataBodySizeLarge < -1))
		{
		if (iVerbose)
			{
			iLog.LogL(_L("Invalid countCorruptHTTPDataBodySizeLarge in FILTERPARAMS command\n"));
			}
		User::Leave(KErrArgument);
		}

	lex = iTokens[4];
	if ((lex.Val(countCorruptHTTPDataBodySizeSmall) != KErrNone) || (countCorruptHTTPDataBodySizeSmall < -1))
		{
		if (iVerbose)
			{
			iLog.LogL(_L("Invalid countCorruptHTTPDataBodySizeSmall in FILTERPARAMS command\n"));
			}
		User::Leave(KErrArgument);
		}

	lex = iTokens[5];
	if ((lex.Val(countCorruptOCSPData) != KErrNone) || (countCorruptOCSPData < -1))
		{
		if (iVerbose)
			{
			iLog.LogL(_L("Invalid countCorruptOCSPData in FILTERPARAMS command\n"));
			}
		User::Leave(KErrArgument);
		}

	lex = iTokens[6];
	if ((lex.Val(countInternalErrorResp) != KErrNone) || (countInternalErrorResp < -1))
		{
		if (iVerbose)
			{
			iLog.LogL(_L("Invalid countInternalErrorResp in FILTERPARAMS command\n"));
			}
		User::Leave(KErrArgument);
		}

	lex = iTokens[7];
	if ((lex.Val(countTryLaterResp) != KErrNone) || (countTryLaterResp < -1))
		{
		if (iVerbose)
			{
			iLog.LogL(_L("Invalid countTryLaterResp in FILTERPARAMS command\n"));
			}
		User::Leave(KErrArgument);
		}

	lex = iTokens[8];
	if ((lex.Val(countSigValidateFailure) != KErrNone) || (countSigValidateFailure < -1))
		{
		if (iVerbose)
			{
			iLog.LogL(_L("Invalid countSigValidateFailure in FILTERPARAMS command\n"));
			}
		User::Leave(KErrArgument);
		}

	iEngine->SetFilterParameters(numDelayResp, countDropResp,
			countCorruptHTTPDataHeader, countCorruptHTTPDataBodySizeLarge, countCorruptHTTPDataBodySizeSmall,
			countCorruptOCSPData, 
			countInternalErrorResp, countTryLaterResp, countSigValidateFailure);

	User::RequestComplete(iOriginalRequestStatus, KErrNone);
	}

// Pass a certificate (file) to be used as a root cert for validation of the response
void CTOCSPCommand::DirectAuthorisationCommandL()
	{
	iState = EDirectAuthorisationCommand;
	
	switch (iTokens.Count())
		{
	case 1:
		// since CA Delegate support was added, the script
		// cannot assume that direct authorisation is always
		// used.  Therefore, the special case of
		// "AUTHORISATIONCERT AUTHCERTNONE" is used to indicate that
		// direct authorisation should be used, even if no
		// certs are supplied.
		if (iTokens[0] != KAuthCertNone)
			User::Leave(KErrArgument);
		
		iEngine->UseDirectAuthorisation();
		User::RequestComplete(iOriginalRequestStatus, KErrNone);
		return;
		
	case 2:
		iCert.Set(iTokens[0]);
		iLabel.Set(iTokens[1]);
		break;
		
	default:
		WrongNumberOfArgumentsL();
		}

	iEngine->AddDirectAuthorisationCert(iCert, iLabel, iStatus);
	SetActive();
	}

void CTOCSPCommand::CADelegateAuthorisationCommandL()
/**
	Instruct engine to use CA delegate authorisation.
 */
	{
	if (iTokens.Count() != 0)
		{
		WrongNumberOfArgumentsL();
		}

	iEngine->UseCADelegateAuthorisation();
	User::RequestComplete(iOriginalRequestStatus, KErrNone);
	}

void CTOCSPCommand::CADirectAuthorisationCommandL()
/**
	Instruct engine to expect the response is signed
	directly by the CA.
 */
	{
	if (iTokens.Count() != 0)
		{
		WrongNumberOfArgumentsL();
		}

	iEngine->UseCADirectAuthorisation();
	User::RequestComplete(iOriginalRequestStatus, KErrNone);
	}

void CTOCSPCommand::AllAuthorisationSchemesCommandL()
/**
	Instruct engine to expect allocate all supported
	authorisation schemes.

	The script will have to specify which authorisation certs
	are supported for direct authorisation with AUTHORISATIONCERT,
	later in the script.
 */
	{
	if (iTokens.Count() != 0)
		{
		WrongNumberOfArgumentsL();
		}

	iEngine->UseAllAuthorisationSchemes();
	User::RequestComplete(iOriginalRequestStatus, KErrNone);
	}

void CTOCSPCommand::CancelCommandL()
	{
	if (iTokens.Count() != 1)
		{
		WrongNumberOfArgumentsL();
		}

	TLex timeLex(iTokens[0]);
	TInt time = 0;
	User::LeaveIfError(timeLex.Val(time));

	iEngine->SetCancelTime(time);

	User::RequestComplete(iOriginalRequestStatus, KErrNone);
	}

void CTOCSPCommand::CheckCommand()
	{
	iState = ECheckCommand;
	iEngine->Check(iStatus);
	SetActive();
	}

void CTOCSPCommand::TestCancelCommand()
	{
	iTestResult = iTestResult && iCheckCancelled;

	User::RequestComplete(iOriginalRequestStatus, KErrNone);
	}

void CTOCSPCommand::TestSummaryCommandL()
	{
	if (iTokens.Count() != 1)
		{
		WrongNumberOfArgumentsL();
		}

	TLex expectedResultLex(iTokens[0]);
	TInt expectedResult = 0;
	User::LeaveIfError(expectedResultLex.Val(expectedResult));

	iTestResult = iTestResult && iEngine->TestSummaryL(STATIC_CAST(OCSP::TResult, expectedResult));

	User::RequestComplete(iOriginalRequestStatus, KErrNone);
	}


void CTOCSPCommand::TestOutcomeCommandL()
	{
	if (iTokens.Count() != 3)
		{
		WrongNumberOfArgumentsL();
		}

	TLex certIndexLex(iTokens[0]);
	TInt certIndex = 0;
	User::LeaveIfError(certIndexLex.Val(certIndex));

	TLex expectedStatusLex(iTokens[1]);
	TInt expectedStatus = 0;
	User::LeaveIfError(expectedStatusLex.Val(expectedStatus));

	TLex expectedResultLex(iTokens[2]);
	TInt expectedResult = 0;
	User::LeaveIfError(expectedResultLex.Val(expectedResult));

	TOCSPOutcome expectedOutcome(STATIC_CAST(OCSP::TStatus, expectedStatus), STATIC_CAST(OCSP::TResult, expectedResult));

	iTestResult = iTestResult && iEngine->TestOutcomeL(certIndex, expectedOutcome);

	User::RequestComplete(iOriginalRequestStatus, KErrNone);
	}

void CTOCSPCommand::TestTransportCommandL()
	{
	if (iTokens.Count() != 4)
		{
		WrongNumberOfArgumentsL();
		}

	TLex lex(iTokens[0]);
	TInt retryCountNum = 0;
	if ((lex.Val(retryCountNum) != KErrNone) || (retryCountNum < 0))
		{
		if (iVerbose)
			{
			iLog.LogL(_L("Invalid request number in TESTTRANSPORT command"), ETrue);
			}
		User::Leave(KErrArgument);
		}

	// Check the method name supplied (do a case insensitive compare)
	TPtrC ptrHttpMethod = iTokens[1];
	if (ptrHttpMethod.CompareC(_L("GET"), 2, NULL) && 
			ptrHttpMethod.CompareC(_L("POST"), 2, NULL))
		{
		if (iVerbose)
			{
			iLog.LogL(_L("Invalid HTTP method name in TESTTRANSPORT command"), ETrue);
			}
		User::Leave(KErrArgument);
		}

	// Get the response time range
	TInt expRespTimeRangeMin = 0;
	TInt expRespTimeRangeMax = 0;
	lex = iTokens[2];
	if (lex.Val(expRespTimeRangeMin) != KErrNone)
		{
		if (iVerbose)
			{
			iLog.LogL(_L("Invalid range (min) for response time in TESTTRANSPORT command"), ETrue);
			}
		User::Leave(KErrArgument);
		}

	lex = iTokens[3];
	if ((lex.Val(expRespTimeRangeMax) != KErrNone) || expRespTimeRangeMax < expRespTimeRangeMin)
		{
		if (iVerbose)
			{
			iLog.LogL(_L("Invalid range (max) for response time in TESTTRANSPORT command"), ETrue);
			}
		User::Leave(KErrArgument);
		}

	iTestResult = iTestResult && iEngine->TestTransportL(retryCountNum, ptrHttpMethod,  
															expRespTimeRangeMin, expRespTimeRangeMax);

	User::RequestComplete(iOriginalRequestStatus, KErrNone);
	}

void CTOCSPCommand::TestTransportRetryCommandL()
	{
	if (iTokens.Count() != 1)
		{
		WrongNumberOfArgumentsL();
		}

	TLex lex(iTokens[0]);
	TInt retryCount = 0;
	if ((lex.Val(retryCount) != KErrNone) || (retryCount < 0))
		{
		if (iVerbose)
			{
			iLog.LogL(_L("Invalid request count in TESTTRANSPORTRETRY command"), ETrue);
			}
		User::Leave(KErrArgument);
		}

	iTestResult = iTestResult && iEngine->TestTransportRetryL(retryCount);

	User::RequestComplete(iOriginalRequestStatus, KErrNone);
	}

void CTOCSPCommand::LogResponseCommandL()
	{
	if (iTokens.Count() != 1)
		{
		WrongNumberOfArgumentsL();
		}

    iEngine->LogResponseL(iTokens[0]);

	User::RequestComplete(iOriginalRequestStatus, KErrNone);
	}


void CTOCSPCommand::LogRequestCommandL()
	{
	if (iTokens.Count() != 1)
		{
		WrongNumberOfArgumentsL();
		}

    iEngine->LogRequestL(iTokens[0]);

	User::RequestComplete(iOriginalRequestStatus, KErrNone);
	}


void CTOCSPCommand::SetVerbose(TBool aVerbose)
	{
	iVerbose = aVerbose;
	iEngine->SetVerbose(aVerbose);
	}


void CTOCSPCommand::FinishedTestL()
	{	
	TBuf<64> timeBuf;
	TTime time;
	time.UniversalTime();
	TDateTime dateTime = time.DateTime();
	_LIT(KDateFormat,"%02d:%02d:%02d:%03d "); 
	timeBuf.AppendFormat(KDateFormat,dateTime.Hour(),dateTime.Minute(),dateTime.Second(),(dateTime.MicroSecond()/1000));	
		
	iLog.LogL(_L("\r\n"));
	iLog.LogL(timeBuf);	
	iLog.LogL(_L("Command = END_TESTCASE "), EFalse);
	iLog.LogL(iTmsId, ETrue);
	iLog.LogL(_L(" ***TestCaseResult = "), ETrue);
	
	iResult.ResultL(iTestResult);
	if (iVerbose)
		{
		if (iTestResult)
			{
			iLog.LogL(_L("PASS\n"), ETrue);
			}
		else
			{
			iLog.LogL(_L("FAIL\n"), ETrue);
			}
		}
	}


void CTOCSPCommand::WrongNumberOfArgumentsL()
	{
	iLog.LogL(_L("Wrong number of arguments for command"), ETrue);
	User::Leave(KErrArgument);
	}

void CTOCSPCommand::InvalidArgumentL(const TDesC& aCommand, const TDesC& aCommandOptions )
	{
	iLog.LogL(_L("Invalid input for:"), ETrue);
	iLog.LogL(aCommand, ETrue);
	iLog.LogL(_L(", Expected:"), ETrue);
	iLog.LogL(aCommandOptions, ETrue);
	User::Leave(KErrArgument);
	}

void CTOCSPCommand::InvalidArgumentL(const TDesC& aCommand)
	{
	iLog.LogL(_L("Invalid input:"), ETrue);
	iLog.LogL(aCommand, ETrue);
	User::Leave(KErrArgument);
	}

void CTOCSPCommand::SetResponderCertCheck()
	{
	iEngine->SetReponderCertCheck();
	User::RequestComplete(iOriginalRequestStatus, KErrNone);
	}

void CTOCSPCommand::AddCertToStoreL()
{
	if(iTokens.Count()!= 2)
		{
		WrongNumberOfArgumentsL();
		}
	
	TPtrC certFileName = iTokens[0];
	TPtrC certLabel = iTokens[1];
	
	iEngine->AddCertToStore(certFileName, certLabel, ECACertificate, iStatus);
	
	iState = EEngineEnd;
	SetActive();
}

void CTOCSPCommand::SetCheckCertsWithAiaOnly()
	{
	iEngine->SetCheckCertsWithAiaOnly(ETrue);
	User::RequestComplete(iOriginalRequestStatus, KErrNone);
	}

void CTOCSPCommand::SetUseAIAL()
	{	
	if (iTokens.Count() != 1) 
		{
		WrongNumberOfArgumentsL();
		}
	
	if(iTokens[0].Compare(KTOCSPEnable) == 0)
		{
		iEngine->SetUseAIA(ETrue);
		}
	else if(iTokens[0].Compare(KTOCSPDisable) == 0)
		{
		iEngine->SetUseAIA(EFalse);
		}
	else
		{
		InvalidArgumentL(KTOCSPUseAIA,KTOCSPEnableDisable);
		}
	
	User::RequestComplete(iOriginalRequestStatus, KErrNone);
	}
