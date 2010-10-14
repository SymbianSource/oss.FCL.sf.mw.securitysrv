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

#include "advsecsettingscerttrustsettingsview.h"
#include "advsecsettingstrustedcertusageuids.h"
#include "advsecsettingscertificatemodel.h"
#include "advsecsettingscertificate.h"
#include <QGraphicsLinearLayout>
#include <HbGroupBox>
#include <HbDataForm>
#include <HbDataFormModel>
#include <HbMenu>
#include <HbAction>

const QString KText("text");
const QString KAdditionalText("additionalText");
const QString KHexUsageIdFormat("0x%1");
const int KHexFieldWidth = 8;
const int KHexBase = 16;


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// AdvSecSettingsCertTrustSettingsView::AdvSecSettingsCertTrustSettingsView()
// ---------------------------------------------------------------------------
//
AdvSecSettingsCertTrustSettingsView::AdvSecSettingsCertTrustSettingsView(
    AdvSecSettingsCertificateModel &model, QGraphicsItem *parent) :
    AdvSecSettingsViewBase(0, parent), mModel(model), mCertificate(0),
    mIsCertDeletable(false), mViewLabel(0), mDataForm(0), mDataFormModel(0)
{
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical);

    mViewLabel = new HbGroupBox;
    layout->addItem(mViewLabel);

    HbScrollArea *scrollArea = new HbScrollArea;
    mDataForm = new HbDataForm;
    scrollArea->setContentWidget(mDataForm);
    layout->addItem(scrollArea);

    setLayout(layout);

    connect(&mModel, SIGNAL(getTrustSettingsCompleted()), this, SLOT(refreshDisplay()));
    connect(&mModel, SIGNAL(deleteCertificateCompleted()), this, SLOT(close()));
    connect(this, SIGNAL(aboutToClose()), this, SLOT(saveTrustSettings()));
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertTrustSettingsView::~AdvSecSettingsCertTrustSettingsView()
// ---------------------------------------------------------------------------
//
AdvSecSettingsCertTrustSettingsView::~AdvSecSettingsCertTrustSettingsView()
{
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertTrustSettingsView::setCertificate()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertTrustSettingsView::setCertificate(const AdvSecSettingsCertificate &cert)
{
    mCertificate = &cert;
    mIsCertDeletable = mModel.isDeletable(cert);

    menu()->clearActions();
    if (mIsCertDeletable) {
        HbAction *deleteAction = new HbAction(hbTrId("txt_common_menu_delete"));
        connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteCertificate()));
        menu()->addAction(deleteAction);
    }

    mUsageIdAndTrust.clear();
    mModel.getTrustSettings(cert, mUsageIdAndTrust);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertTrustSettingsView::refreshDisplay()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertTrustSettingsView::refreshDisplay()
{
    if (mDataFormModel) {
        mDataForm->setModel(0);
        delete mDataFormModel;
        mDataFormModel = 0;
    }
    mDataFormModel = new HbDataFormModel(this);

    QMapIterator<int,bool> iter(mUsageIdAndTrust);
    while (iter.hasNext()) {
        iter.next();
        HbDataFormModelItem *item = mDataFormModel->appendDataFormItem(
            HbDataFormModelItem::ToggleValueItem, usageName(iter.key()));
        if (iter.value()) {
            // TODO: localized UI string needed
            item->setContentWidgetData(KText, tr("On"));
            item->setContentWidgetData(KAdditionalText, tr("Off"));
        } else {
            // TODO: localized UI string needed
            item->setContentWidgetData(KText, tr("Off"));
            item->setContentWidgetData(KAdditionalText, tr("On"));
        }
        if (!mIsCertDeletable) {
            item->setEnabled(false);
        }
    }
    connect(mDataFormModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
        this, SLOT(toggleChange(QModelIndex)));

    mViewLabel->setHeading(mCertificate->label());
    mDataForm->setModel(mDataFormModel);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertTrustSettingsView::toggleChange()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertTrustSettingsView::toggleChange(const QModelIndex &itemIndex)
{
    if (mIsCertDeletable) {
        HbDataFormModelItem *item = mDataFormModel->itemFromIndex(itemIndex);

        int usageId = 0;
        QString label = item->label();
        QMapIterator<int,QString> iter(mUsageIdAndName);
        while (iter.hasNext() && !usageId) {
            iter.next();
            if (label == iter.value()) {
                usageId = iter.key();
            }
        }

        if (usageId) {
            QVariant data = item->contentWidgetData(KText);
            if (data.toString() == tr("On")) {      // TODO: use localized UI string
                mUsageIdAndTrust[usageId] = true;
            } else {
                mUsageIdAndTrust[usageId] = false;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertTrustSettingsView::saveTrustSettings()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertTrustSettingsView::saveTrustSettings()
{
    if (mIsCertDeletable) {
        Q_ASSERT(mCertificate != 0);
        mModel.setTrustSettings(*mCertificate, mUsageIdAndTrust);
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertTrustSettingsView::deleteCertificate()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertTrustSettingsView::deleteCertificate()
{
    Q_ASSERT(mCertificate != 0);
    // TODO: localized UI string needed
    QString confirmText(tr("Delete %1?").arg(mCertificate->label()));
    displayQuestionNote(confirmText, this, SLOT(deleteConfirmationAccepted()));
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertTrustSettingsView::deleteConfirmationAccepted()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertTrustSettingsView::deleteConfirmationAccepted()
{
    Q_ASSERT(mCertificate != 0);
    mModel.deleteCertificate(*mCertificate);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertTrustSettingsView::usageName()
// ---------------------------------------------------------------------------
//
QString AdvSecSettingsCertTrustSettingsView::usageName(int usageId)
{
    if (mUsageIdAndName.isEmpty()) {
        mModel.getCertificateUsageNames(mUsageIdAndName);
    }

    QString name;
    if (mUsageIdAndName.contains(usageId)) {
        name = mUsageIdAndName.value(usageId);
    } else {
        name = QString(KHexUsageIdFormat).arg(usageId, KHexFieldWidth, KHexBase);
    }
    return name;
}

