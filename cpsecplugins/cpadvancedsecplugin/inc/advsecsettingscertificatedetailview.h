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
* Description:  Certificate detail view in advanced security settings
*
*/

#ifndef ADVSECSETTINGSCERTIFICATEDETAILVIEW_H
#define ADVSECSETTINGSCERTIFICATEDETAILVIEW_H

#include "advsecsettingsviewbase.h"

class AdvSecSettingsCertificate;
class AdvSecSettingsCertificateModel;
class HbGroupBox;
class HbScrollArea;
class HbLabel;


/**
 * Certificate detail view in advanced security settings. Certificate
 * detail view shows certificate details (subject, issuer, validity
 * dates, fingerprints, etc).
 */
class AdvSecSettingsCertificateDetailView : public AdvSecSettingsViewBase
{
	Q_OBJECT

public:    // constructor and destructor
    explicit AdvSecSettingsCertificateDetailView(AdvSecSettingsCertificateModel &model,
        QGraphicsItem *parent = 0);
    virtual ~AdvSecSettingsCertificateDetailView();

public:    // new functions
    void setCertificate(const AdvSecSettingsCertificate &cert);

private slots:
    void certificateDetailsCompleted();
    void showTrustSettings();
    void deleteCertificate();
    void deleteConfirmationAccepted();

private:    // data
    AdvSecSettingsCertificateModel &mModel;
    const AdvSecSettingsCertificate *mCertificate;    // not owned
    QMap<int, QString> mDetails;
    HbGroupBox *mViewLabel;
    HbScrollArea *mScrollArea;
    HbLabel *mDetailsText;
};

#endif // ADVSECSETTINGSCERTIFICATEDETAILVIEW_H

