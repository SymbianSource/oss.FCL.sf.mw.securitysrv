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

#ifndef ADVSECSETTINGSSECURITYMODULEMODELPRIVATE_SYMBIAN_H
#define ADVSECSETTINGSSECURITYMODULEMODELPRIVATE_SYMBIAN_H

#include <e32base.h>                    // CActive
#include <f32file.h>                    // RFs
#include <ct/rmpointerarray.h>          // RMPointerArray
#include <QMap>

class AdvSecSettingsSecurityModuleModel;
class CAdvSecSettingsSecurityModuleEraser;
class CUnifiedKeyStore;
class MCTKeyStore;
class MCTAuthenticationObject;
class CCTKeyInfo;
class TCTKeyAttributeFilter;


/**
 * Symbian specific private security module model.
 */
class AdvSecSettingsSecurityModuleModelPrivate : public CActive
{
public:     // constructor and destructor
    explicit AdvSecSettingsSecurityModuleModelPrivate(AdvSecSettingsSecurityModuleModel *q);
    ~AdvSecSettingsSecurityModuleModelPrivate();

public:     // new functions
    void initialize();
    int moduleCount() const;
    QMap<QString,QString> moduleLabelsAndLocations() const;
    void getModuleStatus(int moduleIndex);
    void setPinCodeRequestState(int moduleIndex, bool isRequested);
    void changePinCode(int moduleIndex);  // unblocks PIN code if EPinBlocked
    void closeModule(int moduleIndex);
    bool isSigningPinSupported(int moduleIndex) const;
    void changeSigningPinCode(int moduleIndex);
    bool isDeletable(int moduleIndex) const;
    void deleteModule(int moduleIndex);

protected:  // from CActive
    void DoCancel();
    void RunL();
    TInt RunError(TInt aError);

private:    // new functions
    void ConstructL();
    void ListKeysFromFirstKeyStoreL();
    void ListKeysFromOneKeyStoreL();
    void ListKeysFromNextKeyStoreL();
    void AppendKeyStoresWithProtectedKeysL();
    void ChangeCurrentAuthObjectPinCode();
    TInt DoStartDeletingModule(TInt aModuleIndex);
    void ReturnModuleStatusL();
    void ReturnPinChanged();
    void ReturnPinRequestStateChanged();
    void ReturnModuleClosed();
    void ReturnModuleDeleted();

private:    // data
    AdvSecSettingsSecurityModuleModel *q_ptr;  // not owned
    RFs iFs;
    CUnifiedKeyStore *iUnifiedKeyStore;
    RMPointerArray<MCTKeyStore> iProtectedKeyStores;  // items not owned
    RMPointerArray<MCTAuthenticationObject> iAuthenticationObjects;  // items not owned
    TInt iKeyStoreIndex;
    RMPointerArray<CCTKeyInfo> iKeys;
    TCTKeyAttributeFilter *iAllKeysFilter;
    MCTKeyStore *iCurrentKeyStore;  // not owned
    MCTAuthenticationObject *iCurrentAuthObject;  // not owned
    TInt iTimeRemaining;
    CAdvSecSettingsSecurityModuleEraser *iModuleEraser;
    enum TState {
        ENotInitialized,
        EInitializing,
        EListingKeys,
        EReadingTimeRemaining,
        EChangingPinCode,
        EUnblockingToChangePinCode,
        EEnablingPinCodeRequest,
        EDisablingPinCodeRequest,
        EClosingAuthObject,
        EDeletingModule,
        EIdle
    } iState;
};

#endif // ADVSECSETTINGSSECURITYMODULEMODELPRIVATE_SYMBIAN_H
