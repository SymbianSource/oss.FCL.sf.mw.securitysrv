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
* Description:  Helper class to construct certificate details
*
*/

#include "advsecsettingscertdetailsbuilder_symbian.h"
#include "advsecsettingscertificatemodel.h"
#include "advsecsettingstrustedcertusageuids.h"
#include "advsecsettingsstoreuids.h"
#include <unifiedkeystore.h>            // CUnifiedKeyStore
#include <mctkeystore.h>                // CCTKeyInfo
#include <cctcertinfo.h>                // CCTCertInfo
#include <x509cert.h>                   // CX509Certificate
#include <X509CertNameParser.h>         // X509CertNameParser
#include <hash.h>                       // MD5 fingerprint
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <HbTextResolverSymbian>

// TODO: fix this
#include "../../../pkiutilities/DeviceToken/inc/TrustedSitesStore.h"    // CTrustSitesStore

_LIT(KLocAlgorithmRSA, "txt_certificate_list_rsa");
_LIT(KLocAlgorithmDSA, "txt_certificate_list_dsa");
_LIT(KLocAlgorithmDH, "txt_certificate_list_dh");
_LIT(KLocAlgorithmMD2, "txt_certificate_list_md2");
_LIT(KLocAlgorithmMD5, "txt_certificate_list_md5");
_LIT(KLocAlgorithmSHA1, "txt_certificate_list_sha1");
_LIT(KLocAlgorithmSHA2, "txt_certificate_list_sha2");
_LIT(KLocAlgorithmUnknown, "txt_certificate_list_unknown");
_LIT(KLocCertTypeX509, "txt_certificate_list_x509");
_LIT(KLocCertTypeUnknown, "txt_certificate_list_unknown");

const QString KFormatForAlgorithmNames = "%1%2";
const QString KTrustedSitesSeparator = ", ";

const int KCertManUiContinuousHexString = 0;
const int KCertManUiTwoDigitsInBlock = 2;

// implemented in advsecsettingssecuritymodulemodel_symbian_p.cpp
QString CopyStringL(const TDesC16 &aDes16);
QString CopyStringL(const TDesC8 &aDes8);


// ======== LOCAL FUNCTIONS ========

// ---------------------------------------------------------------------------
// CopyDateL()
// ---------------------------------------------------------------------------
//
void CopyDateL(const TDateTime& aFromDateTime, QDateTime& aToDateTime)
{
    QT_TRYCATCH_LEAVING(
        QDate date(aFromDateTime.Year(), aFromDateTime.Month()+1, aFromDateTime.Day()+1);
        aToDateTime.setDate(date));
}

// ---------------------------------------------------------------------------
// HexFormattedStringL()
// ---------------------------------------------------------------------------
//
QString HexFormattedStringL(const TDesC8& aDes, int blockSize = KCertManUiContinuousHexString)
{
    QString string;
    int blockIndex = 1;
    for (TInt index = 0; index < aDes.Length(); ++index) {
        QString number;
        QT_TRYCATCH_LEAVING(
            number.sprintf("%02X", aDes[ index ]);
            string.append(number));
        if (blockIndex == blockSize) {
            string.append(" ");
            blockIndex = 1;
        } else {
            ++blockIndex;
        }
    }
    return string;
}

// ---------------------------------------------------------------------------
// ResetAndDestroyCleanup()
// ---------------------------------------------------------------------------
//
QString Location(const TUid &aTokenType)
{
    QString location;
    switch (aTokenType.iUid) {
    // TODO: localized UI strings needed
    case KAdvSecSettingsFileCertStore:
    case KAdvSecSettingsFileKeyStore:
    case KAdvSecSettingsTrustedServerCertStore:
    case KAdvSecSettingsDeviceCertStore:
    case KAdvSecSettingsDeviceKeyStore:
    case KAdvSecSettingsSWICertStore:
        //: Certificate details location, phone memory
        location = QObject::tr("Phone memory");
        break;
    case KAdvSecSettingsWIMCertAndKeyStore:
        //: Certificate details location, SIM or WIM card
        location = QObject::tr("Smart card");
        break;
    default:
        break;
    }
    return location;
}

// ---------------------------------------------------------------------------
// ResetAndDestroyCleanup()
// ---------------------------------------------------------------------------
//
void ResetAndDestroyCleanup( TAny* aAny )
    {
    RPointerArray<HBufC>* array = reinterpret_cast<RPointerArray<HBufC> *>( aAny );
    array->ResetAndDestroy();
    }

// ---------------------------------------------------------------------------
// CleanupResetAndDestroyPushL()
// ---------------------------------------------------------------------------
//
void CleanupResetAndDestroyPushL( RPointerArray<HBufC>& aArray )
    {
    TCleanupItem item( &ResetAndDestroyCleanup, &aArray );
    CleanupStack::PushL( item );
    }


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertDetailsBuilder::NewL()
// ---------------------------------------------------------------------------
//
CAdvSecSettingsCertDetailsBuilder *CAdvSecSettingsCertDetailsBuilder::NewL(
    RFs &aFs, CUnifiedCertStore &aCertStore)
{
    CAdvSecSettingsCertDetailsBuilder *self = new(ELeave) CAdvSecSettingsCertDetailsBuilder(
        aFs, aCertStore);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertDetailsBuilder::~CAdvSecSettingsCertDetailsBuilder()
// ---------------------------------------------------------------------------
//
CAdvSecSettingsCertDetailsBuilder::~CAdvSecSettingsCertDetailsBuilder()
{
    Cancel();
    delete iCertificate;
    delete iCertificateUrl;
    iKeys.Close();
    delete iKeyFilter;
    delete iKeyStore;

    // not owned
    iDetails = NULL;
    iClientStatus = NULL;
    iCertInfo = NULL;
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertDetailsBuilder::GetDetailsL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertDetailsBuilder::GetDetailsL(const CCTCertInfo &aCert,
    QMap<int,QString> &aDetails, TRequestStatus &aStatus)
{
    if (!iClientStatus && (iState == ENotInitialized ||
        iState == EInitializingKeyStore || iState == EIdle)) {
        iCertInfo = &aCert;
        iDetails = &aDetails;
        iClientStatus = &aStatus;
        aStatus = KRequestPending;

        if (!iKeyStore) {
            InitializeKeyStoreL();  // Retrieving starts after initialization is complete
        } else {
            RetrieveCertificateL();
        }
    } else {
        TRequestStatus *status = &aStatus;
        User::RequestComplete(status, KErrGeneral);
    }
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertDetailsBuilder::DoCancel()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertDetailsBuilder::DoCancel()
{
    switch (iState) {
    case EInitializingKeyStore:
        iKeyStore->CancelInitialize();
        iState = ENotInitialized;
        break;
    case ERetrievingCertificate:
        iCertStore.CancelRetrieve();
        iState = EIdle;
        break;
    case EListingKeys:
        iKeyStore->CancelList();
        iState = EIdle;
        break;
    default:
        break;
    }
    User::RequestComplete(iClientStatus, KErrCancel);
    iClientStatus = NULL;
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertDetailsBuilder::RunL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertDetailsBuilder::RunL()
{
    User::LeaveIfError(iStatus.Int());

    switch (iState) {
    case EInitializingKeyStore:
        RetrieveCertificateL();
        break;
    case ERetrievingCertificate:
        ListKeysL();
        break;
    case EListingKeys:
        ReturnCertificateDetailsL();
        break;
    default:
        ASSERT(EFalse);
        break;
    }
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertDetailsBuilder::RunError()
// ---------------------------------------------------------------------------
//
TInt CAdvSecSettingsCertDetailsBuilder::RunError(TInt aError)
{
    User::RequestComplete(iClientStatus, aError);
    if (iState != EInitializingKeyStore) {
        iState = EIdle;
    } else {
        iState = EFailed;
    }
    return KErrNone;
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertDetailsBuilder::()
// ---------------------------------------------------------------------------
//
CAdvSecSettingsCertDetailsBuilder::CAdvSecSettingsCertDetailsBuilder(RFs &aFs,
    CUnifiedCertStore &aCertStore) : CActive(CActive::EPriorityLow), iFs(aFs),
    iCertStore(aCertStore), iCertUrlPtr(0, 0)
{
    CActiveScheduler::Add(this);
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertDetailsBuilder::ConstructL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertDetailsBuilder::ConstructL()
{
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertDetailsBuilder::InitializeKeyStoreL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertDetailsBuilder::InitializeKeyStoreL()
{
    ASSERT(!iKeyStore);
    iKeyStore = CUnifiedKeyStore::NewL(iFs);
    iKeyStore->Initialize(iStatus);
    iState = EInitializingKeyStore;
    SetActive();
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertDetailsBuilder::RetrieveCertificateL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertDetailsBuilder::RetrieveCertificateL()
{
    ASSERT(iCertInfo);
    if (iCertificate) {
        delete iCertificate;
        iCertificate = NULL;
    }
    if (iCertificateUrl) {
        delete iCertificateUrl;
        iCertificateUrl = NULL;
    }

    if (iCertInfo->CertificateFormat() == EX509Certificate) {
        iCertStore.Retrieve(*iCertInfo, iCertificate, iStatus);
    } else {
        iCertificateUrl = HBufC8::NewL(iCertInfo->Size());
        iCertUrlPtr.Set(iCertificateUrl->Des());
        iCertStore.Retrieve(*iCertInfo, iCertUrlPtr, iStatus);
    }
    iState = ERetrievingCertificate;
    SetActive();
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertDetailsBuilder::ListKeysL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertDetailsBuilder::ListKeysL()
{
    ASSERT(iKeyStore);
    if (!iKeyFilter) {
        iKeyFilter = new TCTKeyAttributeFilter;
    }
    iKeyFilter->iKeyId = iCertInfo->SubjectKeyId();
    iKeys.Close();
    iKeyStore->List(iKeys, *iKeyFilter, iStatus);
    iState = EListingKeys;
    SetActive();
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertDetailsBuilder::ReturnCertificateDetailsL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertDetailsBuilder::ReturnCertificateDetailsL()
{
    AppendLabelL();
    AppendIssuerL();
    AppendSubjectL();
    AppendValidityPeriodL();
    AppendLocationL();
    AppendFormatL();
    AppendKeyUsageL();
    AppendAlgorithmL();
    AppendSerialNumberL();
    AppendFingerprintsL();
    AppendPublicKeyL();
    AppendTrustedSitesL();

    User::RequestComplete(iClientStatus, KErrNone);
    iClientStatus = NULL;
    iState = EIdle;
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertDetailsBuilder::CertificateFormatStringL()
// ---------------------------------------------------------------------------
//
QString CAdvSecSettingsCertDetailsBuilder::CertificateFormatStringL(
    const TCertificateFormat aFormat) const
{
    HBufC* format = NULL;

    switch (aFormat) {
    case EX509Certificate:
        format = HbTextResolverSymbian::LoadLC(KLocCertTypeX509);
        break;
    case EWTLSCertificate:
        // TODO: localized UI strings needed
        format = _L("WTLS").AllocLC();
        break;
    case EX968Certificate:
        // TODO: localized UI strings needed
        format = _L("X968").AllocLC();
        break;
    case EX509CertificateUrl:
        // TODO: localized UI strings needed
        format = _L("X509 URL").AllocLC();
        break;
    case EWTLSCertificateUrl:
        // TODO: localized UI strings needed
        format = _L("WTLS URL").AllocLC();
        break;
    case EX968CertificateUrl:
        // TODO: localized UI strings needed
        format = _L("X968 URL").AllocLC();
        break;
    case EUnknownCertificate:
    default:
        //: Unknown certificate format type
        format = HbTextResolverSymbian::LoadLC(KLocCertTypeUnknown);
        break;
    }

    QString formatString = CopyStringL(*format);
    CleanupStack::PopAndDestroy(format);
    return formatString;
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertDetailsBuilder::AlgorithmNameStringL()
// ---------------------------------------------------------------------------
//
QString CAdvSecSettingsCertDetailsBuilder::AlgorithmNameStringL(
    TAlgorithmId aAlgorithmId) const
{
    HBufC* algorithm = NULL;

    switch (aAlgorithmId) {
    case ERSA:
        algorithm = HbTextResolverSymbian::LoadLC(KLocAlgorithmRSA);
        break;
    case EDSA:
        algorithm = HbTextResolverSymbian::LoadLC(KLocAlgorithmDSA);
        break;
    case EDH:
        algorithm = HbTextResolverSymbian::LoadLC(KLocAlgorithmDH);
        break;
    case EMD2:
        algorithm = HbTextResolverSymbian::LoadLC(KLocAlgorithmMD2);
        break;
    case EMD5:
        algorithm = HbTextResolverSymbian::LoadLC(KLocAlgorithmMD5);
        break;
    case ESHA1:
        algorithm = HbTextResolverSymbian::LoadLC(KLocAlgorithmSHA1);
        break;
    case ESHA224:
    case ESHA256:
    case ESHA384:
    case ESHA512:
        algorithm = HbTextResolverSymbian::LoadLC(KLocAlgorithmSHA2);
        break;
    default:
        break;
    }

    QString algorithmName;
    if (algorithm) {
        algorithmName = CopyStringL(*algorithm);
        CleanupStack::PopAndDestroy(algorithm);
    }
    return algorithmName;
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertDetailsBuilder::AppendLabelL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertDetailsBuilder::AppendLabelL()
{
    ASSERT(iCertInfo);
    HBufC *label = NULL;
    if (iCertInfo->CertificateFormat() == EX509Certificate) {
        CX509Certificate *cert = static_cast<CX509Certificate*>(iCertificate);
        HBufC* secondaryName = NULL;
        X509CertNameParser::PrimaryAndSecondaryNameL(*cert, label, secondaryName);
        delete secondaryName;
    } else {
        label = iCertInfo->Label().AllocL();
    }
    CleanupStack::PushL(label);
    iDetails->insert(AdvSecSettingsCertificateModel::Label, CopyStringL(*label));
    CleanupStack::PopAndDestroy(label);
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertDetailsBuilder::AppendIssuerL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertDetailsBuilder::AppendIssuerL()
{
    if (iCertificate) {
        HBufC *issuer = NULL;
        ASSERT(iCertInfo);
        if (iCertInfo->CertificateFormat() == EX509Certificate) {
            CX509Certificate* cert = static_cast<CX509Certificate*>( iCertificate );
            X509CertNameParser::IssuerFullNameL(*cert, issuer);
        } else {
            issuer = iCertificate->IssuerL();
        }
        CleanupStack::PushL(issuer);
        iDetails->insert(AdvSecSettingsCertificateModel::Issuer, CopyStringL(*issuer));
        CleanupStack::PopAndDestroy(issuer);
    }
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertDetailsBuilder::AppendSubjectL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertDetailsBuilder::AppendSubjectL()
{
    if (iCertificate) {
        HBufC *subject = NULL;
        ASSERT(iCertInfo);
        if (iCertInfo->CertificateFormat() == EX509Certificate) {
            CX509Certificate* cert = static_cast<CX509Certificate*>( iCertificate );
            X509CertNameParser::SubjectFullNameL(*cert, subject);
        } else {
            subject = iCertificate->SubjectL();
        }
        CleanupStack::PushL(subject);
        iDetails->insert(AdvSecSettingsCertificateModel::Subject, CopyStringL(*subject));
        CleanupStack::PopAndDestroy(subject);
    }
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertDetailsBuilder::AppendValidityPeriodL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertDetailsBuilder::AppendValidityPeriodL()
{
    if (iCertificate) {
        CValidityPeriod validityPeriod(iCertificate->ValidityPeriod());

        TDateTime sValidFrom(validityPeriod.Start().DateTime());
        QDateTime qValidFrom;
        CopyDateL(sValidFrom, qValidFrom);
        QString validFrom(qValidFrom.toString(Qt::DefaultLocaleShortDate));
        iDetails->insert(AdvSecSettingsCertificateModel::ValidFrom, validFrom);

        TDateTime sValidTo(validityPeriod.Finish().DateTime());
        QDateTime qValidTo;
        CopyDateL(sValidTo, qValidTo);
        QString validTo(qValidTo.toString(Qt::DefaultLocaleShortDate));
        iDetails->insert(AdvSecSettingsCertificateModel::ValidTo, validTo);
    }
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertDetailsBuilder::()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertDetailsBuilder::AppendLocationL()
{
    ASSERT(iCertInfo);
    QString location;
    if (iCertificateUrl) {
        if (iCertInfo->CertificateFormat() == EX509CertificateUrl) {
            location = CopyStringL(*iCertificateUrl);
        }
    } else {
        location = Location(iCertInfo->Token().TokenType().Type());
    }
    if (!location.isEmpty()) {
        iDetails->insert(AdvSecSettingsCertificateModel::Location, location);
    }
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertDetailsBuilder::AppendFormatL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertDetailsBuilder::AppendFormatL()
{
    ASSERT(iCertInfo);
    QString certFormat = CertificateFormatStringL(iCertInfo->CertificateFormat());
    iDetails->insert(AdvSecSettingsCertificateModel::Format, certFormat);
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertDetailsBuilder::AppendKeyUsageL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertDetailsBuilder::AppendKeyUsageL()
{
    // TODO: should be displayed only for user and device certicates
    if (iKeys.Count()) {
        const TInt KFirstKeyIndex = 0;
        // TODO: localized UI strings needed

        // Key Usage
        QString keyUsage;
        TKeyUsagePKCS15 usage = iKeys[KFirstKeyIndex]->Usage();
        if ((usage & EPKCS15UsageSign) || (usage & EPKCS15UsageSignRecover)) {
            keyUsage = QObject::tr("Client authentication");
        }
        if (usage & EPKCS15UsageNonRepudiation) {
            keyUsage = QObject::tr("Digital signing");
        }
        if (!keyUsage.isEmpty()) {
            iDetails->insert(AdvSecSettingsCertificateModel::KeyUsage, keyUsage);
        }

        // Key Location
        QString keyLocation;
        keyLocation = Location(iKeys[KFirstKeyIndex]->Token().TokenType().Type());
        if (!keyLocation.isEmpty()) {
            iDetails->insert(AdvSecSettingsCertificateModel::KeyLocation, keyLocation);
        }
    }
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertDetailsBuilder::AppendAlgorithmL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertDetailsBuilder::AppendAlgorithmL()
{
    if (iCertificate) {
        QString digestAlg = AlgorithmNameStringL(
            iCertificate->SigningAlgorithm().DigestAlgorithm().Algorithm());
        QString asymmerticAlg = AlgorithmNameStringL(
            iCertificate->SigningAlgorithm().AsymmetricAlgorithm().Algorithm());
        if (digestAlg.isNull() || asymmerticAlg.isNull()) {
            HBufC* unknownAlg = HbTextResolverSymbian::LoadLC(KLocAlgorithmUnknown);
            iDetails->insert(AdvSecSettingsCertificateModel::Algorithm, CopyStringL(*unknownAlg));
            CleanupStack::PopAndDestroy(unknownAlg);
        } else {
            QString algorithm(KFormatForAlgorithmNames.arg(digestAlg, asymmerticAlg));
            iDetails->insert(AdvSecSettingsCertificateModel::Algorithm, algorithm);
        }
    }
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertDetailsBuilder::AppendSerialNumberL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertDetailsBuilder::AppendSerialNumberL()
{
    if (iCertificate) {
        TPtrC8 serialNumber = iCertificate->SerialNumber();
        iDetails->insert(AdvSecSettingsCertificateModel::SerialNumber,
                HexFormattedStringL(serialNumber));
    }
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertDetailsBuilder::AppendFingerprintsL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertDetailsBuilder::AppendFingerprintsL()
{
    if (iCertificate) {
        // Fingerprint (SHA1)
        TPtrC8 sha1Fingerprint = iCertificate->Fingerprint();
        iDetails->insert(AdvSecSettingsCertificateModel::FingerprintSHA1,
                HexFormattedStringL(sha1Fingerprint, KCertManUiTwoDigitsInBlock));

        // Fingerprint (MD5)
        CMD5* md5 = CMD5::NewL();
        CleanupStack::PushL(md5);
        const TInt KRsaFingerprintLength = 20;
        TBuf8<KRsaFingerprintLength> rsaFingerprint = md5->Hash(iCertificate->Encoding());
        iDetails->insert(AdvSecSettingsCertificateModel::FingerprintMD5,
                HexFormattedStringL(rsaFingerprint, KCertManUiTwoDigitsInBlock));
        CleanupStack::PopAndDestroy(md5);
    }
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertDetailsBuilder::AppendPublicKeyL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertDetailsBuilder::AppendPublicKeyL()
{
    if (iCertificate) {
        TPtrC8 publicKey = iCertificate->PublicKey().KeyData();
        iDetails->insert(AdvSecSettingsCertificateModel::PublicKey,
                HexFormattedStringL(publicKey, KCertManUiTwoDigitsInBlock));
    }
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsCertDetailsBuilder::AppendTrustedSitesL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsCertDetailsBuilder::AppendTrustedSitesL()
{
    if (iCertificate) {
        CTrustSitesStore* trustedSitesStore = CTrustSitesStore::NewL();
        CleanupStack::PushL(trustedSitesStore);

        QStringList trustedSitesList;
        RPointerArray<HBufC> trustedSitesArray;
        CleanupResetAndDestroyPushL(trustedSitesArray);
        trustedSitesStore->GetTrustedSitesL(iCertificate->Encoding(), trustedSitesArray);
        TInt count = trustedSitesArray.Count();
        if (count) {
            for (TInt index = 0; index < trustedSitesArray.Count(); index++) {
                trustedSitesList.append(CopyStringL(*(trustedSitesArray[index])));
            }
            // TODO: might be better to return trusted sites as a QStringList (in QVariant)
            iDetails->insert(AdvSecSettingsCertificateModel::TrustedSites,
                trustedSitesList.join(KTrustedSitesSeparator));
        }
        CleanupStack::PopAndDestroy(2, trustedSitesStore); // trustedSitesArray, trustedSitesStore
    }
}

