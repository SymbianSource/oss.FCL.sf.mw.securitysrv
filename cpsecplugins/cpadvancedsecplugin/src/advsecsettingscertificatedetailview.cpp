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

#include "advsecsettingscertificatedetailview.h"
#include "advsecsettingscertificatemodel.h"
#include "advsecsettingscertificate.h"
#include "advsecsettingscerttrustsettingsview.h"
#include <QGraphicsLinearLayout>
#include <QTextStream>
#include <HbGroupBox>
#include <HbScrollArea>
#include <HbLabel>
#include <HbMenu>
#include <HbAction>


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateDetailView::AdvSecSettingsCertificateDetailView()
// ---------------------------------------------------------------------------
//
AdvSecSettingsCertificateDetailView::AdvSecSettingsCertificateDetailView(
        AdvSecSettingsCertificateModel &model, QGraphicsItem *parent) :
        AdvSecSettingsViewBase(0, parent), mModel(model), mCertificate(0),
        mDetails(), mViewLabel(0), mScrollArea(0), mDetailsText(0)
{
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical);

    mViewLabel = new HbGroupBox;
    layout->addItem(mViewLabel);

    mScrollArea = new HbScrollArea;
    mDetailsText = new HbLabel;
    mDetailsText->setTextWrapping(Hb::TextWordWrap);
    mScrollArea->setContentWidget(mDetailsText);
    mScrollArea->setVerticalScrollBarPolicy(HbScrollArea::ScrollBarAlwaysOn);
    layout->addItem(mScrollArea);

    setLayout(layout);

    connect(&mModel, SIGNAL(getCertificateDetailsCompleted()),
        this, SLOT(certificateDetailsCompleted()));
    connect(&mModel, SIGNAL(deleteCertificateCompleted()),
        this, SLOT(displayPreviousView()));
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateDetailView::~AdvSecSettingsCertificateDetailView()
// ---------------------------------------------------------------------------
//
AdvSecSettingsCertificateDetailView::~AdvSecSettingsCertificateDetailView()
{
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateDetailView::setCertificate()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateDetailView::setCertificate(
    const AdvSecSettingsCertificate &cert)
{
    mCertificate = &cert;

    menu()->clearActions();
    if (cert.certType() == AdvSecSettingsCertificate::AuthorityCertificate) {
        HbAction *trustSettingsAction = 0;
        trustSettingsAction = new HbAction(hbTrId("txt_certificate_manager_menu_trust_settings"));
        connect(trustSettingsAction, SIGNAL(triggered()), this, SLOT(showTrustSettings()));
        menu()->addAction(trustSettingsAction);
    }
    if (mModel.isDeletable(cert)) {
        HbAction *deleteAction = new HbAction(hbTrId("txt_common_menu_delete"));
        connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteCertificate()));
        menu()->addAction(deleteAction);
    }

    mDetails.clear();
    mModel.getCertificateDetails(cert, mDetails);

    mDetailsText->clear();
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateDetailView::certificateDetailsCompleted()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateDetailView::certificateDetailsCompleted()
{
    QString label;
    QString details;
    QTextStream stream(&details);

    bool isFirst(true);
    QMapIterator<int, QString> iter(mDetails);
    while (iter.hasNext()) {
        if (!isFirst) {
            stream << endl;
        }
        isFirst = false;

        int key = iter.next().key();
        switch (key) {
        case AdvSecSettingsCertificateModel::Label:
            label = iter.value();
            break;
        case AdvSecSettingsCertificateModel::Issuer:
            stream << hbTrId("txt_certificate_list_issuer") << endl
                << iter.value() << endl;
            break;
        case AdvSecSettingsCertificateModel::Subject:
            stream << hbTrId("txt_certificate_list_subject") << endl
                << iter.value() << endl;
            break;
        case AdvSecSettingsCertificateModel::ValidFrom:
            stream << hbTrId("txt_certificate_list_valid_from") << endl
                << iter.value() << endl;
            break;
        case AdvSecSettingsCertificateModel::ValidTo:
            stream << hbTrId("txt_certificate_list_valid_until") << endl
                << iter.value() << endl;
            break;
        case AdvSecSettingsCertificateModel::Location:
            // TODO: localized UI string needed
            stream << tr("Location:") << endl << iter.value() << endl;
            break;
        case AdvSecSettingsCertificateModel::Format:
            stream << hbTrId("txt_certificate_list_certificate_format") << endl
                << iter.value() << endl;
            break;
        case AdvSecSettingsCertificateModel::KeyUsage:
            // TODO: localized UI string needed
            stream << tr("Key usage:") << endl << iter.value() << endl;
            break;
        case AdvSecSettingsCertificateModel::KeyLocation:
            // TODO: localized UI string needed
            stream << tr("Key location:") << endl << iter.value() << endl;
            break;
        case AdvSecSettingsCertificateModel::Algorithm:
            stream << hbTrId("txt_certificate_list_algorithm") << endl
                << iter.value() << endl;
            break;
        case AdvSecSettingsCertificateModel::SerialNumber:
            stream << hbTrId("txt_certificate_list_serial_number") << endl
                << iter.value() << endl;
            break;
        case AdvSecSettingsCertificateModel::FingerprintSHA1:
            stream << hbTrId("txt_certificate_list_fingerprint_sha1") << endl
                << iter.value() << endl;
            break;
        case AdvSecSettingsCertificateModel::FingerprintMD5:
            stream << hbTrId("txt_certificate_list_fingerprint_md5") << endl
                << iter.value() << endl;
            break;
        case AdvSecSettingsCertificateModel::PublicKey:
            // TODO: localized UI string needed
            stream << tr("Public key:") << endl << iter.value() << endl;
            break;
        case AdvSecSettingsCertificateModel::TrustedSites:
            // TODO: localized UI string needed
            stream << tr("Trusted sites:") << endl << iter.value() << endl;
            break;
        default:
            break;
        }
    }

    mViewLabel->setHeading(label);
    mDetailsText->setPlainText(details);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateDetailView::showTrustSettings()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateDetailView::showTrustSettings()
{
    Q_ASSERT(mCertificate != 0);
    AdvSecSettingsCertTrustSettingsView *trustSettingsView =
        new AdvSecSettingsCertTrustSettingsView(mModel);
    trustSettingsView->setDeleteOnClose(true);
    trustSettingsView->setCertificate(*mCertificate);
    displayView(trustSettingsView);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateDetailView::deleteCertificate()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateDetailView::deleteCertificate()
{
    Q_ASSERT(mCertificate != 0);
    // TODO: localized UI string needed
    QString confirmText(tr("Delete %1?").arg(mCertificate->label()));
    displayQuestionNote(confirmText, this, SLOT(deleteConfirmationAccepted()));
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateDetailView::deleteConfirmationAccepted()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateDetailView::deleteConfirmationAccepted()
{
    Q_ASSERT(mCertificate != 0);
    mModel.deleteCertificate(*mCertificate);
}

