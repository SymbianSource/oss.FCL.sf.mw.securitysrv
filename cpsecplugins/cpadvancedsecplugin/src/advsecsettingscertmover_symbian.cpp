/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0""
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  Helper class to move certificates
*
*/

#include "advsecsettingscertmover_symbian.h"
#include "advsecsettingsstoreuids.h"
#include <unifiedcertstore.h>           // CUnifiedCertStore
#include <unifiedkeystore.h>            // CUnifiedKeyStore
#include <cctcertinfo.h>                // CCTCertInfo
#include <mctwritablecertstore.h>       // MCTWritableCertStore
#include <mctkeystoremanager.h>         // MCTKeyStoreManager

const TInt KMaxBufferLength = 0x3000;   // 12kB, for keys and certificates
_LIT_SECURITY_POLICY_C1( KKeyStoreUsePolicy, ECapabilityReadUserData );

// TODO: replace with proper logging
#ifdef _DEBUG
#define TRACE(x)        RDebug::Printf(x)
#define TRACE1(x, y)    RDebug::Printf((x), (y))
#define TRACE2(x, y, z) RDebug::Printf((x), (y), (z))
#else
#define TRACE(x)
#define TRACE1(x, y)
#define TRACE2(x, y, z)
#endif


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertMover::NewL()
// ---------------------------------------------------------------------------
//
CAdvSecSettingsCertMover *CAdvSecSettingsCertMover::NewL(RFs &aFs)
{
    TRACE("CAdvSecSettingsCertMover::NewL()");
    CAdvSecSettingsCertMover* self = new( ELeave ) CAdvSecSettingsCertMover(aFs);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertMover::~CAdvSecSettingsCertMover()
// ---------------------------------------------------------------------------
//
CAdvSecSettingsCertMover::~CAdvSecSettingsCertMover()
{
    TRACE("CAdvSecSettingsCertMover::~CAdvSecSettingsCertMover()");
    Cancel();
    delete iCertStore;
    iTargetCertStore = NULL;
    delete iCertFilter;
    iCerts.Close();

    delete iKeyStore;
    iSourceKeyStore = NULL;
    iTargetKeyStore = NULL;
    delete iKeyFilter;
    iKeys.Close();
    if (iSavedKeyInfo) {
        iSavedKeyInfo->Release();
    }

    delete iDataBuffer;
    iClientStatus = NULL;
    iCertInfo = NULL;
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertMover::Move()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertMover::Move(const CCTCertInfo &aCert,
    TInt aSourceCertStoreTokenId, TInt aTargetCertStoreTokenId,
    TRequestStatus &aStatus)
{
    TRACE("CAdvSecSettingsCertMover::Move()");
    aStatus = KRequestPending;
    if (!iClientStatus) {
        iClientStatus = &aStatus;

        iCertInfo = &aCert;
        iSourceCertStoreTokenId = aSourceCertStoreTokenId;
        iTargetCertStoreTokenId = aTargetCertStoreTokenId;

        if (iState <= EIdle) {
            // Start move operation if initializations are complete.
            if (iState == EIdle) {
                TRAPD(err, StartMoveOperationL());
                if (err) {
                    TRACE1("CAdvSecSettingsCertMover::Move(), error %d", err);
                    User::RequestComplete(iClientStatus, err);
                    iClientStatus = NULL;
                }
            }
            // If initializations are not complete yet, then moving
            // starts in RunL() after initializations are completed.
        } else {
            // Possibly initializations have failed.
            TRACE("CAdvSecSettingsCertMover::Move(), RequestComplete KErrGeneral");
            User::RequestComplete(iClientStatus, KErrGeneral);
            iClientStatus = NULL;
        }
    } else {
        TRACE("CAdvSecSettingsCertMover::Move(), RequestComplete KErrInUse");
        TRequestStatus* status = &aStatus;
        User::RequestComplete(status, KErrInUse);
    }
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertMover::DoCancel()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertMover::DoCancel()
{
    TRACE("CAdvSecSettingsCertMover::DoCancel()");
    switch (iState) {
    case EInitializingCertStore:
        iCertStore->CancelInitialize();
        iState = ENotInitialized;
        break;
    case EInitializingKeyStore:
        iKeyStore->CancelInitialize();
        iState = ENotInitialized;
        break;
    case EMovingKeyListingKeys:
        iKeyStore->CancelList();
        iState = EIdle;
        break;
    case EMovingKeyExportingKeys:
        iSourceKeyStore->CancelExportKey();
        iState = EIdle;
        break;
    case EMovingKeyImportingKeys:
        iTargetKeyStore->CancelImportKey();
        iState = EIdle;
        break;
    case EMovingKeyDeletingOriginal:
        iSourceKeyStore->CancelDeleteKey();
        iState = EIdle;
        break;
    case EMovingCertListingCerts:
        iCertStore->CancelList();
        iState = EIdle;
        break;
    case EMovingCertRetrievingCerts:
        iSourceCertStore->CancelRetrieve();
        iState = EIdle;
        break;
    case EMovingCertAddingCerts:
        iTargetCertStore->CancelAdd();
        iState = EIdle;
        break;
    case EMovingCertDeletingOriginal:
        iSourceCertStore->CancelRemove();
        iState = EIdle;
        break;
    default:
        break;
    }

    if (iClientStatus) {
        TRACE("CAdvSecSettingsCertMover::DoCancel(), RequestComplete KErrCancel");
        User::RequestComplete(iClientStatus, KErrCancel);
        iClientStatus = NULL;
    }
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertMover::RunL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertMover::RunL()
{
    TRACE2("CAdvSecSettingsCertMover::RunL(), iState=%d, iStatus.Int()=%d", iState, iStatus.Int());
    User::LeaveIfError(iStatus.Int());

    switch (iState) {
    case EInitializingCertStore:
        iKeyStore->Initialize(iStatus);
        iState = EInitializingKeyStore;
        SetActive();
        break;
    case EInitializingKeyStore:
        iState = EIdle;
        if (iClientStatus) {
            StartMoveOperationL();
        }
        break;
    case EMovingKeyListingKeys:
        ExportFirstKeyL();
        break;
    case EMovingKeyExportingKeys:
        SaveExportedKeyL();
        break;
    case EMovingKeyImportingKeys:
        DeleteOriginalKeyL();
        break;
    case EMovingKeyDeletingOriginal:
        ExportNextKeyL();
        break;
    case EMovingCertListingCerts:
        RetrieveFirstCertL();
        break;
    case EMovingCertRetrievingCerts:
        SaveRetrievedCertL();
        break;
    case EMovingCertAddingCerts:
        DeleteOriginalCertL();
        break;
    case EMovingCertDeletingOriginal:
        RetrieveNextCertL();
        break;
    default:
        ASSERT(EFalse);
        break;
    }
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertMover::RunError()
// ---------------------------------------------------------------------------
//
TInt CAdvSecSettingsCertMover::RunError(TInt aError)
{
    TRACE1("CAdvSecSettingsCertMover::RunError(), aError=%d", aError);
    if (iClientStatus) {
        TRACE1("CAdvSecSettingsCertMover::RunError(), RequestComplete %d", aError);
        User::RequestComplete(iClientStatus, aError);
        iClientStatus = NULL;
    }
    if (iState < EIdle) {
        iState = EFailed;
    } else {
        iState = EIdle;
    }
    return KErrNone;
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertMover::CAdvSecSettingsCertMover()
// ---------------------------------------------------------------------------
//
CAdvSecSettingsCertMover::CAdvSecSettingsCertMover(RFs &aFs) :
    CActive(CActive::EPriorityLow), iFs(aFs), iDataPtr(0,0)
{
    CActiveScheduler::Add(this);
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertMover::ConstructL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertMover::ConstructL()
{
    TRACE("CAdvSecSettingsCertMover::ConstructL()");
    const TBool KWriteMode = ETrue;
    iCertStore = CUnifiedCertStore::NewL(iFs, KWriteMode);
    iKeyStore = CUnifiedKeyStore::NewL(iFs);

    iDataBuffer = HBufC8::New(KMaxBufferLength);
    iDataPtr.Set(iDataBuffer->Des());

    iCertStore->Initialize(iStatus);
    iState = EInitializingCertStore;
    SetActive();
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertMover::StartMoveOperationL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertMover::StartMoveOperationL()
{
    TRACE("CAdvSecSettingsCertMover::StartMoveOperationL()");
    FindSourceAndTargetKeyStoresL();
    FindSourceAndTargetCertStoreL();
    StartMovingKeysL();
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertMover::FindSourceAndTargetKeyStoresL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertMover::FindSourceAndTargetKeyStoresL()
{
    TRACE("CAdvSecSettingsCertMover::FindSourceAndTargetKeyStoresL()");
    TInt keyStoreSourceTokenId = CorrespondingKeyStoreTokenId(iSourceCertStoreTokenId);
    TInt keyStoreTargetTokenId = CorrespondingKeyStoreTokenId(iTargetCertStoreTokenId);
    TInt keyStoreSourceIndex = KErrNotFound;
    TInt keyStoreTargetIndex = KErrNotFound;

    TInt count = iKeyStore->KeyStoreManagerCount();
    for (TInt index = 0; index < count; index++) {
        MCTKeyStoreManager& keystoremanager = iKeyStore->KeyStoreManager(index);
        MCTToken& token = keystoremanager.Token();
        TUid tokenTypeUid = token.Handle().iTokenTypeUid;
        if (tokenTypeUid.iUid == keyStoreSourceTokenId) {
            keyStoreSourceIndex = index;
        }
        if (tokenTypeUid.iUid == keyStoreTargetTokenId) {
            keyStoreTargetIndex = index;
        }
    }

    if (keyStoreSourceIndex == KErrNotFound || keyStoreTargetIndex == KErrNotFound) {
        User::Leave(KErrNotFound);
    }

    iSourceKeyStore = &( iKeyStore->KeyStoreManager(keyStoreSourceIndex) );
    iTargetKeyStore = &( iKeyStore->KeyStoreManager(keyStoreTargetIndex) );
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertMover::FindSourceAndTargetCertStoreL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertMover::FindSourceAndTargetCertStoreL()
{
    TRACE("CAdvSecSettingsCertMover::FindSourceAndTargetCertStoreL()");
    TInt certStoreSourceIndex = KErrNotFound;
    TInt certStoreTargetIndex = KErrNotFound;

    TInt count = iCertStore->WritableCertStoreCount();
    for (TInt index = 0; index < count; index++) {
        MCTWritableCertStore& writablestore = iCertStore->WritableCertStore(index);
        MCTToken& token = writablestore.Token();
        TUid tokenTypeUid = token.Handle().iTokenTypeUid;
        if (tokenTypeUid.iUid == iSourceCertStoreTokenId) {
            certStoreSourceIndex = index;
        }
        if (tokenTypeUid.iUid == iTargetCertStoreTokenId) {
            certStoreTargetIndex = index;
        }
    }

    if (certStoreSourceIndex == KErrNotFound || certStoreTargetIndex == KErrNotFound) {
        User::Leave(KErrNotFound);
    }

    iSourceCertStore = &( iCertStore->WritableCertStore(certStoreSourceIndex) );
    iTargetCertStore = &( iCertStore->WritableCertStore(certStoreTargetIndex) );
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertMover::CorrespondingKeyStoreTokenId()
// ---------------------------------------------------------------------------
//
TInt CAdvSecSettingsCertMover::CorrespondingKeyStoreTokenId(TInt aCertStoreTokenId)
{
    TInt keyStoreTokenId = KErrNotFound;
    switch (aCertStoreTokenId) {
    case KAdvSecSettingsFileCertStore:
        keyStoreTokenId = KAdvSecSettingsFileKeyStore;
        break;
    case KAdvSecSettingsDeviceCertStore:
        keyStoreTokenId = KAdvSecSettingsDeviceKeyStore;
        break;
    default:
        ASSERT(EFalse);     // Unsupported cert store token id used
        break;
    }
    return keyStoreTokenId;
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertMover::StartMovingKeysL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertMover::StartMovingKeysL()
{
    TRACE("CAdvSecSettingsCertMover::StartMovingKeysL()");
    if (iKeyFilter) {
        delete iKeyFilter;
        iKeyFilter = NULL;
    }
    iKeyFilter = new( ELeave ) TCTKeyAttributeFilter;
    iKeyFilter->iKeyId = iCertInfo->SubjectKeyId();
    iKeyFilter->iPolicyFilter =  TCTKeyAttributeFilter::EAllKeys;
    iKeyStore->List(iKeys, *iKeyFilter, iStatus);
    iState = EMovingKeyListingKeys;
    SetActive();
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertMover::ExportFirstKeyL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertMover::ExportFirstKeyL()
{
    TRACE1("CAdvSecSettingsCertMover::ExportFirstKeyL(), iKeys.Count()=%d", iKeys.Count());
    iKeyIndex = 0;
    ExportOneKeyL();
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertMover::ExportOneKeyL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertMover::ExportOneKeyL()
{
    TRACE("CAdvSecSettingsCertMover::ExportOneKeyL()");
    if (iKeyIndex < iKeys.Count()) {
        const CCTKeyInfo& keyInfo = *(iKeys[iKeyIndex]);
        iSourceKeyStore->ExportKey(keyInfo.Handle(), iDataBuffer, iStatus);
        iState = EMovingKeyExportingKeys;
        SetActive();
    } else {
        TRACE("CAdvSecSettingsCertMover::ExportOneKeyL(), all done");
        StartMovingCertificatesL();
    }
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertMover::ExportNextKeyL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertMover::ExportNextKeyL()
{
    TRACE("CAdvSecSettingsCertMover::ExportNextKeyL()");
    ++iKeyIndex;
    ExportOneKeyL();
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertMover::SaveExportedKeyL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertMover::SaveExportedKeyL()
{
    TRACE("CAdvSecSettingsCertMover::SaveExportedKeyL()");
    const CCTKeyInfo& keyInfo = *(iKeys[iKeyIndex]);
    iSourceKeyHandle = keyInfo.Handle();

    // TODO: is this needed? should iSavedKeyInfo be always used?
    // Keys having CCTKeyInfo::ELocal access type cannot be imported.
    // Workaround is to create almost identical copy of CCTKeyInfo without
    // ELocal access type flag. UsePolicy is also updated.
    TInt accessType = keyInfo.AccessType();
    if (accessType & CCTKeyInfo::ELocal) {
        accessType ^= CCTKeyInfo::ELocal;

        HBufC* label = keyInfo.Label().AllocLC();
        if (iSavedKeyInfo) {
            iSavedKeyInfo->Release();
            iSavedKeyInfo = NULL;
        }
        iSavedKeyInfo = CCTKeyInfo::NewL( keyInfo.ID(), keyInfo.Usage(),
            keyInfo.Size(), NULL, label, keyInfo.Token(), keyInfo.HandleID(),
            KKeyStoreUsePolicy, keyInfo.ManagementPolicy(),keyInfo.Algorithm(),
            keyInfo.AccessType(), keyInfo.Native(), keyInfo.StartDate(),
            keyInfo.EndDate() );
        CleanupStack::Pop(label);

        iTargetKeyStore->ImportKey(*iDataBuffer, iSavedKeyInfo, iStatus);
    } else {
        iTargetKeyStore->ImportKey(*iDataBuffer, iKeys[iKeyIndex], iStatus);
    }
    iState = EMovingKeyImportingKeys;
    SetActive();
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertMover::DeleteOriginalKeyL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertMover::DeleteOriginalKeyL()
{
    TRACE("CAdvSecSettingsCertMover::DeleteOriginalKeyL()");
    iSourceKeyStore->DeleteKey(iSourceKeyHandle, iStatus);
    iState = EMovingKeyDeletingOriginal;
    SetActive();
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertMover::StartMovingCertificatesL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertMover::StartMovingCertificatesL()
{
    TRACE("CAdvSecSettingsCertMover::StartMovingCertificatesL()");
    if (iCertFilter) {
        delete iCertFilter;
        iCertFilter = NULL;
    }
    iCertFilter = CCertAttributeFilter::NewL();
    iCertFilter->SetOwnerType(EUserCertificate);
    iCertFilter->SetSubjectKeyId(iCertInfo->SubjectKeyId());
    iCertStore->List(iCerts, *iCertFilter, iStatus);
    iState = EMovingCertListingCerts;
    SetActive();
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertMover::RetrieveFirstCertL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertMover::RetrieveFirstCertL()
{
    TRACE1("CAdvSecSettingsCertMover::RetrieveFirstCertL(), iCerts.Count()=%d", iCerts.Count());
    iCertIndex = 0;
    RetrieveOneCertL();
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertMover::RetrieveOneCertL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertMover::RetrieveOneCertL()
{
    TRACE("CAdvSecSettingsCertMover::RetrieveOneCertL()");
    if (iCertIndex < iCerts.Count()) {
        const CCTCertInfo& certInfo = *(iCerts[iCertIndex]);
        iSourceCertStore->Retrieve(certInfo, iDataPtr, iStatus);
        iState = EMovingCertRetrievingCerts;
        SetActive();
    } else {
        TRACE("CAdvSecSettingsCertMover::RetrieveOneCertL(), all done");
        iState = EIdle;
        User::RequestComplete(iClientStatus, KErrNone);
        iClientStatus = NULL;
    }
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertMover::RetrieveNextCertL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertMover::RetrieveNextCertL()
{
    TRACE("CAdvSecSettingsCertMover::RetrieveNextCertL()");
    ++iCertIndex;
    RetrieveOneCertL();
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertMover::SaveRetrievedCertL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertMover::SaveRetrievedCertL()
{
    TRACE("CAdvSecSettingsCertMover::SaveRetrievedCertL()");
    const CCTCertInfo& certInfo = *(iCerts[iCertIndex]);
    iTargetCertStore->Add(certInfo.Label(), EX509Certificate, EUserCertificate,
        &(certInfo.SubjectKeyId()), &(certInfo.IssuerKeyId()), *iDataBuffer, iStatus);
    iState = EMovingCertAddingCerts;
    SetActive();
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertMover::DeleteOriginalCertL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertMover::DeleteOriginalCertL()
{
    TRACE("CAdvSecSettingsCertMover::DeleteOriginalCertL()");
    const CCTCertInfo& certInfo = *(iCerts[iCertIndex]);
    iSourceCertStore->Remove(certInfo, iStatus);
    iState = EMovingCertDeletingOriginal;
    SetActive();
}

