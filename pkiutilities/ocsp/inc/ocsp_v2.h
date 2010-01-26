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
// ocsp.h
// Header specifying client interface to the OCSP module.
// 
//

#ifndef __OCSP_H__
#define __OCSP_H__

/**
 * @file
 * @publishedPartner
 * @released
 */

#include <e32base.h>
#include <signed.h>
#include <f32file.h>
#include <hash.h>
#include <x500dn.h>
#include <x509cert.h>
#include <ct/rmpointerarray.h>
#include <cctcertinfo.h>
#include <asn1enc.h>
#include <asn1dec.h>

// Enums placed in OCSP namespace
namespace OCSP
	{
	// Globally-reserved error codes - range is -7601 to -7649
	enum 
		{
		KErrTransportFailure	= -7601,
		KErrInvalidURI			= -7602,
		KErrNoCertificates		= -7603,
		KErrTransportTimeout	= -7604,
		KErrServerNotFound		= -7605
		};

	/**
	 * What we think of the response, or why we haven't got one
	 * 
	 */
	enum TStatus 
		{
		// Error in communication with server
		ETransportError = 1,

		// Internal problem processing response
		EClientInternalError = 2,

		// No OCSP server specified
		ENoServerSpecified = 3,

		// Invalid sever URI
		EInvalidURI = 4,
		
		// Timeout error
		ETimeOut = 5,
		
		// Problems understanding the response
		EMalformedResponse = 10,
		EUnknownResponseType = 11,
		EUnknownCriticalExtension = 12,

		// Unsuccessful responses from server
		EMalformedRequest = 20,
		EServerInternalError = 21,
		ETryLater = 22,
		ESignatureRequired = 23,
		EClientUnauthorised = 24,
			
		// Response validation failures
		EMissingCertificates = 30,
		EResponseSignatureValidationFailure = 31,
		
		// Time problems
		EThisUpdateTooLate = 40,
		EThisUpdateTooEarly = 41,
		ENextUpdateTooEarly = 42,
		ECertificateNotValidAtValidationTime = 43,

		// Nonce error
		ENonceMismatch = 50,

		// Response sufficiently valid to use - clients to decide if
		// missing nonce is sufficiently serious to require rejection
		EMissingNonce = 51,
		
		EValid = 100,
		};
	
	/**
	 * What does the response say about the certificates?
	 * IMPORTANT: Do not change the order these are in
	 *
	 */
	enum TResult
		{
		EGood = 10, EUnknown = 20, ERevoked = 30
		};
	}
		
class TOCSPOutcome
	{
public:
	TOCSPOutcome(OCSP::TStatus aStatus, OCSP::TResult aResult)
		: iStatus(aStatus), iResult(aResult) {};

	TOCSPOutcome()
		: iStatus(OCSP::EClientInternalError), iResult(OCSP::EUnknown) {};

	TBool operator==(const TOCSPOutcome& aRhs) const
		{ return iStatus == aRhs.iStatus && iResult == aRhs.iResult; }

	OCSP::TStatus iStatus;
	OCSP::TResult iResult;
	};


#ifndef SYMBIAN_ENABLE_SPLIT_HEADERS
#include <ocsprequestandresponse.h>
#endif

#endif // __OCSP_H__
