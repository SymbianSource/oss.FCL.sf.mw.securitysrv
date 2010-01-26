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
* Description:   Implementation of TrustedSitesStore
*
*/



#include <x509cert.h>
#include "TrustedSitesStore.h"
#include "DevandTruSrvCertStore.h"
#include "DevTokenType.h"
#include "DevToken.h"
#include "DevTokenUtils.h"
#include "DevTokenImplementationUID.hrh"

const TInt KSHA1Length = 20;

// Define the default size. If overflow, the function 
// SendSyncRequestAndHandleOverflowL() will handle.
const TInt KDefaultBufferSize = 256;

// ======== MEMBER FUNCTIONS ========

// -----------------------------------------------------------------------------
// CTrustSitesStore::NewL()
// -----------------------------------------------------------------------------
//
EXPORT_C CTrustSitesStore* CTrustSitesStore::NewL()
    {
    CTrustSitesStore* me = new (ELeave) CTrustSitesStore();
    CleanupStack::PushL(me);
    me->ConstructL();
    CleanupStack::Pop(me);
    return (me);
    }


// -----------------------------------------------------------------------------
// CTrustSitesStore::CTrustSitesStore()
// -----------------------------------------------------------------------------
//
CTrustSitesStore::CTrustSitesStore() :
  iRequestPtr(NULL, 0, 0) 
    {
    }


// -----------------------------------------------------------------------------
// CTrustSitesStore::~CTrustSitesStore()
// -----------------------------------------------------------------------------
//
EXPORT_C CTrustSitesStore::~CTrustSitesStore()
    {
    FreeRequestBuffer();  
    if ( iClientSession )
        {
        iClientSession->Close();
        delete iClientSession;
        }
    }


// -----------------------------------------------------------------------------
// CTrustSitesStore::ConstructL()
// -----------------------------------------------------------------------------
//
void CTrustSitesStore::ConstructL()
    { 
    iClientSession = RDevTokenClientSession::ClientSessionL();
    User::LeaveIfError(iClientSession->Connect(ETruSitesStore));
    }


// -----------------------------------------------------------------------------
// CTrustSitesStore::AddL()
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CTrustSitesStore::AddL( const TDesC8& aCertificate, const TDesC& aTrustedSite )
    {
    if( !(aCertificate.Length() > KSHA1Length) || !( aTrustedSite.Length() > 0) )
        {
        User::LeaveIfError(KErrArgument);
        }

    CX509Certificate* cert = CX509Certificate::NewL(aCertificate);
    TBuf8<KSHA1Length> certHash;
    certHash.Copy(cert->Fingerprint());
    delete cert;

    TIpcArgs args(&certHash, &aTrustedSite, NULL, NULL);
    TInt r = iClientSession->SendRequest(EAddTrustSite, args);  
    return r; 
    }


// -----------------------------------------------------------------------------
// CTrustSitesStore::AddForgivenSiteL()
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CTrustSitesStore::AddForgivenSiteL( const TDesC& aSite, const TBool& aIsOutOfDateAllowed )
    {
    if( !( aSite.Length() > 0) )
        {
        User::LeaveIfError(KErrArgument);
        }

    TIpcArgs args( &aSite, aIsOutOfDateAllowed );
    TInt r = iClientSession->SendRequest(EAddForgivenSite, args);  
    return r; 	
    }


// -----------------------------------------------------------------------------
// CTrustSitesStore::RemoveForgivenSiteL()
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CTrustSitesStore::RemoveForgivenSiteL( const TDesC& aSite )
	{
	TIpcArgs args( &aSite );
	TInt r = iClientSession->SendRequest(ERemoveForgivenSite, args);  
	return r;
	}


// -----------------------------------------------------------------------------
// CTrustSitesStore::IsOutOfDateAllowedL()
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CTrustSitesStore::IsOutOfDateAllowedL( const TDesC8& aCertificate, const TDesC& aTrustedSite )
    {
    if( !(aCertificate.Length() > 0) || !( aTrustedSite.Length() > 0) )
        {
        User::LeaveIfError(KErrArgument);
        }

    CX509Certificate* cert = CX509Certificate::NewL(aCertificate);
    TBuf8<KSHA1Length> certHash;
    certHash.Copy(cert->Fingerprint());
    delete cert;
    cert = NULL;

    TIpcArgs args(&certHash, &aTrustedSite );
    TInt r = iClientSession->SendRequest(EIsOutOfDateAllowed, args); 
    if( r > 0 )
        {
        return ETrue;
        }
    else
        {
        return EFalse;  
        } 	
   }

// -----------------------------------------------------------------------------
// CTrustSitesStore::IsTrustedSiteL()
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CTrustSitesStore::IsTrustedSiteL( const TDesC8& aCertificate, const TDesC& aTrustedSite )
    {
    if( !(aCertificate.Length() > 0) || !( aTrustedSite.Length() > 0) )
        {
        User::LeaveIfError(KErrArgument);
        }

    CX509Certificate* cert = CX509Certificate::NewL(aCertificate);
    TBuf8<KSHA1Length> certHash;
    certHash.Copy(cert->Fingerprint());
    delete cert;
    cert = NULL;

    TIpcArgs args(&certHash, &aTrustedSite, &aCertificate );
    TInt r = iClientSession->SendRequest(EIsTrustedSite, args); 
    if( r > 0 )
        {
        return ETrue;
        }
    else
        {
        return EFalse;  
        } 
    }


// -----------------------------------------------------------------------------
// CTrustSitesStore::GetTrustedSitesL()
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CTrustSitesStore::GetTrustedSitesL( const TDesC8& aCertificate, RPointerArray<HBufC>& aListOfSites )       
    {
    if( !(aCertificate.Length() > 0 ) ) 
        {
        User::LeaveIfError(KErrArgument);
        }

    CX509Certificate* cert = CX509Certificate::NewL(aCertificate);
    TBuf8<KSHA1Length> certHash;
    certHash.Copy(cert->Fingerprint()); 
    delete cert;
    cert = NULL;

    TIpcArgs args( &certHash, &iRequestPtr, NULL, NULL );

    TRAPD(err, SendSyncRequestAndHandleOverflowL(KDefaultBufferSize, args));
    if(err!=KErrNone)
        {
        return err;
        }

    TRAP(err, DevTokenDataMarshaller::ReadL( iRequestPtr, aListOfSites ));
    if( err!= KErrNone )
        {
        return err;
        }
    return KErrNone;    
    }


// -----------------------------------------------------------------------------
// CTrustSitesStore::RemoveL()
// -----------------------------------------------------------------------------
//
/*EXPORT_C TInt CTrustSitesStore::RemoveL( const TDesC8& aCertificate )
    {
    if( !(aCertificate.Length() > 0 ) )
        {
        User::LeaveIfError(KErrArgument);
        }

    CX509Certificate* cert = CX509Certificate::NewL(aCertificate);
    TBuf8<KSHA1Length> certHash;
    certHash.Copy(cert->Fingerprint()); 

    TIpcArgs args( &certHash, NULL, NULL, NULL );

    TInt r = iClientSession->SendRequest(ERemove, args);  
    return r; 
    }*/


// -----------------------------------------------------------------------------
// CTrustSitesStore::FreeRequestBuffer()
// -----------------------------------------------------------------------------
//
void CTrustSitesStore::FreeRequestBuffer() 
    {
    delete iRequestDataBuf; 
    iRequestDataBuf = NULL;
    iRequestPtr.Set(NULL, 0, 0);
    }


// -----------------------------------------------------------------------------
// CTrustSitesStore::AllocRequestBuffer()
// -----------------------------------------------------------------------------
//
TInt CTrustSitesStore::AllocRequestBuffer(TInt aReqdSize) 
    {
    ASSERT(aReqdSize > 0);
    TInt result = KErrNoMemory;

    FreeRequestBuffer();
    iRequestDataBuf = HBufC8::NewMax(aReqdSize);
    if (iRequestDataBuf)
        {
        iRequestPtr.Set(iRequestDataBuf->Des());
        iRequestPtr.FillZ();
        result = KErrNone;
        }

    return result;
    }


// -----------------------------------------------------------------------------
// CTrustSitesStore::SendSyncRequestAndHandleOverflowL()
// -----------------------------------------------------------------------------
//
void CTrustSitesStore::SendSyncRequestAndHandleOverflowL(TInt aInitialBufSize,
                          const TIpcArgs& aArgs) 
    {

    TDevTokenMessages message;
    message = EGetTrustedSites; 

    User::LeaveIfError(AllocRequestBuffer(aInitialBufSize));

    TInt err = iClientSession->SendRequest(message, aArgs);
    if (err == KErrOverflow)
        {
        TInt sizeReqd = 0;
        TPckg<TInt> theSize(sizeReqd);
        theSize.Copy(iRequestPtr);
        User::LeaveIfError(AllocRequestBuffer(sizeReqd));     
        err = iClientSession->SendRequest(message, aArgs);
        }

    User::LeaveIfError(err);
    }
 
//EOF



