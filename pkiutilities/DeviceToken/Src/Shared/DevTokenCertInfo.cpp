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
* Description:   Implementation of DevTokenCertInfo
*
*/



#include "DevTokenDataTypes.h"

// ======== MEMBER FUNCTIONS ========


// ---------------------------------------------------------------------------
// CDevTokenCertInfo::NewLC(const CDevTokenCertInfo& aOther)
// ---------------------------------------------------------------------------
// 
EXPORT_C CDevTokenCertInfo* CDevTokenCertInfo::NewLC(const CDevTokenCertInfo& aOther)
    {
    CDevTokenCertInfo* self = NewL(aOther);
    CleanupStack::PushL(self);
    return self;
    }


// ---------------------------------------------------------------------------
// CDevTokenCertInfo::NewL(const CDevTokenCertInfo& aOther)
// ---------------------------------------------------------------------------
// 
EXPORT_C CDevTokenCertInfo* CDevTokenCertInfo::NewL(const CDevTokenCertInfo& aOther)
    {
    return new (ELeave) CDevTokenCertInfo(aOther);
    }


// ---------------------------------------------------------------------------
// CDevTokenCertInfo::CDevTokenCertInfo(const CDevTokenCertInfo& aOther)
// ---------------------------------------------------------------------------
//
CDevTokenCertInfo::CDevTokenCertInfo(const CDevTokenCertInfo& aOther) :
    MCertInfo(aOther)
    {
    }


// ---------------------------------------------------------------------------
// CDevTokenCertInfo* CDevTokenCertInfo::NewL()
// ---------------------------------------------------------------------------
//
EXPORT_C CDevTokenCertInfo* CDevTokenCertInfo::NewL(const TDesC& aLabel,
               TCertificateFormat aFormat,
               TCertificateOwnerType aCertificateOwnerType,
               TInt aSize,
               const TKeyIdentifier* aSubjectKeyId,   
               const TKeyIdentifier* aIssuerKeyId,
               TInt aCertificateId)
    {
    CDevTokenCertInfo* self = new(ELeave) CDevTokenCertInfo(
        aLabel,
        aFormat, 
        aCertificateOwnerType, 
        aSize, 
        aSubjectKeyId, 
        aIssuerKeyId,
        aCertificateId);    
    CleanupStack::PushL( self );
    self->ConstructL(NULL);
    CleanupStack::Pop( self );    
    return self;
    }


// ---------------------------------------------------------------------------
// CDevTokenCertInfo* CDevTokenCertInfo::NewL()
// ---------------------------------------------------------------------------
//
EXPORT_C CDevTokenCertInfo* CDevTokenCertInfo::NewL(const TDesC& aLabel,
               TCertificateFormat aFormat,
               TCertificateOwnerType aCertificateOwnerType,
               TInt aSize,
               const TKeyIdentifier* aSubjectKeyId,   
               const TKeyIdentifier* aIssuerKeyId,
               TInt aCertificateId,
               const TBool aDeletable)
    {
    CDevTokenCertInfo* self = new(ELeave) CDevTokenCertInfo(
        aLabel,
        aFormat, 
        aCertificateOwnerType, 
        aSize, 
        aSubjectKeyId,  
        aIssuerKeyId,
        aCertificateId, 
        aDeletable);
    CleanupStack::PushL( self );
    self->ConstructL(NULL);
    CleanupStack::Pop( self );    
    return self;
    }


// ---------------------------------------------------------------------------
// CDevTokenCertInfo* CDevTokenCertInfo::NewLC()
// ---------------------------------------------------------------------------
//
EXPORT_C CDevTokenCertInfo* CDevTokenCertInfo::NewLC(const TDesC& aLabel,
              TCertificateFormat aFormat,
              TCertificateOwnerType aCertificateOwnerType,
              TInt aSize,
              const TKeyIdentifier* aSubjectKeyId,
              const TKeyIdentifier* aIssuerKeyId,
              TInt aCertificateId)
    {
    CDevTokenCertInfo* self = CDevTokenCertInfo::NewL(
        aLabel,
        aFormat, 
        aCertificateOwnerType, 
        aSize, 
        aSubjectKeyId, 
        aIssuerKeyId,
        aCertificateId);
    CleanupStack::PushL(self);
    return self;
    }


// ---------------------------------------------------------------------------
// CDevTokenCertInfo* CDevTokenCertInfo::NewLC()
// ---------------------------------------------------------------------------
//
EXPORT_C CDevTokenCertInfo* CDevTokenCertInfo::NewLC(const TDesC& aLabel,
              TCertificateFormat aFormat,
              TCertificateOwnerType aCertificateOwnerType,
              TInt aSize,
              const TKeyIdentifier* aSubjectKeyId,
              const TKeyIdentifier* aIssuerKeyId,
              TInt aCertificateId,
                const TBool aDeletable)
    {
    CDevTokenCertInfo* self = CDevTokenCertInfo::NewL(
        aLabel,
        aFormat, 
        aCertificateOwnerType, 
        aSize, 
        aSubjectKeyId, 
        aIssuerKeyId,
        aCertificateId, 
        aDeletable);
    CleanupStack::PushL(self);
    return self;
    }


// ---------------------------------------------------------------------------
// CDevTokenCertInfo::CDevTokenCertInfo()
// ---------------------------------------------------------------------------
//
CDevTokenCertInfo::CDevTokenCertInfo(const TDesC& aLabel, 
           TCertificateFormat aFormat,
           TCertificateOwnerType aCertificateOwnerType,
           TInt aSize,
           const TKeyIdentifier* aSubjectKeyId,
           const TKeyIdentifier* aIssuerKeyId,
           TInt aCertificateId,
           const TBool aDeletable) :
  MCertInfo(aLabel, aFormat, aCertificateOwnerType, aSize, aSubjectKeyId,
        aIssuerKeyId, aCertificateId, aDeletable)
    {
    }


// ---------------------------------------------------------------------------
// CDevTokenCertInfo* CDevTokenCertInfo::NewL(RReadStream& aStream)
// ---------------------------------------------------------------------------
//
EXPORT_C CDevTokenCertInfo* CDevTokenCertInfo::NewL(RReadStream& aStream)
    {
    CDevTokenCertInfo* me = NewLC(aStream);
    CleanupStack::Pop(me);
    return me;
    }


// ---------------------------------------------------------------------------
// CDevTokenCertInfo* CDevTokenCertInfo::NewLC(RReadStream& aStream)
// ---------------------------------------------------------------------------
//
EXPORT_C CDevTokenCertInfo* CDevTokenCertInfo::NewLC(RReadStream& aStream)
    {
    CDevTokenCertInfo* self = new (ELeave) CDevTokenCertInfo();
    CleanupStack::PushL(self);
    self->InternalizeL(aStream);
    return self;
    }


// ---------------------------------------------------------------------------
// CDevTokenCertInfo::CDevTokenCertInfo() 
// ---------------------------------------------------------------------------
//
CDevTokenCertInfo::CDevTokenCertInfo() :
    MCertInfo()
    {
    }

//EOF
