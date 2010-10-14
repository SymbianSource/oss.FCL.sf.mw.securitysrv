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

#ifndef ADVSECSETTINGSSECURITYMODULEERASER_SYMBIAN_H
#define ADVSECSETTINGSSECURITYMODULEERASER_SYMBIAN_H

#include <e32base.h>                    // CActive
#include <ct/rmpointerarray.h>          // RMPointerArray

class CUnifiedKeyStore;
class CCTKeyInfo;
class TCTKeyAttributeFilter;
class MCTKeyStore;


/**
 * Security module removing helper class. This class deletes the security
 * module by deleting all the keys controlled by the module.
 */
class CAdvSecSettingsSecurityModuleEraser : public CActive
{
public:     // constructor and destructor
    static CAdvSecSettingsSecurityModuleEraser *NewL(CUnifiedKeyStore &aKeyStore);
    ~CAdvSecSettingsSecurityModuleEraser();

public:     // new functions
    void Erase(TInt aKeyStoreIndex, TRequestStatus &aStatus);

protected:  // from CActive
    void DoCancel();
    void RunL();
    TInt RunError(TInt aError);

private:    // new functions
    CAdvSecSettingsSecurityModuleEraser(CUnifiedKeyStore &aKeyStore);
    void ConstructL();
    void DeleteFirstKeyL();
    void DeleteOneKeyL();
    void DeleteNextKeyL();

private:    // data
    TRequestStatus *iClientStatus;  // not owned
    CUnifiedKeyStore &iKeyStore;
    MCTKeyStore *iTargetKeyStore;  // not owned
    TCTKeyAttributeFilter *iKeyFilter;
    RMPointerArray<CCTKeyInfo> iKeys;
    TInt iKeyIndex;

    enum TEraserState {
        EIdle,
        EListingKeys,
        EDeletingKey
    } iState;
};

#endif // ADVSECSETTINGSSECURITYMODULEERASER_SYMBIAN_H
