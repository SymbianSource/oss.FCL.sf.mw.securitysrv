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

#ifndef ADVSECSETTINGSSECURITYMODULEMODELPRIVATE_STUB_H
#define ADVSECSETTINGSSECURITYMODULEMODELPRIVATE_STUB_H

#include <QObject>

class AdvSecSettingsSecurityModuleModel;
class AdvSecSettingsSecurityModule;


/**
 * Generic stub for platform specific private security module model.
 * This is just a stub. Real functionality is provided in platform specific
 * AdvSecSettingsSecurityModuleModelPrivate implementation.
 */
class AdvSecSettingsSecurityModuleModelPrivate : public QObject
{
    Q_OBJECT

public:     // constructor and destructor
    explicit AdvSecSettingsSecurityModuleModelPrivate(AdvSecSettingsSecurityModuleModel *q);
    virtual ~AdvSecSettingsSecurityModuleModelPrivate();

public:     // new functions
    void initialize();
    int moduleCount() const;
    QMap<QString,QString> moduleLabelsAndLocations() const;
    void getModuleStatus(int moduleIndex);
    void setPinCodeRequestState(int moduleIndex, bool isRequested);
    void changePinCode(int moduleIndex);  // unblocks PIN code if EPinBlocked
    void closeModule(int moduleIndex);
    bool isSigningPinSupported(int moduleIndex) const;
    void changeSigningPinCode(int moduleIndex);
    bool isDeletable(int moduleIndex) const;
    void deleteModule(int moduleIndex);

private:    // data
    Q_DISABLE_COPY(AdvSecSettingsSecurityModuleModelPrivate)
    AdvSecSettingsSecurityModuleModel *q_ptr;
};

#endif // ADVSECSETTINGSSECURITYMODULEMODELPRIVATE_STUB_H
