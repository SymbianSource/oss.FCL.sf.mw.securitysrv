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
* Description:   Implementation of DevTokenDHParams
*
*/



#include "DevTokenDataTypes.h"

// ======== MEMBER FUNCTIONS ========


// ---------------------------------------------------------------------------
// CDevTokenDHParams::~CDevTokenDHParams()
// ---------------------------------------------------------------------------
// 
EXPORT_C CDevTokenDHParams::~CDevTokenDHParams()
    {
    iN.Close();
    iG.Close();
    }


// ---------------------------------------------------------------------------
// CDevTokenDHParams::CDevTokenDHParams(RInteger aN, RInteger aG)
// ---------------------------------------------------------------------------
//
EXPORT_C CDevTokenDHParams::CDevTokenDHParams(RInteger aN, RInteger aG)
    : iN(aN), iG(aG)
    {
    }


// ---------------------------------------------------------------------------
// CDevTokenDHParams::NewL(const TInteger& aN, const TInteger& aG)
// ---------------------------------------------------------------------------
//
EXPORT_C CDevTokenDHParams* CDevTokenDHParams::NewL(const TInteger& aN, const TInteger& aG)
    {
    RInteger n = RInteger::NewL(aN);
    CleanupStack::PushL(n);
    RInteger g = RInteger::NewL(aG);
    CleanupStack::PushL(g);
    CDevTokenDHParams* self = new (ELeave) CDevTokenDHParams(n, g);
    CleanupStack::Pop(2); // g, n
    return self;
    }


// ---------------------------------------------------------------------------
// CDevTokenDHParams::TakeN()
// ---------------------------------------------------------------------------
//
EXPORT_C RInteger CDevTokenDHParams::TakeN()
    {
    RInteger result = iN;
    iN = RInteger();
    return result;
    }


// ---------------------------------------------------------------------------
// CDevTokenDHParams::TakeG()
// ---------------------------------------------------------------------------
//
EXPORT_C RInteger CDevTokenDHParams::TakeG()
    {
    RInteger result = iG;
    iG = RInteger();
    return result;
    }

//EOF

