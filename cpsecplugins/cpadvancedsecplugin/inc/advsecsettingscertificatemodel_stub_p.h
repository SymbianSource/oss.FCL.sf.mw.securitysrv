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
* Description:  Generic stub for platform specific private certificate handling model
*
*/

#ifndef ADVSECSETTINGSCERTIFICATEMODELPRIVATE_STUB_H
#define ADVSECSETTINGSCERTIFICATEMODELPRIVATE_STUB_H

#include <QObject>
#include <QMap>

class AdvSecSettingsCertificateModel;
class AdvSecSettingsCertificate;


/**
 * Generic stub for platform specific private certificate handling model.
 * This is just a stub. Real functionality is provided in platform specific
 * AdvSecSettingsCertificateModelPrivate implementations.
 */
class AdvSecSettingsCertificateModelPrivate : public QObject
{
    Q_OBJECT

public:
    explicit AdvSecSettingsCertificateModelPrivate(
        AdvSecSettingsCertificateModel *q);
    virtual ~AdvSecSettingsCertificateModelPrivate();

    void initialize();
    void getCertificates(QList<AdvSecSettingsCertificate *> &certList);
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

private:
    Q_DISABLE_COPY(AdvSecSettingsCertificateModelPrivate)
    AdvSecSettingsCertificateModel *q_ptr;
};

#endif // ADVSECSETTINGSCERTIFICATEMODELPRIVATE_STUB_H

