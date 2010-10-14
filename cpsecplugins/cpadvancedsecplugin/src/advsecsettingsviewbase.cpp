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

#include "advsecsettingsviewbase.h"
#include <HbMainWindow>
#include <HbAction>
#include <HbMenu>
#include <HbMessageBox>


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// AdvSecSettingsViewBase::AdvSecSettingsViewBase()
// ---------------------------------------------------------------------------
//
AdvSecSettingsViewBase::AdvSecSettingsViewBase(QGraphicsWidget *widget,
    QGraphicsItem *parent) : CpBaseSettingView(widget, parent), mPreviousView(0),
    mItemSpecificMenu(0), mDeleteOnClose(false)
{
    connect(this, SIGNAL(aboutToClose()), this, SLOT(displayPreviousView()));
}

// ---------------------------------------------------------------------------
// AdvSecSettingsViewBase::AdvSecSettingsViewBase()
// ---------------------------------------------------------------------------
//
AdvSecSettingsViewBase::~AdvSecSettingsViewBase()
{
    delete mItemSpecificMenu;
    mItemSpecificMenu = 0;
}

// ---------------------------------------------------------------------------
// AdvSecSettingsViewBase::displayPreviousView()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsViewBase::displayPreviousView()
{
    if (mPreviousView) {
        HbMainWindow &window = *mainWindow();
        HbView *currentView = window.currentView();
        window.setCurrentView(mPreviousView);
        mPreviousView = 0;
        if (mDeleteOnClose) {
            window.removeView(currentView);
            currentView->deleteLater();
        }
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsViewBase::displayError()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsViewBase::displayError(int error)
{
    HbMessageBox *messageBox = new HbMessageBox(HbMessageBox::MessageTypeWarning);
    messageBox->setAttribute(Qt::WA_DeleteOnClose);
    messageBox->setStandardButtons(HbMessageBox::Ok);

    // TODO: proper error texts, localised UI texts needed
    QString text = tr("Error %1").arg(error);
    messageBox->setText(text);
    messageBox->open();
}

// ---------------------------------------------------------------------------
// AdvSecSettingsViewBase::displayView()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsViewBase::displayView(AdvSecSettingsViewBase *view)
{
    HbMainWindow &window = *mainWindow();
    QList<HbView *> addedViews = window.views();
    if (!addedViews.contains(view)) {
        window.addView(view);
    }
    view->mPreviousView = window.currentView();
    window.setCurrentView(view);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsViewBase::hasPreviousView()
// ---------------------------------------------------------------------------
//
bool AdvSecSettingsViewBase::hasPreviousView() const
{
    return (mPreviousView != 0);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsViewBase::setDeleteOnClose()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsViewBase::setDeleteOnClose(bool deleteOnClose)
{
    mDeleteOnClose = deleteOnClose;
}

// ---------------------------------------------------------------------------
// AdvSecSettingsViewBase::clearItemSpecificMenu()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsViewBase::clearItemSpecificMenu()
{
    if (!mItemSpecificMenu) {
        mItemSpecificMenu = new HbMenu;
    } else {
        mItemSpecificMenu->clearActions();
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsViewBase::addItemSpecificMenuAction()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsViewBase::addItemSpecificMenuAction(const QString &text,
    const QObject *receiver, const char *member)
{
    HbAction *action = new HbAction(text);
    connect(action, SIGNAL(triggered()), receiver, member);
    mItemSpecificMenu->addAction(action);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsViewBase::displayItemSpecificMenu()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsViewBase::displayItemSpecificMenu(const QPointF &position)
{
    mItemSpecificMenu->setPreferredPos(position);
    mItemSpecificMenu->open();
}

// ---------------------------------------------------------------------------
// AdvSecSettingsViewBase::displayQuestionNote()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsViewBase::displayQuestionNote(const QString &text,
    const QObject *receiver, const char *accepedMember)
{
    HbMessageBox *messageBox = new HbMessageBox;
    messageBox->setAttribute(Qt::WA_DeleteOnClose);

    messageBox->clearActions();
    HbAction *yesAction = new HbAction(hbTrId("txt_common_button_yes"));
    connect(yesAction, SIGNAL(triggered()), receiver, accepedMember);
    messageBox->addAction(yesAction);
    HbAction *noAction = new HbAction(hbTrId("txt_common_button_no"));
    messageBox->addAction(noAction);

    messageBox->setText(text);
    messageBox->open();
}

