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
* Description:   Implementation of the CSecModUIContainerSignature class
*
*/


// INCLUDE FILES
#include "SecModUIContainerSignature.h"
#include "SecModUIModel.h"
#include "wim.hlp.hrh"
#include <SecModUI.rsg>
#include <aknlists.h>

// ================= MEMBER FUNCTIONS =======================
// ---------------------------------------------------------
// CSecModUIContainerSignature::CSecModUIContainerSignature(
//     CSecModUIModel& aModel)
// Constructor
// ---------------------------------------------------------
//
CSecModUIContainerSignature::CSecModUIContainerSignature(CSecModUIModel& aModel, CSecModUIViewSignature* aView )
    :CSecModUIContainerBase(aModel, KSEMOD_HLP_SIGNINGCODE_VIEW)
    {    
    iView = aView;
	}

// Destructor
CSecModUIContainerSignature::~CSecModUIContainerSignature()
    {
    
    }

// ---------------------------------------------------------
// CSecModUIContainerSignature::ConstructListL()
// Constructs listbox
// ---------------------------------------------------------
//
void CSecModUIContainerSignature::ConstructListL()	
    {
    iListBox = new (ELeave) CAknSettingStyleListBox();
    iListBox->SetObserver( this );
    }

// ---------------------------------------------------------
// CSecModUIContainerSignature::SetupListItemsL()
// Setups list items to listbox.
// ---------------------------------------------------------
//	
void CSecModUIContainerSignature::SetupListItemsL()
    {
    iModel.LoadPinNRLabelsL(*iListBox, ETrue);
    }    

// ---------------------------------------------------------
// CSecModUIContainerSignature::HandleControlEventL()
// 
// ---------------------------------------------------------
// 
void CSecModUIContainerSignature::HandleControlEventL(CCoeControl* /*aControl*/,
                                               TCoeEvent aEventType)
    {
	if( aEventType == EEventStateChanged )
	    {
	    iView->UpdateCbaL(iListBox->CurrentItemIndex());	
	    }
    }     
       

// End of File  
