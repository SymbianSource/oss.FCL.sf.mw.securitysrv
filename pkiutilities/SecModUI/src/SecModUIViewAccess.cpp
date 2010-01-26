/*
* Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Implementation of the CSecModUIViewAccess class
*
*/


// INCLUDE FILES
#include  "SecModUIContainerAccess.h"
#include  "SecModUIViewAccess.h" 
#include  "SecModUIModel.h"
#include  "SecModUISyncWrapper.h"
#include  "secmodui.hrh"
#include  <SecModUI.rsg>
#include  <aknViewAppUi.h>
#include  <avkon.hrh>
#include  <aknlists.h>
#include  <eikmenup.h>
#include  <featmgr.h>

// CONSTANTS

// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CSecModUIViewAccess::CSecModUIViewAccess
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CSecModUIViewAccess::CSecModUIViewAccess(CSecModUIModel& aModel):
    CSecModUIViewBase(aModel)
    {
    }

// ---------------------------------------------------------
// CSecModUIViewAccess::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CSecModUIViewAccess::ConstructL()
    {
    BaseConstructL( R_SECMODUI_VIEW_ACCESS );
    }

// ---------------------------------------------------------
// CSecModUIViewAccess::~CSecModUIViewAccess()
// destructor
// ---------------------------------------------------------
//
CSecModUIViewAccess::~CSecModUIViewAccess()
    {
    
    }

// ---------------------------------------------------------
// CSecModUIViewCode::NewLC
// Two-phased constructor.
// ---------------------------------------------------------
//
CSecModUIViewAccess* CSecModUIViewAccess::NewLC(CSecModUIModel& aModel)
    {
    CSecModUIViewAccess* self = new (ELeave) CSecModUIViewAccess(aModel);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------
// TUid CSecModUIViewAccess::Id()
//
// ---------------------------------------------------------
//
TUid CSecModUIViewAccess::Id() const
    {
    return KSecModUIViewAccessId;
    }

// ---------------------------------------------------------
// void CSecModUIViewCode::CreateContainerL()
// Creates container
// ---------------------------------------------------------
//
void CSecModUIViewAccess::CreateContainerL()
    {
    iContainer = new (ELeave) CSecModUIContainerAccess(iModel, this);
    }

// ---------------------------------------------------------
// CSecModUIViewAccess::AddNavipaneLabelL()
// Default implementation is empty
// ---------------------------------------------------------
//
void CSecModUIViewAccess::AddNaviPaneLabelL()
    { 
    DoAddNaviPaneL(R_NAVIPANE_ACCESS_TEXT);   
    }

// ---------------------------------------------------------
// CSecModUIViewAccess::HandleCommandL(TInt aCommand)
// takes care of view command handling
// ---------------------------------------------------------
//
void CSecModUIViewAccess::HandleCommandL(TInt aCommand)
    {
    if (iModel.Wrapper().IsActive())
        {
        return;
        }   
    switch ( aCommand )
        {
        case ESecModUICmdChange:
            {
            HandleChangeL();            
            break;
            }
        case ESecModUICmdUnblock:
            {
            iModel.UnblockPinL(KPinGIndex);
            UpdateListBoxItemL(EIndexCodeStatus);
            UpdateListBoxItemL(EIndexCodeRequest);
            UpdateCbaL();
            break;
            }
        case ESecModUICmdClose:
            {
            iModel.CloseAuthObjL(KPinGIndex);
            UpdateListBoxItemL(EIndexCodeStatus);
            UpdateCbaL();
            break;
            }
        case EAknSoftkeyChange:
            {
            HandleChangeL();	
            break;	
            }
        case EAknSoftkeyClose:
            {
            iModel.CloseAuthObjL(KPinGIndex);
            UpdateListBoxItemL(EIndexCodeStatus);
            UpdateCbaL();
            break;	
            }
            
        case EAknSoftkeyUnlock:
            {
            iModel.UnblockPinL(KPinGIndex);
            UpdateListBoxItemL(EIndexCodeStatus);
            UpdateListBoxItemL(EIndexCodeRequest);
            UpdateCbaL();
            break;	
            }
        default:
            {
            CSecModUIViewBase::HandleCommandL( aCommand );
            break;
            }
        }
    }

// ---------------------------------------------------------
// CSecModUIViewAccess::HandleChangeL()
// takes care of Change command handling
// ---------------------------------------------------------
//
void CSecModUIViewAccess::HandleChangeL()
    {
    TInt currentItem = iContainer->ListBox().CurrentItemIndex();
    
    switch (currentItem)
        {
        case EIndexCodeLabel:
            {
            iModel.ChangeOrUnblockPinL(KPinGIndex);
            UpdateListBoxItemL(EIndexCodeStatus);
            UpdateCbaL();
            break;
            }
        case EIndexCodeRequest:
            {
            if (KErrCancel != iModel.ChangeCodeRequestL(KPinGIndex))
                {
                UpdateListBoxItemL(EIndexCodeRequest);
                UpdateListBoxItemL(EIndexCodeStatus);
                UpdateCbaL();
                }
            break;
            }
        default:
            {
            break;
            }                
        }
    }
// ---------------------------------------------------------
// CSecModUIViewAccess::HandleForegroundEventL()
// ---------------------------------------------------------
//
void CSecModUIViewAccess::HandleForegroundEventL(TBool aForeground)
    {
	if( aForeground && !iModel.Wrapper().IsActive() )
	    {
	    UpdateListBoxItemL(EIndexCodeRequest);
        UpdateListBoxItemL(EIndexCodeStatus);
        UpdateCbaL();	
	    }
    }


// ---------------------------------------------------------
// CSecModUIViewAccess::UpdateListBoxItemL(TInt aIndex)
// Updates listbox item
// ---------------------------------------------------------
//
void CSecModUIViewAccess::UpdateListBoxItemL(TInt aIndex)
    {
    TBuf<KMaxSettItemSize> item; 
        CDesCArray* itemArray = STATIC_CAST(CDesCArray*, 
            iContainer->ListBox().Model()->ItemTextArray());
    switch(aIndex)
        {
        case EIndexCodeRequest:
            {
            iModel.PINRequestItemL(KPinGIndex, item);
            break;
            }
        case EIndexCodeStatus:
            {
            iModel.PINStatusItemL(KPinGIndex, item, EFalse);
            break;
            }
        default: 
            {
            return;
            }
        }
    itemArray->Delete(aIndex);    
    itemArray->InsertL(aIndex, item);
    iContainer->ListBox().HandleItemAdditionL();
    iContainer->DrawNow();
    }

// ---------------------------------------------------------
// CSecModUIViewAccess::UpdateCbaL()
// Updates Cba area
// ---------------------------------------------------------
//    
void CSecModUIViewAccess::UpdateCbaL()
    {
    TInt currentItem = iContainer->ListBox().CurrentItemIndex();
    switch (currentItem)
            {
            case EIndexCodeLabel:
                {
                 if( iModel.PinChangeable(KPinGIndex) )
                     {
                     SetCbaL(R_SECMODUI_SOFTKEYS_OPTIONS_BACK_CHANGE);
                     }   
                 else
                     {
                     if( !iModel.PinUnblockable(KPinGIndex) )
                         {
                         SetCbaL(R_SECMODUI_SOFTKEYS_OPTIONS_BACK);
                         } 
                     else
                         {
                         SetCbaL(R_SECMODUI_SOFTKEYS_OPTIONS_BACK_UNBLOCK);	
                         }     
                     }    	    
                break;
                }
            case EIndexCodeRequest:
                {
                if( iModel.PinRequestChangeable(KPinGIndex) )
                     {
                     SetCbaL(R_SECMODUI_SOFTKEYS_OPTIONS_BACK_CHANGE);
                     }   
                 else
                     {
                     if( !iModel.PinUnblockable(KPinGIndex) )
                         {
                         SetCbaL(R_SECMODUI_SOFTKEYS_OPTIONS_BACK);
                         } 
                     else
                         {
                         SetCbaL(R_SECMODUI_SOFTKEYS_OPTIONS_BACK_UNBLOCK);	
                         }     
                     }     
                break;
                }
            case EIndexCodeStatus:
                {
                if( !iModel.PinChangeable(KPinGIndex) && iModel.PinUnblockable(KPinGIndex))
                    {
                    SetCbaL(R_SECMODUI_SOFTKEYS_OPTIONS_BACK_UNBLOCK);	
                    }     
                else if(iModel.PinOpen(KPinGIndex))  
                    {
                	SetCbaL(R_SECMODUI_SOFTKEYS_OPTIONS_BACK_CLOSE);
                    }
                else
                    {
                    SetCbaL(R_SECMODUI_SOFTKEYS_OPTIONS_BACK);	
                    }    
                
                break;
                }
            default:
                {
                break;
                }
            }    	
    }


// ---------------------------------------------------------
// CSecModUIViewAccess::SetCbaL( TInt aCbaResourceId )
// Sets Cba area
// ---------------------------------------------------------
//  
void CSecModUIViewAccess::SetCbaL( TInt aCbaResourceId )
    {
	CEikButtonGroupContainer*  cba = Cba();
    cba->SetCommandSetL(aCbaResourceId);
    cba->DrawNow();
    }
    
    
// ---------------------------------------------------------
// TUid CSecModUIViewAccess::DynInitMenuPaneL(
//      TInt aResourceId, CEikMenuPane* aMenuPane)
//
// ---------------------------------------------------------
//
void CSecModUIViewAccess::DynInitMenuPaneL(
    TInt aResourceId, 
    CEikMenuPane* aMenuPane)
    {
    if( R_SECMODUI_VIEW_ACCESS_MENU == aResourceId )
        {        
        if( aResourceId == !FeatureManager::FeatureSupported( KFeatureIdHelp ) )
            {
            aMenuPane->DeleteMenuItem( EAknCmdHelp ); //remove help
            }     

        TInt currentItem = iContainer->ListBox().CurrentItemIndex();
        if( currentItem == EIndexCodeLabel )
            {
            if( !iModel.PinChangeable(KPinGIndex) )
                {
                aMenuPane->SetItemDimmed( ESecModUICmdChange, ETrue );
                }
            }
        else if( currentItem == EIndexCodeRequest )
            {
            if( !iModel.PinRequestChangeable(KPinGIndex) )
                {
                aMenuPane->SetItemDimmed( ESecModUICmdChange, ETrue );
                }
            }
        else // it is EIndexCodeStatus
            {
            aMenuPane->SetItemDimmed( ESecModUICmdChange, ETrue );
            }

        if( !iModel.PinUnblockable(KPinGIndex) )
            {
            aMenuPane->SetItemDimmed( ESecModUICmdUnblock, ETrue );
            }
        if( !iModel.PinOpen(KPinGIndex) )
            {
            aMenuPane->SetItemDimmed( ESecModUICmdClose, ETrue );
            }
        }
    }

// ---------------------------------------------------------
// CSecModUIViewAccess::HandleEnterKeyL(TInt aIndex)
//     
// ---------------------------------------------------------
//   
void CSecModUIViewAccess::HandleEnterKeyL(TInt aIndex)
    {
    switch(aIndex)
        {        
        case EIndexCodeLabel:
            {
            HandleChangeL();
            break;
            }
        case EIndexCodeRequest:
            {
            HandleChangeL();
            break;
            }
        case EIndexCodeStatus:
            {
            iModel.CloseAuthObjL(KPinGIndex);
            UpdateListBoxItemL(EIndexCodeStatus);
            UpdateCbaL();
            break;
            }
        default:
            {
            break;
            }
        }
    }

// ---------------------------------------------------------
// CSecModUIViewAccess::HandleListBoxEventL(
//     CEikListBox* aListBox, TListBoxEvent aEventType)
// ---------------------------------------------------------
//   
void CSecModUIViewAccess::HandleListBoxEventL(
    CEikListBox* aListBox, 
    TListBoxEvent aEventType)
    {
    if(iModel.Wrapper().IsActive())
        {
        return;	
        }
        
    TInt index = aListBox->CurrentItemIndex();

    switch( aEventType )
		{
		case EEventEnterKeyPressed:
		case EEventItemDoubleClicked:
		case EEventItemSingleClicked:
			{
			HandleEnterKeyL(index);
            break;
			}
        default:
            {
			break;
            }
		}           
    }


// ---------------------------------------------------------
// CSecModUIViewAccess::DoDeactivate()
// 
// ---------------------------------------------------------
//
void CSecModUIViewAccess::DoActivateL(
   const TVwsViewId& aPrevViewId,TUid /*aCustomMessageId*/,
   const TDesC8& /*aCustomMessage*/)
    {
  
    if (KNullUid == iPrevViewId.iViewUid)
        {
        iPrevViewId = aPrevViewId;
        }
    SetTitlePaneL();
    AddNaviPaneLabelL();
                    
    if (!iContainer)
        {
        CreateContainerL();
        iContainer->SetMopParent(this);
        iContainer->ConstructL(ClientRect());
        iContainer->ListBox().SetListBoxObserver(this);
        AppUi()->AddToStackL(*this, iContainer);
        }         
   iContainer->MakeVisible(ETrue);
   iContainer->SetRect(ClientRect());
   iContainer->ActivateL();
   
   UpdateCbaL();  
   }

// ---------------------------------------------------------
// CSecModUIViewAccess::DoDeactivate()
// 
// ---------------------------------------------------------
//
void CSecModUIViewAccess::DoDeactivate()
    {
    if ( iContainer )
        {
        AppUi()->RemoveFromViewStack( *this, iContainer );
        }
    
    delete iContainer;
    iContainer = NULL;    
    }    
    
// End of File

