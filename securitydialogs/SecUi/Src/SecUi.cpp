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

_LIT(KDirAndFile,"z:SecUi.rsc");


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
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI): InitializeLibL: Secui has been initialized already"));
        #endif
        TSecUi* instance=(TSecUi*) Dll::Tls();
        instance->IncreaseClientCount();
        return;
    }
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI): InitializeLibL: First initialization"));
    #endif
	TSecUi* self = new (ELeave) TSecUi();
	CleanupStack::PushL(self);
	self->ConstructL();
    self->IncreaseClientCount();
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
    if (Dll::Tls()==NULL)
        return;
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI): UnInitializeLibL"));
    #endif
	TSecUi* instance=(TSecUi*) Dll::Tls();
    instance->DecreaseClientCount();
    //only delete the lib is there are no clients using it
    if(instance->CanBeFreed())
        {
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI): UnInitializeLibL: Last uninitialize"));
        #endif
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
	if (iResourceFileOffset >= 0)
		{
		CEikonEnv::Static()->DeleteResourceFile(iResourceFileOffset);
		}
	}
//
// ----------------------------------------------------------
// TSecUi::ConstructL()
// Symbian OS default constructor
// ----------------------------------------------------------
//
void TSecUi::ConstructL()
	{
	iResourceFileOffset = CCoeEnv::Static()->AddResourceFileL(ResourceFileName());
    iClientCount = 0;
	}
//
// ----------------------------------------------------------
// TSecUi::ResourceFileName
// Returns the resource file name of SecurityUI dll.
// ----------------------------------------------------------
//
TFileName TSecUi::ResourceFileName()
	{
	
	TParse parse;
    parse.Set(KDirAndFile, &KDC_RESOURCE_FILES_DIR, NULL); 
	TFileName resourceFileName(parse.FullName());
	BaflUtils::NearestLanguageFile(CCoeEnv::Static()->FsSession(), resourceFileName);
	return resourceFileName;
	
	}

// -----------------------------------------------------------------------------
// TSecUi::CanBeFreed()
// -----------------------------------------------------------------------------
//
TBool TSecUi::CanBeFreed()
    {
	if (iClientCount <= 0)
	{
        #if defined(_DEBUG)
		RDebug::Print(_L("(SECUI): No clients; Can be freed: clients(%d) "), iClientCount);
        #endif
		return ETrue;
	}
	else
	{
        #if defined(_DEBUG)
		RDebug::Print(_L("(SECUI): Can NOT be freed: clients(%d) "), iClientCount);
		#endif
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
    #if defined(_DEBUG)
	RDebug::Print(_L("(SECUI): IncreaseClientCount, clients now(%d) "), iClientCount);
    #endif
    }

// -----------------------------------------------------------------------------
// TSecUi::DecreaseClientCount()
// -----------------------------------------------------------------------------
//

void TSecUi::DecreaseClientCount()
    {
	--iClientCount;
    #if defined(_DEBUG)
	RDebug::Print(_L("(SECUI): DecreaseClientCount, clients now(%d) "), iClientCount);
    #endif
    }

	
// End of file


