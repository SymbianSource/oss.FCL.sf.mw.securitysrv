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
* Description:  Secure connections test application
*
*/

#include "tlsconnectionapp.h"
#include "tlsconnection.h"
#include <hbmainwindow.h>
#include <hbview.h>
#include <QDebug>
#include <QGraphicsLinearLayout>
#include <hbcombobox.h>
#include <hblineedit.h>
#include <hblabel.h>
#include <hbpushbutton.h>
#include <hbmessagebox.h>


TlsConnectionApplication::TlsConnectionApplication(int& aArgc, char* aArgv[])
    : HbApplication(aArgc, aArgv), mMainWindow(0), mMainView(0), mHost(0),
      mPort(0), mOpeningDelay(0), mStatusLabel(0), mConnection(0)
{
    qDebug() << "TLStest TlsConnectionApplication begin";

    mMainWindow = new HbMainWindow();
    mMainView = new HbView();
    mMainView->setTitle(tr("TLS Test"));

    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical);

    mHost = new HbComboBox;
    QStringList defaultHosts;
    defaultHosts << "www.nokia.com"
        << "www.f-secure.com"
        << "www.google.fi"
        << "www.tut.fi";
    mHost->addItems(defaultHosts);
    connect(mHost, SIGNAL(editingFinished()), this, SLOT(addHostName()));
    mHost->setEditable(true);
    layout->addItem(mHost);

    mPort = new HbLineEdit(tr("Port?"));
    mPort->setText(tr("443"));
    layout->addItem(mPort);

    mOpeningDelay = new HbLineEdit(tr("Opening delay?"));
    mOpeningDelay->setText(tr("0"));
    layout->addItem(mOpeningDelay);

    mStatusLabel = new HbLabel(tr("Inactive"));
    layout->addItem(mStatusLabel);

    HbPushButton *activateButton = new HbPushButton(tr("Run Test"));
    connect(activateButton, SIGNAL(clicked()), this, SLOT(runTestPressed()));
    layout->addItem(activateButton);

    HbPushButton *closeButton = new HbPushButton("Close");
    connect(closeButton, SIGNAL(clicked()), qApp, SLOT(quit()));
    layout->addItem(closeButton);

    mMainView->setLayout(layout);
    mMainWindow->addView(mMainView);
    mMainWindow->show();

    qDebug() << "TLStest TlsConnectionApplication end";
}

TlsConnectionApplication::~TlsConnectionApplication()
{
    qDebug() << "TLStest ~TlsConnectionApplication begin";

    delete mConnection;
    delete mMainView;
    delete mMainWindow;

    qDebug() << "TLStest ~TlsConnectionApplication end";
}

void TlsConnectionApplication::HandleNetworkEvent( TTlsConnectionState aEvent, TInt aError )
{
    switch( aEvent ) {
	case ENotInitialized:
		showProgress( tr("NotInitialized"), aError );
		break;
	case EDisconnected:
		showProgress( tr("Disconnected"), aError );
		break;
	case EIdle:
		showProgress( tr("Idle"), aError );
		break;
	case EConnectingNetwork:
		showProgress( tr("ConnectingNetwork"), aError );
		break;
	case EOpeningDelay:
		showProgress( tr("OpeningDelay"), aError );
		break;
	case EResolvingHostName:
		showProgress( tr("ResolvingHostName"), aError );
		break;
	case EConnectingServer:
		showProgress( tr("ConnectingServer"), aError );
		break;
	case EHandshaking:
		showProgress( tr("Handshaking"), aError );
		break;
	case EConnecting:
		showProgress( tr("Connecting"), aError );
		break;
	case ESending:
		showProgress( tr("Sending"), aError );
		break;
	case EReading:
		showProgress( tr("Reading"), aError );
		break;
	case EAllDone:
		showProgress( tr("AllDone"), aError );
		break;
	default:
		showProgress( tr("unknown"), aError );
		break;
	}
}

void TlsConnectionApplication::HandleTransferData( const TDesC8& /*aData*/, TInt aLength )
{
    qDebug() << "TLStest HandleTransferData, length:" << aLength;
}

void TlsConnectionApplication::runTestPressed()
{
    qDebug() << "TLStest runTestPressed begin";

    if (mConnection) {
        delete mConnection;
        mConnection = NULL;
    }

    TPtrC host( reinterpret_cast<const TText*>( mHost->currentText().constData() ));
    TInt port = getInt(mPort->text(), 443);
    TInt delay = getInt(mOpeningDelay->text(), 0);
    qDebug() << "TLStest runTestPressed host:" << mHost->currentText() << "port" << port << "delay" << delay;
    TRAPD( err, mConnection = CTlsConnection::NewL( *this ) );
    qDebug() << "TLStest runTestPressed err:" << err;
    if( !err ) {
        qDebug() << "TLStest runTestPressed CTlsConnection created, connecting";
        TRAP( err, mConnection->ConnectL( host, port, delay ) );
        qDebug() << "TLStest runTestPressed err:" << err;
    }

    qDebug() << "TLStest runTestPressed end";
}

void TlsConnectionApplication::addHostName()
{
    QString newHostName = mHost->currentText();
    mHost->addItem(newHostName);
}

void TlsConnectionApplication::showProgress(const QString& aStep, int aError)
{
    qDebug() << "TLStest showProgress step" << aStep << "error" << aError;
    if (mStatusLabel) {
        if (!aError) {
            mStatusLabel->setPlainText(aStep);
        } else {
            QString msg(aStep);
            msg.append(tr(" error %1").arg(aError));
            mStatusLabel->setPlainText(msg);
        }
    }
}

TInt TlsConnectionApplication::getInt(const QString& aString, int defaultValue)
{
    qDebug() << "TLStest getInt" << aString << "defaultValue" << defaultValue;

    bool ok = false;
    TInt value = aString.toInt(&ok, 10);
    if (!ok) {
        value = defaultValue;
    }

    qDebug() << "TLStest getInt returns " << value;
    return value;
}
