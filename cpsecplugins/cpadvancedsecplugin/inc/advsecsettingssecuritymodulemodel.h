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

#ifndef ADVSECSETTINGSSECURITYMODULEMODEL_H
#define ADVSECSETTINGSSECURITYMODULEMODEL_H

#include <QObject>
#include <QMap>

class AdvSecSettingsSecurityModule;
class AdvSecSettingsSecurityModuleModelPrivate;


/**
 * Security module model in advanced security settings.
 */
class AdvSecSettingsSecurityModuleModel : public QObject
{
    Q_OBJECT

public:     // constructor and destructor
    explicit AdvSecSettingsSecurityModuleModel(QObject *parent = 0);
    virtual ~AdvSecSettingsSecurityModuleModel();

public:     // new definitions
    enum SecurityModuleDetailsField {
        Label,
        Version,
        Location,
        SerialNumber,
        Manufacturer
    };
    enum AuthenticationStatus {
        EPinEntered         = 0x01, // Set if the module is open, PIN already entered
        EPinRequested       = 0x02, // Set if PIN code required to open the module
        EPinChangeAllowed   = 0x04, // Set if PIN code change is allowed
        EPinBlocked         = 0x10, // Set if PIN code is blocked, requires unblocking PIN
        EBlockedPermanently = 0x20, // Set if PIN code blocked and cannot be unblocked
    };

public:     // new functions
    void initialize();
    int moduleCount() const;
    QMap<QString,QString> moduleLabelsAndLocations() const;
    void getModuleDetails(int moduleIndex);
    void getModuleStatus(int moduleIndex);
    // Functions related to PIN-G (aka access PIN, login PIN)
    void setPinCodeRequestState(int moduleIndex, bool isRequested);
    void changePinCode(int moduleIndex);  // unblocks PIN code if EPinBlocked
    void closeModule(int moduleIndex);
    // Functions related to PIN-NR (aka signature PIN, signing PIN)
    bool isSigningPinSupported(int moduleIndex) const;
    void changeSigningPinCode(int moduleIndex);
    bool isDeletable(int moduleIndex) const;
    void deleteModule(int moduleIndex);

signals:
    void initializeCompleted();
    void detailsCompleted(QMap<int,QString> details);
    void statusCompleted(int authenticationStatus);
    void statusChanged(int moduleIndex, int authenticationStatus);
    void pinCodeRequestStateCompleted();
    void pinCodeChangeCompleted();
    void closeCompleted();
    void signingPinCodeChangeCompleted();
    void deleteCompleted();
    void errorOccurred(int error);

private:    // new functions
    void handleInitializeCompleted();
    void handleStatusCompleted(int authenticationStatus);
    void handleStatusChanged(int moduleIndex, int authenticationStatus);
    void handlePinCodeRequestSet();
    void handlePinCodeChanged();
    void handleModuleClosed();
    void handleSigningPinCodeChanged();
    void handleModuleDeleted();
    void handleError(int error);

private:    // data
    Q_DISABLE_COPY(AdvSecSettingsSecurityModuleModel)
    friend class AdvSecSettingsSecurityModuleModelPrivate;
    AdvSecSettingsSecurityModuleModelPrivate *d_ptr;
};

#endif // ADVSECSETTINGSSECURITYMODULEMODEL_H
