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
* Description:  Certificate class in advanced security settings
*
*/

#ifndef ADVSECSETTINGSCERTIFICATE_H
#define ADVSECSETTINGSCERTIFICATE_H

#include <QObject>


/**
 * Certificate class in advances security settings.
 */
class AdvSecSettingsCertificate : public QObject
{
    Q_OBJECT

public:     // new definitions
    enum CertificateType {
        NotDefined,
        AuthorityCertificate,
        TrustedSiteCertificate,
        PersonalCertificate,
        DeviceCertificate
    };

public:     // constructor and destructor
    explicit AdvSecSettingsCertificate(QObject *parent = 0);
    virtual ~AdvSecSettingsCertificate();

public:     // new functions
    CertificateType certType() const;
    void setCertType(CertificateType type);

    const QString &label() const;
    void setLabel(const QString &label);

    int modelIndex() const;
    void setModelIndex(int index);

private:    // new functions
    Q_DISABLE_COPY(AdvSecSettingsCertificate)

private:    // data
    CertificateType mCertType;
    QString mLabel;
    int mModelIndex;
};


#endif  // ADVSECSETTINGSCERTIFICATE_H
