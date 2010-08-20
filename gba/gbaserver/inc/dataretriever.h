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
* Description:  This file contains class CDataRetriever. It encapsulates
*                http connection to the bsf  
*
*/


#ifndef DATARETRIEVER_H
#define DATARETRIEVER_H

#include <e32base.h>

#include <stringpool.h>
#include <http.h>
#include <es_sock.h> 
#include <commdbconnpref.h>
#include <cmmanager.h>
#include <hash.h>
#include "M3GPPAuthenticationCallback.h"


// HTTPHeader data lenghts
const TInt KMaxHeaderNameLen = 32;
const TInt KMaxHeaderValueLen = 128;
const TInt KMaxBootstrapRespLen = 1024;


// forward declarations
class MBootstrapCallBack;

class MHttpRequestEvents
    {
    public:
        virtual void EventRequestCompletedL(TInt aErrorCode) = 0;
    };


/** 
 * Handles all events for the active transaction.
 */
class C3GPPBootstrapHttpHandler : public CBase, public MHTTPTransactionCallback
    {
    public:
            
        static C3GPPBootstrapHttpHandler* NewLC(    MHttpRequestEvents* aEventSink,
                                                    M3GPPAuthenticationCallback* aDataRetrieverCallBack );
        static C3GPPBootstrapHttpHandler* NewL( MHttpRequestEvents* aEventSink,
                                                M3GPPAuthenticationCallback* aDataRetrieverCallBack );
        virtual ~C3GPPBootstrapHttpHandler();
        //methods from MHTTPTransactionCallback
        virtual void MHFRunL(RHTTPTransaction aTransaction, const THTTPEvent& aEvent);
        virtual TInt MHFRunError(TInt aError, RHTTPTransaction aTransaction, const THTTPEvent& aEvent);
        HBufC8* GetResponse();  // intended to contain the TID!
           
        void Reset();
    private:
        C3GPPBootstrapHttpHandler(  MHttpRequestEvents* aEventSink,
                                    M3GPPAuthenticationCallback* aDataRetrieverCallBack );
        void ConstructL();
        // handles HTTP 401 Authorization required
        TInt HandleAuthorizationChallengeL(RHTTPTransaction aTransaction, const THTTPEvent& aEvent);
        // HTTP DIGEST authentication stuff
        TInt FindHeaderPartToUseL(RHTTPTransaction aTransaction) const;
        void EncodeDigestAuthL( const RString& aUsername, const RString& aPW,
                                RHTTPTransaction& aTransaction, TInt headerPart, const THTTPHdrVal& aRequestUri);
        TBool FindAuth(const TDesC8& aQop) const;
        void GenerateCNonce(TDes8& aNonce);
        void Hash(const TDesC8& aMessage, TDes8& aHash);
        void HATwoL(const RStringF& aMethod, const RString& aRequestUri, const TDesC8& aHentity, TDes8& aResult);
        void HAOneL(const RString& aUsername,const RString& aPW, const RString& aRealm, TDes8& aResult);
        void DigestCalcL( const TDesC8& aHentity,TDes8& result );
        TInt CheckRspAuthL( RHTTPTransaction aTransaction );

    private:
        // The digest calculator
        CMD5*               iMD5Calculator;
        // A seed for random numbers, used for client nonces.
        TInt64              iSeed;
        MHttpRequestEvents* iEventSink;
        MHTTPDataSupplier*  iRespBody;
        HBufC8*             iResponse;
        TBool               iSucceeded;
        RString             iUsername;
        RString             iPassword;
        RString             iNonce;
        RString             iCnonce;
        RString             iUri; 
        RString             iRealm;
        RStringF            iMethod;
        M3GPPAuthenticationCallback* iDataRetrieverCallBack;
    };



class CDataRetriever : public CBase, public MHttpRequestEvents
    {
    public:
        static CDataRetriever* NewLC( MBootstrapCallBack* aBootstrapCallBack );
        static CDataRetriever* NewL( MBootstrapCallBack* aBootstrapCallBack );
        ~CDataRetriever();
        void MakeRequestL( TRequestStatus* aRequestStatus,
                           const TDesC8& aIdentity,
                           const TDesC8& aRealm,
                           const TDesC8& aBsfUri,
                           const TDesC8& aReferrerNAFUri,
                           const TInt& aIAPID ); 
                           
        void CancelRequest();
        HBufC8 * QueryResponseValueL();
        // event from httphandler when request has been completed, 
        //aErrorCode is standard symbian error codes
        void EventRequestCompletedL(TInt aErrorCode);
  
    private:
        void SetHeaderL(RHTTPHeaders aHeaders, TInt aHdrField, const TDesC8& aHdrValue);
        void SetupSessionL( const TInt& aIAPID );
        TUint32 GetInternetDestIDL();
        CDataRetriever( MBootstrapCallBack* aBootstrapCallBack );
        void ConstructL();
        TBool ValidateGivenIAPIDL( const TInt& aIAPID );
    
    private:
        enum TInternalState
            {
            EReadyForRequest,   //  Ready and waiting for MakeRequest call
            EMakeRequestCalled, //  Request function was called, now waiting for http request to complete
            };
            
    private:
          TInternalState            iInternalState;
          TRequestStatus*           iCallerRequestStatus;   // status of the caller
          RHTTPSession              iHTTPSession;
          RHTTPTransaction          iHTTPTransaction;
          RConnection               iConnection;
          RSocketServ               iSockServ;
          C3GPPBootstrapHttpHandler* iHttpHandler;          // we own this    
          MBootstrapCallBack*       iBootstrapCallBack;     // don't own this
          RCmManager             	iCmManager;
    };



#endif // DATARETRIEVER_H

//EOF
