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
* Description:   Implementation of the CSecModUIViewCode class
*
*/


// INCLUDE FILES
#include  "secmodui.hrh"
#include  "SecModUIViewCode.h"
#include  "SecModUIContainerCode.h" 
#include  "SecModUIModel.h"
#include  "SecModUILogger.h"
#include  "SecModUISyncWrapper.h"
#include  <aknViewAppUi.h>
#include  <avkon.hrh>
#include  <aknlists.h>
#include  <SecModUI.rsg>

// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CSecModUIViewCode::CSecModUIViewCode
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CSecModUIViewCode::CSecModUIViewCode(CSecModUIModel& aModel):CSecModUIViewBase(aModel)
    {
    }

// ---------------------------------------------------------
// CSecModUIViewCode::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CSecModUIViewCode::ConstructL()
    {
    LOG_ENTERFN("CSecModUIViewCode::ConstructL()");     
    BaseConstructL( R_SECMODUI_VIEW_CODE );
    LOG_LEAVEFN("CSecModUIViewCode::ConstructL()");        
    }

// ---------------------------------------------------------
// CSecModUIViewCode::~CSecModUIViewCode()
// destructor
// ---------------------------------------------------------
//
CSecModUIViewCode::~CSecModUIViewCode()
    {  
    LOG_ENTERFN("CSecModUIViewCode::~CSecModUIViewCode()");     
    LOG_LEAVEFN("CSecModUIViewCode::~CSecModUIViewCode()");        
    }

// -----------------------------------------------------------------------------
// CSecModUIViewCode::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSecModUIViewCode* CSecModUIViewCode::NewLC(CSecModUIModel& aModel)
    {
    LOG_ENTERFN("CSecModUIViewCode::NewLC()");   
    CSecModUIViewCode* self = new (ELeave) CSecModUIViewCode(aModel);
    CleanupStack::PushL(self);
    self->ConstructL();
    LOG_LEAVEFN("CSecModUIViewCode::NewLC()");        
    return self;
    }

// ---------------------------------------------------------
// TUid CSecModUIViewCode::Id()
// 
// ---------------------------------------------------------
//
TUid CSecModUIViewCode::Id() const
    {
    return KSecModUIViewCodeId;
    }

// ---------------------------------------------------------
// TUid CSecModUIViewCode::CreateContainerL()
// Creates container
// ---------------------------------------------------------
//
void CSecModUIViewCode::CreateContainerL()
    {
    LOG_ENTERFN("CSecModUIViewCode::CreateContainerL()");   
    iContainer = new (ELeave) CSecModUIContainerCode(iModel);
    LOG_LEAVEFN("CSecModUIViewCode::CreateContainerL()");        
    }

// ---------------------------------------------------------
// CSecModUIViewCode::HandleCommandL(TInt aCommand)
// takes care of view command handling
// ---------------------------------------------------------
//
void CSecModUIViewCode::HandleCommandL(TInt aCommand)
    {   
    LOG_ENTERFN("CSecModUIViewCode::HandleCommandL()");   
    if (iModel.Wrapper().IsActive())
        {
        return;
        }
    switch ( aCommand )
        {
        case ESecModUICmdOpen:
            {
            iModel.OpenAuthObjViewL(iContainer->ListBox().CurrentItemIndex());
            break;
            }
        case ESecModUICmdModuleInfo:
            {
            iModel.ViewOpenedSecModDetailsL();
            break;
            }
        case EAknSoftkeyOpen:
            {
            iModel.OpenAuthObjViewL(iContainer->ListBox().CurrentItemIndex());
            break;	
            }
        default:
            {
            CSecModUIViewBase::HandleCommandL(aCommand);
            break;
            }
        }
    LOG_LEAVEFN("CSecModUIViewCode::HandleCommandL()");        
    }
    


// ---------------------------------------------------------
// CSecModUIViewBase::DoActivateL(...)
// 
// ---------------------------------------------------------
//
/*void CSecModUIViewCode::DoActivateL(
   const TVwsViewId& aPrevViewId,TUid aCustomMessageId,
   const TDesC8& aCustomMessage)
    {
    LOG_ENTERFN("CSecModUIViewCode::DoActivateL()");   
    if (KNullUid == iPrevViewId.iViewUid)
        {
        iPrevViewId = aPrevViewId;
        }
    SetTitlePaneL();
    AddNaviPaneLabelL();
    
    if (aPrevViewId.iViewUid == KSecModUIViewMainId)
        {
        AppUi()->RemoveFromViewStack( *this, iContainer );
        delete iContainer;
        iContainer = NULL;
        }
                    
    if (!iContainer)
        {
        CreateContainerL();
        iContainer->SetMopParent(this);
        iContainer->ConstructL(ClientRect());
        iContainer->ListBox().SetListBoxObserver(this);        
        }         
   AppUi()->AddToStackL(*this, iContainer);            
   iContainer->MakeVisible(ETrue);
   iContainer->SetRect(ClientRect());
   iContainer->ActivateL();
     
   LOG_LEAVEFN("CSecModUIViewCode::DoActivateL()");    
   }*/

// ---------------------------------------------------------
// CSecModUIViewBase::DoDeactivate()
// 
// ---------------------------------------------------------
//
/*void CSecModUIViewCode::DoDeactivate()
    {
    LOG_ENTERFN("CSecModUIViewCode::DoDeactivate()");   
    if ( iContainer )
        {
        AppUi()->RemoveFromViewStack( *this, iContainer );
        }
        
    LOG_LEAVEFN("CSecModUIViewCode::DoDeactivate()");    
    }*/

// ---------------------------------------------------------
// CSecModUIViewCode::HandleListBoxEventL(
//     CEikListBox* aListBox, TListBoxEvent aEventType)
// ---------------------------------------------------------
//    
void CSecModUIViewCode::HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType)
    {
    LOG_ENTERFN("CSecModUIViewCode::HandleListBoxEventL()");   
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
			iModel.OpenAuthObjViewL(aListBox->CurrentItemIndex());
            break;
			}
        default:
            {
			break;
            }
		}           
    LOG_LEAVEFN("CSecModUIViewCode::HandleListBoxEventL()");        		
    }

// End of File
