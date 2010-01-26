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
* Description:   Implementation of the CSecModUIViewBase class
*
*/


// INCLUDE FILES
#include  "SecModUIViewBase.h"
#include  "SecModUIContainerBase.h" 
#include  "SecModUIModel.h"
#include  "SecModUILogger.h"
#include  "secmodui.hrh"
#include  <SecModUI.rsg>
#include  <aknViewAppUi.h>
#include  <avkon.hrh>
#include  <eiktxlbx.h>
#include  <aknnavi.h>
#include  <aknnavide.h> 
#include  <barsread.h>
#include    <hlplch.h>   // For HlpLauncher 
#include  <featmgr.h>


// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CSecModUIViewBase::CSecModUIViewBase
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CSecModUIViewBase::CSecModUIViewBase(CSecModUIModel& aModel):
    CAknView(), iModel(aModel)
    {    
    LOG_WRITE( "CSecModUIViewBase::CSecModUIViewBase" );
    }

// ---------------------------------------------------------
// CSecModUIViewBase::~CSecModUIViewBase()
// destructor
// ---------------------------------------------------------
//
CSecModUIViewBase::~CSecModUIViewBase()
    {
    LOG_WRITE( "CSecModUIViewBase::~CSecModUIViewBase" );
    delete iNaviDecorator;
    if (iContainer)
        {
        AppUi()->RemoveFromViewStack( *this, iContainer );
        }
    delete iContainer;
    }
    
// ---------------------------------------------------------
// CSecModUIViewBase::HandleCommandL(TInt aCommand)
// takes care of view command handling
// ---------------------------------------------------------
//
void CSecModUIViewBase::HandleCommandL(TInt aCommand)
    {
    LOG_ENTERFN("CSecModUIViewBase::HandleCommandL()");   
    switch ( aCommand )
        {
        case EAknSoftkeyBack:
            {
            AppUi()->ActivateLocalViewL(iPrevViewId.iViewUid);
            break;
            }
        case ESecModUICmdModuleInfo:
            {
            iModel.ViewOpenedSecModDetailsL();
            break;
            }
        case EAknCmdExit:
            {
            ((CAknViewAppUi*)iAvkonAppUi)->HandleCommandL(EAknCmdExit);
            break;
            }
        case EAknCmdHelp: 
            {
            if( FeatureManager::FeatureSupported( KFeatureIdHelp ) )
                {
                HlpLauncher::LaunchHelpApplicationL(
                    iEikonEnv->WsSession(), AppUi()->AppHelpContextL() );   
                }    
            break;
            }    
        default:
            {            
            break;
            }
        }
    LOG_LEAVEFN("CSecModUIViewBase::HandleCommandL()");    
    }

// ---------------------------------------------------------
// CSecModUIViewBase::HandleClientRectChange()
// ---------------------------------------------------------
//
void CSecModUIViewBase::HandleClientRectChange()
    {
    LOG_ENTERFN("CSecModUIViewBase::HandleClientRectChange()");   
    if ( iContainer )
        {
        iContainer->SetRect( ClientRect() );
        }
    LOG_LEAVEFN("CSecModUIViewBase::HandleClientRectChange()");        
    }

// ---------------------------------------------------------
// CSecModUIViewBase::DoActivateL(...)
// 
// ---------------------------------------------------------
//
void CSecModUIViewBase::SetTitlePaneL()
    {
    iModel.ActivateTokenLabelToTitleL();
    }

// ---------------------------------------------------------
// CSecModUIViewBase::AddNavipaneLabelL()
// Default implementation is empty
// ---------------------------------------------------------
//
void CSecModUIViewBase::AddNaviPaneLabelL()
    {
    DoAddNaviPaneL();               
    }

// ---------------------------------------------------------
// CSecModUIViewBase::DoAddNaviPaneL()
// Creates navi pane text
// ---------------------------------------------------------
//
void CSecModUIViewBase::DoAddNaviPaneL(TInt aResource)
    {
    TUid naviPaneUid;
    naviPaneUid.iUid = EEikStatusPaneUidNavi;
    CEikStatusPane* statusPane = StatusPane();
    CAknNavigationControlContainer* naviPane =
        (CAknNavigationControlContainer*) statusPane->ControlL(naviPaneUid);
    if(!iNaviDecorator)
        {
        // Let's try to create navipane
        CEikStatusPaneBase::TPaneCapabilities subPane =
        statusPane->PaneCapabilities(naviPaneUid);
    
        if (subPane.IsPresent() && subPane.IsAppOwned())
            {
            if (0 != aResource)
                {
                TResourceReader reader;
                iCoeEnv->CreateResourceReaderLC(reader, aResource);

                 // set the navigation pane label
                iNaviDecorator = naviPane->CreateNavigationLabelL(reader);
                CleanupStack::PopAndDestroy();
                }
             else
                {
                iNaviDecorator = naviPane->CreateNavigationLabelL();
                }
            }
        }
    if (iNaviDecorator)
        {
        naviPane->PushL(*iNaviDecorator);
        }
    }
        
// ---------------------------------------------------------
// CSecModUIViewBase::DoActivateL(...)
// 
// ---------------------------------------------------------
//
void CSecModUIViewBase::DoActivateL(
   const TVwsViewId& aPrevViewId,TUid /*aCustomMessageId*/,
   const TDesC8& /*aCustomMessage*/)
    {
    LOG_ENTERFN("CSecModUIViewBase::DoActivateL()");   
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
   iContainer->ListBox().SetTopItemIndex( iTopItem );
   //the iCurrentPostion can be -1, if no keystore presents.
   if ( iCurrentPosition >= 0 )
       {
       iContainer->ListBox().SetCurrentItemIndex( iCurrentPosition ); 	
       } 
   iContainer->MakeVisible(ETrue);
   iContainer->SetRect(ClientRect());
   iContainer->ActivateL();
   
   
   LOG_LEAVEFN("CSecModUIViewBase::DoActivateL()");    
   }

// ---------------------------------------------------------
// CSecModUIViewBase::DoDeactivate()
// 
// ---------------------------------------------------------
//
void CSecModUIViewBase::DoDeactivate()
    {
    LOG_ENTERFN("CSecModUIViewBase::DoDeactivate()");   
    if ( iContainer )
        {
        AppUi()->RemoveFromViewStack( *this, iContainer );
        iCurrentPosition = iContainer->ListBox().CurrentItemIndex();
        iTopItem = iContainer->ListBox().TopItemIndex();
        }
    
    delete iContainer;
    iContainer = NULL;
    LOG_LEAVEFN("CSecModUIViewBase::DoDeactivate()");    
    }

// ---------------------------------------------------------
// CSecModUIContainerBase::HandleListBoxEventL(
//     CEikListBox* aListBox, TListBoxEvent aEventType)
// ---------------------------------------------------------
//    
void CSecModUIViewBase::HandleListBoxEventL(
    CEikListBox* /*aListBox*/, 
    TListBoxEvent aEventType)
    {
    LOG_ENTERFN("CSecModUIContainerBase::HandleListBoxEventL()");   
    switch( aEventType )
		{	
        default:
            {
			break;
            }
		} 
    LOG_LEAVEFN("CSecModUIContainerBase::HandleListBoxEventL()");    		          
    }

// End of File
