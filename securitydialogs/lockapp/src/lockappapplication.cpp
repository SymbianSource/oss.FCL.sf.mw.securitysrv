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
* Description:  Application class that also creates the appserver
 *
*/


#include <eikstart.h>
#include <eikenv.h>
#include "lockappdocument.h"
#include "lockappserver.h"
#include "lockappapplication.h"

// UID for the application, this should correspond to the uid defined in the mmp file
static const TUid KUidLockAppApp =
    {
    0x10283322
    };

// ---------------------------------------------------------------------------
// Create an application, and return a pointer to it
// ---------------------------------------------------------------------------
CApaApplication* NewApplication( )
    {
    return new CLockAppApplication;
    }

// ---------------------------------------------------------------------------
// Main function of the application executable.
// ---------------------------------------------------------------------------
TInt E32Main( )
    {
    return EikStart::RunApplication( NewApplication );
    }

// ---------------------------------------------------------------------------
// Returns the UID for the application
// ---------------------------------------------------------------------------
TUid CLockAppApplication::AppDllUid( ) const
    {
    // Return the UID for the LockApp application
    return KUidLockAppApp;
    }

// ---------------------------------------------------------------------------
// Creates a document object and passes ownership.
// ---------------------------------------------------------------------------
CApaDocument* CLockAppApplication::CreateDocumentL( )
    {
    // Create an LockApp document, and return a pointer to it
    CApaDocument* document = CLockAppDocument::NewL( *this );
    return document;
    }
