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
* Description:   Implementation of DevandTruSrvCertStorePlugin
*
*/



#include <e32std.h>
#include <implementationproxy.h>
#include <mctcertstore.h>
#include <ct.h>
#include "DevTokenImplementationUID.hrh"
#include "DevTokenType.h"

static CCTTokenType* NewFunctionL();

// ======== LOCAL FUNCTIONS ========

// ---------------------------------------------------------------------------
// CCTTokenType* NewFunctionL()
// ---------------------------------------------------------------------------
//
CCTTokenType* NewFunctionL()
    {
    CCTTokenType* TruSrvCertStore = NULL;
    //Implementation UID of Trust Server Certstore
    TUid uid;
    uid.iUid = TRUSRVCERTSTORE_IMPLEMENTATION_UID;
    TruSrvCertStore = CDevTokenType::NewL( uid );
    return ( TruSrvCertStore );
    }


// -----------------------------------------------------------------------------
// TImplementationProxy ImplementationTable[] 
// -----------------------------------------------------------------------------
//
const TImplementationProxy ImplementationTable[] =
    {
    IMPLEMENTATION_PROXY_ENTRY( TRUSRVCERTSTORE_IMPLEMENTATION_UID, NewFunctionL )
    };


// -----------------------------------------------------------------------------
// TImplementationProxy* ImplementationGroupProxy()
// This function is needed by ECom and is the only one exported function
// -----------------------------------------------------------------------------
//
EXPORT_C const TImplementationProxy* ImplementationGroupProxy( TInt& aTableCount )
    {
    aTableCount = sizeof( ImplementationTable) / sizeof( TImplementationProxy );
    return ( ImplementationTable );
    }



