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
* Description:   Implementation of DevandTruSrvCertStoreEntryList
*
*/



#include "DevandTruSrvCertStoreEntryList.h"
#include "DevandTruSrvCertStoreEntry.h"
#include "DevTokenDataTypes.h"
#include "DevTokenUtil.h"


// ======== MEMBER FUNCTIONS ========

//CDevandTruSrvCertStoreEntryList

// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreEntryList::NewLC()
// ---------------------------------------------------------------------------
//
CDevandTruSrvCertStoreEntryList* CDevandTruSrvCertStoreEntryList::NewLC()
    {
    CDevandTruSrvCertStoreEntryList* self = new(ELeave) CDevandTruSrvCertStoreEntryList();
    CleanupStack::PushL(self);
    return self;
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreEntryList::NewL()
// ---------------------------------------------------------------------------
//
CDevandTruSrvCertStoreEntryList* CDevandTruSrvCertStoreEntryList::NewL(RReadStream& aStream)
    {
    CDevandTruSrvCertStoreEntryList* self = new(ELeave) CDevandTruSrvCertStoreEntryList();
    CleanupStack::PushL(self);
    self->InternalizeL(aStream);
    CleanupStack::Pop();
    return self;
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreEntryList::CDevandTruSrvCertStoreEntryList()
// ---------------------------------------------------------------------------
//
CDevandTruSrvCertStoreEntryList::CDevandTruSrvCertStoreEntryList()
    {
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreEntryList::~CDevandTruSrvCertStoreEntryList()
// ---------------------------------------------------------------------------
//
CDevandTruSrvCertStoreEntryList::~CDevandTruSrvCertStoreEntryList()
    {
    iEntries.ResetAndDestroy();
    iEntries.Close();
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreEntryList::ExternalizeL()
// ---------------------------------------------------------------------------
//
void CDevandTruSrvCertStoreEntryList::ExternalizeL(RWriteStream& aStream) const
    {
    TInt count = iEntries.Count();
    aStream.WriteInt32L(count);
    for (TInt i = 0 ; i < count ; ++i)
        {
        aStream << *iEntries[i];
        }
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreEntryList::InternalizeL()
// ---------------------------------------------------------------------------
//
void CDevandTruSrvCertStoreEntryList::InternalizeL(RReadStream& aStream)
    {
    TInt count = aStream.ReadInt32L();

    for (TInt i = 0 ; i < count ; ++i)
        {
        CDevandTruSrvCertStoreEntry* entry = CDevandTruSrvCertStoreEntry::NewLC(aStream);

        // Check certificate id is unique
        TInt handle = entry->CertInfo().CertificateId();
        if (IndexForHandle(handle) != KErrNotFound)
            {
            User::Leave(KErrCorrupt);
            }

        AppendL(entry);
        CleanupStack::Pop(entry);   
        }
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreEntryList::Count()
// ---------------------------------------------------------------------------
//
TInt CDevandTruSrvCertStoreEntryList::Count() const
    {
    return iEntries.Count();
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreEntryList::AppendL()
// ---------------------------------------------------------------------------
//
TInt CDevandTruSrvCertStoreEntryList::AppendL(CDevandTruSrvCertStoreEntry* aCertInfo)
    {
    assert(aCertInfo, EPanicCertStoreEntryListAppendArguments);
    User::LeaveIfError(iEntries.Append(aCertInfo));
    return iEntries.Count() - 1;
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreEntryList::Remove()
// ---------------------------------------------------------------------------
//
CDevandTruSrvCertStoreEntry* CDevandTruSrvCertStoreEntryList::Remove(TInt aIndex)
    {
    CDevandTruSrvCertStoreEntry* entry = iEntries[aIndex];
    iEntries.Remove(aIndex);
    return entry;
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreEntryList::Replace()
// ---------------------------------------------------------------------------
//
CDevandTruSrvCertStoreEntry* CDevandTruSrvCertStoreEntryList::Replace(TInt aIndex, CDevandTruSrvCertStoreEntry* aNewEntry)
    {
    assert(aNewEntry, EPanicCertStoreEntryListReplaceArguments);
    CDevandTruSrvCertStoreEntry* entry = iEntries[aIndex];
    iEntries[aIndex] = aNewEntry;
    return entry;
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreEntryList::IndexForHandle()
// ---------------------------------------------------------------------------
//
TInt CDevandTruSrvCertStoreEntryList::IndexForHandle(TInt aHandle) const
    {
    for (TInt i = 0 ; i < iEntries.Count() ; ++i)
        {
        CDevandTruSrvCertStoreEntry* entry = iEntries[i];
        if (entry->CertInfo().Reference() == aHandle)
            {
            return i;
            }
        }

    return KErrNotFound;
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreEntryList::LabelExists()
// ---------------------------------------------------------------------------
//
TBool CDevandTruSrvCertStoreEntryList::LabelExists(const TDesC& aLabel) const
    {
    for (TInt i = 0 ; i < iEntries.Count() ; ++i)
        {
        CDevandTruSrvCertStoreEntry* entry = iEntries[i];
        if (entry->CertInfo().Label() == aLabel)
            {
            return ETrue;
            }
        }

    return EFalse;
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreEntryList::GetByIndex()
// ---------------------------------------------------------------------------
//
const CDevandTruSrvCertStoreEntry& CDevandTruSrvCertStoreEntryList::GetByIndex(
    TInt aIndex) const
    {
    return *iEntries[aIndex];
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreEntryList::GetByHandleL()
// ---------------------------------------------------------------------------
//
const CDevandTruSrvCertStoreEntry& CDevandTruSrvCertStoreEntryList::GetByHandleL(
    TInt aHandle) const
    {
    TInt index = IndexForHandle(aHandle);
    User::LeaveIfError(index);
    return GetByIndex(index);
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreEntryList::NextFreeHandle()
// ---------------------------------------------------------------------------
//
TInt CDevandTruSrvCertStoreEntryList::NextFreeHandle() const 
    {
    TInt count = iEntries.Count();
    TInt maxHandle = -1;
    for (TInt i = 0; i < count; i++)
        {
        CDevandTruSrvCertStoreEntry* entry = iEntries[i];
        if (entry->CertInfo().Reference() > maxHandle)
            {
            maxHandle = entry->CertInfo().Reference();
            }
        }

    return maxHandle + 1;
    }

//EOF

