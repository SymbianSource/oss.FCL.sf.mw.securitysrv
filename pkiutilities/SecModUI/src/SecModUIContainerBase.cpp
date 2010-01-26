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
* Description:   Implementation of the CSecModUIContainerBase class
*
*/


// INCLUDE FILES
#include "SecModUIContainerBase.h"
#include "SecModUIModel.h"
#include "SecModUILogger.h"
#include <SecModUI.rsg>
#include <aknlists.h>
#include <barsread.h>
#include <StringLoader.h>

// ================= MEMBER FUNCTIONS =======================
CSecModUIContainerBase::CSecModUIContainerBase(
    CSecModUIModel& aModel,
    const TDesC& aContextName)
    :iModel(aModel), iContextName(aContextName)
    {
    LOG_WRITE( "CSecModUIContainerBase::CSecModUIContainerBase" );
	}

// ---------------------------------------------------------
// CSecModUIContainerBase::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CSecModUIContainerBase::ConstructL(const TRect& aRect)
    {
    CreateWindowL();
    CreateListL();
    SetupListItemsL();
    SetRect(aRect);
    }

// Destructor
CSecModUIContainerBase::~CSecModUIContainerBase()
    {
    LOG_ENTERFN("CSecModUIContainerBase::~CSecModUIContainerBase()");              
    delete iListBox;
    LOG_LEAVEFN("CSecModUIContainerBase::~CSecModUIContainerBase()");
	}

// ---------------------------------------------------------
// CSecModUIContainerBase::ConstructListL()
// Constructs listbox
// ---------------------------------------------------------
//
void CSecModUIContainerBase::ConstructListL()	
    {
    LOG_ENTERFN("CSecModUIContainerBase::ConstructListL()");   
    iListBox = new (ELeave) CAknSingleStyleListBox();
    LOG_LEAVEFN("CSecModUIContainerBase::ConstructListL()");    
    }

// ---------------------------------------------------------
// CSecModUIContainerBase::ListBox()
// 
// ---------------------------------------------------------
//
CEikTextListBox& CSecModUIContainerBase::ListBox()
    {
    return *iListBox;
    }

// ---------------------------------------------------------
// CSecModUIContainerBase::CreateResourceReaderLC(
//  TResourceReader& aReader)
// 
// ---------------------------------------------------------
//
void CSecModUIContainerBase::CreateResourceReaderLC(TResourceReader& aReader)	
    {
    LOG_ENTERFN("CSecModUIContainerBase::CreateResourceReaderLC()");   
    iEikonEnv->CreateResourceReaderLC(aReader, R_SECMODUI_DYNAMIC_LISTBOX);
    LOG_LEAVEFN("CSecModUIContainerBase::CreateResourceReaderLC()");    
    }

// ---------------------------------------------------------
// CSecModUIContainerBase::CreateListL()
// Constructs listbox from resource
// ---------------------------------------------------------
//	
void CSecModUIContainerBase::CreateListL()
    {
    LOG_ENTERFN("CSecModUIContainerBase::CreateListL()");   
    ConstructListL();
    __ASSERT_ALWAYS(iListBox, Panic(EPanicNullPointer));
    iListBox->SetContainerWindowL(*this);
    
    iListBox->CreateScrollBarFrameL(ETrue);
	iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(
	    CEikScrollBarFrame::EOff,CEikScrollBarFrame::EAuto);
    
    TResourceReader reader;
    
    CreateResourceReaderLC(reader);
    iListBox->ConstructFromResourceL(reader);
    CleanupStack::PopAndDestroy(); // reader
    
    HBufC* stringHolder = StringLoader::LoadLC(R_TEXT_RESOURCE_VIEW_EMPTY_WIM_LIST);
	iListBox->View()->SetListEmptyTextL(*stringHolder);
	CleanupStack::PopAndDestroy(stringHolder);
	LOG_LEAVEFN("CSecModUIContainerBase::CreateListL()");    
    }
    
// ---------------------------------------------------------
// CSecModUIContainerBase::SetupListItemsL()
// Setups list items to listbox.
// ---------------------------------------------------------
//	
void CSecModUIContainerBase::SetupListItemsL()
    {
    LOG_ENTERFN("CSecModUIContainerBase::SetupListItemsL()");   
    LOG_LEAVEFN("CSecModUIContainerBase::SetupListItemsL()");    
    }

// ---------------------------------------------------------
// CSecModUIContainerBase::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CSecModUIContainerBase::SizeChanged()
    {
    LOG_ENTERFN("CSecModUIContainerBase::SizeChanged()");   
    __ASSERT_ALWAYS(iListBox, Panic(EPanicNullPointer));
    iListBox->SetRect(Rect());
    LOG_LEAVEFN("CSecModUIContainerBase::SizeChanged()");    
    }

// ---------------------------------------------------------
// CSecModUIContainerBase::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CSecModUIContainerBase::CountComponentControls() const
    {
    LOG_ENTERFN("CSecModUIContainerBase::CountComponentControls()");   
    LOG_LEAVEFN("CSecModUIContainerBase::CountComponentControls()");    
    return 1;
    }

// ---------------------------------------------------------
// CSecModUIContainerBase::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* CSecModUIContainerBase::ComponentControl(TInt aIndex) const
    {
    LOG_ENTERFN("CSecModUIContainerBase::ComponentControl()");   
    LOG_LEAVEFN("CSecModUIContainerBase::ComponentControl()");    
    switch ( aIndex )
        {
        case 0:
            return iListBox;           
        default:
            return NULL;
        }
    }
    
// ---------------------------------------------------------
// CSecModUIContainerBase::OfferKeyEventL(
//     const TKeyEvent& aKeyEvent,TEventCode aType)
// ---------------------------------------------------------
//
TKeyResponse CSecModUIContainerBase::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
    {
    if (iModel.Wrapper().IsActive())
        {
        // Do not start new operation, if previous is still ongoing.
        return EKeyWasConsumed;
        }
    LOG_ENTERFN("CSecModUIContainerBase::OfferKeyEventL()");   
    LOG_LEAVEFN("CSecModUIContainerBase::OfferKeyEventL()");    
    return iListBox->OfferKeyEventL(aKeyEvent, aType);
    }
    
// ---------------------------------------------------------
// CCertManUIContainerAuthority::HandleResourceChange
// 
// ---------------------------------------------------------
//
void CSecModUIContainerBase::HandleResourceChange(TInt aType)
    {        
    CCoeControl::HandleResourceChange(aType);        
    if (aType == KEikDynamicLayoutVariantSwitch)
        {		
        TRect mainPaneRect;
        AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, mainPaneRect);
        SetRect(mainPaneRect);
		DrawDeferred();
        }  
    }


// ---------------------------------------------------------
// CSecModUIContainerBase::GetHelpContext
// This function is called when Help application is launched.  
// (other items were commented in a header).
// ---------------------------------------------------------
//
void CSecModUIContainerBase::GetHelpContext( 
    TCoeHelpContext& aContext ) const
    {
	aContext.iMajor = KUidSecMod;
    aContext.iContext = iContextName;
    }


// ---------------------------------------------------------
// CSecModUIContainerBase::FocusChanged(TDrawNow aDrawNow)
// ---------------------------------------------------------
//
void CSecModUIContainerBase::FocusChanged( TDrawNow aDrawNow )
    {
    if ( iListBox )
        {
        iListBox->SetFocus( IsFocused(), aDrawNow );
        }
    }
// End of File  
