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
* Description:  Symbian specific container class for a security module
*
*/

#ifndef ADVSECSETTINGSSECURITYMODULE_SYMBIAN_H
#define ADVSECSETTINGSSECURITYMODULE_SYMBIAN_H

#include <e32base.h>                    // CBase
#include <ct/rmpointerarray.h>          // RMPointerArray

class CCTKeyInfo;
class MCTKeyStore;
class MCTAuthenticationObject;


/**
 * Symbian specific container class for a security module.
 */
class CAdvSecSettingsSecurityModuleSymbian : public CBase
{
public:     // constructor and destructor
    CAdvSecSettingsSecurityModuleSymbian(MCTKeyStore &aProtectedKeyStore);
    ~CAdvSecSettingsSecurityModuleSymbian();

public:     // new functions
    const TDesC &Label() const;
    TBool IsDeletable() const;
    TBool IsSigningPinSupported() const;
    RMPointerArray<CCTKeyInfo> &KeyInfoArray();

private:    // data
    MCTKeyStore &iProtectedKeyStore;
    RMPointerArray<CCTKeyInfo> iKeyInfoArray;
    RMPointerArray<MCTAuthenticationObject> iAuthenticationObjects;
};

#endif // ADVSECSETTINGSSECURITYMODULE_SYMBIAN_H
