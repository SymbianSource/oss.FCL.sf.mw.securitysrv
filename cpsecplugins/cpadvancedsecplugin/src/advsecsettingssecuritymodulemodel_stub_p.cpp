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
* Description:  Generic stub for platform specific private security module model
*
*/

#include "advsecsettingssecuritymodulemodel_stub_p.h"


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::AdvSecSettingsSecurityModuleModelPrivate()
// ---------------------------------------------------------------------------
//
AdvSecSettingsSecurityModuleModelPrivate::AdvSecSettingsSecurityModuleModelPrivate(
    AdvSecSettingsSecurityModuleModel *q) : QObject(0), q_ptr(q)
{
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::~AdvSecSettingsSecurityModuleModelPrivate()
// ---------------------------------------------------------------------------
//
AdvSecSettingsSecurityModuleModelPrivate::~AdvSecSettingsSecurityModuleModelPrivate()
{
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::initialize()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModelPrivate::initialize()
{
    q_ptr->handleInitializeCompleted();
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::moduleCount()
// ---------------------------------------------------------------------------
//
int AdvSecSettingsSecurityModuleModelPrivate::moduleCount() const
{
    return 0;
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::moduleLabelsAndLocations()
// ---------------------------------------------------------------------------
//
QMap<QString,QString> AdvSecSettingsSecurityModuleModelPrivate::moduleLabelsAndLocations() const
{
    QMap<QString,QString> emptyMap;
    return emptyMap;
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::getModuleStatus()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModelPrivate::getModuleStatus(int /*moduleIndex*/)
{
    q_ptr->handleStatusCompleted(0, false, false);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::setPinCodeRequestState()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModelPrivate::setPinCodeRequestState(
    int /*moduleIndex*/, bool /*isRequested*/)
{
    q_ptr->handlePinCodeRequestSet();
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::changePinCode()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModelPrivate::changePinCode(int /*moduleIndex*/)
{
    q_ptr->handlePinCodeChanged();
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::closeModule()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModelPrivate::closeModule(int /*moduleIndex*/)
{
    q_ptr->handleModuleClosed();
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::isSigningPinSupported()
// ---------------------------------------------------------------------------
//
bool AdvSecSettingsSecurityModuleModelPrivate::isSigningPinSupported(int /*moduleIndex*/) const
{
    return false;
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::changeSigningPinCode()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModelPrivate::changeSigningPinCode(int /*moduleIndex*/)
{
    q_ptr->handleSigningPinCodeChanged();
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::isDeletable()
// ---------------------------------------------------------------------------
//
bool AdvSecSettingsSecurityModuleModelPrivate::isDeletable(int /*moduleIndex*/) const
{
    return false;
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::deleteModule()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModelPrivate::deleteModule(int /*moduleIndex*/)
{
    q_ptr->handleModuleDeleted();
}

