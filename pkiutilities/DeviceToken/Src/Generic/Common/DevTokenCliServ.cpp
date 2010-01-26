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
* Description:   Implementation of DevTokenCliServ
*
*/



#include "DevTokenCliServ.h"
#include "DevTokenTypesEnum.h"
#include "DevTokenImplementationUID.hrh"

const TInt KDeviceTokenTypeUidValues[] = { DEVCERTKEYSTORE_IMPLEMENTATION_UID, DEVCERTSTORE_IMPLEMENTATION_UID, TRUSRVCERTSTORE_IMPLEMENTATION_UID };

// ======== MEMBER FUNCTIONS ========

// -----------------------------------------------------------------------------
// RSupportedTokensArray::RSupportedTokensArray()
// -----------------------------------------------------------------------------
//
RSupportedTokensArray::RSupportedTokensArray()
    {
    _LIT(KKeyStoreName, "device cert keystore");
    _LIT(KDeviceCertStoreName, "device certstore");
    _LIT(KTruSrvCertStoreName, "trust server certstore");
    const TDesC* array[] = {&KKeyStoreName, &KDeviceCertStoreName, &KTruSrvCertStoreName }; 
    Copy(array, ETotalTokensSupported);
    }


// -----------------------------------------------------------------------------
// RTokenTypeUIDLookup::RTokenTypeUIDLookup()
// -----------------------------------------------------------------------------
//
RTokenTypeUIDLookup::RTokenTypeUIDLookup() :
    TFixedArray<TInt, ETotalTokensSupported>(KDeviceTokenTypeUidValues, ETotalTokensSupported)
    {
    }
    
//EOF

