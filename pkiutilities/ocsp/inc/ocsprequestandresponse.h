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

#ifndef OCSPREQUESTANDRESPONSE_H
#define OCSPREQUESTANDRESPONSE_H

/**
 * @file
 * @internalTechnology
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

#ifdef SYMBIAN_ENABLE_SPLIT_HEADERS
#include "ocsp.h"
#endif

#define KOCSPNonceBytes 16

// Default values for transport parameters
const TUint KTransportDefaultRequestRetryCount 	= 1;	// No retry
const TInt KTransportDefaultRequestTimeout 		= -1;	// Timeout disabled

class COCSPRequestCertInfo;
class COCSPResponseCertInfo;
class COCSPCertID;
class MOCSPTransport;
class COCSPResponse;
class COCSPTransportHandler;
class MOCSPAuthorisationScheme;
class COCSPTransaction;
class COCSPValidator;
class COCSPRequestCertInfo;
class COCSPResponseDecoder;

class CASN1EncBase;
class CASN1EncContainer;
class TASN1DecGeneric;

class CX509Certificate;
class CX509CertChain;
class CPKIXCertChainBase;
class COCSPParameters;


/**
 * Represents an OCSP protocol request.
 */

class COCSPRequest : public CBase
	{
public:

	/**
	 * Get the nonce used in the request.  Returns NULL if a nonce was not used.
	 */

	IMPORT_C const TDesC8* Nonce() const;

	/**
	 * Get the number of certificates used in a response
	 */
	
	IMPORT_C TInt CertCount() const;

	/**
	 * Get a COCSPRequestCertInfo object containing details about one of the
	 * certificates present in the request.
	 */

	IMPORT_C const COCSPRequestCertInfo& CertInfo(TUint aIndex) const;
	
public:

	// Not exported
	static COCSPRequest* NewLC(TBool aUseNonce);
	static COCSPRequest* NewL(TBool aUseNonce);
	
	~COCSPRequest();
	void AddCertificateL(const CX509Certificate& aSubject,
						 const CX509Certificate& aIssuer);

private:

	COCSPRequest();
	void ConstructL(TBool aUseNonce);

	HBufC8* iNonce;
	RPointerArray<const COCSPRequestCertInfo> iCertInfos;
	};

/**
 * Information about a certificate sent as part of an OCSP request.
 */

class COCSPRequestCertInfo : public CBase
	{
public:

	/**
	 * Get the certificate present in the request.
	 */
	
	IMPORT_C const CX509Certificate& Subject() const;

	/**
	 * Get the issuer of the certificate present in the request.
	 */
	
	IMPORT_C const CX509Certificate& Issuer() const;

public:

	// Not exported
	static COCSPRequestCertInfo* NewLC(
		const CX509Certificate& aSubject,
		const CX509Certificate& aIssuer);

	~COCSPRequestCertInfo();

	const COCSPCertID& CertID() const;

 private:
	
	COCSPRequestCertInfo(const CX509Certificate& aIssuer,
						 const CX509Certificate& aSubject);
	void ConstructL();

private:

	const CX509Certificate& iSubject;
	const CX509Certificate& iIssuer;
	COCSPCertID* iCertID;
	};


/**
 * Represents an OCSP protocol response.
 */

class COCSPResponse : public CSignedObject
	{
public:

	/**
	 * Get the number of certificates statuses present in the response.
	 */

	IMPORT_C TInt CertCount() const;

	/**
	 * Get a COCSPResponseCertInfo object containing details about one of the
	 * certificate statuses present in the response.
	 */

	IMPORT_C const COCSPResponseCertInfo& CertInfo(TUint aIndex) const;

	/**
	 * Get the producedAt time for the response.
	 */

	IMPORT_C TTime ProducedAt() const;

	/**
	 * Get the archiveCutoff time for the response, or NULL if it was not present.
	 */

	IMPORT_C const TTime* ArchiveCutoff() const;

public:

	// This class is created and initialised by the response decoder
	friend class COCSPResponseDecoder;
	
	~COCSPResponse();

	/**
	 * Get the index for the given cert, or KErrNotFound.
	 */
	
	TInt Find(const COCSPCertID& aCertID) const;

	// Enums to use in DataElementEncoding
	enum
		{
		ECertificateChain,
		ENonce,
		EResponderIDName,    // Only one of the ResponderIDs won't be NULL
		EResponderIDKeyHash
		};

  	// From CSignedObject
  	const TPtrC8* DataElementEncoding(const TUint aIndex) const;

private:

	COCSPResponse();

  	// From CSignedObject
	const TPtrC8 SignedDataL() const;
	void InternalizeL(RReadStream& aStream);

private:

	// Time of response, and of archiveCutoff extension (if present)
	TTime iProducedAt;
	TTime* iArchiveCutoff;

	// Value of nonce in response extension (if present)
	TPtrC8 iNonce;

	// Point to the signing certificates in the response
	TPtrC8 iSigningCerts;

	// Point to the signed portion of the data
	TPtrC8 iSignedData;

	// There are two ways the responder ID can be specified - only
	// one of these will be used
	TPtrC8 iResponderIDName;
	TPtrC8 iResponderIDKeyHash;

	RPointerArray<COCSPResponseCertInfo> iCertInfos;
	};


/**
 * Information about a certificate status, as given in OCSP response
 * singleResponse data item.
 */

class COCSPResponseCertInfo : public CBase
	{
public:
	
	IMPORT_C OCSP::TResult Status() const;
	IMPORT_C TTime ThisUpdate() const;
	IMPORT_C const TTime* NextUpdate() const;     // NULL if nextUpdate not set
	IMPORT_C const TTime* RevocationTime() const; // NULL if not revoked

public:
	
	static COCSPResponseCertInfo* NewLC(CArrayPtr<TASN1DecGeneric>& aItems);
	~COCSPResponseCertInfo();
	COCSPCertID& CertID() const;
	
private:
	void ConstructL(CArrayPtr<TASN1DecGeneric>& aItems);

private:
	OCSP::TResult iStatus;

	TTime iThisUpdate;
	TTime* iNextUpdate;
	TTime* iRevocationTime;

	COCSPCertID* iCertID;
	};

/**
	Utility class contains functions common to all validation
	schemes.
 */
namespace OCSPUtils
	{
	TBool IsAIAForOCSPPresentL(const CX509Certificate& aCert);
	
	TBool DoesResponderIdMatchCertL( const COCSPResponse& aResponse, 
											const CX509Certificate& aResponderCert);
	TBool DoesDNNameMatchL( 	const COCSPResponse& aResponse, const CX509Certificate& aCert);
	TBool DoesIssuerKeyMatchL(const COCSPResponse& aResponse, const CX509Certificate& aCert);
	TBool IsResponseSignedByCertL( COCSPResponse* aResponse, const CX509Certificate& aCert);
	TBool DoesCertHaveOCSPNoCheckExt( const CX509Certificate& aCert);
	TDesC8* ServerUriL(	const CX509Certificate& aCert, const COCSPParameters* aParameters );
	TBool IsUriAvailableL( const CX509Certificate& aCert, const COCSPParameters* aParameters );
	TDesC8* GetAIAL(const CX509Certificate& aCert);
	TBool DoesCertHaveOCSPSigningExtL(const CX509Certificate& aCert);
	CX509Certificate* GetResponderCertLC(const TDesC8& aEncodedCerts);
	}

// Base class interface for authorisation schemes, for plugging into the validator object
class MOCSPAuthorisationScheme
	{
public:
	virtual ~MOCSPAuthorisationScheme() {};

	virtual void ValidateL(
		OCSP::TStatus& aOCSPStatus, COCSPResponse& aResponse,
		const TTime aValidationTime, TRequestStatus& aStatus,
		const COCSPRequest& aRequest) = 0;

	virtual void CancelValidate() = 0;
	// Returns the responder certificate. This method shall return valid responder
	// certificate for delegate and direct schemes, for the cadirect scheme although
	// the CA cert is the response cert but it returns NULL as the need for retrieving
	// this certificate is to further send it for OCSP checking, this logic is only valid 
	// for delegate and direct schemes.
	virtual const CX509Certificate* ResponderCert() const = 0;
	};


class CPKIXValidationResultBase;
class CCertAttributeFilter;
class MCertStore;

class COCSPDelegateAuthorisationScheme : public CActive, public MOCSPAuthorisationScheme
/**
	Implement part of S2.2.2 of RFC 2560.
	
	"a CA Designated Responder (Authorized Responder) who holds
	a specially marked certificate issued directly by the CA,
	indicating that the responder may issue OCSP responses for
	that CA."
*/
	{
public:
	IMPORT_C static COCSPDelegateAuthorisationScheme* NewLC(MCertStore& aCertStore);
	virtual ~COCSPDelegateAuthorisationScheme();
	
	virtual void ValidateL(
		OCSP::TStatus& aOCSPStatus, COCSPResponse& aResponse,
		const TTime aValidationTime, TRequestStatus& aStatus,
		const COCSPRequest& aRequest);
	
	virtual void CancelValidate();
	const CX509Certificate* ResponderCert() const;
	
protected:
	virtual void RunL();
	virtual void DoCancel();
	TInt RunError(TInt aError);
	
private:
	COCSPDelegateAuthorisationScheme(MCertStore& aCertStore);
	void ConstructL();
	
	void ValidateDelegateCertL(const TDesC8& aResponseCertChain, const TTime aValidationTime);
	void OnChainValidationL();
	
	// searches on the basis of the issuer's subject name
	void ValidateFromRootsL();
	
	void OnRetrieveNextL();
	void OnRetrievingEntryL();
	
private:

	enum TDelegateAuthState
	{
	EOnChainValidation,
	ERetrieveNext,
	ERetrievingEntry,
	};
	/**
		This is a required argument for CPKIXCertChainBase, even
		though no certificates are used from the store.
	 */
	MCertStore& iCertStore;
	/**
		Pointer to client's OSCP status.  This object sets the
		status according to the result of the validation.  If
		the validation cannot be carried out, the default value
		is OCSP::EResponseSignatureValidationFailure.
	 */
	OCSP::TStatus* iOCSPStatus;
	/**
		Pointer to client's request status.  This is recorded
		on the call to Validate() and used to notify the client
		of completion later.
	 */
	TRequestStatus* iClientStatus;
	/**
		Array used to hold pointer to issuer cert.  This contains
		the pointer to the CA.
	 */
	RPointerArray<CX509Certificate> iRespSignIntCert;
	/**
		This object is used to chain the response signer back
		to the CA.
	 */
	CPKIXCertChainBase* iRespSignCertChainBase;
	/**
		The cert chain stores the validation result in
		this object.
		
		@see iRespSignCertChain;
	 */
	CPKIXValidationResultBase* iPKIXResultBase;
	/**
		Response from OCSP server.  Once it has been established
		that the certificate sent with the response is an authorised
		responder for the CA, the response is checked to ensure
		that it really is signed by that certificate.
		
		This is not const because, if DSA is used, then
		SetParametersL() will be called on the response before
		the signature is verified.
	 */
	COCSPResponse* iResponse;
	const COCSPRequest* iRequest;
	CCertAttributeFilter* iCertFilter;
	RMPointerArray<CCTCertInfo> iCertStoreEntries;
	TDelegateAuthState	iState;
	HBufC8* iEncodedCert;
	const CX509Certificate* iResponseCert;
	TInt iCertCount;
	TTime iValidationTime;
	TBool iValidateFromResponse;
	
	};

class COCSPCaDirectAuthorisationScheme : public CBase, public MOCSPAuthorisationScheme
/**
	Implement part of S2.2 of RFC 2560.
	
	"The key used to sign the response MUST belong to one of the following...

	-- the CA who issued the certificate in question"
*/
	{
public:
	IMPORT_C static COCSPCaDirectAuthorisationScheme* NewLC();
	const CX509Certificate* ResponderCert() const;
	
private:
	virtual void ValidateL(
		OCSP::TStatus& aOCSPStatus, COCSPResponse& aResponse,
		const TTime aValidationTime, TRequestStatus& aStatus,
		const COCSPRequest& aRequest);
	virtual void CancelValidate();

	TBool DoValidateL(
		const COCSPRequest& aRequest, COCSPResponse& aResponse);
	TBool CertChainMatchesCertL(
		const TDesC8& aCertChainData, const CX509Certificate& aCert);
	};

// Authorisation scheme taking a cert store UID, and allowing those cert chains
// with a root cert in the cert store valid for that UID
	
	/**
	Implement part of S2.2 of RFC 2560.
	
	"The key used to sign the response MUST belong to one of the following...

	-- a Trusted Responder whose public key is trusted by the requester"
*/
class COCSPDirectAuthorisationScheme : public CActive, public MOCSPAuthorisationScheme
	{
public:
	IMPORT_C static COCSPDirectAuthorisationScheme* NewLC(const TUid& aCertStoreUid, MCertStore& aCertStore);

	~COCSPDirectAuthorisationScheme();

	// From MOCSPAuthorisationScheme
	void ValidateL(OCSP::TStatus& aOCSPStatus, COCSPResponse& aResponse, 
		const TTime aValidationTime, TRequestStatus& aStatus,
		const COCSPRequest& aRequest);
	void CancelValidate();
	
	const CX509Certificate* ResponderCert() const;
	
protected:
	virtual void DoCancel();
	virtual void RunL();
	virtual TInt RunError(TInt aError);
	
private:
	void ConstructL();
	
	COCSPDirectAuthorisationScheme(const TUid& aCertStoreUid, MCertStore& aCertStore);

	TBool ValidateSignatureL();
	void ValidateFromRoots();
	void ValidateCertChainL(); 
	void StartValidateL();
	void OnValidateCertChainL();
	void OnListCertEntries();
	void OnRetrieveNextL();
	void OnRetrievingEntryL();

private:
	TUid iCertStoreUid;

	COCSPResponse* iResponse;

	CPKIXCertChainBase* iCertChain;

	TRequestStatus* iClientStatus;
	CPKIXValidationResultBase* iPKIXResult;
	OCSP::TStatus* iOCSPStatus;

	enum TStatus
		{
		EValidateCertChain, // waiting for cert chain validation to complete
		EListCertEntries,   // waiting listing to complete
		ERetrieveNext,      // get next cert entry
		ERetrievingEntry,   // waiting for entry retrieval to complete
		};
	TStatus iState;

	MCertStore& iCertStore;
	
	CCertAttributeFilter* iCertFilter;
	RMPointerArray<CCTCertInfo> iCertStoreEntries;
	HBufC8* iEncodedCertBuf;
	TPtr8* iEncodedCertPtr;

	RFs iFs;
	TInt iCurEntry; // while async processing 509 certs, points to the current one

	TTime iValidationTime;
	const COCSPRequest* iRequest;
	const CX509Certificate* iResponseCert;
	};


/**
 * Interface through which the OCSP module sends requests to the outside world.
 * Normally, the appropriate concrete objects are made automatically, depending
 * on the URI, and so clients don't have to deal with this.  However, clients
 * can override the automatically created object by specifying their own
 * transport object instead.
 */

class MOCSPTransport
	{
public:

	virtual ~MOCSPTransport() {};

	/**
	 * Send request data to a URI. This is an asynchronous method.
	 * @param aURI			Target URI for sending the request
	 * @param aRequest		Request packet
	 * @param aTimeout		Request timeout in milliseconds (ignored if feature not supported)
	 * @param aStatus		The request status object. On request completion, indicates the status
	 */
	virtual void SendRequest(const TDesC8& aURI,
							 const TDesC8& aRequest,
							 const TInt aTimeout,
							 TRequestStatus& aStatus) = 0;

	/**
	 * Cancel the request.
	 */
	
	virtual void CancelRequest() = 0;

	// The following methods will only be called after the aStatus above is completed

	/**
	 * Get the response data.  Panic if completion gave an error.
	 */
	
	virtual TPtrC8 GetResponse() const = 0;	
	};


/**
 * Specifies paremeters needed to perform revocation checking.  The important
 * things to set are the certificates to be checked all - all the other
 * parameters have defaults.
 */

class COCSPParameters : public CBase
	{
public:
	
	/**
	 * Create a new parameters object.
	 */

	IMPORT_C static COCSPParameters* NewL();
	IMPORT_C static COCSPParameters* NewLC();
	
	/**
	 * Add a certificate chain to be checked.  Does not take ownership -
	 * certificates must not be destroyed until the OCSP check complete.
	 */

	IMPORT_C void AddCertificatesL(const CX509CertChain& aChain);

	/**
	 * Add a certificate to be checked.  Does not take ownership - certificates
	 * must not be destroyed until the OCSP check complete.
	 */

	IMPORT_C void AddCertificateL(const CX509Certificate& aSubject, const CX509Certificate& aIssuer);

	/**
	 * Set whether or not to use a nonce.  By default this is turned on.
	 */

	IMPORT_C void SetUseNonce(TBool aUseNonce);

	/**
	 * Set the URI of the OCSP responder to use.  The descriptor is copied.
	 *
	 * @param aURI The URI of the responder to use.
	 * @param aUseAIA Indicates whether to use the responder specified in
	 * certifcate AuthorityInfoAccess extensions in preference to the one given
	 * here.
	 */

	IMPORT_C void SetURIL(const TDesC8& aURI, TBool aUseAIA);

	/**
	 * Set the transport to use.  This method must be called - if no transport
	 * is set COCSPClient::NewL will leave with KErrArgument.  This object takes
	 * ownership.
	 */

	IMPORT_C void SetTransport(MOCSPTransport* aTransport);

	/**
	 * Set the retry count for failed send request attempts.
	 * A default value of 1 (no retry) is used.
	 * 
	 * @param aRetryCount		Retry count
	 */
	IMPORT_C void SetRetryCount(const TUint aRetryCount);

	/**
	 * Set the request timeout.
	 * A default value of -1 (default transport timeout) is used.
	 * 
	 * @param aTimeout			Request timeout in milliseconds
	 */
	IMPORT_C void SetTimeout(const TInt aTimeout);

	/**
	 * Add an authorisation scheme - takes ownership if it doesn't leave.
	 * Clients must specify at least one such scheme, used to validate the signature
	 * on the response.
	 */

	IMPORT_C void AddAuthorisationSchemeL(MOCSPAuthorisationScheme* aScheme);

	IMPORT_C void AddAllAuthorisationSchemesL(const TUid& aCertStoreUid, MCertStore& aCertStore);

	/**
	 * Specify the authorisation time (optional - uses producedAt time from response otherwise).
	 */

	IMPORT_C void SetValidationTimeL(const TTime& aValidationTime);

	/**
	 * Specify the maximum age of the thisUpdate field in seconds (zero => don't check).
	 */

	IMPORT_C void SetMaxStatusAgeL(TUint aMaxAge);

	/**
	 * Specify how much leeway we allow when comparing times.
	 */

	IMPORT_C void SetTimeLeewayL(TUint aLeewaySeconds);
	
	IMPORT_C void SetOCSPCheckForResponderCert(const TBool aResponderCertCheck);
	
	IMPORT_C void SetCheckCertsWithAiaOnly(const TBool aCheckCertsWithAiaOnly);
	
	IMPORT_C void SetUseAIA(const TBool aUseAIA);
	
public:
	
	// Not exported

	~COCSPParameters();
	TUint CertCount() const;
	const CX509Certificate& SubjectCert(TUint aIndex) const;
	const CX509Certificate& IssuerCert(TUint aIndex) const;
	TBool UseNonce() const;
	const TDesC8& DefaultURI() const;
	TBool UseAIA() const;
	MOCSPTransport* Transport() const;
	TUint AuthSchemeCount() const;
	MOCSPAuthorisationScheme& AuthScheme(TUint aIndex) const;
	const TTime* ValidationTime() const;
	const TUint* MaxStatusAge() const;
	const TUint* TimeLeeway() const;
	TBool GenerateResponseForMissingUri() const;

	TUint RetryCount() const;
	TInt Timeout() const;
	TBool ReponderCertCheck() const;
	const CX509Certificate& CACert(TUint aIndex) const;
	TBool CheckCertsWithAiaOnly() const;
	
private:
	void ConstructL();
	COCSPParameters();
	
#ifdef _DEBUG
	/** Panic codes used by this functions in this class. */
	enum TPanic
	{
		/** AddAllAuthorisationSchemesL() called when already added at least one. */
		EAAASAlreadyHaveSchemes = 0x10
	};
	static void Panic(TPanic aPanic);
#endif
private:

	RPointerArray<CX509Certificate>			iSubjectCerts;
	RPointerArray<CX509Certificate>			iIssuerCerts;	
	TBool									iUseNonce;
	HBufC8*									iDefaultURI;
	MOCSPTransport*							iTransport;
	TBool									iUseAIA;
	RPointerArray<MOCSPAuthorisationScheme> iAuthSchemes;
	TTime*									iValidationTime;
	TUint*									iMaxStatusAge;
	TUint*									iTimeLeeway;
	TBool									iGenerateResponseForMissingUri;
	
	TBool									iResponderCertCheck;
	TUint									iRetryCount;
	TInt									iTimeout;
	TBool									iCheckCertsWithAiaOnly;
	};


/**
 * Checks the revocation state of one or more certificates.
 *
 * The current implementation makes one OCSP request for every certificate
 * checked - future impelementations may check multiple certificates with a
 * single request.
 */

class COCSPClient : public CActive
	{
public:

	/**
	 * Create a new OCSP client.
	 * 
	 * @param aParams An object describing the parameters for the check.  This
	 * method takes ownership if it does not leave.
	 */

	IMPORT_C static COCSPClient* NewL(const COCSPParameters* aParams);
	
	~COCSPClient();

	/**
	 * Start the checker - this is an asynchronous method.
	 */

	IMPORT_C void Check(TRequestStatus& aStatus);

	/**
	 * Cancel checking.
	 */

	IMPORT_C void CancelCheck(void);

	/**
	 * Get the summary result of the check.  If any certificates were revoked,
	 * this returns ERevoked.  If no certificates were revoked, but if there
	 * were any errors communicating with ocsp servers or any certs had unknown
	 * status, this returns EUnknown.  Otherwise it returns EGood. 
	 * Panics if the check has not been run, or is not complete.
	 */

	IMPORT_C OCSP::TResult SummaryResult(void) const;

	/**
	 * Get the number of transactions made.
	 * Panics if the check has not been run, or is not complete.
	 */

	IMPORT_C TInt TransactionCount(void) const;

	/** 
	 * Get the request object for a specified transaction.
	 * Panics if the check has not been run, is not complete, or the index is invalid.
	 */

	IMPORT_C const COCSPRequest& Request(TInt aIndex) const;

	/**
	 * Get the outcome for an individual transaction.
	 */

	IMPORT_C const TOCSPOutcome& Outcome(TInt aIndex) const;

	/** 
	 * Get the response object for a specified transaction.  If there was an
	 * error sending the request, this may return NULL for the corresponding
	 * response.
	 * Panics if the check has not been run, is not complete, or the index is invalid.
	 */

	IMPORT_C const COCSPResponse* Response(TInt aIndex) const;
	
	IMPORT_C TBool CertsAvailableForOCSPCheck();
	
protected:
	virtual void RunL();
	virtual TInt RunError(TInt aErr);
	virtual void DoCancel();
	
private:
	
	enum TState
		{
		EInitial,
		ESendingRequest,		// Request is being sent to server
		EValidatingResponse,	// Request is being validated
		EHaveResult,			// Validation complete
		EError,					// We got an error
		};
		
	COCSPClient();
	void ConstructL(const COCSPParameters* aParams);
	void Destroy();
	void DoCheck();
	void SendRequest();
	void DoSendRequestL();
	void ValidateResponseL();
	void HandleResponseReceivedL();
	void HandleResponseValidatedL();
	void HandleTransactionErrorL(OCSP::TStatus aStatus);
	
	TState							iState;	
	TRequestStatus*					iClientStatus;
	const COCSPParameters*			iParams;
	const TDesC8*					iURI;
	MOCSPTransport*					iTransport;
	COCSPTransaction*				iTransaction;
	RPointerArray<COCSPRequest>		iRequests;
	RPointerArray<COCSPResponse>	iResponses;
	COCSPValidator*					iValidator;
	RArray<TOCSPOutcome>			iOutcomes;
	OCSP::TResult					iSummaryResult;
	
	};


#endif // OCSPREQUESTANDRESPONSE_H
