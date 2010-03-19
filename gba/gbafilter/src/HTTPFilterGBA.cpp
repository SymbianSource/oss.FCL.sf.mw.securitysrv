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
* Description:  Implementation of CHTTPFilterGBA
*
*/

#include <http/rhttptransaction.h>
#include <http/rhttpheaders.h>
#include <http/rhttpresponse.h>
#include <httperr.h>
#include <httpstringconstants.h>
#include <imcvcodc.h>      //for base64 en/decoding
#include "HTTPFilterGBA.h"
#include "GbaCommon.h"
#include <bautils.h>
#include <e32math.h>
#include <hash.h>
#include <e32const.h>
#include "GBALogger.h"

//Constants
const TInt KStatusTextLength512 = 512;
const TInt KB64KeySize = 64;
const TInt KIntegerConstant8 = 8;
const TInt KIntegerConstant32 = 32;
const TInt KIntegerConstant33 = 33;
// Length of a digest hash when represented in hex
const TInt KHashLength = 32;
// Length of a digest hash before converting to hex.
const TInt KRawHashLength = 16;
// Length of nonce-count
const TInt KNonceCountLength = 8;

_LIT8( KUserAgentProductToken,"3gpp-gba");
_LIT8(K3GPPRealmString,"3GPP-Bootstrapping@");
_LIT8( KMd5AlgorithmStr, "MD5" );
_LIT8( KMd5SessAlgorithmStr, "MD5-sess" );
_LIT8( KQopAuthStr, "auth" );
_LIT8( KQopAuthIntStr, "auth-int" );
_LIT8( KAuthenticationInfoStr, "Authentication-Info" );
_LIT8( KColon, ":" );
_LIT8( KDefaultProtocolAlg, "\x01\x00\x00\x00\x02");
_LIT(KGBAHttpFilter, "HTTP FILTER GBA");

// format for output of data/time values
#if defined (_DEBUG) && defined (_LOGGING)
_LIT(KDateFormat,"%D%M%Y%/0%1%/1%2%/2%3%/3 %:0%H%:1%T%:2%S.%C%:3");
#endif

void PanicGBAHttpFilters(TInt aErr = 0)
{
    User::Panic(KGBAHttpFilter, aErr);
}

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CHTTPFilterGBA::CHTTPFilterGBA
// Constructor
// -----------------------------------------------------------------------------
//
CHTTPFilterGBA::CHTTPFilterGBA()
		: iBootstrapCount(0),iHaveGbaBootstrapCredentials(EFalse)
{

}

// -----------------------------------------------------------------------------
// CHTTPFilterGBA::InstallFilterL
// Initialize the filter and register it to sesssion's filter collection
// -----------------------------------------------------------------------------
//
CEComFilter* CHTTPFilterGBA::InstallFilterL(TAny* aParams)
{
	GBA_TRACE_DEBUG(("CHTTPFilterGBA::CHTTPFilterGBA++"));
	__ASSERT_DEBUG(aParams != NULL, PanicGBAHttpFilters());
	RHTTPSession* session = REINTERPRET_CAST(RHTTPSession*, aParams);
	CHTTPFilterGBA* filter = new (ELeave) CHTTPFilterGBA();
	CleanupStack::PushL(filter);
	filter->ConstructL(*session);
	CleanupStack::Pop(filter);
	GBA_TRACE_DEBUG(("CHTTPFilterGBA::CHTTPFilterGBA--"));
	return filter;
}
	
// -----------------------------------------------------------------------------
// CHTTPFilterGBA::ConstructL
// Memory and resource allocation, leaves
// -----------------------------------------------------------------------------
//
void CHTTPFilterGBA::ConstructL(RHTTPSession aSession)
{
    GBA_TRACE_DEBUG(("CHTTPFilterGBA::ConstructL++"));
	
	//......................................................................................
	
	//Unload GBA http filter from existing GBA and MBMS server's
	
	
	//This GBA http filter cannot be loaded from old GBA server (GBAServer) and it's 
	//Required to reject a filter loading request from a GBA server  on existing platforms
	//where GBA component is already built-in and the GBA server UID3 is 0x2000F868
	//Note- some s60 3.2 and 5.0 platforms already has GBA component as built in.
	//Reason: Ub interface should not have the User-Agent string "3gpp-gba"
    if( RProcess().SecureId() == TSecureId (0x2000F868))
    {
    	GBA_TRACE_DEBUG(("Not accessible from GBA Server++"));
        return;
    }
    
    //This GBA http filter cannot be loaded from MBMS server (MBMSServer) and it's 
    //Required to reject a filter loading request from a MBMS server
    //where MBMS component is already build-in[N96 platforms only] and the mbms server UID3 is 0x2001957B
    //Reason: Ub interface should not have the User-Agent string "3gpp-gba"
    if( RProcess().SecureId() == TSecureId (0x2001957B))
    {
        GBA_TRACE_DEBUG(("Not accessible from MBMS Server++"));
        return;
    } 
    //......................................................................................
       
   	iProtocolIdentifier.Copy(KDefaultProtocolAlg);

	iUICCLabel = KUSIM;
	iStringPool = aSession.StringPool();

	// register the filter
	RStringF filterName = iStringPool.OpenFStringL( KHTTPFilterGBAName );
	CleanupClosePushL( filterName );

    iOpaqueStr = iStringPool.StringF( HTTP::EOpaque, RHTTPSession::GetTable() );
    iNonceStr = iStringPool.StringF( HTTP::ENonce, RHTTPSession::GetTable() );
    iQopStr = iStringPool.StringF( HTTP::EQop, RHTTPSession::GetTable() );
    iStaleStr = iStringPool.StringF( HTTP::EStale, RHTTPSession::GetTable() );
    
    iMd5Str = iStringPool.OpenFStringL( KMd5AlgorithmStr );
    iMd5SessStr = iStringPool.OpenFStringL( KMd5SessAlgorithmStr );
    iQopAuthStr = iStringPool.OpenFStringL( KQopAuthStr );
    iAuthInfo = iStringPool.OpenFStringL( KAuthenticationInfoStr );
    
    iRealmStr = iStringPool.StringF( HTTP::ERealm, RHTTPSession::GetTable() );
    iUsernameStr = iStringPool.StringF( HTTP::EUsername, RHTTPSession::GetTable() );
    iPasswordStr = iStringPool.StringF( HTTP::EPassword, RHTTPSession::GetTable() );
	iUserAgent = iStringPool.OpenFStringL( KUserAgentProductToken );
    
    //Regsiter for THTTPEvent::ESubmit http event
    //The User-Agent string gets appended with "3gpp-gba" string, 
    //indicating client is gba capable to the application server.
    aSession.FilterCollection().AddFilterL( *this,
                                            THTTPEvent::ESubmit,
                                            RStringF(),
                                            KAnyStatusCode,
                                            EStatusCodeHandler,
                                            filterName);
   
    //Regsiter for THTTPEvent::EGotResponseHeaders http event with "Authentication-Info" header 
    //and HTTPStatus::EOk http status code,just to know if the credentials established are valid.
    aSession.FilterCollection().AddFilterL( *this,
                                            THTTPEvent::EGotResponseHeaders,
                                            iAuthInfo,
                                            HTTPStatus::EOk,
                                            EStatusCodeHandler,
                                            filterName );

    //Regsiter for THTTPEvent::EGotResponseHeaders http event with "WWW-Authenticate" header 
    //and HTTPStatus::EUnauthorized http status code
    //The priority for this is set to MHTTPFilter::EStatusCodeHandler - 1,
    //so the gba filter intercepts the transaction before the default 
    //http digest authentication filter(which has priority MHTTPFilter::EStatusCodeHandler).

    aSession.FilterCollection().AddFilterL( *this, 
											THTTPEvent::EGotResponseHeaders,	// Any transaction event
											iStringPool.StringF(HTTP::EWWWAuthenticate,RHTTPSession::GetTable()),
											HTTPStatus::EUnauthorized,
											MHTTPFilter::EStatusCodeHandler - 1,	  // Priority of filter
											filterName );						      // Name of filter

	CleanupStack::PopAndDestroy( &filterName );
	
	//Check if the "user agent" header field in http session headers has "3gpp-gba" string in it.
	//If it does n't have "3gpp-gba" string,append  it to the existing value.
    TBool found = EFalse;
    TPtrC8 rawFieldData;
    RHTTPHeaders sessionHeaders = aSession.RequestSessionHeadersL();
    RStringF sessionHeaderfieldname = aSession.StringPool().StringF( HTTP::EUserAgent, RHTTPSession::GetTable());
    sessionHeaders.GetRawField(sessionHeaderfieldname, rawFieldData);
    found = rawFieldData.Find(iUserAgent.DesC()) != KErrNotFound;

    if(!found)
        sessionHeaders.SetFieldL( sessionHeaderfieldname, iUserAgent );	    

    iMD5Calculator = CMD5::NewL();
	GBA_TRACE_DEBUG(("CHTTPFilterGBA::ConstructL--"));
}


//------------------------------------------------------------------------
// CHTTPFilterGBA::~CHTTPFilterGBA
// Destructor
//------------------------------------------------------------------------
//
CHTTPFilterGBA::~CHTTPFilterGBA()
{
	GBA_TRACE_DEBUG(("CHTTPFilterGBA::~CHTTPFilterGBA++"));
	CleanupAll();															
	GBA_TRACE_DEBUG(("CHTTPFilterGBA::~CHTTPFilterGBA--"));
}

//------------------------------------------------------------------------
// CHTTPFilterGBA::MHFLoad
//------------------------------------------------------------------------
//
void CHTTPFilterGBA::MHFLoad(RHTTPSession, THTTPFilterHandle)
{
	GBA_TRACE_DEBUG(("CHTTPFilterGBA::MHFLoad"));
	++iLoadCount;
}

//------------------------------------------------------------------------
// CHTTPFilterGBA::MHFUnload
//------------------------------------------------------------------------
//
void CHTTPFilterGBA::MHFUnload(RHTTPSession , THTTPFilterHandle)
{
	GBA_TRACE_DEBUG(("CHTTPFilterGBA::MHFUnload++"));
	
    __ASSERT_DEBUG( iLoadCount >= 0, PanicGBAHttpFilters() );
    
	if (--iLoadCount)
	{
		return;
	}
	delete this;
	GBA_TRACE_DEBUG(("CHTTPFilterGBA::MHFUnload--"));
}

//------------------------------------------------------------------------
// CHTTPFilterGBA::MHFRunL
// See MHTTPFilterBase::MHFRunL 
//------------------------------------------------------------------------
//
void CHTTPFilterGBA::MHFRunL(RHTTPTransaction aTransaction, const THTTPEvent& aEvent)
{
	GBA_TRACE_DEBUG(("CHTTPFilterGBA::MHFRunL++"));
	
	GBA_TRACE_DEBUG_NUM(("MHFRunL Event %d"), aEvent.iStatus );
	
	if (aEvent.iUID != KHTTPUid) 
		return;

	switch(aEvent.iStatus)
	{
		case THTTPEvent::ESubmit:
		{
			DoSubmitL(aTransaction);
		}
		break;

		case THTTPEvent::EGotResponseHeaders:
		{
			// Get HTTP status code from header (e.g. 200)
			RHTTPResponse resp = aTransaction.Response();
			TInt status = resp.StatusCode();
			GBA_TRACE_DEBUG_NUM(("MHFRunL staus code %d"), status );
			// Get status text (e.g. "OK")
			TBuf<KStatusTextLength512> statusText;
			statusText.Copy(resp.StatusText().DesC());
			
			GBA_TRACE_DEBUG(("status text:"));
			GBA_TRACE_DEBUG(statusText);
			CheckHeadersL( aTransaction );
		}
		break;
		case THTTPEvent::EGotResponseBodyData:
		{
            GBA_TRACE_DEBUG(("Event: EGotResponseBodyData"));
		}
		break;
		case THTTPEvent::EResponseComplete:
		{
            GBA_TRACE_DEBUG(("Event: EResponseComplete"));
			Cleanup( aTransaction );
		}
		break;
		case THTTPEvent::EFailed:
		{
            GBA_TRACE_DEBUG(("Event: EFailed"));			
			Cleanup( aTransaction );		
		}
		break;
		default: 
		{
            GBA_TRACE_DEBUG_NUM(("Unknow Event: ID - %d" ), aEvent.iStatus );
		}
		break;
	}
	GBA_TRACE_DEBUG(("CHTTPFilterGBA::MHFRunL--"));
}


//------------------------------------------------------------------------
// CHTTPFilterGBA::MHFRunError
// See MHTTPFilterBase::MHFRunError
//------------------------------------------------------------------------
//
TInt CHTTPFilterGBA::MHFRunError(TInt /*aError*/, RHTTPTransaction aTransaction, const THTTPEvent& )
{
    GBA_TRACE_DEBUG(("CHTTPFilterGBA::MHFRunError++"));
	TInt error = 0;
	// map aError to global error message
	// pass the errorcode forward
	THTTPEvent httpEvent(error);
	TRAP_IGNORE(aTransaction.SendEventL(httpEvent, THTTPEvent::EIncoming, THTTPFilterHandle::ECurrentFilter ));
	GBA_TRACE_DEBUG(("CHTTPFilterGBA::MHFRunError--"));
	return KErrNone;
}

//------------------------------------------------------------------------
// CHTTPFilterGBA::MHFSessionRunL
// See MHTTPFilterBase::MHFSessionRunL
//------------------------------------------------------------------------
//
void CHTTPFilterGBA::MHFSessionRunL(const THTTPSessionEvent& )
{
	// do nothing
}

//------------------------------------------------------------------------
// CHTTPFilterGBA::MHFSessionRunL
// See MHTTPFilterBase::MHFSessionRunL
//------------------------------------------------------------------------
//
TInt CHTTPFilterGBA::MHFSessionRunError(TInt aError, const THTTPSessionEvent& )
{
	GBA_TRACE_DEBUG(("CHTTPFilterGBA::MHFSessionRunError"));
	// session problem, need to close GBA engine
	return aError;
}

//------------------------------------------------------------------------
// CHTTPFilterGBA::DumpResponseHeadersL
// Dump the response headers to LOG file
//------------------------------------------------------------------------
//
#if defined (_DEBUG)
void CHTTPFilterGBA::DumpResponseHeadersL( RHTTPResponse& aResponse )
{
	GBA_TRACE_DEBUG("Dump the header...");

	RHTTPHeaders headers = aResponse.GetHeaderCollection();
	THTTPHdrFieldIter it = headers.Fields();
	
	while( it.AtEnd() == EFalse )
	{
        RStringTokenF fieldName = it();
        RStringF fieldNameStr = iStringPool.StringF (fieldName );
        THTTPHdrVal fieldVal;
        TInt fieldParts = 0; // For the case if next the call fails.
        TRAP_IGNORE( fieldParts =  headers.FieldPartsL( fieldNameStr ) );
		// dump the first part of  a  header field
		for ( TInt ii = 0; ii < fieldParts; ii++ )
		{	
			if( headers.GetField( fieldNameStr, ii, fieldVal ) == KErrNone )
			{
				const TDesC8& fieldNameDesC = fieldNameStr.DesC();
				GBA_TRACE_DEBUG(" : ");
                GBA_TRACE_DEBUG(fieldNameDesC);

				switch( fieldVal.Type() )
				{
				case THTTPHdrVal::KTIntVal:
					{
						GBA_TRACE_DEBUG_NUM(("%d"), fieldVal.Int() );
					}
					break;
				case THTTPHdrVal::KStrFVal:
					{
						RStringF fieldValStr = iStringPool.StringF( fieldVal.StrF() );
						const TDesC8& fieldValDesC = fieldValStr.DesC();
                        GBA_TRACE_DEBUG(fieldValDesC);
					}
					break;
				case THTTPHdrVal::KStrVal:
					{
						RString fieldValStr = iStringPool.String( fieldVal.Str() );
						const TDesC8& fieldValDesC = fieldValStr.DesC();
						GBA_TRACE_DEBUG(fieldValDesC);
					}
					break;
				case THTTPHdrVal::KDateVal:
					{
						TDateTime date = fieldVal.DateTime();
						TBuf<KStatusTextLength512> dateTimeString;
						TTime t( date );
						t.FormatL( dateTimeString, KDateFormat );
						TBuf8<KStatusTextLength512> dtStr;
						dtStr.Copy( dateTimeString.Left( 128 ) );
						GBA_TRACE_DEBUG(dtStr);
					}
					break;
				case THTTPHdrVal::KNoType:
				default:
					{
					GBA_TRACE_DEBUG("Unrecognized value type");
					}
					break;
				}
			}
		}

		++it;
	}
	GBA_TRACE_DEBUG("Header is Dumped already");
}
#endif

//------------------------------------------------------------------------
// CHTTPFilterGBA::CheckHeadersL
// Check HTTP headers and extract MIME type
//------------------------------------------------------------------------
// 
void CHTTPFilterGBA::CheckHeadersL(  RHTTPTransaction& aTrans )
{
	GBA_TRACE_DEBUG(("CHTTPFilterGBA::CheckHeadersL++"));
	// read the header data and check the MIME type here	
	// check the status and body
	RHTTPResponse response = aTrans.Response();
	TInt status = response.StatusCode();

	#if defined (_DEBUG)
	DumpResponseHeadersL( response );
	#endif

	THTTPHdrVal fieldVal;

	// check if status == 401 and realm is 3GPP then we need to bootstrap, if we are already bootstrappign this is the second call
	if(  status == HTTPStatus::EUnauthorized )
	{
	    TInt headerPart = FindHeaderPartToUseL(aTrans);
        if( headerPart == KErrNotFound )
            return;
    	
	    THTTPHdrVal headerVal;
        TInt cred = KErrNotFound;

	    if( aTrans.PropertySet().Property( iRealmStr, headerVal ) )
	    	cred = DFindCredentials( headerVal, aTrans.Request().URI() );
	    RStringF wwwAuthHeader = iStringPool.StringF(HTTP::EWWWAuthenticate,RHTTPSession::GetTable());
	    RHTTPHeaders headers(aTrans.Response().GetHeaderCollection());

	    RString realm;
	    THTTPHdrVal hdrVal;
	    if (!headers.GetParam(wwwAuthHeader, iStringPool.StringF(HTTP::ERealm,RHTTPSession::GetTable()), 
				  hdrVal, headerPart))
	    {
			realm = hdrVal;
			const TDesC8& val = realm.DesC();
			if(val.FindF(K3GPPRealmString) == 0)
			{
				GBA_TRACE_DEBUG(("3GPP-Bootstrapping realm detected...\n"));
	            THTTPHdrVal authTypeParam;
	            if ( headers.GetField( wwwAuthHeader, headerPart, authTypeParam ) == KErrNone )
	                // we need to store realm, opaque, nonce and qop values to
	                // use them when resubmitting the request
	                {
	                THTTPHdrVal algorithmVal;
	                THTTPHdrVal nonceVal;
	                THTTPHdrVal realmVal;
	                THTTPHdrVal opaqueVal;
	                THTTPHdrVal staleVal;
	                TInt algVal( EAlgUnknown );
	                TInt authVal( EQopNone );

	                aTrans.PropertySet().RemoveProperty( iNonceStr );
	                aTrans.PropertySet().RemoveProperty( iRealmStr );
	                aTrans.PropertySet().RemoveProperty( iOpaqueStr );
	                aTrans.PropertySet().RemoveProperty( iQopStr );
	                aTrans.PropertySet().RemoveProperty( iStaleStr );

	                if( !headers.GetParam( wwwAuthHeader, 
	                                      iStringPool.StringF( HTTP::EAlgorithm, 
	                                                      RHTTPSession::GetTable() ), 
	                                      algorithmVal, 
	                                      headerPart ) )
	                    {
	                    if( !algorithmVal.Str().DesC().CompareF( iMd5Str.DesC() ) )
	                        {
	                        algVal = EAlgMd5;
	                        }
	                    else if( !algorithmVal.Str().DesC().CompareF( iMd5SessStr.DesC() ) )
	                        {
	                        algVal = EAlgMd5Sess;
	                        }
	                    }

	                authVal = CheckQop( headers, wwwAuthHeader, headerPart );

	                headers.GetParam( wwwAuthHeader, iNonceStr, nonceVal, headerPart );
	                headers.GetParam( wwwAuthHeader, iRealmStr, realmVal, headerPart );
	                headers.GetParam( wwwAuthHeader, iOpaqueStr, opaqueVal, headerPart );
	                headers.GetParam( wwwAuthHeader, iStaleStr, staleVal, headerPart );

	                if( ( authVal & EQopAuth || authVal == EQopNone )
	                    && nonceVal.Type() == THTTPHdrVal::KStrVal
	                    && realmVal.Type() == THTTPHdrVal::KStrVal )
	                    {
	                    aTrans.PropertySet().SetPropertyL( iNonceStr, nonceVal );
	                    aTrans.PropertySet().SetPropertyL( iRealmStr, realmVal );
	                    // Opaque
	                    if( opaqueVal.Type() == THTTPHdrVal::KStrVal )
	                        {
	                        aTrans.PropertySet().SetPropertyL( iOpaqueStr, opaqueVal );
	                        }

	                    // Qop
	                    aTrans.PropertySet().SetPropertyL( iQopStr, authVal );
	                    // Algorithm
	                    aTrans.PropertySet().SetPropertyL( iStringPool.StringF( HTTP::EAlgorithm, 
	                                                              RHTTPSession::GetTable() ), 
	                                                              algVal );

	                    _LIT8( KTrue, "TRUE" );
	                    if( staleVal.Type() == THTTPHdrVal::KStrVal 
	                        && !staleVal.Str().DesC().CompareF( KTrue ) )
	                        // stale is true -> nonce expired
	                        {
                            GBA_TRACE_DEBUG(("Stale found"));
	                        // update nonce value
	                        TDCredentials& creds = iDCredentials[cred];
	                        iStringPool.String( creds.iNonce ).Close();
	                        creds.iNonce = nonceVal.Str().Copy();

	                        // indicate to engine that no uname/pwd dialog needed
	                        staleVal.SetInt( 1 );
	                        aTrans.PropertySet().SetPropertyL( iStaleStr, staleVal );
	                        }

	                    }
	                }
	 				RHTTPTransactionPropertySet propSet = aTrans.PropertySet();
	                THTTPHdrVal staleVal;

					// don't show the uname&pwd dialog
	                staleVal.SetInt( 1 );
	                propSet.SetPropertyL( iStaleStr, staleVal );
	                // if we are already bootstrapping results will be retrieved when request is resubmitted
	                GBA_TRACE_DEBUG(("Do Bootstrap\n"));
	                // start bootstrap process
	                if(!iGbaUtility)
	                    {
	                    iGbaUtility = CGbaUtility::NewL(*this);
	                    
	                    //setbsf address required only for testing purpose
	                    if(iBsfAddress.Size())
	                        {
                            iGbaUtility->SetBSFAddress(iBsfAddress);
	                        }    
	                    }
	                
	                // obtain NAF FQDN from URI
	                iGbaInputParams.iNAFName.Copy(aTrans.Request().URI().UriDes());
	                iGbaInputParams.iUICCLabel.Copy(iUICCLabel);
	                iGbaInputParams.iProtocolIdentifier.Copy(iProtocolIdentifier);
	                
	                //iBootstrapCount == 0  - soft bootstrapp - get cradentials from gba cache
	                //iBootstrapCount == 1  - force bootstrapp - get cradentials from BSF server
	                //iBootstrapCount > 1  need to cancel http transaction to break out of the UE-NAF communication loop
	                if(iBootstrapCount == 0){
	                iGbaInputParams.iFlags = EGBADefault;
                        GBA_TRACE_DEBUG(("iCredentials.Flags = EGBADefault"));
	                }
	                else if(iBootstrapCount == 1){
	                iGbaInputParams.iFlags = EGBAForce;
                        GBA_TRACE_DEBUG(("iCredentials.Flags = EGBAForce"));
	                }
	                else{
                        GBA_TRACE_DEBUG(("iCredentials.Flags"));
                        iBootstrapCount = 0;
                        return;
	                }
	                
	                //Default accesspoint will be used - iAPID = -1
	                iGbaInputParams.iAPID = -1;
                    
	                // fetch credentials from bootstrapper
	                iGbaUtility->Bootstrap(iGbaInputParams, iGbaOutputParams);
	                iBootstrapCount++;
	                iBootstrapPending = ETrue;
	                if(!iBootstrapWait.IsStarted())
	                    {
                        iBootstrapWait.Start();
	                    }
	                
	                if( iHaveGbaBootstrapCredentials )
	                    {
                        GBA_TRACE_DEBUG(("Have stored credentials"));
                        RHTTPTransactionPropertySet propSet = aTrans.PropertySet();
                        GBA_TRACE_DEBUG(("BTID:"));
                        GBA_TRACE_DEBUG(iGbaOutputParams.iBTID);
                        // Encodes the KNAF to generate a password
                        TImCodecB64 b64coder;
                        TBuf8<KB64KeySize> keyBase64;
                        b64coder.Initialise();
                        b64coder.Encode( iGbaOutputParams.iKNAF, keyBase64 );
                        RString username = iStringPool.OpenStringL( iGbaOutputParams.iBTID );
                        CleanupClosePushL<RString>( username );
                        RString password = iStringPool.OpenStringL( keyBase64 );
                        CleanupClosePushL<RString>( password );
                        propSet.SetPropertyL( iUsernameStr, username );
                        propSet.SetPropertyL( iPasswordStr, password );
                        CleanupStack::PopAndDestroy(&password);
                        CleanupStack::PopAndDestroy(&username);
                        //Cancel the http transaction
                        aTrans.Cancel();
                        // Re-submit the http request with much needed credentials
                        aTrans.SubmitL(); 
                        }
	                else
	                    {
                        //Since bootstrapping failed,Do not have to resubmit the http request ?
                        return;
                        }
			}
	    }
		}

	if( status == HTTPStatus::EOk && iGbaUtility){
		// if we have bootstrapped, it seems that we have succeeded
		iBootstrapCount = 0;
	}
   	GBA_TRACE_DEBUG(("CHTTPFilterGBA::CheckHeadersL--"));
	
}

void CHTTPFilterGBA::DoSubmitL( RHTTPTransaction aTransaction )
{
	GBA_TRACE_DEBUG(("CHTTPFilterGBA::DoSubmitL++"));
	
    // always add 3gpp-gba into user-agent field in headers,
    // THIS CAN BE REMOVED WHEN GBA IS ADDED INTO THE CATALOG OF THE DEVICE
    // WHICH IS STATIC.
	
	RHTTPHeaders hdr = aTransaction.Request().GetHeaderCollection();
	
	RStringF fieldname = aTransaction.Session().StringPool().StringF(HTTP::EUserAgent,RHTTPSession::GetTable());
	
	TBool found = EFalse;
	TPtrC8 rawFieldData;
	
	hdr.GetRawField(fieldname, rawFieldData);
	
	found = rawFieldData.Find(iUserAgent.DesC()) != KErrNotFound;
	
	//Append "3gpp-gba" string to the "user agent" transaction header field only when,
	//http transaction user agent header field has been set already(rawFieldData.Size() > 0) by an application,
	//which opened the http transaction and also
	//the transaction user agent header field value does not have "3gpp-gba" string in it.
	
	if(!found && rawFieldData.Size())
	    hdr.SetFieldL(fieldname, iUserAgent);
	
    TInt cred = KErrNotFound;
    
    //iHaveGbaBootstrapCredentials - ETrue ,when the GBA bootstrapping is completed successfully
    if( iHaveGbaBootstrapCredentials )
        {
        // Get credentials from URI, only for wwwAuthenticate
        // If there are credentials in the transaction properties, add them to the list
        DGetCredentialsFromPropertiesL( aTransaction );
    
        // just in case we always remove stale value from properties
        aTransaction.PropertySet().RemoveProperty( iStaleStr );
    
        // If we have credentials for this URL, add them
        cred = DFindCredentialsForURI( aTransaction.Request().URI() );
    
        if ( cred != KErrNotFound )
            {
            EncodeDigestAuthL( cred, aTransaction );
            }
        }
    GBA_TRACE_DEBUG(("CHTTPFilterGBA::DoSubmitL--"));
}

//-----------------------------------------------------------------------------
// CHTTPFilterGBA::Cleanup
// Cleanup the resource related with a transaction
//-----------------------------------------------------------------------------
//
void CHTTPFilterGBA::Cleanup( const RHTTPTransaction& /*aTrans*/ )
{
}

//-----------------------------------------------------------------------------
// CHTTPFilterGBA::CleanupAll
// Cleanup all the GBA transactions, in case a session error happens or a session
// is closed.
//-----------------------------------------------------------------------------
//
void CHTTPFilterGBA::CleanupAll()
{	
	GBA_TRACE_DEBUG(("CHTTPFilterGBA::CleanupAll++"));
	while(TInt i = iDCredentials.Count())
	{
		DRemoveCredentialsFromList(i-1);
	} 

	iQopAuthStr.Close();
	iMd5SessStr.Close();
	iMd5Str.Close();
	iAuthInfo.Close();
	iUserAgent.Close();
    delete iMD5Calculator;
    iMD5Calculator = NULL;
    delete 	iGbaUtility;
    iGbaUtility = NULL;
    GBA_TRACE_DEBUG(("CHTTPFilterGBA::CleanupAll--"));
}


TInt CHTTPFilterGBA::FindHeaderPartToUseL(RHTTPTransaction aTransaction) const
{
    // There may be several different authentication fields. We need
    // to chose the strongest one we understnad. Currently, we only
    // support 2, Basic and Digest, and Digest is the strongest
    // assuming there is a qop that we can accept. Therefore, we keep
    // track of the last good basic one we found, and return the
    // moment we find an acceptable digest one.

    // While we're here, we check that all desired fields are there.
	GBA_TRACE_DEBUG(("CHTTPFilterGBA::FindHeaderPartToUseL++"));
    RStringF wwwAuthenticate = iStringPool.StringF(HTTP::EWWWAuthenticate,RHTTPSession::GetTable());
    RStringF realm = iStringPool.StringF(HTTP::ERealm,RHTTPSession::GetTable());
    RStringF nonce = iStringPool.StringF(HTTP::ENonce,RHTTPSession::GetTable());
    RStringF qop   = iStringPool.StringF(HTTP::EQop,RHTTPSession::GetTable());
    TInt lastGoodBasic = KErrNotFound;
    RHTTPHeaders headers = aTransaction.Response().GetHeaderCollection();
    const TInt parts = headers.FieldPartsL(wwwAuthenticate);
    GBA_TRACE_DEBUG_NUM((" FindHeaderPartToUseL part no:1 = %d"), parts );
    for (TInt ii = 0; ii < parts; ii++)
    {
	THTTPHdrVal fieldVal;// The name of the current field.
	THTTPHdrVal hdrVal;//A scratch hdrVal
	TInt error = headers.GetField(wwwAuthenticate, ii, fieldVal);
	if( error != KErrNone )
	    return lastGoodBasic;
	TInt x = fieldVal.StrF().Index(RHTTPSession::GetTable());
	GBA_TRACE_DEBUG_NUM((" FindHeaderPartToUseL part no:1 = %d"), x );
	switch (fieldVal.StrF().Index(RHTTPSession::GetTable()))
	{
	    case HTTP::EDigest:
	    	
	    	{
	    	GBA_TRACE_DEBUG_NUM((" FindHeaderPartToUseL realm  = %d"), headers.GetParam(wwwAuthenticate, realm, hdrVal, ii) );
	    	
	    	GBA_TRACE_DEBUG_NUM((" FindHeaderPartToUseL nonce  = %d"), headers.GetParam(wwwAuthenticate, nonce, hdrVal, ii) );
	    
			if (headers.GetParam(wwwAuthenticate, realm, hdrVal, ii) == 
			    KErrNone &&
			    headers.GetParam(wwwAuthenticate, nonce, hdrVal, ii) == 
			    KErrNone)
			{
				GBA_TRACE_DEBUG_NUM((" FindHeaderPartToUseL qop  = %d"), headers.GetParam(wwwAuthenticate, qop, hdrVal, ii) );
				
			    // We've got a realm and a nonce. If there's a qop, we
			    // need to check it's acceptable.
			    if (headers.GetParam(wwwAuthenticate, qop, hdrVal, ii) != 
				KErrNone || FindAuth(hdrVal.Str().DesC()))
			    	{
			    	GBA_TRACE_DEBUG_NUM((" FindHeaderPartToUseL part no = %d"), ii );
			    	return ii;
			    	}
				
			}
	    	}
		break;

	    default:
		// We don't understand this, whatever it is. Ignore it.
		break;
	}
    }
    GBA_TRACE_DEBUG(("CHTTPFilterGBA::FindHeaderPartToUseL--"));
    return lastGoodBasic;
}


TBool CHTTPFilterGBA::FindAuth(const TDesC8& aQop) const
{
	GBA_TRACE_DEBUG(("CHTTPFilterGBA::FindAuth++"));
    TPtrC8 tokens = aQop;
    TPtrC8 token;
    const TDesC8& auth = iStringPool.StringF(HTTP::EAuth,RHTTPSession::GetTable()).DesC();
    TInt len;
    do
    {
    	len = tokens.Locate(',');
    	if(len == -1)
    		len = tokens.Length();
    	token.Set(tokens.Ptr(),len);
		if (token.Compare(auth) == 0)
		{
		    return ETrue;
    	}
    	tokens.Set(tokens.Ptr()+len);
    }  while(tokens.Length());
    GBA_TRACE_DEBUG(("CHTTPFilterGBA::FindAuth--"));
    return EFalse;
}



void CHTTPFilterGBA::EncodeDigestAuthL( TInt aCred, RHTTPTransaction aTransaction )
    {
    GBA_TRACE_DEBUG(("CHTTPFilterGBA::EncodeDigestAuthL++"));

	// For now, only support RFC2069 format. The digest is
	// MD5(H(A1):nonce:H(A2))
	TBuf8<KHashLength> hash;
    TBuf8<KNonceCountLength> nonceCount;

    TDCredentials cred = iDCredentials[aCred];
    

    THTTPHdrVal nonce( iStringPool.String( cred.iNonce ) );
    THTTPHdrVal realm( iStringPool.String( cred.iRealm ) );
    THTTPHdrVal qop;
    THTTPHdrVal uname( iStringPool.String( cred.iUser ) );
    THTTPHdrVal password( iStringPool.String( cred.iPassword ) );
    THTTPHdrVal opaque( iStringPool.String( cred.iOpaque ) );

    if( cred.iQop & EQopAuth )
        // recent release supports only qop 'auth'.
        {
        qop.SetStrF( iQopAuthStr );
        }

	THTTPHdrVal requestUri;
    // we get url from request and modify as made in chttpclienthandler.cpp
    RString uriStr = RequestUriL( aTransaction );
    CleanupClosePushL<RString>( uriStr );

	requestUri.SetStr( uriStr );
	
	HBufC8* stringToHash = NULL;
	RStringF auth = iStringPool.StringF(HTTP::EAuth,RHTTPSession::GetTable());

	TBuf8<KHashLength> cnonce;
	if ( cred.iQop != EQopNone )
		{
        GenerateCNonce( cnonce );

        // 7 is for colons
        // 8 for nc-value
        stringToHash = HBufC8::NewMaxLC( KHashLength +  // H(A1)
                                         1 +    // ":"
                                         nonce.Str().DesC().Length() + 
                                         1 +    // ":"
                                         KIntegerConstant8 +    // nc value
                                         1 +    // ":"
                                         KHashLength +  // cnonce
                                         1 +    // ":"
										 qop.StrF().DesC().Length() +
                                         1 +    // ":"
										 KHashLength ); // H(A2)
		TPtr8 stringMod = stringToHash->Des();
		stringMod.Zero();
		HAOneL( cred.iAlgorithm, uname, password, realm.Str(), nonce, cnonce, stringMod );
		stringMod.Append(':');
		stringMod.Append(nonce.Str().DesC());

        // nc value
		stringMod.Append(':');
        ++cred.iNc;
        // nc value is of 8 chars length and in hexadecimal format
        nonceCount.NumFixedWidth( cred.iNc, EHex, KIntegerConstant8 );
		stringMod.Append( nonceCount );
		stringMod.Append(':');

		stringMod.Append(cnonce);
		stringMod.Append(':');
		stringMod.Append(auth.DesC());
		stringMod.Append(':');
		HATwoL(aTransaction.Request().Method(), requestUri.Str(), hash);
		stringMod.Append(hash);
		}
	else
		{
		// The response is the hash of H(A1):nonce:H(A2).
		// Crqeate a buffer for the string to convert into MD5. The
		// length is 32 characters for each of the hashes, 2
		// characters for the 2 colons, plus the length of the nonce
		stringToHash = HBufC8::NewMaxLC(nonce.Str().DesC().Length() + 
												 2 * KHashLength + 2);
		TPtr8 stringMod = stringToHash->Des();
		stringMod.Zero();
		HAOneL( cred.iAlgorithm, uname, password, realm.Str(), nonce, cnonce, stringMod );
		stringMod.Append(':');
		stringMod.Append(nonce.Str().DesC());
		stringMod.Append(':');
		HATwoL(aTransaction.Request().Method(), requestUri.Str(), hash);
		stringMod.Append(hash);
		}
	
	Hash(*stringToHash, hash);
	CleanupStack::PopAndDestroy(stringToHash);

	// OK. hash now contains the digest response. Set up the header.
	RHTTPHeaders requestHeaders(aTransaction.Request().GetHeaderCollection());
	
	requestHeaders.RemoveField(iStringPool.StringF(HTTP::EAuthorization,RHTTPSession::GetTable())); 

	requestHeaders.SetFieldL(iStringPool.StringF(HTTP::EAuthorization,RHTTPSession::GetTable()), 
							 THTTPHdrVal(iStringPool.StringF(HTTP::EDigest,RHTTPSession::GetTable())),
							 iStringPool.StringF(HTTP::EUsername,RHTTPSession::GetTable()),
							 uname);
	requestHeaders.SetFieldL(iStringPool.StringF(HTTP::EAuthorization,RHTTPSession::GetTable()), 
							 THTTPHdrVal(iStringPool.StringF(HTTP::EDigest,RHTTPSession::GetTable())),
							 iStringPool.StringF(HTTP::ERealm,RHTTPSession::GetTable()),
							 realm);
	requestHeaders.SetFieldL(iStringPool.StringF(HTTP::EAuthorization,RHTTPSession::GetTable()), 
							 THTTPHdrVal(iStringPool.StringF(HTTP::EDigest,RHTTPSession::GetTable())),
							 iStringPool.StringF(HTTP::ENonce,RHTTPSession::GetTable()),
							 nonce);
	RString hashStr = iStringPool.OpenStringL(hash);
	CleanupClosePushL<RString>(hashStr);

	THTTPHdrVal hdrVal;
    hdrVal.SetStr(hashStr);
	requestHeaders.SetFieldL(iStringPool.StringF(HTTP::EAuthorization,RHTTPSession::GetTable()), 
							 THTTPHdrVal(iStringPool.StringF(HTTP::EDigest,RHTTPSession::GetTable())),
							 iStringPool.StringF(HTTP::EResponse,RHTTPSession::GetTable()),
							 hdrVal);

	requestHeaders.SetFieldL(iStringPool.StringF(HTTP::EAuthorization,RHTTPSession::GetTable()), 
							 THTTPHdrVal(iStringPool.StringF(HTTP::EDigest,RHTTPSession::GetTable())),
							 iStringPool.StringF(HTTP::EUri,RHTTPSession::GetTable()),
							 requestUri);
	CleanupStack::PopAndDestroy(&hashStr);
	if ( cred.iQop != EQopNone )
		{
        // QOP
		hdrVal.SetStrF( auth );
		requestHeaders.SetFieldL(iStringPool.StringF(HTTP::EAuthorization,RHTTPSession::GetTable()), 
								 THTTPHdrVal(iStringPool.StringF(HTTP::EDigest,RHTTPSession::GetTable())),
								 iStringPool.StringF(HTTP::EQop,RHTTPSession::GetTable()),
								 hdrVal);
		RString cnonceString = iStringPool.OpenStringL(cnonce);
        // CNonce
		CleanupClosePushL<RString>( cnonceString );
		hdrVal.SetStr(cnonceString);
		requestHeaders.SetFieldL(iStringPool.StringF(HTTP::EAuthorization,RHTTPSession::GetTable()), 
								 THTTPHdrVal(iStringPool.StringF(HTTP::EDigest,RHTTPSession::GetTable())),
								 iStringPool.StringF(HTTP::ECnonce,RHTTPSession::GetTable()),
								 hdrVal);
		CleanupStack::PopAndDestroy( &cnonceString );
        // Nonce-count
        RString nonceStr = iStringPool.OpenStringL( nonceCount );
        CleanupClosePushL<RString>( nonceStr );
		hdrVal.SetStr( nonceStr );
		requestHeaders.SetFieldL(iStringPool.StringF(HTTP::EAuthorization,RHTTPSession::GetTable()), 
								 THTTPHdrVal(iStringPool.StringF(HTTP::EDigest,RHTTPSession::GetTable())),
								 iStringPool.StringF(HTTP::ENc,RHTTPSession::GetTable()),
								 hdrVal);
        CleanupStack::PopAndDestroy( &nonceStr );
		}
	
	if(  opaque.Str().DesC().Length() )
		requestHeaders.SetFieldL(iStringPool.StringF(HTTP::EAuthorization,RHTTPSession::GetTable()), 
								 THTTPHdrVal(iStringPool.StringF(HTTP::EDigest,RHTTPSession::GetTable())),
								 iStringPool.StringF(HTTP::EOpaque,RHTTPSession::GetTable()),
								 opaque );	

    if( cred.iAlgorithm == EAlgMd5 )
        {
		requestHeaders.SetFieldL(iStringPool.StringF(HTTP::EAuthorization,RHTTPSession::GetTable()), 
								 THTTPHdrVal(iStringPool.StringF(HTTP::EDigest,RHTTPSession::GetTable())),
								 iStringPool.StringF(HTTP::EAlgorithm,RHTTPSession::GetTable()),
								 iMd5Str );
        }
    else if( cred.iAlgorithm == EAlgMd5Sess )
        {
		requestHeaders.SetFieldL(iStringPool.StringF(HTTP::EAuthorization,RHTTPSession::GetTable()), 
								 THTTPHdrVal(iStringPool.StringF(HTTP::EDigest,RHTTPSession::GetTable())),
								 iStringPool.StringF(HTTP::EAlgorithm,RHTTPSession::GetTable()),
								 iMd5SessStr );
        }

    // set it again
    // this may be needed if auth. fails and stale is true.
    aTransaction.PropertySet().SetPropertyL( iRealmStr, realm );
    
    CleanupStack::PopAndDestroy( &uriStr );
    GBA_TRACE_DEBUG(("CHTTPFilterGBA::EncodeDigestAuthL--"));
    }
  
void CHTTPFilterGBA::DAddCredentialsToListL( RString aUsernameStr,
        RString aPasswordStr,
        RString aRealmStr,
        RStringF aUriStr,
        RString aOpaque,
        RString aNonce,
        TInt aQop,
        TInt aAuthAlg )
    {
    GBA_TRACE_DEBUG(("CHTTPFilterGBA::DAddCredentialsToListL++"));
    GBA_TRACE_DEBUG("uname: ");
    GBA_TRACE_DEBUG(aUsernameStr.DesC());
    GBA_TRACE_DEBUG("pwd: ");
    GBA_TRACE_DEBUG(aPasswordStr.DesC());
    GBA_TRACE_DEBUG("realm: ");
    GBA_TRACE_DEBUG(aRealmStr.DesC());
    GBA_TRACE_DEBUG("uri: ");
    GBA_TRACE_DEBUG(aUriStr.DesC());
    GBA_TRACE_DEBUG_NUM(("qop: %d"), aQop );
    GBA_TRACE_DEBUG_NUM(("alg: %d"), aAuthAlg );

    TDCredentials newCred;
    newCred.iUser = aUsernameStr;
    newCred.iPassword = aPasswordStr;
    newCred.iRealm = aRealmStr;
    newCred.iURI = aUriStr;
    newCred.iOpaque = aOpaque;
    newCred.iNonce = aNonce;
    newCred.iQop = aQop;
    newCred.iAlgorithm = aAuthAlg;
    newCred.iNc = 0;
    User::LeaveIfError( iDCredentials.Append( newCred ) );
    GBA_TRACE_DEBUG(("CHTTPFilterGBA::DAddCredentialsToListL--"));
    }

// -----------------------------------------------------------------------------
// CHTTPFilterGBA::HAOneL
// Calculates H(A1)
// -----------------------------------------------------------------------------
//
void CHTTPFilterGBA::HAOneL(int aAlgorithm,
                                           const RString& aUsername,
                                           const RString& aPW,
								           const RString& aRealm, 
                                           const RString& aNonceValue,
                                           TDesC8& aCNonce,
                                           TDes8& aResult )
	{
	GBA_TRACE_DEBUG(("CHTTPFilterGBA::HAOneL++"));
    TBuf8<KHashLength> hash;

	HBufC8* a1 = HBufC8::NewMaxL(aUsername.DesC().Length() + 
								 aPW.DesC().Length() + 
								 aRealm.DesC().Length() + 2 );
	TPtr8 a1Mod = a1->Des();
	a1Mod.Zero();
	a1Mod.Append( aUsername.DesC() );
	a1Mod.Append( KColon );
	a1Mod.Append( aRealm.DesC() );
	a1Mod.Append( KColon );
	a1Mod.Append( aPW.DesC() );

	GBA_TRACE_DEBUG("a1 first hash material");
	GBA_TRACE_DEBUG(*a1);
	

	Hash( *a1, hash );
	
	GBA_TRACE_DEBUG("a1 first hash");
	GBA_TRACE_DEBUG(hash);

    delete a1;
    a1 = NULL;

    if( aAlgorithm == EAlgMd5Sess )
        {
	    a1 = HBufC8::NewMaxL( KHashLength +
                               1 +  // ":"
                               aNonceValue.DesC().Length() +
                               1 +  // ":"
							   aCNonce.Length() );
	    TPtr8 a1Mod = a1->Des();
	    a1Mod.Zero();
        a1Mod.Append( hash );
        a1Mod.Append( KColon );
        a1Mod.Append( aNonceValue.DesC() );
        a1Mod.Append( KColon );
        a1Mod.Append( aCNonce );

        GBA_TRACE_DEBUG("a1 final hash material");
        GBA_TRACE_DEBUG(*a1);

	    Hash(*a1, aResult);
	    
        GBA_TRACE_DEBUG("a1 final hash");
        GBA_TRACE_DEBUG(hash);
	    delete a1;
        a1 = NULL;
        }
    else
        {
        aResult.Copy( hash );
        }
    GBA_TRACE_DEBUG(("CHTTPFilterGBA::HAOneL--"));
	}


// -----------------------------------------------------------------------------
// CHTTPFilterGBA::HAtwoL
// Calculates H(A2)
// -----------------------------------------------------------------------------
//
void CHTTPFilterGBA::HATwoL(const RStringF& aMethod, 
								           const RString& aRequestUri,
								           TDes8& aResult)
	{
	GBA_TRACE_DEBUG(("CHTTPFilterGBA::HAtwoL++"));
	// In the auth qop, a2 is Method:digest-uri-value
	// Digest-uri-value. We don't support auth-int qop
	// Allocate enough space for the method, the URI and the colon.
	TPtrC8 requestUri = aRequestUri.DesC();
	TPtrC8 method = aMethod.DesC();
	HBufC8* a2 = HBufC8::NewMaxLC(requestUri.Length() + method.Length() + 1);
	TPtr8 a2Mod = a2->Des();
	a2Mod.Zero();
	a2Mod.Append(method);
	a2Mod.Append(':');
	a2Mod.Append(requestUri);

    GBA_TRACE_DEBUG("a2 final hash material");
    GBA_TRACE_DEBUG(*a2);

	Hash(*a2, aResult);

    GBA_TRACE_DEBUG("a2 final hash");
    GBA_TRACE_DEBUG(aResult);

	CleanupStack::PopAndDestroy(a2);
	GBA_TRACE_DEBUG(("CHTTPFilterGBA::HAtwoL--"));
	}

// -----------------------------------------------------------------------------
// CHTTPFilterGBA::Hash
// Calculates hash value
// -----------------------------------------------------------------------------
//
void CHTTPFilterGBA::Hash(const TDesC8& aMessage, TDes8& aHash)
	{
	GBA_TRACE_DEBUG(("CHTTPFilterGBA::Hash++"));
	// Calculate the 128 bit (16 byte) hash
	iMD5Calculator->Reset();
	TPtrC8 hash = iMD5Calculator->Hash( aMessage );
	// Now print it as a 32 byte hex number
	aHash.Zero();
	_LIT8(formatStr, "%02x");
	for (TInt ii = 0; ii < KRawHashLength; ii++)
		{
		TBuf8<2> scratch;
		scratch.Zero();
		scratch.Format( formatStr, hash[ii] );
		aHash.Append( scratch );
		}
	GBA_TRACE_DEBUG(("CHTTPFilterGBA::Hash--"));
	}

// -----------------------------------------------------------------------------
// CHTTPFilterGBA::GenerateCNonce
// Generate unique cnonce value.
// -----------------------------------------------------------------------------
//
void CHTTPFilterGBA::GenerateCNonce(TDes8& aNonce)
	{
	GBA_TRACE_DEBUG(("CHTTPFilterGBA::GenerateCNonce++"));
	// 'Inspired by' CObexAuthenticator

	// The purpose of the client nonce is to protect against 'chosen
	// plaintext' attacks where a hostile server tricks the client
	// into supplying a password using a specific server nonce that
	// allows an (as yet undiscovered) flaw in MD5 to recover the
	// password. As such the only important thing about client nonces
	// is that they change and aren't predictable. See section 4.9 of
	// RFC2616

	TTime time;
	time.UniversalTime();
	TInt64 randomNumber = Math::Rand(iSeed);
	randomNumber <<= KIntegerConstant32;
	randomNumber += Math::Rand(iSeed);
	TBuf8<KIntegerConstant33> key;
	key.Zero();
	key.AppendNum(time.Int64(), EHex);
	key.Append(_L8(":"));
	key.AppendNum(randomNumber, EHex);
	
	Hash(key, aNonce);
	GBA_TRACE_DEBUG(("CHTTPFilterGBA::GenerateCNonce--"));
	}

// -----------------------------------------------------------------------------
// CHTTPFilterGBA::RequestUriLC
// Returns requested URI in form that can be used in uri field.
// This code comes from 
// Application-protocols/http/httpclient/chttpclienthandler.cpp.
// -----------------------------------------------------------------------------
//
RString CHTTPFilterGBA::RequestUriL( RHTTPTransaction& aTransaction )
    {
    GBA_TRACE_DEBUG(("CHTTPFilterGBA::RequestUriL++"));
    TBool useProxy( EFalse );
    THTTPHdrVal useProxyHdr;
    RStringF proxyUsage = iStringPool.StringF( HTTP::EProxyUsage, RHTTPSession::GetTable() );
    RHTTPRequest request = aTransaction.Request();
    const TUriC8& uri = request.URI();
    RStringF scheme = iStringPool.OpenFStringL( uri.Extract( EUriScheme ) );
    CleanupClosePushL<RStringF>( scheme );
    CUri8* uriToUse = CUri8::NewLC( uri );

    if( aTransaction.Session().ConnectionInfo().Property( proxyUsage, useProxyHdr ) )
        {
	    // Is a proxy being used?
        useProxy = ( useProxyHdr.StrF().Index(RHTTPSession::GetTable()) == HTTP::EUseProxy );
        }

	if( !useProxy || (useProxy && scheme.Index(RHTTPSession::GetTable()) == HTTP::EHTTPS ))
		{
		// Not going via a proxy - need to remove the scheme and authority parts
		uriToUse->RemoveComponentL( EUriScheme );
		uriToUse->RemoveComponentL( EUriHost );	// this also removes the userinfo + port
		}

    RString uriStr = iStringPool.OpenStringL( uriToUse->Uri().UriDes()  );

    CleanupStack::PopAndDestroy( uriToUse );
    CleanupStack::PopAndDestroy( &scheme );
    GBA_TRACE_DEBUG(("CHTTPFilterGBA::RequestUriL--"));
    return uriStr;    
    }

// -----------------------------------------------------------------------------
// CHTTPFilterGBA::CheckQop
//
// -----------------------------------------------------------------------------
//
TInt CHTTPFilterGBA::CheckQop( RHTTPHeaders& aHeaders,
                                                RStringF& aWwwAuthHeader,
                                                TInt aHeaderPart )
    {
    GBA_TRACE_DEBUG(("CHTTPFilterGBA::CheckQop++"));
    THTTPHdrVal qopVal;
    TInt retVal( EQopNone );
    
    if( !aHeaders.GetParam( aWwwAuthHeader, iQopStr, qopVal, aHeaderPart ) )
        {
        TPtrC8 qopBuf( qopVal.Str().DesC() );
        TPtrC8 qopValue;
        TInt comma;

        do
            {
            comma = qopBuf.Locate( ',' );
            
            if( comma != KErrNotFound )
                {
                qopValue.Set( qopBuf.Left(comma) );
                }
            else
                {
                qopValue.Set( qopBuf );
                }

            if( !qopValue.CompareF(iQopAuthStr.DesC()) )
                {
                retVal |= EQopAuth;
                }
            else if( !qopValue.CompareF(KQopAuthIntStr) )
                {
                retVal |= EQopAuthInt;
                }
            
            qopBuf.Set( qopBuf.Right(qopBuf.Length()-comma-1));
            }while( comma != KErrNotFound );
        }
    GBA_TRACE_DEBUG(("CHTTPFilterGBA::CheckQop--"));    
    return retVal;
    }

void CHTTPFilterGBA::DGetCredentialsFromPropertiesL( RHTTPTransaction& aTransaction )
    {
    GBA_TRACE_DEBUG(("CHTTPFilterGBA::DGetCredentialsFromPropertiesL++"));
    THTTPHdrVal usernameVal;
    THTTPHdrVal passwordVal;
    THTTPHdrVal realmVal;
    THTTPHdrVal nonceVal;
    THTTPHdrVal staleVal;
    const TUriC8& uri = aTransaction.Request().URI();
    RHTTPTransactionPropertySet propSet = aTransaction.PropertySet();
    TBool staleAvail = propSet.Property( iStaleStr, staleVal );
    
    
    if ( propSet.Property( iUsernameStr, usernameVal ) &&
         propSet.Property( iPasswordStr, passwordVal ) &&
         propSet.Property( iRealmStr, realmVal ) && 
         propSet.Property( iNonceStr, nonceVal )
          )
        // if stale is TRUE we don't need to update credential
        {
        

        TInt pushCounter( 0 );
        TInt cred = DFindCredentialsForURI( uri );
        if ( cred != KErrNotFound )
            {
            // Remove old credentials from the list
            DRemoveCredentialsFromList( cred );
            }

        TPtrC8 uriPathPtr( uri.UriDes() );

        if ( uriPathPtr.LocateReverse( '/' ) > 0 )
            {
            uriPathPtr.Set( uriPathPtr.Left( uriPathPtr.LocateReverse( '/' ) ) );
            }

        RStringF uriStr = iStringPool.OpenFStringL( uriPathPtr );
        CleanupClosePushL( uriStr );
        ++pushCounter;

        THTTPHdrVal opaqueVal;

        RString opaqueStr;
        
        if( !propSet.Property( iOpaqueStr, opaqueVal ) )
            // this isn't a error case
            {    
            opaqueStr = iStringPool.OpenStringL( KNullDesC8 );
            CleanupClosePushL<RString>( opaqueStr );
            ++pushCounter;
            }
        else
            {
            opaqueStr = opaqueVal;
            }
       
        THTTPHdrVal qopVal;

        if( !propSet.Property( iQopStr, qopVal ) )
            // this isn't a error case
            {
            qopVal.SetInt( EQopNone );
            }

      
        THTTPHdrVal algVal;
        if( !propSet.Property( iStringPool.StringF( HTTP::EAlgorithm, 
                                           RHTTPSession::GetTable() ), algVal) )
            {
            algVal.SetInt( EAlgUnknown );
            }
        // Add credentials to the list from the transaction's properties
       DAddCredentialsToListL( usernameVal.Str().Copy(), 
                                passwordVal.Str().Copy(),
                                realmVal.Str().Copy(), 
                                uriStr.Copy(), 
                                opaqueStr.Copy(), 
                                nonceVal.Str().Copy(),
                                qopVal.Int(),
                                algVal.Int()
                                );
        // this is not the traditional way to pop and destroy elements
        // but in this case this is the simplest one.
        CleanupStack::PopAndDestroy( pushCounter ); // [opaqueVal,]uriStr
        }
  
    GBA_TRACE_DEBUG(("CHTTPFilterGBA::DGetCredentialsFromPropertiesL--"));
    }


TInt CHTTPFilterGBA::DFindCredentialsForURI( const TUriC8& aURI )
    {
    GBA_TRACE_DEBUG(("CHTTPFilterGBA::DFindCredentialsForURI++"));
    // Check if any of the stored URIs are the beginning of the URI
    // we're trying now. If they are, we can immediately attempt to
    // re-use the existing credentials.
    _LIT8 ( KSchemeEnd, "://" );
    TInt count = iDCredentials.Count();

    for ( TInt cred = 0; cred < count; ++cred )
        {
        const TPtrC8& transDes = aURI.UriDes();
        const TPtrC8& credDes =
            iStringPool.StringF( iDCredentials[ cred ].iURI ).DesC();
        TPtrC8 transNoSchemePtr( transDes );
        TPtrC8 credNoSchemePtr( credDes );

        if ( transNoSchemePtr.Find( KSchemeEnd() ) > 0 )
            {
            transNoSchemePtr.Set( transNoSchemePtr.Mid( transNoSchemePtr.Find( KSchemeEnd ) ) );
            }

        if ( credNoSchemePtr.Find( KSchemeEnd() ) > 0 )
            {
            credNoSchemePtr.Set( credNoSchemePtr.Mid( credNoSchemePtr.Find( KSchemeEnd ) ) );
            }

        if ( transNoSchemePtr.Length() >= credNoSchemePtr.Length() )
            {
            // The URI is long enough, which is a good start.

            if ( transNoSchemePtr.Left( credNoSchemePtr.Length() ).Compare( credNoSchemePtr ) == 0 )
                {
                // The descriptors match. Things are looking good. In
                // the interests of paranoia, if we haven't matched
                // the entire URI, check that the character after the
                // match is '/'.
                if ( transNoSchemePtr.Length() == credNoSchemePtr.Length() )
                    {
                    GBA_TRACE_DEBUG_NUM("Found cred(1): %d", cred ); 
                    GBA_TRACE_DEBUG("uri: ");
                    GBA_TRACE_DEBUG(aURI.UriDes());
                    return cred;
                    }

                else if ( transNoSchemePtr[ credNoSchemePtr.Length() ] == '/' )
                    {
                    GBA_TRACE_DEBUG_NUM("Found cred(2): %d", cred ); 
                    GBA_TRACE_DEBUG("uri: ");
                    GBA_TRACE_DEBUG(aURI.UriDes());
                    return cred;
                    }
                }
            }
        }
    GBA_TRACE_DEBUG(("CHTTPFilterGBA::DFindCredentialsForURI--"));
    return KErrNotFound;
    }


void CHTTPFilterGBA::DRemoveCredentialsFromList( TInt aCredId )
    {
    GBA_TRACE_DEBUG(("CHTTPFilterGBA::DRemoveCredentialsFromList++"));
    GBA_TRACE_DEBUG_NUM("DRemoveCredentialsFromList: %d", aCredId );

    TDCredentials& creds = iDCredentials[ aCredId ];
    iStringPool.String( creds.iUser ).Close();
    iStringPool.String( creds.iPassword ).Close();
    iStringPool.StringF( creds.iURI ).Close();
    iStringPool.String( creds.iRealm ).Close();
    iStringPool.String( creds.iOpaque ).Close();
    iStringPool.String( creds.iNonce ).Close();
    iDCredentials.Remove( aCredId );
    iDCredentials.Close();
    GBA_TRACE_DEBUG(("CHTTPFilterGBA::DRemoveCredentialsFromList--"));
    }

TInt CHTTPFilterGBA::DFindCredentials( const RString& aRealm,
        const TUriC8& aURI )
    {
    GBA_TRACE_DEBUG(("CHTTPFilterGBA::DFindCredentials++"));
    TInt count = iDCredentials.Count();

    for ( TInt cred = 0; cred < count; ++cred )
        {
        if ( iDCredentials[ cred ].iRealm == aRealm )
            {
            TUriParser8 parser;
            parser.Parse( iStringPool.StringF( iDCredentials[ cred ].iURI ).DesC() );

            if ( !parser.Compare( aURI, EUriHost ) &&
                 ( ( !parser.Compare( aURI, EUriPort ) )
                   || ( !parser.IsPresent( EUriPort ) &&
                        !aURI.IsPresent( EUriPort ) ) ) )
                {                
                GBA_TRACE_DEBUG_NUM("Found cred(3): %d", cred );
                return cred;
                }
            }
        }
    GBA_TRACE_DEBUG(("CHTTPFilterGBA::DFindCredentials--"));
    return KErrNotFound;
    }


void CHTTPFilterGBA::BootstrapComplete( TInt aError )
{
    GBA_TRACE_DEBUG(("CHTTPFilterGBA::BootstrapComplete--"));
    if( aError == KErrNone)
        {
        iBootstrapPending = EFalse;
        iHaveGbaBootstrapCredentials = ETrue;
        }
    else
        {
        iBootstrapPending = EFalse;
        iHaveGbaBootstrapCredentials = EFalse;
        }
    iBootstrapWait.AsyncStop();
}

//EOF

