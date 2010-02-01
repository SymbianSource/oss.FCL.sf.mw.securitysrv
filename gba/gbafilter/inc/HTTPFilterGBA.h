/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies). 
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  GBA Filter definitions
*
*/


#ifndef __HTTPFILTERGBA_H_
#define __HTTPFILTERGBA_H_

#include <http/mhttpfilter.h>
#include <http/rhttpsession.h>
#include <http/cecomfilter.h>
#include <http/framework/httplogger.h>

#include <GbaUtility.h>

class RHTTPResponse;
class RHTTPTransaction;
class CMD5;

const TInt KMaxTypeLength = 255;
const TInt KBsfAddressSize = 128;


class CHTTPFilterGBA : public CEComFilter, public MHTTPFilter, public MGbaObserver
{
    enum
        {
        EAlgUnknown,
        EAlgMd5,
        EAlgMd5Sess
        };

    enum
        {
        EQopNone,
        EQopAuth = 1,
        EQopAuthInt = 2
        };
public:

	static CEComFilter* InstallFilterL(TAny* aSession);
 
	virtual ~CHTTPFilterGBA();

public:	// Methods from MHTTPFilterBase

	/// @see MHTTPFilterBase::MHFRunL
	virtual void MHFRunL(RHTTPTransaction aTransaction, const THTTPEvent& aEvent);

	/// @see MHTTPFilterBase::MHFSessionRunL
	virtual void MHFSessionRunL(const THTTPSessionEvent& aEvent);

	/// @see MHTTPFilterBase::MHFRunError
	virtual TInt MHFRunError(TInt aError, RHTTPTransaction aTransaction, const THTTPEvent& aEvent);

	/// @see MHTTPFilterBase::MHFSessionRunError
	virtual TInt MHFSessionRunError(TInt aError, const THTTPSessionEvent& aEvent);

public:	// Methods from MHTTPFilter

	/// @see MHTTPFilter::MHFUnload
	virtual void MHFUnload(RHTTPSession aSession, THTTPFilterHandle aHandle);

	/// @see MHTTPFilter::MHFLoad
	virtual void MHFLoad(RHTTPSession aSession, THTTPFilterHandle aHandle);
	
	inline void SetProtocolIdentifierL( const TDesC8& aIdentifier){
	    iProtocolIdentifier = aIdentifier;
	}
	
	inline void SetGBAFlags(const TUint8 aFlags){
	    iFlags = aFlags;
	}
	
	inline void SetGBALabel(const TDesC8& aLabel){
	    iUICCLabel =  aLabel ;
	}

    //Only for testing purpose
	//To force BSF address for use
	inline void SetBSFAddress(const TDesC8& aBsfAddress){
	        iBsfAddress = aBsfAddress;
	};

 private:
	
	CHTTPFilterGBA();

	///ContructL also Installs the filter.
	void ConstructL( RHTTPSession aSession );
	
#if defined (_DEBUG)
	void DumpResponseHeadersL( RHTTPResponse& aResponse );
#endif

	void CheckHeadersL( RHTTPTransaction& aTrans );
	
	void ProcessBodyPartL( RHTTPTransaction& aTrans );
	
	void Cleanup( const RHTTPTransaction& aTrans );
	
	void CleanupAll();
	
    TInt FindHeaderPartToUseL(RHTTPTransaction aTransaction) const;
    
	TBool FindAuth(const TDesC8& aQop) const;
	
	void DoSubmitL( RHTTPTransaction aTransaction );
	
	void EncodeDigestAuthL( TInt aCred, RHTTPTransaction aTransaction );
	
    void DAddCredentialsToListL( 	RString aUsernameStr,
    								RString aPasswordStr,
    								RString aRealmStr,
    								RStringF aUriStr,
    								RString aOpaque,
    								RString aNonce,
    								TInt aQop,
    								TInt aAuthAlg );
   /**
    * Calculate H(A1). See section 3.2.2.2
    */
    void HAOneL( int aAlgorithm,
               const RString& aUsername,
               const RString& aPW,
		       const RString& aRealm,
               const RString& aNonceValue,
               TDesC8& aCNonce,
               TDes8& aResult);

    
    /**
    * Calculate H(A2). See section 3.2.2.3
    */
    void HATwoL(const RStringF& aMethod, const RString& aRequestUri, TDes8& aResult);

    /**
    * Calculate a hash and in the form of a 32 character hex result.
    */
    void Hash(const TDesC8& aMessage, TDes8& aHash);

    /**
    * Generate a cnonce value
    */	
    void GenerateCNonce( TDes8& aNonce );

    /**
    * Returns the requested uri 
    * that can be used in uri field or request header
    *
    * @param aTransaction current transaction
    * @return requested URI
    */
    RString RequestUriL( RHTTPTransaction& aTransaction);
                   
    /**
    * Check and return QoP from the response header.
    * @param aHeader authentication header
    * @param aWwwAuthHeader authentication header name
    * @param aHeaderPart which authentication header field in the response header
    * @return one of KQopXXX enum. KQopAuth is prefered.
    */    
    TInt CheckQop( RHTTPHeaders& headers,
                   RStringF& aWwwAuthHeader,
                   TInt aHeaderPart );
 
 	void DGetCredentialsFromPropertiesL( RHTTPTransaction& aTransaction );
 	
 	TInt DFindCredentialsForURI( const TUriC8& aURI );
 	
 	void DRemoveCredentialsFromList( TInt aCredId );
 	
 	TInt DFindCredentials( const RString& aRealm, const TUriC8& aURI );
 	
 	//Inherited from MGbaObserver
    void BootstrapComplete(TInt error);
 	
     class TDCredentials
        {
        public:
            RStringToken iUser;  //username
            RStringToken iPassword; //password
            RStringTokenF iURI;  //uri
            RStringToken iRealm; //Realm used - there may be multiple for each host
            RStringToken iOpaque;
            RStringToken iNonce;
            TInt iQop;
            TInt iAlgorithm;
            TInt iNc;   // nonce-count
        };

	/// A count to make sure we delete at the right time.
	TInt        iLoadCount;
	RStringPool iStringPool;
    RStringF    iOpaqueStr;
    RStringF    iNonceStr;
	RStringF    iQopStr;
	RStringF 	iStaleStr;
	RStringF    iAuthInfo;
	RStringF 	iUsernameStr;
	RStringF 	iPasswordStr;
    RStringF    iQopAuthStr;
    RStringF    iMd5Str;
    RStringF    iMd5SessStr;
    RStringF    iRealmStr;
	RStringF 	iUserAgent;
    TInt64      iSeed;
    CMD5*       iMD5Calculator;
	TInt		iOffset;
	TUint8      iFlags;
	TBuf8<5>    iProtocolIdentifier;
	TInt        iBootstrapCount;
	TBool       iBootstrapPending;
	TBool       iHaveGbaBootstrapCredentials;
	TBool       iGBAU;
	TBuf8<KMaxTypeLength>     iUICCLabel;
	CActiveSchedulerWait      iBootstrapWait;
	CGbaUtility*              iGbaUtility;
    TGBABootstrapInputParams  iGbaInputParams;
    TGBABootstrapOutputParams iGbaOutputParams;
    //Only for testing  purpose
    TBuf8<KBsfAddressSize>    iBsfAddress;
    // Internal list of valid credentials
    RArray<TDCredentials>     iDCredentials;

};

#endif //__HTTPFILTERGBA_H_

//EOF
