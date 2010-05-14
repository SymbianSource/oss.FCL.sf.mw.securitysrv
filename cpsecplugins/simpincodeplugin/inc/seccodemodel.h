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

#ifndef SECCODEMODEL_H
#define SECCODEMODEL_H

// System includes
#include <QObject>

// Forward declarations
class SecCodeModelPrivate;

// Class declaration
class SecCodeModel : public QObject
{
    Q_OBJECT

public:
    SecCodeModel(QObject *parent = 0);
    virtual ~SecCodeModel();
    bool pinCodeRequest() const;
    bool changePinCodeRequest();
    int changePinCode();
    int changePin2Code();
 
private: // data
    SecCodeModelPrivate *d_ptr;
};

#endif //SECCODEMODEL_H
