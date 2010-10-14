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
* Description:  Helper class to remove a security module
*
*/


#include "advsecsettingssecuritymoduleeraser_symbian.h"
#include <unifiedkeystore.h>            // CUnifiedKeyStore
#include <ct/tcttokenobjecthandle.h>    // TCTTokenObjectHandle


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CAdvSecSettingsSecurityModuleEraser::NewL()
// ---------------------------------------------------------------------------
//
CAdvSecSettingsSecurityModuleEraser *CAdvSecSettingsSecurityModuleEraser::NewL(
    CUnifiedKeyStore &aKeyStore)
{
    CAdvSecSettingsSecurityModuleEraser *self = new(ELeave) CAdvSecSettingsSecurityModuleEraser(
        aKeyStore);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsSecurityModuleEraser::~CAdvSecSettingsSecurityModuleEraser()
// ---------------------------------------------------------------------------
//
CAdvSecSettingsSecurityModuleEraser::~CAdvSecSettingsSecurityModuleEraser()
{
    iKeys.Close();
    delete iKeyFilter;
    iTargetKeyStore = NULL;
    iClientStatus = NULL;
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsSecurityModuleEraser::Erase()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsSecurityModuleEraser::Erase(TInt aKeyStoreIndex, TRequestStatus &aStatus)
{
    aStatus = KRequestPending;
    if (!IsActive() && (iState == EIdle)) {
        if (aKeyStoreIndex >= 0 && aKeyStoreIndex < iKeyStore.KeyStoreCount()) {
            iTargetKeyStore = &(iKeyStore.KeyStore(aKeyStoreIndex));
            iClientStatus = &aStatus;

            iKeys.Reset();
            iTargetKeyStore->List(iKeys, *iKeyFilter, iStatus);
            iState = EListingKeys;
            SetActive();
        } else {
            TRequestStatus *status = &aStatus;
            User::RequestComplete(status, KErrArgument);
        }
    } else {
        TRequestStatus *status = &aStatus;
        User::RequestComplete(status, KErrInUse);
    }
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsSecurityModuleEraser::DoCancel()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsSecurityModuleEraser::DoCancel()
{
    switch (iState) {
    case EListingKeys:
        iTargetKeyStore->CancelList();
        break;
    case EDeletingKey:
        iKeyStore.CancelDeleteKey();
        break;
    default:
        break;
    }
    iState = EIdle;

    User::RequestComplete(iClientStatus, KErrCancel);
    iClientStatus = NULL;
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsSecurityModuleEraser::RunL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsSecurityModuleEraser::RunL()
{
    switch (iState) {
    case EListingKeys:
        DeleteFirstKeyL();
        break;
    case EDeletingKey:
        DeleteNextKeyL();
        break;
    default:
        ASSERT(EFalse);
        break;
    }
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsSecurityModuleEraser::RunError()
// ---------------------------------------------------------------------------
//
TInt CAdvSecSettingsSecurityModuleEraser::RunError(TInt aError)
{
    User::RequestComplete(iClientStatus, aError);
    iClientStatus = NULL;
    iState = EIdle;
    return KErrNone;
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsSecurityModuleEraser::CAdvSecSettingsSecurityModuleEraser()
// ---------------------------------------------------------------------------
//
CAdvSecSettingsSecurityModuleEraser::CAdvSecSettingsSecurityModuleEraser(
    CUnifiedKeyStore &aKeyStore) : CActive(CActive::EPriorityStandard),
    iKeyStore(aKeyStore)
{
    CActiveScheduler::Add(this);
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsSecurityModuleEraser::ConstructL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsSecurityModuleEraser::ConstructL()
{
    iKeyFilter = new(ELeave) TCTKeyAttributeFilter;
    iKeyFilter->iPolicyFilter = TCTKeyAttributeFilter::EAllKeys;
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsSecurityModuleEraser::DeleteFirstKeyL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsSecurityModuleEraser::DeleteFirstKeyL()
{
    iKeyIndex = 0;
    DeleteOneKeyL();
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsSecurityModuleEraser::DeleteOneKeyL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsSecurityModuleEraser::DeleteOneKeyL()
{
    if (iKeyIndex < iKeys.Count()) {
        iKeyStore.DeleteKey(iKeys[iKeyIndex]->Handle(), iStatus);
        iState = EDeletingKey;
        SetActive();
    } else {
        User::RequestComplete(iClientStatus, KErrNone);
        iClientStatus = NULL;
        iState = EIdle;
    }
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsSecurityModuleEraser::DeleteNextKeyL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsSecurityModuleEraser::DeleteNextKeyL()
{
    ++iKeyIndex;
    DeleteOneKeyL();
}

