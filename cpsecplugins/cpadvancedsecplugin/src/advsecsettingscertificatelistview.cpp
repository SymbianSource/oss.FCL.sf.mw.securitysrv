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

#include "advsecsettingscertificatelistview.h"
#include "advsecsettingscertificatemodel.h"
#include "advsecsettingscertificatedetailview.h"
#include "advsecsettingscerttrustsettingsview.h"
#include <QGraphicsLinearLayout>
#include <HbAbstractViewItem>
#include <HbGroupBox>
#include <HbListWidget>
#include <HbListWidgetItem>
#include <HbLabel>
#include <HbStackedWidget>


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateListView::AdvSecSettingsCertificateListView()
// ---------------------------------------------------------------------------
//
AdvSecSettingsCertificateListView::AdvSecSettingsCertificateListView(
    QGraphicsItem *parent) : AdvSecSettingsViewBase(0, parent),
    mCertType(AdvSecSettingsCertificate::NotDefined),
    mListWidget(0), mEmptyText(0), mRetrievingText(0), mStackedWidget(0),
    mIsRetrieving(true), mModelCertificateList(), mDisplayedCertificates(),
    mModel(0), mDetailView(), mTrustSettingsView(0), mCurrentCertificate(0)
{
    mModel = new AdvSecSettingsCertificateModel(this);
    mModel->initialize();
    connect(mModel, SIGNAL(initializeCompleted()),
        this, SLOT(readAllCertificatesFromModel()));
    connect(mModel, SIGNAL(getCertificatesCompleted()),
        this, SLOT(displayAllCertificatesReadFromModel()));
    connect(mModel, SIGNAL(deleteCertificateCompleted()),
        this, SLOT(refreshAfterCurrentCertRemoved()));
    connect(mModel, SIGNAL(moveToPersonalCertificatesCompleted()),
        this, SLOT(refreshAfterCurrentCertRemoved()));
    connect(mModel, SIGNAL(moveToDeviceCertificatesCompleted()),
        this, SLOT(refreshAfterCurrentCertRemoved()));
    connect(mModel, SIGNAL(errorOccurred(int)),
        this, SLOT(displayError(int)));

    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical);

    // View title
    mViewLabel = new HbGroupBox;
    layout->addItem(mViewLabel);

    // Certificate list or empty text
    mStackedWidget = new HbStackedWidget;

    mListWidget = new HbListWidget;
    connect(mListWidget, SIGNAL(activated(QModelIndex)),
        this, SLOT(openCertificate(QModelIndex)));
    connect(mListWidget, SIGNAL(longPressed(HbAbstractViewItem*, QPointF)),
        this, SLOT(indicateLongPress(HbAbstractViewItem*, QPointF)));
    mStackedWidget->addWidget(mListWidget);

    // TODO: localized UI string needed
    mEmptyText = new HbLabel(tr("No certificates"));
    mEmptyText->setAlignment(Qt::AlignCenter);
    mStackedWidget->addWidget(mEmptyText);

    mRetrievingText = new HbLabel(tr("Retrieving..."));
    mRetrievingText->setAlignment(Qt::AlignCenter);
    mStackedWidget->addWidget(mRetrievingText);

    mStackedWidget->setCurrentWidget(mRetrievingText);
    layout->addItem(mStackedWidget);

    setLayout(layout);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateListView::~AdvSecSettingsCertificateListView()
// ---------------------------------------------------------------------------
//
AdvSecSettingsCertificateListView::~AdvSecSettingsCertificateListView()
{
    clearModelCertificates();
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateListView::displayCertificates()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateListView::displayCertificates(
    AdvSecSettingsCertificate::CertificateType type)
{
    if (type != mCertType) {
        mCertType = type;
        switch (mCertType) {
        case AdvSecSettingsCertificate::AuthorityCertificate:
            mViewLabel->setHeading(hbTrId("txt_certificate_manager_subhead_authority_certific"));
            break;
        case AdvSecSettingsCertificate::TrustedSiteCertificate:
            mViewLabel->setHeading(hbTrId("txt_certificate_manager_subhead_trusted_site_certi"));
            break;
        case AdvSecSettingsCertificate::PersonalCertificate:
            mViewLabel->setHeading(hbTrId("txt_certificate_manager_subhead_personal_certifica"));
            break;
        case AdvSecSettingsCertificate::DeviceCertificate:
            mViewLabel->setHeading(hbTrId("txt_certificate_manager_subhead_device_certificate"));
            break;
        default:
            break;
        }
    }
    refreshDisplayedCertificates();
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateListView::setCurrentIndex()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateListView::setCurrentIndex(const QModelIndex& index)
{
    mListWidget->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateListView::indicateLongPress()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateListView::indicateLongPress(
    HbAbstractViewItem *item, const QPointF &position)
{
    Q_ASSERT(item != 0);
    mCurrentCertificate = mDisplayedCertificates.at(item->modelIndex().row());

    clearItemSpecificMenu();
    addItemSpecificMenuAction(hbTrId("txt_common_menu_open"),
        this, SLOT(openCurrentCertificate()));

    switch (mCertType) {
    case AdvSecSettingsCertificate::AuthorityCertificate:
        addItemSpecificMenuAction(hbTrId("txt_certificate_manager_menu_trust_settings"),
            this, SLOT(displayCurrentCertTrustSettings()));
        break;
    case AdvSecSettingsCertificate::TrustedSiteCertificate:
        break;
    case AdvSecSettingsCertificate::PersonalCertificate:
        addItemSpecificMenuAction(hbTrId("txt_certificate_manager_menu_move_to_device_certif"),
            this, SLOT(moveCurrentCertToDeviceCertificates()));
        break;
    case AdvSecSettingsCertificate::DeviceCertificate:
        addItemSpecificMenuAction(hbTrId("txt_certificate_manager_menu_move_to_personal_cert"),
            this, SLOT(moveCurrentCertToPersonalCertificates()));
        break;
    default:
        break;
    }

    Q_ASSERT(mCurrentCertificate != 0);
    if (mModel->isDeletable(*mCurrentCertificate)) {
        addItemSpecificMenuAction(hbTrId("txt_common_menu_delete"),
            this, SLOT(deleteCurrentCertificate()));
    }

    displayItemSpecificMenu(position);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateListView::openCertificate()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateListView::openCertificate(const QModelIndex& modelIndex)
{
    mCurrentCertificate = mDisplayedCertificates.at(modelIndex.row());
    openCurrentCertificate();
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateListView::openCurrentCertificate()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateListView::openCurrentCertificate()
{
    if (!mDetailView) {
        mDetailView = new AdvSecSettingsCertificateDetailView(*mModel, this);
    }
    Q_ASSERT(mCurrentCertificate != 0);
    mDetailView->setCertificate(*mCurrentCertificate);
    displayView(mDetailView);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateListView::displayCurrentCertTrustSettings()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateListView::displayCurrentCertTrustSettings()
{
    if (!mTrustSettingsView) {
        mTrustSettingsView = new AdvSecSettingsCertTrustSettingsView(*mModel, this);
    }
    Q_ASSERT(mCurrentCertificate != 0);
    mTrustSettingsView->setCertificate(*mCurrentCertificate);
    displayView(mTrustSettingsView);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateListView::deleteCurrentCertificate()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateListView::deleteCurrentCertificate()
{
    Q_ASSERT(mCurrentCertificate != 0);
    // TODO: localized UI string needed
    QString confirmText(tr("Delete %1?").arg(mCurrentCertificate->label()));
    displayQuestionNote(confirmText, this, SLOT(deleteConfirmationAccepted()));
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateListView::deleteConfirmationAccepted()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateListView::deleteConfirmationAccepted()
{
    Q_ASSERT(mCurrentCertificate != 0);
    mModel->deleteCertificate(*mCurrentCertificate);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateListView::moveCurrentCertToDeviceCertificates()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateListView::moveCurrentCertToDeviceCertificates()
{
    Q_ASSERT(mCurrentCertificate != 0);
    QString confirmText(hbTrId("txt_certificate_info_device_certificates_can_be_us"));
    displayQuestionNote(confirmText, this, SLOT(moveToDeviceCertsConfirmationAccepted()));
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateListView::moveToDeviceCertsConfirmationAccepted()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateListView::moveToDeviceCertsConfirmationAccepted()
{
    Q_ASSERT(mCurrentCertificate != 0);
    mModel->moveToDeviceCertificates(*mCurrentCertificate);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateListView::moveCurrentCertToPersonalCertificates()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateListView::moveCurrentCertToPersonalCertificates()
{
    Q_ASSERT(mCurrentCertificate != 0);
    // TODO: localized UI string needed
    QString confirmText(tr("Using personal certificates may require user confirmation. Continue?"));
    displayQuestionNote(confirmText, this, SLOT(moveToPersonalCertsConfirmationAccepted()));
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateListView::moveToPersonalCertsConfirmationAccepted()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateListView::moveToPersonalCertsConfirmationAccepted()
{
    Q_ASSERT(mCurrentCertificate != 0);
    mModel->moveToPersonalCertificates(*mCurrentCertificate);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateListView::readAllCertificatesFromModel()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateListView::readAllCertificatesFromModel()
{
    clearModelCertificates();
    mModel->getCertificates(mModelCertificateList);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateListView::displayAllCertificatesReadFromModel()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateListView::displayAllCertificatesReadFromModel()
{
    if (mCertType != AdvSecSettingsCertificate::NotDefined) {
        mIsRetrieving = false;
        refreshDisplayedCertificates();
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateListView::clearModelCertificates()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateListView::clearModelCertificates()
{
    if (!mModelCertificateList.isEmpty()) {
        QMutableListIterator<AdvSecSettingsCertificate *> iter(mModelCertificateList);
        while (iter.hasNext()) {
            delete iter.next();
        }
        mModelCertificateList.clear();
    }
    mCurrentCertificate = 0;
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateListView::refreshDisplayedCertificates()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateListView::refreshDisplayedCertificates()
{
    int currentRow = mListWidget->currentRow();
    mListWidget->clear();
    mDisplayedCertificates.clear();
    QListIterator<AdvSecSettingsCertificate *> iter(mModelCertificateList);
    while (iter.hasNext()) {
        AdvSecSettingsCertificate *cert = iter.next();
        if (cert->certType() == mCertType) {
            mDisplayedCertificates.append(cert);
            HbListWidgetItem *item = new HbListWidgetItem;
            item->setText(cert->label());
            mListWidget->addItem(item);
        }
    }
    if (mListWidget->count()) {
        if (currentRow < mListWidget->count()) {
            mListWidget->setCurrentRow(currentRow);
        }
        mStackedWidget->setCurrentWidget(mListWidget);
    } else {
        if (mIsRetrieving) {
            mStackedWidget->setCurrentWidget(mRetrievingText);
        } else {
            mStackedWidget->setCurrentWidget(mEmptyText);
        }
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateListView::refreshAfterCurrentCertRemoved()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateListView::refreshAfterCurrentCertRemoved()
{
    Q_ASSERT(mCurrentCertificate != 0);

    QString currentCertLabel = mCurrentCertificate->label();
    for (TInt row = 0; row < mListWidget->count(); row++) {
        HbListWidgetItem *item = mListWidget->item(row);
        if (item->text() == currentCertLabel) {
            item = mListWidget->takeItem(row);
            delete item;
        }
    }
    if (mListWidget->count()) {
        mStackedWidget->setCurrentWidget(mListWidget);
    } else {
        mStackedWidget->setCurrentWidget(mEmptyText);
    }

    mDisplayedCertificates.removeAll(mCurrentCertificate);
    mCurrentCertificate = 0;

    readAllCertificatesFromModel();
}

