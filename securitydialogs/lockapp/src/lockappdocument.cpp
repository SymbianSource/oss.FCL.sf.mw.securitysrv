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
* Description:  LockApp application document class
 *
*/


#include "lockappappui.h"
#include "lockappdocument.h"

// ---------------------------------------------------------------------------
// Standard Symbian OS construction sequence
// ---------------------------------------------------------------------------
CLockAppDocument* CLockAppDocument::NewL(CEikApplication& aApp)
    {
    CLockAppDocument* self = NewLC(aApp);
    CleanupStack::Pop(self);
    return self;
    }

// ---------------------------------------------------------------------------
// Standard Symbian OS construction sequence
// ---------------------------------------------------------------------------
CLockAppDocument* CLockAppDocument::NewLC(CEikApplication& aApp)
    {
    CLockAppDocument* self = new (ELeave) CLockAppDocument(aApp);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

CLockAppDocument::~CLockAppDocument()
    {
    // no implementation required
    }

// ---------------------------------------------------------------------------
//  Create the application user interface, and return a pointer to it,
//  The framework takes ownership of this object.
// ---------------------------------------------------------------------------
CEikAppUi* CLockAppDocument::CreateAppUiL()
    {
    CEikAppUi* appUi = new (ELeave) CLockAppAppUi;
    return appUi;
    }

void CLockAppDocument::ConstructL()
    {
    // no implementation required
    }

CLockAppDocument::CLockAppDocument( CEikApplication& aApp ) :
	CAknDocument(aApp)
	{
	// no implementation required
	}
