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
* Description:   Implementation of Application class
*
*/


// INCLUDE FILES
#include    "CertSaverApp.h"
#include    "CertSaverDocument.h"

#include    <eikstart.h>

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CCertSaverApp::AppDllUid()
// Returns application UID
// ---------------------------------------------------------
//
TUid CCertSaverApp::AppDllUid() const
    {
    return KUidCertSaver;
    }

// ---------------------------------------------------------
// CCertSaverApp::CreateDocumentL()
// Creates CCertSaverDocument object
// ---------------------------------------------------------
//
CApaDocument* CCertSaverApp::CreateDocumentL()
    {
    return CCertSaverDocument::NewL( *this );
    }

// ---------------------------------------------------------
// NewApplication()
// Returns created application object
// ---------------------------------------------------------
//
LOCAL_C CApaApplication* NewApplication()
    {
    return new CCertSaverApp;
    }

// ---------------------------------------------------------
// E32Main()
// Main startup entry point
// ---------------------------------------------------------
//
GLDEF_C TInt E32Main()
    {
    return EikStart::RunApplication( NewApplication );
    }

// End of File
