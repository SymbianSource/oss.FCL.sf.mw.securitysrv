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
* Description:  Security module view in advanced security settings.
*
*/

#include "advsecsettingssecuritymoduleview.h"
#include "advsecsettingssecuritymodulemodel.h"
#include <QGraphicsLinearLayout>
#include <HbGroupBox>
#include <HbDataForm>
#include <HbDataFormModel>
#include <HbLineEdit>
#include <QDebug>

const QString KEchoModeProperty = "echoMode";
const QString KTextProperty = "text";
const QString KAdditionalTextProperty = "additionalText";
const QString KReadOnlyProperty = "readOnly";
const QString KPasswordValue = "****";


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleView::AdvSecSettingsSecurityModuleView()
// ---------------------------------------------------------------------------
//
AdvSecSettingsSecurityModuleView::AdvSecSettingsSecurityModuleView(
    AdvSecSettingsSecurityModuleModel &model, QGraphicsItem *parent) :
    AdvSecSettingsViewBase(0, parent), mModel(model), mViewLabel(0),
    mModulePin(0), mModulePinRequested(0), mModuleStatus(0), mSigningPin(0),
    mModelIndex(0)
{
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical);

    // View title
    mViewLabel = new HbGroupBox;
    layout->addItem(mViewLabel);

    HbDataForm *dataForm = new HbDataForm;
    mDataFormModel = new HbDataFormModel;
    HbDataFormModelItem *rootItem = mDataFormModel->invisibleRootItem();

    // Module PIN
    // TODO: localized UI strings needed
    HbDataFormModelItem *moduleGroup = 0;
    moduleGroup = mDataFormModel->appendDataFormGroup(tr("Module PIN"), rootItem);
    mModulePin = mDataFormModel->appendDataFormItem(HbDataFormModelItem::TextItem,
        tr("PIN code"), moduleGroup);
    mModulePin->setContentWidgetData(KEchoModeProperty, HbLineEdit::Password);
    mModulePin->setContentWidgetData(KTextProperty, KPasswordValue);
    mModulePin->setContentWidgetData(KReadOnlyProperty, true);

    // Module PIN Request
    mModulePinRequested = mDataFormModel->appendDataFormItem(
        HbDataFormModelItem::ToggleValueItem, tr("PIN code required"), moduleGroup);
    mModulePinRequested->setContentWidgetData(KTextProperty, tr("On"));
    //mModulePinRequested->setContentWidgetData(KAdditionalTextProperty, tr("Changing..."));
    // TODO: remove
    mModulePinRequested->setEnabled(false);

    // Module Status
    mModuleStatus = mDataFormModel->appendDataFormItem(
        HbDataFormModelItem::ToggleValueItem, tr("Status"), moduleGroup);
    mModuleStatus->setContentWidgetData(KTextProperty, tr("Closed"));
    mModuleStatus->setEnabled(false);

    dataForm->setModel(mDataFormModel);
    layout->addItem(dataForm);
    setLayout(layout);

    connect(dataForm, SIGNAL(activated(const QModelIndex &)),
        this, SLOT(itemActivated(const QModelIndex &)));
    connect(&mModel, SIGNAL(statusCompleted(int)), this, SLOT(moduleStatusChanged(int)));
    connect(&mModel, SIGNAL(statusChanged(int)), this, SLOT(moduleStatusChanged(int)));
    connect(&mModel, SIGNAL(pinCodeRequestStateCompleted()), this, SLOT(updateModuleStatus()));
    connect(&mModel, SIGNAL(pinCodeChangeCompleted()), this, SLOT(updateModuleStatus()));
    connect(&mModel, SIGNAL(closeCompleted()), this, SLOT(updateModuleStatus()));
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleView::~AdvSecSettingsSecurityModuleView()
// ---------------------------------------------------------------------------
//
AdvSecSettingsSecurityModuleView::~AdvSecSettingsSecurityModuleView()
{
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleView::setSecurityModule()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleView::setSecurityModule(const QString &moduleTitle,
    int modelIndex)
{
    mViewLabel->setHeading(moduleTitle);
    mModelIndex = modelIndex;

    // Signing PIN
    if (mModel.isSigningPinSupported(mModelIndex)) {
        if (!mSigningPin) {
            HbDataFormModelItem *rootItem = mDataFormModel->invisibleRootItem();
            HbDataFormModelItem *signingGroup = 0;
            signingGroup = mDataFormModel->appendDataFormGroup(tr("Signing PIN"), rootItem);
            mSigningPin = mDataFormModel->appendDataFormItem(HbDataFormModelItem::TextItem,
                tr("PIN code"), signingGroup);
            mSigningPin->setContentWidgetData(KEchoModeProperty, HbLineEdit::Password);
            mSigningPin->setContentWidgetData(KTextProperty, KPasswordValue);
            mSigningPin->setContentWidgetData(KReadOnlyProperty, true);
        }
    } else {
        if (mSigningPin) {
            mDataFormModel->removeItem(mSigningPin);
            mSigningPin = 0;
        }
    }

    updateModuleStatus();
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleView::updateModuleStatus()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleView::updateModuleStatus()
{
    mModel.getModuleStatus(mModelIndex);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleView::moduleStatusChanged()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleView::moduleStatusChanged(int status)
{
    // TODO: localized UI strings needed
    if (status & AdvSecSettingsSecurityModuleModel::EBlockedPermanently) {
        mModulePinRequested->setContentWidgetData(KTextProperty, tr("Blocked"));
        mModulePinRequested->setEnabled(false);
    } else if (status & AdvSecSettingsSecurityModuleModel::EPinBlocked) {
        mModulePinRequested->setContentWidgetData(KTextProperty, tr("Blocked"));
        // TODO: mModulePinRequested->setEnabled(true);
        mModulePinRequested->setEnabled(false);
    } else {
        if (status & AdvSecSettingsSecurityModuleModel::EPinRequested) {
            mModulePinRequested->setContentWidgetData(KTextProperty, tr("On"));
        } else {
            mModulePinRequested->setContentWidgetData(KTextProperty, tr("Off"));
        }
        if (status & AdvSecSettingsSecurityModuleModel::EPinChangeAllowed) {
            // TODO:
            //mModulePinRequested->setContentWidgetData(KAdditionalTextProperty, tr("Changing..."));
            //mModulePinRequested->setEnabled(true);
            mModulePinRequested->setEnabled(false);
        } else {
            mModulePinRequested->setContentWidgetData(KAdditionalTextProperty, QString());
            mModulePinRequested->setEnabled(false);
        }
    }
    if (status & AdvSecSettingsSecurityModuleModel::EPinEntered) {
        mModuleStatus->setContentWidgetData(KTextProperty, tr("Open"));
        // TODO:
        //mModuleStatus->setContentWidgetData(KAdditionalTextProperty, tr("Closing..."));
        //mModuleStatus->setEnabled(true);
        mModuleStatus->setEnabled(false);
    } else {
        mModuleStatus->setContentWidgetData(KTextProperty, tr("Closed"));
        mModuleStatus->setContentWidgetData(KAdditionalTextProperty, QString());
        mModuleStatus->setEnabled(false);
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleView::itemActivated()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleView::itemActivated(const QModelIndex &/*itemIndex*/)
{
    // TODO: this does not work yet
#if 0
    bool isOperationStarted = false;
    HbDataFormModelItem *item = mDataFormModel->itemFromIndex(itemIndex);
    if (item == mModulePin) {
        mModel.changePinCode(mModelIndex);
        isOperationStarted = true;
    } else if (item == mModulePinRequested) {
        QString contentData = mModulePinRequested->contentWidgetData(KTextProperty).toString();
        bool enable = (contentData != tr("On"));
        mModel.setPinCodeRequestState(mModelIndex, enable);
        isOperationStarted = true;
    } else if (item == mModuleStatus) {
        QString contentData = mModulePinRequested->contentWidgetData(KTextProperty).toString();
        bool isClosed = (contentData == tr("Closed"));
        if (!isClosed) {
            mModel.closeModule(mModelIndex);
            isOperationStarted = true;
        }
    } else if (item == mSigningPin) {
        mModel.changeSigningPinCode(mModelIndex);
        isOperationStarted = true;
    } else {
        // ignored, one of the group titles
    }
    if (isOperationStarted) {
        mModulePin->setEnabled(false);
        mModulePinRequested->setEnabled(false);
        mModuleStatus->setEnabled(false);
        if (mSigningPin) {
            mSigningPin->setEnabled(false);
        }
    }
#endif
}

