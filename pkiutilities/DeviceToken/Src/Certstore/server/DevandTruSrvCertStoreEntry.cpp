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
* Description:   Implementation of DevandTruSrvCertStoreEntry
*
*/



#include "DevandTruSrvCertStoreEntry.h"
#include "DevTokenDataTypes.h"
#include "DevTokenUtil.h"


// ======== MEMBER FUNCTIONS ========

//CDevandTruSrvCertStoreEntry

// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreEntry::NewL()
// ---------------------------------------------------------------------------
//
CDevandTruSrvCertStoreEntry* CDevandTruSrvCertStoreEntry::NewL(const CDevTokenCertInfo& aCertInfo,
                     RArray<TUid> aCertificateApps,
                     TBool aTrusted,
                     TStreamId aDataStreamId)
    {
    CDevandTruSrvCertStoreEntry* self = new(ELeave) CDevandTruSrvCertStoreEntry();
    CleanupStack::PushL(self);
    self->ConstructL(aCertInfo, aCertificateApps, aTrusted, aDataStreamId);
    CleanupStack::Pop(self);
    return self;
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreEntry::NewLC()
// ---------------------------------------------------------------------------
//
CDevandTruSrvCertStoreEntry* CDevandTruSrvCertStoreEntry::NewLC(RReadStream& aStream)
    {
    CDevandTruSrvCertStoreEntry* self = new(ELeave) CDevandTruSrvCertStoreEntry();
    CleanupStack::PushL(self);
    self->InternalizeL(aStream);
    return self;
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreEntry::CDevandTruSrvCertStoreEntry()
// ---------------------------------------------------------------------------
//
CDevandTruSrvCertStoreEntry::CDevandTruSrvCertStoreEntry()
    { 
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreEntry::ConstructL()
// ---------------------------------------------------------------------------
//
void CDevandTruSrvCertStoreEntry::ConstructL(const CDevTokenCertInfo& aCertInfo,
                 RArray<TUid> aCertificateApps,
                 TBool aTrusted,
                 TStreamId aDataStreamId)
    {
    assert(aDataStreamId != KNullStreamId, EPanicCertStoreEntryConstructArguments);

    iCertInfo = CDevTokenCertInfo::NewL(aCertInfo);

    for (TInt i = 0 ; i < aCertificateApps.Count() ; ++i)
        {
        User::LeaveIfError(iCertificateApps.Append(aCertificateApps[i]));
        }

    iTrusted = aTrusted;  
    iDataStreamId = aDataStreamId;
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreEntry::~CDevandTruSrvCertStoreEntry()
// ---------------------------------------------------------------------------
//
CDevandTruSrvCertStoreEntry::~CDevandTruSrvCertStoreEntry()
    {
    delete iCertInfo;
    iCertificateApps.Close();
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreEntry::ExternalizeL()
// ---------------------------------------------------------------------------
//
void CDevandTruSrvCertStoreEntry::ExternalizeL(RWriteStream& aStream) const
    {
    aStream << *iCertInfo;
    TInt count = iCertificateApps.Count();
    aStream.WriteInt32L(count);
    for (TInt i = 0 ; i < count ; ++i)
        {
        aStream << iCertificateApps[i];
        }
    aStream.WriteUint8L(iTrusted);
    aStream << iDataStreamId;
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreEntry::InternalizeL()
// ---------------------------------------------------------------------------
//
void CDevandTruSrvCertStoreEntry::InternalizeL(RReadStream& aStream)
    {
    assert(!iCertInfo, EPanicCertStoreEntryInternalizeState);
    iCertInfo = CDevTokenCertInfo::NewL(aStream);

    TInt count = aStream.ReadInt32L();
    for (TInt i = 0 ; i < count ; ++i)
        {
        TUid id;
        aStream >> id;
        User::LeaveIfError(iCertificateApps.Append(id));
        }

    iTrusted = !!aStream.ReadUint8L();
    aStream >> iDataStreamId;
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreEntry::CertInfo()
// ---------------------------------------------------------------------------
//
const CDevTokenCertInfo& CDevandTruSrvCertStoreEntry::CertInfo() const
    {
    return *iCertInfo;
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreEntry::CertificateApps()
// ---------------------------------------------------------------------------
//
const RArray<TUid>& CDevandTruSrvCertStoreEntry::CertificateApps() const
    {
    return iCertificateApps;
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreEntry::IsApplicable()
// ---------------------------------------------------------------------------
//
TBool CDevandTruSrvCertStoreEntry::IsApplicable(const TUid& aApplication) const
    {
    for (TInt i = 0 ; i < iCertificateApps.Count() ; ++i)
        {
        if (iCertificateApps[i] == aApplication)
            {
            return ETrue;
            }
        }
    return EFalse;
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreEntry::Trusted()
// ---------------------------------------------------------------------------
//
TBool CDevandTruSrvCertStoreEntry::Trusted() const
    {
    return iTrusted;
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreEntry::DataStreamId()
// ---------------------------------------------------------------------------
//
TStreamId CDevandTruSrvCertStoreEntry::DataStreamId() const
    {
    return iDataStreamId;
    }

//EOF

