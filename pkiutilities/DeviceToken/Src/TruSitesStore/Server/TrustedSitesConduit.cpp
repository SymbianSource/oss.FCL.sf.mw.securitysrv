/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Implementation of TrustedSitesStoreConduit
*
*/



#include "TrustedSitesConduit.h"
#include "TrustedSitesServer.h"
#include "DevTokenMarshaller.h"
#include "DevTokenUtil.h"
#include "DevTokenDataTypes.h"


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CTrustedSitesConduit::NewL()
// ---------------------------------------------------------------------------
//
CTrustedSitesConduit* CTrustedSitesConduit::NewL(CTrustedSitesServer& aServer)
    {
    return new (ELeave) CTrustedSitesConduit(aServer);
    }


// ---------------------------------------------------------------------------
// CTrustedSitesConduit::CTrustedSitesConduit()
// ---------------------------------------------------------------------------
//
CTrustedSitesConduit::CTrustedSitesConduit(CTrustedSitesServer& aServer) :
  iServer(aServer)
    {
    }


// ---------------------------------------------------------------------------
// CTrustedSitesConduit::~CTrustedSitesConduit()
// ---------------------------------------------------------------------------
//
CTrustedSitesConduit::~CTrustedSitesConduit()
    {
    }


// ---------------------------------------------------------------------------
// CTrustedSitesConduit::AllocResponseBufferLC()
// ---------------------------------------------------------------------------
//
HBufC8* CTrustedSitesConduit::AllocResponseBufferLC(TInt aSize, const RMessage2& aMessage)
    {
    TInt writeBufLen = aMessage.GetDesLengthL(1);

    if (aSize > writeBufLen)
        {
        TPckg<TInt> theRequiredLength(aSize);
        aMessage.WriteL(1, theRequiredLength);
        User::Leave(KErrOverflow);
        }

    HBufC8* result = HBufC8::NewMaxLC(aSize);
    TPtr8 ptr = result->Des();
    ptr.FillZ();

    return result;
    }


// ---------------------------------------------------------------------------
// CTrustedSitesConduit::ServiceTrustedSitesRequestL()
// ---------------------------------------------------------------------------
//
void CTrustedSitesConduit::ServiceTrustedSitesRequestL(const RMessage2& aMessage)
    {
    TDevTokenMessages request = (TDevTokenMessages) aMessage.Function();
    TInt result = KErrNone;

    switch (request)
        {
        case EAddTrustSite:
            AddL(aMessage);   
            break;
            
        case EAddForgivenSite:
            AddForgivenSiteL( aMessage );
            break;
        
        case ERemoveForgivenSite:
            RemoveForgivenSiteL( aMessage );
            break;
            
        case EIsTrustedSite:
            result = IsTrustedSiteL( aMessage );
            break;
        case EGetTrustedSites:
            GetTrustedSitesL( aMessage );
            break;
            
        case EIsOutOfDateAllowed:
            result = IsOutOfDateAllowedL( aMessage ); 
            break;
        default:
        // Client made an illegal request
        PanicClient(aMessage, EPanicInvalidRequest);
        return;
        }

    aMessage.Complete(result);
    }


// ---------------------------------------------------------------------------
// CTrustedSitesConduit::AddL()
// ---------------------------------------------------------------------------
//
void CTrustedSitesConduit::AddL( const RMessage2& aMessage )
    {
    HBufC8* certHash = HBufC8::NewLC(20);
    TPtr8 ptrCertHash = certHash->Des();
    aMessage.ReadL(0,ptrCertHash);  

    TInt bufLen = aMessage.GetDesLengthL(1);
    HBufC* siteName = HBufC::NewLC(bufLen);
    TPtr ptrSiteName = siteName->Des();
    aMessage.ReadL(1,ptrSiteName);

    iServer.AddL( *certHash, *siteName, aMessage );    

    CleanupStack::PopAndDestroy(2);
    }


// ---------------------------------------------------------------------------
// CTrustedSitesConduit::AddForgivenSiteL()
// ---------------------------------------------------------------------------
//
void CTrustedSitesConduit::AddForgivenSiteL( const RMessage2& aMessage )
    {
    TInt bufLen = aMessage.GetDesLengthL(0);
    HBufC* forgivenSite = HBufC::NewLC(bufLen);
    TPtr ptrSite = forgivenSite->Des();
    aMessage.ReadL(0,ptrSite);
    
    TInt outofdateallowed = aMessage.Int1();
    iServer.AddForgivenSiteL( *forgivenSite, (TBool)outofdateallowed, aMessage );    

    CleanupStack::PopAndDestroy(1);
    }
    
// ---------------------------------------------------------------------------
// CTrustedSitesConduit::RemoveForgivenSiteL()
// ---------------------------------------------------------------------------
//
void CTrustedSitesConduit::RemoveForgivenSiteL( const RMessage2& aMessage )
    {
    TInt bufLen = aMessage.GetDesLengthL(0);
    
    if ( bufLen == 0 )
    	{
    	iServer.RemoveAllForgivenSitesL( aMessage ); 
    	}
    else
    	{
        HBufC* forgivenSite = HBufC::NewLC( bufLen );
        TPtr ptrSite = forgivenSite->Des();
        aMessage.ReadL(0,ptrSite);
        
        iServer.RemoveThisForgivenSiteL( *forgivenSite, aMessage );    

        CleanupStack::PopAndDestroy(1);
    	}
    }



// ---------------------------------------------------------------------------
// CTrustedSitesConduit::IsTrustedSiteL()
// ---------------------------------------------------------------------------
//
TInt CTrustedSitesConduit::IsTrustedSiteL( const RMessage2& aMessage ) 
    {
    HBufC8* certHash = HBufC8::NewLC(20);
    TPtr8 ptrCertHash = certHash->Des();
    aMessage.ReadL(0,ptrCertHash);  

    TInt bufLen = aMessage.GetDesLengthL(1);
    HBufC* siteName = HBufC::NewLC(bufLen);
    TPtr ptrSiteName = siteName->Des();
    aMessage.ReadL(1,ptrSiteName);
    
    //get the certifcate
    bufLen = aMessage.GetDesLength(2);
    HBufC8* cert = HBufC8::NewLC( bufLen );
    TPtr8 ptrCert = cert->Des();
    aMessage.ReadL(2, ptrCert );
    
    TBool result = iServer.IsTrustedSiteL( *certHash, *siteName, *cert, aMessage );

    CleanupStack::PopAndDestroy(3); //certHash, siteName, cert

    return result? 1 : 0; 
    }


// ---------------------------------------------------------------------------
// CTrustedSitesConduit::IsOutOfDateAllowedL()
// ---------------------------------------------------------------------------
//
TInt CTrustedSitesConduit::IsOutOfDateAllowedL( const RMessage2& aMessage )
    {
    HBufC8* certHash = HBufC8::NewLC(20);
    TPtr8 ptrCertHash = certHash->Des();
    aMessage.ReadL(0,ptrCertHash);  

    TInt bufLen = aMessage.GetDesLengthL(1);
    HBufC* siteName = HBufC::NewLC(bufLen);
    TPtr ptrSiteName = siteName->Des();
    aMessage.ReadL(1,ptrSiteName);

    
    TBool result = iServer.IsOutOfDateAllowedL( *certHash, *siteName, aMessage );

    CleanupStack::PopAndDestroy(2); //certHash, siteName

    return result? 1 : 0; 
    }


// ---------------------------------------------------------------------------
// CTrustedSitesConduit::GetTrustedSitesL()
// ---------------------------------------------------------------------------
//
void CTrustedSitesConduit::GetTrustedSitesL( const RMessage2& aMessage )
    {
    HBufC8* certHash = HBufC8::NewLC(20);
    TPtr8 ptrCertHash = certHash->Des();
    aMessage.ReadL(0,ptrCertHash);

    RPointerArray<HBufC> sites;

    iServer.GetTrustedSitesL(*certHash, sites, aMessage );

    HBufC8* clientBuffer = AllocResponseBufferLC(DevTokenDataMarshaller::Size(sites), aMessage);  
    TPtr8 theData(clientBuffer->Des());

    DevTokenDataMarshaller::Write(sites, theData);

    aMessage.WriteL(1, theData);

    sites.ResetAndDestroy();

    CleanupStack::PopAndDestroy(2); //certhash, clientbuffer
    }
  

//EOF
