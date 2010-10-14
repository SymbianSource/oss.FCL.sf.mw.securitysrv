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
* Description:  Certificate list view in advanced security settings
*
*/

#ifndef ADVSECSETTINGSCERTIFICATELISTVIEW_H
#define ADVSECSETTINGSCERTIFICATELISTVIEW_H

#include "advsecsettingsviewbase.h"
#include "advsecsettingscertificate.h"  // CertificateType

class HbAbstractViewItem;
class HbGroupBox;
class HbListWidget;
class HbLabel;
class HbStackedWidget;
class AdvSecSettingsCertificateModel;
class AdvSecSettingsCertificateDetailView;
class AdvSecSettingsCertTrustSettingsView;


/**
 * Certificate list view in advanced security settings. Certificate
 * list view shows certificate list -- either authority certificates,
 * list, trusted site certificates list, personal certificates list,
 * or dervice certificates list.
 */
class AdvSecSettingsCertificateListView : public AdvSecSettingsViewBase
{
	Q_OBJECT

public:    // constructor and destructor
    explicit AdvSecSettingsCertificateListView(QGraphicsItem *parent = 0);
    virtual ~AdvSecSettingsCertificateListView();

public:    // new functions
    void displayCertificates(AdvSecSettingsCertificate::CertificateType type);
    void setCurrentIndex(const QModelIndex& index);

private slots:
    void indicateLongPress(HbAbstractViewItem *item, const QPointF &position);
    void openCertificate(const QModelIndex& modelIndex);
    void openCurrentCertificate();
    void displayCurrentCertTrustSettings();
    void deleteCurrentCertificate();
    void deleteConfirmationAccepted();
    void moveCurrentCertToDeviceCertificates();
    void moveToDeviceCertsConfirmationAccepted();
    void moveCurrentCertToPersonalCertificates();
    void moveToPersonalCertsConfirmationAccepted();
    void readAllCertificatesFromModel();
    void displayAllCertificatesReadFromModel();
    void refreshAfterCurrentCertRemoved();

private:    // new functions
    void clearModelCertificates();
    void refreshDisplayedCertificates();

private:    // data
    AdvSecSettingsCertificate::CertificateType mCertType;
    HbGroupBox *mViewLabel;
    HbListWidget *mListWidget;      // Displayed cert list, for example Authority certificates
    HbLabel *mEmptyText;
    HbLabel *mRetrievingText;
    HbStackedWidget *mStackedWidget;
    bool mIsRetrieving;
    QList<AdvSecSettingsCertificate *> mModelCertificateList;   // All certs read from model
    QList<AdvSecSettingsCertificate *> mDisplayedCertificates;  // Displayed (like Authority) certs
    AdvSecSettingsCertificateModel *mModel;
    AdvSecSettingsCertificateDetailView *mDetailView;
    AdvSecSettingsCertTrustSettingsView *mTrustSettingsView;
    AdvSecSettingsCertificate *mCurrentCertificate; // not owned
};

#endif // ADVSECSETTINGSCERTIFICATELISTVIEW_H

