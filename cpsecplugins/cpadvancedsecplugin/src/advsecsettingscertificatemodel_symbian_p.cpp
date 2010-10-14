/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of the License "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  Symbian specific advanced security settings certificate model
*
*/

#include "advsecsettingscertificatemodel_symbian_p.h"
#include "advsecsettingscertificatemodel.h"
#include "advsecsettingscertificate.h"
#include "advsecsettingstrustedcertusageuids.h"
#include "advsecsettingsstoreuids.h"
#include "advsecsettingscertlistbuilder_symbian.h"
#include "advsecsettingscertdetailsbuilder_symbian.h"
#include "advsecsettingscertmover_symbian.h"
#include <cctcertinfo.h>            // CCTCertInfo
#include <certificateapps.h>        // CCertificateAppInfoManager
#include <HbTextResolverSymbian>

_LIT(KTranslationFile, "z:\\resource\\qt\\translations");
_LIT(KTranslationPath, "certificate_manager_");

_LIT(KLocNativeInstallationUsage, "txt_certificate_manager_formlabel_symbian_instal");
_LIT(KLocJavaInstallationUsage, "txt_certificate_manager_formlabel_java_installing");
_LIT(KLocWidgetInstallationUsage, "txt_certificate_manager_formlabel_widget_installat");
_LIT(KLocInternetUsage, "txt_certificate_manager_formlabel_internet");
_LIT(KLocOcspUsage, "txt_certificate_manager_formlabel_online_certifica");
_LIT(KLocVPNUsage, "txt_certificate_manager_formlabel_vpn");

#define TRAP_HANDLE_AND_RETURN_IF_ERROR(_f)     \
{                                               \
    TInt __thr_error = KErrNone;                \
    TRAP(__thr_error, _f);                      \
    if (__thr_error) {                          \
        q_ptr->handleError(__thr_error);        \
        return;                                 \
    }                                           \
}


// ======== LOCAL FUNCTIONS ========

// ---------------------------------------------------------------------------
// CopyStringL()
// ---------------------------------------------------------------------------
//
QString CopyStringL(const TDesC16 &aDes16)
{
    QString string;
    QT_TRYCATCH_LEAVING(string = QString::fromUtf16(aDes16.Ptr(), aDes16.Length()));
    return string;
}

// ---------------------------------------------------------------------------
// CopyStringL()
// ---------------------------------------------------------------------------
//
QString CopyStringL(const TDesC8 &aDes8)
{
    QString string;
    QT_TRYCATCH_LEAVING(string = QString::fromUtf8(
        reinterpret_cast<const char*>(aDes8.Ptr()), aDes8.Length()));
    return string;
}


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModelPrivate::AdvSecSettingsCertificateModelPrivate()
// ---------------------------------------------------------------------------
//
AdvSecSettingsCertificateModelPrivate::AdvSecSettingsCertificateModelPrivate(
    AdvSecSettingsCertificateModel *q) : CActive(CActive::EPriorityLow), q_ptr(q),
    iState(ENotInitialized)
{
    CActiveScheduler::Add(this);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModelPrivate::~AdvSecSettingsCertificateModelPrivate()
// ---------------------------------------------------------------------------
//
AdvSecSettingsCertificateModelPrivate::~AdvSecSettingsCertificateModelPrivate()
{
    Cancel();

    delete iMover;
    delete iCertAppInfoManager;
    delete iCertStore;
    iFs.Close();

    iCertInfo = NULL;
    iCertInfoArray.Close();
    delete iCertListBuilder;
    delete iDetailsBuilder;

    iTrustedUids = NULL;
    iApplications.Reset();
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModelPrivate::initialize()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModelPrivate::initialize()
{
    if (!IsActive() && (iState == ENotInitialized)) {
        TRAP_HANDLE_AND_RETURN_IF_ERROR(ConstructL());
        iCertStore->Initialize(iStatus);
        iState = EInitializing;
        SetActive();
    } else {
        q_ptr->handleError(KErrAlreadyExists);
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModelPrivate::getCertificates()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModelPrivate::getCertificates(
    QList<AdvSecSettingsCertificate*> &certList)
{
    if (!IsActive() && (iState == EIdle)) {
        if (!iCertListBuilder) {
            TRAP_HANDLE_AND_RETURN_IF_ERROR(iCertListBuilder =
                CAdvSecSettingsCertListBuilder::NewL(iFs, *iCertStore));
        }
        iCertListBuilder->GetCertificateList(certList, iCertInfoArray, iStatus);
        iState = EGettingCertificatesList;
        SetActive();
    } else {
        q_ptr->handleError(KErrInUse);
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModelPrivate::getCertificateDetails()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModelPrivate::getCertificateDetails(
    const AdvSecSettingsCertificate &cert, QMap<int,QString> &details)
{
    if (!IsActive() && (iState == EIdle)) {
        iCertInfo = CertificateInfo(cert);
        if (iCertInfo) {
            if (!iDetailsBuilder) {
                TRAP_HANDLE_AND_RETURN_IF_ERROR(iDetailsBuilder =
                    CAdvSecSettingsCertDetailsBuilder::NewL(iFs, *iCertStore));
            }
            TRAP_HANDLE_AND_RETURN_IF_ERROR(
                iDetailsBuilder->GetDetailsL(*iCertInfo, details, iStatus));
            iState = EBuildingCertificateDetails;
            SetActive();
        }
    } else {
        q_ptr->handleError(KErrInUse);
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModelPrivate::isDeletable()
// ---------------------------------------------------------------------------
//
bool AdvSecSettingsCertificateModelPrivate::isDeletable(
    const AdvSecSettingsCertificate &cert) const
{
    const CCTCertInfo* certInfo = CertificateInfo(cert);
    if (certInfo) {
        return certInfo->IsDeletable();
    }
    return false;
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModelPrivate::deleteCertificate()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModelPrivate::deleteCertificate(
    const AdvSecSettingsCertificate &cert)
{
    if (!IsActive() && (iState == EIdle)) {
        iCertInfo = CertificateInfo(cert);
        if (iCertInfo) {
            iCertStore->Remove(*iCertInfo, iStatus);
            iState = EDeletingCertificate;
            SetActive();
        }
    } else {
        q_ptr->handleError(KErrInUse);
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModelPrivate::getTrustSettings()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModelPrivate::getTrustSettings(
    const AdvSecSettingsCertificate &cert, QMap<int,bool> &usageIdAndTrust)
{
    if (!IsActive() && (iState == EIdle)) {
        iCertInfo = CertificateInfo(cert);
        if (iCertInfo) {
            iTrustedUids = &usageIdAndTrust;
            iApplications.Reset();
            iCertStore->Applications(*iCertInfo, iApplications, iStatus);
            iState = EReadingApplications;
            SetActive();
        }
    } else {
        q_ptr->handleError(KErrInUse);
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModelPrivate::setTrustSettings()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModelPrivate::setTrustSettings(
    const AdvSecSettingsCertificate &cert, const QMap<int,bool> &usageIdAndTrust)
{
    if (!IsActive() && (iState == EIdle)) {
        iCertInfo = CertificateInfo(cert);
        if (iCertInfo) {
            iApplications.Reset();
            QMapIterator<int,bool> usageIter(usageIdAndTrust);
            while (usageIter.hasNext()) {
                usageIter.next();
                if (usageIter.value()) {
                    iApplications.Append(TUid::Uid(usageIter.key()));
                }
            }

            iCertStore->SetApplicability(*iCertInfo, iApplications, iStatus);
            iState = EWritingApplications;
            SetActive();
        }
    } else {
        q_ptr->handleError(KErrInUse);
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModelPrivate::getCertificateUsageNames()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModelPrivate::getCertificateUsageNames(
    QMap<int,QString> &usageIdAndName)
{
    usageIdAndName.clear();
    QT_TRAP_THROWING(DoGetCertificateUsageNamesL(usageIdAndName));
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModelPrivate::moveToPersonalCertificates()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModelPrivate::moveToPersonalCertificates(
    const AdvSecSettingsCertificate &cert)
{
    if (!IsActive() && (iState == EIdle)) {
        iCertInfo = CertificateInfo(cert);
        if (iCertInfo) {
            if (!iMover) {
                TRAP_HANDLE_AND_RETURN_IF_ERROR(iMover =
                    CAdvSecSettingsCertMover::NewL(iFs));
            }
            // personal certificate store == file certificate store
            iMover->Move(*iCertInfo, KAdvSecSettingsDeviceCertStore,
                KAdvSecSettingsFileCertStore, iStatus);
            iState = EMovingCertificateToPersonalStore;
            SetActive();
        }
    } else {
        q_ptr->handleError(KErrInUse);
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModelPrivate::moveToDeviceCertificates()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModelPrivate::moveToDeviceCertificates(
    const AdvSecSettingsCertificate &cert)
{
    if (!IsActive() && (iState == EIdle)) {
        iCertInfo = CertificateInfo(cert);
        if (iCertInfo) {
            if (!iMover) {
                TRAP_HANDLE_AND_RETURN_IF_ERROR(iMover =
                    CAdvSecSettingsCertMover::NewL(iFs));
            }
            iMover->Move(*iCertInfo, KAdvSecSettingsFileCertStore,
                KAdvSecSettingsDeviceCertStore, iStatus);
            iState = EMovingCertificateToDeviceStore;
            SetActive();
        }
    } else {
        q_ptr->handleError(KErrInUse);
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModelPrivate::DoCancel()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModelPrivate::DoCancel()
{
    switch (iState) {
    case EInitializing:
        iCertStore->CancelInitialize();
        break;
    case EGettingCertificatesList:
        delete iCertListBuilder;
        iCertListBuilder = NULL;
        break;
    case EBuildingCertificateDetails:
        delete iDetailsBuilder;
        iDetailsBuilder = NULL;
        break;
    case EDeletingCertificate:
        iCertStore->CancelRemove();
        break;
    case EReadingApplications:
        iCertStore->CancelApplications();
        break;
    case EWritingApplications:
        iCertStore->CancelSetApplicability();
        break;
    case EMovingCertificateToPersonalStore:
    case EMovingCertificateToDeviceStore:
        delete iMover;
        iMover = NULL;
        break;
    default:
        break;
    }
    if (iState == EInitializing) {
        iState = ENotInitialized;
    } else {
        iState = EIdle;
    }
    q_ptr->handleError(KErrCancel);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModelPrivate::RunL()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModelPrivate::RunL()
{
    User::LeaveIfError(iStatus.Int());

    switch (iState) {
    case EInitializing:
        iState = EIdle;
        q_ptr->handleInitializeCompleted();
        break;
    case EGettingCertificatesList:
        iState = EIdle;
        q_ptr->handleGetCertificatesCompleted();
        break;
    case EBuildingCertificateDetails:
        iState = EIdle;
        q_ptr->handleGetCertificateDetailsCompleted();
        break;
    case EDeletingCertificate:
        iState = EIdle;
        q_ptr->handleDeleteCertificateCompleted();
        break;
    case EReadingApplications:
        ReturnTrustSettingsDetails();
        iState = EIdle;
        q_ptr->handleGetTrustSettingsCompleted();
        break;
    case EWritingApplications:
        iState = EIdle;
        q_ptr->handleSetTrustSettingsCompleted();
        break;
    case EMovingCertificateToPersonalStore:
        iState = EIdle;
        q_ptr->handleMoveToPersonalCertificateCompleted();
        break;
    case EMovingCertificateToDeviceStore:
        iState = EIdle;
        q_ptr->handleMoveToDeviceCertificatesCompleted();
        break;
    default:
        ASSERT(EFalse);
        break;
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModelPrivate::RunError()
// ---------------------------------------------------------------------------
//
TInt AdvSecSettingsCertificateModelPrivate::RunError(TInt aError)
{
    if (iState == EInitializing) {
        iState = ENotInitialized;
    } else {
        iState = EIdle;
    }
    q_ptr->handleError(aError);
    return KErrNone;
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModelPrivate::ConstructL()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModelPrivate::ConstructL()
{
    User::LeaveIfError(iFs.Connect());
    const TBool KWriteMode = ETrue;
    iCertStore = CUnifiedCertStore::NewL(iFs, KWriteMode);
    iCertAppInfoManager = CCertificateAppInfoManager::NewL();
    if (!HbTextResolverSymbian::Init(KTranslationFile, KTranslationPath)) {
        // TODO: text resolver initialisation failed
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModelPrivate::ReturnTrustSettingsDetails()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModelPrivate::ReturnTrustSettingsDetails()
{
    if (iTrustedUids && iCertInfo) {
        iTrustedUids->clear();

        // Set applicable trusted usages for different kinds of certs.
        TUid tokenType(iCertInfo->Token().TokenType().Type());
        if (tokenType.iUid == KAdvSecSettingsSWICertStore) {
            iTrustedUids->insert(KAdvSecSettingsTrustedUsageSwInstallNativeSis, false);
            iTrustedUids->insert(KAdvSecSettingsTrustedUsageSwInstallSisOcsp, false);
        } else if (tokenType.iUid == KAdvSecSettingsMidpCertStore) {
            iTrustedUids->insert(KAdvSecSettingsTrustedUsageSwInstallJava, false);
        } else {
            const RArray<TCertificateAppInfo> &apps(iCertAppInfoManager->Applications());
            for (TInt index = 0; index < apps.Count(); ++index) {
                const TCertificateAppInfo &appInfo = apps[index];
                int usageId = appInfo.Id().iUid;
                iTrustedUids->insert(usageId, false);
            }
        }

        // Set trusted usages, usually this just replaces some of the values set above.
        TInt count = iApplications.Count();
        for (TInt index = 0; index < count; ++index ) {
            iTrustedUids->insert(iApplications[index].iUid, true);
        }

        // Native and Java installation use certs only from SWI and MIDP cert stores.
        // Hence, native and Java install usages are not displayed for certs in file
        // cert store.
        if (tokenType.iUid == KAdvSecSettingsFileCertStore) {
            iTrustedUids->remove(KAdvSecSettingsTrustedUsageSwInstallNativeSis);
            iTrustedUids->remove(KAdvSecSettingsTrustedUsageSwInstallJava);
        }
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModelPrivate::DoGetCertificateUsageNamesL()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModelPrivate::DoGetCertificateUsageNamesL(
    QMap<int,QString> &usageIdAndName)
{
    // Pre-defined localized usage names
    HBufC* usageName = HbTextResolverSymbian::LoadLC(KLocNativeInstallationUsage);
    usageIdAndName[KAdvSecSettingsTrustedUsageSwInstallNativeSis] = CopyStringL(*usageName);
    CleanupStack::PopAndDestroy(usageName);

    usageName = HbTextResolverSymbian::LoadLC(KLocJavaInstallationUsage);
    usageIdAndName[KAdvSecSettingsTrustedUsageSwInstallJava] = CopyStringL(*usageName);
    CleanupStack::PopAndDestroy(usageName);

    usageName = HbTextResolverSymbian::LoadLC(KLocWidgetInstallationUsage);
    usageIdAndName[KAdvSecSettingsTrustedUsageSwInstallWidget] = CopyStringL(*usageName);
    CleanupStack::PopAndDestroy(usageName);

    usageName = HbTextResolverSymbian::LoadLC(KLocInternetUsage);
    usageIdAndName[KAdvSecSettingsTrustedUsageInternet] = CopyStringL(*usageName);
    CleanupStack::PopAndDestroy(usageName);

    usageName = HbTextResolverSymbian::LoadLC(KLocOcspUsage);
    usageIdAndName[KAdvSecSettingsTrustedUsageSwInstallSisOcsp] = CopyStringL(*usageName);
    CleanupStack::PopAndDestroy(usageName);

    usageName = HbTextResolverSymbian::LoadLC(KLocVPNUsage);
    usageIdAndName[KAdvSecSettingsTrustedUsageVPN] = CopyStringL(*usageName);
    CleanupStack::PopAndDestroy(usageName);

    // TODO: localized UI string needed
    usageIdAndName[KAdvSecSettingsTrustedUsageWap] = QString("Wap");

    // Possible additional usage names defined in system databases
    if (iCertAppInfoManager) {
        const RArray<TCertificateAppInfo> &apps(iCertAppInfoManager->Applications());
        for(TInt index = 0; index < apps.Count(); ++index) {
            const TCertificateAppInfo &appInfo = apps[index];

            int usageId = appInfo.Id().iUid;
            if (!usageIdAndName.contains(usageId)) {
                QString usageName = CopyStringL(appInfo.Name());
                usageIdAndName.insert(usageId, usageName);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModelPrivate::CertificateInfo()
// ---------------------------------------------------------------------------
//
const CCTCertInfo *AdvSecSettingsCertificateModelPrivate::CertificateInfo(
    const AdvSecSettingsCertificate &cert) const
{
    TInt index = cert.modelIndex();
    if (index >= 0 && index < iCertInfoArray.Count()) {
        return iCertInfoArray[index];
    }
    return NULL;
}

