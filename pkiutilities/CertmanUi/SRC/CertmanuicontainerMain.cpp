/*
* Copyright (c) 2003-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   Implementation of class CCertManUIContainerMain
*
*/


// INCLUDE FILES
#include <eikclbd.h>
#include <AknIconArray.h>
#include <certmanui.rsg>
#include <csxhelp/cm.hlp.hrh>   // for help context of CertManUI

#include "CertmanuiCertificateHelper.h"
#include "CertmanuiKeeper.h"
#include "CertmanuicontainerMain.h"
#include "CertmanuiSyncWrapper.h"
#include "Certmanui.hrh"
#include "CertManUILogger.h"
#include "CertmanuiCommon.h"

#include "Certmanuidll.h"       // for applicationUID


// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CCertManUIContainerMain::CCertManUIContainerMain(
// CCertManUIViewPersonal& aParent, CCertManUIKeeper& aKeeper)
// Constructor with parent
// ---------------------------------------------------------
//
CCertManUIContainerMain::CCertManUIContainerMain(
    CCertManUIViewMain& aParent, CCertManUIKeeper& aKeeper )
    : iParent( aParent ), iKeeper( aKeeper )
    {
    CERTMANUILOGGER_WRITE_TIMESTAMP(
        "CCertManUIContainerMain::CCertManUIContainerMain" );
    }

// ---------------------------------------------------------
// CCertManUIContainerMain::~CCertManUIContainerMain()
// Destructor
// ---------------------------------------------------------
//
CCertManUIContainerMain::~CCertManUIContainerMain()
    {
    CERTMANUILOGGER_ENTERFN(
    		" CCertManUIContainerMain::~CCertManUIContainerMain" );

    delete iListBox;

    CERTMANUILOGGER_LEAVEFN(
    		" CCertManUIContainerMain::~CCertManUIContainerMain" );
    }

// ---------------------------------------------------------
// CCertManUIContainerMain::ConstructL(
// const TRect& aRect, TInt aCurrentPosition, TInt aTopItem)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CCertManUIContainerMain::ConstructL(
    const TRect& aRect, TInt /*aCurrentPosition*/, TInt /*aTopItem*/ )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerMain::ConstructL" );

    CreateWindowL();
    CreateListBoxL();

    if ( iListBox )
        {
        // Creates graphic.
        iListBox->ItemDrawer()->ColumnData()->
            SetIconArray( iIconHandler->CreateIconArrayL() );
        }

    SetRect( aRect );

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerMain::ConstructL" );
    }

// ---------------------------------------------------------
// CCertManUIContainerMain::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CCertManUIContainerMain::SizeChanged()
    {
    iListBox->SetRect( Rect() );
    }

// ---------------------------------------------------------
// CCertManUIContainerMain::FocusChanged(TDrawNow aDrawNow)
// ---------------------------------------------------------
//
void CCertManUIContainerMain::FocusChanged( TDrawNow aDrawNow )
    {
    if ( iListBox )
        {
        iListBox->SetFocus( IsFocused(), aDrawNow );
        }
    }

// ---------------------------------------------------------
// CCertManUIContainerMain::HandleListBoxEventL(
// CEikListBox* /*aListBox*/, TListBoxEvent aEventType)
// ---------------------------------------------------------
//
void CCertManUIContainerMain::HandleListBoxEventL(
    CEikListBox* /*aListBox*/, TListBoxEvent aEventType )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerMain::HandleListBoxEventL" );

    switch( aEventType )
        {
        case EEventItemDoubleClicked:
        case EEventItemSingleClicked:
        case EEventEnterKeyPressed:
            {
            TInt index = iListBox->CurrentItemIndex();

            if ( index == 0 )
                {
                ((CAknViewAppUi*)iAvkonAppUi)->ActivateLocalViewL( KCertManUIViewAuthorityId );
                }
            else if ( index == 1 )
                {
                ((CAknViewAppUi*)iAvkonAppUi)->ActivateLocalViewL( KCertManUIViewTrustedSiteId );
                }
            else if ( index == 2 )
                {
                ((CAknViewAppUi*)iAvkonAppUi)->ActivateLocalViewL( KCertManUIViewPersonalId );
                }
            else if ( index == 3 )
                {
                ((CAknViewAppUi*)iAvkonAppUi)->ActivateLocalViewL( KCertManUIViewDeviceId );
                }

            break;
            }
        default:
            {
            break;
            }
    		}

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerMain::HandleListBoxEventL" );
    }

// ---------------------------------------------------------
// CCertManUIContainerMain::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CCertManUIContainerMain::CountComponentControls() const
    {
    // return number of controls inside this container
    return 1;
    }

// ---------------------------------------------------------
// CCertManUIContainerMain::ComponentControl(TInt /*aIndex*/) const
// ---------------------------------------------------------
//
CCoeControl* CCertManUIContainerMain::ComponentControl( TInt /*aIndex*/ ) const
    {
    return iListBox;
    }

// ---------------------------------------------------------
// CCertManUIContainerMain::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CCertManUIContainerMain::Draw( const TRect& aRect ) const
    {
    CWindowGc& gc = SystemGc();
    gc.SetPenStyle( CGraphicsContext::ENullPen );
    gc.SetBrushColor( KRgbGray );
    gc.SetBrushStyle( CGraphicsContext::ESolidBrush );
    gc.DrawRect( aRect );
    }

// ---------------------------------------------------------
// CCertManUIContainerMain::HandleControlEventL(
//    CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/)
// Cannot be changed to non-leaving function.
// L-function is required by the class definition, even if empty.
// ---------------------------------------------------------
//
void CCertManUIContainerMain::HandleControlEventL(
    CCoeControl* /*aControl*/, TCoeEvent /*aEventType*/ )
    {
    }

// ---------------------------------------------------------
// CCertManUIContainerMain::OfferKeyEventL(
// const TKeyEvent& aKeyEvent, TEventCode aType)
// Handles the key events OK button, arrow keys, delete key.
// OK button functionality is different depending on the number of marked items
// in the list; zero marked -> show details view, one or more marked -> show
// OKOptions list, list empty -> OK inactive.
// Delete active only if list not empty.
// ---------------------------------------------------------
//
TKeyResponse CCertManUIContainerMain::OfferKeyEventL(
    const TKeyEvent& aKeyEvent, TEventCode aType )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerMain::OfferKeyEventL" );
    TKeyResponse retval = EKeyWasNotConsumed;

    retval = iListBox->OfferKeyEventL( aKeyEvent, aType );

		CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerMain::OfferKeyEventL" );

    return retval;
    }

// ---------------------------------------------------------
// CCertManUIContainerMain::CreateListBoxL()
// Creates listbox, sets empty listbox text.
// ---------------------------------------------------------
//
void CCertManUIContainerMain::CreateListBoxL()
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerMain::CreateListBoxL" );

    iKeeper.ActivateTitleL( KViewTitleCertManUI );

    iListBox = new( ELeave ) CAknSettingStyleListBox;
    iListBox->SetContainerWindowL( *this );
    iListBox->ConstructL( this, EAknListBoxMarkableList );
    iListBox->CreateScrollBarFrameL( ETrue );
    iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(
          CEikScrollBarFrame::EOff,CEikScrollBarFrame::EAuto );
    iListBox->SetListBoxObserver( this );


    MDesCArray* itemList = iListBox->Model()->ItemTextArray();
    CDesCArray* itemArray = ( CDesCArray* )itemList;

    HBufC*  stringHolder = StringLoader::LoadLC(
                       R_TEXT_RESOURCE_MAIN_VIEW_AUTHORITY );
    itemArray->AppendL( *stringHolder );
    CleanupStack::PopAndDestroy();  // stringHolder

    stringHolder = StringLoader::LoadLC(
                       R_TEXT_RESOURCE_MAIN_VIEW_TRUSTED );
    itemArray->AppendL( *stringHolder );
    CleanupStack::PopAndDestroy();  // stringHolder

    stringHolder = StringLoader::LoadLC(
                       R_TEXT_RESOURCE_MAIN_VIEW_PERSONAL );
    itemArray->AppendL( *stringHolder );
    CleanupStack::PopAndDestroy();  // stringHolder

    stringHolder = StringLoader::LoadLC(
                       R_TEXT_RESOURCE_MAIN_VIEW_DEVICE );
    itemArray->AppendL( *stringHolder );
    CleanupStack::PopAndDestroy();  // stringHolder

    iListBox->HandleItemAdditionL();

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerMain::CreateListBoxL" );
    }

// ---------------------------------------------------------
// CCertManUIContainerMain::DrawListBoxL(TInt aCurrentPosition, TInt aTopItem)
// Draws listbox, fetches graphic icons for markable list
// ---------------------------------------------------------
//
void CCertManUIContainerMain::DrawListBoxL( TInt aCurrentPosition, TInt aTopItem )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerMain::DrawListBoxL" );

    if ( aCurrentPosition >= 0 )
        {
        iListBox->SetTopItemIndex( aTopItem );
        iListBox->SetCurrentItemIndex( aCurrentPosition );
        }
    ActivateL();
    DrawNow();

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerMain::DrawListBoxL" );
    }

// ---------------------------------------------------------
// CCertManUIContainerAuthority::HandleResourceChange
//
// ---------------------------------------------------------
//
void CCertManUIContainerMain::HandleResourceChange(TInt aType)
    {
    CCoeControl::HandleResourceChange(aType);

    if ( aType == KEikDynamicLayoutVariantSwitch )
        {
        TRect mainPaneRect;
        AknLayoutUtils::LayoutMetricsRect( AknLayoutUtils::EMainPane, mainPaneRect );
        SetRect( mainPaneRect );
        DrawNow();
        }
    }

// ---------------------------------------------------------
// CCertManUIContainerMain::IconArrayResourceId
// Loads Mark/Unmark icons from resource
// ---------------------------------------------------------
//
TInt CCertManUIContainerMain::IconArrayResourceId() const
    {
    return R_CERTMANUI_CERTIFICATE_LISTBOX_MARK_ICONS;
    }

// ---------------------------------------------------------
// CCertManUIContainerMain::CreateIconArrayL
// Creates Icon Array
// ---------------------------------------------------------
//
CAknIconArray* CCertManUIContainerMain::CreateIconArrayL() const
    {
    CAknIconArray* icons = new ( ELeave ) CAknIconArray( KGranularity );
    CleanupStack::PushL( icons );
    icons->ConstructFromResourceL( IconArrayResourceId() );
    CleanupStack::Pop(); // icons
    return icons;
    }


// ---------------------------------------------------------
// CCertManUIContainerMain::GetHelpContext
// This function is called when Help application is launched.
// (other items were commented in a header).
// ---------------------------------------------------------
//

void CCertManUIContainerMain::GetHelpContext( TCoeHelpContext& aContext ) const
    {
    aContext.iMajor = KUidCM;
    aContext.iContext = KCM_HLP_MAIN;
    }

// End of File
