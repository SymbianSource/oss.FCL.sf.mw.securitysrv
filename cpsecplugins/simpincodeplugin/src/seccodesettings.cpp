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
#include "seccodesettings.h"
#include "seccodemodel.h"

// Class declaration
class SecCodeSettingsPrivate
{
public:
    SecCodeSettingsPrivate()
    {
        mSecCodeModel = new SecCodeModel();
    }
    
    ~SecCodeSettingsPrivate()
    {
        delete mSecCodeModel;
    }
    
    SecCodeModel *mSecCodeModel;
};

// ======== MEMBER FUNCTIONS ========

/*!
    Constructor
*/
SecCodeSettings::SecCodeSettings(QObject * parent /*= 0*/)
    : QObject(parent), d_ptr(new SecCodeSettingsPrivate())
{

}

/*!
    Destructor
*/
SecCodeSettings::~SecCodeSettings()
{
    delete d_ptr;
}

/*!
    Get Pin Code Request
*/
bool SecCodeSettings::pinCodeRequest() const
{
    return d_ptr->mSecCodeModel->pinCodeRequest();
}

/*!
    Change Pin Code request
*/
bool SecCodeSettings::changePinCodeRequest()
{
    return d_ptr->mSecCodeModel->changePinCodeRequest();
}

/*!
    Change Pin Code
*/
void SecCodeSettings::changePinCode()
{
    d_ptr->mSecCodeModel->changePinCode();
}

/*!
    Change Pin2 Code
*/
void SecCodeSettings::changePin2Code()
{    
    d_ptr->mSecCodeModel->changePin2Code();
}
