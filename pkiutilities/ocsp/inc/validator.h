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
// request.h
// Header for OCSP validator object.
// 
//

/**
 @file 
 @internalComponent 
*/

#ifndef __OCSP_VALIDATOR_H__
#define __OCSP_VALIDATOR_H__

#include <ocsp.h>
#include "ocsprequestandresponse.h"
/**
 * Validator.  Used to verify the legitimacy of an OCSP response, and the
 * certificates therein.  May specify time for certificate validation - if
 * omitted, the producedAt time in the response is used instead.
 */

NONSHARABLE_CLASS(COCSPValidator) : public CActive
	{
public:
	static COCSPValidator* NewL( const COCSPParameters& aParameters);
	
	~COCSPValidator();

	/**
	 * Specify Authorisation scheme(s) - doesn't take ownership
	 */
	
	void AddAuthorisationSchemeL(MOCSPAuthorisationScheme& aScheme);

	/**
	 * Specify the authorisation time (NULL => use producedAt from response)
	 * Default is to use producedAt.
	 */
	
	void SetValidationTimeL(const TTime* aValidationTime);

	/**
	 * Specify the permissable maximum age of the thisUpdate field in
	 * seconds. zero => don't check, default is 30 days. 
	 */

	void SetMaxStatusAge(TUint aMaxAge);

	/**
	 * Specify how much leeway we allow when comparing times - if we're using
	 * the device clock, this allows for the fact that it may be out by a
	 * certain amount.  Default is 5 seconds.
	 */
	
	void SetTimeLeeway(TUint aLeewaySeconds);

	/**
	 * Validate a request/response pair, setting aOutcome to the result.
	 * This is an asynchronous method.
	 */
	
	void Validate(const COCSPRequest& aRequest, COCSPResponse& aResponse,
				  TOCSPOutcome& aOutcome, TRequestStatus& aStatus);
		
protected:
	virtual void DoCancel();
	virtual void RunL();
	virtual TInt RunError(TInt aError);

 private:
 	void ConstructL();
	COCSPValidator( const COCSPParameters& aParameters);

	void DoValidateL(const COCSPRequest& aRequest, COCSPResponse& aResponse,
					 TOCSPOutcome& aOutcome, TRequestStatus& aStatus);

	// Specific validation areas - return false if we should give up now.
	TBool IsResponseWellFormed(); // Checks correct certs present in response.
	TBool ValidateTimeL();
	TBool ValidateNonce();

	void ProcessSchemeValidationL();
	void CheckSchemeValidationL();
	void FinalResponseValidationL();
	
	TTime ValidationTime() const;

	TBool TimeIsBeforeL(const TTime& aBefore, const TTime& aAfter);
	
	 void SendResponderCertL();
	 void ValidateResponderCertL();
	 
	 OCSP::TResult CheckOCSPStatus(const COCSPResponse* aResponse) const;
	 
private:

	// Array of integers, giving for each cert in the response the corresponding position in
	// the request
	RArray<TInt> iRequestIndex;
	
	const COCSPRequest* iRequest;
	COCSPResponse* iResponse;

	// User-supplied time for validation
	TTime* iValidationTime;

	// How old to allow thisUpdate to be
	TUint iMaxStatusAge;

	// Authorisation objects
	RPointerArray<MOCSPAuthorisationScheme> iAuthorisationScheme;

	TOCSPOutcome* iOutcome;
	TRequestStatus* iValidationStatus;

	enum TState
		{
		EValidating,
		EWaitingResponse,
		EValidateResponderCert,
		};

	TState iState;

	// index of the last authorisation scheme we used
	TInt iIndexScheme; 

	TUint iLeewaySeconds;
	
	TBool iResponderCertCheck;
	TBool iUseNonce;
	// ownership lies with the relevant auth scheme
	const CX509Certificate* iResponderCert;
	const CX509Certificate* iIssuerCert;
	COCSPRequest* iResponderCertRequest;
	COCSPResponse* iResponderCertResponse;
	COCSPTransaction* iTransaction;
	// ownership lies with COCSPClient
	const COCSPParameters* iParameters;
	MOCSPAuthorisationScheme* iSchemeInUse;
	};

#endif
