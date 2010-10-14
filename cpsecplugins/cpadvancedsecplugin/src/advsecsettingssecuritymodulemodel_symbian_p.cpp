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
* Description:  Symbian specific private security module model
*
*/

#include "advsecsettingssecuritymodulemodel_symbian_p.h"
#include "advsecsettingssecuritymodulemodel.h"
#include "advsecsettingsstoreuids.h"
#include "advsecsettingssecuritymoduleeraser_symbian.h"
#include <unifiedkeystore.h>            // CUnifiedKeyStore
#include <mctauthobject.h>              // MCTAuthenticationObject
#include <QDebug>

// TODO: split implementation into smaller parts, use CAdvSecSettingsSecurityModuleSymbian

// Local functions in other cpp-files within this project
QString CopyStringL(const TDesC16 &aDes16);
QString Location(const TUid &aTokenType);

// TODO: replace with proper logging
#ifdef _DEBUG
#define TRACE(x)        RDebug::Printf(x)
#define TRACE1(x,y)     RDebug::Printf((x),(y))
#define TRACE2(x,y,z)   RDebug::Printf((x),(y),(z))
#else
#define TRACE(x)
#define TRACE1(x,y)
#define TRACE2(x,y,z)
#endif


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::AdvSecSettingsSecurityModuleModelPrivate()
// ---------------------------------------------------------------------------
//
AdvSecSettingsSecurityModuleModelPrivate::AdvSecSettingsSecurityModuleModelPrivate(
    AdvSecSettingsSecurityModuleModel *q) : CActive(CActive::EPriorityLow), q_ptr(q),
    iState(ENotInitialized)
{
    TRACE("AdvSecSettingsSecurityModuleModelPrivate::AdvSecSettingsSecurityModuleModelPrivate");
    CActiveScheduler::Add(this);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::~AdvSecSettingsSecurityModuleModelPrivate()
// ---------------------------------------------------------------------------
//
AdvSecSettingsSecurityModuleModelPrivate::~AdvSecSettingsSecurityModuleModelPrivate()
{
    TRACE("AdvSecSettingsSecurityModuleModelPrivate::~AdvSecSettingsSecurityModuleModelPrivate");
    Cancel();
    delete iModuleEraser;
    iCurrentKeyStore = NULL;
    iCurrentAuthObject = NULL;
    iProtectedKeyStores.Reset();
    iAuthenticationObjects.Reset();
    iKeys.Close();
    delete iAllKeysFilter;
    delete iUnifiedKeyStore;
    iFs.Close();
    q_ptr = NULL;
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::initialize()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModelPrivate::initialize()
{
    TRACE("AdvSecSettingsSecurityModuleModelPrivate::initialize");
    if ((iState == ENotInitialized) && !IsActive()) {
        TRAPD(err, ConstructL());
        if (err == KErrNone) {
            iUnifiedKeyStore->Initialize(iStatus);
            iState = EInitializing;
            SetActive();
        } else {
            q_ptr->handleError(err);
        }
    } else {
        q_ptr->handleError(KErrAlreadyExists);
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::moduleCount()
// ---------------------------------------------------------------------------
//
int AdvSecSettingsSecurityModuleModelPrivate::moduleCount() const
{
    TRACE("AdvSecSettingsSecurityModuleModelPrivate::moduleCount");
    return iProtectedKeyStores.Count();
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::moduleLabelsAndLocations()
// ---------------------------------------------------------------------------
//
QMap<QString,QString> AdvSecSettingsSecurityModuleModelPrivate::moduleLabelsAndLocations() const
{
    TRACE("AdvSecSettingsSecurityModuleModelPrivate::moduleLabelsAndLocations");
    QMap<QString,QString> map;

    for (TInt index = 0; index < iProtectedKeyStores.Count(); index++) {
        MCTToken &keyStoreToken = iProtectedKeyStores[index]->Token();

        QString label;
        if (keyStoreToken.TokenType().Type().iUid == KAdvSecSettingsFileKeyStore) {
            // TODO: localized UI string needed
            label = CopyStringL(_L("Phone key store"));
        } else {
            label = CopyStringL(keyStoreToken.Label());
        }

        map[label] = Location(keyStoreToken.TokenType().Type());
    }

    return map;
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::getModuleStatus()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModelPrivate::getModuleStatus(int moduleIndex)
{
    TRACE1("AdvSecSettingsSecurityModuleModelPrivate::getModuleStatus, moduleIndex=%d", moduleIndex);
    if ((iState == EIdle) && !IsActive()) {
        if (moduleIndex >= 0 && moduleIndex < iAuthenticationObjects.Count()) {
            // TODO: this does not work yet
#if 0
            iCurrentAuthObject = iAuthenticationObjects[moduleIndex];
            iTimeRemaining = 0;
            iCurrentAuthObject->TimeRemaining(iTimeRemaining, iStatus);
            iState = EReadingTimeRemaining;
            SetActive();
#else
            TInt tempStatus = AdvSecSettingsSecurityModuleModel::EPinRequested;
            q_ptr->handleStatusCompleted(tempStatus);
#endif
        } else {
            q_ptr->handleError(KErrArgument);
        }
    } else {
        q_ptr->handleError(KErrInUse);
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::setPinCodeRequestState()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModelPrivate::setPinCodeRequestState(int moduleIndex,
    bool isRequested)
{
    TRACE2("AdvSecSettingsSecurityModuleModelPrivate::setPinCodeRequestState, moduleIndex=%d isRequested=%d",
        moduleIndex, isRequested);
    if ((iState == EIdle) && !IsActive()) {
        if (moduleIndex >= 0 && moduleIndex < iAuthenticationObjects.Count()) {
            iCurrentAuthObject = iAuthenticationObjects[moduleIndex];
            if (isRequested) {
                iCurrentAuthObject->Enable(iStatus);
                iState = EEnablingPinCodeRequest;
            } else {
                iCurrentAuthObject->Disable(iStatus);
                iState = EDisablingPinCodeRequest;
            }
            SetActive();
        } else {
            q_ptr->handleError(KErrArgument);
        }
    } else {
        q_ptr->handleError(KErrInUse);
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::changePinCode()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModelPrivate::changePinCode(int moduleIndex)
{
    TRACE1("AdvSecSettingsSecurityModuleModelPrivate::changePinCode, moduleIndex=%d", moduleIndex);
    if ((iState == EIdle) && !IsActive()) {
        if (moduleIndex >= 0 && moduleIndex < iAuthenticationObjects.Count()) {
            iCurrentAuthObject = iAuthenticationObjects[moduleIndex];
            ChangeCurrentAuthObjectPinCode();
        } else {
            q_ptr->handleError(KErrArgument);
        }
    } else {
        q_ptr->handleError(KErrInUse);
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::closeModule()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModelPrivate::closeModule(int moduleIndex)
{
    TRACE1("AdvSecSettingsSecurityModuleModelPrivate::closeModule, moduleIndex=%d", moduleIndex);
    if ((iState == EIdle) && !IsActive()) {
        if (moduleIndex >= 0 && moduleIndex < iAuthenticationObjects.Count()) {
            iCurrentAuthObject = iAuthenticationObjects[moduleIndex];
            iCurrentAuthObject->Close(iStatus);
            iState = EClosingAuthObject;
            SetActive();
        } else {
            q_ptr->handleError(KErrArgument);
        }
    } else {
        q_ptr->handleError(KErrInUse);
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::isSigningPinSupported()
// ---------------------------------------------------------------------------
//
bool AdvSecSettingsSecurityModuleModelPrivate::isSigningPinSupported(int moduleIndex) const
{
    TRACE1("AdvSecSettingsSecurityModuleModelPrivate::isSigningPinSupported, moduleIndex=%d",
        moduleIndex);
    if (moduleIndex >= 0 && moduleIndex < iProtectedKeyStores.Count()) {
        MCTToken &keyStoreToken = iProtectedKeyStores[moduleIndex]->Token();
        if (keyStoreToken.TokenType().Type().iUid == KAdvSecSettingsFileKeyStore) {
            return false;
        } else {
            return true;
        }
    } else {
        return false;
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::changeSigningPinCode()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModelPrivate::changeSigningPinCode(int /*moduleIndex*/)
{
    TRACE("AdvSecSettingsSecurityModuleModelPrivate::changeSigningPinCode" );

    // TODO: implement
    q_ptr->handleError(KErrNotSupported);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::isDeletable()
// ---------------------------------------------------------------------------
//
bool AdvSecSettingsSecurityModuleModelPrivate::isDeletable(int moduleIndex) const
{
    TRACE1("AdvSecSettingsSecurityModuleModelPrivate::isDeletable, moduleIndex=%d",
        moduleIndex);
    if (moduleIndex >= 0 && moduleIndex < iProtectedKeyStores.Count()) {
        MCTToken &keyStoreToken = iProtectedKeyStores[moduleIndex]->Token();
        if (keyStoreToken.TokenType().Type().iUid == KAdvSecSettingsFileKeyStore) {
            TRACE("AdvSecSettingsSecurityModuleModelPrivate::isDeletable, true");
            return true;
        }
    }
    TRACE("AdvSecSettingsSecurityModuleModelPrivate::isDeletable, false");
    return false;
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::deleteModule()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModelPrivate::deleteModule(int moduleIndex)
{
    TRACE1("AdvSecSettingsSecurityModuleModelPrivate::deleteModule, moduleIndex=%d",
        moduleIndex);
    if (moduleIndex >= 0 && moduleIndex < iProtectedKeyStores.Count()) {
        MCTToken &keyStoreToken = iProtectedKeyStores[moduleIndex]->Token();
        if (keyStoreToken.TokenType().Type().iUid == KAdvSecSettingsFileKeyStore) {
            TInt err = DoStartDeletingModule(moduleIndex);
            if (err) {
                q_ptr->handleError(err);
            }
        }
    } else {
        q_ptr->handleError(KErrArgument);
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::DoCancel()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModelPrivate::DoCancel()
{
    TRACE("AdvSecSettingsSecurityModuleModelPrivate::DoCancel");
    switch (iState) {
    case EInitializing:
        iUnifiedKeyStore->CancelInitialize();
        iState = ENotInitialized;
        break;
    case EListingKeys:
        iCurrentKeyStore->CancelList();
        iState = ENotInitialized;
        break;
    case EReadingTimeRemaining:
        iCurrentAuthObject->CancelTimeRemaining();
        iState = EIdle;
        break;
    case EChangingPinCode:
        iCurrentAuthObject->CancelChangeReferenceData();
        iState = EIdle;
        break;
    case EUnblockingToChangePinCode:
        iCurrentAuthObject->CancelUnblock();
        iState = EIdle;
        break;
    case EEnablingPinCodeRequest:
        iCurrentAuthObject->CancelEnable();
        iState = EIdle;
        break;
    case EDisablingPinCodeRequest:
        iCurrentAuthObject->CancelDisable();
        iState = EIdle;
        break;
    case EClosingAuthObject:
        iCurrentAuthObject->CancelClose();
        iState = EIdle;
        break;
    case EDeletingModule:
        delete iModuleEraser;
        iModuleEraser = NULL;
        break;
    default:
        break;
    }
    q_ptr->handleError(KErrCancel);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::RunL()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModelPrivate::RunL()
{
    TRACE2("AdvSecSettingsSecurityModuleModelPrivate::RunL, status=%d state=%d", iStatus.Int(), iState);
    User::LeaveIfError(iStatus.Int());
    switch (iState) {
    case EInitializing:
        TRACE("AdvSecSettingsSecurityModuleModelPrivate::RunL, EInitializing");
        iAuthenticationObjects.Reset();
        iProtectedKeyStores.Reset();
        ListKeysFromFirstKeyStoreL();
        break;
    case EListingKeys:
        TRACE("AdvSecSettingsSecurityModuleModelPrivate::RunL, EListingKeys");
        AppendKeyStoresWithProtectedKeysL();
        ListKeysFromNextKeyStoreL();
        break;
    case EReadingTimeRemaining:
        TRACE("AdvSecSettingsSecurityModuleModelPrivate::RunL, EReadingTimeRemaining");
        ReturnModuleStatusL();
        break;
    case EChangingPinCode:
        TRACE("AdvSecSettingsSecurityModuleModelPrivate::RunL, EChangingPinCode");
        ReturnPinChanged();
        break;
    case EUnblockingToChangePinCode:
        TRACE("AdvSecSettingsSecurityModuleModelPrivate::RunL, EUnblockingToChangePinCode");
        ChangeCurrentAuthObjectPinCode();
        break;
    case EEnablingPinCodeRequest:
    case EDisablingPinCodeRequest:
        TRACE("AdvSecSettingsSecurityModuleModelPrivate::RunL, EEnabling/DisablingPinCodeRequest");
        ReturnPinRequestStateChanged();
        break;
    case EClosingAuthObject:
        TRACE("AdvSecSettingsSecurityModuleModelPrivate::RunL, EClosingAuthObject");
        ReturnModuleClosed();
        break;
    case EDeletingModule:
        TRACE("AdvSecSettingsSecurityModuleModelPrivate::RunL, EDeletingModule");
        ReturnModuleDeleted();
        break;
    default:
        TRACE("AdvSecSettingsSecurityModuleModelPrivate::RunL, default");
        ASSERT(EFalse);
        break;
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::RunError()
// ---------------------------------------------------------------------------
//
TInt AdvSecSettingsSecurityModuleModelPrivate::RunError(TInt aError)
{
    TRACE1("AdvSecSettingsSecurityModuleModelPrivate::RunError, aError=%d", aError);
    q_ptr->handleError(aError);
    return KErrNone;
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::ConstructL()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModelPrivate::ConstructL()
{
    TRACE("AdvSecSettingsSecurityModuleModelPrivate::ConstructL");
    User::LeaveIfError(iFs.Connect());
    iUnifiedKeyStore = CUnifiedKeyStore::NewL(iFs);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::ListKeysFromFirstKeyStoreL()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModelPrivate::ListKeysFromFirstKeyStoreL()
{
    TRACE1("AdvSecSettingsSecurityModuleModelPrivate::ListKeysFromFirstKeyStoreL, count=%d",
        iUnifiedKeyStore->KeyStoreCount());
    iKeyStoreIndex = 0;
    ListKeysFromOneKeyStoreL();
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::ListKeysFromOneKeyStoreL()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModelPrivate::ListKeysFromOneKeyStoreL()
{
    TRACE1("AdvSecSettingsSecurityModuleModelPrivate::ListKeysFromOneKeyStoreL, index=%d",
        iKeyStoreIndex);
    if (iKeyStoreIndex < iUnifiedKeyStore->KeyStoreCount()) {
        iKeys.Close();
        if (!iAllKeysFilter) {
            iAllKeysFilter = new( ELeave ) TCTKeyAttributeFilter;
            iAllKeysFilter->iPolicyFilter = TCTKeyAttributeFilter::EAllKeys;
        }
        iCurrentKeyStore = &(iUnifiedKeyStore->KeyStore(iKeyStoreIndex));
        iCurrentKeyStore->List(iKeys, *iAllKeysFilter, iStatus);
        iState = EListingKeys;
        SetActive();
    } else {
        iState = EIdle;
        q_ptr->handleInitializeCompleted();
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::ListKeysFromNextKeyStoreL()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModelPrivate::ListKeysFromNextKeyStoreL()
{
    TRACE("AdvSecSettingsSecurityModuleModelPrivate::ListKeysFromNextKeyStoreL");
    ++iKeyStoreIndex;
    ListKeysFromOneKeyStoreL();
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::AppendKeyStoresWithProtectedKeysL()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModelPrivate::AppendKeyStoresWithProtectedKeysL()
{
    TRACE("AdvSecSettingsSecurityModuleModelPrivate::AppendKeyStoresWithProtectedKeysL");
    for (TInt index = 0; index < iKeys.Count(); index++) {
        const CCTKeyInfo &keyInfo = *(iKeys[index]);
        if (keyInfo.Protector()) {
            TRACE("AdvSecSettingsSecurityModuleModelPrivate::AppendKeyStoresWithProtectedKeysL, added");
            iAuthenticationObjects.AppendL(keyInfo.Protector());
            iProtectedKeyStores.AppendL(&(iUnifiedKeyStore->KeyStore(iKeyStoreIndex)));
        }
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::ChangeCurrentAuthObjectPinCode()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModelPrivate::ChangeCurrentAuthObjectPinCode()
{
    TRACE("AdvSecSettingsSecurityModuleModelPrivate::ChangeCurrentAuthObjectPinCode");
    TInt32 currentAuthObjectStatus = iCurrentAuthObject->Status();
    if (currentAuthObjectStatus & EAuthObjectBlocked) {
        TRACE("AdvSecSettingsSecurityModuleModelPrivate::ChangeCurrentAuthObjectPinCode, blocked");
        if (currentAuthObjectStatus & EUnblockDisabled) {
            TRACE("AdvSecSettingsSecurityModuleModelPrivate::ChangeCurrentAuthObjectPinCode, permanently");
            iState = EIdle;
            q_ptr->handleError(KErrPermissionDenied);
        } else {
            TRACE("AdvSecSettingsSecurityModuleModelPrivate::ChangeCurrentAuthObjectPinCode, unblocking");
            iCurrentAuthObject->Unblock(iStatus);
            iState = EUnblockingToChangePinCode;
            SetActive();
        }
    } else {
        if (currentAuthObjectStatus & EChangeDisabled) {
            TRACE("AdvSecSettingsSecurityModuleModelPrivate::ChangeCurrentAuthObjectPinCode, cannot change");
            iState = EIdle;
            q_ptr->handleError(KErrPermissionDenied);
        } else {
            TRACE("AdvSecSettingsSecurityModuleModelPrivate::ChangeCurrentAuthObjectPinCode, changing pin");
            iCurrentAuthObject->ChangeReferenceData(iStatus);
            iState = EChangingPinCode;
            SetActive();
        }
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::DoStartDeletingModule()
// ---------------------------------------------------------------------------
//
TInt AdvSecSettingsSecurityModuleModelPrivate::DoStartDeletingModule(TInt aModuleIndex)
{
    if (!iModuleEraser) {
        TRAPD(err, iModuleEraser = CAdvSecSettingsSecurityModuleEraser::NewL(*iUnifiedKeyStore));
        if (err) {
            return err;
        }
    }
    iModuleEraser->Erase(aModuleIndex, iStatus);
    iState = EDeletingModule;
    SetActive();
    return KErrNone;
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::ReturnModuleStatusL()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModelPrivate::ReturnModuleStatusL()
{
    int advSecSettingsStatus = 0;

    TUint32 currentTCTAuthenticationStatus = iCurrentAuthObject->Status();
    if (currentTCTAuthenticationStatus & EUnblockDisabled) {
        advSecSettingsStatus |= AdvSecSettingsSecurityModuleModel::EBlockedPermanently;
    }
    if (currentTCTAuthenticationStatus & EAuthObjectBlocked) {
        advSecSettingsStatus |= AdvSecSettingsSecurityModuleModel::EPinBlocked;
    }
    advSecSettingsStatus |= AdvSecSettingsSecurityModuleModel::EPinChangeAllowed;
    if (currentTCTAuthenticationStatus & EChangeDisabled) {
        advSecSettingsStatus ^= AdvSecSettingsSecurityModuleModel::EPinChangeAllowed;
    }
    if (currentTCTAuthenticationStatus & EEnabled) {
        advSecSettingsStatus |= AdvSecSettingsSecurityModuleModel::EPinRequested;
    }
    if (iTimeRemaining > 0) {
        advSecSettingsStatus |= AdvSecSettingsSecurityModuleModel::EPinEntered;
    }

    TRACE1("AdvSecSettingsSecurityModuleModelPrivate::ReturnModuleStatusL 0x%08x",
        advSecSettingsStatus);
    iState = EIdle;
    q_ptr->handleStatusCompleted(advSecSettingsStatus);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::ReturnPinChanged()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModelPrivate::ReturnPinChanged()
{
    TRACE("AdvSecSettingsSecurityModuleModelPrivate::ReturnPinChanged");
    iState = EIdle;
    q_ptr->handlePinCodeChanged();
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::ReturnPinRequestStateChanged()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModelPrivate::ReturnPinRequestStateChanged()
{
    TRACE("AdvSecSettingsSecurityModuleModelPrivate::ReturnPinRequestStateChanged");
    iState = EIdle;
    q_ptr->handlePinCodeRequestSet();
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::ReturnModuleClosed()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModelPrivate::ReturnModuleClosed()
{
    TRACE("AdvSecSettingsSecurityModuleModelPrivate::ReturnModuleClosed");
    iState = EIdle;
    q_ptr->handleModuleClosed();
}

// ---------------------------------------------------------------------------
// AdvSecSettingsSecurityModuleModelPrivate::ReturnModuleDeleted()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsSecurityModuleModelPrivate::ReturnModuleDeleted()
{
    TRACE("AdvSecSettingsSecurityModuleModelPrivate::ReturnModuleDeleted");
    iState = EIdle;
    q_ptr->handleModuleDeleted();
}

