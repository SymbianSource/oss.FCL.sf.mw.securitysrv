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
#include "seccodemodel_p.h"

// ======== MEMBER FUNCTIONS ========
/*!
    NewL
*/
SecCodeModelPrivate* SecCodeModelPrivate::NewL()
{
    return NULL;
}

/*!
    Constructor
*/
SecCodeModelPrivate::SecCodeModelPrivate()
{
}

/*!
    ConstructL
*/
void SecCodeModelPrivate::ConstructL()
{

}

/*!
    Destructor
*/
SecCodeModelPrivate::~SecCodeModelPrivate()
{
}

/*!
    Get Pin Code Request State
*/
bool SecCodeModelPrivate::pinCodeRequest() const
{
    QString information;
    information = QString("Emulator NOT supported!");
    HbMessageBox messageBox(information);
    messageBox.exec();
    return false;
}

/*!
    Get the Change Pin Remaining Attempts
*/
bool SecCodeModelPrivate::changePinCodeRequest()
{
    QString information;
    information = QString("Emulator NOT supported!");
    HbMessageBox messageBox(information);
    messageBox.exec();
    return true;
}

/*!
    Change Pin Code
*/
int SecCodeModelPrivate::changePinCode()
{
    QString information;
    information = QString("Emulator NOT supported!");
    HbMessageBox messageBox(information);
    messageBox.exec();
    return 0;
}

/*!
    Change Pin2 Code
*/
int SecCodeModelPrivate::changePin2Code()
{
    QString information;
    information = QString("Emulator NOT supported!");
    HbMessageBox messageBox(information);
    messageBox.exec();
    return 0;
}
