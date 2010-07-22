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

#ifndef TLSCONNECTIONAPP_H
#define TLSCONNECTIONAPP_H

#include <hbapplication.h>
#include "tlsconnectionobserver.h"

class HbMainWindow;
class HbView;
class HbComboBox;
class HbLineEdit;
class HbLabel;
class CTlsConnection;


class TlsConnectionApplication : public HbApplication, public MTlsConnectionObserver
{
    Q_OBJECT

public:     // constructor and destructor
    TlsConnectionApplication(int& aArgc, char* aArgv[]);
    virtual ~TlsConnectionApplication();

public:		// from MTlsConnectionObserver
    void HandleNetworkEvent(TTlsConnectionState aEvent, TInt aError);
    void HandleTransferData(const TDesC8& aData, TInt aLength);

private slots:
    void runTestPressed();
    void addHostName();

private:    // new functions
    void showProgress(const QString& aStep, int aError);
    TInt getInt(const QString& aString, int defaultValue);

private:    // data
    HbMainWindow *mMainWindow;
    HbView       *mMainView;
    HbComboBox   *mHost;
    HbLineEdit   *mPort;
    HbLineEdit   *mOpeningDelay;
    HbLabel      *mStatusLabel;
    CTlsConnection *mConnection;
};

#endif  // TLSCONNECTIONAPP_H

