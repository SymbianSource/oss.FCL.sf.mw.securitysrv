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
#include <hbmessagebox.h>
#include <QGraphicsLinearLayout>

const int KNoError = 0;             // KErrNone
const int KParameterError = -6;     // KErrArgument

// TODO: replace with OST tracing
#ifdef _DEBUG
#include <QDebug>
#define TRACE(x)        qDebug() << x
#define TRACE1(x,y)     qDebug() << x << y
#else
#define TRACE(x)
#define TRACE1(x,y)
#endif


// ----------------------------------------------------------------------------
// UntrustedCertificateDialog::UntrustedCertificateDialog()
// ----------------------------------------------------------------------------
//
UntrustedCertificateDialog::UntrustedCertificateDialog(const QVariantMap &parameters) :
    HbDialog(), mLastError(KNoError), mContent(0), mResultMap(),
    mShowEventReceived(false), mOkAction(0)
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
    TRACE("UntrustedCertificateDialog::closeDeviceDialog");
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
    TRACE("UntrustedCertificateDialog::hideEvent");
    HbDialog::hideEvent(event);
    emit deviceDialogClosed();
}

// ----------------------------------------------------------------------------
// UntrustedCertificateDialog::showEvent()
// ----------------------------------------------------------------------------
//
void UntrustedCertificateDialog::showEvent(QShowEvent *event)
{
    TRACE("UntrustedCertificateDialog::showEvent");
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
    TRACE("UntrustedCertificateDialog::constructDialog");
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
        mOkAction = new HbAction(qtTrId("txt_common_button_ok"), this);
        addAction(mOkAction);
        disconnect(mOkAction, SIGNAL(triggered()), this, SLOT(close()));
        connect(mOkAction, SIGNAL(triggered()), this, SLOT(handleAccepted()));

        HbAction *cancelAction = new HbAction(qtTrId("txt_common_button_cancel"), this);
        addAction(cancelAction);
        disconnect(cancelAction, SIGNAL(triggered()), this, SLOT(close()));
        connect(cancelAction, SIGNAL(triggered()), this, SLOT(handleRejected()));
    } else {
        HbAction *closeAction = new HbAction(qtTrId("txt_common_button_close"), this);
        addAction(closeAction);
        disconnect(closeAction, SIGNAL(triggered()), this, SLOT(close()));
        connect(closeAction, SIGNAL(triggered()), this, SLOT(handleRejected()));
    }

    return true;
}

// ----------------------------------------------------------------------------
// UntrustedCertificateDialog::updateFromParameters()
// ----------------------------------------------------------------------------
//
bool UntrustedCertificateDialog::updateFromParameters(const QVariantMap &parameters)
{
    TRACE("UntrustedCertificateDialog::updateFromParameters");
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
    TRACE1("UntrustedCertificateDialog::sendResult", result);
    QVariant resultValue(result);
    mResultMap.insert(KUntrustedCertificateDialogResult, resultValue);
    emit deviceDialogData(mResultMap);
}

// ----------------------------------------------------------------------------
// UntrustedCertificateDialog::confirmPermanentAccept()
// ----------------------------------------------------------------------------
//
void UntrustedCertificateDialog::confirmPermanentAccept()
{
    TRACE("UntrustedCertificateDialog::confirmPermanentAccept");

    HbDialog *dialog = new HbDialog();
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    dialog->setTimeout(HbPopup::NoTimeout);
    dialog->setDismissPolicy(HbDialog::NoDismiss);
    dialog->setModal(true);

    // TODO: localised UI string needed
    QString questionTitle = tr("Certificate access");
    dialog->setHeadingWidget(new HbLabel(questionTitle));

    HbWidget *widget = new HbWidget();
    QGraphicsLinearLayout* layout = new QGraphicsLinearLayout();
    // TODO: localized UI string needed
    QString questionText =
        tr("Connections to '%1' will be made without warnings. Continue?")
        .arg(mContent->serverName());
    HbLabel *textLabel = new HbLabel(questionText);
    textLabel->setTextWrapping(Hb::TextWordWrap);
    layout->addItem(textLabel);
    widget->setLayout(layout);
    dialog->setContentWidget(widget);

    HbAction *yesAction = new HbAction(hbTrId("txt_common_button_yes"));
    dialog->addAction(yesAction);
    disconnect(yesAction, SIGNAL(triggered()), this, SLOT(close()));
    connect(yesAction, SIGNAL(triggered()), this, SLOT(handlePermanentAcceptance()));
    dialog->addAction(new HbAction(hbTrId("txt_common_button_no")));

    dialog->show();
}

// ----------------------------------------------------------------------------
// UntrustedCertificateDialog::handleAccepted()
// ----------------------------------------------------------------------------
//
void UntrustedCertificateDialog::handleAccepted()
{
    TRACE("UntrustedCertificateDialog::handleAccepted");
    if (mContent->isPermanentAcceptChecked()) {
        confirmPermanentAccept();
    } else {
        sendResult(KDialogAccepted);
		close();
    }
}

// ----------------------------------------------------------------------------
// UntrustedCertificateDialog::handleRejected()
// ----------------------------------------------------------------------------
//
void UntrustedCertificateDialog::handleRejected()
{
    TRACE("UntrustedCertificateDialog::handleRejected");
    sendResult(KDialogRejected);
    close();
}

// ----------------------------------------------------------------------------
// UntrustedCertificateDialog::handlePermanentAcceptance()
// ----------------------------------------------------------------------------
//
void UntrustedCertificateDialog::handlePermanentAcceptance()
{
    TRACE("UntrustedCertificateDialog::handlePermanentAcceptance");
    sendResult(KDialogAcceptedPermanently);
    close();
}

