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
* Description:  RemoteLock (on/off) setting page
*
*
*/

#include <aknnotewrappers.h>
#include <eikmenub.h>
#include <SecUi.rsg>
#include <e32base.h>

#include "SecUiRemoteLockSettingPage.h"
#include "secui.hrh"



// ----------------------------------------------------------
// CRemoteLockSettingPage::CRemoteLockSettingPage()
// C++ default constructor
// ----------------------------------------------------------
CRemoteLockSettingPage::CRemoteLockSettingPage( TInt aResourceId, TInt& aCurrentSelectionItem, CDesCArrayFlat* aItemArray )
: CAknRadioButtonSettingPage( aResourceId, aCurrentSelectionItem, aItemArray ),
  iRemoteLockStatus( aCurrentSelectionItem )
    {
    #ifdef _DEBUG
    RDebug::Print( _L( "(SecUi)CRemoteLockSettingPage::CRemoteLockSettingPage()" ) );
    #endif // DEBUG
    iPrevSelectionItem = aCurrentSelectionItem;
    }



// ----------------------------------------------------------
// CRemoteLockSettingPage::ConstructL()
// Symbian OS default constructor
// ----------------------------------------------------------
void CRemoteLockSettingPage::ConstructL()
    {
    #ifdef _DEBUG
    RDebug::Print( _L( "(SecUi)CRemoteLockSettingPage::ConstructL() - Enter" ) );
    #endif // DEBUG

    CAknRadioButtonSettingPage::ConstructL();
    const TSize screenSize = iCoeEnv->ScreenDevice()->SizeInPixels();
    iPopoutCba = CEikButtonGroupContainer::NewL( CEikButtonGroupContainer::ECba, CEikButtonGroupContainer::EHorizontal, this, R_AVKON_SOFTKEYS_OK_CANCEL__OK );
    iPopoutCba->SetBoundingRect( TRect( screenSize ) );

    #ifdef _DEBUG
    RDebug::Print( _L( "(SecUi)CRemoteLockSettingPage::ConstructL() - Exit" ) );
    #endif // DEBUG

    return;
    }



// ----------------------------------------------------------
// CRemoteLockSettingPage::~CRemoteLockSettingPage()
// Destructor
// ----------------------------------------------------------
CRemoteLockSettingPage::~CRemoteLockSettingPage()
    {
    #ifdef _DEBUG
    RDebug::Print( _L( "(SecUi)CRemoteLockSettingPage::~CRemoteLockSettingPage() - Enter" ) );
    #endif // DEBUG

    delete iPopoutCba;
    iPopoutCba = NULL;

    #ifdef _DEBUG
    RDebug::Print( _L( "(SecUi)CRemoteLockSettingPage::~CRemoteLockSettingPage() - Exit" ) );
    #endif // DEBUG
    }



// ----------------------------------------------------------
// CRemoteLockSettingPage::ProcessCommandL()
// 
// ----------------------------------------------------------
void CRemoteLockSettingPage::ProcessCommandL( TInt aCommandId )
    {
    #ifdef _DEBUG
    RDebug::Print( _L( "(SecUi)CRemoteLockSettingPage::ProcessCommandL() - Enter" ) );
    #endif // DEBUG

	TInt currentItem = ListBoxControl()->CurrentItemIndex();
	// Respond to softkey events

	switch ( aCommandId )
		{
		case EAknSoftkeySelect:
		case EAknSoftkeyOk:
		    {
		    switch ( currentItem )
		        {
		        case KRemoteLockSettingItemOn: // RemoteLock on
		            iRemoteLockStatus = KRemoteLockSettingItemOn;
		            AttemptExitL( ETrue );
		            break;

		        case KRemoteLockSettingItemOff: // RemoteLock off
		            iRemoteLockStatus = KRemoteLockSettingItemOff;
		            AttemptExitL( ETrue );
		            break;

                default:
                    // Do nothing
                    break;
                }
			break;
		    }

		default:
			CAknSettingPage::ProcessCommandL( aCommandId );
			break;
		}

    #ifdef _DEBUG
    RDebug::Print( _L( "(SecUi)CRemoteLockSettingPage::ProcessCommandL() - Exit" ) );
    #endif // DEBUG

    return;
	}
//---------------------------------------------------------------------------------------
// CRemoteLockSettingPage::HandlePointerEventL()
// Passes pointer events to CAknRadioButtonSettingPage
// 
//---------------------------------------------------------------------------------------
//

void CRemoteLockSettingPage::HandlePointerEventL(const TPointerEvent& aPointerEvent)
    {
    if ( AknLayoutUtils::PenEnabled() )
        {       	        	
            TInt currentItem = ListBoxControl()->CurrentItemIndex();
            CAknRadioButtonSettingPage::HandlePointerEventL(aPointerEvent);
            
    		if(aPointerEvent.iType == TPointerEvent::EButton1Up)
    		    {
		    if(iPrevSelectionItem!=currentItem)
			iPrevSelectionItem=currentItem;
		    else
		    {
    		    switch ( currentItem )
    		        {
    		        case KRemoteLockSettingItemOn: // RemoteLock on
    		            iRemoteLockStatus = KRemoteLockSettingItemOn;
    		            AttemptExitL( ETrue );
    		            break;

    		        case KRemoteLockSettingItemOff: // RemoteLock off
    		            iRemoteLockStatus = KRemoteLockSettingItemOff;
    		            AttemptExitL( ETrue );
    		            break;

                    default:
                        // Do nothing
                        break;
                    }
                    }
    		    }
    		    
        }
    }


// End of file

