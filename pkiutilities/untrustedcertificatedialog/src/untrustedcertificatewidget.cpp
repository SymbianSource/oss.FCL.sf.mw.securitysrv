/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  Device dialog plugin that shows untrusted certificate
*               dialog for TLS server authentication failure errors.
*
*/

#include "untrustedcertificatewidget.h"
#include "untrustedcertificatedefinitions.h"
#include "untrustedcertificateinfobase.h"
#include <hblabel.h>
#include <hbcheckbox.h>
#include <hbgroupbox.h>
#include <hbtextedit.h>
#include <QGraphicsLinearLayout>

const int KUnknownError = -5;           // KErrNotSupported


// ======== MEMBER FUNCTIONS ========

// ----------------------------------------------------------------------------
// UntrustedCertificateWidget::UntrustedCertificateWidget()
// ----------------------------------------------------------------------------
//
UntrustedCertificateWidget::UntrustedCertificateWidget(QGraphicsItem *parent,
        Qt::WindowFlags flags) : HbWidget(parent, flags),
        mMainLayout(0), mProblemDescription(0), mAcceptPermanently(0),
        mCertificateDetailsGroupBox(0), mCertificateDetailsText(0),
        mCertificateInfo(0), mServerName(), mValidationError(0),
        mIsSavingServerNamePossible(true)
{
}

// ----------------------------------------------------------------------------
// UntrustedCertificateWidget::~UntrustedCertificateWidget()
// ----------------------------------------------------------------------------
//
UntrustedCertificateWidget::~UntrustedCertificateWidget()
{
    delete mCertificateInfo;
}

// ----------------------------------------------------------------------------
// UntrustedCertificateWidget::constructFromParameters()
// ----------------------------------------------------------------------------
//
void UntrustedCertificateWidget::constructFromParameters(
    const QVariantMap &parameters)
{
    processParameters(parameters);

    Q_ASSERT(mMainLayout == 0);
    mMainLayout = new QGraphicsLinearLayout(Qt::Vertical);

    Q_ASSERT(mProblemDescription == 0);
    QString text = descriptionText().arg(mServerName);
    mProblemDescription = new HbLabel(text);
    mProblemDescription->setTextWrapping(Hb::TextWordWrap);
    mMainLayout->addItem(mProblemDescription);

    if (isPermanentAcceptAllowed()) {
        Q_ASSERT(mAcceptPermanently == 0);
        // TODO: localised UI string needed
        mAcceptPermanently = new HbCheckBox(tr("Accept permanently"));
        mMainLayout->addItem(mAcceptPermanently);
    }

    Q_ASSERT(mCertificateDetailsGroupBox == 0);
    mCertificateDetailsGroupBox = new HbGroupBox;
    // TODO: localized UI string needed
    mCertificateDetailsGroupBox->setHeading(tr("Details"));

    Q_ASSERT(mCertificateDetailsText == 0);
    mCertificateDetailsText = new HbTextEdit;
    QString certificateDetails = mCertificateInfo->certificateDetails(mServerName);
    mCertificateDetailsText->setPlainText(certificateDetails);
    mCertificateDetailsText->setReadOnly(true);

    mCertificateDetailsGroupBox->setContentWidget(mCertificateDetailsText);
    mCertificateDetailsGroupBox->setCollapsed(true);
    mMainLayout->addItem(mCertificateDetailsGroupBox);

    setLayout(mMainLayout);
    }

// ----------------------------------------------------------------------------
// UntrustedCertificateWidget::updateFromParameters()
// ----------------------------------------------------------------------------
//
void UntrustedCertificateWidget::updateFromParameters(
    const QVariantMap &parameters)
{
    processParameters(parameters);
}

// ----------------------------------------------------------------------------
// UntrustedCertificateWidget::isPermanentAcceptAllowed()
// ----------------------------------------------------------------------------
//
bool UntrustedCertificateWidget::isPermanentAcceptAllowed() const
{
    return (mCertificateInfo->isDateValid() && (mServerName.length() > 0) &&
        mIsSavingServerNamePossible);
}

// ----------------------------------------------------------------------------
// UntrustedCertificateWidget::isPermanentAcceptChecked()
// ----------------------------------------------------------------------------
//
bool UntrustedCertificateWidget::isPermanentAcceptChecked() const
{
    if (mAcceptPermanently) {
        return (mAcceptPermanently->checkState() == Qt::Checked);
    }
    return false;
}

// ----------------------------------------------------------------------------
// UntrustedCertificateWidget::serverName()
// ----------------------------------------------------------------------------
//
QString UntrustedCertificateWidget::serverName() const
{
    return mServerName;
}

// ----------------------------------------------------------------------------
// UntrustedCertificateWidget::processParameters()
// ----------------------------------------------------------------------------
//
void UntrustedCertificateWidget::processParameters(const QVariantMap &parameters)
{
    mServerName = parameters.value(KUntrustedCertServerName).toString();

    bool ok = false;
    mValidationError = KUnknownError;
    int value = parameters.value(KUntrustedCertValidationError).toInt(&ok);
    if (ok) {
        mValidationError = value;
    }

    QByteArray encodedCert = parameters.value(KUntrustedCertEncodedCertificate).toByteArray();
    processEncodedCertificate(encodedCert);

    if (parameters.contains(KUntrustedCertTrustedSiteStoreFail)) {
        mIsSavingServerNamePossible = false;
    }
}

