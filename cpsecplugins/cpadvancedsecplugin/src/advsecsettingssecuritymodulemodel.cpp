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
* Description:  Security module model in advanced security settings
*
*/

#include "advsecsettingssecuritymodulemodel.h"

#if defined(Q_OS_SYMBIAN)
#include "advsecsettingssecuritymodulemodel_symbian_p.h"
#else
#include "advsecsettingssecuritymodulemodel_stub_p.h"
#endif


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModel::AdvSecSettingsSecurityModuleModel()
// ---------------------------------------------------------------------------
//
AdvSecSettingsSecurityModuleModel::AdvSecSettingsSecurityModuleModel(
    QObject *parent) : QObject(parent)
{
    d_ptr = new AdvSecSettingsSecurityModuleModelPrivate(this);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModel::~AdvSecSettingsSecurityModuleModel()
// ---------------------------------------------------------------------------
//
AdvSecSettingsSecurityModuleModel::~AdvSecSettingsSecurityModuleModel()
{
    delete d_ptr;
    d_ptr = 0;
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModel::initialize()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModel::initialize()
{
    if (d_ptr) {
        d_ptr->initialize();
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModel::moduleCount()
// ---------------------------------------------------------------------------
//
int AdvSecSettingsSecurityModuleModel::moduleCount() const
{
    if (d_ptr) {
        return d_ptr->moduleCount();
    }
    return 0;
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModel::moduleLabelsAndLocations()
// ---------------------------------------------------------------------------
//
QMap<QString,QString> AdvSecSettingsSecurityModuleModel::moduleLabelsAndLocations() const
{
    if (d_ptr) {
        return d_ptr->moduleLabelsAndLocations();
    }
    QMap<QString,QString> emptyMap;
    return emptyMap;
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModel::getModuleStatus()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModel::getModuleStatus(int moduleIndex)
{
    if (d_ptr) {
        d_ptr->getModuleStatus(moduleIndex);
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModel::setPinCodeRequestState()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModel::setPinCodeRequestState(int moduleIndex,
    bool isRequested)
{
    if (d_ptr) {
        d_ptr->setPinCodeRequestState(moduleIndex, isRequested);
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModel::changePinCode()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModel::changePinCode(int moduleIndex)
{
    if (d_ptr) {
        d_ptr->changePinCode(moduleIndex);
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModel::closeModule()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModel::closeModule(int moduleIndex)
{
    if (d_ptr) {
        d_ptr->closeModule(moduleIndex);
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModel::isSigningPinSupported()
// ---------------------------------------------------------------------------
//
bool AdvSecSettingsSecurityModuleModel::isSigningPinSupported(int moduleIndex) const
{
    if (d_ptr) {
        return d_ptr->isSigningPinSupported(moduleIndex);
    }
    return false;
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModel::changeSigningPinCode()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModel::changeSigningPinCode(int moduleIndex)
{
    if (d_ptr) {
        d_ptr->changeSigningPinCode(moduleIndex);
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModel::isDeletable()
// ---------------------------------------------------------------------------
//
bool AdvSecSettingsSecurityModuleModel::isDeletable(int moduleIndex) const
{
    if (d_ptr) {
        return d_ptr->isDeletable(moduleIndex);
    }
    return false;
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModel::deleteModule()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModel::deleteModule(int moduleIndex)
{
    if (d_ptr) {
        d_ptr->deleteModule(moduleIndex);
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModel::handleInitializeCompleted()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModel::handleInitializeCompleted()
{
    emit initializeCompleted();
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModel::handleStatusCompleted()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModel::handleStatusCompleted(
    int authenticationStatus)
{
    emit statusCompleted(authenticationStatus);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModel::handleStatusChanged()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModel::handleStatusChanged(int moduleIndex,
    int authenticationStatus)
{
    emit statusChanged(moduleIndex, authenticationStatus);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModel::handlePinCodeRequestSet()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModel::handlePinCodeRequestSet()
{
    emit pinCodeRequestStateCompleted();
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModel::handlePinCodeChanged()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModel::handlePinCodeChanged()
{
    emit pinCodeChangeCompleted();
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModel::handleModuleClosed()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModel::handleModuleClosed()
{
    emit closeCompleted();
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModel::handleSigningPinCodeChanged()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModel::handleSigningPinCodeChanged()
{
    emit signingPinCodeChangeCompleted();
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModel::handleModuleDeleted()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModel::handleModuleDeleted()
{
    emit deleteCompleted();
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModel::handleError()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModel::handleError(int error)
{
    emit errorOccurred(error);
}

