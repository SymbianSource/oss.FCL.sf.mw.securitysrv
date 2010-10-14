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

#include "advsecsettingssecuritymodule_symbian.h"
#include <cctcertinfo.h>                // CCTCertInfo
#include <mctkeystore.h>                // CCTKeyInfo


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CAdvSecSettingsSecurityModuleSymbian::CAdvSecSettingsSecurityModuleSymbian()
// ---------------------------------------------------------------------------
//
CAdvSecSettingsSecurityModuleSymbian::CAdvSecSettingsSecurityModuleSymbian(
    MCTKeyStore &aProtectedKeyStore) : CBase(), iProtectedKeyStore(aProtectedKeyStore)
{
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsSecurityModuleSymbian::~CAdvSecSettingsSecurityModuleSymbian()
// ---------------------------------------------------------------------------
//
CAdvSecSettingsSecurityModuleSymbian::~CAdvSecSettingsSecurityModuleSymbian()
{
    iKeyInfoArray.Close();
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsSecurityModuleSymbian::Label()
// ---------------------------------------------------------------------------
//
const TDesC &CAdvSecSettingsSecurityModuleSymbian::Label() const
{
    return KNullDesC;
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsSecurityModuleSymbian::IsDeletable()
// ---------------------------------------------------------------------------
//
TBool CAdvSecSettingsSecurityModuleSymbian::IsDeletable() const
{
    return EFalse;
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsSecurityModuleSymbian::IsSigningPinSupported()
// ---------------------------------------------------------------------------
//
TBool CAdvSecSettingsSecurityModuleSymbian::IsSigningPinSupported() const
{
    return EFalse;
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsSecurityModuleSymbian::KeyInfoArray()
// ---------------------------------------------------------------------------
//
RMPointerArray<CCTKeyInfo> &CAdvSecSettingsSecurityModuleSymbian::KeyInfoArray()
{
    return iKeyInfoArray;
}

