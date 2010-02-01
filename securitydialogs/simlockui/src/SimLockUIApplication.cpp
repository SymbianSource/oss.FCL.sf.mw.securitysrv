/*
* ============================================================================
*  Name        : SimLockUIApplication.cpp
*  Part of     : Sim Lock UI Application
*  Description : Implementation of Sim Lock UI Application UI Methods
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

// User include files
#include "SimLockUIDocument.h"
#include "SimLockUIApplication.h"
#include "SimLockUI.hrh"

// UID for the application, this should correspond to the uid defined in the mmp file
static const TUid KUidSimLockUIApp = {SIMLOCK_UI_UID3};

// ---------------------------------------------------------------------------
// CSimLockUIApplication::CreateDocumentL
// ---------------------------------------------------------------------------
CApaDocument* CSimLockUIApplication::CreateDocumentL()
    {  
    // Create an SimLockUI document, and return a pointer to it
    CApaDocument* document = CSimLockUIDocument::NewL( *this );
    return document;
    }

// ---------------------------------------------------------------------------
// CSimLockUIApplication::AppDllUid
// ---------------------------------------------------------------------------
TUid CSimLockUIApplication::AppDllUid() const
    {
    // Return the UID for the SimLockUI application
    return KUidSimLockUIApp;
    }

// End of file

