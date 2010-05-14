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
 * Description:
 *
 */

#ifndef SECCODESETTINGS_H
#define SECCODESETTINGS_H

// System includes
#include <QObject>
#include "seccodeuiglobal.h"

// Forward declarations
class SecCodeSettingsPrivate;

// Class declaration
class SecCodeSettings : public QObject
{
    Q_OBJECT
    
public:
    explicit SecCodeSettings(QObject *parent = 0);
    virtual ~SecCodeSettings();
    bool pinCodeRequest() const;
    bool changePinCodeRequest();
    void changePinCode();
    void changePin2Code();
    
private:
    SecCodeSettingsPrivate *d_ptr;
};

#endif //SECCODESETTINGS_H
