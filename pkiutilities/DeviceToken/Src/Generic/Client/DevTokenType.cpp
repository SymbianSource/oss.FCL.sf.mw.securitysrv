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
* Description:   Implementation of DevTokenType
*
*/



#include <ct.h>
#include <e32test.h>
#include "DevTokenType.h"
#include "DevTokenTypeSession.h"
#include "DevTokenUtils.h"
#include "DevtokenLog.h"

// ======== MEMBER FUNCTIONS ========

// -----------------------------------------------------------------------------
// CCTTokenType* CDevTokenType::NewL(TUid aUid)
// -----------------------------------------------------------------------------
//
EXPORT_C CCTTokenType* CDevTokenType::NewL(TUid aUid)
    {
    TRACE_PRINT("CDevTokenType::NewL()--->");

    //Destroyed by call to CCTTokenType::Release (refcounted)
    CDevTokenType* me = new (ELeave) CDevTokenType();
    CleanupStack::PushL(me);
    me->ConstructL(aUid);

    TRACE_PRINT("CDevTokenType::NewL()<----");
    CleanupStack::Pop(me);

    //  Caller only receives a valid CDevTokenType instance if the server has connected
    return (me);
    }


// -----------------------------------------------------------------------------
// CDevTokenType::CDevTokenType()
// -----------------------------------------------------------------------------
//
CDevTokenType::CDevTokenType()
    {
    }


// -----------------------------------------------------------------------------
// CDevTokenType::~CDevTokenType()
// -----------------------------------------------------------------------------
//
CDevTokenType::~CDevTokenType()
    {
    delete iTokenTypeSession;
    }


// -----------------------------------------------------------------------------
// CDevTokenType::ConstructL(TUid aUid)
// -----------------------------------------------------------------------------
//
void CDevTokenType::ConstructL(TUid aUid)
    {
    iUid = aUid;
    iTokenTypeSession = CDevTokenTypeSession::NewL(aUid);
    }


// -----------------------------------------------------------------------------
// CDevTokenType::List(RCPointerArray<HBufC>& aTokens, TRequestStatus& aStatus)
// -----------------------------------------------------------------------------
//
void CDevTokenType::List(RCPointerArray<HBufC>& aTokens, TRequestStatus& aStatus)
    {
    __ASSERT_ALWAYS(iTokenTypeSession, DevTokenPanic(ETokenTypeSession));
    TRAPD(err, iTokenTypeSession->ListTokensL(aTokens));
    TRequestStatus* status = &aStatus;
    User::RequestComplete(status, err);
    }


// -----------------------------------------------------------------------------
// CDevTokenType::CancelList()
// -----------------------------------------------------------------------------
//
void CDevTokenType::CancelList()
    {
    // implementation not asynchronous
    }


// -----------------------------------------------------------------------------
// CDevTokenType::OpenToken()
// -----------------------------------------------------------------------------
//
void CDevTokenType::OpenToken(const TDesC& aTokenInfo, MCTToken*& aToken, TRequestStatus& aStatus)
    {
    __ASSERT_ALWAYS(iTokenTypeSession, DevTokenPanic(ETokenTypeSession));

    TRAPD(err, iTokenTypeSession->OpenTokenL(aTokenInfo, this, aToken));
    if ( err == KErrNone )
        {
        //  Token opened, increment our reference count
        IncReferenceCount();
        }
    TRequestStatus* status = &aStatus;
    User::RequestComplete(status, err);
    }


// -----------------------------------------------------------------------------
// CDevTokenType::OpenToken()
// -----------------------------------------------------------------------------
//
void CDevTokenType::OpenToken(TCTTokenHandle aHandle, MCTToken*& aToken, TRequestStatus& aStatus)
    {
    __ASSERT_ALWAYS(iTokenTypeSession, DevTokenPanic(ETokenTypeSession));

    TRAPD(err, iTokenTypeSession->OpenTokenL(aHandle, this, aToken));
    if ( err == KErrNone )
        {
        //  Token opened, increment our reference count
        IncReferenceCount();
        }
    TRequestStatus* status = &aStatus;
    User::RequestComplete(status, err);
    }


// -----------------------------------------------------------------------------
// CDevTokenType::CancelOpenToken()
// -----------------------------------------------------------------------------
//
void CDevTokenType::CancelOpenToken()
    {
    // implementation not asynchronous
    }

//EOF

