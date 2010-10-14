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
* Description:  Certificate trust settings view in advanced security settings
*
*/

#ifndef ADVSECSETTINGSCERTTRUSTSETTINGSVIEW_H
#define ADVSECSETTINGSCERTTRUSTSETTINGSVIEW_H

#include "advsecsettingsviewbase.h"

class AdvSecSettingsCertificate;
class AdvSecSettingsCertificateModel;
class HbGroupBox;
class HbDataForm;
class HbDataFormModel;


/**
 * Certificate trust settings view in advanced security settings. Certificate
 * detail view shows certificate details (subject, issuer, validity
 * dates, fingerprints, etc).
 */
class AdvSecSettingsCertTrustSettingsView : public AdvSecSettingsViewBase
{
	Q_OBJECT

public:    // constructor and destructor
    explicit AdvSecSettingsCertTrustSettingsView(
        AdvSecSettingsCertificateModel &model, QGraphicsItem *parent = 0);
    virtual ~AdvSecSettingsCertTrustSettingsView();

public:    // new functions
    void setCertificate(const AdvSecSettingsCertificate &cert);

private slots:
    void refreshDisplay();
    void toggleChange(const QModelIndex &itemIndex);
    void saveTrustSettings();
    void deleteCertificate();
    void deleteConfirmationAccepted();

private:    // new functions
    QString usageName(int usageId);

private:    // data
    AdvSecSettingsCertificateModel &mModel;
    const AdvSecSettingsCertificate *mCertificate;    // not owned
    bool mIsCertDeletable;
    HbGroupBox *mViewLabel;
    HbDataForm *mDataForm;
    HbDataFormModel *mDataFormModel;
    QMap<int,bool> mUsageIdAndTrust;
    QMap<int,QString> mUsageIdAndName;
};

#endif // ADVSECSETTINGSCERTTRUSTSETTINGSVIEW_H

