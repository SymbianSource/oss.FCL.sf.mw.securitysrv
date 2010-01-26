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
* Description:   The header file of DevCertKeyDataManager
*
*/



#ifndef __DEVCERTKEYDATAMANAGER_H__
#define __DEVCERTKEYDATAMANAGER_H__

#include <s32file.h>
#include <ct/tcttokenobjecthandle.h>
#include <ct.h>

class CDevTokenKeyInfo;

/**
 *  class CDevCertKeyData
 *
 * Server side in-memory representation of a key for file key store management
 * purposes. Objects of this type are stored in an in-memory array for quick
 * keystore lookup
 *
 *  @lib
 *  @since S60 v3.2
 */
class CDevCertKeyData : public CBase
    {
    public:
        
        // Create a new key data object
        static CDevCertKeyData* NewLC(TInt aObjectId, const TDesC& aLabel, TStreamId aInfoData,
        
        TStreamId aPublicData, TStreamId aPrivateData);
        
        // Read a key data object from a stream
        static CDevCertKeyData* NewL(RStoreReadStream& aReadStream);
        
    public:
        
        ~CDevCertKeyData(); 
        
    public:
        
        // Write out the key data to a stream
        void ExternalizeL(RWriteStream&) const;
        
    public:
        
        inline const TDesC& Label() const;
        
        inline TInt32 Handle() const;
        
        inline TStreamId InfoDataStreamId() const;
        
        inline TStreamId PublicDataStreamId() const;
        
        inline TStreamId PrivateDataStreamId() const;
        
    private:
        
        CDevCertKeyData(TInt aObjectId, TStreamId aInfoData,
        
        TStreamId aPublicData, TStreamId aPrivateData);
        
        CDevCertKeyData();
        
        void ConstructL(const TDesC& aLabel);
        
        void InternalizeL(RReadStream&);
        
    private:
        
        TInt iObjectId;       // Data to identify the key
        
        TStreamId iInfoData;    // ID of stream holding publicly available data for key 
        
        TStreamId iPublicKeyData; // ID of stream holding public key data
        
        TStreamId iPrivateKeyData;  // ID of stream holding private key data
        
        HBufC* iLabel;        // Key label data
    };

inline const TDesC& CDevCertKeyData::Label() const
    {
    return *iLabel;
    }

inline TInt32 CDevCertKeyData::Handle() const
    {
    return iObjectId;
    }

inline TStreamId CDevCertKeyData::InfoDataStreamId() const
    {
    return iInfoData;
    }

inline TStreamId CDevCertKeyData::PublicDataStreamId() const
    {
    return iPublicKeyData;
    }

inline TStreamId CDevCertKeyData::PrivateDataStreamId() const
    {
    return iPrivateKeyData;
    }


/**
 *  class CDevCertKeyDataManager
 *
 * Access the server file store of all keys and key data. The only class to
 * access the store, which maintains store integrity When a new key is created,
 * it is represented by a CDevCertKeyData object and added to the array.  
 *
 *  @lib
 *  @since S60 v3.2
 */
class CDevCertKeyDataManager : public CBase
    {
    public:
        
        static CDevCertKeyDataManager* NewL();
        
        ~CDevCertKeyDataManager();
        
    public: 
        
        void AddL(const CDevCertKeyData*);
        
        void RemoveL(TInt aObjectId);
        
        TBool IsKeyAlreadyInStore(const TDesC& aKeyLabel) const;
        
    public:
        
        // Create a new key data object for a key create/import and leave it one the cleanup stack
        const CDevCertKeyData* CreateKeyDataLC( const TDesC& aLabel );
        
        // Reads the info data for a given key, returning a new CKeyInfo that's on the cleanup stack
        CDevTokenKeyInfo* ReadKeyInfoLC(const CDevCertKeyData& aKeyData) const;
        
        // Writes key info data for a key
        void WriteKeyInfoL(const CDevCertKeyData& aKeyData, const CDevTokenKeyInfo& aKeyInfo);
        
        // Writes key info data and reverts changes to the store if if leaves
        void SafeWriteKeyInfoL(const CDevCertKeyData& aKeyData, const CDevTokenKeyInfo& aKeyInfo);
        
        // Methods for opening data streams for a key
        void OpenPublicDataStreamLC(const CDevCertKeyData& aKeyData, RStoreWriteStream& aStream);
        
        void OpenPublicDataStreamLC(const CDevCertKeyData& aKeyData, RStoreReadStream& aStream) const;
        
        void OpenPrivateDataStreamLC(const CDevCertKeyData& aKeyData, RStoreWriteStream& aStream);
        
        void OpenPrivateDataStreamLC(const CDevCertKeyData& aKeyData, RStoreReadStream& aStream);

    public:
        
        TInt Count() const;
        
        const CDevCertKeyData* operator[](TInt aIndex) const;
        
        const CDevCertKeyData* Lookup(TInt aObjectId) const;

    private:
        
        CDevCertKeyDataManager();
        
        void ConstructL();
        
    private:                      //  Manages access to store
        
        void OpenStoreL();
        
        void OpenStoreInFileL(const TDesC& aFile);
        
        void CreateStoreInFileL(const TDesC& aFile);
        
    private:
        
        void OpenInfoDataStreamLC(const CDevCertKeyData& aKeyData, RStoreWriteStream&);
        
        static void RevertStore(TAny* aStore);      //  Cleanupitem
        
        void WriteKeysToStoreL();
        
        TStreamId CreateWriteStreamL();
        
        void CompactStore();
        
    private:
        
        RFile iFile;
        
        RFs iFs;
        
        CPermanentFileStore* iFileStore;
        
        TStreamId iRootStreamId;      // Root of the store
        
        TStreamId iInfoStreamId;      // Stream that contains list of key data

    private:
        
        TInt iKeyIdentifier;
        
        RPointerArray<const CDevCertKeyData> iKeys; // In memory representation of keys in the store
    };

#endif

//EOF

