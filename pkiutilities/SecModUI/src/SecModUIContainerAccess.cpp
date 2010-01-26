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
* Description:   Implementation of the CSecModUIContainerAccess class
*
*/


// INCLUDE FILES
#include "SecModUIContainerAccess.h"
#include "SecModUIModel.h"
#include "wim.hlp.hrh"
#include <SecModUI.rsg>
#include <aknlists.h>

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CSecModUIContainerAccess::CSecModUIContainerAccess(
//     CSecModUIModel& aModel)
// Constructor
// ---------------------------------------------------------
//
CSecModUIContainerAccess::CSecModUIContainerAccess(CSecModUIModel& aModel, CSecModUIViewAccess* aView)
    :CSecModUIContainerBase(aModel, KSECMOD_HLP_ACCESSCODE_VIEW)
    {  
    iView = aView;  
	}

// Destructor
CSecModUIContainerAccess::~CSecModUIContainerAccess()
    {
    }

// ---------------------------------------------------------
// CSecModUIContainerSignature::ConstructListL()
// Constructs listbox
// ---------------------------------------------------------
//
void CSecModUIContainerAccess::ConstructListL()	
    {
    iListBox = new (ELeave) CAknSettingStyleListBox();
    iListBox->SetObserver(this);
    }


// ---------------------------------------------------------
// CSecModUIContainerAccess::SetupListItemsL()
// Setups list items to listbox.
// ---------------------------------------------------------
//	
void CSecModUIContainerAccess::SetupListItemsL()
    {
    iModel.LoadPinGItemsL(*iListBox);
    }   

// ---------------------------------------------------------
// CSecModUIContainerAccess::HandleControlEventL()
// Notify the highlight move event
// ---------------------------------------------------------
//    
void CSecModUIContainerAccess::HandleControlEventL(CCoeControl* /*aControl*/,
                                               TCoeEvent aEventType)
    {
	if( aEventType == EEventStateChanged )
	    {
	    iView->UpdateCbaL();	
	    }
    }     

// End of File  
