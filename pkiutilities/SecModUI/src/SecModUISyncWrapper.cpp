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


#include "SecModUISyncWrapper.h"
#include <ct/ccttokentypeinfo.h>
#include <ct/mcttokentype.h>
#include <ct/ccttokentype.h>
#include <ct/tcttokenobjecthandle.h>
#include <mctauthobject.h>
#include <unifiedkeystore.h>
#include "SecModUILogger.h"

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
    LOG_ENTERFN("CSecModUISyncWrapper::NewL");
    CSecModUISyncWrapper* wrap = CSecModUISyncWrapper::NewLC();
    CleanupStack::Pop(wrap);
    LOG_LEAVEFN("CSecModUISyncWrapper::NewL");
    return wrap;
    }

// Destructor
CSecModUISyncWrapper::~CSecModUISyncWrapper()
    {
    LOG_ENTERFN("CSecModUISyncWrapper::~CSecModUISyncWrapper");
    Cancel();
    iOperation = EOperationNone;
    LOG_LEAVEFN("CSecModUISyncWrapper::~CSecModUISyncWrapper");
    }

// -----------------------------------------------------------------------------
// CSecModUISyncWrapper::Initialize(CUnifiedKeyStore& aKeyStore)
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
TInt CSecModUISyncWrapper::Initialize(CUnifiedKeyStore& aKeyStore)
    {
    LOG_ENTERFN("CSecModUISyncWrapper::Initialize");
    if( !IsActive() && !iWait.IsStarted() )
        {
        iOperation = EOperationInit;
        iObject = STATIC_CAST(TAny*, &aKeyStore);
        aKeyStore.Initialize(iStatus);
        SetActive();
        iWait.Start();
        iOperation = EOperationNone;
        LOG_LEAVEFN("CSecModUISyncWrapper::Initialize");
        return iStatus.Int();
        }
    LOG_LEAVEFN("CSecModUISyncWrapper::Initialize - KErrInUse");
    return KErrInUse;
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::GetAuthObjectInterface(...)
// -----------------------------------------------------------------------------
//
TInt CSecModUISyncWrapper::GetAuthObjectInterface(
    MCTToken& aToken, MCTTokenInterface*& aTokenInterface)
    {
    LOG_ENTERFN("CSecModUISyncWrapper::GetAuthObjectInterface");
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
        LOG_LEAVEFN("CSecModUISyncWrapper::GetAuthObjectInterface");
        return iStatus.Int();
        }
    LOG_LEAVEFN("CSecModUISyncWrapper::GetAuthObjectInterface - KErrInUse");
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
    LOG_ENTERFN("CSecModUISyncWrapper::ListAuthObjects");
    if( !IsActive() && !iWait.IsStarted() )
        {
        iOperation = EOperationListAOs;
        iObject = STATIC_CAST(TAny*, &aAuthObjList);
        aAuthObjList.List( aAuthObjects, iStatus );
        iOperation = EOperationListAOs;
        SetActive();
        iWait.Start();
        iOperation = EOperationNone;
        LOG_LEAVEFN("CSecModUISyncWrapper::ListAuthObjects");
        return iStatus.Int();
        }
    LOG_LEAVEFN("CSecModUISyncWrapper::ListAuthObjects - KErrInUse");
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
    LOG_ENTERFN("CSecModUISyncWrapper::ListKeys");
    if( !IsActive() && !iWait.IsStarted() )
        {
        iOperation = EOperationListKeys;
        iObject = STATIC_CAST(TAny*, &aKeyStore);
        aKeyStore.List(aKeysInfos, aFilter, iStatus);
        SetActive();
        iWait.Start();
        iOperation = EOperationNone;
        LOG_LEAVEFN("CSecModUISyncWrapper::ListKeys");
        return iStatus.Int();
        }
    LOG_LEAVEFN("CSecModUISyncWrapper::ListKeys - KErrInUse");
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
    LOG_ENTERFN("CSecModUISyncWrapper::DeleteKey");
    if( !IsActive() && !iWait.IsStarted() )
        {
        iOperation = EOperationDelKey;
        iObject = STATIC_CAST(TAny*, &aKeyStore);
        aKeyStore.DeleteKey(aHandle, iStatus);
        SetActive();
        iWait.Start();
        iOperation = EOperationNone;
        LOG_LEAVEFN("CSecModUISyncWrapper::DeleteKey");
        return iStatus.Int();
        }
    LOG_LEAVEFN("CSecModUISyncWrapper::DeleteKey - KErrInUse");
    return KErrInUse;
    }

// -----------------------------------------------------------------------------
// CSecModUISyncWrapper::ChangeReferenceData(MCTAuthenticationObject& aAuthObject)
// -----------------------------------------------------------------------------
//
TInt CSecModUISyncWrapper::ChangeReferenceData(
    MCTAuthenticationObject& aAuthObject)
    {
    LOG_ENTERFN("CSecModUISyncWrapper::ChangeReferenceData");
    if( !IsActive() && !iWait.IsStarted() )
        {
        iOperation = EOperationChangeReferenceData;
        iObject = STATIC_CAST(TAny*, &aAuthObject);
        aAuthObject.ChangeReferenceData(iStatus);
        SetActive();
        iWait.Start();
        iOperation = EOperationNone;
        LOG_LEAVEFN("CSecModUISyncWrapper::ChangeReferenceData");
        return iStatus.Int();
        }
    LOG_LEAVEFN("CSecModUISyncWrapper::ChangeReferenceData - KErrInUse");
    return KErrInUse;
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::UnblockAuthObject(MCTAuthenticationObject& aAuthObject)
// -----------------------------------------------------------------------------
//
TInt CSecModUISyncWrapper::UnblockAuthObject(
    MCTAuthenticationObject& aAuthObject)
    {
    LOG_ENTERFN("CSecModUISyncWrapper::UnblockAuthObject");
    if( !IsActive() && !iWait.IsStarted() )
        {
        iOperation = EOperationUnblockAO;
        iObject = STATIC_CAST(TAny*, &aAuthObject);
        aAuthObject.Unblock(iStatus);
        SetActive();
        iWait.Start();
        iOperation = EOperationNone;
        LOG_LEAVEFN("CSecModUISyncWrapper::UnblockAuthObject");
        return iStatus.Int();
        }
    LOG_LEAVEFN("CSecModUISyncWrapper::UnblockAuthObject - KErrInUse");
    return KErrInUse;
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::EnableAuthObject(MCTAuthenticationObject& aAuthObject)
// -----------------------------------------------------------------------------
//
TInt CSecModUISyncWrapper::EnableAuthObject(
    MCTAuthenticationObject& aAuthObject)
    {
    LOG_ENTERFN("CSecModUISyncWrapper::EnableAuthObject");
    if( !IsActive() && !iWait.IsStarted() )
        {
        iOperation = EOperationEnableAO;
        iObject = STATIC_CAST(TAny*, &aAuthObject);
        aAuthObject.Enable(iStatus);
        iOperation = EOperationUnblockAO;
        SetActive();
        iWait.Start();
        iOperation = EOperationNone;
        LOG_LEAVEFN("CSecModUISyncWrapper::EnableAuthObject");
        return iStatus.Int();
        }
    LOG_LEAVEFN("CSecModUISyncWrapper::EnableAuthObject - KErrInUse");
    return KErrInUse;
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::DisableAuthObject(MCTAuthenticationObject& aAuthObject)
// -----------------------------------------------------------------------------
//
TInt CSecModUISyncWrapper::DisableAuthObject(
    MCTAuthenticationObject& aAuthObject)
    {
    LOG_ENTERFN("CSecModUISyncWrapper::DisableAuthObject");
    if( !IsActive() && !iWait.IsStarted() )
        {
        iOperation = EOperationDisableAO;
        iObject = STATIC_CAST(TAny*, &aAuthObject);
        aAuthObject.Disable(iStatus);
        SetActive();
        iWait.Start();
        iOperation = EOperationNone;
        LOG_LEAVEFN("CSecModUISyncWrapper::DisableAuthObject");
        return iStatus.Int();
        }
    LOG_LEAVEFN("CSecModUISyncWrapper::DisableAuthObject - KErrInUse");
    return KErrInUse;
    }

// -----------------------------------------------------------------------------
// CSecModUISyncWrapper::CloseAuthObject(MCTAuthenticationObject& aAuthObject)
// -----------------------------------------------------------------------------
//
TInt CSecModUISyncWrapper::CloseAuthObject(
    MCTAuthenticationObject& aAuthObject)
    {
    LOG_ENTERFN("CSecModUISyncWrapper::CloseAuthObject");
    if( !IsActive() && !iWait.IsStarted() )
        {
        iOperation = EOperationCloseAO;
        iObject = STATIC_CAST(TAny*, &aAuthObject);
        aAuthObject.Close(iStatus);
        SetActive();
        iWait.Start();
        iOperation = EOperationNone;
        LOG_LEAVEFN("CSecModUISyncWrapper::CloseAuthObject");
        return iStatus.Int();
        }
    LOG_LEAVEFN("SecModUISyncWrapper::CloseAuthObject - KErrInUse");
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
    LOG_ENTERFN("CSecModUISyncWrapper::TimeRemaining");
    if( !IsActive() && !iWait.IsStarted() )
        {
        iOperation = EOperationTimeRemAO;
        iObject = STATIC_CAST(TAny*, &aAuthObject);
        aAuthObject.TimeRemaining(aStime, iStatus);
        SetActive();
        iWait.Start();
        iOperation = EOperationNone;
        LOG_LEAVEFN("CSecModUISyncWrapper::TimeRemaining");
        return iStatus.Int();
        }
    LOG_LEAVEFN("CSecModUISyncWrapper::TimeRemaining - KErrInUse");
    return KErrInUse;
    }

// -----------------------------------------------------------------------------
// CSecModUISyncWrapper::DoCancel
// Cancels the ongoing operation if possible.
// -----------------------------------------------------------------------------
//
void CSecModUISyncWrapper::DoCancel()
    {
    LOG_ENTERFN("CSecModUISyncWrapper::DoCancel");
    switch ( iOperation )
        {
        case EOperationInit:
            {
            LOG_WRITE("CSecModUISyncWrapper::DoCancel: EOperationInit");
            STATIC_CAST(CUnifiedKeyStore*, iObject)->CancelInitialize();
            break;
            }
        case EOperationGetAOInterface:
            {
            LOG_WRITE("CSecModUISyncWrapper::DoCancel: EOperationGetAOInterface");
            STATIC_CAST(MCTToken*, iObject)->CancelGetInterface();
            break;
            }
        case EOperationListAOs:
            {
            LOG_WRITE("CSecModUISyncWrapper::DoCancel: EOperationListAOs");
            STATIC_CAST(MCTAuthenticationObjectList*, iObject)->CancelList();
            break;
            }
        case EOperationListKeys:
            {
            LOG_WRITE("CSecModUISyncWrapper::DoCancel: EOperationListKeys");
            STATIC_CAST(MCTKeyStore*, iObject)->CancelList();
            break;
            }
        case EOperationDelKey:
            {
            LOG_WRITE("CSecModUISyncWrapper::DoCancel: EOperationDelKey");
            STATIC_CAST(CUnifiedKeyStore*, iObject)->CancelDeleteKey();
            break;
            }
        case EOperationChangeReferenceData:
            {
            LOG_WRITE("CSecModUISyncWrapper::DoCancel: EOperationChangeReferenceData");
            STATIC_CAST(MCTAuthenticationObject*, iObject)->
                CancelChangeReferenceData();
            break;
            }
        case EOperationUnblockAO:
            {
            LOG_WRITE("CSecModUISyncWrapper::DoCancel: EOperationInit");
            STATIC_CAST(MCTAuthenticationObject*, iObject)->CancelUnblock();
            break;
            }
        case EOperationEnableAO:
            {
            LOG_WRITE("CSecModUISyncWrapper::DoCancel: EOperationEnableAO");
            STATIC_CAST(MCTAuthenticationObject*, iObject)->CancelEnable();
            break;
            }
        case EOperationDisableAO:
            {
            LOG_WRITE("CSecModUISyncWrapper::DoCancel: EOperationDisableAO");
            STATIC_CAST(MCTAuthenticationObject*, iObject)->CancelDisable();
            break;
            }
        case EOperationCloseAO:
            {
            LOG_WRITE("CSecModUISyncWrapper::DoCancel: EOperationCloseAO");
            STATIC_CAST(MCTAuthenticationObject*, iObject)->CancelClose();
            break;
            }
        case EOperationTimeRemAO:
            {
            LOG_WRITE("CSecModUISyncWrapper::DoCancel: EOperationTimeRemAO");
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
    LOG_LEAVEFN("CSecModUISyncWrapper::DoCancel");
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

