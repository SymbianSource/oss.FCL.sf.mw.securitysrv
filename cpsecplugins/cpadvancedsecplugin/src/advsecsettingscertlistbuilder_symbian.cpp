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
* Description:  Helper class to construct certificate list
*
*/

#include "advsecsettingscertlistbuilder_symbian.h"
#include "advsecsettingslabeledcertinfo_symbian.h"
#include "advsecsettingsstoreuids.h"    // KAdvSecSettingsDeviceCertStore
#include <x509cert.h>                   // CX509Certificate
#include <ccertattributefilter.h>       // CCertAttributeFilter
#include <X509CertNameParser.h>         // X509CertNameParser::PrimaryAndSecondaryNameL
#include <stl/_auto_ptr.h>              // std::auto_ptr

_LIT(KNameSeparator, " ");

// implemented in advsecsettingssecuritymodulemodel_symbian_p.cpp
QString CopyStringL(const TDesC16 &aDes16);


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertListBuilder::NewL()
// ---------------------------------------------------------------------------
//
CAdvSecSettingsCertListBuilder *CAdvSecSettingsCertListBuilder::NewL(
    RFs &aFs, CUnifiedCertStore &aCertStore)
{
    CAdvSecSettingsCertListBuilder *self = new(ELeave) CAdvSecSettingsCertListBuilder(
        aFs, aCertStore);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertListBuilder::~CAdvSecSettingsCertListBuilder()
// ---------------------------------------------------------------------------
//
CAdvSecSettingsCertListBuilder::~CAdvSecSettingsCertListBuilder()
{
    Cancel();
    iLabeledCertInfos.ResetAndDestroy();
    delete iCertificate;
    delete iCertAttributeFilter;
    iCertInfoArray.Close();
    iCertList = NULL;
    iClientInfoArray = NULL;
    iClientStatus = NULL;
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertListBuilder::GetCertificateList()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertListBuilder::GetCertificateList(
    QList<AdvSecSettingsCertificate*> &certList,
    RMPointerArray<CCTCertInfo> &aCertInfoArray,
    TRequestStatus &aStatus)
{
    aStatus = KRequestPending;
    if (!IsActive() && (iState == EIdle)) {
        iClientStatus = &aStatus;
        iCertList = &certList;
        iClientInfoArray = &aCertInfoArray;

        iCertInfoArray.Close();
        ResetCertAttributeFilter();
        iCertStore.List(iCertInfoArray, *iCertAttributeFilter, iStatus);
        iState = EListingCerts;
        SetActive();
   } else {
        TRequestStatus *status = &aStatus;
        User::RequestComplete(status, KErrInUse);
    }
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertListBuilder::DoCancel()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertListBuilder::DoCancel()
{
    switch (iState) {
    case EListingCerts:
        iCertStore.CancelList();
        break;
    case ERetrievingCert:
        iCertStore.CancelRetrieve();
        break;
    case EProcessingCert:
        // nothing to do
        break;
    default:
        break;
    }
    User::RequestComplete(iClientStatus, KErrCancel);
    iClientStatus = NULL;
    iState = EIdle;
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertListBuilder::RunL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertListBuilder::RunL()
{
    User::LeaveIfError(iStatus.Int());
    switch (iState) {
    case EListingCerts:
        ProcessFirstCertificateL();
        break;
    case ERetrievingCert:
        ProcessNextCertificateL();
        break;
    case EProcessingCert:
        ProcessNextCertificateL();
        break;
    default:
        ASSERT(EFalse);
        break;
    }
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertListBuilder::RunError()
// ---------------------------------------------------------------------------
//
TInt CAdvSecSettingsCertListBuilder::RunError(TInt aError)
{
    User::RequestComplete(iClientStatus, aError);
    iClientStatus = NULL;
    iState = EIdle;
    return KErrNone;
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertListBuilder::CAdvSecSettingsCertListBuilder()
// ---------------------------------------------------------------------------
//
CAdvSecSettingsCertListBuilder::CAdvSecSettingsCertListBuilder(RFs &aFs,
    CUnifiedCertStore &aCertStore) : CActive(CActive::EPriorityLow),
    iFs(aFs), iCertStore(aCertStore), iCertPtr(0,0)
{
    CActiveScheduler::Add(this);
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertListBuilder::ConstructL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertListBuilder::ConstructL()
{
    iCertAttributeFilter = CCertAttributeFilter::NewL();
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertListBuilder::ResetCertAttributeFilter()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertListBuilder::ResetCertAttributeFilter()
{
    ASSERT(iCertAttributeFilter);
    iCertAttributeFilter->iLabelIsSet = EFalse;
    iCertAttributeFilter->iUidIsSet = EFalse;
    iCertAttributeFilter->iFormatIsSet = EFalse;
    iCertAttributeFilter->iKeyUsage = EX509UsageAll;
    iCertAttributeFilter->iOwnerTypeIsSet = EFalse;
    iCertAttributeFilter->iSubjectKeyIdIsSet = EFalse;
    iCertAttributeFilter->iIssuerKeyIdIsSet = EFalse;
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertListBuilder::ProcessFirstCertificateL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertListBuilder::ProcessFirstCertificateL()
{
    iCertInfoIndex = 0;
    StartProcessingCertificateL();
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertListBuilder::ProcessNextCertificateL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertListBuilder::ProcessNextCertificateL()
{
    CompleteProcessingCertificateL();
    ++iCertInfoIndex;
    StartProcessingCertificateL();
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertListBuilder::StartProcessingCertificateL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertListBuilder::StartProcessingCertificateL()
{
    if (iCertInfoIndex < iCertInfoArray.Count()) {
        CCTCertInfo *certInfo = iCertInfoArray[iCertInfoIndex];

        if ((certInfo->CertificateOwnerType() == ECACertificate) &&
            (certInfo->CertificateFormat() == EX509Certificate)) {
            // Have to retrieve the cert to add parts of subject
            // to certificate label. For example, all Java certs
            // have the same label by default.
            if (iCertificate) {
                delete iCertificate;
                iCertificate = NULL;
            }
            iCertificate = HBufC8::NewL(certInfo->Size());
            iCertPtr.Set(iCertificate->Des());
            iCertStore.Retrieve(*certInfo, iCertPtr, iStatus);
            iState = ERetrievingCert;
            SetActive();
        } else {
            // Certificate label can be used as such.
            TRequestStatus *status = &iStatus;
            User::RequestComplete(status, KErrNone);
            iState = EProcessingCert;
            SetActive();
        }

    } else {
        ReturnCertificateListL();

        User::RequestComplete(iClientStatus, KErrNone);
        iClientStatus = NULL;
        iState = EIdle;
    }
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertListBuilder::CompleteProcessingCertificateL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertListBuilder::CompleteProcessingCertificateL()
{
    ASSERT(iCertInfoIndex < iCertInfoArray.Count());
    CCTCertInfo *certInfo = iCertInfoArray[iCertInfoIndex];

    CAdvSecSettingsLabeledCertInfo *labeledCertInfo =
        new(ELeave) CAdvSecSettingsLabeledCertInfo(*certInfo);
    CleanupStack::PushL(labeledCertInfo);
    labeledCertInfo->SetLabelL(certInfo->Label());

    if (iCertificate) {
        CX509Certificate *cert = CX509Certificate::NewL(*iCertificate);
        CleanupStack::PushL(cert);

        HBufC *primaryName = NULL;
        HBufC *secondaryName = NULL;
        TInt err = X509CertNameParser::PrimaryAndSecondaryNameL(*cert,
            primaryName, secondaryName, certInfo->Label());
        if (!err) {
            if (primaryName && primaryName->Length()) {
                labeledCertInfo->AppendLabelL(KNameSeparator, *primaryName);
            } else {
                if (secondaryName && secondaryName->Length()) {
                    labeledCertInfo->AppendLabelL(KNameSeparator, *secondaryName);
                }
            }
        }

        CleanupStack::PopAndDestroy(cert);
        delete iCertificate;
        iCertificate = NULL;
    }

    iLabeledCertInfos.AppendL(labeledCertInfo);
    CleanupStack::Pop(labeledCertInfo);
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertListBuilder::ReturnCertificateListL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertListBuilder::ReturnCertificateListL()
{
    TLinearOrder<CAdvSecSettingsLabeledCertInfo> order( CAdvSecSettingsLabeledCertInfo::Compare );
    iLabeledCertInfos.Sort( order );

    iCertList->clear();
    iClientInfoArray->Reset();
    TInt count = iLabeledCertInfos.Count();
    for (TInt index = 0; index < count; index++) {
        const CCTCertInfo &certInfo = iLabeledCertInfos[index]->CertInfo();
        iClientInfoArray->AppendL(&certInfo);

        std::auto_ptr<AdvSecSettingsCertificate> cert(new(ELeave) AdvSecSettingsCertificate);
        cert->setModelIndex(index);
        cert->setCertType(CertType(certInfo));
        cert->setLabel(CopyStringL(iLabeledCertInfos[index]->Label()));
        iCertList->append(cert.release());
    }
    iLabeledCertInfos.ResetAndDestroy();
    iCertInfoArray.Reset();
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertListBuilder::CertType()
// ---------------------------------------------------------------------------
//
AdvSecSettingsCertificate::CertificateType CAdvSecSettingsCertListBuilder::CertType(
    const CCTCertInfo &aCertInfo)
{
    AdvSecSettingsCertificate::CertificateType type =
        AdvSecSettingsCertificate::NotDefined;
    TUid storeType;
    switch (aCertInfo.CertificateOwnerType()) {
    case ECACertificate:
        type = AdvSecSettingsCertificate::AuthorityCertificate;
        break;
    case EUserCertificate:
        storeType = aCertInfo.Handle().iTokenHandle.iTokenTypeUid;
        if (storeType.iUid == KAdvSecSettingsDeviceCertStore) {
            type = AdvSecSettingsCertificate::DeviceCertificate;
        } else {
            type = AdvSecSettingsCertificate::PersonalCertificate;
        }
        break;
    case EPeerCertificate:
        type = AdvSecSettingsCertificate::TrustedSiteCertificate;
        break;
    default:
        break;
    }
    return type;
}

