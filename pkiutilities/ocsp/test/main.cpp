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
// Top-level test control implementation
// 
//

#include "main.h"
#include "comms.h"
#include "script.h"
#include "logger.h"
#include "result.h"
#include "panic.h"
#include "tcertutils.h"
#include "bautils.h" 


_LIT(KRunStep, "RUN_TEST_STEP");

// Optional 3rd argument to turn on OOM tests
_LIT(KOOMArgument, "OOM");

// First script test to run
#ifdef _DEBUG
const TInt KMinTransaction = 1;
#endif

// First heap fail point to try
const TInt KMinHeapFailPoint = 1;

// Number of allocations to test for before bailing out
const TInt KMaxHeapFailPoint = 100000;

// Number of consecutive successful completions before we belive we've finished
// an OOM test
const TInt KCompletions = 5;

// Where to find file cert store files

_LIT(KCertStoreCertsFilePath, "\\private\\101f72a6\\cacerts.dat");
_LIT(KCertStoreClientsFilePath, "\\private\\101f72a6\\certapps.dat");


CTOCSPMain* CTOCSPMain::NewLC(const TDesC& aScriptFile, const TDesC& aLogFile)
	{
	CTOCSPMain* self = new (ELeave) CTOCSPMain(aScriptFile);
	CleanupStack::PushL(self);
	self->ConstructL(aLogFile);
	return self;
	}

CTOCSPMain::CTOCSPMain(const TDesC& aScriptFile) :
	CActive(EPriorityNormal),
	iScriptFile(aScriptFile)
	{
	CActiveScheduler::Add(this);
	}

void CTOCSPMain::ConstructL(const TDesC& aLogFile)
	{	
	iLog = CTOCSPLogger::NewL(aLogFile);
	User::LeaveIfError(iFs.Connect());

	// this is the name of the test file we will write to from the script
	iTocspTestFile.Copy(_L("\\system\\tocsp\\tocsptestfile.txt"));

	if (BaflUtils::FolderExists(iFs, iTocspTestFile))
		{		
		if (BaflUtils::FileExists( iFs, iTocspTestFile ))
			{
			// delete the test file if it exists before starting the tests
			iFs.Delete(iTocspTestFile);	
			}
		}
	}

CTOCSPMain::~CTOCSPMain()
	{
	Cancel();
	delete iLog;
	delete iTest;
	delete iResult;
	if (BaflUtils::FolderExists(iFs, iTocspTestFile))
			{		
			if (BaflUtils::FileExists( iFs, iTocspTestFile ))
				{
				// delete the test file if it exists at the end of the tests
				iFs.Delete(iTocspTestFile);
				}
			}
	iFs.Close();
	}

void CTOCSPMain::RunNormalTestsL()
	{
	iLog->LogL(_L("<html><body><pre>\r\n"), ETrue);
	TPtrC scriptcontents;
	TPtrC inicontents;
	TPtrC runTestStep;
	TPtrC iniFileName;
	TPtrC iniSectionName;
	TBuf<512> iniFilePath;
	
	TInt err = KErrNone;
	TInt pos = 0;
	
	iLog->LogL(_L("Starting normal OCSP tests...\n"), ETrue);

    iError = KErrNone;

	ResetTestL();

	__UHEAP_MARK;

//	TBool runStep = EFalse;
	
//	To check if the input file is .script file
//	IF a script file
//	Reads in the contents of the script file and loads it on 'scriptcontents'
//	Extracs RUN_TEST_STEP line onto 'runTestStep'
//	Extracts .ini file name and the appropriate .ini section name
//	Generates the .ini file path
//	Reads in the contents of the ini file and loads it on 'inicontents'
//	Extracts the approritate section part and sets iSectionData
	
	iScriptFlag = TEFparser::FileType(iScriptFile);
	
	if(iScriptFlag)
		{
		do
			// get the run test step lines from the script file
			{
			err = TEFparser::ReadFileLC(iFs, iScriptFile, scriptcontents);
			runTestStep.Set(TEFparser::GetRunTestStep(scriptcontents, KRunStep, pos, err));
			if(err == KErrNone)
				{
				// get the ini file info for the current test step
				err = TEFparser::GetIniFileInfo(runTestStep, iniFileName, iniSectionName);
				if (err == KErrNone)
					{
					// get the path to the ini file for the current test step
					err = TEFparser::GetiniPath(iniFileName, iScriptFile, iniFilePath);
					if (err == KErrNone)
						{
						// get the data in section for of the ini file for the current test step and write it into the
						// test file
						err = TEFparser::GetSectionData(iniFilePath, iniSectionName, iTocspTestFile, iFs);
						if (err != KErrNone)
							{
							ReportErrorL();
							}
						CleanupStack::PopAndDestroy();
						}	
						else
						{
							ReportErrorL();
						}
					}
					else
					{
						ReportErrorL();
					}
				}
			else
				{
				CleanupStack::PopAndDestroy();
				}
			}while(err == KErrNone);
		// execute the tests for the complete test file with all the ini file section data in
		iResult = CTOCSPResult::NewL();
		iTest = CTOCSPScript::NewL(*iLog, *iResult, iTocspTestFile);
		iTest->SetVerbose(ETrue);
		iState = ERunningNormalTest;
		iTest->RunTestL(iStatus);
		SetActive();
		CActiveScheduler::Start();
		}
	else
		{
		iResult = CTOCSPResult::NewL();
		iTest = CTOCSPScript::NewL(*iLog, *iResult, iScriptFile);
		iTest->SetVerbose(ETrue);
		iState = ERunningNormalTest;
		iTest->RunTestL(iStatus);
		SetActive();
		CActiveScheduler::Start();
		}
		
	
	}

void CTOCSPMain::RunOOMTestsL()
	{
#ifdef _DEBUG
	iLog->LogL(_L("\nStarting OCSP OOM tests from script...\n"), ETrue);

    iError = KErrNone;

	iTransaction = KMinTransaction;
	iFailPoint = KMinHeapFailPoint;
	iCompletions = 0;
	ResetTestL();
	StartNextOOMTestL();

    // Run async code
	CActiveScheduler::Start();

	ReportErrorL();
#else
	iLog->LogL(_L("No OOM tests run on release build.\n"), ETrue);
	// reports a single test run to satisfy corebuilder log parsing requirements
	iLog->LogL(_L("0 tests failed out of 1\n"));
#endif
	}

// Report any error and raise it to the client
void CTOCSPMain::ReportErrorL()
	{
	if (iError != KErrNone)
		{	
		iLog->LogL(_L("Exiting with leave code: "));
		iLog->LogL(iError);
		iLog->NewLineL();
		iLog->LogL(_L("1 tests failed out of 1 (unknown)\n"));
		}
	}

TInt CTOCSPMain::RunError(TInt aError)
	{
	if (iState == ERunningNormalTest || iState == ERunningOOMTest)
		{
		// attempt to clean up after ourselves
		delete iTest;
		iTest = NULL;

		delete iResult;
		iResult = NULL;

		__UHEAP_MARKEND;
		}

	iError = aError;
	CActiveScheduler::Stop();	
	return KErrNone;
	}

void CTOCSPMain::DoCancel()
	{
	switch (iState)
		{
		case ERunningNormalTest:
		case ERunningOOMTest:
			iTest->Cancel();
			break;

		default:
			Panic(KErrCorrupt);
		}
	}

void CTOCSPMain::RunL()
	{
	switch (iState)
		{
		case ERunningNormalTest:
			FinishedNormalTestL();
			break;

		case ERunningOOMTest:
			FinishedOOMTestL();
			break;

		default:
			Panic(KErrCorrupt);
		}
	}

void CTOCSPMain::FinishedNormalTestL()
	{
	delete iTest;
	iTest = NULL;
	
	iLog->NewLineL();
	if (iStatus == KErrNone)
		{
		iLog->LogL(_L("Tests completed OK"), ETrue);
		iLog->NewLineL();
		}
	else
		{
		iLog->NewLineL();
		iLog->LogL(_L("ERROR: Leave code = "), ETrue);
		iLog->LogL(iStatus.Int());
		iLog->NewLineL();
		User::Leave(iStatus.Int());
		}
	
	iResult->LogSummaryL(*iLog);

	delete iResult;
	iResult = NULL;
	
	__UHEAP_MARKEND;

	CActiveScheduler::Stop();
	}

void CTOCSPMain::StartNextOOMTestL()
	{
	if (iFailPoint == 1)
		{
		iLog->LogL(_L("*** Testing transaction "), ETrue);
		iLog->LogL(iTransaction, ETrue);
		iLog->LogL(_L("\n"), ETrue);
		}

	iLog->LogL(_L("\n"));
	iLog->LogL(iFailPoint);
	iLog->LogL(_L(" "));

	__UHEAP_MARK;

	iResult = CTOCSPResult::NewL();
	iTest = CTOCSPScript::NewL(*iLog, *iResult, iTocspTestFile, iTransaction);

	iTest->SetVerbose(EFalse);
	iTest->RunTestL(iStatus);
	iState = ERunningOOMTest;
	SetActive();

	__UHEAP_SETFAIL(RHeap::EFailNext, iFailPoint);
	}

void CTOCSPMain::FinishedOOMTestL()
	{
	__UHEAP_SETFAIL(RHeap::ENone, 0);

	TBool more = ETrue;

	if (iTest)
		{
		more = CheckOOMTestResultL();
		}

	delete iTest;
	iTest = NULL;
	delete iResult;
	iResult = NULL;

	__UHEAP_MARKEND;

	if (more)
		{
		ResetTestL();
		StartNextOOMTestL();
		}
	else
		{
		CActiveScheduler::Stop();
		}
	}

TBool CTOCSPMain::CheckOOMTestResultL()
	{
	if (iStatus != KErrNone &&
		iStatus != KErrNoMemory  &&
		iStatus != KErrDiskFull)
		{
		// error, report and exit
		iLog->LogL(_L("\nHeap fail test (fail at "), ETrue);
		iLog->LogL(iFailPoint);
		iLog->LogL(_L(") exited with unexpected return code "));
		iLog->LogL(iStatus.Int());
		iLog->LogL(_L("\n"));
		iLog->LogL(_L("1 tests failed out of 1\n"));
		User::Leave(iStatus.Int());
		}

	// Count number of successive successful completions
	if (iStatus == KErrNone)
		{
		++iCompletions;
		}
	else
		{
		iCompletions = 0;
		}
	
	if (iCompletions == KCompletions)
		{
		// finished a round of testing
		if (iTransaction < iTest->TransactionCount())
			{
			// more transactions
			iFailPoint = KMinHeapFailPoint;
			++iTransaction;
			iCompletions = 0;
			iLog->NewLineL();
			}
		else
			{
			// we're finished
			iLog->LogL(_L("0 tests failed out of 1\n"));
			return EFalse;
			}
		}
	else
		{
		// go round again
		++iFailPoint;
		if (iFailPoint > KMaxHeapFailPoint)
			{
			iLog->LogL(_L("\nHeap fail test exceeded "), ETrue);
			iLog->LogL(KMaxHeapFailPoint);
			iLog->LogL(_L(" allocations\n"));
			iLog->LogL(_L("1 tests failed out of 1\n"));
			User::Leave(KErrGeneral);		  
			}
		}

	return ETrue;
	}

void CTOCSPMain::ResetTestL()
	{
	User::After(5 * 1000 * 1000);
	// We could use CCertUtils to remove the certs, but it's a lot faster just
	// to delete the files
	TDriveUnit sysDrive (RFs::GetSystemDrive());
	TDriveName driveName(sysDrive.Name());
	//file cert store files
	TBuf<128> certStoreFilePath (driveName);
	certStoreFilePath.Append(KCertStoreCertsFilePath);
	
	TInt error;
	error = iFs.Delete(certStoreFilePath);
	if (error != KErrNone && error != KErrNotFound && error != KErrPathNotFound)
	{
		// Handle a KErrInUse error. We try to delete up to 10 times, 
		// and wait 1 sec between two attemps
		if( KErrInUse == error) 
		{
			TInt tryCount = 10;
			do 
			{
				User::After(1000000);	
				error = iFs.Delete(certStoreFilePath);
				
			} while(0 != tryCount-- && error != KErrNone);
			
			if( 0 == tryCount ) 
			{
				User::Leave(error);
			}
			
		}
		else 
		{
			User::Leave(error);	
		}
	
	}

	//file cert store files
	certStoreFilePath.Copy(driveName);
	certStoreFilePath.Append(KCertStoreClientsFilePath);
	
	error = iFs.Delete(certStoreFilePath);
	if (error != KErrNone && error != KErrNotFound && error != KErrPathNotFound)
		{
		User::Leave(error);
		}
	}

LOCAL_C void RunScriptTestL()
    {
	// Start comms stuff - only necessary on WINS where this is an executable
#ifdef __WINS__
	InitCommsL();
#endif

	CActiveScheduler* scheduler = new (ELeave) CActiveScheduler();
	CleanupStack::PushL(scheduler);
	CActiveScheduler::Install(scheduler);

	// Get command line
	HBufC* argv = HBufC::NewLC(User::CommandLineLength());
	TPtr cmd(argv->Des());
	User::CommandLine(cmd);

	// Validate command line
	TLex argumentsValidation(cmd);	
	TInt argc = 0;
	while (argumentsValidation.NextToken().Length()) argc++;

	if (argc < 2 || argc > 3)
		{
		User::Leave(KErrArgument);
		}

	// Command line had 2 args - OK
	TLex arguments(cmd);
	TPtrC inputFile(arguments.NextToken());
	TPtrC logFile(arguments.NextToken());

	CTOCSPMain *main = CTOCSPMain::NewLC(inputFile, logFile);

	TBool memoryTests = EFalse;

	if (argc == 3)
		{
		TPtrC memArg(arguments.NextToken());
		if (memArg != KOOMArgument)
			{
			User::Leave(KErrArgument);
			}
		memoryTests = ETrue;
		}

	if (memoryTests)
		{
		main->RunOOMTestsL();
		}
	else
		{
		main->RunNormalTestsL();
		}

	CleanupStack::PopAndDestroy(main);

	CleanupStack::PopAndDestroy(argv);
	CleanupStack::PopAndDestroy(scheduler);
    }

// Entry point
GLDEF_C TInt E32Main()
    {
	__UHEAP_MARK;
	CTrapCleanup* cleanup=CTrapCleanup::New();

	TRAPD(error,RunScriptTestL()); 
	__ASSERT_ALWAYS(!error, User::Panic(_L("OCSP TEST ERROR"), error));

	delete cleanup;
	__UHEAP_MARKEND;
	return 0;
    }
