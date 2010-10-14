/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0""
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  Advanced security settings main view
*
*/

#ifndef ADVSECSETTINGSMAINVIEW_H
#define ADVSECSETTINGSMAINVIEW_H

#include "advsecsettingsviewbase.h"

class AdvSecSettingsSecurityModuleModel;
class AdvSecSettingsCertificateListView;
class AdvSecSettingsSecurityModuleView;
class HbWidget;
class HbGroupBox;
class HbListWidget;


/**
 * Main view for the advanced security settings. The main advanced
 * security settings view contains certificates, protected content,
 * and security module group boxes.
 */
class AdvSecSettingsMainView : public AdvSecSettingsViewBase
{
    Q_OBJECT

public:     // constructor and destructor
    explicit AdvSecSettingsMainView(QGraphicsItem *parent = 0);
    virtual ~AdvSecSettingsMainView();

public slots:
    void displayCertListView(const QModelIndex& modelIndex);
    void displaySecurityModuleView(const QModelIndex& modelIndex);
    void securityModuleInitialized();

private:    // new functions
    HbWidget *createCertificatesTopLevel();
    HbWidget *createProtectedContentTopLevel();
    HbWidget *createSecurityModuleTopLevel();
    bool isSecurityModulesAvailable();

private:    // data
    AdvSecSettingsSecurityModuleModel *mSecModModel;
    AdvSecSettingsCertificateListView *mCertListView;
    AdvSecSettingsSecurityModuleView *mSecModView;
    HbGroupBox *mSecModGroupBox;
    HbListWidget *mSecModList;
    HbMenu *mContextMenu;
};

#endif // ADVSECSETTINGSMAINVIEW_H

