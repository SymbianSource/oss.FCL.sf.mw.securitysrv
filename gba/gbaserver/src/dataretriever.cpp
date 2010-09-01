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
* Description:  Implementation of bootstrap and data retriever
*
*/


#include <e32math.h> 
#include <hash.h>
#include <commdb.h>
#include <es_enum.h>
#include <centralrepository.h>
#include <cmconnectionmethodext.h>
#include <cmconnectionmethoddef.h>
#include <cmpluginpacketdatadef.h>
#include <cmdestinationext.h>
#include <http/thttpevent.h>
#include "dataretriever.h"
#include "GbaCommon.h"
#include "GBALogger.h"
#include "bootstrap.h"	 

_LIT8(KUserAgent,"S60 GBA 1.0");
_LIT8(KAccept,"*/*");
_LIT8(KNCVal_2, ":00000001:");
_LIT8(Kfopaque,"");
_LIT8(Kfalgo,"AKAv1-MD5");
_LIT8(Kalgorithm,"algorithm");
_LIT8(formatStr, "%02x");
_LIT8(KAuthInt,"auth-int");
_LIT8(KAuts,"auts");
_LIT8(KHTTPAuthInfo,"Authentication-Info");
_LIT8(KMD5HashofEmpty,"d41d8cd98f00b204e9800998ecf8427e");
_LIT8(KClose, "Close");
_LIT(KColon, ":");


//Constants
const TInt KNumber3 = 3;
const TInt KNumber5 = 5;
const TInt KNumber8 = 8;
const TInt KNumber32 = 32;
const TInt KNumber33 = 33;
const TInt KNumber34 = 34;
// Length of a digest hash when represented in hex
const TInt KHashLength = 32;
// Length of a digest hash before converting to hex.
const TInt KRawHashLength = 16;
const TInt KMaxRequestBuffer = 255;
const TInt KAlgLength = 40;

// ---------------------------------------------------------------------------
// CDataRetriever::NewL()
// ---------------------------------------------------------------------------
//
CDataRetriever* CDataRetriever::NewL( MBootstrapCallBack* aBootstrapCallBack )
    {
    CDataRetriever* self = NewLC( aBootstrapCallBack );
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// CDataRetriever::NewLC()
// ---------------------------------------------------------------------------
//
CDataRetriever* CDataRetriever::NewLC( MBootstrapCallBack* aBootstrapCallBack )
    {
    CDataRetriever* self = new (ELeave) CDataRetriever( aBootstrapCallBack ) ;
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }


// ---------------------------------------------------------------------------
// CDataRetriever::CDataRetriever()
// ---------------------------------------------------------------------------
//
CDataRetriever::CDataRetriever( MBootstrapCallBack* aBootstrapCallBack ) 
    : iBootstrapCallBack( aBootstrapCallBack ) 
    { 
    }
    
    
// ---------------------------------------------------------------------------
// CDataRetriever::ConstructL()
// ---------------------------------------------------------------------------
//
void CDataRetriever::ConstructL()
    {
    iHttpHandler = C3GPPBootstrapHttpHandler::NewL( this, iBootstrapCallBack );
    iInternalState = EReadyForRequest;
    iCmManagerExt.OpenL();
    }


// ---------------------------------------------------------------------------
// CDataRetriever::~CDataRetriever()
// ---------------------------------------------------------------------------
//
CDataRetriever::~CDataRetriever()
    {
    GBA_TRACE_DEBUG(("CDataRetriever::~CDataRetriever"));
    
    delete iHttpHandler;
    iHTTPTransaction.Close();
    iHTTPSession.Close();
    iConnection.Close();
    iSockServ.Close();
    iCmManagerExt.Close();
    
    if ( iInternalState == EMakeRequestCalled )
        {
        GBA_TRACE_DEBUG(("CDataRetriever::~CDataRetriever EMakeRequestCalled"));
        User::RequestComplete(iCallerRequestStatus, KErrCancel);
        }
    
    GBA_TRACE_DEBUG(("CDataRetriever::~CDataRetriever End"));
    }

// ---------------------------------------------------------------------------
// CDataRetriever::EventRequestCompletedL()
// ---------------------------------------------------------------------------
//
void CDataRetriever::EventRequestCompletedL(TInt aErrorCode)
    {
    GBA_TRACE_DEBUG(("CDataRetriever::EventRequestCompletedL enter"));
    
    if ( iInternalState == EMakeRequestCalled )
        {
        GBA_TRACE_DEBUG(("CDataRetriever::EventRequestCompletedL EMakeRequestCalled"));

        TRAPD( err, iBootstrapCallBack->CompleteBootStrappingL( aErrorCode ) );
        if( err == KErrNone )
            User::RequestComplete(iCallerRequestStatus,aErrorCode);
        else
            User::RequestComplete(iCallerRequestStatus, err);
        // Get Ready for new request
        iHttpHandler->Reset();
        iHTTPTransaction.Close();
        iHTTPSession.Close();
        iConnection.Close();
        iSockServ.Close();
        iInternalState = EReadyForRequest;
        }
    }


// ---------------------------------------------------------------------------
// CDataRetriever::CancelRequest()
// ---------------------------------------------------------------------------
//
void CDataRetriever::CancelRequest()
    {
    GBA_TRACE_DEBUG(("CDataRetriever::CancelRequest"));
    if ( iInternalState == EMakeRequestCalled )
        {        
        iInternalState = EReadyForRequest;
        iHttpHandler->Reset();
        iHTTPTransaction.Close();
        iHTTPSession.Close();
        iConnection.Close();
        iSockServ.Close();
        User::RequestComplete( iCallerRequestStatus, KErrCancel );
        }
    }
    
    
// ---------------------------------------------------------------------------
// CDataRetriever::MakeRequestL()
// ---------------------------------------------------------------------------
//
void CDataRetriever::MakeRequestL(
                 TRequestStatus* aRequestStatus,
                 const TDesC8& aIdentity,
                 const TDesC8& aRealm,
                 const TDesC8& aBsfUri,
                 const TDesC8& aReferrerNAFUri,
                 const TInt& aIAPID )
    {
    GBA_TRACE_BEGIN(); 
    
    if ( iInternalState != EReadyForRequest )
        {        
        User::LeaveIfError( KErrInUse );
        }
    
    iCallerRequestStatus = aRequestStatus;
  
    SetupSessionL( aIAPID );

    // Add headers appropriate to all methods
    TUriParser8 URIParse; 

    User::LeaveIfError( URIParse.Parse( aBsfUri ) );

    iHTTPTransaction = iHTTPSession.OpenTransactionL( URIParse, *iHttpHandler );
    RHTTPHeaders hdr = iHTTPTransaction.Request().GetHeaderCollection();
    
    GBA_TRACE_DEBUG(URIParse.Extract(EUriHost) );
         
    SetHeaderL(hdr,HTTP::EHost, URIParse.Extract(EUriHost) );     
    SetHeaderL(hdr,HTTP::EUserAgent, KUserAgent);
    SetHeaderL(hdr,HTTP::EAccept, KAccept);     
    SetHeaderL(hdr,HTTP::EReferer, aReferrerNAFUri);
    
    SetHeaderL(hdr, HTTP::EConnection, KClose);
  
    TBuf8<KMaxRequestBuffer> bufAuth;
    bufAuth.Copy(_L8("Digest username=\""));
    bufAuth.Append(aIdentity);
    bufAuth.Append(_L8("\", realm=\""));
    
    bufAuth.Append(aRealm);
    bufAuth.Append(_L8("\", nonce=\"\", uri=\"/\", response=\"\""));

    GBA_TRACE_DEBUG(bufAuth);
    SetHeaderL(hdr,HTTP::EAuthorization,bufAuth);
    
    iHTTPTransaction.SubmitL();    
    iInternalState = EMakeRequestCalled;
    GBA_TRACE_END();
    }

// ---------------------------------------------------------------------------
// CDataRetriever::SetHeaderL()
// ---------------------------------------------------------------------------
//
void CDataRetriever::SetHeaderL(RHTTPHeaders aHeaders, TInt aHdrField, const TDesC8& aHdrValue)
    {
    RStringF valStr = iHTTPSession.StringPool().OpenFStringL(aHdrValue);
    THTTPHdrVal val(valStr);
    aHeaders.SetFieldL(iHTTPSession.StringPool().StringF(aHdrField,RHTTPSession::GetTable()), val);
    valStr.Close();
    }

// ---------------------------------------------------------------------------
// CDataRetriever::SetupSessionL()
// Establish a connection for bootstrapping
// ---------------------------------------------------------------------------
//
void CDataRetriever::SetupSessionL( const TInt& aIAPID )
    {
    GBA_TRACE_DEBUG(("CDataRetriever::SetupSession"));
    // check that sockserver is available

    User::LeaveIfError(iSockServ.Connect() );
    GBA_TRACE_DEBUG(("Socket server connected")); 

    // check that connection is possible
    User::LeaveIfError(iConnection.Open(iSockServ) );
    GBA_TRACE_DEBUG(("Connection opened")); 
    
    if ( !ValidateGivenIAPIDL( aIAPID ) )
        {	
        GBA_TRACE_DEBUG(("No valid iapid from user")); 
        TConnSnapPref snapPref;
        TUint32 snapid = GetInternetDestIDL();
        GBA_TRACE_DEBUG_NUM(("the chosen snap id is %d"), snapid ); 
        snapPref.SetSnap( snapid );
        TInt err = iConnection.Start( snapPref );
        
        if ( err != KErrNone )
            {
            GBA_TRACE_DEBUG_NUM(("internet nap error is %d"), err ); 
            TCommDbConnPref connPref;
            connPref.SetDialogPreference( ECommDbDialogPrefPrompt );
            User::LeaveIfError( iConnection.Start( connPref ) );
            }    
       }
    else
        {
        GBA_TRACE_DEBUG(("User gives valid iapid go with it"));
        //given iapid is valid, go with it
        TCommDbConnPref connPref;
        connPref.SetDirection( ECommDbConnectionDirectionOutgoing );
        
        //No dialog present
        connPref.SetDialogPreference( ECommDbDialogPrefDoNotPrompt );
        connPref.SetIapId( aIAPID );	
         
        User::LeaveIfError( iConnection.Start( connPref ) );
        }
    
    GBA_TRACE_DEBUG(("Access point selection done!"));       

    GBA_TRACE_DEBUG(("CDataRetriever::SetupSession, connect choosen is done "));
    
    iHTTPSession.OpenL();
	    
    //GBA Http  filter gets loaded into to http session when session is opened
	//Remove GBA http filter from http session iHTTPSession
    //Reason: Ub interface should not have the User-Agent string "3gpp-gba"
	RStringF filterName = iHTTPSession.StringPool().OpenFStringL( KHTTPFilterGBAName );
    CleanupClosePushL<RStringF>( filterName );
    iHTTPSession.FilterCollection().RemoveFilter( filterName );
    CleanupStack::PopAndDestroy( &filterName );
	
    GBA_TRACE_DEBUG(("Session opened"));  
    
    //Set selected access point.
    RStringPool strPool = iHTTPSession.StringPool();
    RHTTPConnectionInfo httpconnInfo = iHTTPSession.ConnectionInfo();
    
    httpconnInfo.SetPropertyL ( strPool.StringF(HTTP::EHttpSocketServ,
            RHTTPSession::GetTable() ), THTTPHdrVal (iSockServ.Handle()) );
    
    TInt connPtr = REINTERPRET_CAST(TInt, &iConnection);

    httpconnInfo.SetPropertyL ( strPool.StringF(HTTP::EHttpSocketConnection,
              RHTTPSession::GetTable() ), THTTPHdrVal (connPtr) );
    
    //if 2G interface and cert is untrusted, surpress the warning dialog
    if ( iBootstrapCallBack->InterfaceIs2G() )
        {
        //surpress the warning dialog
        httpconnInfo.SetPropertyL ( strPool.StringF(HTTP::ESecureDialog,
              RHTTPSession::GetTable() ),strPool.StringF(HTTP::EDialogNoPrompt,
              RHTTPSession::GetTable() )  );
        }
  
    GBA_TRACE_DEBUG(("SetupSession done")); 
    }


// ---------------------------------------------------------------------------
// CDataRetriever::ValidateGivenIAPIDL()
// ---------------------------------------------------------------------------
//
TBool CDataRetriever::ValidateGivenIAPIDL( const TInt& aIAPID )
    {
    using namespace CMManager;
    GBA_TRACE_DEBUG((" start valid iapid given by user"));
    //aIAPID == -1 selected default accesspoint
    if ( aIAPID == -1 )
        {
        return EFalse;
        }
    else
        {
        TInt CleanupCounter = 0;
   
        RArray<TUint32> destIdArray;
        CleanupClosePushL( destIdArray );
        CleanupCounter++;
        iCmManagerExt.AllDestinationsL( destIdArray );

        for ( TInt i = 0; i< destIdArray.Count(); i++ )
           {
           RCmDestinationExt dest = iCmManagerExt.DestinationL( destIdArray[i] );
           CleanupClosePushL( dest );
           CleanupCounter++;
         
           for ( TInt j=0; j < dest.ConnectionMethodCount(); j++ )
              {
           
              TUint32 iapId = dest. ConnectionMethodL(j).GetIntAttributeL( ECmIapId );
              
              if ( iapId == aIAPID )
                  {
                  GBA_TRACE_DEBUG((" IAPID matches !!"));
                  CleanupStack::PopAndDestroy( CleanupCounter );
                  return ETrue;
                  }
              }
             
           CleanupStack::PopAndDestroy( &dest ); //dest
           CleanupCounter--; // dest
           }
        GBA_TRACE_DEBUG((" search is over no matching IAPID!!"));
        CleanupStack::PopAndDestroy( CleanupCounter );
        return EFalse;    
        }    
    }

// ---------------------------------------------------------------------------
// CDataRetriever::QueryResponseValueL()
// ---------------------------------------------------------------------------
//
HBufC8* CDataRetriever::QueryResponseValueL() 
    {
    return iHttpHandler->GetResponse();
    } 

// ---------------------------------------------------------------------------
// CDataRetriever::GetInternetDestIDL()
// ---------------------------------------------------------------------------
//
TUint32 CDataRetriever::GetInternetDestIDL()
    {
    using namespace CMManager;
    GBA_TRACE_DEBUG((" Get the Dest ID of Internet"));

    TInt CleanupCounter = 0;
   
    RArray<TUint32> destIdArray;
    CleanupClosePushL( destIdArray );
    CleanupCounter++;
    iCmManagerExt.AllDestinationsL( destIdArray );

    for ( TInt i = 0; i< destIdArray.Count(); i++ )
         {
         RCmDestinationExt dest = iCmManagerExt.DestinationL( destIdArray[i] );
         CleanupClosePushL( dest );
         CleanupCounter++;
         
         if ( dest.MetadataL( ESnapMetadataInternet ) )
             {
             GBA_TRACE_DEBUG_NUM((" found the internet dest id = %d"), destIdArray[i] );
             
             TUint32 desID = destIdArray[i];
             CleanupStack::PopAndDestroy( CleanupCounter );
             return desID; 
             }
             
         CleanupStack::PopAndDestroy( &dest );
         CleanupCounter--; // dest
         }
    GBA_TRACE_DEBUG(("not found the internet dest id") );
    CleanupStack::PopAndDestroy( CleanupCounter );
    return NULL;
	}
    

// C3GPPBootstrapHttpHandler 
// -----------------------------------------------------------------------------
// C3GPPBootstrapHttpHandler::ConstructL()
// -----------------------------------------------------------------------------
//
void C3GPPBootstrapHttpHandler::ConstructL()
    {
    iResponse=HBufC8::NewL(KMaxBootstrapRespLen);
    iMD5Calculator =  CMD5::NewL(); 
    iSucceeded=EFalse;
    }

// -----------------------------------------------------------------------------
// C3GPPBootstrapHttpHandler::C3GPPBootstrapHttpHandler()
// -----------------------------------------------------------------------------
//
C3GPPBootstrapHttpHandler::C3GPPBootstrapHttpHandler( MHttpRequestEvents* aEventSink, M3GPPAuthenticationCallback* aDataRetrieverCallBack )
        : iEventSink ( aEventSink ), iDataRetrieverCallBack ( aDataRetrieverCallBack )
    {
    }


// -----------------------------------------------------------------------------
// C3GPPBootstrapHttpHandler::~C3GPPBootstrapHttpHandler()
// -----------------------------------------------------------------------------
//
C3GPPBootstrapHttpHandler::~C3GPPBootstrapHttpHandler()
    {
    GBA_TRACE_DEBUG(("C3GPPBootstrapHttpHandler::~C3GPPBootstrapHttpHandler()"));
    Reset();
	delete iResponse;
	delete iMD5Calculator;
    GBA_TRACE_DEBUG(("C3GPPBootstrapHttpHandler::~C3GPPBootstrapHttpHandler() end"));
    }



// -----------------------------------------------------------------------------
// C3GPPBootstrapHttpHandler::Reset()
// -----------------------------------------------------------------------------
//
void C3GPPBootstrapHttpHandler::Reset()
    {
    // close strings in string pool if they were opened 
    iUsername.Close();
    iPassword.Close();
    iNonce.Close();
    iCnonce.Close();
    iUri.Close();
    iRealm.Close();
    iMethod.Close();
    }


// -----------------------------------------------------------------------------
// C3GPPBootstrapHttpHandler::NewLC()
// -----------------------------------------------------------------------------
//
C3GPPBootstrapHttpHandler* C3GPPBootstrapHttpHandler::NewLC( MHttpRequestEvents* aEventSink, 
                                                             M3GPPAuthenticationCallback* aDataRetrieverCallBack )
    {
    C3GPPBootstrapHttpHandler* me = new(ELeave)C3GPPBootstrapHttpHandler( aEventSink, aDataRetrieverCallBack );
    CleanupStack::PushL(me);
    me->ConstructL();
    return me;
    }


// -----------------------------------------------------------------------------
// C3GPPBootstrapHttpHandler::NewL()
// -----------------------------------------------------------------------------
//
C3GPPBootstrapHttpHandler* C3GPPBootstrapHttpHandler::NewL( MHttpRequestEvents* aEventSink ,
                                                            M3GPPAuthenticationCallback* aDataRetrieverCallBack )
    {
    C3GPPBootstrapHttpHandler* me = NewLC(aEventSink, aDataRetrieverCallBack );
    CleanupStack::Pop(me);
    return me;
    }



// -----------------------------------------------------------------------------
// C3GPPBootstrapHttpHandler::MHFRunL()
// -----------------------------------------------------------------------------
//
void C3GPPBootstrapHttpHandler::MHFRunL( RHTTPTransaction aTransaction, const THTTPEvent& aEvent )
    {
    GBA_TRACE_DEBUG_NUM(("MHFRunL received event %d"), aEvent.iStatus );
    
    switch (aEvent.iStatus)
       {
       case THTTPEvent::EGotResponseHeaders:
          {
          // HTTP response headers have been received. 
          // We can determine now if there is going to be a response body to save.
          GBA_TRACE_DEBUG(("THTTPEvent::EGotResponseHeaders"));
          RHTTPResponse resp = aTransaction.Response();
          TInt status = resp.StatusCode();
          
          RStringF statusStr = resp.StatusText();
      
          // 401 = we received a authorization challenge
          if ( status == HTTPStatus::EUnauthorized )
             {
             // resubmit the transaction and hope for the best
             TInt result = HandleAuthorizationChallengeL(aTransaction, aEvent);
             if( result == KErrNone )
                 {
                 aTransaction.Cancel();
                 aTransaction.SubmitL();
                 }
             else
                 {
                 aTransaction.Cancel();
                 iEventSink->EventRequestCompletedL( result );
                 }
             //a new request with password is sent.
             return;
             }
          TInt responseBodySize = 0;
      
          // 200 received, succeeded with body
          if ( resp.HasBody() && ( status == HTTPStatus::EOk ) )
              {
              MHTTPDataSupplier *respbody = resp.Body();
              if ( respbody )
                  {
                  responseBodySize = respbody->OverallDataSize();
                  GBA_TRACE_DEBUG_NUM(("Response body size is %d"), responseBodySize );
                  iSucceeded = ETrue;
                  }
              }
          //Reallocate iResponse buffer size if the response size is greater than KMaxBootstrapRespLen
          if( responseBodySize > KMaxBootstrapRespLen )
              iResponse = iResponse->ReAllocL( responseBodySize );
          // we take this opportunity to reset our own buffers
          TPtr8 ptrResp(iResponse->Des());
          
          ptrResp.Zero();
          } 
          break;
      
      case THTTPEvent::EGotResponseBodyData:
          {
          GBA_TRACE_DEBUG(("THTTPEvent::EGotResponseBodyData"));
          // Get the body data supplier
          iRespBody = aTransaction.Response().Body();
      
          // Some (more) body data has been received (in the HTTP response)
          // Append to the output file if we're saving responses
          TPtr8 ptrResp(iResponse->Des());
          
          TPtrC8 bodyData;
          TBool lastChunk = EFalse;
                    
          while ( !lastChunk ) 
              {
              GBA_TRACE_DEBUG(("THTTPEvent::EGotResponseBodyData in loop"));
              lastChunk = iRespBody->GetNextDataPart( bodyData );
              
              //Issue with formatting string with a special character
              //GBA_TRACE_DEBUG( bodyData );
              if ( bodyData.Length() <= ( ptrResp.MaxLength() - ptrResp.Length() ) )
                  { 
                  ptrResp.Append( bodyData );  
                  }
              else
                  {
                  break;  
                  }
              } 
      
          GBA_TRACE_DEBUG_NUM(("Response body received size is %d"), iResponse->Length() );
          // Done with that bit of body data
          iRespBody->ReleaseData();
          } 
          break;
          
      case THTTPEvent::EResponseComplete:
          {
          GBA_TRACE_DEBUG(("THTTPEvent::EResponseComplete"));
          // The transaction's response is complete
          } 
          break;
          
      case THTTPEvent::ESucceeded:
         {
         GBA_TRACE_DEBUG(("THTTPEvent::ESucceeded || KErrDisconnected"));
         if ( iSucceeded )
             {
              // verify authrsp
              User::LeaveIfError(CheckRspAuthL(aTransaction));
              
              //Set it back
              iSucceeded=EFalse;
              
              GBA_TRACE_DEBUG(("THTTPEvent::ESucceeded || KErrDisconnected 1"));
              iEventSink->EventRequestCompletedL(KErrNone); 
             }
         else
            {
            iEventSink->EventRequestCompletedL( KErrDisconnected );        
            GBA_TRACE_DEBUG(("Connection was closed"));
            }
        } 
        break;
        
     case THTTPEvent::EFailed:
        {
        GBA_TRACE_DEBUG(("THTTPEvent::EFailed"));
        RHTTPResponse resp = aTransaction.Response();
        TInt status = resp.StatusCode();
        GBA_TRACE_DEBUG_NUM( ("http StatusCode : %d "), status );
        iEventSink->EventRequestCompletedL( KErrUnknown );
        } 
        break;
     
     //The transaction is being cancelled. An outgoing event. 
     case THTTPEvent::ECancel:
        {
        GBA_TRACE_DEBUG(("THTTPEvent::ECancel"));
        aTransaction.Cancel();
        } 
        break;

     // this warning can come when going through proxies
     case THTTPEvent::ERedirectedTemporarily:
        break;
        
     default:
         {
         GBA_TRACE_DEBUG_NUM(("Failed with HTTP event event: %d "),aEvent.iStatus );
         if( aEvent.iStatus < 0 )
             {
             // close off the transaction if it's an error
             aTransaction.Cancel();
             iEventSink->EventRequestCompletedL( aEvent.iStatus );
             }
         } 
         break;
       }
    }


// -----------------------------------------------------------------------------
// C3GPPBootstrapHttpHandler::MHFRunError()
// -----------------------------------------------------------------------------
// 
TInt C3GPPBootstrapHttpHandler::MHFRunError(TInt aError, RHTTPTransaction /*aTransaction*/, const THTTPEvent& aEvent)
    {
    GBA_TRACE_DEBUG_NUM(("MHFRunError called with error code = %d"), aError);    
    GBA_TRACE_DEBUG_NUM(("MHFRunError called with http event = %d"),aEvent.iStatus );
    // tell caller that there was an error
    TRAPD(err, iEventSink->EventRequestCompletedL(aError));
    return err;
    }
  

// -----------------------------------------------------------------------------
// C3GPPBootstrapHttpHandler::HandleAuthorizationChallengeL()
// -----------------------------------------------------------------------------
// 
TInt C3GPPBootstrapHttpHandler::HandleAuthorizationChallengeL( RHTTPTransaction aTransaction, const THTTPEvent& /*aEvent*/ )
    {
    GBA_TRACE_BEGIN();
    TInt pushCount = 0;
    RStringPool stringPool = aTransaction.Session().StringPool(); 
    RStringF wwwAuthHeader = stringPool.StringF(HTTP::EWWWAuthenticate,RHTTPSession::GetTable());
    RHTTPHeaders headers(aTransaction.Response().GetHeaderCollection());
    
    //find header which has digest algorithm and qop is auth-int
    TInt headerPart = FindHeaderPartToUseL( aTransaction );

    if ( headerPart == KErrNotFound )
        {
        return KErrArgument;
        }

    THTTPHdrVal authTypeParam;
    TInt error = headers.GetField( wwwAuthHeader, headerPart, authTypeParam );
  
    // If anything goes wrong, just stop. This will have the effect of
    // returning the error response from the server to the client.
    if ( error != KErrNone )
        {
        return error;     
        }
        
    TBool    haveUserPW = EFalse;
    RString  realm;
    RString  username;
    RString  password;
    RString  nonce;
    RString  algorithm;
    RString  resync;                  
  
    // Get realm value from response header
    THTTPHdrVal realmVal;
    error  = headers.GetParam( wwwAuthHeader, 
                           stringPool.StringF( HTTP::ERealm, RHTTPSession::GetTable() ), realmVal, headerPart ) ;
    if ( error != KErrNone )
        {
        return error;
        }
    
    realm = realmVal.Str();
    GBA_TRACE_DEBUG(("Realm:"));
    GBA_TRACE_DEBUG(realm.DesC());
    
    THTTPHdrVal nonceVal;
    //Get nonce value from response header
    // contain the RAND and AUTH
    error = headers.GetParam( stringPool.StringF(HTTP::EWWWAuthenticate,RHTTPSession::GetTable()), 
                              stringPool.StringF(HTTP::ENonce,RHTTPSession::GetTable()),
                              nonceVal,
                              headerPart);
    if ( error != KErrNone )
        {
        return error;
        }
    
    nonce=nonceVal.Str();
    GBA_TRACE_DEBUG(("Nonce:"));
    GBA_TRACE_DEBUG(nonce.DesC());
  
    //Fetch algorithm type
    THTTPHdrVal algVal;  
    error = headers.GetParam( stringPool.StringF(HTTP::EWWWAuthenticate,RHTTPSession::GetTable()), 
                              stringPool.StringF(HTTP::EAlgorithm,RHTTPSession::GetTable()),
                              algVal,
                              headerPart);
    if ( error != KErrNone )
        {
        return error;
        }
    
    algorithm=algVal.Str();
    GBA_TRACE_DEBUG(("Algorithm:"));
    GBA_TRACE_DEBUG(algorithm.DesC());
    
    // If the SQN is out of range, Resync is required
    TBool resyncrequired = EFalse;
    
    //Get password from smart card
    //The smart card will first authenticate the nonce value 
    RStringF authType = authTypeParam.StrF(); 
    
    TRAP( error, haveUserPW = iDataRetrieverCallBack->GetCredentialsL( aTransaction.Request().URI(),
              realm, nonce, algorithm, username, password, resync, resyncrequired ) ) ;
    
    if ( error != KErrNone )
        {
        GBA_TRACE_DEBUG_NUM(("Get Gredentails failed with error %d"), error );
        return error;
        }
    
    if ( haveUserPW || resyncrequired )
        {
        CleanupClosePushL(username);
        pushCount++;
        CleanupClosePushL(password);
        pushCount++;       
        //Resync required
        if( resyncrequired )
            {
            CleanupClosePushL(resync);
            pushCount++;     
            }
        
        GBA_TRACE_DEBUG(("HandleAuthorizationChallengeL: received following credentials"));
        GBA_TRACE_DEBUG(("Realm:"));
        GBA_TRACE_DEBUG(realm.DesC());
        GBA_TRACE_DEBUG(("Username:"));
        GBA_TRACE_DEBUG(username.DesC());
        GBA_TRACE_DEBUG(("Password:"));
        GBA_TRACE_DEBUG_BINARY(password.DesC());
        GBA_TRACE_DEBUG(("Resync:"));
        GBA_TRACE_DEBUG_BINARY(resync.DesC());
        
        }
    else //No password and no resync required
        {
        CleanupStack::PopAndDestroy( pushCount ); 
        return KErrNotFound;  
        }

    // Encode the Username/Password
    switch ( authType.Index( RHTTPSession::GetTable() ) )
        {
        case HTTP::EDigest:
           {
           THTTPHdrVal requestUri;
           RHTTPTransactionPropertySet propSet = aTransaction.PropertySet();
           TBool exist = propSet.Property( stringPool.StringF(HTTP::EUri,RHTTPSession::GetTable()), requestUri );
           
           if(!exist)
               {
               CleanupStack::PopAndDestroy( pushCount ); 
               return KErrNotFound;
               }
           
           //save rsp auth calcuation
           //Save the string handle for rsp auth checking
           //Close the previous handle, then set new one
           iUsername.Close();
           iUsername = username.Copy();
           iPassword.Close();
           iPassword = password.Copy();
           iRealm.Close();
           iRealm = realm.Copy();
           iNonce.Close();
           iNonce = nonce.Copy();
           iUri.Close();
           iUri = requestUri.Str().Copy();
           
           TRAP ( error, EncodeDigestAuthL( username, password, aTransaction, headerPart, requestUri ) );
           GBA_TRACE_DEBUG(("After Encode Digest AuthL"));
           if ( error != KErrNone )
               {
               GBA_TRACE_DEBUG_NUM(("After Encode Digest AuthL error %d"), error );
               CleanupStack::PopAndDestroy( pushCount );
               return error;
               }
           }
           break;
        default:
           {
           CleanupStack::PopAndDestroy( pushCount );
           return KErrArgument;
           }
           
        }
     GBA_TRACE_DEBUG(("After Encode Digest AuthL 1"));
    // Resubmit the request to the server
    // add resyncronization header to the request if it is available
    if ( resyncrequired )
        {
        THTTPHdrVal hdrVal;
        RStringF unStr = stringPool.OpenFStringL(KAuts);
        CleanupClosePushL(unStr);
        pushCount++;
        hdrVal.SetStr(resync);
        RHTTPHeaders requestHeaders(aTransaction.Request().GetHeaderCollection());
        requestHeaders.SetFieldL( stringPool.StringF(HTTP::EAuthorization,RHTTPSession::GetTable()), 
                                 THTTPHdrVal(stringPool.StringF(HTTP::EDigest,RHTTPSession::GetTable())),
                                 unStr,
                                 hdrVal );
       }
    GBA_TRACE_DEBUG(("After Encode Digest AuthL 2"));   
    CleanupStack::PopAndDestroy( pushCount ); 
    GBA_TRACE_DEBUG(("After Encode Digest AuthL 3"));
    return KErrNone;
    }


// -----------------------------------------------------------------------------
// C3GPPBootstrapHttpHandler::FindHeaderPartToUseL()
// -----------------------------------------------------------------------------
//
TInt C3GPPBootstrapHttpHandler::FindHeaderPartToUseL( RHTTPTransaction aTransaction ) const
    {
    //We only pick up the first digest authentication fields:
    
    RStringPool stringPool = aTransaction.Session().StringPool(); 

    RStringF wwwAuthenticate = stringPool.StringF(HTTP::EWWWAuthenticate,RHTTPSession::GetTable());
    RStringF realm = stringPool.StringF(HTTP::ERealm,RHTTPSession::GetTable());
    RStringF nonce = stringPool.StringF(HTTP::ENonce,RHTTPSession::GetTable());
    RStringF qop   = stringPool.StringF(HTTP::EQop,RHTTPSession::GetTable());
    
    RHTTPHeaders headers = aTransaction.Response().GetHeaderCollection();
    const TInt parts = headers.FieldPartsL( wwwAuthenticate );
    
    for ( TInt i = 0; i < parts; i++ )
    {
    THTTPHdrVal fieldVal;// The name of the current field.
    THTTPHdrVal hdrVal;//A scratch hdrVal
    
    TInt error = headers.GetField(wwwAuthenticate, i, fieldVal);
    
    if(error != KErrNone)
        {
        return KErrNotFound;
        }
    
    switch ( fieldVal.StrF().Index(RHTTPSession::GetTable() ) )
        {
        case HTTP::EDigest:
            {
            // It is Digest, let us check realm, nonce, qop then
            if ( headers.GetParam(wwwAuthenticate, realm, hdrVal, i ) == KErrNone 
                 &&
                 headers.GetParam(wwwAuthenticate, nonce, hdrVal, i ) == KErrNone)
                {
                // We've got a realm and a nonce. If there's a qop, we
                // need to check it's acceptable.
               if ( headers.GetParam(wwwAuthenticate, qop, hdrVal, i ) == KErrNone 
                    && 
                    FindAuth( hdrVal.Str().DesC() ) )
                     {
                     // the header has qop and it is auth-int.
                     // we found it
                     return i;
                     }

                }
            }
            break;

        default:
            //Ingore if it is not Digest
            break;
        }
    }
    return KErrNotFound;
}



// -----------------------------------------------------------------------------
// C3GPPBootstrapHttpHandler::EncodeDigestAuthL()
// -----------------------------------------------------------------------------
//
void C3GPPBootstrapHttpHandler::EncodeDigestAuthL( const RString& aUsername, 
            const RString& aPW, 
            RHTTPTransaction& aTransaction,
            TInt aHeaderPart,
            const THTTPHdrVal& aRequestUri )
    {
    GBA_TRACE_BEGIN();
    TInt pushCount = 0; 
    
    RStringPool stringPool = aTransaction.Session().StringPool(); 

    GBA_TRACE_DEBUG(("Username:"));
    GBA_TRACE_DEBUG(aUsername.DesC());

    GBA_TRACE_DEBUG(("Passwd:"));
    GBA_TRACE_DEBUG_BINARY(aPW.DesC());
  
    TBuf8<KHashLength> hash;
    TBuf8<KHashLength> cnonce;
    //Nonce
    THTTPHdrVal nonce;
    RHTTPHeaders headers = aTransaction.Response().GetHeaderCollection();
    TInt error = headers.GetParam(stringPool.StringF(HTTP::EWWWAuthenticate,RHTTPSession::GetTable()), 
          stringPool.StringF(HTTP::ENonce,RHTTPSession::GetTable()), nonce,
          aHeaderPart);
    
    if ( error != KErrNone )
        {
        User::LeaveIfError( error );
        }
    
    //realm
    THTTPHdrVal realm;
    error = headers.GetParam(stringPool.StringF(HTTP::EWWWAuthenticate,RHTTPSession::GetTable()),
           stringPool.StringF(HTTP::ERealm,RHTTPSession::GetTable()), realm,
           aHeaderPart);
    
    if ( error != KErrNone )
        {
        User::LeaveIfError( error );
        }
    
    //qop       
    THTTPHdrVal qop;
    error = headers.GetParam(stringPool.StringF(HTTP::EWWWAuthenticate,RHTTPSession::GetTable()),
           stringPool.StringF(HTTP::EQop,RHTTPSession::GetTable()), qop,
           aHeaderPart);
    
   if ( error != KErrNone )
        {
        User::LeaveIfError( error );
        }
    
    GenerateCNonce( cnonce );
    
    GBA_TRACE_DEBUG(("cnonce:"));
    GBA_TRACE_DEBUG_BINARY( cnonce );
    
    RString cnonceString = stringPool.OpenStringL(cnonce); 
    CleanupClosePushL(cnonceString);
    pushCount++;
    
    // stored for auth calculations
    iCnonce.Close();
    iCnonce = cnonceString.Copy();
    
    // stored for auth calculations
    iMethod.Close();
    iMethod = aTransaction.Request().Method().Copy();
    
    DigestCalcL( KMD5HashofEmpty, hash );
    
    GBA_TRACE_DEBUG(("hash:"));
    GBA_TRACE_DEBUG_BINARY( hash );
    
    // OK. hash now contains the digest response. Set up the header.
    RHTTPHeaders requestHeaders(aTransaction.Request().GetHeaderCollection());
    RHTTPHeaders responseHeaders(aTransaction.Response().GetHeaderCollection());
  
    requestHeaders.RemoveField(stringPool.StringF(HTTP::EAuthorization,RHTTPSession::GetTable())); 
    
    //Copy useranme
    THTTPHdrVal hdrVal;
    hdrVal.SetStr(aUsername);
    
    //Set the username to request header
    requestHeaders.SetFieldL(stringPool.StringF(HTTP::EAuthorization,RHTTPSession::GetTable()), 
           THTTPHdrVal(stringPool.StringF(HTTP::EDigest,RHTTPSession::GetTable())),
           stringPool.StringF(HTTP::EUsername,RHTTPSession::GetTable()),
           hdrVal);
        
    // copy Realm from response headers
    responseHeaders.GetParam(stringPool.StringF(HTTP::EWWWAuthenticate,RHTTPSession::GetTable()), 
           stringPool.StringF(HTTP::ERealm,RHTTPSession::GetTable()),
           hdrVal, 
           aHeaderPart);
    
    // Set the Realm to request header
    requestHeaders.SetFieldL(stringPool.StringF(HTTP::EAuthorization,RHTTPSession::GetTable()), 
           THTTPHdrVal(stringPool.StringF(HTTP::EDigest,RHTTPSession::GetTable())),
           stringPool.StringF(HTTP::ERealm,RHTTPSession::GetTable()),
           hdrVal);
    
    // copy nonce from response headers
    responseHeaders.GetParam(stringPool.StringF(HTTP::EWWWAuthenticate,RHTTPSession::GetTable()), 
           stringPool.StringF(HTTP::ENonce,RHTTPSession::GetTable()), 
           hdrVal,
           aHeaderPart);
    
    // set nonce to request header
    requestHeaders.SetFieldL(stringPool.StringF(HTTP::EAuthorization,RHTTPSession::GetTable()), 
           THTTPHdrVal(stringPool.StringF(HTTP::EDigest,RHTTPSession::GetTable())),
           stringPool.StringF(HTTP::ENonce,RHTTPSession::GetTable()),
           hdrVal);
    
    // Set uri to request header.
    // the uri has already been fetched from http property setting
    requestHeaders.SetFieldL(stringPool.StringF(HTTP::EAuthorization,RHTTPSession::GetTable()), 
           THTTPHdrVal(stringPool.StringF(HTTP::EDigest,RHTTPSession::GetTable())),
           stringPool.StringF(HTTP::EUri,RHTTPSession::GetTable()),
           aRequestUri);
    
    // Qop with quotation marks   
    // copy qop from response headers
     responseHeaders.GetParam(stringPool.StringF(HTTP::EWWWAuthenticate,RHTTPSession::GetTable()), 
           stringPool.StringF(HTTP::EQop,RHTTPSession::GetTable()), 
           hdrVal,
           aHeaderPart);
    
    // Set Qop to request header
     requestHeaders.SetFieldL(stringPool.StringF(HTTP::EAuthorization,RHTTPSession::GetTable()), 
           THTTPHdrVal(stringPool.StringF(HTTP::EDigest,RHTTPSession::GetTable())),
           stringPool.StringF(HTTP::EQop,RHTTPSession::GetTable()),
           hdrVal);
     
    //Set Nc
    hdrVal.SetStrF(stringPool.StringF(HTTP::E00000001,RHTTPSession::GetTable()));
    requestHeaders.SetFieldL(stringPool.StringF(HTTP::EAuthorization,RHTTPSession::GetTable()), 
                  THTTPHdrVal(stringPool.StringF(HTTP::EDigest,RHTTPSession::GetTable())),
                  stringPool.StringF(HTTP::ENc,RHTTPSession::GetTable()),
                  hdrVal);
     
    // Set Cnonce to request header.
    hdrVal.SetStr(cnonceString);
    requestHeaders.SetFieldL(stringPool.StringF(HTTP::EAuthorization,RHTTPSession::GetTable()), 
           THTTPHdrVal(stringPool.StringF(HTTP::EDigest,RHTTPSession::GetTable())),
           stringPool.StringF(HTTP::ECnonce,RHTTPSession::GetTable()),
           hdrVal);
   
    // Set response to request header.
    RString hashStr = stringPool.OpenStringL(hash);
    CleanupClosePushL(hashStr);
    hdrVal.SetStr(hashStr);
    requestHeaders.SetFieldL(stringPool.StringF(HTTP::EAuthorization,RHTTPSession::GetTable()), 
           THTTPHdrVal(stringPool.StringF(HTTP::EDigest,RHTTPSession::GetTable())),
           stringPool.StringF(HTTP::EResponse,RHTTPSession::GetTable()),
           hdrVal);
    CleanupStack::PopAndDestroy(&hashStr);

    // Get Opaque from response header.
    error = responseHeaders.GetParam(stringPool.StringF(HTTP::EWWWAuthenticate,RHTTPSession::GetTable()), 
             stringPool.StringF(HTTP::EOpaque,RHTTPSession::GetTable()), 
             hdrVal, aHeaderPart);
    
    // Set Opaque 
    if ( error != KErrNotFound )
        {
        GBA_TRACE_DEBUG((" found the opaque value and copy it"));
        requestHeaders.SetFieldL(stringPool.StringF(HTTP::EAuthorization,RHTTPSession::GetTable()), 
                       THTTPHdrVal(stringPool.StringF(HTTP::EDigest,RHTTPSession::GetTable())),
                       stringPool.StringF(HTTP::EOpaque,RHTTPSession::GetTable()),
                       hdrVal);
        } 
    else
        {
        GBA_TRACE_DEBUG((" No opaque value send empty one"));
        RString opaque =  stringPool.OpenStringL(Kfopaque);
        GBA_TRACE_DEBUG((" No opaque value send empty one 1"));
        CleanupClosePushL(opaque);
        pushCount++;
        hdrVal.SetStr(opaque);
       GBA_TRACE_DEBUG((" No opaque value send empty one 2"));
        requestHeaders.SetFieldL(stringPool.StringF(HTTP::EAuthorization,RHTTPSession::GetTable()), 
             THTTPHdrVal(stringPool.StringF(HTTP::EDigest,RHTTPSession::GetTable())),
             stringPool.StringF(HTTP::EOpaque,RHTTPSession::GetTable()),
             hdrVal);
       }
    GBA_TRACE_DEBUG((" No opaque value send empty one 3"));
    // Set algorithm.   
    TBuf8<KAlgLength> algbuf;
    algbuf.Copy(_L8(", "));
    algbuf.Append(Kalgorithm);
    algbuf.Append(_L8("="));
    algbuf.Append(Kfalgo);
   
    GBA_TRACE_DEBUG((" No opaque value send empty one 5"));
   
    RStringF valStr = stringPool.OpenFStringL(algbuf);
    THTTPHdrVal val(valStr);
    requestHeaders.SetFieldL(stringPool.StringF(HTTP::EAuthorization,RHTTPSession::GetTable()), val);
    valStr.Close();
    
    GBA_TRACE_DEBUG((" No opaque value send empty one 4"));

    CleanupStack::PopAndDestroy(pushCount);
    GBA_TRACE_DEBUG((" No opaque value send empty one 6"));
    }


// -----------------------------------------------------------------------------
// C3GPPBootstrapHttpHandler::FindAuth()
// -----------------------------------------------------------------------------
//
TBool C3GPPBootstrapHttpHandler::FindAuth(const TDesC8& aQop) const
    {
    GBA_TRACE_BEGIN();
    if ( aQop == KAuthInt )
        {
        GBA_TRACE_DEBUG((" qop is equal to auth-int"));
        return ETrue;
        }
    else
        {
        GBA_TRACE_DEBUG((" qop is NOT equal to auth-int"));
        GBA_TRACE_DEBUG((""));
        return EFalse;    
        }    
    }


// -----------------------------------------------------------------------------
// C3GPPBootstrapHttpHandler::HA1L()
// -----------------------------------------------------------------------------
//
void C3GPPBootstrapHttpHandler::HAOneL(const RString& aUsername,const RString& aPW,
           const RString& aRealm, TDes8& aResult)
    {
    HBufC8* a1 = HBufC8::NewMaxLC(aUsername.DesC().Length() 
           + aPW.DesC().Length()  
           + 1
           + aRealm.DesC().Length() + 1
           );
    TPtr8 a1Mod = a1->Des();
    a1Mod.Zero();
    
    a1Mod.Append(aUsername.DesC());
    a1Mod.Append(':');
    a1Mod.Append(aRealm.DesC());
    a1Mod.Append(':');
    a1Mod.Append(aPW.DesC().Ptr(),aPW.DesC().Length());

    GBA_TRACE_DEBUG(a1Mod);
    Hash(*a1, aResult);
    GBA_TRACE_DEBUG(("after hash"));
    CleanupStack::PopAndDestroy(a1);
    }


// -----------------------------------------------------------------------------
// C3GPPBootstrapHttpHandler::HAtwoL()
// -----------------------------------------------------------------------------
//
void C3GPPBootstrapHttpHandler::HATwoL(const RStringF& aMethod, 
           const RString& aRequestUri,
           const TDesC8& aHentity,
           TDes8& aResult)
    {
    // In the auth qop, a2 is Method:digest-uri-value:empty body hash
    // Allocate enough space for the method, the URI and the colon, empty content-hash and colon.
    TPtrC8 requestUri = aRequestUri.DesC();
    TPtrC8 method = aMethod.DesC();
    HBufC8* a2 = HBufC8::NewMaxLC( requestUri.Length() + method.Length() + KNumber34 );
    TPtr8 a2Mod = a2->Des();
    a2Mod.Zero();
    a2Mod.Append(method);
    a2Mod.Append(':');
    
    a2Mod.Append( requestUri );
    GBA_TRACE_DEBUG( requestUri );
    // since for this method is always auth-int we always add hentity
    a2Mod.Append(':');
    a2Mod.Append(aHentity);
    GBA_TRACE_DEBUG(a2Mod);

    Hash(*a2, aResult);

    GBA_TRACE_DEBUG_BINARY(aResult);
    CleanupStack::PopAndDestroy(a2);
    
    GBA_TRACE_DEBUG(("Exit HATwoL"));
    }


// -----------------------------------------------------------------------------
// C3GPPBootstrapHttpHandler::Hash()
// -----------------------------------------------------------------------------
//
void C3GPPBootstrapHttpHandler::Hash(const TDesC8& aMessage, TDes8& aHash)
    {
    GBA_TRACE_DEBUG(("hash"));
    // Calculate the 128 bit (16 byte) hash
    iMD5Calculator->Reset();
    TPtrC8 hash = iMD5Calculator->Hash(aMessage);
    GBA_TRACE_DEBUG(("after hash"));
    // Now print it as a 32 byte hex number
    aHash.Zero();
    for ( TInt ii = 0; ii < KRawHashLength; ii++ )
        {
        TBuf8<2> scratch;
        scratch.Zero();
        scratch.Format(formatStr, hash[ii]);
        aHash.Append(scratch);
      }
    }


// -----------------------------------------------------------------------------
// C3GPPBootstrapHttpHandler::GenerateCNonce()
// -----------------------------------------------------------------------------
//
void C3GPPBootstrapHttpHandler::GenerateCNonce(TDes8& aNonce)
    {
    // The purpose of the client nonce is to protect aextst 'chosen
    // plaintext' attacks where a hostile server tricks the client
    // into supplying a password using a specific server nonce that
    // allows an (as yet undiscovered) flaw in MD5 to recover the
    // password. As such the only important thing about client nonces
    // is that they change and aren't predictable. See section 4.9 of
    // RFC2616

    TTime time;
    time.UniversalTime();
    TInt64 randomNumber = Math::Rand(iSeed);
    randomNumber <<= KNumber32;
    randomNumber += Math::Rand(iSeed);
    TBuf8<KNumber33> key;
    key.Zero();
    key.AppendNum(time.Int64(), EHex);
    key.Append(KColon);
    key.AppendNum(randomNumber, EHex);
  
    Hash(key, aNonce);
    }


// -----------------------------------------------------------------------------
// C3GPPBootstrapHttpHandler::CheckRspAuthL()
// -----------------------------------------------------------------------------
//
TInt C3GPPBootstrapHttpHandler::CheckRspAuthL(RHTTPTransaction aTransaction)
    {
    RStringF wwwAuthInfoHeader = aTransaction.Session().StringPool().OpenFStringL(KHTTPAuthInfo);
    RHTTPHeaders headers(aTransaction.Response().GetHeaderCollection());

    THTTPHdrVal rspAuthVal;
    
    TInt retVal = headers.FieldPartsL(wwwAuthInfoHeader);

    TInt error = headers.GetField(wwwAuthInfoHeader, 0, rspAuthVal);

    if(error != KErrNone)
    {
    	wwwAuthInfoHeader.Close();
    	return error;
    } 
    wwwAuthInfoHeader.Close();
    return KErrNone;      
    }


// -----------------------------------------------------------------------------
// C3GPPBootstrapHttpHandler::DigestCalcL()
// -----------------------------------------------------------------------------
//
void C3GPPBootstrapHttpHandler::DigestCalcL(
    const TDesC8& aHentity,
    TDes8& aResult )
    {
    TInt stringToHashLength = iNonce.DesC().Length() + 
          KNumber8 + // auth-int length 
          KNumber3 * KHashLength + KNumber8 + iPassword.DesC().Length() + KNumber5;
    HBufC8* stringToHash = HBufC8::NewMaxLC(stringToHashLength);
    TPtr8 stringMod = stringToHash->Des();
    stringMod.Zero();
    HAOneL(iUsername, iPassword, iRealm, stringMod);
    stringMod.Append(':');
    stringMod.Append(iNonce.DesC());
    stringMod.Append(KNCVal_2);
    stringMod.Append(iCnonce.DesC());
    stringMod.Append(':');
    stringMod.Append(KAuthInt);
    stringMod.Append(':');
    TBuf8<KHashLength> hash;
    HATwoL(iMethod, iUri, aHentity,hash);
    GBA_TRACE_DEBUG_BINARY(stringMod);
    stringMod.Append(hash);   
    GBA_TRACE_DEBUG(stringMod);
    Hash(*stringToHash, aResult);
    CleanupStack::PopAndDestroy(stringToHash);
    };

// -----------------------------------------------------------------------------
// C3GPPBootstrapHttpHandler::GetResponse()
// -----------------------------------------------------------------------------
//
 HBufC8* C3GPPBootstrapHttpHandler::GetResponse()
    {
    return iResponse->Alloc();
    };
    
//EOF
