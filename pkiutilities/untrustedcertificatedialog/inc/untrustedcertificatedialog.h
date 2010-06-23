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

#ifndef UNTRUSTEDCERTIFICATEDIALOG_H
#define UNTRUSTEDCERTIFICATEDIALOG_H

#include <hbdialog.h>
#include <hbdevicedialoginterface.h>

class UntrustedCertificateWidget;


/**
 * Untrusted certificate dialog.
 * This dialog is displayed when a secure socket (CSecureSocket) is opened
 * using EDialogModeAttended mode, and the server sends untrusted certificate.
 * Untrusted certificate dialog shows the certificate details and prompts if
 * the secure connection is allowed to proceed.
 */
class UntrustedCertificateDialog : public HbDialog, public HbDeviceDialogInterface
{
    Q_OBJECT

public:     // constructor and destructor
    UntrustedCertificateDialog(const QVariantMap &parameters);
    virtual ~UntrustedCertificateDialog();

public:     // from HbDeviceDialogInterface
    bool setDeviceDialogParameters(const QVariantMap &parameters);
    int deviceDialogError() const;
    void closeDeviceDialog(bool byClient);
    HbDialog *deviceDialogWidget() const;

signals:    // required by device dialog framework
    void deviceDialogClosed();
    void deviceDialogData(const QVariantMap &data);

protected:  // from HbPopup (via HbDialog)
    void hideEvent(QHideEvent *event);
    void showEvent(QShowEvent *event);

private:    // new functions
    bool isParametersValid(const QVariantMap &parameters);
    bool constructDialog(const QVariantMap &parameters);
    bool updateFromParameters(const QVariantMap &parameters);
    void sendResult(int result);
    void confirmPermanentAccept();

private slots:  // new slots
    void handleAccepted();
    void handleRejected();
    void handlePermanentAcceptance();

private:    // new functions
    Q_DISABLE_COPY(UntrustedCertificateDialog)

private:    // data
    int mLastError;
    UntrustedCertificateWidget *mContent;
    QVariantMap mResultMap;
    bool mShowEventReceived;
    HbAction *mOkAction;
};

#endif // UNTRUSTEDCERTIFICATEDIALOG_H

