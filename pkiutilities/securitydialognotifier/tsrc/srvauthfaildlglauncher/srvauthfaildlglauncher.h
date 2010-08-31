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

#ifndef SRVAUTHFAILDLGLAUNCHER_H
#define SRVAUTHFAILDLGLAUNCHER_H

#include <hbapplication.h>

class HbMainWindow;
class HbView;
class HbComboBox;
class HbTextEdit;
class HbCheckBox;


class SrvAuthFailDlgLauncher : public HbApplication
{
    Q_OBJECT

public:     // constructor and destructor
    SrvAuthFailDlgLauncher(int& argc, char* argv[]);
    ~SrvAuthFailDlgLauncher();

private slots:  // new functions
    void activateDialog();

private:    // data
    HbMainWindow *mMainWindow;
    HbView       *mMainView;
    HbComboBox   *mFilesList;
    HbTextEdit   *mHostName;
    HbComboBox   *mValidationResult;
    HbCheckBox   *mCancelShortly;
};

#endif  // SRVAUTHFAILDLGLAUNCHER_H

