/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Implementation of the CSecModUIContainerMain class
*
*/


// INCLUDE FILES
#include "SecModUIContainerMain.h"
#include "SecModUIModel.h"
#include "SecModUILogger.h"
#include "wim.hlp.hrh"
#include <SecModUI.rsg>
#include <aknlists.h>
#include <barsread.h>
#include <StringLoader.h>


// ================= MEMBER FUNCTIONS =======================
CSecModUIContainerMain::CSecModUIContainerMain(CSecModUIModel& aModel)
    :CSecModUIContainerBase(aModel, KSECMOD_HLP_MAIN_VIEW)
    {    
	}

// Destructor
CSecModUIContainerMain::~CSecModUIContainerMain()
    { 
    LOG_ENTERFN("CSecModUIContainerMain::~CSecModUIContainerMain()");   
    LOG_LEAVEFN("CSecModUIContainerMain::~CSecModUIContainerMain()");       
	}

// ---------------------------------------------------------
// CSecModUIContainerSignature::ConstructListL()
// Constructs listbox
// ---------------------------------------------------------
//
void CSecModUIContainerMain::ConstructListL()	
    {
    iListBox = new (ELeave) CAknDoubleStyleListBox();
    }
	
// ---------------------------------------------------------
// CSecModUIContainerMain::SetupListItemsL()
// Setups list items to listbox.
// ---------------------------------------------------------
//	
void CSecModUIContainerMain::SetupListItemsL()
    {
    LOG_ENTERFN("CSecModUIContainerMain::SetupListItemsL()");   
    iModel.LoadTokenLabelsL(*iListBox);
    LOG_LEAVEFN("CSecModUIContainerMain::SetupListItemsL()");   
    }
       
// End of File  
