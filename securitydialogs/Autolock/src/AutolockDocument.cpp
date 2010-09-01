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
#include <apgwgnam.h>
#include "AutolockDocument.h"
#include "AutolockAppUiPS.h"

// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------
// CAutolockDocument::ConstructL()
// destructor.
// ----------------------------------------------------
//
CAutolockDocument::~CAutolockDocument()
    {
    }
// ----------------------------------------------------
// CAutolockDocument::ConstructL()
// Symbian OS default constructor can leave..
// ----------------------------------------------------
//
void CAutolockDocument::ConstructL()
    {
    }
// ----------------------------------------------------
// CAutolockDocument::NewL()
// Two-phased constructor.
// ----------------------------------------------------
//
CAutolockDocument* CAutolockDocument::NewL(CEikApplication& aApp)     
    {
    CAutolockDocument* self = new (ELeave) CAutolockDocument( aApp );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }
// ----------------------------------------------------
// CAutolockDocument::CreateAppUiL()
// constructs CAutolockAppUi
// ----------------------------------------------------
//
CEikAppUi* CAutolockDocument::CreateAppUiL()
    {
    return new (ELeave) CAutolockAppUi;
    }

// ----------------------------------------------------
// CAutolockDocument::UpdateTaskNameL()
// Sets app hidden...
// ----------------------------------------------------
//
void CAutolockDocument::UpdateTaskNameL( CApaWindowGroupName* aWgName )
	{
	CEikDocument::UpdateTaskNameL( aWgName );
	aWgName->SetHidden( ETrue );
	}
// End of File  
