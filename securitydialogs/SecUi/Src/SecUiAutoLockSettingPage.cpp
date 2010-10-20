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
* Description:  Autolock period settingpage
*
*
*/

#include <eikmenub.h>
#include <e32base.h>
#include "SecUiAutoLockSettingPage.h"
#include "secui.hrh"
#include <featmgr.h>


//
// ----------------------------------------------------------
// CAutoLockSettingPage::CAutoLockSettingPage()
// C++ default constructor
// ----------------------------------------------------------
// 
CAutoLockSettingPage::CAutoLockSettingPage(TInt aResourceId, TInt& aCurrentSelectionItem, CDesCArrayFlat* aItemArray, TInt& aAutoLockValue) : 
					CBase(),iAutoLockValue(aAutoLockValue)

	{
		aResourceId = aResourceId;
		aCurrentSelectionItem = aCurrentSelectionItem;
		aItemArray = aItemArray;
		aAutoLockValue = aAutoLockValue;
	}

//
// ----------------------------------------------------------
// CAutoLockSettingPage::ConstructL()
// Symbian OS default constructor
// ----------------------------------------------------------
// 
void CAutoLockSettingPage::ConstructL()
	{
	}

//
// ----------------------------------------------------------
// CAutoLockSettingPage::CAutoLockSettingPage()
// Destructor
// ----------------------------------------------------------
// 
CAutoLockSettingPage::~CAutoLockSettingPage()
	{
	}

//
// ----------------------------------------------------------
// CAutoLockSettingPage::ProcessCommandL()
// 
// ----------------------------------------------------------
// 
void CAutoLockSettingPage::ProcessCommandL(TInt aCommandId)
	{
		aCommandId = aCommandId;
	}
//
// ----------------------------------------------------------
// CAutoLockSettingPage::SetPeriodMaximumValue()
// 
// ----------------------------------------------------------
// 
void CAutoLockSettingPage::SetPeriodMaximumValue(TInt aMaximumValue)
	{
		aMaximumValue = aMaximumValue;
	}
	
//---------------------------------------------------------------------------------------
// CAutoLockSettingPage::HandlePointerEventL()
// Passes pointer events to CAknRadioButtonSettingPage
// 
//---------------------------------------------------------------------------------------
//

void CAutoLockSettingPage::HandlePointerEventL(const TPointerEvent& aPointerEvent)
    {
    	(void)aPointerEvent;
    }
// End of file

