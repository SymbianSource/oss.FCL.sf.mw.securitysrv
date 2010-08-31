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

#ifndef UNTRUSTEDCERTDIALOGLAUNCHER_H
#define UNTRUSTEDCERTDIALOGLAUNCHER_H

#include <hbapplication.h>

class HbMainWindow;
class HbView;
class HbComboBox;
class HbTextEdit;


class UntrustedCertDialogLauncher : public HbApplication
{
    Q_OBJECT

public:     // constructor and destructor
    UntrustedCertDialogLauncher(int& argc, char* argv[]);
    ~UntrustedCertDialogLauncher();

private:    // new functions
    void activateDialog(const QVariantMap &params);
    QVariantMap readParams();

private slots:  // new functions
    void activateDialogDateValid();
    void activateDialogOutOfDate();
    void activateDialogUntrusted();
    void activateDialogInvalid();
    void dataReceived(QVariantMap data);
    void deviceDialogClosed();

private:    // data
    HbMainWindow *mMainWindow;
    HbView       *mMainView;
    HbComboBox   *mFilesList;
    HbTextEdit   *mHostName;
};

#endif  // UNTRUSTEDCERTDIALOGLAUNCHER_H

