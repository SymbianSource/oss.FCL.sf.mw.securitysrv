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

#ifndef SECCODEMODEL_P_H
#define SECCODEMODEL_P_H

// System includes
#include <QObject>
#include <secuisecuritysettings.h>
// Class declaration
class SecCodeModelPrivate
{
public:
    static SecCodeModelPrivate* NewL();
    SecCodeModelPrivate();
    ~SecCodeModelPrivate();
    bool pinCodeRequest() const;
    bool changePinCodeRequest();
    int changePinCode();
    int changePin2Code();

private:
    void ConstructL();

private: // data
    //to launch SecUI
    CSecuritySettings* iSecurity;
};

#endif //SECCODEMODEL_P_H
