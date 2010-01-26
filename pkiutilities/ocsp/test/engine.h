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
// Declase engine for testing OCSP module.  Knows nothing about the details of the
// script syntax.  Is point of contact between script-reading classes and the
// actual OCSP module itself.
// 
//

/**
 @file 
 @internalTechnology
*/

#ifndef __TOCSP_ENGINE_H__
#define __TOCSP_ENGINE_H__

#include <e32base.h>
#include <ocsp.h>
#include <callbacktimer.h>
#include "tcertutils.h"
#include "ocsprequestandresponse.h"

class CTOCSPLogger;

class COCSPRequest;
class COCSPTransaction;
class COCSPValidator;
class MOCSPTransport;

class CX509Certificate;
class CUnifiedCertStore;

class TTransportLog
	{
public:
	TBuf8<4> iHttpMethod;
	TInt iTransDurationMs;
	};

class CTOCSPEngine : public CActive, private MTimerObserver
	{
public:
	static CTOCSPEngine* NewL(CTOCSPLogger& aLog);
	~CTOCSPEngine();

	// COMMANDS TO ACCESS OCSP MODULE

	void StartL(TRequestStatus& aStatus); 	// Delete all old objects, create a new transaction object
	void EndL(TRequestStatus& aStatus); // Clean up after tests run
 	void SetURIL(const TDesC8& aURI, TBool aUseAIA=ETrue);


	void SetDefaultTransportL();
	void SetTestTransportL(const TDesC& aResponseFile, const TDesC* aRequestFile);
	void SetOcspSupportTransportL();
	void SetTransportRetryCount(TUint aRetryCount);
	void SetTransportTimeout(TInt aTimeout);

	void SetNonce(const TBool aNonce);
	void AddCertL(const TDesC8& aSubject, const TDesC8& aIssuer);

	void SetValidationTimeL(const TTime& aWhen);
	void AddDirectAuthorisationCert(const TDesC& aCert, const TDesC& aLabel,
									TRequestStatus& aStatus);
	void UseDirectAuthorisation();
	void UseCADelegateAuthorisation();
	void UseCADirectAuthorisation();
	void UseAllAuthorisationSchemes();
	void SetMaxStatusAgeL(TUint aMaxAge);
		
	void SetCancelTime(TInt aTime);
	void Check(TRequestStatus& aStatus);
	TBool TestSummaryL(OCSP::TResult aExpected);
	TBool TestOutcomeL(TInt aCertIndex, const TOCSPOutcome& aExpected);
	TBool TestTransportL(TInt aRetryCountNum, const TDesC& aExpectedHttpMethod,
						TInt aExpectedRespTimeMin, TInt aExpectedRespTimeMax);
	TBool TestTransportRetryL(TInt aRetryCount);

    void LogResponseL(const TDesC& aFilename);
    void LogRequestL(const TDesC& aFilename);

	static TPtrC TranslateStatusL(OCSP::TStatus aStatus);
	static TPtrC TranslateResultL(OCSP::TResult aResult);

	/** Set whether to print informational messages. */
	void SetVerbose(TBool aVerbose);
	
	void SetReponderCertCheck();
	void AddCertToStore(const TDesC& aCertFileName, const TDesC& aLabel, 
						TCertificateOwnerType aCertType, TRequestStatus& aStatus);
	void SetFilterParameters(TInt aCountDropResp, TInt aNumDelayResp,
			TInt countCorruptHTTPDataHeader, TInt countCorruptHTTPDataBodySizeLarge, TInt countCorruptHTTPDataBodySizeSmall,
			TInt aCountCorruptOCSPData,
			TInt aCountInternalErrorResp, TInt aCountTryLaterResp,
			TInt aCountSigValidateFailure);
	void SetCheckCertsWithAiaOnly(TBool aCheckCertsWithAiaOnly);
	void SetUseAIA(TBool aUseAIA);
	
private:
	CTOCSPEngine(CTOCSPLogger& aLog);
	void ConstructL();
	void Destroy();
	
	void Reset();
	void PrepareAuthorisationL();
	void DefineAndSetFilterParametersL();
	void DeleteFilterParameters();
	void LogValidationL(const TOCSPOutcome& aOutcome) const;
	void LogRequestCompleteL();
	void ReadTransportLogL();
	TBool ReadLineL(const TDesC8& aBuffer, TInt& aPos, TPtrC8& aLine) const;
	void LogResponseDetailsL(void);
	void InitDirectAuthL();
	void CleanUpDirectAuthL(TRequestStatus& aStatus);
	void SetTransportL(MOCSPTransport* aTransport);

	// Implementation of CActive
	void RunL();
	void DoCancel();
    TInt RunError(TInt aError);

    // Implementation of MTimerRun
	void TimerRun(TInt aError);


    HBufC8* ReadDataL(RFs& session, const TDesC& aFileName) const;

	void DoAddDirectAuthorisationCertL(const TDesC& aCert, const TDesC& aLabel, 
		TRequestStatus& aStatus);
    void DoCheckL(TRequestStatus& aStatus);

private:
	enum TState 
		{
		EInitCertStore,
		EAddingCert,
		ERemovingCerts,
        EChecking,
		};

private:
	RFs iFs;
	CUnifiedCertStore* iUnifiedCertStore;
	TState iState;
	TRequestStatus* iOriginalRequestStatus;

	CTOCSPLogger& iLog;

	// OCSP objects
	COCSPParameters* iParams;
	COCSPClient* iClient;

	// Have we added a direct authorisation scheme object?
	TBool iUseDirectAuthorisation;
	/** Set to use CA Delegate authorisation when the certificate is checked. */
	TBool iUseCADelegateAuthorisation;
	/**
		When this is set the response is checked to see if it signed
		by the issuing CA.
	 */
	TBool iUseCADirectAuthorisation;
	/**
		Set when "ALLSCHEMES" command is parsed.  This uses the
		COCSPParameters::AddAllAuthorisationSchemesL() function.
	 */
	TBool iUseAllSchemes;

	// The cert objects - we own them
	RPointerArray<CX509Certificate> iSubjectCerts;
	RPointerArray<CX509Certificate> iIssuerCerts;
	RPointerArray<CX509Certificate> iSigningCerts;

	CCertUtils* iCertUtils;

	TBuf<100> iCert;
	TBuf<100> iLabel;

	TBool iVerbose;

	// The filename to log requests to, or null
	HBufC* iRequestLog;
	
	TUint32 iIap;

	MOCSPTransport* iTransport;

	// Http transport filter settings
	TInt iNumDelayResp;
	TInt iCountDropResp;
	TInt iCountCorruptHTTPDataHeader;
	TInt iCountCorruptHTTPDataBodySizeLarge;
	TInt iCountCorruptHTTPDataBodySizeSmall;
	TInt iCountCorruptOCSPData;
	TInt iCountInternalErrorResp;
	TInt iCountTryLaterResp;
	TInt iCountSigValidateFailure;

	// Hold information read from the transport filter log
	RArray<TTransportLog> iTransportLog;

	// If non-zero the time interval in milliseconds to cancel issued request
	TInt iCancelTime;
	CCallbackTimer* iTimer;
	};

#endif
