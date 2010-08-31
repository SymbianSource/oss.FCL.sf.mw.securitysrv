/*
* Copyright (c) 2005-2010 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:   Implementation of the CSecModUISyncWrapper class
*                Implements a synchronous wrapper for easier use of Symbian's
*                Security Frameworks's API's.
*
*/


#include "cpsecmodsyncwrapper.h"
#include <ct/ccttokentypeinfo.h>
#include <ct/mcttokentype.h>
#include <ct/ccttokentype.h>
#include <ct/tcttokenobjecthandle.h>
#include <mctauthobject.h>
#include <unifiedkeystore.h>
#include <mctkeystore.h>
#include <../../inc/cpsecplugins.h>


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CSecModUISyncWrapper::CSecModUISyncWrapper()
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CSecModUISyncWrapper::CSecModUISyncWrapper() : CActive( EPriorityStandard )
    {
    CActiveScheduler::Add(this);
    }


// -----------------------------------------------------------------------------
// CSecModUISyncWrapper::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSecModUISyncWrapper* CSecModUISyncWrapper::NewLC()
    {
    CSecModUISyncWrapper* wrap = new (ELeave) CSecModUISyncWrapper();
    CleanupStack::PushL(wrap);
    return wrap;
    }

// -----------------------------------------------------------------------------
// CSecModUISyncWrapper::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSecModUISyncWrapper* CSecModUISyncWrapper::NewL()
    {
    RDEBUG("0", 0);
    CSecModUISyncWrapper* wrap = CSecModUISyncWrapper::NewLC();
    CleanupStack::Pop(wrap);
    return wrap;
    }

// Destructor
CSecModUISyncWrapper::~CSecModUISyncWrapper()
    {
    Cancel();
    iOperation = EOperationNone;
    }

// -----------------------------------------------------------------------------
// CSecModUISyncWrapper::Initialize(CUnifiedKeyStore& aKeyStore)
// Two-phased constructor.
// -----------------------------------------------------------------------------
//

TInt CSecModUISyncWrapper::Initialize(CUnifiedKeyStore& aKeyStore)
    {
    if( !IsActive() && !iWait.IsStarted() )
        {
        iOperation = EOperationInit;
        iObject = STATIC_CAST(TAny*, &aKeyStore);
        aKeyStore.Initialize(iStatus);
        SetActive();
        iWait.Start();
        iOperation = EOperationNone;
        return iStatus.Int();
        }
    return KErrInUse;
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::GetAuthObjectInterface(...)
// -----------------------------------------------------------------------------
//
TInt CSecModUISyncWrapper::GetAuthObjectInterface(
    MCTToken& aToken, MCTTokenInterface*& aTokenInterface)
    {
    RDEBUG("0", 0);
    if( !IsActive() && !iWait.IsStarted() )
        {
        iOperation = EOperationGetAOInterface;
        iObject = STATIC_CAST(TAny*, &aToken);
        const TUid KUidInterfaceAO = { KCTInterfaceAuthenticationObject };
        aToken.GetInterface(KUidInterfaceAO, aTokenInterface, iStatus);
        iOperation = EOperationGetAOInterface;
        SetActive();
        iWait.Start();
        iOperation = EOperationNone;
        return iStatus.Int();
        }
    return KErrInUse;
    }

// -----------------------------------------------------------------------------
// CSecModUISyncWrapper::ListAuthObjects(...)
// -----------------------------------------------------------------------------
//
TInt CSecModUISyncWrapper::ListAuthObjects(
    MCTAuthenticationObjectList& aAuthObjList,
    RMPointerArray<MCTAuthenticationObject>& aAuthObjects)
    {
    if( !IsActive() && !iWait.IsStarted() )
        {
        iOperation = EOperationListAOs;
        iObject = STATIC_CAST(TAny*, &aAuthObjList);
        aAuthObjList.List( aAuthObjects, iStatus );
        iOperation = EOperationListAOs;
        SetActive();
        iWait.Start();
        iOperation = EOperationNone;
        return iStatus.Int();
        }
    return KErrInUse;
    }

// -----------------------------------------------------------------------------
// CSecModUISyncWrapper::ListKeys(...)
// -----------------------------------------------------------------------------
//
TInt CSecModUISyncWrapper::ListKeys(
    MCTKeyStore& aKeyStore,
    RMPointerArray<CCTKeyInfo>& aKeysInfos,
    const TCTKeyAttributeFilter& aFilter)
    {
    RDEBUG("0", 0);
    if( !IsActive() && !iWait.IsStarted() )
        {
        iOperation = EOperationListKeys;
        iObject = STATIC_CAST(TAny*, &aKeyStore);
        aKeyStore.List(aKeysInfos, aFilter, iStatus);
        SetActive();
        iWait.Start();
        iOperation = EOperationNone;
        return iStatus.Int();
        }
    return KErrInUse;
    }

// -----------------------------------------------------------------------------
// CSecModUISyncWrapper::DeleteKey(...)
// -----------------------------------------------------------------------------
//
TInt CSecModUISyncWrapper::DeleteKey(
    CUnifiedKeyStore& aKeyStore,
    TCTTokenObjectHandle aHandle)
    {
    RDEBUG("0", 0);
    if( !IsActive() && !iWait.IsStarted() )
        {
        iOperation = EOperationDelKey;
        iObject = STATIC_CAST(TAny*, &aKeyStore);
        aKeyStore.DeleteKey(aHandle, iStatus);
        SetActive();
        iWait.Start();
        iOperation = EOperationNone;
        return iStatus.Int();
        }
    return KErrInUse;
    }

// -----------------------------------------------------------------------------
// CSecModUISyncWrapper::ChangeReferenceData(MCTAuthenticationObject& aAuthObject)
// -----------------------------------------------------------------------------
//
TInt CSecModUISyncWrapper::ChangeReferenceData(
    MCTAuthenticationObject& aAuthObject)
    {
    RDEBUG("0", 0);
    if( !IsActive() && !iWait.IsStarted() )
        {
        iOperation = EOperationChangeReferenceData;
        iObject = STATIC_CAST(TAny*, &aAuthObject);
        aAuthObject.ChangeReferenceData(iStatus);
        SetActive();
        iWait.Start();
        iOperation = EOperationNone;
        return iStatus.Int();
        }
    return KErrInUse;
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::UnblockAuthObject(MCTAuthenticationObject& aAuthObject)
// -----------------------------------------------------------------------------
//
TInt CSecModUISyncWrapper::UnblockAuthObject(
    MCTAuthenticationObject& aAuthObject)
    {
    RDEBUG("0", 0);
    if( !IsActive() && !iWait.IsStarted() )
        {
        iOperation = EOperationUnblockAO;
        iObject = STATIC_CAST(TAny*, &aAuthObject);
        aAuthObject.Unblock(iStatus);
        SetActive();
        iWait.Start();
        iOperation = EOperationNone;
        return iStatus.Int();
        }
    return KErrInUse;
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::EnableAuthObject(MCTAuthenticationObject& aAuthObject)
// -----------------------------------------------------------------------------
//
TInt CSecModUISyncWrapper::EnableAuthObject(
    MCTAuthenticationObject& aAuthObject)
    {
    RDEBUG("0", 0);
    if( !IsActive() && !iWait.IsStarted() )
        {
        iOperation = EOperationEnableAO;
        iObject = STATIC_CAST(TAny*, &aAuthObject);
        aAuthObject.Enable(iStatus);
        iOperation = EOperationUnblockAO;
        SetActive();
        iWait.Start();
        iOperation = EOperationNone;
        return iStatus.Int();
        }
    return KErrInUse;
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::DisableAuthObject(MCTAuthenticationObject& aAuthObject)
// -----------------------------------------------------------------------------
//
TInt CSecModUISyncWrapper::DisableAuthObject(
    MCTAuthenticationObject& aAuthObject)
    {
    if( !IsActive() && !iWait.IsStarted() )
        {
        iOperation = EOperationDisableAO;
        iObject = STATIC_CAST(TAny*, &aAuthObject);
        aAuthObject.Disable(iStatus);
        SetActive();
        iWait.Start();
        iOperation = EOperationNone;
        return iStatus.Int();
        }
    return KErrInUse;
    }

// -----------------------------------------------------------------------------
// CSecModUISyncWrapper::CloseAuthObject(MCTAuthenticationObject& aAuthObject)
// -----------------------------------------------------------------------------
//
TInt CSecModUISyncWrapper::CloseAuthObject(
    MCTAuthenticationObject& aAuthObject)
    {
    if( !IsActive() && !iWait.IsStarted() )
        {
        iOperation = EOperationCloseAO;
        iObject = STATIC_CAST(TAny*, &aAuthObject);
        aAuthObject.Close(iStatus);
        SetActive();
        iWait.Start();
        iOperation = EOperationNone;
        return iStatus.Int();
        }
    return KErrInUse;
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::TimeRemaining(...)
// -----------------------------------------------------------------------------
//
TInt CSecModUISyncWrapper::TimeRemaining(
    MCTAuthenticationObject& aAuthObject,
    TInt& aStime )
    {
    RDEBUG("0", 0);
    if( !IsActive() && !iWait.IsStarted() )
        {
        iOperation = EOperationTimeRemAO;
        iObject = STATIC_CAST(TAny*, &aAuthObject);
        aAuthObject.TimeRemaining(aStime, iStatus);
        SetActive();
        iWait.Start();
        iOperation = EOperationNone;
        return iStatus.Int();
        }
    return KErrInUse;
    }

// -----------------------------------------------------------------------------
// CSecModUISyncWrapper::DoCancel
// Cancels the ongoing operation if possible.
// -----------------------------------------------------------------------------
//
void CSecModUISyncWrapper::DoCancel()
    {
    RDEBUG("iOperation", iOperation);
    switch ( iOperation )
        {
        case EOperationInit:
            {
            STATIC_CAST(CUnifiedKeyStore*, iObject)->CancelInitialize();
            break;
            }
        case EOperationGetAOInterface:
            {
            STATIC_CAST(MCTToken*, iObject)->CancelGetInterface();
            break;
            }
        case EOperationListAOs:
            {
            STATIC_CAST(MCTAuthenticationObjectList*, iObject)->CancelList();
            break;
            }
        case EOperationListKeys:
            {
            STATIC_CAST(MCTKeyStore*, iObject)->CancelList();
            break;
            }
        case EOperationDelKey:
            {
            STATIC_CAST(CUnifiedKeyStore*, iObject)->CancelDeleteKey();
            break;
            }
        case EOperationChangeReferenceData:
            {
            STATIC_CAST(MCTAuthenticationObject*, iObject)->
                CancelChangeReferenceData();
            break;
            }
        case EOperationUnblockAO:
            {
            STATIC_CAST(MCTAuthenticationObject*, iObject)->CancelUnblock();
            break;
            }
        case EOperationEnableAO:
            {
            STATIC_CAST(MCTAuthenticationObject*, iObject)->CancelEnable();
            break;
            }
        case EOperationDisableAO:
            {
            STATIC_CAST(MCTAuthenticationObject*, iObject)->CancelDisable();
            break;
            }
        case EOperationCloseAO:
            {
            STATIC_CAST(MCTAuthenticationObject*, iObject)->CancelClose();
            break;
            }
        case EOperationTimeRemAO:
            {
            STATIC_CAST(MCTAuthenticationObject*, iObject)->CancelTimeRemaining();
            break;
            }
        default:
            {
            break;
            }
        }
    if (iWait.IsStarted())
        {
        iWait.AsyncStop();
        }
    }

// -----------------------------------------------------------------------------
// CSecModUISyncWrapper::RunL
// If no errors happened, stop. Show an error note if needed.
// -----------------------------------------------------------------------------
//
void CSecModUISyncWrapper::RunL()
    {
    iWait.AsyncStop();
    }

