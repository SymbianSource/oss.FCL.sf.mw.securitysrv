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
* Description:  Test application for untrusted certificate dialog.
*
*/

#include "dialoglauncher.h"
#include "../../inc/untrustedcertificatedefinitions.h"
#include <hbmainwindow.h>
#include <hbview.h>
#include <hblabel.h>
#include <hbcombobox.h>
#include <hbtextedit.h>
#include <hbpushbutton.h>
#include <hbdevicedialog.h>
#include <hbmessagebox.h>
#include <QDir>
#include <QBuffer>
#include <QGraphicsLinearLayout>

#ifdef Q_OS_SYMBIAN
#include <securitydefs.h>       // EValidatedOK, ESignatureInvalid, EDateOutOfRange
#endif

const QString RootDir = "c:/";


UntrustedCertDialogLauncher::UntrustedCertDialogLauncher(int& argc, char* argv[])
    : HbApplication(argc, argv), mMainWindow(0), mMainView(0)
{
    mMainWindow = new HbMainWindow();
    mMainView = new HbView();
    mMainView->setTitle(tr("UntrustedCertDlg"));

    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical);

    HbLabel *certFile = new HbLabel(tr("Certificate file:"));
    layout->addItem(certFile);

    mFilesList = new HbComboBox;
    mFilesList->setEditable(false);
    QDir dir(RootDir);
    if (dir.exists()) {
        QFileInfoList list = dir.entryInfoList(QDir::Files);
        QListIterator<QFileInfo> iter(list);
        while (iter.hasNext()) {
            const QFileInfo &info(iter.next());
            mFilesList->addItem(info.fileName());
        }
    }
    layout->addItem(mFilesList);

    mHostName = new HbTextEdit(tr("some.host.name.com"));
    layout->addItem(mHostName);

    HbPushButton *button = 0;
    button = new HbPushButton("Open (valid)");
    connect(button, SIGNAL(clicked()), this, SLOT(activateDialogDateValid()));
    layout->addItem(button);

    button = new HbPushButton("Open (out-of-date)");
    connect(button, SIGNAL(clicked()), this, SLOT(activateDialogOutOfDate()));
    layout->addItem(button);

    button = new HbPushButton("Open (untrusted)");
    connect(button, SIGNAL(clicked()), this, SLOT(activateDialogUntrusted()));
    layout->addItem(button);

    button = new HbPushButton("Open (invalid)");
    connect(button, SIGNAL(clicked()), this, SLOT(activateDialogInvalid()));
    layout->addItem(button);

    button = new HbPushButton("Exit");
    connect(button, SIGNAL(clicked()), qApp, SLOT(quit()));
    layout->addItem(button);

    mMainView->setLayout(layout);
    mMainWindow->addView(mMainView);
    mMainWindow->show();
}

UntrustedCertDialogLauncher::~UntrustedCertDialogLauncher()
{
    delete mMainView;
    delete mMainWindow;
}

void UntrustedCertDialogLauncher::activateDialog(const QVariantMap &params)
{
    HbDeviceDialog *deviceDialog = new HbDeviceDialog(HbDeviceDialog::NoFlags, this);
    connect(deviceDialog, SIGNAL(dataReceived(QVariantMap)),
        this, SLOT(dataReceived(QVariantMap)));
    connect(deviceDialog, SIGNAL(deviceDialogClosed()),
        this, SLOT(deviceDialogClosed()));

    if (!deviceDialog->show(KUntrustedCertificateDialog, params)) {
        HbMessageBox::warning(tr("Cannot open dialog"));
    }
}

QVariantMap UntrustedCertDialogLauncher::readParams()
{
    QVariantMap params;

    QString fileName = mFilesList->currentText();
    QFile file;
    QDir::setCurrent(RootDir);
    file.setFileName(fileName);
    file.open(QIODevice::ReadOnly);
    QByteArray fileContent = file.readAll();
    file.close();

    params.insert(KUntrustedCertEncodedCertificate, fileContent);

    QString serverName = mHostName->toPlainText();
    params.insert(KUntrustedCertServerName, serverName);

    return params;
}

void UntrustedCertDialogLauncher::activateDialogDateValid()
{
    QVariantMap params = readParams();

    int validationError = EValidatedOK;
    params.insert(KUntrustedCertValidationError, validationError);

    activateDialog(params);
}

void UntrustedCertDialogLauncher::activateDialogOutOfDate()
{
    QVariantMap params = readParams();

    int validationError = EDateOutOfRange;
    params.insert(KUntrustedCertValidationError, validationError);

    activateDialog(params);
}

void UntrustedCertDialogLauncher::activateDialogUntrusted()
{
    QVariantMap params = readParams();

    int validationError = EChainHasNoRoot;
    params.insert(KUntrustedCertValidationError, validationError);

    activateDialog(params);
}

void UntrustedCertDialogLauncher::activateDialogInvalid()
{
    QVariantMap params = readParams();

    int validationError = ECertificateRevoked;
    params.insert(KUntrustedCertValidationError, validationError);

    activateDialog(params);
}

void UntrustedCertDialogLauncher::dataReceived(QVariantMap data)
{
    QString values;
    foreach(const QString key, data.keys()) {
        values.append(key);
        values.append(tr(" = "));
        values.append(data.value(key).toString());
        values.append(tr("\n"));
    }

    HbMessageBox::information(tr("dataReceived %1").arg(values));
}

void UntrustedCertDialogLauncher::deviceDialogClosed()
{
    HbMessageBox::information(tr("deviceDialogClosed"));
}

