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

#include "advsecsettingssecuritymoduledetailview.h"
#include "advsecsettingssecuritymodulemodel.h"
#include <QGraphicsLinearLayout>
#include <HbGroupBox>
#include <HbScrollArea>
#include <HbMenu>
#include <HbAction>
#include <HbLabel>


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleDetailView::AdvSecSettingsSecurityModuleDetailView()
// ---------------------------------------------------------------------------
//
AdvSecSettingsSecurityModuleDetailView::AdvSecSettingsSecurityModuleDetailView(
    AdvSecSettingsSecurityModuleModel &model, QGraphicsItem *parent) :
    AdvSecSettingsViewBase(0, parent), mModel(model), mModuleIndex(0),
    mViewLabel(0), mDetailsText(0)
{
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical);

    mViewLabel = new HbGroupBox;
    layout->addItem(mViewLabel);

    HbScrollArea *scrollArea = new HbScrollArea;
    mDetailsText = new HbLabel;
    mDetailsText->setTextWrapping(Hb::TextWordWrap);
    scrollArea->setContentWidget(mDetailsText);
    scrollArea->setVerticalScrollBarPolicy(HbScrollArea::ScrollBarAlwaysOn);
    layout->addItem(scrollArea);

    setLayout(layout);

    connect(&mModel, SIGNAL(detailsCompleted(QMap<int,QString>)),
        this, SLOT(detailsCompleted(QMap<int,QString>)));
    connect(&mModel, SIGNAL(deleteCompleted()),
        this, SLOT(displayPreviousView()));
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleDetailView::~AdvSecSettingsSecurityModuleDetailView()
// ---------------------------------------------------------------------------
//
AdvSecSettingsSecurityModuleDetailView::~AdvSecSettingsSecurityModuleDetailView()
{
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleDetailView::setSecurityModule()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleDetailView::setSecurityModule(int index)
{
    mModuleIndex = index;
    // TODO: remove
    mDetailsText->setPlainText(tr("TODO: Add info for module %1").arg(index));

    menu()->clearActions();
    if (mModel.isDeletable(index)) {
        HbAction *deleteAction = new HbAction(hbTrId("txt_common_menu_delete"));
        connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteSecurityModule()));
        menu()->addAction(deleteAction);
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleDetailView::detailsCompleted()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleDetailView::detailsCompleted(QMap<int,QString> /*details*/)
{
    // TODO: implement
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleDetailView::deleteSecurityModule()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleDetailView::deleteSecurityModule()
{
    Q_ASSERT(mViewLabel != 0);
    // TODO: localized UI string needed
    QString confirmText(tr("Delete %1?").arg(mViewLabel->heading()));
    displayQuestionNote(confirmText, this, SLOT(deleteConfirmationAccepted()));
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleDetailView::deleteConfirmationAccepted()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleDetailView::deleteConfirmationAccepted()
{
    mModel.deleteModule(mModuleIndex);
}

