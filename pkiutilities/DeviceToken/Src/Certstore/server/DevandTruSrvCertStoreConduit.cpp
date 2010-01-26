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
* Description:   Implementation of DevandTruSrvCertStoreConduit
*
*/



#include "DevandTruSrvCertStoreConduit.h"
#include "DevandTruSrvCertStoreServer.h"
#include "DevTokenMarshaller.h"
#include "DevTokenUtil.h"
#include "DevTokenDataTypes.h"


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreConduit::NewL()
// ---------------------------------------------------------------------------
//
CDevandTruSrvCertStoreConduit* CDevandTruSrvCertStoreConduit::NewL(CDevandTruSrvCertStoreServer& aServer)
    {
    return new (ELeave) CDevandTruSrvCertStoreConduit(aServer);
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreConduit::CDevandTruSrvCertStoreConduit()
// ---------------------------------------------------------------------------
//
CDevandTruSrvCertStoreConduit::CDevandTruSrvCertStoreConduit(CDevandTruSrvCertStoreServer& aServer) :
  iServer(aServer)
    {
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreConduit::~CDevandTruSrvCertStoreConduit()
// ---------------------------------------------------------------------------
//
CDevandTruSrvCertStoreConduit::~CDevandTruSrvCertStoreConduit()
    {
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreConduit::AllocResponseBufferLC()
// Allocate a buffer for the externalised response - this is then copied into
// the client's buffer.
//
// This also checks that the client's buffer is big enough (this is assumed to
// be in message slot 3).  If the client buffer isn't big enough, the required
// length is passed back in the first word of the buffer and the method leaves
// with KErrOverflow.
// ---------------------------------------------------------------------------
// 
HBufC8* CDevandTruSrvCertStoreConduit::AllocResponseBufferLC(TInt aSize, const RMessage2& aMessage)
    {
    TInt writeBufLen = aMessage.GetDesLengthL(3);

    if (aSize > writeBufLen)
        {
        TPckg<TInt> theRequiredLength(aSize);
        aMessage.WriteL(3, theRequiredLength);
        User::Leave(KErrOverflow);
        }

    HBufC8* result = HBufC8::NewMaxLC(aSize);
    TPtr8 ptr = result->Des();
    ptr.FillZ();

    return result;
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreConduit::ServiceCertStoreRequestL()
// ---------------------------------------------------------------------------
//
void CDevandTruSrvCertStoreConduit::ServiceCertStoreRequestL(const RMessage2& aMessage)
    {
    TDevTokenMessages request = (TDevTokenMessages) aMessage.Function();
    TInt result = KErrNone;

    switch (request)
        {
        case EListCerts:
            ListCertsL(aMessage);   
            break;

        case EGetCert:
            GetCertL(aMessage);     
            break;

        case EApplications:
            ApplicationsL(aMessage);  
            break;

        case EIsApplicable:
            result = IsApplicableL(aMessage); 
            break;

        case ETrusted:
            result = TrustedL(aMessage);      
            break;

        case ERetrieve:
            RetrieveL(aMessage);    
            break;

        case EAddCert:
            AddCertL(aMessage);     
            break;

        case ERemoveCert:
            RemoveCertL(aMessage);    
            break;

        case ESetApplicability:
            SetApplicabilityL(aMessage);
            break;

        case ESetTrust:
            SetTrustL(aMessage);
            break;

        default:
            // Client made an illegal request
            PanicClient(aMessage, EPanicInvalidRequest);
            return;
        }

    aMessage.Complete(result);
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreConduit::ListCertsL()
// ---------------------------------------------------------------------------
//
void CDevandTruSrvCertStoreConduit::ListCertsL(const RMessage2& aMessage)
    {
    // p[1] has filter as set by the client

    TInt bufLength = aMessage.GetDesLengthL(1);
    HBufC8* certFilterBuf = HBufC8::NewMaxLC(bufLength);
    TPtr8 ptr(certFilterBuf->Des());  
    aMessage.ReadL(1, ptr);
    CCertAttributeFilter* certFilter = NULL;
    DevTokenDataMarshaller::ReadL(*certFilterBuf, certFilter);
    CleanupStack::PopAndDestroy(certFilterBuf);
    CleanupStack::PushL(certFilter);

    RPointerArray<CDevTokenCertInfo> certs;
    CleanupClosePushL(certs);
    iServer.ListL(*certFilter, certs, aMessage, EFalse );

    HBufC8* clientBuffer = AllocResponseBufferLC(DevTokenDataMarshaller::Size(certs), aMessage);  
    TPtr8 theData(clientBuffer->Des());
    DevTokenDataMarshaller::Write(certs, theData);
    aMessage.WriteL(3, theData);
    CleanupStack::PopAndDestroy(3, certFilter);
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreConduit::GetCertL()
// ---------------------------------------------------------------------------
//
void CDevandTruSrvCertStoreConduit::GetCertL(const RMessage2& aMessage)
    {
    // p[0] is the cert handle

    const CDevTokenCertInfo& certInfo = iServer.GetCertL(aMessage.Int0());

    HBufC8* clientBuffer = AllocResponseBufferLC(DevTokenDataMarshaller::Size(certInfo), aMessage); 
    TPtr8 ptr(clientBuffer->Des());
    DevTokenDataMarshaller::Write(certInfo, ptr);
    aMessage.WriteL(3, ptr);
    CleanupStack::PopAndDestroy(clientBuffer);
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreConduit::ApplicationsL()
// ---------------------------------------------------------------------------
//
void CDevandTruSrvCertStoreConduit::ApplicationsL(const RMessage2& aMessage)
    {
    // p[0] is the cert handle

    const RArray<TUid>& apps = iServer.ApplicationsL(aMessage.Int0());

    HBufC8* clientBuffer = AllocResponseBufferLC(DevTokenDataMarshaller::Size(apps), aMessage); 
    TPtr8 outPtr(clientBuffer->Des());
    DevTokenDataMarshaller::Write(apps, outPtr);
    aMessage.WriteL(3, outPtr);
    CleanupStack::PopAndDestroy(clientBuffer);
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreConduit::IsApplicableL()
// ---------------------------------------------------------------------------
//
TInt CDevandTruSrvCertStoreConduit::IsApplicableL(const RMessage2& aMessage)
    {
    // p[0] is the cert handle
    // p[2] is the UID
    // p[3] to return the applicability bool

        TUid uid = { aMessage.Int2() };
    TBool result = iServer.IsApplicableL(aMessage.Int0(), uid);
    return result ? 1 : 0;  // Convert TBool to TInt
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreConduit::TrustedL()
// ---------------------------------------------------------------------------
//
TInt CDevandTruSrvCertStoreConduit::TrustedL(const RMessage2& aMessage)
    {
    //  p[1] has CCertInfo in question, p[2] to return the trusted bool

    TBool result = iServer.TrustedL(aMessage.Int0()); 
    return result ? 1 : 0;  // Convert TBool to TInt
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreConduit::RetrieveL()
// ---------------------------------------------------------------------------
//
void CDevandTruSrvCertStoreConduit::RetrieveL(const RMessage2& aMessage)
    {
    // p[0] is the cert handle
    // p[3] the buffer to return the retrieved certificate

    HBufC8* certData = iServer.RetrieveLC(aMessage.Int0(), aMessage, EFalse );

    TInt maxSize = aMessage.GetDesLengthL(3);
    if (maxSize < certData->Size())
        {
        //  Client has passed in a descriptor which is too short
        User::Leave(KErrOverflow);
        }

    aMessage.WriteL(3, *certData);
    CleanupStack::PopAndDestroy(certData);
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreConduit::AddCertL()
// ---------------------------------------------------------------------------
//
void CDevandTruSrvCertStoreConduit::AddCertL(const RMessage2& aMessage)
    {
    //  p[1] has TDevTokenAddCertDataStruct, p[3] has certificate data
    TPckgBuf<TDevTokenAddCertDataStruct> data;
    aMessage.ReadL(1, data);

    TInt maxSize = aMessage.GetDesLengthL(3);
    HBufC8* certData = HBufC8::NewMaxLC(maxSize);
    TPtr8 ptr(certData->Des());
    ptr.FillZ();
    aMessage.ReadL(3, ptr);

    iServer.AddL(data(), *certData, aMessage, EFalse );

    CleanupStack::PopAndDestroy(certData);
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreConduit::RemoveCertL()
// ---------------------------------------------------------------------------
//
void CDevandTruSrvCertStoreConduit::RemoveCertL(const RMessage2& aMessage)
    {
    // p[0] is the cert handle

    iServer.RemoveL(aMessage.Int0(), aMessage, EFalse );
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreConduit::SetApplicabilityL()
// ---------------------------------------------------------------------------
//
void CDevandTruSrvCertStoreConduit::SetApplicabilityL(const RMessage2& aMessage)
    {
    // p[0] is the cert handle
    // p[2] the array of UIDs

    TInt bufLen = aMessage.GetDesLengthL(2);
    HBufC8* buf = HBufC8::NewLC(bufLen);
    TPtr8 bufPtr(buf->Des());
    aMessage.ReadL(2, bufPtr);

    RArray<TUid> apps;
    CleanupClosePushL(apps);
    DevTokenDataMarshaller::ReadL(*buf, apps);

    iServer.SetApplicabilityL(aMessage.Int0(), apps, aMessage);
    CleanupStack::PopAndDestroy(2, buf);  
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreConduit::SetTrustL()
// ---------------------------------------------------------------------------
//
void CDevandTruSrvCertStoreConduit::SetTrustL(const RMessage2& aMessage)
    {
    // p[0] is the cert handle
    // p[2] the trust setting

    TBool trusted = !!aMessage.Int2();    
    iServer.SetTrustL(aMessage.Int0(), trusted, aMessage);
    }

//EOF

