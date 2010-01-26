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
* Description:   The header file of DevandTruSrvCertStoreEntryList
*
*/



#ifndef __DEVANDTRUSRVCERTSTOREENTRYLIST_H__
#define __DEVANDTRUSRVCERTSTOREENTRYLIST_H__

#include <s32file.h>

class CDevandTruSrvCertStoreEntry;

/**
 * An in-memory list of the certs in the store
 *
 * @lib
 * @since S60 v3.2.
 */
class CDevandTruSrvCertStoreEntryList : public CBase
    {
    public:
    
        // Create a new, empty cert list
        static CDevandTruSrvCertStoreEntryList* NewLC();

        // Read the cert list from a stream
        static CDevandTruSrvCertStoreEntryList* NewL(RReadStream& aStream);

        virtual ~CDevandTruSrvCertStoreEntryList();

        // Write the cert list to a stream
        void ExternalizeL(RWriteStream& aStream) const;

        // Get the number of certificates in the list
        TInt Count() const;

        /**
         * Get the index of a mapping given the cert handle
         *
         * @param aHandle 
         * @return The index of the cert or KErrNotFound if it is not present
         */
        TInt IndexForHandle(TInt aHandle) const;

        // Determine whether a certificate exists with the specified label
        TBool LabelExists(const TDesC& aLabel) const;

        /**
         * Get a mapping given the cert handle
         * leave KErrNotFound If the specified cert is not present
         * 
         * @param aHandle 
         * @return 
         */
        const CDevandTruSrvCertStoreEntry& GetByHandleL(TInt aHandle) const;

        // Get a mapping given its index.
        const CDevandTruSrvCertStoreEntry& GetByIndex(TInt aIndex) const;

        // Get the next free handle to use when adding a cert
        TInt NextFreeHandle() const;

        // Add a mapping and return its index, takes ownership
        TInt AppendL(CDevandTruSrvCertStoreEntry* aEntry);

        // Removes and returns an entry from the list, relinquishes ownership
        CDevandTruSrvCertStoreEntry* Remove(TInt aIndex);

        // Replace an entry and return the old one, takes/relinquishes ownership
        CDevandTruSrvCertStoreEntry* Replace(TInt aIndex, CDevandTruSrvCertStoreEntry* aEntry); 

    private:
        
        CDevandTruSrvCertStoreEntryList();
        
        void InternalizeL(RReadStream& aStream);

    private:
        
        RPointerArray<CDevandTruSrvCertStoreEntry> iEntries;
    };

#endif //__DEVANDTRUSRVCERTSTOREENTRYLIST_H__

//EOF

