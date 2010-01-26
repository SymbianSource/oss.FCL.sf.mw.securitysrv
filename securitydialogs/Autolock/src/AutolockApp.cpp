/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:
*
*/


// INCLUDE FILES
#include    "AutolockApp.h"
#include    "AutolockDocument.h"
#include <eikstart.h>


// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CAutolockApp::AppDllUid()
// Returns application UID
// ---------------------------------------------------------
//
TUid CAutolockApp::AppDllUid() const
    {
    return KUidAutolock;
    }

// ---------------------------------------------------------
// CAutolockApp::CreateDocumentL()
// Creates CAutolockDocument object
// ---------------------------------------------------------
//
CApaDocument* CAutolockApp::CreateDocumentL()
    {
    return CAutolockDocument::NewL( *this );
    }

// ================= OTHER EXPORTED FUNCTIONS ==============
//

LOCAL_C CApaApplication* NewApplication()
    {
    return new CAutolockApp;
    }

GLDEF_C TInt E32Main()
    {
    return EikStart::RunApplication(NewApplication);
    }

// End of File  

