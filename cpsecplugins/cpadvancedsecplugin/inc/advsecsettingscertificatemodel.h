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
* Description:  Certificate handling model in advanced security settings
*
*/

#ifndef ADVSECSETTINGSCERTIFICATEMODEL_H
#define ADVSECSETTINGSCERTIFICATEMODEL_H

#include <QObject>
#include <QList>
#include <QMap>

class AdvSecSettingsCertificateModelPrivate;
class AdvSecSettingsCertificate;


/**
 * Certificate handling engine in advanced security settings control panel plugin.
 */
class AdvSecSettingsCertificateModel : public QObject
{
    Q_OBJECT

public:     // constructor and destructor
    explicit AdvSecSettingsCertificateModel(QObject *parent = 0);
    virtual ~AdvSecSettingsCertificateModel();

public:     // new definitions
    enum AdvSecSettingsCertificateDetailsField {
        Label,
        Issuer,
        Subject,
        ValidFrom,
        ValidTo,
        Location,
        Format,
        KeyUsage,
        KeyLocation,
        Algorithm,
        SerialNumber,
        FingerprintSHA1,
        FingerprintMD5,
        PublicKey,
        TrustedSites
    };

public:     // new functions
    void initialize();  // asynchronous
    void getCertificates(QList<AdvSecSettingsCertificate*> &certList);  // asynchronous
    void getCertificateDetails(const AdvSecSettingsCertificate &cert,
        QMap<int,QString> &details);  // asynchronous
    bool isDeletable(const AdvSecSettingsCertificate &cert) const;
    void deleteCertificate(const AdvSecSettingsCertificate &cert);  // asynchronous
    void getTrustSettings(const AdvSecSettingsCertificate &cert,
        QMap<int,bool> &usageIdAndTrust);  // asynchronous
    void setTrustSettings(const AdvSecSettingsCertificate &cert,
        const QMap<int,bool> &usageIdAndTrust);  // asynchronous
    void getCertificateUsageNames(QMap<int,QString> &usageIdAndName);
    void moveToPersonalCertificates(const AdvSecSettingsCertificate &cert);  // asynchronous
    void moveToDeviceCertificates(const AdvSecSettingsCertificate &cert);  // asynchronous

signals:    // new signals
    void initializeCompleted();
    void getCertificatesCompleted();
    void getCertificateDetailsCompleted();
    void deleteCertificateCompleted();
    void getTrustSettingsCompleted();
    void setTrustSettingsCompleted();
    void moveToPersonalCertificatesCompleted();
    void moveToDeviceCertificatesCompleted();
    void errorOccurred(int error);

protected:  // new functions
    void handleInitializeCompleted();
    void handleGetCertificatesCompleted();
    void handleGetCertificateDetailsCompleted();
    void handleDeleteCertificateCompleted();
    void handleGetTrustSettingsCompleted();
    void handleSetTrustSettingsCompleted();
    void handleMoveToPersonalCertificateCompleted();
    void handleMoveToDeviceCertificatesCompleted();
    void handleError(int error);

private:    // data
    Q_DISABLE_COPY(AdvSecSettingsCertificateModel)
    friend class AdvSecSettingsCertificateModelPrivate;
    AdvSecSettingsCertificateModelPrivate *d_ptr;
};

#endif // ADVSECSETTINGSCERTIFICATEMODEL_H
