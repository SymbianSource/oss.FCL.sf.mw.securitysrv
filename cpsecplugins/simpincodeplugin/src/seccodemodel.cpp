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

// User includes
#include "seccodemodel.h"
#include "seccodemodel_p.h"

// ======== MEMBER FUNCTIONS ========

/*!
    Constructor
*/
SecCodeModel::SecCodeModel(QObject *parent/*= 0*/)
    : QObject(parent)
{
    QT_TRAP_THROWING(d_ptr = SecCodeModelPrivate::NewL());
}

/*!
    Destructor
*/
SecCodeModel::~SecCodeModel()
{
    delete d_ptr;
}

/*!
    Get Pin Code Request State
*/
bool SecCodeModel::pinCodeRequest() const
{
    return d_ptr->pinCodeRequest();
}

/*!
    Change Pin Code
*/
bool SecCodeModel::changePinCodeRequest()
{
    return d_ptr->changePinCodeRequest();
}

/*!
    Change Pin Code
*/
int SecCodeModel::changePinCode()
{
    return d_ptr->changePinCode();
}

/*!
    Change Pin2 Code
*/
int SecCodeModel::changePin2Code()
{
    return d_ptr->changePin2Code();
}
