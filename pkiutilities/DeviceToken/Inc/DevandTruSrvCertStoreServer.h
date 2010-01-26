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
* Description:   The header file of DevandTruSrvCertStoreServer
*
*/



#ifndef __DEVANDTRUSRVCERTSTORESERVER_H__
#define __DEVANDTRUSRVCERTSTORESERVER_H__

#include <s32file.h>
#include <e32std.h>
#include <securitydefs.h>

class CDevTokenCertInfo;
class CDevandTruSrvCertStoreSession;
class CDevandTruSrvCertStoreConduit;
class CDevandTruSrvCertStoreEntry;
class CDevandTruSrvCertStoreEntryList;
class CCertAttributeFilter;
class CTrustedSitesServer;

struct TDevTokenAddCertDataStruct;

/**
 *  class CDevandTruSrvCertStoreServer
 *
 * This class implements a certificate store which is stored on a file on the
 * device.  NewL calls the constructor and ConstructL. The constructor does
 * nothing except initializing to 0. ConstructL calls OpenStoreL, then RestoreL
 * and then sets the values of iStoreInfo.  OpenStoreL opens the store and
 * copies it from the ROM if necessary.  RestoreL()
 *
 *  @lib 
 *  @since S60 3.2
 */
class CDevandTruSrvCertStoreServer : public CBase
    {
    public:
        
        static CDevandTruSrvCertStoreServer* NewL( CTrustedSitesServer* aTrustedSitesServer);
        
        virtual ~CDevandTruSrvCertStoreServer();
        
        CDevandTruSrvCertStoreSession* CreateSessionL();

    public:
        
        // List certs, returned pointers are owned by this class
        void ListL(const CCertAttributeFilter& aFilter, RPointerArray<CDevTokenCertInfo>& aCertsOut, const RMessage2& aMessage, TBool aFromTruSiteSrv ) const;
        
        // Get cert info, returned pointer owned by this class, leaves if not found
        const CDevTokenCertInfo& GetCertL(TInt aHandle) const;
        
        const RArray<TUid>& ApplicationsL(TInt aHandle) const;  
        
        TBool IsApplicableL(TInt aHandle, TUid aApplication) const;
        
        TBool TrustedL(TInt aHandle) const;
        
        HBufC8* RetrieveLC(TInt aHandle, const RMessage2& aMessage, TBool aFromTruSiteSrv ) const;
        
        void AddL(const TDevTokenAddCertDataStruct& aInfo,
                  const TDesC8& aCert,
                  const RMessage2& aMessage, TBool aFromTruSiteSrv );
        
        void RemoveL(TInt aHandle, const RMessage2& aMessage, TBool aFromTruSiteSrv );
        
        void SetApplicabilityL(TInt aHandle,
                               const RArray<TUid>& aApplications,
                               const RMessage2& aMessage);
        
        void SetTrustL(TInt aHandle,
                       TBool aTrusted,
                       const RMessage2& aMessage);

    private:
        
        CDevandTruSrvCertStoreServer( CTrustedSitesServer* aTrustedSitesServer );
        
        void ConstructL();
        
        // Security policy
        const TSecurityPolicy& AddRemovePolicy(TCertificateOwnerType aOwnerType) const;
        
        const TSecurityPolicy& RetrievePolicy(TCertificateOwnerType aOwnerType) const;
        
        const TSecurityPolicy& ListPolicy(TCertificateOwnerType aOwnerType) const;
        
        const TSecurityPolicy& WriteTrustSettingsPolicy() const;
        
        TBool CertEntryMatchesFilter(const CCertAttributeFilter& aFilter,
        
        const CDevandTruSrvCertStoreEntry& aEntry) const;
        
        void DoAddL(const CDevTokenCertInfo& aCertInfo, const TDesC8& aCertData);
        
        TStreamId WriteCertDataStreamL(const TDesC8& aData);
        
        // Write the cert entry list to the store
        void UpdateStoreL();
        
        void CompactStoreL();
        
        // Open the store and load the index from a file
        void RestoreL(const TDesC& aFilename);
        
        // Open the store file, copying from ROM or creating it first if necessary
        void OpenStoreL();
        
        // Create an empty store file, and leave it closed
        void CreateStoreFileL(const TDesC& aFile);
        
        void DoCreateStoreFileL(const TDesC& aFile);

    private:
        
        // Conduit for marshalling/unmarshalling client communications
        
        CDevandTruSrvCertStoreConduit* iConduit;
        
        RFs iFs;
        
        CPermanentFileStore* iStore;
        
        // The list of certificates contained in the store.
        
        CDevandTruSrvCertStoreEntryList* iEntryList;
        
        // The store stream containing the list of certs
        TStreamId iStreamId;
        
        CTrustedSitesServer* iTrustedSitesServer; //Not owned;
    };

#endif  //__DEVANDTRUSRVCERTSTORESERVER_H__

//EOF

