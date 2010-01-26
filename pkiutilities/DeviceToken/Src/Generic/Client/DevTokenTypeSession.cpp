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
* Description:   Implementation of DevTokenTypeSession
*
*/



#include <ct.h>
#include <e32test.h>
#include "DevTokenTypeSession.h"
#include "DevTokenUtils.h"
#include "DevToken.h"
#include "DevTokenTypesEnum.h"
#include "DevTokenCliServ.h"
#include "DevtokenLog.h"

// ======== MEMBER FUNCTIONS ========

// -----------------------------------------------------------------------------
// CDevTokenTypeSession::NewL(TUid aUid)
// -----------------------------------------------------------------------------
//
CDevTokenTypeSession* CDevTokenTypeSession::NewL(TUid aUid)
    {
    TRACE_PRINT("CDevTokenTypeSession::NewL()");
    // Destroyed by owner object (~CDevTokenTypeSession)
    CDevTokenTypeSession* me = new (ELeave) CDevTokenTypeSession(aUid);
    CleanupStack::PushL(me);
    me->ConstructL();
    CleanupStack::Pop(me);
    return (me);
    }


// -----------------------------------------------------------------------------
// CDevTokenTypeSession::CDevTokenTypeSession(TUid aUid) 
// -----------------------------------------------------------------------------
//
CDevTokenTypeSession::CDevTokenTypeSession(TUid aUid) :
    iUid(aUid)
    {
    } 


// -----------------------------------------------------------------------------
// CDevTokenTypeSession::~CDevTokenTypeSession(TUid aUid) 
// -----------------------------------------------------------------------------
//
CDevTokenTypeSession::~CDevTokenTypeSession()
    {
    iClientSession.Close();
    }


// -----------------------------------------------------------------------------
// void CDevTokenTypeSession::ConstructL()
// -----------------------------------------------------------------------------
//
void CDevTokenTypeSession::ConstructL()
    {
    TRACE_PRINT("CDevTokenTypeSession::ConstructL()");
    iTokenId = (EDevTokenEnum) KErrNotFound;

    RTokenTypeUIDLookup tokenUids;
    for ( TInt i = 0 ; i < ETotalTokensSupported ; ++i )
        {
        TRACE_PRINT("CDevTokenTypeSession::ConstructL() 1");
        if ( tokenUids[i] == iUid.iUid )
            {
            TRACE_PRINT("CDevTokenTypeSession::ConstructL() 2");
            iTokenId = (EDevTokenEnum) i;
            break;
            }
        }
    TRACE_PRINT("CDevTokenTypeSession::ConstructL() 3");
    User::LeaveIfError((TInt) iTokenId);
    TRACE_PRINT("CDevTokenTypeSession::ConstructL()4 ");
    User::LeaveIfError(iClientSession.Connect(iTokenId));
    TRACE_PRINT("CDevTokenTypeSession::ConstructL()5 ");
    }


// -----------------------------------------------------------------------------
// CDevTokenTypeSession::ListTokensL(RCPointerArray<HBufC>& aTokens)
// -----------------------------------------------------------------------------
//
void CDevTokenTypeSession::ListTokensL(RCPointerArray<HBufC>& aTokens)
    {
    RSupportedTokensArray tokenNames;
    HBufC* name = tokenNames[iTokenId]->AllocLC();
    User::LeaveIfError(aTokens.Append(name));
    CleanupStack::Pop(name);
    }


// -----------------------------------------------------------------------------
// CDevTokenTypeSession::OpenTokenL()
// -----------------------------------------------------------------------------
//
void CDevTokenTypeSession::OpenTokenL(const TDesC& aTokenInfo, 
                                      MCTTokenType* aTokenType, 
                                      MCTToken*& aToken)
    {
    RSupportedTokensArray tokenNames;
    if ( *tokenNames[iTokenId] != aTokenInfo )
        {
        User::Leave(KErrArgument);
        }
    aToken = CDevToken::NewL(iTokenId, aTokenType, iClientSession);
    }


// -----------------------------------------------------------------------------
// CDevTokenTypeSession::OpenTokenL()
// -----------------------------------------------------------------------------
//
void CDevTokenTypeSession::OpenTokenL(TCTTokenHandle aHandle, 
                                      MCTTokenType* aTokenType, 
                                      MCTToken*& aToken)
    {   
    if ( aHandle.iTokenTypeUid != iUid ||
        aHandle.iTokenId != iTokenId )
        {
        User::Leave(KErrArgument);
        }
    aToken = CDevToken::NewL(iTokenId, aTokenType, iClientSession);
    }

//EOF

