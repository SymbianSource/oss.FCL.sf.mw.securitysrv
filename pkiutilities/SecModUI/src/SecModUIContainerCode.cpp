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
* Description:   Implementation of the CSecModUIContainerCode class
*
*/


// INCLUDE FILES
#include "SecModUIContainerCode.h"
#include "SecModUIModel.h"
#include "SecModUILogger.h"
#include "wim.hlp.hrh"
#include <SecModUI.rsg>
#include <aknlists.h>

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CSecModUIContainerCode::CSecModUIContainerCode(
//     CSecModUIModel& aModel)
// Constructor
// ---------------------------------------------------------
//
CSecModUIContainerCode::CSecModUIContainerCode(CSecModUIModel& aModel)
    :CSecModUIContainerBase(aModel, KSECMOD_HLP_CODE_VIEW)
    {    
	}

// Destructor
CSecModUIContainerCode::~CSecModUIContainerCode()
    {
    LOG_ENTERFN( "CSecModUIContainerCode::~CSecModUIContainerCode" );
    LOG_LEAVEFN( "CSecModUIContainerCode::~CSecModUIContainerCode" );
    }
    
// ---------------------------------------------------------
// CSecModUIContainerBase::CreateResourceReaderLC(
//  TResourceReader& aReader)
// 
// ---------------------------------------------------------
//
void CSecModUIContainerCode::CreateResourceReaderLC(TResourceReader& aReader)	
    {
    LOG_ENTERFN( "CSecModUIContainerCode::CreateResourceReaderLC" );
    iEikonEnv->CreateResourceReaderLC(aReader, R_SECMODUI_CODES_LISTBOX);
    LOG_LEAVEFN( "CSecModUIContainerCode::CreateResourceReaderLC" );
    }

// ---------------------------------------------------------
// CSecModUIContainerCode::SetupListItemsL()
// Setups list items to listbox.
// ---------------------------------------------------------
//	
void CSecModUIContainerCode::SetupListItemsL()
    {
    LOG_ENTERFN( "CSecModUIContainerCode::SetupListItemsL" );
    CTextListBoxModel* model = iListBox->Model();
    //model->SetOwnershipType(
    CDesCArray* itemArray = STATIC_CAST(CDesCArray*, model->ItemTextArray());
    iModel.CheckCodeViewStringsL(*itemArray);
    LOG_LEAVEFN( "CSecModUIContainerCode::SetupListItemsL" );
    }    
 
// End of File  
