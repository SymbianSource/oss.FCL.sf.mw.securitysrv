/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Implementation of DevCertKeyStoreSession
*
*/



#include "DevCertKeyStoreSession.h"
#include "DevCertKeyStoreServer.h"
#include "DevCertOpenedKeysSrv.h"


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CDevCertKeyStoreSession::NewL()
// ---------------------------------------------------------------------------
// 
CDevCertKeyStoreSession* CDevCertKeyStoreSession::NewL( CDevCertKeyStoreServer& aServer )
    {
    return new (ELeave) CDevCertKeyStoreSession( aServer );
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreSession::CDevCertKeyStoreSession()
// ---------------------------------------------------------------------------
// 
CDevCertKeyStoreSession::CDevCertKeyStoreSession(CDevCertKeyStoreServer& aServer )
  : iServer(aServer)
    {
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreSession::~CDevCertKeyStoreSession()
// ---------------------------------------------------------------------------
//
CDevCertKeyStoreSession::~CDevCertKeyStoreSession()
    {
    iServer.RemoveSession(*this);

    for (TInt i = 0 ; i < iOpenedKeys.Count() ; ++i)
        {
        CDevCertOpenedKeySrv* object = iOpenedKeys[i].iObject;
        delete object;
        }
    iOpenedKeys.Close();
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreSession::DoServiceL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreSession::DoServiceL(const RMessage2& aMessage)
    {
    iServer.ServiceRequestL(aMessage, *this);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreSession::AddOpenedKeyL()
// ---------------------------------------------------------------------------
//
TInt CDevCertKeyStoreSession::AddOpenedKeyL(CDevCertOpenedKeySrv& aObject)
    {
    TObjectIndex oi;
    oi.iObject = &aObject;
    oi.iHandle = ++iLastHandle;
    User::LeaveIfError(iOpenedKeys.InsertInSignedKeyOrder(oi));
    return oi.iHandle;
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreSession::RemoveOpenedKeyL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreSession::RemoveOpenedKeyL(TInt aHandle)
    {
    TObjectIndex oi;
    oi.iHandle = aHandle;
    TInt pos = iOpenedKeys.Find(oi);
    User::LeaveIfError(pos);
    delete iOpenedKeys[pos].iObject;
    iOpenedKeys.Remove(pos);  
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreSession::OpenedKey()
// ---------------------------------------------------------------------------
//
CDevCertOpenedKeySrv* CDevCertKeyStoreSession::OpenedKey(TInt aHandle)
    {
    TObjectIndex oi;
    oi.iHandle = aHandle;
    TInt pos = iOpenedKeys.Find(oi);
    return (pos != KErrNotFound) ? iOpenedKeys[pos].iObject : NULL;
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreSession::HasOpenKey()
// ---------------------------------------------------------------------------
//
TBool CDevCertKeyStoreSession::HasOpenKey(TInt aHandle)
    {
    TBool result = EFalse;
    for (TInt i = 0 ; i < iOpenedKeys.Count() ; ++i)
        {
        CDevCertOpenedKeySrv* object = iOpenedKeys[i].iObject;
        if (object->Handle() == aHandle)
            {
            result = ETrue;
            break;
            }
        }
    return result;
    }

//EOF

