/*
* Copyright (c) 2003-2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   ECOM proxy table for CertManUI plugin
*
*/


// INCLUDE FILES
#include <e32std.h>
#include <ecom/implementationproxy.h>

#include "CertmanuiPlugin.h"

// Constants
const TImplementationProxy KCertManUIPluginImplementationTable[] =
    {
    IMPLEMENTATION_PROXY_ENTRY( 0x10008D3D, CCertManUIPlugin::NewL )
    };


// ---------------------------------------------------------------------------
// ImplementationGroupProxy
// Gate/factory function
//
// ---------------------------------------------------------------------------
//
EXPORT_C const TImplementationProxy* ImplementationGroupProxy( TInt& aTableCount )
    {
    aTableCount = sizeof( KCertManUIPluginImplementationTable )
          / sizeof( TImplementationProxy );
    return KCertManUIPluginImplementationTable;
    }

// End of File
