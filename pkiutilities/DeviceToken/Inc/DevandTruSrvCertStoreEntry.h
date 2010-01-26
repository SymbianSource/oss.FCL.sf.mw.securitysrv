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
* Description:   The header file of DevandTruSrvCertStoreEntry
*
*/



#ifndef __DEVANDTRUSRVCERTSTOREENRY_H__
#define __DEVANDTRUSRVCERTSTOREENRY_H__

#include <e32base.h>
#include <s32std.h>

class CDevTokenCertInfo;

/**
 *  class CDevandTruSrvCertStoreEntry
 *
 * This class represents a cert in the store.  It contains the cert info, the id
 * of the stream containing the certificate itself and its applications and
 * trust settings.
 *
 *  @lib 
 *  @since S60 v3.2
 */
class CDevandTruSrvCertStoreEntry : public CBase
    {
    public:
        
        // Create a new CDevTokenCertStoreEntry
        static CDevandTruSrvCertStoreEntry* NewL(const CDevTokenCertInfo& aEntry,
                                                 RArray<TUid> aCertificateApps,
                                                 TBool aTrusted,
                                                 TStreamId aDataStreamId);
        
        // Read a CDevTokenCertStoreEntry from a stream.
        static CDevandTruSrvCertStoreEntry* NewLC(RReadStream& aStream);
        
        virtual ~CDevandTruSrvCertStoreEntry();
        
        // Write a CDevTokenCertStoreEntry to a stream.
        void ExternalizeL(RWriteStream& aDataStream) const;
        
        // Accessors
        const CDevTokenCertInfo& CertInfo() const;
        
        TStreamId DataStreamId() const;
        
        const RArray<TUid>& CertificateApps() const;
        
        TBool IsApplicable(const TUid& aApplication) const;
        
        TBool Trusted() const;

    private:
        
        CDevandTruSrvCertStoreEntry();
        
        void ConstructL(const CDevTokenCertInfo& aEntry, RArray<TUid> aCertificateApps,
        
        TBool aTrusted, TStreamId aDataStreamId);
        
        void InternalizeL(RReadStream& aStream);

    private:
        
        CDevTokenCertInfo* iCertInfo;
        
        RArray<TUid> iCertificateApps;
        
        TBool iTrusted;
        
        TStreamId iDataStreamId;
    };

#endif __DEVANDTRUSRVCERTSTOREENRY_H__

//EOF

