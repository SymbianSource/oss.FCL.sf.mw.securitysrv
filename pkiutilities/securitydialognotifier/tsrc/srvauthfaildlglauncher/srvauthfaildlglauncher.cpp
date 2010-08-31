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

#include "srvauthfaildlglauncher.h"
#include <HbMainWindow>
#include <HbView>
#include <HbLabel>
#include <HbComboBox>
#include <HbTextEdit>
#include <HbCheckBox>
#include <HbPushButton>
#include <HbDeviceDialog>
#include <HbMessageBox>
#include <QDir>
#include <QBuffer>
#include <QGraphicsLinearLayout>


#ifdef Q_OS_SYMBIAN
#include <securitydefs.h>       // TValidationError
#include <secdlgimpldefs.h>     // TServerAuthenticationFailureInput, KUidSecurityDialogNotifier

HBufC8* GetInputBufferL( const QString& server, const QByteArray& certificate, int reason )
    {
    const TPtrC16 serverNameUnicode(reinterpret_cast<const TText*>(server.constData()),
        server.length());
    const TPtrC8 encodedCert(reinterpret_cast<const TText8*>(certificate.constData()),
        certificate.length());

    const TInt KServerNameMaxLength = 512;
    TBuf8<KServerNameMaxLength> serverName;
    serverName.Copy(serverNameUnicode);

    TServerAuthenticationFailureInput serverAuthenticationInput;
    serverAuthenticationInput.iOperation = EServerAuthenticationFailure;
    serverAuthenticationInput.iFailureReason = static_cast<TValidationError>(reason);
    serverAuthenticationInput.iServerNameLength = serverName.Length();
    serverAuthenticationInput.iEncodedCertLength = encodedCert.Length();

    TServerAuthenticationFailureInputBuf inputBuf( serverAuthenticationInput );

    TInt bufferSize = sizeof( inputBuf ) + serverName.Length() + encodedCert.Length();
    HBufC8* packedBuffer = HBufC8::NewL( bufferSize );
    TPtr8 packedBufferPtr( packedBuffer->Des() );

    packedBufferPtr.Append( inputBuf );
    packedBufferPtr.Append( serverName );
    packedBufferPtr.Append( encodedCert );

    return packedBuffer;
    }

bool ShowDialogL( const QString& server, const QByteArray& certificate, int reason, bool cancel )
    {
    RNotifier notifier;
    User::LeaveIfError( notifier.Connect() );
    CleanupClosePushL( notifier );

    HBufC8* buffer = GetInputBufferL( server, certificate, reason );
    CleanupStack::PushL( buffer );

    TRequestStatus status;
    TPckgBuf<TServerAuthenticationFailureDialogResult> resultPckg;
    notifier.StartNotifierAndGetResponse( status, KUidSecurityDialogNotifier,
        *buffer, resultPckg );
    if( cancel )
        {
        const TInt KTenSecDelay = 10000000;
        User::After( KTenSecDelay );
        notifier.CancelNotifier( KUidSecurityDialogNotifier );
        }
    User::WaitForRequest( status );
    User::LeaveIfError( status.Int() );

    CleanupStack::PopAndDestroy( buffer );
    CleanupStack::PopAndDestroy( &notifier );
    return( resultPckg() == EContinue );
    }

#endif  // Q_OS_SYMBIAN


const QString KTestCertDir = "c:/data/testCerts";

SrvAuthFailDlgLauncher::SrvAuthFailDlgLauncher(int& argc, char* argv[])
    : HbApplication(argc, argv), mMainWindow(0), mMainView(0)
{
    mMainWindow = new HbMainWindow();
    mMainView = new HbView();
    mMainView->setTitle(tr("SrvAuthFailDlgLauncher"));

    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical);

    layout->addItem(new HbLabel(tr("Server host name:")));
    mHostName = new HbTextEdit(tr("some.host.com"));
    layout->addItem(mHostName);
    layout->addStretch();

    layout->addItem(new HbLabel(tr("Certificate file:")));
    mFilesList = new HbComboBox;
    mFilesList->setEditable(false);
    QDir dir(KTestCertDir);
    if (dir.exists()) {
        QFileInfoList list = dir.entryInfoList(QDir::Files);
        QListIterator<QFileInfo> iter(list);
        while (iter.hasNext()) {
            const QFileInfo &info(iter.next());
            mFilesList->addItem(info.fileName());
        }
    }
    layout->addItem(mFilesList);
    layout->addStretch();

    layout->addItem(new HbLabel(tr("Validation result:")));
    mValidationResult = new HbComboBox;
    QStringList validationResults;
    validationResults
	    << "EValidatedOK"
	    << "EChainHasNoRoot"
	    << "ESignatureInvalid"
	    << "EDateOutOfRange"
	    << "ENameIsExcluded"
	    << "ENameNotPermitted"
	    << "ENotCACert"
	    << "ECertificateRevoked"
	    << "EUnrecognizedCriticalExtension"
	    << "ENoBasicConstraintInCACert"
	    << "ENoAcceptablePolicy"
	    << "EPathTooLong"
	    << "ENegativePathLengthSpecified"
	    << "ENamesDontChain"
	    << "ERequiredPolicyNotFound"
	    << "EBadKeyUsage"
	    << "ERootCertNotSelfSigned"
	    << "ECriticalExtendedKeyUsage"
	    << "ECriticalCertPoliciesWithQualifiers"
	    << "ECriticalPolicyMapping"
	    << "ECriticalDeviceId"
	    << "ECriticalSid"
	    << "ECriticalVid"
	    << "ECriticalCapabilities";
    mValidationResult->addItems(validationResults);
    mValidationResult->setCurrentIndex(1);      // EChainHasNoRoot
    layout->addItem(mValidationResult);
    layout->addStretch();

    mCancelShortly = new HbCheckBox("Cancel after 10 seconds");
    layout->addItem(mCancelShortly);

    HbPushButton *button = 0;
    button = new HbPushButton("Launch dialog");
    connect(button, SIGNAL(clicked()), this, SLOT(activateDialog()));
    layout->addItem(button);

    button = new HbPushButton("Exit");
    connect(button, SIGNAL(clicked()), qApp, SLOT(quit()));
    layout->addItem(button);

    mMainView->setLayout(layout);
    mMainWindow->addView(mMainView);
    mMainWindow->show();
}

SrvAuthFailDlgLauncher::~SrvAuthFailDlgLauncher()
{
    delete mMainView;
    delete mMainWindow;
}

void SrvAuthFailDlgLauncher::activateDialog()
{
    QString server = mHostName->toPlainText();

    QString fileName = mFilesList->currentText();
    QFile file;
    QDir::setCurrent(KTestCertDir);
    file.setFileName(fileName);
    file.open(QIODevice::ReadOnly);
    QByteArray fileContent = file.readAll();
    file.close();

    int reason = mValidationResult->currentIndex();
    bool cancel = mCancelShortly->isChecked();

    int error = 0;
    bool accepted = false;
#ifdef Q_OS_SYMBIAN
    TRAP( error, accepted = ShowDialogL(server, fileContent, reason, cancel) );
#endif // Q_OS_SYMBIAN
    if (!error) {
        if (accepted) {
            HbMessageBox::information(QString("Accepted"));
        } else {
            HbMessageBox::information(QString("Declined"));
        }
    } else {
        if (error == KErrCancel) {
            HbMessageBox::information(QString("Cancelled"));
        } else {
            HbMessageBox::information(QString("Failed, error %1").arg(error));
        }
    }
}

