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
#include <SecUi.rsg>
#include <e32base.h>
#include <AknQueryDialog.h>
#include "SecUiAutoLockSettingPage.h"
#include "secui.hrh"
#include <featmgr.h>
/**
*CAutolockQuery used in autolock period query
*/
class CAutolockQuery
	: public CAknNumberQueryDialog
	{
	public: // Constructors and destructors
		/**
		* C++ constructor.
		*/
		CAutolockQuery(TInt& aNumber,const TTone aTone = ENoTone);
	protected: // From base classes
		/**
		* From CCAknNumberQueryDialog 
		*/
		void PreLayoutDynInitL();
	};
//
// ----------------------------------------------------------
// CAutoLockSettingPage::CAutoLockSettingPage()
// C++ default constructor
// ----------------------------------------------------------
// 
CAutoLockSettingPage::CAutoLockSettingPage(TInt aResourceId, TInt& aCurrentSelectionItem, CDesCArrayFlat* aItemArray, TInt& aAutoLockValue) : 
					CAknRadioButtonSettingPage(aResourceId, aCurrentSelectionItem, aItemArray),iAutoLockValue(aAutoLockValue)

	{
        iOriginalIndex = aCurrentSelectionItem;
	}

//
// ----------------------------------------------------------
// CAutoLockSettingPage::ConstructL()
// Symbian OS default constructor
// ----------------------------------------------------------
// 
void CAutoLockSettingPage::ConstructL()
	{
	CAknRadioButtonSettingPage::ConstructL();
	const TSize screenSize = iCoeEnv->ScreenDevice()->SizeInPixels();
	FeatureManager::InitializeLibL();
	}

//
// ----------------------------------------------------------
// CAutoLockSettingPage::CAutoLockSettingPage()
// Destructor
// ----------------------------------------------------------
// 
CAutoLockSettingPage::~CAutoLockSettingPage()
	{
	FeatureManager::UnInitializeLib();
	}

//
// ----------------------------------------------------------
// CAutoLockSettingPage::ProcessCommandL()
// 
// ----------------------------------------------------------
// 
void CAutoLockSettingPage::ProcessCommandL(TInt aCommandId)
	{
	TInt cur = ListBoxControl()->CurrentItemIndex();
	// Respond to softkey events

	switch (aCommandId)
		{
		case EAknSoftkeySelect:
		case EAknSoftkeyOk:
			// autolock off
			if (cur == 0)
				{
				iAutoLockValue = 0;
				if(iOriginalIndex == 0)
				    { //User re-selected "Autolock off"; no use in changing lock setting
				        AttemptExitL(EFalse);
				    }
				else
				    {
				        AttemptExitL(ETrue);
				    }
				
				}
			// user defined
			if (cur == 1)
				{
				CAutolockQuery* dlg = new (ELeave) CAutolockQuery(iAutoLockValue);
				dlg->PrepareLC(R_AUTOLOCK_TIME_QUERY);
				if(FeatureManager::FeatureSupported(KFeatureIdSapTerminalControlFw ))
				{
				//set min and max values from SCP server to the dialog.
				TInt minimum = 0;
				if(iMaximum <= 0) //maximum value has not been defined; default value used instead.
				    iMaximum = 1440;
				dlg->SetMinimumAndMaximum(minimum, iMaximum);
			}
				if(dlg->RunLD())
					{
					AttemptExitL(ETrue);
					}
				else
					AttemptExitL(EFalse);
				}
			break;

		default:
			CAknSettingPage::ProcessCommandL(aCommandId);
			break;
		}

	}
//
// ----------------------------------------------------------
// CAutoLockSettingPage::SetPeriodMaximumValue()
// 
// ----------------------------------------------------------
// 
void CAutoLockSettingPage::SetPeriodMaximumValue(TInt aMaximumValue)
	{
	if(FeatureManager::FeatureSupported(KFeatureIdSapTerminalControlFw ))
	{
		iMaximum = aMaximumValue;
}
	}
	
//---------------------------------------------------------------------------------------
// CAutoLockSettingPage::HandlePointerEventL()
// Passes pointer events to CAknRadioButtonSettingPage
// 
//---------------------------------------------------------------------------------------
//

void CAutoLockSettingPage::HandlePointerEventL(const TPointerEvent& aPointerEvent)
    {
    if ( AknLayoutUtils::PenEnabled() )
        {
        	TInt cur = ListBoxControl()->CurrentItemIndex();
         	CAknRadioButtonSettingPage::HandlePointerEventL(aPointerEvent);
         	//only take into account the "Up" event. Otherwise we'll end up having 2 dialogs.
         	if(aPointerEvent.iType == TPointerEvent::EButton1Up)
         	{

         	    //only react to the event if the pen is actually inside the dialog.
         	    if(ListBoxControl()->Rect().Contains(aPointerEvent.iPosition))
         	        {
         	            // autolock off
        				if (cur == 0)
        					{
        					    iAutoLockValue = 0;
        						if(iOriginalIndex == 0)
                				    { //User re-selected "Autolock off"; no use in changing lock setting
                				        AttemptExitL(EFalse);
                				    }
                				else
                				    {
                				        AttemptExitL(ETrue);
                				    }
        					}
        				// user defined
        				if (cur == 1)
        					{
        					CAutolockQuery* dlg = new (ELeave) CAutolockQuery(iAutoLockValue);
						dlg->PrepareLC(R_AUTOLOCK_TIME_QUERY);
					if(FeatureManager::FeatureSupported(KFeatureIdSapTerminalControlFw ))
					{
        					//set min and max values from SCP server to the dialog.
        					TInt minimum = 0;			
				          if(iMaximum <= 0) //maximum value has not been defined; default value used instead.
				            iMaximum = 1440;        						
        					dlg->SetMinimumAndMaximum(minimum, iMaximum);
				}
        					if(dlg->RunLD())
        						{
        						AttemptExitL(ETrue);
        						}
        					else
        						AttemptExitL(EFalse);
         	        }
         		
				}
		    }
         	}
        }
//
// ---------------------------------------------------------
// CAutolockQuery::CAutolockQuery()
// 
// ---------------------------------------------------------
//
CAutolockQuery::CAutolockQuery(TInt& aNumber,const TTone aTone)
	: CAknNumberQueryDialog( aNumber, aTone)
	{
	}
	
//
// ---------------------------------------------------------
// CAutolockQuery::PreLayoutDynInitL()
// 
// ---------------------------------------------------------
//
void CAutolockQuery::PreLayoutDynInitL()
	{
	CAknNumberQueryDialog::PreLayoutDynInitL();
	if (iNumber == 0)
		{
		MakeLeftSoftkeyVisible(EFalse);
		}
	}
    
// End of file

