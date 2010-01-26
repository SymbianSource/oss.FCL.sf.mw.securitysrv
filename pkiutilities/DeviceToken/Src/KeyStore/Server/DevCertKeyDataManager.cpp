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
* Description:   Implementation of DevCertKeyDataManager
*
*/



#include "DevCertKeyDataManager.h"
#include "DevTokenDataTypes.h"
#include "DevTokenCliServ.h"
#include "DevTokenUtil.h"

_LIT(KDevCertKeyStoreFilename,"DevCertKeys.dat");


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CDevCertKeyDataManager::NewL()
// Key store data manager - maintains array of objects representing keys
// ---------------------------------------------------------------------------
//
CDevCertKeyDataManager* CDevCertKeyDataManager::NewL()
    {
    CDevCertKeyDataManager* self = new (ELeave) CDevCertKeyDataManager();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }


// ---------------------------------------------------------------------------
// CDevCertKeyDataManager::~CDevCertKeyDataManager()
// ---------------------------------------------------------------------------
//
CDevCertKeyDataManager::~CDevCertKeyDataManager()
    {
    if (iFileStore)
        {
        CompactStore();
        delete iFileStore;
        }

    iFile.Close(); // May already have been closed by store
    iFs.Close();

    iKeys.ResetAndDestroy();
    iKeys.Close();
    }


// ---------------------------------------------------------------------------
// CDevCertKeyDataManager::CDevCertKeyDataManager()
// ---------------------------------------------------------------------------
//
CDevCertKeyDataManager::CDevCertKeyDataManager() :
  iRootStreamId(KNullStreamId),
  iInfoStreamId(KNullStreamId)
    {
    }


// ---------------------------------------------------------------------------
// CDevCertKeyDataManager::ConstructL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyDataManager::ConstructL()
    {
    User::LeaveIfError(iFs.Connect());
    OpenStoreL();

    RStoreReadStream lookupStream;
    lookupStream.OpenLC(*iFileStore, iInfoStreamId);

    TInt count = lookupStream.ReadInt32L();
    for (TInt index = 0; index < count; index++)
        {
        const CDevCertKeyData* keyData = CDevCertKeyData::NewL(lookupStream);

        if (keyData->Handle() > iKeyIdentifier)
        iKeyIdentifier = keyData->Handle();

        iKeys.Append(keyData);
        }

    CleanupStack::PopAndDestroy(&lookupStream);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyDataManager::OpenStoreL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyDataManager::OpenStoreL()
    {
    //  Tries to locate a key store file on the default drive and then from ROM
    //  If it cannot find one, tries to create a file with permanent file store
    //  inside it In all cases, should initialise iFileStore unless it cannot
    //  create the file/store/streams

    __ASSERT_DEBUG(!iFileStore, PanicServer(EPanicStoreInitialised));

    TFileName fullPath;
    FileUtils::MakePrivateFilenameL(iFs, KDevCertKeyStoreFilename, fullPath);

    FileUtils::EnsurePathL(iFs, fullPath);
    TRAPD(result, OpenStoreInFileL(fullPath));

    if (result == KErrInUse) 
        {   
        // Cannot access the file now. Abort server startup rather than wiping the keystore.
        User::Leave(result); 
        }

    if (result != KErrNone)
        {   
        // Not yet opened a valid store, either no file to be found, or no valid
        // store in it. Copy the original one stored in the ROM.
        delete iFileStore;
        iFileStore = NULL;

        TFileName romPath;
        FileUtils::MakePrivateROMFilenameL(iFs, KDevCertKeyStoreFilename, romPath);

        if (result != KErrNotFound)
            {
            // Wipe the keystore if we can't open it (it's corrupt anyway)
            User::LeaveIfError(iFs.Delete(fullPath));
            }

        // Copy data from rom and open it 
        TRAPD(err,
        FileUtils::CopyL(iFs, romPath, fullPath);
        OpenStoreInFileL(fullPath)
        );

        if (KErrNone != err)
            {
            // We tried to copy the keystore from ROM. For some reason this
            // failed and we still cannot open the file. Create a new one from
            // scratch.
            CreateStoreInFileL(fullPath);
            }
        }

    __ASSERT_DEBUG(iFileStore, PanicServer(EPanicStoreInitialised));
    __ASSERT_DEBUG((KNullStreamId!=iRootStreamId), PanicServer(EPanicRootStreamNotReady));
    __ASSERT_DEBUG((KNullStreamId!=iInfoStreamId), PanicServer(EPanicManagerStreamNotReady));
    }


// ---------------------------------------------------------------------------
// CDevCertKeyDataManager::CreateStoreInFileL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyDataManager::CreateStoreInFileL(const TDesC& aFile)
    {
    TInt r = iFs.MkDirAll(aFile);
    if ( (r!=KErrNone) && (r!=KErrAlreadyExists) )
    User::Leave(r);

    iFileStore = CPermanentFileStore::ReplaceL(iFs, aFile, EFileRead | EFileWrite | EFileShareExclusive);
    iFileStore->SetTypeL(KPermanentFileStoreLayoutUid);

    TCleanupItem cleanupStore(RevertStore, iFileStore);
    CleanupStack::PushL(cleanupStore);

    // Create info stream - Currently no passphrase created, and no keys
    RStoreWriteStream managerStream;
    iInfoStreamId = managerStream.CreateLC(*iFileStore);
    managerStream.WriteUint32L(0); // Write key count of zero
    managerStream.CommitL();
    CleanupStack::PopAndDestroy(&managerStream);

    // Create root stream - just contains id of info stream
    RStoreWriteStream rootStream;
    iRootStreamId = rootStream.CreateLC(*iFileStore);
    iFileStore->SetRootL(iRootStreamId);
    rootStream.WriteUint32L(iInfoStreamId.Value());   
    rootStream.CommitL();
    CleanupStack::PopAndDestroy(&rootStream);

    WriteKeysToStoreL();

    iFileStore->CommitL();
    CleanupStack::Pop(); // cleanupStore
    }


// ---------------------------------------------------------------------------
// CDevCertKeyDataManager::OpenStoreInFileL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyDataManager::OpenStoreInFileL(const TDesC& aFile)
    {
    // Make sure the file isn't write protected
    User::LeaveIfError(iFs.SetAtt(aFile, 0, KEntryAttReadOnly));

    User::LeaveIfError(iFile.Open(iFs, aFile, EFileRead | EFileWrite | EFileShareExclusive));

    iFileStore = CPermanentFileStore::FromL(iFile);   

    // Get the salt, root and manager TStreamIds
    iRootStreamId = iFileStore->Root();
    if (iRootStreamId == KNullStreamId)
        {
        User::Leave(KErrCorrupt);
        }

    RStoreReadStream rootStream;
    rootStream.OpenLC(*iFileStore, iRootStreamId);
    iInfoStreamId = (TStreamId)(rootStream.ReadUint32L());
    CleanupStack::PopAndDestroy(&rootStream);
    }


// Methods dealing with atomic updates to key data file 

// ---------------------------------------------------------------------------
// CDevCertKeyDataManager::RevertStore()
// This is a cleanup item that reverts the store
// ---------------------------------------------------------------------------
//
void CDevCertKeyDataManager::RevertStore(TAny* aStore)
    {
    CPermanentFileStore* store = reinterpret_cast<CPermanentFileStore*>(aStore);
    TRAP_IGNORE(store->RevertL());
    // We're ignoring the leave code from this becuase there's no way we can
    // handle this sensibly.  This shouldn't be a problem in practice - this
    // will leave if for example the file store is on removable which is
    // unexpectedly remove, and this is never the case for us.
    }


// ---------------------------------------------------------------------------
// CDevCertKeyDataManager::RevertStore()
// Rewrites the info stream (ie the array of key data info) to the store
// ---------------------------------------------------------------------------
//
void CDevCertKeyDataManager::WriteKeysToStoreL()
    {
    RStoreWriteStream lookupStream;
    lookupStream.ReplaceLC(*iFileStore, iInfoStreamId);

    TInt keyCount = iKeys.Count();
    lookupStream.WriteInt32L(keyCount);

    for (TInt index = 0; index < keyCount; index++)
        {
        const CDevCertKeyData* key = iKeys[index];
        key->ExternalizeL(lookupStream);
        }

    lookupStream.CommitL();
    CleanupStack::PopAndDestroy(&lookupStream);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyDataManager::AddL()
// Add a key to the store.  Assumes that the key data streams (info, public key
// and private key) have already been written.
// ---------------------------------------------------------------------------
// 
void CDevCertKeyDataManager::AddL(const CDevCertKeyData* aKeyData)
    {
    ASSERT(aKeyData);

    // Add the key to to the array, rewrite the infostream and 
    // ONLY THEN commit the store
    User::LeaveIfError(iKeys.Append(aKeyData));

    TRAPD(err, WriteKeysToStoreL());

    // Release ownership of key data and reset default passphrase id if store
    // can't be written
    TCleanupItem cleanupStore(RevertStore, iFileStore);
    CleanupStack::PushL(cleanupStore);

    if (err != KErrNone)
        {
        iKeys.Remove(iKeys.Count() - 1);
        User::Leave(err);
        }

    iFileStore->CommitL();

    CleanupStack::Pop(); // cleanupStore
    }


// ---------------------------------------------------------------------------
// CDevCertKeyDataManager::RemoveL()
// "Transaction safe" key removal - only removes the key in memory and file if
//  all operations are successful.
// ---------------------------------------------------------------------------
// 
void CDevCertKeyDataManager::RemoveL(TInt aObjectId)
    {
    TInt index;
    const CDevCertKeyData* key = NULL;
    for (index = 0 ; index < iKeys.Count() ; ++index)
        {
        if (iKeys[index]->Handle() == aObjectId)
            {
            key = iKeys[index];
            break;
            }
        }

    if (!key)
        {
        User::Leave(KErrNotFound);
        }

    TCleanupItem cleanupStore(RevertStore, iFileStore);
    CleanupStack::PushL(cleanupStore);  

    iFileStore->DeleteL(key->PrivateDataStreamId());
    iFileStore->DeleteL(key->PublicDataStreamId());
    iFileStore->DeleteL(key->InfoDataStreamId());

    // Remove the key
    iKeys.Remove(index);

    TRAPD(res, WriteKeysToStoreL());
    if (res != KErrNone)
        {
        User::LeaveIfError(iKeys.Append(key)); // Put it back, shouldn't leave
        User::Leave(res);
        }
    else 
        {
        delete key;   // Cannot leave from the point it's removed to here, so no cleanup stack!
        }   
    iFileStore->CommitL();

    CleanupStack::Pop(); // cleanupStore
    CompactStore();
    }


// ---------------------------------------------------------------------------
// CDevCertKeyDataManager::IsKeyAlreadyInStore().
// ---------------------------------------------------------------------------
// 
TBool CDevCertKeyDataManager::IsKeyAlreadyInStore(const TDesC& aKeyLabel) const
    {// Check each key in the store to determine if aKeyLabel already exists
    TInt keyCount = iKeys.Count();
    TBool isInStore = EFalse;
    for (TInt index = 0; index < keyCount; index++)
        {
        const TDesC& keyLabel = iKeys[index]->Label();
        if (keyLabel.Compare(aKeyLabel)==0)
            {
            isInStore = ETrue;
            break;
            }
        }

    return (isInStore);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyDataManager::Count().
// ---------------------------------------------------------------------------
//
TInt CDevCertKeyDataManager::Count() const
    {
    return iKeys.Count();
    } 


// ---------------------------------------------------------------------------
// CDevCertKeyDataManager:::operator[](TInt aIndex) const
// ---------------------------------------------------------------------------
//
const CDevCertKeyData* CDevCertKeyDataManager::operator[](TInt aIndex) const
    {
    return iKeys[aIndex];
    }


// ---------------------------------------------------------------------------
// CDevCertKeyDataManager:::Lookup()
// ---------------------------------------------------------------------------
//
const CDevCertKeyData* CDevCertKeyDataManager::Lookup(TInt aObjectId) const
    {
    TInt count = Count();
    for (TInt i = 0; i < count; ++i)
        {
        if ((*this)[i]->Handle() == aObjectId)
            {
            return (*this)[i];
            }
        }
    return NULL;
    }


//  Management of file and store therein

// ---------------------------------------------------------------------------
// CDevCertKeyDataManager:::CreateKeyDataLC()
// ---------------------------------------------------------------------------
//
const CDevCertKeyData* CDevCertKeyDataManager::CreateKeyDataLC( const TDesC& aLabel )
    {
    TInt objectId = ++iKeyIdentifier;
    TStreamId infoData = CreateWriteStreamL();
    TStreamId publicKeyData = CreateWriteStreamL();
    TStreamId privateKeyData = CreateWriteStreamL();
    return CDevCertKeyData::NewLC(objectId, aLabel, infoData, publicKeyData, privateKeyData);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyDataManager:::CreateWriteStreamL()
//  Creates a new write stream in the store (which it then closes)
//  Returns the TStreamId associated with it
// ---------------------------------------------------------------------------
//
TStreamId CDevCertKeyDataManager::CreateWriteStreamL()
    {
    __ASSERT_DEBUG(iFileStore, PanicServer(EPanicStoreInitialised));
    if (!iFileStore)
    User::Leave(KErrNotReady);

    RStoreWriteStream newStream;
    TStreamId result = newStream.CreateLC(*iFileStore);
    if (KNullStreamId == result)
    User::Leave(KErrBadHandle);

    newStream.CommitL();
    CleanupStack::PopAndDestroy(&newStream);

    return result;
    }


// ---------------------------------------------------------------------------
// CDevCertKeyDataManager:::ReadKeyInfoLC()
// ---------------------------------------------------------------------------
//
CDevTokenKeyInfo* CDevCertKeyDataManager::ReadKeyInfoLC(const CDevCertKeyData& aKeyData) const
    {
    __ASSERT_ALWAYS(iFileStore, PanicServer(EPanicStoreInitialised));
    RStoreReadStream stream;
    stream.OpenLC(*iFileStore, aKeyData.InfoDataStreamId());
    CDevTokenKeyInfo* info = CDevTokenKeyInfo::NewL(stream);
    CleanupStack::PopAndDestroy(&stream);
    info->CleanupPushL();
    if (info->Handle() != aKeyData.Handle())
        {
        User::Leave(KErrCorrupt); // is this appropriate?
        }
    return info;
    }


// ---------------------------------------------------------------------------
// CDevCertKeyDataManager:::WriteKeyInfoL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyDataManager::WriteKeyInfoL(const CDevCertKeyData& aKeyData, const CDevTokenKeyInfo& aKeyInfo)
    {
    RStoreWriteStream infoStream;
    OpenInfoDataStreamLC(aKeyData, infoStream);
    infoStream << aKeyInfo;
    infoStream.CommitL();
    CleanupStack::PopAndDestroy(&infoStream);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyDataManager:::SafeWriteKeyInfoL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyDataManager::SafeWriteKeyInfoL(const CDevCertKeyData& aKeyData, const CDevTokenKeyInfo& aKeyInfo)
    {
    TCleanupItem cleanupStore(RevertStore, iFileStore);
    CleanupStack::PushL(cleanupStore);  

    WriteKeyInfoL(aKeyData, aKeyInfo);
    iFileStore->CommitL();

    CleanupStack::Pop(); // cleanupStore  
    }


// ---------------------------------------------------------------------------
// CDevCertKeyDataManager:::OpenInfoDataStreamLC()
// ---------------------------------------------------------------------------
//
void CDevCertKeyDataManager::OpenInfoDataStreamLC(const CDevCertKeyData& aKeyData, RStoreWriteStream& aStream)
    {
    __ASSERT_ALWAYS(iFileStore, PanicServer(EPanicStoreInitialised));
    aStream.ReplaceLC(*iFileStore, aKeyData.InfoDataStreamId());
    }


// ---------------------------------------------------------------------------
// CDevCertKeyDataManager:::OpenPublicDataStreamLC()
// ---------------------------------------------------------------------------
//
void CDevCertKeyDataManager::OpenPublicDataStreamLC(const CDevCertKeyData& aKeyData, RStoreWriteStream& aStream)
    {
    __ASSERT_ALWAYS(iFileStore, PanicServer(EPanicStoreInitialised));
    aStream.ReplaceLC(*iFileStore, aKeyData.PublicDataStreamId());
    }


// ---------------------------------------------------------------------------
// CDevCertKeyDataManager:::OpenPublicDataStreamLC()
// ---------------------------------------------------------------------------
//
void CDevCertKeyDataManager::OpenPublicDataStreamLC(const CDevCertKeyData& aKeyData, RStoreReadStream& aStream) const
    {
    __ASSERT_ALWAYS(iFileStore, PanicServer(EPanicStoreInitialised));
    aStream.OpenLC(*iFileStore, aKeyData.PublicDataStreamId());
    }


// ---------------------------------------------------------------------------
// CDevCertKeyDataManager:::OpenPrivateDataStreamLC()
// ---------------------------------------------------------------------------
//
void CDevCertKeyDataManager::OpenPrivateDataStreamLC(const CDevCertKeyData& aKeyData, 
                          RStoreWriteStream& aStream)
    {
    __ASSERT_DEBUG(iFileStore, PanicServer(EPanicStoreInitialised));
    aStream.ReplaceLC(*iFileStore, aKeyData.PrivateDataStreamId());
    }


// ---------------------------------------------------------------------------
// CDevCertKeyDataManager:::OpenPrivateDataStreamLC()
// ---------------------------------------------------------------------------
//
void CDevCertKeyDataManager::OpenPrivateDataStreamLC(const CDevCertKeyData& aKeyData, 
                          RStoreReadStream& aStream) 
    {
    __ASSERT_DEBUG(iFileStore, PanicServer(EPanicStoreInitialised));
    aStream.OpenLC(*iFileStore, aKeyData.PrivateDataStreamId());
    }


// ---------------------------------------------------------------------------
// CDevCertKeyDataManager:::CompactStore()
// Attempt to compact the store - it doesn't matter if these calls leave, it
// will only mean that the store takes up more space than necessary.
// ---------------------------------------------------------------------------
//
void CDevCertKeyDataManager::CompactStore()
    {
    ASSERT(iFileStore);
    TRAP_IGNORE(iFileStore->ReclaimL(); iFileStore->CompactL());
    }


// CDevCertKeyData

// ---------------------------------------------------------------------------
// CDevCertKeyData::NewLC()
// ---------------------------------------------------------------------------
//
CDevCertKeyData* CDevCertKeyData::NewLC(TInt aObjectId, const TDesC& aLabel, TStreamId aInfoData,
                  TStreamId aPublicData, TStreamId aPrivateData)
    {
    CDevCertKeyData* self = new (ELeave) CDevCertKeyData(aObjectId, aInfoData, aPublicData, aPrivateData);
    CleanupStack::PushL(self);
    self->ConstructL(aLabel);
    return self;
    }


// ---------------------------------------------------------------------------
// CDevCertKeyData::NewL()
// ---------------------------------------------------------------------------
//
CDevCertKeyData* CDevCertKeyData::NewL(RStoreReadStream& aReadStream)
    {
    CDevCertKeyData* self = new (ELeave) CDevCertKeyData();
    CleanupStack::PushL(self);
    self->InternalizeL(aReadStream);
    CleanupStack::Pop(self);
    return (self);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyData::~CDevCertKeyData()
// ---------------------------------------------------------------------------
//
CDevCertKeyData::~CDevCertKeyData()
    {
    delete iLabel;
    }


// ---------------------------------------------------------------------------
// CDevCertKeyData::CDevCertKeyData()
// ---------------------------------------------------------------------------
//
CDevCertKeyData::CDevCertKeyData(TInt aObjectId, TStreamId aInfoData,
               TStreamId aPublicData, TStreamId aPrivateData) :
  iObjectId(aObjectId), iInfoData(aInfoData), 
  iPublicKeyData(aPublicData), iPrivateKeyData(aPrivateData)
    {
    ASSERT(iObjectId);
    ASSERT(iInfoData != KNullStreamId);
    ASSERT(iPublicKeyData != KNullStreamId);
    ASSERT(iPrivateKeyData != KNullStreamId);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyData::CDevCertKeyData()
// ---------------------------------------------------------------------------
//
CDevCertKeyData::CDevCertKeyData()
    {
    }


// ---------------------------------------------------------------------------
// CDevCertKeyData::ConstructL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyData::ConstructL(const TDesC& aLabel)
    {
    TInt labelLen = aLabel.Length();
    iLabel = HBufC::NewMaxL(labelLen);
    TPtr theLabel(iLabel->Des());
    theLabel.FillZ();
    theLabel.Copy(aLabel);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyData::InternalizeL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyData::InternalizeL(RReadStream& aReadStream)
    {
    iObjectId = aReadStream.ReadInt32L();
    iInfoData.InternalizeL(aReadStream);
    iPublicKeyData.InternalizeL(aReadStream);
    iPrivateKeyData.InternalizeL(aReadStream);

    TInt labelLen = aReadStream.ReadInt32L();
    iLabel = HBufC::NewMaxL(labelLen);
    TPtr theLabel((TUint16*)iLabel->Ptr(), labelLen, labelLen);
    theLabel.FillZ(labelLen);
    aReadStream.ReadL(theLabel);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyData::ExternalizeL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyData::ExternalizeL(RWriteStream& aWriteStream) const
    {
    aWriteStream.WriteInt32L(iObjectId);
    iInfoData.ExternalizeL(aWriteStream);
    iPublicKeyData.ExternalizeL(aWriteStream);
    iPrivateKeyData.ExternalizeL(aWriteStream);

    TInt labelLen = iLabel->Length();
    aWriteStream.WriteInt32L(labelLen);
    TPtr theLabel(iLabel->Des());
    theLabel.SetLength(labelLen);
    aWriteStream.WriteL(theLabel);
    }

//EOF

