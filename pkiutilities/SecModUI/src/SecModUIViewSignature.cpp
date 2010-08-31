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
* Description:   Implementation of the CSecModUIViewSignature class
*
*/


// INCLUDE FILES
#include  "secmodui.hrh"
#include  "SecModUIViewSignature.h"
#include  "SecModUIContainerSignature.h" 
#include  "SecModUIModel.h"
#include  "SecModUISyncWrapper.h"
#include  <aknViewAppUi.h>
#include  <avkon.hrh>
#include  <SecModUI.rsg>
#include  <aknlists.h>
#include  <eikmenup.h>
#include  <featmgr.h>

// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CSecModUIViewMain::CSecModUIViewMain
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CSecModUIViewSignature::CSecModUIViewSignature(CSecModUIModel& aModel):CSecModUIViewBase(aModel)
    {
    }

// ---------------------------------------------------------
// CSecModUIViewSignature::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CSecModUIViewSignature::ConstructL()
    {
    BaseConstructL( R_SECMODUI_VIEW_SIGNATURE );
    }

// ---------------------------------------------------------
// CSecModUIViewSignature::~CSecModUIViewSignature()
// destructor
// ---------------------------------------------------------
//
CSecModUIViewSignature::~CSecModUIViewSignature()
    {
   
    }

// -----------------------------------------------------------------------------
// CSecModUIViewSignature::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSecModUIViewSignature* CSecModUIViewSignature::NewLC(CSecModUIModel& aModel)
    {
    CSecModUIViewSignature* self = new (ELeave) CSecModUIViewSignature(aModel);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------
// void CSecModUIViewSignature::CreateContainerL()
// Creates container
// ---------------------------------------------------------
//
void CSecModUIViewSignature::CreateContainerL()
    {
    iContainer = new (ELeave) CSecModUIContainerSignature(iModel, this);
    }

// ---------------------------------------------------------
// CSecModUIViewSignature::AddNavipaneLabelL()
// Default implementation is empty
// ---------------------------------------------------------
//
void CSecModUIViewSignature::AddNaviPaneLabelL()
    { 
    DoAddNaviPaneL(R_NAVIPANE_SIGNATURE_TEXT);   
    }

// ---------------------------------------------------------
// TUid CSecModUIViewSignature::Id()
//
// ---------------------------------------------------------
//
TUid CSecModUIViewSignature::Id() const
    {
    return KSecModUIViewSignatureId;
    }

// ---------------------------------------------------------
// CSecModUIViewSignature::HandleCommandL(TInt aCommand)
// takes care of view command handling
// ---------------------------------------------------------
//
void CSecModUIViewSignature::HandleCommandL(TInt aCommand)
    {
    if (iModel.Wrapper().IsActive())
        {
        return;
        }  
    switch ( aCommand )
        {
        case ESecModUICmdChange:
            {
            iModel.ChangePinNrL(iContainer->ListBox().CurrentItemIndex());
            iModel.LoadPinNRLabelsL(iContainer->ListBox(), EFalse);
            iContainer->DrawNow();
            UpdateCbaL(iContainer->ListBox().CurrentItemIndex());
            break;
            }            
        case ESecModUICmdUnblock:
            {
            iModel.UnblockPinNrL(iContainer->ListBox().CurrentItemIndex());
            iModel.LoadPinNRLabelsL(iContainer->ListBox(), EFalse);
            iContainer->DrawNow();
            UpdateCbaL(iContainer->ListBox().CurrentItemIndex());
            break;
            }
        case EAknSoftkeyChange:
            {
            iModel.ChangePinNrL(iContainer->ListBox().CurrentItemIndex());
            iModel.LoadPinNRLabelsL(iContainer->ListBox(), EFalse);
            iContainer->DrawNow();
            UpdateCbaL(iContainer->ListBox().CurrentItemIndex());	
            break;	
            }
        case EAknSoftkeyUnlock:
            {
            iModel.UnblockPinNrL(iContainer->ListBox().CurrentItemIndex());
            iModel.LoadPinNRLabelsL(iContainer->ListBox(), EFalse);
            iContainer->DrawNow();
            UpdateCbaL(iContainer->ListBox().CurrentItemIndex());
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
// TUid CSecModUIViewSignature::DynInitMenuPaneL(
//      TInt aResourceId, CEikMenuPane* aMenuPane)
//
// ---------------------------------------------------------
//
void CSecModUIViewSignature::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane)
    {
    TInt currentItem = iContainer->ListBox().CurrentItemIndex();
    switch(aResourceId)
        {
        case R_SECMODUI_VIEW_SIGNATURE_MENU:
            {
            if( aResourceId == !FeatureManager::FeatureSupported( KFeatureIdHelp ) )
                {
                aMenuPane->DeleteMenuItem( EAknCmdHelp ); //remove help
                } 
            // The security module list is empty.  
            // All except Exit and Help is dimmed.
            if (!iModel.PinNrChangeable(currentItem))
                {
                aMenuPane->SetItemDimmed( ESecModUICmdChange, ETrue );
                }
            if (!iModel.PinNrUnblockable(currentItem))
                {
                aMenuPane->SetItemDimmed( ESecModUICmdUnblock, ETrue );
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
// CSecModUIViewSignature::HandleListBoxEventL(
//     CEikListBox* aListBox, TListBoxEvent aEventType)
// ---------------------------------------------------------
//     
void CSecModUIViewSignature::HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType)
    {
    if(iModel.Wrapper().IsActive())
        {
        return;	
        }
        
    switch( aEventType )
		{
		case EEventEnterKeyPressed:
		case EEventItemDoubleClicked:
		case EEventItemSingleClicked:
			{			
			iModel.ChangeOrUnblockPinNrL(aListBox->CurrentItemIndex());
			iModel.LoadPinNRLabelsL(iContainer->ListBox(), EFalse);
            iContainer->DrawNow();
            UpdateCbaL(aListBox->CurrentItemIndex());
            break;
			}
        default:
            {
			break;
            }
		}           
    }


// ---------------------------------------------------------
// CSecModUIViewSignature::UpdateCbaL()
//     
// ---------------------------------------------------------
//
void CSecModUIViewSignature::UpdateCbaL( TInt aIndex )
    {
    if( iModel.PinNrChangeable(aIndex) )
        {
        SetCbaL(R_SECMODUI_SOFTKEYS_OPTIONS_BACK_CHANGE);
        }
    else
        {
        if( !iModel.PinNrUnblockable(aIndex) )
            {
            SetCbaL(R_SECMODUI_SOFTKEYS_OPTIONS_BACK);
            } 
        else
            {
            SetCbaL(R_SECMODUI_SOFTKEYS_OPTIONS_BACK_UNBLOCK);	
            }     
        }    	
    }


// ---------------------------------------------------------
// CSecModUIViewSignature::SetCbaL()
//     
// ---------------------------------------------------------
// 
void CSecModUIViewSignature::SetCbaL( TInt aCbaResourceId )
    {
	CEikButtonGroupContainer*  cba = Cba();
    cba->SetCommandSetL(aCbaResourceId);
    cba->DrawNow();
    }


// ---------------------------------------------------------
// CSecModUIViewSignature::DoActivateL()
//     
// ---------------------------------------------------------
//     
void CSecModUIViewSignature::DoActivateL(
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
   
   UpdateCbaL(iContainer->ListBox().CurrentItemIndex());
       
   }

// ---------------------------------------------------------
// CSecModUIViewSignature::DoDeactivate()
// 
// ---------------------------------------------------------
//
void CSecModUIViewSignature::DoDeactivate()
    {
 
    if ( iContainer )
        {
        AppUi()->RemoveFromViewStack( *this, iContainer );
        }
    
    delete iContainer;
    iContainer = NULL;
        
    }  
           
// End of File

