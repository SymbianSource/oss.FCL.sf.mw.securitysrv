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

#include "engine.h"
#include "transport.h"
#include "panic.h"
#include "logger.h"
#include "main.h"
#include "requestlogger.h"
#include "testfilterparameters.h"
#include <miscutil.h>

#include <ocsp.h>
#include <x509cert.h>

#include <ocsptransport.h>

#include "ocspsupporttransport.h"

// Log file created by the transport filter
_LIT(KFilterLogFileName, "\\tocsphttpfilter.log");

const TInt KTimeMilliToMicro = 1000;

CTOCSPEngine* CTOCSPEngine::NewL(CTOCSPLogger& aLog)
	{
	CTOCSPEngine* self = new (ELeave) CTOCSPEngine(aLog);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}


CTOCSPEngine::CTOCSPEngine(CTOCSPLogger& aLog) :
	CActive(EPriorityNormal),
	iLog(aLog)
	{
	CActiveScheduler::Add(this);
	}


void CTOCSPEngine::ConstructL()
	{
	// Open file server session
	User::LeaveIfError(iFs.Connect());
	iCertUtils = CCertUtils::NewL(iFs);

	// Create cancellation timer
	iTimer = CCallbackTimer::NewL(*this);
	}

CTOCSPEngine::~CTOCSPEngine()
	{
    Cancel(); // Calls Reset along the way
	Destroy();
	delete iTimer;
	delete iCertUtils;
	delete iUnifiedCertStore;
	iFs.Close();
	}

// Common code between destructor and ResetL()
void CTOCSPEngine::Destroy()
	{
	delete iParams;
	iParams = NULL;
	
	delete iClient;
	iClient = NULL;
	
	iSubjectCerts.ResetAndDestroy();
	iIssuerCerts.ResetAndDestroy();
	iSigningCerts.ResetAndDestroy();
	iTransportLog.Close();

	delete iRequestLog;
	iRequestLog = NULL;

	// Not owned
	iTransport = NULL;
	}

void CTOCSPEngine::Reset()
	{
	Destroy();

	// Delete and reset all the filter parameters
	DeleteFilterParameters();

	iUseDirectAuthorisation = EFalse;
	iUseCADelegateAuthorisation = EFalse;
	iUseCADirectAuthorisation = EFalse;
	iUseAllSchemes = EFalse;

	iCancelTime = 0;
	}


void CTOCSPEngine::StartL(TRequestStatus& aStatus)
	{	
	iParams = COCSPParameters::NewL();	
	InitDirectAuthL();

	// Delete the transport filter log file
	TInt err = iFs.Delete(KFilterLogFileName);
	if ((err != KErrNone) && (err != KErrNotFound))
		{
		if (iVerbose)
			{
			iLog.LogL(_L("Failed to delete transport filter log file ("), ETrue);
			iLog.LogL(KFilterLogFileName);
			iLog.LogL(_L("). Error: "), ETrue);
			iLog.LogL(err);
			}
		User::Leave(err);
		}

	iState = EInitCertStore;
	aStatus = KRequestPending;
	iOriginalRequestStatus = &aStatus;
	SetActive();
		
	// Initialise unified cert store here if it doesn't already exist
	if (!iUnifiedCertStore)
		{
		iUnifiedCertStore = CUnifiedCertStore::NewL(iFs, ETrue);
		iUnifiedCertStore->Initialize(iStatus);
		}
	else
		{
		TRequestStatus* status = &iStatus;
		User::RequestComplete(status, KErrNone);
		}
	}

void CTOCSPEngine::EndL(TRequestStatus& aStatus)
	{
	Reset();
	CleanUpDirectAuthL(aStatus);
	}


void CTOCSPEngine::SetURIL(const TDesC8& aURI, TBool aUseAIA)
	{
	iParams->SetURIL(aURI, aUseAIA);
	}


void CTOCSPEngine::SetDefaultTransportL()
	{
	iIap = 0;
	MOCSPTransport* trans = COCSPTransportDefault::NewL(iIap);
	CleanupStack::PushL(trans);
	SetTransportL(trans);
	CleanupStack::Pop(trans);
	}


void CTOCSPEngine::SetTestTransportL(const TDesC& aResponseFile, const TDesC* aRequestFile)
	{
	CTOCSPTransport* trans = CTOCSPTransport::NewL(aResponseFile, aRequestFile);
	CleanupStack::PushL(trans);
	SetTransportL(trans);
	CleanupStack::Pop(trans);
	}

void CTOCSPEngine::SetOcspSupportTransportL()
	{
	iIap = 0;
	Swi::COcspSupportTransport* trans = Swi::COcspSupportTransport::NewL(iIap);

	CleanupStack::PushL(trans);
	SetTransportL(trans);
	CleanupStack::Pop(trans);
	}

void CTOCSPEngine::SetTransportRetryCount(TUint aRetryCount)
	{
	__ASSERT_DEBUG(iTransport != NULL, Panic(KErrTOCSPScriptParameterError));
	iParams->SetRetryCount(aRetryCount);
	}

void CTOCSPEngine::SetTransportTimeout(TInt aTimeout)
	{
	__ASSERT_DEBUG(iTransport != NULL, Panic(KErrTOCSPScriptParameterError));
	iParams->SetTimeout(aTimeout);
	}

void CTOCSPEngine::SetTransportL(MOCSPTransport* aTransport)
	{
	// To log the request, we wrap the transport in our request logger transport
	if (iRequestLog)
		{
		aTransport = CTOCSPRequestLogger::NewL(*iRequestLog, aTransport);
		}
	iTransport = aTransport;
	iParams->SetTransport(aTransport);
	}

void CTOCSPEngine::SetFilterParameters(TInt aNumDelayResp, TInt aCountDropResp,
			TInt countCorruptHTTPDataHeader, TInt countCorruptHTTPDataBodySizeLarge, TInt countCorruptHTTPDataBodySizeSmall, 
			TInt aCountCorruptOCSPData, 
			TInt aCountInternalErrorResp, TInt aCountTryLaterResp,
			TInt aCountSigValidateFailure)
	{
	iNumDelayResp = aNumDelayResp;
	iCountDropResp = aCountDropResp;
	iCountCorruptHTTPDataHeader = countCorruptHTTPDataHeader;
	iCountCorruptHTTPDataBodySizeLarge = countCorruptHTTPDataBodySizeLarge;
	iCountCorruptHTTPDataBodySizeSmall = countCorruptHTTPDataBodySizeSmall;
	iCountCorruptOCSPData = aCountCorruptOCSPData;
	iCountInternalErrorResp = aCountInternalErrorResp;
	iCountTryLaterResp = aCountTryLaterResp;
	iCountSigValidateFailure = aCountSigValidateFailure;
	}

void CTOCSPEngine::AddCertL(const TDesC8& aSubject, const TDesC8& aIssuer)
	{
	// Create certificate objects and keep hold of them - these are added
	// to the client later

	CX509Certificate* subject = CX509Certificate::NewLC(aSubject);
	User::LeaveIfError(iSubjectCerts.Append(subject));
	CleanupStack::Pop(subject);  // Now owned through iCerts

	CX509Certificate* issuer = CX509Certificate::NewLC(aIssuer);
	User::LeaveIfError(iIssuerCerts.Append(issuer));
	CleanupStack::Pop(issuer);  // Now owned through iCerts
	
	// Add certificates to parameter
	iParams->AddCertificateL(*subject, *issuer);
	}

void CTOCSPEngine::ReadTransportLogL()
	{
	// Read and process the transport filter log (if it exists)
	RFile file;
	TInt size;
	if (file.Open(iFs, KFilterLogFileName, EFileShareAny | EFileRead) != KErrNone)
		{
		return;
		}

	CleanupClosePushL(file);
	file.Size(size);
	RFileReadStream fileStream(file);
	CleanupClosePushL(fileStream);
	RBuf8 logBuffer;
	logBuffer.CreateL(size);
	CleanupClosePushL(logBuffer);
	fileStream.ReadL(logBuffer, size);
	TInt readPos = 0;
	TPtrC8 currentLine;
	while (MiscUtil::ReadNonEmptyLineL(logBuffer, readPos, currentLine))
		{
		TLex8 lex(currentLine);
		TPtrC8 methodName = lex.NextToken();

		// Validate and store the entry
		TTransportLog logEntry;
		if (methodName.Length() <= 4)
			{
			logEntry.iHttpMethod.Copy(methodName);
			logEntry.iHttpMethod.UpperCase();
			}

		// Get the timestamps and calculate transaction duration
		TInt64 startTime = 0;
		TInt64 endTime = 0;
		lex.SkipSpaceAndMark(); User::LeaveIfError(lex.Val(startTime));
		lex.SkipSpaceAndMark(); User::LeaveIfError(lex.Val(endTime));
		logEntry.iTransDurationMs = static_cast<TInt64>((endTime - startTime) / KTimeMilliToMicro);

		iTransportLog.AppendL(logEntry);
		}
	CleanupStack::PopAndDestroy(3, &file); // fileStream, logBuffer
	}

TBool CTOCSPEngine::ReadLineL(const TDesC8& aBuffer, TInt& aPos, TPtrC8& aLine) const
	{
	TBool endOfBuffer = EFalse;	
	aLine.Set(NULL, 0);

	TInt bufferLength = aBuffer.Length();	
	__ASSERT_ALWAYS(aPos >=0 && aPos <= bufferLength, User::Leave(KErrArgument));

	// Skip blank lines
	while (aPos < bufferLength) 
		{
		TChar  c = aBuffer[aPos];
		if (c != '\r' && c != '\n')
			{
			break;
			}
		aPos++;
		}

	// Find the position of the next delimter		
	TInt endPos = aPos;	
	while (endPos < bufferLength)
		{
		TChar c = aBuffer[endPos];
		if (c == '\n' || c == '\r') 
			{
			break;
			}	
		++endPos;
		}

	if (endPos != aPos)	
		{
		TInt tokenLen = endPos - aPos;
		aLine.Set(&aBuffer[aPos], tokenLen);
		}
	else 
		{
		return ETrue; // End of buffer
		}			

	aPos = endPos;
	return endOfBuffer;
	}

void CTOCSPEngine::LogResponseDetailsL()
	{
	if (iVerbose)
		{
		_LIT(KDateFormat, "%F%H:%T %D/%M/%Y");
		TBuf<32> timeString;

		iLog.LogL(_L("Response details:\n"));

		TTime timeNow;
		timeNow.UniversalTime();
		timeNow.FormatL(timeString, KDateFormat);
		iLog.LogL(_L("  Response generation time == "));
		iLog.LogL(timeString);
		iLog.LogL(_L("\n"));

		TBuf<10> indexText;

		for (int i = 0 ; i < iClient->TransactionCount() ; ++i)
			{
			const COCSPResponse* response = iClient->Response(i);

			iLog.LogL(_L("  Response "));
			indexText.Zero();
			indexText.Num(i);
			iLog.LogL(indexText);
			iLog.LogL(_L(":\n"));

			if (response)
				{
				response->ProducedAt().FormatL(timeString, KDateFormat);
				iLog.LogL(_L("    producedAt == "));
				iLog.LogL(timeString);
				iLog.NewLineL();
	
				TInt count = response->CertCount();
				for (TInt index = 0; index < count; ++index)
					{
					const COCSPResponseCertInfo& certInfo = response->CertInfo(index);

					iLog.LogL(_L("    Cert "));
					indexText.Zero();
					indexText.Num(index);
					iLog.LogL(indexText);
					iLog.LogL(_L(" Results:"));
			
					iLog.LogL(_L("\n      status == "));
					iLog.LogL(TranslateResultL(certInfo.Status()));

					certInfo.ThisUpdate().FormatL(timeString, KDateFormat);
					iLog.LogL(_L("\n      thisUpdate == "));
					iLog.LogL(timeString);
					iLog.NewLineL();

					if (certInfo.NextUpdate())
						{
						timeString.Zero();
						certInfo.NextUpdate()->FormatL(timeString, KDateFormat);
						iLog.LogL(_L("      nextUpdate == "));
						iLog.LogL(timeString);
						iLog.NewLineL();
						}

					if (certInfo.RevocationTime())
						{
						timeString.Zero();
						certInfo.RevocationTime()->FormatL(timeString, KDateFormat);
						iLog.LogL(_L("      revocationTime == "));
						iLog.LogL(timeString);
						iLog.NewLineL();
						}
					}
				}
			else
				{
				iLog.LogL(_L("    missing"));			
				iLog.NewLineL();
				}

			const TOCSPOutcome& outcome = iClient->Outcome(i);
			iLog.LogL(_L("    Validation outcome:"));
			iLog.NewLineL();
			iLog.LogL(_L("      status == "));
			iLog.LogL(TranslateStatusL(outcome.iStatus));
			iLog.NewLineL();
			iLog.LogL(_L("      result == "));
			iLog.LogL(TranslateResultL(outcome.iResult));
			iLog.NewLineL();
			}

		iLog.LogL(_L("  OCSP Summary result == "));
		iLog.LogL(TranslateResultL(iClient->SummaryResult()));
		iLog.NewLineL();
		}
	}


void CTOCSPEngine::RunL()
	{
	TInt err = iStatus.Int();
	if( KErrNotFound != err && KErrNone != err )
		{
		iLog.LogL(_L("Client request completed with code:"));  
		iLog.LogL(err);
		iLog.NewLineL();
		User::LeaveIfError(err);
		}

	if( KErrNotFound == err )
		{
		iLog.LogL(_L("Item not found leave code"));
		iLog.NewLineL();
		// Handle error
		User::Leave(err);
		}

	switch (iState)
		{
		case EInitCertStore:
			// Remove all certificates so we always start from the same state 
			iCertUtils->RemoveCertsL(*iUnifiedCertStore, iStatus);
			iState = ERemovingCerts;
			SetActive();
			break;
			
		case EAddingCert:
		case ERemovingCerts:
			User::RequestComplete(iOriginalRequestStatus, KErrNone);
			break;

		case EChecking:
			// Cancel the timer
			iTimer->Cancel();
			ReadTransportLogL();
			LogResponseDetailsL();
			User::RequestComplete(iOriginalRequestStatus, KErrNone);
			break;

		default:
            User::Panic(_L("TOCSP"), 1);
			break;
		}
	}

TInt CTOCSPEngine::RunError(TInt aError)
    {
    User::RequestComplete(iOriginalRequestStatus, aError);
    return KErrNone;
    }

void CTOCSPEngine::DoCancel()
	{
    switch (iState)
        {
		case EInitCertStore:
			if (iUnifiedCertStore)
				{
				iUnifiedCertStore->CancelInitialize();
				}
			break;
			
		case EAddingCert:
		case ERemovingCerts:
            iCertUtils->Cancel();
            break;

		case EChecking:
            iClient->CancelCheck();
            break;

		default:
            User::Panic(_L("TOCSP"), 1);
			break;
        }

	Reset();
    User::RequestComplete(iOriginalRequestStatus, KErrCancel);
	}

// Implementation of MTimerRun
void CTOCSPEngine::TimerRun(TInt /*aError*/)
	{
	// Cancellation timer expired
	this->Cancel();
	}

void CTOCSPEngine::SetValidationTimeL(const TTime& aWhen)
	{
	iParams->SetValidationTimeL(aWhen);
	}


void CTOCSPEngine::SetNonce(const TBool aNonce)
	{
	iParams->SetUseNonce(aNonce);
	}


void CTOCSPEngine::SetMaxStatusAgeL(TUint aMaxAge)
	{
	iParams->SetMaxStatusAgeL(aMaxAge);
	}

void CTOCSPEngine::InitDirectAuthL()
	{
	// Add OCSP test harness to cert store client list
	TName name(_L("TOCSP"));
	iCertUtils->AddApplicationL(name, TUid::Uid(KTOCSP_UID));
	}

void CTOCSPEngine::CleanUpDirectAuthL(TRequestStatus& aStatus)
	{
	iOriginalRequestStatus = &aStatus;
	aStatus = KRequestPending;

	// Should always succeed, as called after InitDirectAuthL
	iCertUtils->RemoveApplicationL(TUid::Uid(KTOCSP_UID));

	//This should remove the X509Certs, which is the only certificate type used by the test.
	iCertUtils->RemoveCertsL(*iUnifiedCertStore, iStatus);
	iState = ERemovingCerts;
	SetActive();
	}

void CTOCSPEngine::AddDirectAuthorisationCert(const TDesC& aCert,
                                              const TDesC& aLabel,
                                              TRequestStatus& aStatus)
	{
    TRAPD(err, DoAddDirectAuthorisationCertL(aCert, aLabel, aStatus));
    if (err != KErrNone)
        {
        Cancel();
        TRequestStatus* status = &aStatus;
        User::RequestComplete(status, err);
        }
    }

void CTOCSPEngine::DoAddDirectAuthorisationCertL(const TDesC& aCert,
												 const TDesC& aLabel,
												 TRequestStatus& aStatus)
	{
	TPtrC cert = aCert;
	HBufC8* certData = ReadDataL(iFs, cert);
	CleanupStack::PushL(certData);

	// Make and store the certificate
	CX509Certificate* cert2 = CX509Certificate::NewLC(*certData);
	User::LeaveIfError(iSigningCerts.Append(cert2));
	CleanupStack::Pop(cert2);  // Now owned through iCerts
	CleanupStack::PopAndDestroy(certData);
	
	iUseDirectAuthorisation = ETrue;

	iCert = aCert;
	iLabel = aLabel;	

	iOriginalRequestStatus = &aStatus;
	aStatus = KRequestPending;
	iState = EAddingCert;

	iCertUtils->AddCert(iLabel, EX509Certificate, ECACertificate, KTOCSP_UID,
						KNullDesC, iCert, *iUnifiedCertStore, iStatus);
	SetActive();
	}


/**
	Called when "AUTHORISATIONCERT AUTHCERTNONE" is parsed.
	Sets a flag so an instance of COCSPDirectAuthorisationScheme
	is allocated in PrepareAuthorisationSchemeL().
	
	This function should be used to test direct authorisation
	without supplying a cert.  When a cert is supplied, use
	AddDirectAuthorisationCert().
	
	@see AddDirectAuthorisationCert
	@see UseCADelegateAuthorisation
 */
void CTOCSPEngine::UseDirectAuthorisation()
	{
	iUseDirectAuthorisation = ETrue;	
	}

/**
	Called when a "CADELEGATE" command is parsed.  It sets
	a flag so that an instance of COCSPDelegateAuthorisationScheme
	is allocated in PrepareAuthorisationSchemeL().

	@see AddDirectAuthorisationCert
	@see UseCADirectAuthorisation
 */
void CTOCSPEngine::UseCADelegateAuthorisation()
	{
	iUseCADelegateAuthorisation = ETrue;
	}

/**
	Called when "CADIRECT" command is parsed.  It sets
	a flag so that an instance of COCSPCaDirectAuthorisationScheme
	is allocated in PrepareAuthorisationSchemeL().

	@see AddDirectAuthorisationCert
	@see UseCADelegateAuthorisation
 	@see UseDirectAuthorisation
*/
void CTOCSPEngine::UseCADirectAuthorisation()
	{
	iUseCADirectAuthorisation = ETrue;
	}

/**
	Called when "ALLSCHEMES" command is parsed.  It sets
	a flag so that all schemes are allocated in 
	PrepareAuthorisationSchemeL().

	@see AddDirectAuthorisationCert
	@see UseCADelegateAuthorisation
	@see UseCADirectAuthorisation
*/
void CTOCSPEngine::UseAllAuthorisationSchemes()
	{
	iUseAllSchemes = ETrue;
	}

void CTOCSPEngine::PrepareAuthorisationL()
	{
	if (iUseAllSchemes)
		{
		// iUseDirectAuthorisation may also be set because
		// "AUTHORISATIONCERT" commands have been parsed.
		ASSERT(!(iUseCADelegateAuthorisation || iUseCADirectAuthorisation));

		// This assumes __SECURITY_PLATSEC_ARCH__ is defined.  This
		// is the case because AddAllAuthorisationSchemesL() is added
		// to 9.1 onwards.
		iParams->AddAllAuthorisationSchemesL(TUid::Uid(KTOCSP_UID), *iUnifiedCertStore);
		return;
		}

	if (iUseDirectAuthorisation)
		{
		// Register direct authorisation object with OCSP validator
		COCSPDirectAuthorisationScheme* scheme =  
			COCSPDirectAuthorisationScheme::NewLC(TUid::Uid(KTOCSP_UID), *iUnifiedCertStore);
		iParams->AddAuthorisationSchemeL(scheme);
		CleanupStack::Pop(); // scheme, now owned by client
		}
	
	if (iUseCADelegateAuthorisation)
		{
		COCSPDelegateAuthorisationScheme* schemeDel =
			COCSPDelegateAuthorisationScheme::NewLC(*iUnifiedCertStore);
		iParams->AddAuthorisationSchemeL(schemeDel);
		CleanupStack::Pop(schemeDel); // scheme, now owned by client		
		}
	
	if (iUseCADirectAuthorisation)
		{
		COCSPCaDirectAuthorisationScheme* schemeCad =
			COCSPCaDirectAuthorisationScheme::NewLC();
		iParams->AddAuthorisationSchemeL(schemeCad);
		CleanupStack::Pop(schemeCad); // scheme, now owned by client		
		}
	}

// Set filter parameters
void CTOCSPEngine::DefineAndSetFilterParametersL()
	{
	TUid categoryUid = TUid::Uid(KFilterParametersCategoryUID);

	// Define the parameters
	RProperty::Define(categoryUid, KFilterParameterNumDelayResp, RProperty::EInt);
	RProperty::Define(categoryUid, KFilterParameterCountDropResp, RProperty::EInt);
	RProperty::Define(categoryUid, KFilterParameterCountCorruptHTTPDataHeader, RProperty::EInt);
	RProperty::Define(categoryUid, KFilterParameterCountCorruptHTTPDataBodySizeLarge, RProperty::EInt);
	RProperty::Define(categoryUid, KFilterParameterCountCorruptHTTPDataBodySizeSmall, RProperty::EInt);
	RProperty::Define(categoryUid, KFilterParameterCountCorruptOCSPData, RProperty::EInt);
	RProperty::Define(categoryUid, KFilterParameterCountInternalErrorResp, RProperty::EInt);
	RProperty::Define(categoryUid, KFilterParameterCountTryLaterResp, RProperty::EInt);
	RProperty::Define(categoryUid, KFilterParameterCountSigValidateFailure, RProperty::EInt);

	// and Set them
	User::LeaveIfError(RProperty::Set(categoryUid, KFilterParameterNumDelayResp, iNumDelayResp));
	User::LeaveIfError(RProperty::Set(categoryUid, KFilterParameterCountDropResp, iCountDropResp));
	User::LeaveIfError(RProperty::Set(categoryUid, KFilterParameterCountCorruptHTTPDataHeader, iCountCorruptHTTPDataHeader));
	User::LeaveIfError(RProperty::Set(categoryUid, KFilterParameterCountCorruptHTTPDataBodySizeLarge, iCountCorruptHTTPDataBodySizeLarge));
	User::LeaveIfError(RProperty::Set(categoryUid, KFilterParameterCountCorruptHTTPDataBodySizeSmall, iCountCorruptHTTPDataBodySizeSmall));
	User::LeaveIfError(RProperty::Set(categoryUid, KFilterParameterCountCorruptOCSPData, iCountCorruptOCSPData));
	User::LeaveIfError(RProperty::Set(categoryUid, KFilterParameterCountInternalErrorResp, iCountInternalErrorResp));
	User::LeaveIfError(RProperty::Set(categoryUid, KFilterParameterCountTryLaterResp, iCountTryLaterResp));
	User::LeaveIfError(RProperty::Set(categoryUid, KFilterParameterCountSigValidateFailure, iCountSigValidateFailure));
	}

void CTOCSPEngine::DeleteFilterParameters()
	{
	TUid categoryUid = TUid::Uid(KFilterParametersCategoryUID);
	RProperty::Delete(categoryUid, KFilterParameterNumDelayResp);
	RProperty::Delete(categoryUid, KFilterParameterCountDropResp);
	RProperty::Delete(categoryUid, KFilterParameterCountCorruptHTTPDataHeader);
	RProperty::Delete(categoryUid, KFilterParameterCountCorruptHTTPDataBodySizeLarge);
	RProperty::Delete(categoryUid, KFilterParameterCountCorruptHTTPDataBodySizeSmall);
	RProperty::Delete(categoryUid, KFilterParameterCountCorruptOCSPData);
	RProperty::Delete(categoryUid, KFilterParameterCountInternalErrorResp);
	RProperty::Delete(categoryUid, KFilterParameterCountTryLaterResp);
	RProperty::Delete(categoryUid, KFilterParameterCountSigValidateFailure);
	// Reset the params
	iNumDelayResp = iCountDropResp = iCountCorruptHTTPDataHeader = iCountCorruptHTTPDataBodySizeLarge = 
		iCountCorruptHTTPDataBodySizeSmall = iCountCorruptOCSPData = iCountInternalErrorResp = 
		iCountTryLaterResp = iCountSigValidateFailure = 0;
	}

void CTOCSPEngine::SetCancelTime(TInt aTime)
	{
	iCancelTime = aTime;
	}

void CTOCSPEngine::Check(TRequestStatus& aStatus)
	{
    TRAPD(err, DoCheckL(aStatus));
    if (err != KErrNone)
        {
        Cancel();
        TRequestStatus* status = &aStatus;
        User::RequestComplete(status, err);
        }
    }

void CTOCSPEngine::DoCheckL(TRequestStatus& aStatus)
 	{
	iOriginalRequestStatus = &aStatus;
	aStatus = KRequestPending;
	iState = EChecking;

	PrepareAuthorisationL();

	// Set filter parameters
	DefineAndSetFilterParametersL();

	if (iVerbose)
		{
		iLog.LogL(_L("Checking...\n"), ETrue);
		}

	iClient = COCSPClient::NewL(iParams);
	iParams = NULL; // Client takes ownership

	// Setup cancellation timer
	if (iCancelTime)
		{
		iTimer->After(iCancelTime * KTimeMilliToMicro);
		}
	iClient->Check(iStatus);
	SetActive();
 	}

void CTOCSPEngine::LogValidationL(const TOCSPOutcome& aOutcome) const
	{
	if (iVerbose)
		{
		iLog.LogL(_L("Validation complete:\n  status == "));
		iLog.LogL(TranslateStatusL(aOutcome.iStatus));
	
		// Output summary result
		iLog.LogL(_L("\n  summary result == "));
		iLog.LogL(TranslateResultL(aOutcome.iResult));
		iLog.NewLineL();
		}
	}


TPtrC CTOCSPEngine::TranslateStatusL(OCSP::TStatus aStatus)
	{
	switch ((TInt) aStatus)
		{
		case OCSP::ETransportError:
			return _L("Transport error");
		case OCSP::ETimeOut:
			return _L("Request timedout");
		case OCSP::EClientInternalError:
			return _L("Client internal error");
		case OCSP::ENoServerSpecified:
			return _L("No server specified");
		case OCSP::EInvalidURI:
			return _L("Invalid URI");
		case OCSP::EMalformedResponse:
			return _L("Malformed response");
		case OCSP::EUnknownResponseType:
			return _L("Unknown response type");
		case OCSP::EUnknownCriticalExtension:
			return _L("Unknown critical extension");
		case OCSP::EMalformedRequest:
			return _L("Server: Malformed request");
		case OCSP::EServerInternalError:
			return _L("Server: Internal error");
		case OCSP::ETryLater:
			return _L("Server: Try later");
		case OCSP::ESignatureRequired:
			return _L("Server: Signature required");
		case OCSP::EClientUnauthorised:
			return _L("Server: Client unauthorised");
		case OCSP::EMissingCertificates:
			return _L("Missing certificates");
		case OCSP::EResponseSignatureValidationFailure:
			return _L("Response signature validation failure");
		case OCSP::EThisUpdateTooLate:
			return _L("Time error: This update too late");
		case OCSP::EThisUpdateTooEarly:
			return _L("Time error: This update too early");
		case OCSP::ENextUpdateTooEarly:
			return _L("Time error: Next update too early");
		case OCSP::ECertificateNotValidAtValidationTime:
			return _L("Time error: Not valid at validation time");
		case OCSP::ENonceMismatch:
			return _L("Nonce mismatch");			
		case OCSP::EMissingNonce:
			return _L("Missing nonce");
		case OCSP::EValid:
			return _L("Valid");
		case TOCSP::ETooManyTransactions:
			return _L("(test) Too many transactions");
		case TOCSP::EURIMismatch:
			return _L("(test) URI mismatch");
		case TOCSP::ERequestMismatch:
			return _L("(test) Request mismatch");
		default:
			return _L("Unknown");
		}
	}


TPtrC CTOCSPEngine::TranslateResultL(OCSP::TResult aResult)
	{
	switch (aResult)
		{
		case OCSP::EGood:
			return _L("Good");
		case OCSP::ERevoked:
			return _L("Revoked");
		case OCSP::EUnknown:
			return _L("Unknown");
		default:
			return _L("Unknown");
		}
	}


HBufC8* CTOCSPEngine::ReadDataL(RFs& session, const TDesC& aFileName) const
	{
	RFile file;
	User::LeaveIfError(file.Open(session, aFileName, EFileRead | EFileShareReadersOnly));
	CleanupClosePushL(file);

	TInt size = 0;
	User::LeaveIfError(file.Size(size));

	HBufC8* data = HBufC8::NewLC(size);
	TPtr8 dataPtr = data->Des();
	User::LeaveIfError(file.Read(dataPtr));

	CleanupStack::Pop(data);
	CleanupStack::PopAndDestroy(); // Close file;
	
	return data;
	}


void CTOCSPEngine::LogRequestL(const TDesC& aFilename)
    {
	delete iRequestLog;
	iRequestLog = NULL;

	iRequestLog = aFilename.AllocL();
	}


void CTOCSPEngine::LogResponseL(const TDesC& aFilename)
    {
	RFs fs;
	User::LeaveIfError(fs.Connect());
	CleanupClosePushL(fs);

	TInt err = fs.MkDirAll(aFilename);
	if (err != KErrAlreadyExists)
		{
		User::LeaveIfError(err);
		}

	RFile file;
	User::LeaveIfError(file.Replace(fs, aFilename, EFileWrite));
	CleanupClosePushL(file);

	RFileWriteStream writeStream(file);
	CleanupClosePushL(writeStream);

	writeStream.WriteUint32L(iClient->TransactionCount());

	for (int i = 0 ; i < iClient->TransactionCount() ; ++i)
		{
		const COCSPResponse* response = iClient->Response(i);
		if (response)
			{
			const TPtrC8 data = response->Encoding();
			writeStream.WriteUint32L(data.Length());
			writeStream.WriteL(data);
			}
		else
			{
			writeStream.WriteUint32L(0);
			}
		}

	CleanupStack::PopAndDestroy(3); // writeStream, file, fs
    }


void CTOCSPEngine::SetVerbose(TBool aVerbose)
	{
	iVerbose = aVerbose;
	}

TBool CTOCSPEngine::TestSummaryL(OCSP::TResult aExpected)
	{
	TBool result = iClient->SummaryResult() == aExpected;

	if (iVerbose && !result)
		{
		iLog.LogL(_L("Summary result fail: expected "));
		iLog.LogL(aExpected);
		iLog.NewLineL();
		}

	return result;
	}

TBool CTOCSPEngine::TestOutcomeL(TInt aIndex, const TOCSPOutcome& aExpected)
	{
	TBool result = EFalse;
	// This assumes one-transaction-per-certificate behaviour
	if(iClient)
		{
		result = iClient->Outcome(aIndex) == aExpected;
		}

	if (iVerbose && !result)
		{
		iLog.LogL(_L("Outcome result fail for cert "));
		iLog.LogL(aIndex);
		iLog.LogL(_L(":\n"));
		//iLog.LogL(_L("Expected Result:\n"));
		iLog.LogL(_L("Expected Status == "));
		iLog.LogL(TranslateStatusL(aExpected.iStatus));
		iLog.NewLineL();
		iLog.LogL(_L("Expected Result == "));
		iLog.LogL(TranslateResultL(aExpected.iResult));
		iLog.NewLineL();
		}

	return result;
	}

void CTOCSPEngine::SetReponderCertCheck()
	{
	iParams->SetOCSPCheckForResponderCert(ETrue);
	}

void CTOCSPEngine::AddCertToStore(const TDesC& aCertFileName, const TDesC& aLabel, TCertificateOwnerType aCertType, TRequestStatus& aStatus)
	{
	iOriginalRequestStatus = &aStatus;
	aStatus = KRequestPending;
	iState = EAddingCert;
	
	iCert = aCertFileName;
	iLabel = aLabel;	
	
	iCertUtils->AddCert(iLabel, EX509Certificate, aCertType, KTOCSP_UID,
						KNullDesC, iCert, *iUnifiedCertStore, iStatus);		
	
	SetActive();
	}

TBool CTOCSPEngine::TestTransportL(TInt aRetryCountNum, const TDesC& aExpectedHttpMethod, 
		TInt aExpectedRespTimeMin, TInt aExpectedRespTimeMax)
	{
	// This assumes one-tranaction-per-certificate behaviour
	// This also assumes test case includes only certificate revocation check request (which 
	// means CHECKRESPONDERCERT command cannot be used with this)
	// Ensure we have read the entry from the transport log
	TBool result = ETrue;
	if (iTransportLog.Count() < (aRetryCountNum + 1))
		{
		if (iVerbose)
			{
			iLog.LogL(_L("Transport filter log contains insufficient entries: "));
			iLog.LogL(_L("\n  Expected number of entries: "));
			iLog.LogL(aRetryCountNum + 1);
			iLog.LogL(_L("\n  Actual entries: "));
			iLog.LogL(iTransportLog.Count());
			iLog.NewLineL();
			}
		result = EFalse;
		}

	TTransportLog& logEntry = iTransportLog[aRetryCountNum];
	// Convert to 8-bit
	RBuf8 expectedMethod;
	expectedMethod.CreateL(aExpectedHttpMethod.Length());
	CleanupClosePushL(expectedMethod);
	expectedMethod.Copy(aExpectedHttpMethod);
	expectedMethod.UpperCase();
	if (expectedMethod != logEntry.iHttpMethod)
		{
		if (iVerbose)
			{
			iLog.LogL(_L("Transport outcome result failed for send number: "));
			iLog.LogL(aRetryCountNum);
			iLog.LogL(_L("\n  Expected method: "));
			iLog.LogL(expectedMethod);
			iLog.LogL(_L("\n  Got method: "));
			iLog.LogL(logEntry.iHttpMethod);
			iLog.NewLineL();
			}
		result = EFalse;
		}
	CleanupStack::PopAndDestroy(&expectedMethod);

	// Test the range of response time
	if (aExpectedRespTimeMin > logEntry.iTransDurationMs || aExpectedRespTimeMax < logEntry.iTransDurationMs)
		{
		if (iVerbose)
			{
			iLog.LogL(_L("Transport outcome result failed for send number: "));
			iLog.LogL(aRetryCountNum);
			iLog.LogL(_L("\n  Expected response time range (ms): "));
			iLog.LogL(aExpectedRespTimeMin);
			iLog.LogL(_L(" to "));
			iLog.LogL(aExpectedRespTimeMax);
			iLog.LogL(_L("\n  Actual response time (ms): "));
			iLog.LogL(logEntry.iTransDurationMs);
			iLog.NewLineL();
			}
		result = EFalse;
		}
	else if (iVerbose)
		{
		iLog.LogL(_L("Send number: "));
		iLog.LogL(aRetryCountNum);
		iLog.LogL(_L(" Response duration(ms): "));
		iLog.LogL(logEntry.iTransDurationMs);
		iLog.NewLineL();
		}

	return result;
	}

TBool CTOCSPEngine::TestTransportRetryL(TInt aRetryCount)
	{
	// This assumes one-tranaction-per-certificate behaviour
	// This also assumes test case includes only certificate revocation check request (which means 
	// CHECKRESPONDERCERT command cannot be used with this)
	// Check the number of retries
	TBool result = ETrue;
	if (iTransportLog.Count() != aRetryCount)
		{
		if (iVerbose)
			{
			iLog.LogL(_L("Transport retry outcome result failed:"));
			iLog.LogL(_L("\n  Expected number of retries: "));
			iLog.LogL(aRetryCount);
			iLog.LogL(_L("\n  Actual retries: "));
			iLog.LogL(iTransportLog.Count());
			iLog.NewLineL();
			}
		result = EFalse;
		}

	return result;
	}

void CTOCSPEngine::SetCheckCertsWithAiaOnly(TBool aCheckCertsWithAiaOnly)
	{
	iParams->SetCheckCertsWithAiaOnly(aCheckCertsWithAiaOnly);
	}

void CTOCSPEngine::SetUseAIA(TBool aUseAIA)
	{
	iParams->SetUseAIA(aUseAIA);
	}
