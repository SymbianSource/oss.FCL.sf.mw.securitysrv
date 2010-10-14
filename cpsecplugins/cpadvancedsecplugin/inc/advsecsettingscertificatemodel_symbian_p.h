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

#ifndef CPADVSECSETTINGSMODELPRIVATE_SYMBIAN_H
#define CPADVSECSETTINGSMODELPRIVATE_SYMBIAN_H

#include <e32base.h>                // CActive
#include <f32file.h>                // RFs
#include <unifiedcertstore.h>       // CUnifiedCertStore related
#include <QMap>

class AdvSecSettingsCertificateModel;
class AdvSecSettingsCertificate;
class CAdvSecSettingsCertListBuilder;
class CAdvSecSettingsCertDetailsBuilder;
class CCertificateAppInfoManager;
class CAdvSecSettingsCertMover;


/**
 * Symbian specific advanced security settings certificate model.
 */
class AdvSecSettingsCertificateModelPrivate : public CActive
{
public:     // constructor and destructor
    explicit AdvSecSettingsCertificateModelPrivate(
        AdvSecSettingsCertificateModel *q = 0);
    ~AdvSecSettingsCertificateModelPrivate();

public:     // new functions
    void initialize();
    void getCertificates(QList<AdvSecSettingsCertificate*> &certList);
    void getCertificateDetails(const AdvSecSettingsCertificate &cert,
        QMap<int,QString> &details);
    bool isDeletable(const AdvSecSettingsCertificate &cert) const;
    void deleteCertificate(const AdvSecSettingsCertificate &cert);
    void getTrustSettings(const AdvSecSettingsCertificate &cert,
        QMap<int,bool> &usageIdAndTrust);
    void setTrustSettings(const AdvSecSettingsCertificate &cert,
        const QMap<int,bool> &usageIdAndTrust);
    void getCertificateUsageNames(QMap<int,QString> &usageIdAndName);
    void moveToPersonalCertificates(const AdvSecSettingsCertificate &cert);
    void moveToDeviceCertificates(const AdvSecSettingsCertificate &cert);

protected:  // from CActive
    void DoCancel();
    void RunL();
    TInt RunError(TInt aError);

private:    // new functions
    Q_DISABLE_COPY(AdvSecSettingsCertificateModelPrivate)
    void ConstructL();
    TInt ResetCertAttributeFilter();
    TInt ResetCertInfoArray();
    void ReturnTrustSettingsDetails();
    void DoGetCertificateUsageNamesL(QMap<int,QString> &usageIdAndName);
    const CCTCertInfo *CertificateInfo(const AdvSecSettingsCertificate &cert) const;

private:    // data
    AdvSecSettingsCertificateModel *q_ptr;

    enum TState {
        ENotInitialized,
        EInitializing,
        EIdle,
        EGettingCertificatesList,
        EGettingCertificatesRetrieve,
        EBuildingCertificateDetails,
        EDeletingCertificate,
        EReadingApplications,
        EWritingApplications,
        EMovingCertificateToPersonalStore,
        EMovingCertificateToDeviceStore
    } iState;

    RFs iFs;
    CUnifiedCertStore *iCertStore;

    /*
     * Common to all certificate specific asynchronous methods. Member iCertInfo
     * contains pointer to the current certificate being processed. Member
     * iCertInfoArray contains all certificates read from certificate databases.
     */
    const CCTCertInfo *iCertInfo;  // not owned
    RMPointerArray<CCTCertInfo> iCertInfoArray;

    /*
     * for getCertificates method. Certificate list is constructed using
     * iCertListBuilder member, which returns data directly to the client.
     */
    CAdvSecSettingsCertListBuilder *iCertListBuilder;

    /*
     * For getCertificateDetails method. Certificate details are constructed
     * using iDetailsBuilder member, which returns data directly to the client.
     */
    CAdvSecSettingsCertDetailsBuilder *iDetailsBuilder;

    /*
     * For getTrustSettings method. Data is retrieved  from certificate store into
     * iApplications member, and then returned to client via iTrustedUids member.
     */
    QMap<int,bool> *iTrustedUids;   // not owned
    RArray<TUid> iApplications;

    /*
     * For getTrustSettings and getCertificateUsageNames methods to get possible
     * certificate application (usage) ids and their names.
     */
    CCertificateAppInfoManager *iCertAppInfoManager;

    /*
     * For moveToPersonalCertificates and moveToDeviceCertificates methods.
     * Takes care of moving certificate and it's key to given target store.
     */
    CAdvSecSettingsCertMover *iMover;
};

#endif // CPADVSECSETTINGSMODELPRIVATE_SYMBIAN_H

