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

// System includes
#include <e32err.h>
#include <exterror.h>
#include <etelmm.h>

// User includes
#include "seccodemodel_p.h"
#include "seccodeuiglobal.h"
#include <../../inc/cpsecplugins.h>

// ======== MEMBER FUNCTIONS ========

/*!
    NewL
*/
SecCodeModelPrivate* SecCodeModelPrivate::NewL()
{
    SecCodeModelPrivate* self = new (ELeave) SecCodeModelPrivate();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(); //self
    return self;
}

/*!
    SecCodeModelPrivate
*/
SecCodeModelPrivate::SecCodeModelPrivate()
{
}

/*!
    ConstructL
*/
void SecCodeModelPrivate::ConstructL()
{
	RDEBUG("0", 0);
	iSecurity = CSecuritySettings::NewL(); 
}

/*!
    Destructor
*/
SecCodeModelPrivate::~SecCodeModelPrivate()
{
	if (iSecurity) {
		delete iSecurity;
	}
}

/*!
    Get Pin Code Request State
*/
bool SecCodeModelPrivate::pinCodeRequest() const
{
	RDEBUG("0", 0);
	bool isLockEnabled = false;
	QT_TRAP_THROWING(isLockEnabled = iSecurity->IsLockEnabledL(
		RMobilePhone::ELockICC));
	RDEBUG("isLockEnabled", isLockEnabled);
	return isLockEnabled;
}

/*!
    Get the Change Pin Remaining Attempts
*/
bool SecCodeModelPrivate::changePinCodeRequest()
{
	RDEBUG("0", 0);
    bool result = false;
	QT_TRAP_THROWING(result =iSecurity->ChangePinRequestL());
	RDEBUG("result", result);
	return result;
}

/*!
    Change Pin Code
*/
int SecCodeModelPrivate::changePinCode()
{
	RDEBUG("0", 0);
	QT_TRAP_THROWING(iSecurity->ChangePinL());
	RDEBUG("0", 0);
	return 0;
}

/*!
    Change Pin2 Code
*/
int SecCodeModelPrivate::changePin2Code()
{
	RDEBUG("0", 0);
	QT_TRAP_THROWING(iSecurity->ChangePin2L());
	RDEBUG("0", 0);
	return 0;
}
