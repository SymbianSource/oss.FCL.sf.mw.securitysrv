/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  GBA Server Policy definitions
*
*/


#ifndef   GBASERVERPOLICY_H
#define   GBASERVERPOLICY_H

#include <e32base.h>
#include "GbaCommon.h"

// Panic Category 
_LIT(KGbaServer, "gbaserver2");


// Total number of policy ranges
const TUint KGBARangeCount = 5;

const TInt GBARanges[KGBARangeCount] =
    {
    EGbaServRequestBootstrap,             // Range 0  
    EGbaServCancelRequestBootstrap,       // Range 1
    EGbaServWriteOption,                  // Range 2
    EGbaServIsGBAUSupported,              // Range 3 
    EGbaServIsGBAUSupported+1             // Range 4
    };                                    

// Policy to implement for each of the ranges in range table.
const TUint8 GBAElementsIndex[KGBARangeCount] =
    {
    1,                                    // ReadDeviceData is required for bootstrap 
    CPolicyServer::EAlwaysPass,           // No cap required for cancel bootstrap
    0,                                    // WriteDeviceData is required for write options
    1,                                    // ReadDeviceData is required for checking gba-u 
    CPolicyServer::ENotSupported          // applies to 5th range (out of range IPC)
    };

// Specific capability checks.
const CPolicyServer::TPolicyElement GBAElements[] =
    {
        // policy "0"; 
        {
        _INIT_SECURITY_POLICY_C1( ECapabilityWriteDeviceData ),
        CPolicyServer::EFailClient
        },
        // policy "1"; 
        {
        _INIT_SECURITY_POLICY_C1( ECapabilityReadDeviceData ),
        CPolicyServer::EFailClient
        }
    };

// Packs all the security definitions into a policy.
const CPolicyServer::TPolicy GBAPolicy =
    {
    CPolicyServer::EAlwaysPass, // all connect attempts should pass
    KGBARangeCount,             // number of ranges
    GBARanges,                  // ranges array
    GBAElementsIndex,           // elements<->ranges index
    GBAElements,                // array of elements
    };

#endif //GBASERVERPOLICY_H
