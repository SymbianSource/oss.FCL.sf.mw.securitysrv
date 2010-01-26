/*
* Copyright (c) 2003 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  DLL Entry point
*
*/
 

#include "e32base.h"
#include "WimTrace.h"

#ifndef EKA2
// -----------------------------------------------------------------------------
// GLDEF_C TInt E32Dll( TDllReason aReason )
// The E32Dll() entry point function
// -----------------------------------------------------------------------------

GLDEF_C TInt E32Dll( TDllReason /*aReason*/ )
    {
    _WIMTRACE ( _L( "WimUtilEntry | E32Dll" ) );
    return( KErrNone );
    }
    
#endif // EKA2

