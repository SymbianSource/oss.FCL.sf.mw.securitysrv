/*
* ============================================================================
*  Name        : SimLockUI.cpp
*  Part of     : Sim Lock UI Application
*  Description : Sim Lock UI App framework code
*  Version     : 
*  
* Copyright (c) 2005-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   Build info file for Ado domain appinstall 
* ============================================================================
*/

// System Includes
#include <eikstart.h>

// User Includes
#include "SimLockUIApplication.h"

// ---------------------------------------------------------------------------
// NewApplication
// ---------------------------------------------------------------------------
CApaApplication* NewApplication()
    {
    // Create an application, and return a pointer to it
    return new CSimLockUIApplication;
    }

// ---------------------------------------------------------------------------
// E32Main
// ---------------------------------------------------------------------------
TInt E32Main()
    {
    return EikStart::RunApplication( NewApplication );
    }

// End of file


