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
* Description:   Implementation of the CSecModUIViewMain class
*
*/


// INCLUDE FILES
#include  "secmodui.hrh"
#include  <SecModUI.rsg>
#include  "SecModUIViewMain.h"
#include  "SecModUIContainerMain.h" 
#include  "SecModUIModel.h"
#include  "SecModUISyncWrapper.h"
#include  "SecModUILogger.h"
#include  <SecModUI.rsg>
#include  <aknViewAppUi.h>
#include  <avkon.hrh>
#include  <aknlists.h>
#include  <eikmenup.h>
#include  <StringLoader.h> 
#include  <badesca.h>
#include  <featmgr.h>
// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CSecModUIViewMain::CSecModUIViewMain
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CSecModUIViewMain::CSecModUIViewMain(CSecModUIModel& aModel):
    CSecModUIViewBase(aModel)
    {
    //LOG_CREATE;
    LOG_WRITE( "CSecModUIViewMain::CSecModUIViewMain" );
    }

// ---------------------------------------------------------
// CSecModUIViewMain::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CSecModUIViewMain::ConstructL()
    {
    LOG_ENTERFN("CSecModUIViewMain::ConstructL()");
    BaseConstructL( R_SECMODUI_VIEW_MAIN );
    LOG_LEAVEFN("CSecModUIViewMain::ConstructL()");
    }

// ---------------------------------------------------------
// TUid CSecModUIViewMain::CreateContainerL()
// Creates container
// ---------------------------------------------------------
//
void CSecModUIViewMain::CreateContainerL()
    {
    LOG_ENTERFN("CSecModUIViewMain::CreateContainerL()");
    iContainer = new (ELeave) CSecModUIContainerMain(iModel);
    LOG_LEAVEFN("CSecModUIViewMain::CreateContainerL()");
    }
// ---------------------------------------------------------
// CSecModUIViewMain::~CSecModUIViewMain()
// destructor
// ---------------------------------------------------------
//
CSecModUIViewMain::~CSecModUIViewMain()
    {
    LOG_WRITE( "CSecModUIViewMain::~CSecModUIViewMain" );    
    }

// -----------------------------------------------------------------------------
// CSecModUIViewMain::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSecModUIViewMain* CSecModUIViewMain::NewLC(CSecModUIModel& aModel)
    {
    LOG_ENTERFN("CSecModUIViewMain::NewLC()");
    CSecModUIViewMain* self = new (ELeave) CSecModUIViewMain(aModel);
    CleanupStack::PushL(self);
    self->ConstructL();
    LOG_LEAVEFN("CSecModUIViewMain::NewLC()");
    return self;
    }
    
// ---------------------------------------------------------
// TUid CSecModUIViewMain::Id()
//
// ---------------------------------------------------------
//
TUid CSecModUIViewMain::Id() const
    {
    return KSecModUIViewMainId;
    }

// ---------------------------------------------------------
// TUid CSecModUIViewMain::DynInitMenuPaneL(
//      TInt aResourceId, CEikMenuPane* aMenuPane)
//
// ---------------------------------------------------------
//
void CSecModUIViewMain::DynInitMenuPaneL(
    TInt aResourceId, 
    CEikMenuPane* aMenuPane)
    {
    LOG_ENTERFN("CSecModUIViewMain::DynInitMenuPaneL()");
        
    if (aResourceId == R_SECMODUI_VIEW_MAIN_MENU)
        {
        if( !FeatureManager::FeatureSupported( KFeatureIdHelp ) )
            {
            aMenuPane->DeleteMenuItem( EAknCmdHelp ); //remove help
            }
        TInt currentItem = iContainer->ListBox().CurrentItemIndex();
        // The security module list is empty.  
        // All except Exit and Help is dimmed.
        if (currentItem == -1)
            {
            aMenuPane->SetItemDimmed( ESecModUICmdOpen, ETrue );
            aMenuPane->SetItemDimmed( ESecModUICmdModuleInfo, ETrue );
            aMenuPane->SetItemDimmed( ESecModUICmdDelKeyStore, ETrue );
            }
        else if (!iModel.IsTokenDeletable(currentItem))
            {
            aMenuPane->SetItemDimmed( ESecModUICmdDelKeyStore, ETrue ); 
            }
         else
            {
            }
        }
    
    LOG_LEAVEFN("CSecModUIViewMain::DynInitMenuPaneL()");
    }

// ---------------------------------------------------------
// CSecModUIViewMain::HandleCommandL(TInt aCommand)
// takes care of view command handling
// ---------------------------------------------------------
//
void CSecModUIViewMain::HandleCommandL(TInt aCommand)
    {  
    LOG_ENTERFN("CSecModUIViewMain::HandleCommandL()"); 
    
    if(iModel.Wrapper().IsActive())
        {
        return;	
        }
    
    switch ( aCommand )
        {
        case ESecModUICmdOpen:
            {
            iModel.OpenTokenL(iContainer->ListBox().CurrentItemIndex());
            break;
            }
        case ESecModUICmdDelKeyStore:
            {
            if (iModel.DeleteKeyStoreL(iContainer->ListBox().CurrentItemIndex()))            
                {
                CDesCArray* itemArray = STATIC_CAST(
                CDesCArray*, iContainer->ListBox().Model()->ItemTextArray());
                itemArray->Reset();                
                iModel.LoadTokenLabelsL(iContainer->ListBox());
                iContainer->DrawNow(); 
                UpdateCbaL();          
                }
            break;
            }
        case ESecModUICmdModuleInfo:
            {
            iModel.ViewSecModDetailsL(iContainer->ListBox().CurrentItemIndex());
            break;
            } 
        case EAknSoftkeyOpen:
            {
            iModel.OpenTokenL(iContainer->ListBox().CurrentItemIndex());
            break;	
            }
        default:
            {
            CSecModUIViewBase::HandleCommandL(aCommand);
            break;
            }
        }
    LOG_LEAVEFN("CSecModUIViewMain::HandleCommandL()");
    }

// ---------------------------------------------------------
// CSecModUIViewMain::SetTitlePaneL()
// 
// ---------------------------------------------------------
//
void CSecModUIViewMain::SetTitlePaneL()
    {
    LOG_ENTERFN("SecModUIViewMain::SetTitlePaneL()");
	HBufC* titleText = StringLoader::LoadLC(R_QTN_WIM_TITLE);
	iModel.ActivateTitleL(*titleText);
	CleanupStack::PopAndDestroy(titleText);
 	LOG_LEAVEFN("SecModUIViewMain::SetTitlePaneL()");
    }

// ---------------------------------------------------------
// CSecModUIViewMain::DoActivateL(...)
// 
// ---------------------------------------------------------
//
void CSecModUIViewMain::DoActivateL(
   const TVwsViewId& aPrevViewId,TUid aCustomMessageId,
   const TDesC8& aCustomMessage)
   {
   LOG_ENTERFN("CSecModUIViewMain::DoActivateL()");   
   CSecModUIViewBase::DoActivateL(
       aPrevViewId, aCustomMessageId, aCustomMessage);
   UpdateCbaL();    
   LOG_LEAVEFN("CSecModUIViewMain::DoActivateL()");    
   }

// ---------------------------------------------------------
// CSecModUIContainerMain::HandleListBoxEventL(
//     CEikListBox* aListBox, TListBoxEvent aEventType)
// ---------------------------------------------------------
//    
void CSecModUIViewMain::HandleListBoxEventL(
    CEikListBox* aListBox, 
    TListBoxEvent aEventType)
    {
    LOG_ENTERFN("CSecModUIViewMain::HandleListBoxEventL()");   
      
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
			iModel.OpenTokenL(aListBox->CurrentItemIndex());
            break;
			}
        default:
            {
			break;
            }
		} 
	LOG_LEAVEFN("CSecModUIViewMain::HandleListBoxEventL()");   	          
    }


// ---------------------------------------------------------
// CSecModUIViewMain::UpdateCbaL()
// ---------------------------------------------------------
//
void CSecModUIViewMain::UpdateCbaL()
    {
    CDesCArray* itemArray = STATIC_CAST(
                CDesCArray*, iContainer->ListBox().Model()->ItemTextArray());
    if(itemArray->Count() == 0 )
       {
       SetCbaL(R_SECMODUI_SOFTKEYS_OPTIONS_BACK);	
       }
    else
       {
       SetCbaL(R_SECMODUI_SOFTKEYS_OPTIONS_BACK_OPEN);	
       }           
    }
    
// ---------------------------------------------------------
// CSecModUIViewMain::SetCbaL( TInt aCbaResourceId )
// Sets Cba area
// ---------------------------------------------------------
//  
void CSecModUIViewMain::SetCbaL( TInt aCbaResourceId )
    {
	CEikButtonGroupContainer*  cba = Cba();
    cba->SetCommandSetL(aCbaResourceId);
    cba->DrawDeferred();
    }
    
// ---------------------------------------------------------
// CSecModUIViewMain::HandleForegroundEventL()
// ---------------------------------------------------------
//
void CSecModUIViewMain::HandleForegroundEventL(TBool aForeground)
    {
	if( aForeground && !iModel.Wrapper().IsActive() )
	    {
	    iModel.InitializeKeyStoreL();          
        CDesCArray* itemArray = STATIC_CAST(CDesCArray*, iContainer->ListBox().Model()->ItemTextArray());
        itemArray->Reset();                
        iModel.LoadTokenLabelsL(iContainer->ListBox());
        iContainer->DrawDeferred(); 
        UpdateCbaL();                	
	    }
    }    
// End of File

