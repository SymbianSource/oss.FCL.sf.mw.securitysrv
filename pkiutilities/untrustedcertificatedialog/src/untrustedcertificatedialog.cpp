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

#include "untrustedcertificatedialog.h"
#include "untrustedcertificatedefinitions.h"
#include "untrustedcertificatewidget.h"
#include <hblabel.h>
#include <hbaction.h>

const int KNoError = 0;             // KErrNone
const int KParameterError = -6;     // KErrArgument


// ----------------------------------------------------------------------------
// UntrustedCertificateDialog::UntrustedCertificateDialog()
// ----------------------------------------------------------------------------
//
UntrustedCertificateDialog::UntrustedCertificateDialog(const QVariantMap &parameters) : HbDialog(),
    mLastError(KNoError), mShowEventReceived(false), mContent(0)
{
    constructDialog(parameters);
}

// ----------------------------------------------------------------------------
// UntrustedCertificateDialog::~UntrustedCertificateDialog()
// ----------------------------------------------------------------------------
//
UntrustedCertificateDialog::~UntrustedCertificateDialog()
{
}

// ----------------------------------------------------------------------------
// UntrustedCertificateDialog::setDeviceDialogParameters()
// ----------------------------------------------------------------------------
//
bool UntrustedCertificateDialog::setDeviceDialogParameters(const QVariantMap &parameters)
{
   return updateFromParameters(parameters);
}

// ----------------------------------------------------------------------------
// UntrustedCertificateDialog::deviceDialogError()
// ----------------------------------------------------------------------------
//
int UntrustedCertificateDialog::deviceDialogError() const
{
    return mLastError;
}

// ----------------------------------------------------------------------------
// UntrustedCertificateDialog::closeDeviceDialog()
// ----------------------------------------------------------------------------
//
void UntrustedCertificateDialog::closeDeviceDialog(bool byClient)
{
    Q_UNUSED(byClient);
    close();

    // If show event has been received, close is signalled from hide event.
    // If not, hide event does not come and close is signalled from here.
    if (!mShowEventReceived) {
        emit deviceDialogClosed();
    }
}

// ----------------------------------------------------------------------------
// UntrustedCertificateDialog::deviceDialogWidget()
// ----------------------------------------------------------------------------
//
HbDialog *UntrustedCertificateDialog::deviceDialogWidget() const
{
    return const_cast<UntrustedCertificateDialog*>(this);
}

// ----------------------------------------------------------------------------
// UntrustedCertificateDialog::hideEvent()
// ----------------------------------------------------------------------------
//
void UntrustedCertificateDialog::hideEvent(QHideEvent *event)
{
    HbDialog::hideEvent(event);
    emit deviceDialogClosed();
}

// ----------------------------------------------------------------------------
// UntrustedCertificateDialog::showEvent()
// ----------------------------------------------------------------------------
//
void UntrustedCertificateDialog::showEvent(QShowEvent *event)
{
    HbDialog::showEvent(event);
    mShowEventReceived = true;
}

// ----------------------------------------------------------------------------
// UntrustedCertificateDialog::isParametersValid()
// ----------------------------------------------------------------------------
//
bool UntrustedCertificateDialog::isParametersValid(const QVariantMap &parameters)
{
    if (parameters.contains(KUntrustedCertEncodedCertificate) &&
        parameters.contains(KUntrustedCertServerName) &&
        parameters.contains(KUntrustedCertValidationError)) {
        return true;
        }
    mLastError = KParameterError;
    return false;
}

// ----------------------------------------------------------------------------
// UntrustedCertificateDialog::constructDialog()
// ----------------------------------------------------------------------------
//
bool UntrustedCertificateDialog::constructDialog(const QVariantMap &parameters)
{
    if (!isParametersValid(parameters)) {
        return false;
    }

    setTimeout(HbPopup::NoTimeout);
    setDismissPolicy(HbPopup::NoDismiss);
    setModal(true);

    //: Title text in untrusted certificate dialog. User is opening secure
    //: connection to site or service, which authenticity cannot be proved,
    //: or there are other problems in the site certificate. User needs to
    //: decide if she/he accepts the risk and opens the secure connection
    //: anyway, or if the connection is rejected.
    //TODO: localised UI string
    //QString titleText = hbTrId("");
    QString titleText = tr("Untrusted certificate");
    setHeadingWidget(new HbLabel(titleText, this));

    Q_ASSERT(mContent == 0);
    mContent = new UntrustedCertificateWidget(this);
    mContent->constructFromParameters(parameters);
    setContentWidget(mContent);

    if (mContent->isCertificateValid()) {
        HbAction *okAction = new HbAction(qtTrId("txt_common_button_ok"), this);
        connect(okAction, SIGNAL(triggered()), this, SLOT(handleAccepted()));
        addAction(okAction);

        HbAction *cancelAction = new HbAction(qtTrId("txt_common_button_cancel"), this);
        connect(cancelAction, SIGNAL(triggered()), this, SLOT(handleRejected()));
        addAction(cancelAction);
    } else {
        HbAction *closeAction = new HbAction(qtTrId("txt_common_button_close"), this);
        connect(closeAction, SIGNAL(triggered()), this, SLOT(handleRejected()));
        addAction(closeAction);
    }

    return true;
}

// ----------------------------------------------------------------------------
// UntrustedCertificateDialog::updateFromParameters()
// ----------------------------------------------------------------------------
//
bool UntrustedCertificateDialog::updateFromParameters(const QVariantMap &parameters)
{
    if (!isParametersValid(parameters)) {
        return false;
    }

    Q_ASSERT(mContent != 0);
    mContent->updateFromParameters(parameters);
    return true;
}

// ----------------------------------------------------------------------------
// UntrustedCertificateDialog::sendResult()
// ----------------------------------------------------------------------------
//
void UntrustedCertificateDialog::sendResult(int result)
{
    QVariant resultValue(result);
    mResultMap.insert(KUntrustedCertificateDialogResult, resultValue);
    emit deviceDialogData(mResultMap);
}

// ----------------------------------------------------------------------------
// UntrustedCertificateDialog::handleAccepted()
// ----------------------------------------------------------------------------
//
void UntrustedCertificateDialog::handleAccepted()
{
    if (mContent->isPermanentAcceptChecked()) {
        sendResult(KDialogAcceptedPermanently);
    } else {
        sendResult(KDialogAccepted);
    }
}

// ----------------------------------------------------------------------------
// UntrustedCertificateDialog::handleRejected()
// ----------------------------------------------------------------------------
//
void UntrustedCertificateDialog::handleRejected()
{
    sendResult(KDialogRejected);
}

