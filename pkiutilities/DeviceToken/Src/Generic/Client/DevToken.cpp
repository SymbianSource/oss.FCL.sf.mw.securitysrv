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
* Description:   Implementation of DevToken
*
*/



#include <e32debug.h>
#include "DevToken.h"
#include "DevTokenUtils.h"
#include "DevTokenInterfaceFactory.h"
#include "DevtokenLog.h"

// Information strings returned by MCTToken::Information()
_LIT(KVersion, "1.00");
_LIT(KSerialNo, "0");
_LIT(KManufacturer, "Nokia");

// ======== MEMBER FUNCTIONS ========

// -----------------------------------------------------------------------------
// MCTToken* CDevToken::NewL()
// -----------------------------------------------------------------------------
//
MCTToken* CDevToken::NewL(EDevTokenEnum aTokenTypeVal, 
                          MCTTokenType* aTokenType, 
                          RDevTokenClientSession& aClient)
    {
    __ASSERT_DEBUG(aTokenType, DevTokenPanic(EBadArgument));
    //Destroyed by MCTToken::Release() (refcounted)
    CDevToken* me = new (ELeave) CDevToken(aTokenTypeVal, aTokenType, aClient);
    return (static_cast<MCTToken*>(me));
    }


// -----------------------------------------------------------------------------
// CDevToken::CDevToken()
// -----------------------------------------------------------------------------
//
CDevToken::CDevToken(EDevTokenEnum aTokenTypeVal, MCTTokenType* aTokenType, RDevTokenClientSession& aClient)
    : iTokenEnum(aTokenTypeVal),
    iTokenType(aTokenType),
    iRefCount(0),
    iClientSession(aClient)
    {
    ASSERT(iTokenEnum < ETotalTokensSupported);
    }


// -----------------------------------------------------------------------------
// MCTTokenType& CDevToken::TokenType()
// -----------------------------------------------------------------------------
//
MCTTokenType& CDevToken::TokenType()
    {
    __ASSERT_DEBUG(iTokenType, DevTokenPanic(ENotInitialised));
    return (*iTokenType);
    }


// -----------------------------------------------------------------------------
// const TDesC& CDevToken::Label()
// -----------------------------------------------------------------------------
//
const TDesC& CDevToken::Label()
    {
    RSupportedTokensArray supportedTokens;
    const TDesC* token = supportedTokens[iTokenEnum];
    return (*token);
    }


// -----------------------------------------------------------------------------
// TCTTokenHandle CDevToken::Handle()
// -----------------------------------------------------------------------------
//
TCTTokenHandle CDevToken::Handle()
    {
    __ASSERT_DEBUG(iTokenType, DevTokenPanic(ENotInitialised));
    return (TCTTokenHandle(iTokenType->Type(), iTokenEnum));
    }


// -----------------------------------------------------------------------------
// TInt& CDevToken::ReferenceCount()
// -----------------------------------------------------------------------------
//
TInt& CDevToken::ReferenceCount()
    {
    return (iRefCount);
    }


// -----------------------------------------------------------------------------
// CDevToken::DoGetInterface()
// -----------------------------------------------------------------------------
//
void CDevToken::DoGetInterface(TUid aRequiredInterface, 
                               MCTTokenInterface*& aReturnedInterface,
                               TRequestStatus& aStatus)
    {
    // No longer calls server to get the interface - just creates a client object of the appropriate type
    TRACE_PRINT("--> DoGetInterface 1");
    aReturnedInterface = NULL;
    TRAPD(result, aReturnedInterface = DevTokenInterfaceFactory::ClientInterfaceL(aRequiredInterface.iUid, *this, iClientSession));
    TRACE_PRINT("--> DoGetInterface 2")
    if (result != KErrNone)
        {
        TRACE_PRINT("-->DoGetInterface 3")
        Release();
        }

    //Complete the TRequestStatus here since not asynchronous
    TRequestStatus* status = &aStatus;
    User::RequestComplete(status, result);
    }


// -----------------------------------------------------------------------------
// TBool CDevToken::DoCancelGetInterface()
// -----------------------------------------------------------------------------
//
TBool CDevToken::DoCancelGetInterface()
    {//Not an asynchronous call for current file store, so nothing to do
    return (EFalse);
    }


// -----------------------------------------------------------------------------
// CDevToken::Information(TTokenInformation aRequiredInformation)
// -----------------------------------------------------------------------------
//
const TDesC& CDevToken::Information(TTokenInformation aRequiredInformation)
    {
    switch ( aRequiredInformation )
        {
        case EVersion:
            return KVersion;

        case ESerialNo:
            return KSerialNo;

        case EManufacturer:
            return KManufacturer;

        default:
            DevTokenPanic(EBadArgument);
        }

    return KNullDesC;
    }

//EOF

