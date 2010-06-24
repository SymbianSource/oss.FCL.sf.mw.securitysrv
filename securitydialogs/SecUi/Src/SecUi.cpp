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
* Description:  Provides initialization and uninitialization for
*               secui resource file.
*
*
*/


#include <e32std.h>
#include <eikenv.h>
#include <bautils.h>
#include "secui.h"
#include 	<data_caging_path_literals.hrh>

// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// TSecUi::InitializeLibL()
// Initializes SecUi resource file
// ----------------------------------------------------------
//
EXPORT_C void TSecUi::InitializeLibL()
	{
	if (Dll::Tls()!=NULL)
    {//Secui has been initialized already; increase client count.
        RDEBUG("Secui has been initialized already", 0);
        TSecUi* instance=(TSecUi*) Dll::Tls();
        instance->IncreaseClientCount();
        instance->iDialogOpened++;
        RDEBUG("instance->iDialogOpened", instance->iDialogOpened);
        return;
    }
    RDEBUG("First initialization", 0);
	TSecUi* self = new (ELeave) TSecUi();
	CleanupStack::PushL(self);
	self->ConstructL();
    self->IncreaseClientCount();
    self->iDialogOpened=0;
	Dll::SetTls(self);
	CleanupStack::Pop();
	}
//
// ----------------------------------------------------------
// TSecUi::UnInitializeLib()
// Uninitializes SecUi resource file
// ----------------------------------------------------------
//
EXPORT_C void TSecUi::UnInitializeLib()
	{
    RDEBUG("0", 0);
    if (Dll::Tls()==NULL)
    		{
    		RDEBUG("!!!!!!!!!! Dll::Tls not yet initialized: 0", 0);
        return;
      	}
    RDEBUG("0", 0);
	TSecUi* instance=(TSecUi*) Dll::Tls();
    instance->DecreaseClientCount();
    //only delete the lib is there are no clients using it
    if(instance->CanBeFreed())
        {
        RDEBUG("Last uninitialize", 0);
	    delete instance;
	    Dll::SetTls(NULL);
        }
	}

//
// ----------------------------------------------------------
// TSecUi::TSecUi()
// C++ default constructor
// ----------------------------------------------------------
//
TSecUi::TSecUi()
	{
	}
//
// ----------------------------------------------------------
// TSecUi::TSecUi()
// Destructor
// ----------------------------------------------------------
//
TSecUi::~TSecUi()
	{
	}
//
// ----------------------------------------------------------
// TSecUi::ConstructL()
// Symbian OS default constructor
// ----------------------------------------------------------
//
void TSecUi::ConstructL()
	{
		RDEBUG("iClientCount", iClientCount);
    iClientCount = 0;
	}

// -----------------------------------------------------------------------------
// TSecUi::CanBeFreed()
// -----------------------------------------------------------------------------
//
TBool TSecUi::CanBeFreed()
    {
    	RDEBUG("iClientCount", iClientCount);
	if (iClientCount <= 0)
	{
		return ETrue;
	}
	else
	{
        return EFalse;
	}
    }

// -----------------------------------------------------------------------------
// TSecUi::IncreaseClientCount()
// -----------------------------------------------------------------------------
//
void TSecUi::IncreaseClientCount()
    {
	++iClientCount;
	RDEBUG("iClientCount", iClientCount);
    }

// -----------------------------------------------------------------------------
// TSecUi::DecreaseClientCount()
// -----------------------------------------------------------------------------
//

void TSecUi::DecreaseClientCount()
    {
	--iClientCount;
	RDEBUG("iClientCount", iClientCount);
    }

	
// End of file


