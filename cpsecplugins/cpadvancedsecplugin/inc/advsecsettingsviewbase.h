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
* Description:  Base class for advanced security settings views
*
*/

#ifndef ADVSECSETTINGSVIEWBASE_H
#define ADVSECSETTINGSVIEWBASE_H

#include <cpbasesettingview.h>      // CpBaseSettingView

class HbMenu;


/**
 * Base class for advanced security settings views.
 */
class AdvSecSettingsViewBase : public CpBaseSettingView
{
    Q_OBJECT

public:     // constructor and destructor
    explicit AdvSecSettingsViewBase(QGraphicsWidget *widget = 0,
        QGraphicsItem *parent = 0);
    virtual ~AdvSecSettingsViewBase();

public slots:
    void displayPreviousView();

protected slots:
    virtual void displayError(int error);

public:     // new functions
    void displayView(AdvSecSettingsViewBase *view);
    bool hasPreviousView() const;
    void setDeleteOnClose(bool deleteOnClose);

protected:  // new functions
    void clearItemSpecificMenu();
    void addItemSpecificMenuAction(const QString &text, const QObject *receiver,
        const char *member);
    void displayItemSpecificMenu(const QPointF &position);
    void displayQuestionNote(const QString &text, const QObject *receiver,
        const char *accepedMember);

private:    // data
    HbView *mPreviousView;      // not owned
    HbMenu *mItemSpecificMenu;
    bool mDeleteOnClose;
};

#endif // ADVSECSETTINGSVIEWBASE_H
