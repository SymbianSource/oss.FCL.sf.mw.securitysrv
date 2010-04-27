/*
* Copyright (c) 2006-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   Implementation of DevandTruSrvCertStoreServer
*
*/


#include <ccertattributefilter.h>
#include <signed.h>
#include <x509cert.h>
#include <x509certext.h>
#include <x509keys.h>
#include <wtlscert.h>
#include <x500dn.h>
#include "DevandTruSrvCertStoreServer.h"
#include "DevandTruSrvCertStoreConduit.h"
#include "DevandTruSrvCertStoreSession.h"
#include "DevandTruSrvCertStoreEntry.h"
#include "DevandTruSrvCertStoreEntryList.h"
#include "DevTokenCliServ.h"
#include "DevTokenUtil.h"
#include "DevTokenDataTypes.h"
#include "TrustedSitesServer.h"

_LIT(KCertStoreFilename,"devandtrusrvcerts.dat");

const TInt KSHA1Length = 20;

// API policing 
_LIT_SECURITY_POLICY_PASS(KPolicyAlwaysPass);
_LIT_SECURITY_POLICY_FAIL(KPolicyAlwaysFail);
_LIT_SECURITY_POLICY_C1(KPolicyRequireReadUserData, ECapabilityReadUserData);
_LIT_SECURITY_POLICY_C1(KPolicyRequireWriteDeviceData, ECapabilityWriteDeviceData);

// ======== MEMBER FUNCTIONS ========

//CDevandTruSrvCertStoreServer

// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreServer::NewL()
// ---------------------------------------------------------------------------
//
CDevandTruSrvCertStoreServer* CDevandTruSrvCertStoreServer::NewL(CTrustedSitesServer* aTrustedSitesServer )
    {
    CDevandTruSrvCertStoreServer* self = new (ELeave) CDevandTruSrvCertStoreServer( aTrustedSitesServer );
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreServer::CDevandTruSrvCertStoreServer()
// ---------------------------------------------------------------------------
//
CDevandTruSrvCertStoreServer::CDevandTruSrvCertStoreServer( CTrustedSitesServer* aTrustedSitesServer )
: iTrustedSitesServer( aTrustedSitesServer )
    {
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreServer::ConstructL()
// ---------------------------------------------------------------------------
//
void CDevandTruSrvCertStoreServer::ConstructL()
    {
    iConduit = CDevandTruSrvCertStoreConduit::NewL(*this);
    User::LeaveIfError(iFs.Connect());
    OpenStoreL();
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreServer::~CDevandTruSrvCertStoreServer()
// ---------------------------------------------------------------------------
//
CDevandTruSrvCertStoreServer::~CDevandTruSrvCertStoreServer()
    {
    delete iStore;
    delete iEntryList;
    delete iConduit;
    iFs.Close();
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreServer::CreateSessionL()
// ---------------------------------------------------------------------------
//
CDevandTruSrvCertStoreSession* CDevandTruSrvCertStoreServer::CreateSessionL()
    {
    return CDevandTruSrvCertStoreSession::NewL(*iConduit);
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreServer::ListPolicy()
// ---------------------------------------------------------------------------
//
const TSecurityPolicy& CDevandTruSrvCertStoreServer::ListPolicy(TCertificateOwnerType aOwnerType) const
    {
    switch (aOwnerType)
        {
        case EUserCertificate:
            return KPolicyAlwaysPass;
        case EPeerCertificate:
            return KPolicyRequireReadUserData;

        default:
            return KPolicyAlwaysFail;
        }
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreServer::AddRemovePolicy()
// ---------------------------------------------------------------------------
//
const TSecurityPolicy& CDevandTruSrvCertStoreServer::AddRemovePolicy(TCertificateOwnerType aOwnerType) const
    {
    switch (aOwnerType)
        {
        case EUserCertificate:
        case EPeerCertificate:
            return KPolicyRequireWriteDeviceData;

        default:
            return KPolicyAlwaysFail;
        }
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreServer::RetrievePolicy()
// ---------------------------------------------------------------------------
//
const TSecurityPolicy& CDevandTruSrvCertStoreServer::RetrievePolicy(TCertificateOwnerType aOwnerType) const
    {
    switch (aOwnerType)
        {
        case EUserCertificate:
        case EPeerCertificate:
            return KPolicyRequireReadUserData;

        default:
            return KPolicyAlwaysFail;
        }
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreServer::WriteTrustSettingsPolicy()
// ---------------------------------------------------------------------------
//
const TSecurityPolicy& CDevandTruSrvCertStoreServer::WriteTrustSettingsPolicy() const
    {
    return KPolicyRequireWriteDeviceData;
    }


// Read-only interface 

// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreServer::ListL()
// ---------------------------------------------------------------------------
//
void CDevandTruSrvCertStoreServer::ListL(const CCertAttributeFilter& aFilter,
                 RPointerArray<CDevTokenCertInfo>& aCertsOut,
                 const RMessage2& aMessage, TBool aFromTruSiteSrv ) const
    {

    // check that if KeyUsage is set, only User certificates are requested
    if (aFilter.iKeyUsage != EX509UsageAll &&
    (!aFilter.iOwnerTypeIsSet || aFilter.iOwnerType != EUserCertificate))
        {
        User::Leave(KErrArgument);
        }

    // API policing
    if (!aFromTruSiteSrv)
        {
        if (!ListPolicy( aFilter.iOwnerType ).CheckPolicy(aMessage))
            {
            User::Leave(KErrPermissionDenied);
            } 	
        } 

    TInt count = iEntryList->Count();
    for (TInt index = 0; index < count; index++)
        {
        const CDevandTruSrvCertStoreEntry& entry = iEntryList->GetByIndex(index);

        if (CertEntryMatchesFilter(aFilter, entry))
            {
            User::LeaveIfError(aCertsOut.Append(&entry.CertInfo()));
            }
        }
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreServer::CertEntryMatchesFilter()
// ---------------------------------------------------------------------------
//
TBool CDevandTruSrvCertStoreServer::CertEntryMatchesFilter(const CCertAttributeFilter& aFilter,
                         const CDevandTruSrvCertStoreEntry& aEntry) const
    {
    if (aFilter.iUidIsSet && !aEntry.IsApplicable(aFilter.iUid))
        {
        return EFalse;
        }

    const CDevTokenCertInfo& certInfo = aEntry.CertInfo();

    if (aFilter.iFormatIsSet && aFilter.iFormat != certInfo.CertificateFormat())
        {
        return EFalse;
        }

    if (aFilter.iOwnerTypeIsSet && aFilter.iOwnerType != certInfo.CertificateOwnerType())
        {
        return EFalse;
        }

    if (aFilter.iSubjectKeyIdIsSet && aFilter.iSubjectKeyId != certInfo.SubjectKeyId())
        {
        return EFalse;
        }

    if (aFilter.iLabelIsSet && aFilter.iLabel != certInfo.Label())
        {
        return EFalse;
        }

    return ETrue;
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreServer::GetCertL()
// ---------------------------------------------------------------------------
//
const CDevTokenCertInfo& CDevandTruSrvCertStoreServer::GetCertL(TInt aHandle) const 
    {
    // Leaves if not found
    return iEntryList->GetByHandleL(aHandle).CertInfo();
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreServer::ApplicationsL()
// ---------------------------------------------------------------------------
//
const RArray<TUid>& CDevandTruSrvCertStoreServer::ApplicationsL(TInt aHandle) const
    {
    return iEntryList->GetByHandleL(aHandle).CertificateApps();
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreServer::IsApplicableL()
// ---------------------------------------------------------------------------
//
TBool CDevandTruSrvCertStoreServer::IsApplicableL(TInt aHandle, TUid aApplication) const
    {
    return iEntryList->GetByHandleL(aHandle).IsApplicable(aApplication);
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreServer::TrustedL()
// ---------------------------------------------------------------------------
//
TBool CDevandTruSrvCertStoreServer::TrustedL(TInt aHandle) const
    { 
    return iEntryList->GetByHandleL(aHandle).Trusted();
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreServer::RetrieveLC()
// ---------------------------------------------------------------------------
//
HBufC8* CDevandTruSrvCertStoreServer::RetrieveLC(TInt aHandle, const RMessage2& aMessage, TBool aFromTruSiteSrv) const
    {
    const CDevandTruSrvCertStoreEntry& entry = iEntryList->GetByHandleL(aHandle);

    // API policing
    if ( !aFromTruSiteSrv )
        {
        if (!RetrievePolicy(entry.CertInfo().CertificateOwnerType()).CheckPolicy(aMessage))
            {
            User::Leave( KErrPermissionDenied );
            }	
        }

    TInt size = entry.CertInfo().Size();
    HBufC8* buf = HBufC8::NewMaxLC(size);
    TPtr8 ptr = buf->Des();
    ptr.FillZ();

    RStoreReadStream stream;
    stream.OpenLC(*iStore, entry.DataStreamId());
    stream.ReadL(ptr, size);
    CleanupStack::PopAndDestroy(&stream);

    return buf;
    }


// Writable interface

// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreServer::AddL()
// ---------------------------------------------------------------------------
//
void CDevandTruSrvCertStoreServer::AddL(const TDevTokenAddCertDataStruct& aInfo,
                const TDesC8& aCert,
                const RMessage2& aMessage, TBool aFromTruSiteSrv )
    {
    // Check subject key id and cert data are supplied, issuer key id is optional
    if (aInfo.iSubjectKeyId == KNullDesC8 || aCert == KNullDesC8)
        {
        User::Leave(KErrArgument);
        }

    // Create cert entry (this sanity checks the rest of the arguments)
    CDevTokenCertInfo* certInfo = CDevTokenCertInfo::NewLC(
        aInfo.iLabel,
        aInfo.iFormat, 
        aInfo.iCertificateOwnerType,
        aCert.Length(),
        &aInfo.iSubjectKeyId,
        &aInfo.iIssuerKeyId,
        iEntryList->NextFreeHandle(),
        aInfo.iDeletable);

    // API policing
    if ( !aFromTruSiteSrv )
        {
        if (!AddRemovePolicy(aInfo.iCertificateOwnerType).CheckPolicy(aMessage))
            {
            User::Leave(KErrPermissionDenied);
            }	
        }

    CompactStoreL();

    TRAPD(err, DoAddL(*certInfo, aCert));
    CleanupStack::PopAndDestroy(certInfo);

    if (err != KErrNone)
        {
        iStore->Revert();
        User::Leave(err);
        }
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreServer::DoAddL()
// ---------------------------------------------------------------------------
//
void CDevandTruSrvCertStoreServer::DoAddL(const CDevTokenCertInfo& aCertInfo, const TDesC8& aCertData)
    {
    TStreamId dataStreamId = WriteCertDataStreamL(aCertData);

    RArray<TUid> initialApps;
    CleanupClosePushL(initialApps);

    CDevandTruSrvCertStoreEntry* entry = CDevandTruSrvCertStoreEntry::NewL(
        aCertInfo,
        initialApps,
        EFalse,
        dataStreamId);
    CleanupStack::PopAndDestroy(&initialApps);
    CleanupStack::PushL(entry);

    TInt index = iEntryList->AppendL(entry);
    CleanupStack::Pop(entry); // iEntryList has taken ownership

    TRAPD(err, UpdateStoreL());
    if ( (err == KErrNone) && (aCertInfo.CertificateOwnerType() == EPeerCertificate ))
        {
        CX509Certificate* cert = CX509Certificate::NewLC( aCertData );
        TBuf8<KSHA1Length> certHash;
        certHash.Copy(cert->Fingerprint());

        const CX500DistinguishedName& dName = cert->SubjectName();
        // Retrieve CN
        HBufC* cn = dName.ExtractFieldL( KX520CommonName );
        if( cn )
            {
            CleanupStack::PushL( cn );
            iTrustedSitesServer->AddL( certHash, *cn );
            CleanupStack::PopAndDestroy( cn );
            }

        CleanupStack::PopAndDestroy( cert );
        }
    if (err != KErrNone)
        {
        iEntryList->Remove(index);
        delete entry;
        User::Leave(err);
        }
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreServer::WriteCertDataStreamL()
// ---------------------------------------------------------------------------
//
TStreamId CDevandTruSrvCertStoreServer::WriteCertDataStreamL(const TDesC8& aData)
    {
    RStoreWriteStream stream;
    TStreamId streamId = stream.CreateLC(*iStore);
    stream.WriteL(aData);
    stream.CommitL();
    CleanupStack::PopAndDestroy(&stream);
    return streamId;
    } 


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreServer::RemoveL()
// ---------------------------------------------------------------------------
//
void CDevandTruSrvCertStoreServer::RemoveL(TInt aHandle, const RMessage2& aMessage,  TBool aFromTruSiteSrv)
    {
    TInt index = iEntryList->IndexForHandle(aHandle);
    User::LeaveIfError(index);

    // API policing
    const CDevandTruSrvCertStoreEntry& entry = iEntryList->GetByIndex(index);
    if (!aFromTruSiteSrv)
        {
        if (!AddRemovePolicy(entry.CertInfo().CertificateOwnerType()).CheckPolicy(aMessage))
            {
            User::Leave(KErrPermissionDenied);
            }	
        }

    CompactStoreL();

    CDevandTruSrvCertStoreEntry* oldEntry = iEntryList->Remove(index);
    TRAPD(err, UpdateStoreL());
    if (err == KErrNone)
        {
        // If trusted site certificate is removed, the corresponding records
        // in trusted site store should also be removed to stop trusting.
        if( oldEntry->CertInfo().CertificateOwnerType() == EPeerCertificate )
            {
            TInt size = oldEntry->CertInfo().Size();
            HBufC8* buf = HBufC8::NewMaxLC(size);
            TPtr8 ptr = buf->Des();
            ptr.FillZ();

            RStoreReadStream stream;
            stream.OpenLC(*iStore, oldEntry->DataStreamId());
            stream.ReadL(ptr, size);
            CleanupStack::PopAndDestroy(&stream);


            CX509Certificate* cert = CX509Certificate::NewLC(*buf);
            TBuf8<KSHA1Length> certHash;
            certHash.Copy(cert->Fingerprint()); 

            iTrustedSitesServer->RemoveL( certHash );

            CleanupStack::PopAndDestroy( 2 );//buf, cert
            }
        delete oldEntry;
        }
    else
        {
        iStore->Revert();
        // This will always succeed because we just did a remove
        iEntryList->AppendL(oldEntry);
        }
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreServer::SetApplicabilityL()
// ---------------------------------------------------------------------------
//
void CDevandTruSrvCertStoreServer::SetApplicabilityL(TInt /*aHandle*/,
                       const RArray<TUid>& /*aApps*/,
                       const RMessage2& aMessage)
    {
    // Setting Applicability is meant for CA certificate only.
    // in DevandTruSrvCertStore, we only store Personal ceritificate(Device Cert )
    // and Peer certificate( trust server cert ).
    
    aMessage.Complete( KErrNotSupported ); 
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreServer::SetTrustL()
// ---------------------------------------------------------------------------
//
void CDevandTruSrvCertStoreServer::SetTrustL(TInt /*aHandle*/,
                   TBool /*aTrusted*/,
                   const RMessage2& aMessage)
    {
    // Setting Trust is meant for CA certificate only.
    // in DevandTruSrvCertStore, we only store Personal ceritificate(Device Cert )
    // and Peer certificate( trust server cert ).
    
    aMessage.Complete( KErrNotSupported ); 
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreServer::UpdateStoreL()
// ---------------------------------------------------------------------------
//
void CDevandTruSrvCertStoreServer::UpdateStoreL()
    {
    RStoreWriteStream stream;
    stream.ReplaceLC(*iStore, iStreamId);
    stream << *iEntryList;
    stream.CommitL();
    CleanupStack::PopAndDestroy(&stream);

    iStore->CommitL();
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreServer::CompactStoreL()
// ---------------------------------------------------------------------------
//
void CDevandTruSrvCertStoreServer::CompactStoreL()
    {
    iStore->ReclaimL();
    iStore->CompactL();
    iStore->CommitL();
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreServer::RestoreL()
// ---------------------------------------------------------------------------
//
void CDevandTruSrvCertStoreServer::RestoreL(const TDesC& aFilename)
    {
    // Make sure the store is not read-only
    User::LeaveIfError(iFs.SetAtt(aFilename, KEntryAttNormal, KEntryAttReadOnly));

    // Open the store
    RFile file;
    User::LeaveIfError(file.Open(iFs, aFilename, EFileRead | EFileWrite));
    CleanupClosePushL(file);
    CPermanentFileStore* store = CPermanentFileStore::FromL(file);
    CleanupStack::Pop(&file); // now owned by store
    CleanupStack::PushL(store);

    // Read id of cert list stream
    TStreamId caCertEntryStreamId;
    RStoreReadStream stream;
    stream.OpenLC(*store, store->Root());
    stream >> iStreamId;
    CleanupStack::PopAndDestroy(&stream);

    // Read the certificate list
    RStoreReadStream caCertEntryStream;
    caCertEntryStream.OpenLC(*store, iStreamId);
    iEntryList = CDevandTruSrvCertStoreEntryList::NewL(caCertEntryStream);
    CleanupStack::PopAndDestroy(&caCertEntryStream);

    assert(!iStore, EPanicCertStoreRestoreState);
    iStore = store;
    CleanupStack::Pop(store);
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreServer::OpenStoreL()
// ---------------------------------------------------------------------------
//
void CDevandTruSrvCertStoreServer::OpenStoreL()
    {
    TBuf<KMaxFilenameLength> filename;
    FileUtils::MakePrivateFilenameL(iFs, KCertStoreFilename, filename);

    TRAPD(err, RestoreL(filename));

    if (err == KErrNoMemory || err == KErrInUse)
        {
        User::Leave(err);
        }

    if (err != KErrNone)
        {
        // Couldn't open RAM based store, copy from ROM 

        FileUtils::EnsurePathL(iFs, filename);

        TBuf<KMaxFilenameLength> romFilename;
        FileUtils::MakePrivateROMFilenameL(iFs, KCertStoreFilename, romFilename);

        if (FileUtils::ExistsL(iFs, romFilename))
            {
            FileUtils::CopyL(iFs, romFilename, filename);
            }
        else
            {
            CreateStoreFileL(filename);
            }

        // Retry open, and leave on failure
        RestoreL(filename);
        }

    assert(iStore, EPanicCertStoreOpenState);
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreServer::CreateStoreFileL()
// ---------------------------------------------------------------------------
//
void CDevandTruSrvCertStoreServer::CreateStoreFileL(const TDesC& aFile)
    {
    TRAPD(err, DoCreateStoreFileL(aFile));
    if (err != KErrNone)
        {
        // Attempt to delete file, but don't complain if it fails
        iFs.Delete(aFile);  
        User::Leave(err);
        }
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreServer::DoCreateStoreFileL()
// ---------------------------------------------------------------------------
//
void CDevandTruSrvCertStoreServer::DoCreateStoreFileL(const TDesC& aFile)
    {
    CPermanentFileStore* fileStore = CPermanentFileStore::ReplaceLC(iFs, aFile, EFileRead | EFileWrite | EFileShareExclusive);
    fileStore->SetTypeL(KPermanentFileStoreLayoutUid);

    // Create info stream
    CDevandTruSrvCertStoreEntryList* emptyCertList = CDevandTruSrvCertStoreEntryList::NewLC();  
    RStoreWriteStream infoStream;
    TStreamId streamId = infoStream.CreateLC(*fileStore);
    infoStream << *emptyCertList;
    infoStream.CommitL();
    CleanupStack::PopAndDestroy(2, emptyCertList);

    // Create root stream - just contains id of info stream
    RStoreWriteStream rootStream;
    TStreamId rootStreamId = rootStream.CreateLC(*fileStore);
    fileStore->SetRootL(rootStreamId);
    rootStream << streamId;
    rootStream.CommitL();
    CleanupStack::PopAndDestroy(&rootStream);

    fileStore->CommitL();
    CleanupStack::PopAndDestroy(fileStore);
    }

//EOF

