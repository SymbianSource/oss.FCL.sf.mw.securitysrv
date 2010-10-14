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
* Description:  Security module detail view in advanced security settings
*
*/

#ifndef ADVSECSETTINGSSECURITYMODULEDETAILVIEW_H
#define ADVSECSETTINGSSECURITYMODULEDETAILVIEW_H

#include "advsecsettingsviewbase.h"

class AdvSecSettingsSecurityModuleModel;
class HbGroupBox;
class HbLabel;


/**
 * Shows security module detail view.
 */
class AdvSecSettingsSecurityModuleDetailView : public AdvSecSettingsViewBase
{
    Q_OBJECT

public:     // constructor and destructor
    explicit AdvSecSettingsSecurityModuleDetailView(
        AdvSecSettingsSecurityModuleModel &model, QGraphicsItem *parent = 0);
    virtual ~AdvSecSettingsSecurityModuleDetailView();

public:    // new functions
    void setSecurityModule(int index);

private slots:
    void detailsCompleted(QMap<int,QString> details);
    void deleteSecurityModule();
    void deleteConfirmationAccepted();

private:    // data
    AdvSecSettingsSecurityModuleModel &mModel;
    int mModuleIndex;
    HbGroupBox *mViewLabel;
    HbLabel *mDetailsText;
};

#endif // ADVSECSETTINGSSECURITYMODULEDETAILVIEW_H
