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
* Description:  Security module view in advanced security settings
*
*/

#ifndef ADVSECSETTINGSSECURITYMODULEVIEW_H
#define ADVSECSETTINGSSECURITYMODULEVIEW_H

#include "advsecsettingsviewbase.h"

class AdvSecSettingsSecurityModuleModel;
class HbGroupBox;
class HbDataFormModel;
class HbDataFormModelItem;


/**
 * Shows info about one security module (such as WIM or personal
 * key store). Provides also functionality to change access codes
 * (like module PIN code, and signing PIN code).
 */
class AdvSecSettingsSecurityModuleView : public AdvSecSettingsViewBase
{
    Q_OBJECT

public:     // constructor and destructor
    explicit AdvSecSettingsSecurityModuleView(AdvSecSettingsSecurityModuleModel &model,
        QGraphicsItem *parent = 0);
    virtual ~AdvSecSettingsSecurityModuleView();

public:    // new functions
    void setSecurityModule(const QString &moduleTitle, int modelIndex);

private slots:
    void updateModuleStatus();
    void moduleStatusChanged(int status);
    void itemActivated(const QModelIndex &itemIndex);

private:    // data
    AdvSecSettingsSecurityModuleModel &mModel;
    HbGroupBox *mViewLabel;
    HbDataFormModel *mDataFormModel;
    HbDataFormModelItem *mModulePin;
    HbDataFormModelItem *mModulePinRequested;
    HbDataFormModelItem *mModuleStatus;
    HbDataFormModelItem *mSigningPin;
    int mModelIndex;
};

#endif // ADVSECSETTINGSSECURITYMODULEVIEW_H
