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
* Description:   Implementation of class CCertManUIContainerTrustedSite
*                Reads & updates certificate list.
*                Handles markable list for certificates.
*
*/


// INCLUDE FILES
#include <eikclbd.h>
#include <AknIconArray.h>
#include <csxhelp/cm.hlp.hrh>   // for help context of CertManUI
#include <certmanui.rsg>

#include "CertmanuiCertificateHelper.h"
#include "CertmanuiKeeper.h"
#include "CertmanuicontainerTrustedSite.h"
#include "CertmanuiSyncWrapper.h"
#include "Certmanui.hrh"
#include "CertManUILogger.h"
#include "CertmanuiCommon.h"

#include "Certmanuidll.h"       // for applicationUID

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CCertManUIContainerTrustedSite::CCertManUIContainerTrustedSite(
// CCertManUIViewAuthority& aParent, CCertManUIKeeper& aKeeper)
// Constructor with parent
// ---------------------------------------------------------
//
CCertManUIContainerTrustedSite::CCertManUIContainerTrustedSite(
    CCertManUIViewTrustedSite& aParent, CCertManUIKeeper& aKeeper ) :
    iParent ( aParent ), iKeeper ( aKeeper )
    {
    CERTMANUILOGGER_WRITE_TIMESTAMP(
        "CCertManUIContainerTrustedSite::CCertManUIContainerTrustedSite" );
    }

// ---------------------------------------------------------
// CCertManUIContainerTrustedSite::~CCertManUIContainerTrustedSite()
// Destructor
// ---------------------------------------------------------
//
CCertManUIContainerTrustedSite::~CCertManUIContainerTrustedSite()
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerTrustedSite::~CCertManUIContainerTrustedSite" );

    delete iListBox;

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerTrustedSite::~CCertManUIContainerTrustedSite" );
    }

// ---------------------------------------------------------
// CCertManUIContainerTrustedSite::ConstructL(
// const TRect& aRect, TInt& aCurrentPosition, TInt& aTopItem)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CCertManUIContainerTrustedSite::ConstructL(
    const TRect& aRect, TInt& aCurrentPosition, TInt& aTopItem )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerTrustedSite::ConstructL" );

    CreateWindowL();
    CreateListBoxL();
    UpdateListBoxL( aCurrentPosition, aTopItem );
    if ( iListBox )
        {
        // Creates graphic.
        iListBox->ItemDrawer()->ColumnData()->
             SetIconArray( iIconHandler->CreateIconArrayL() );
    }
    SetRect( aRect );

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerTrustedSite::ConstructL" );
    }

// ---------------------------------------------------------
// CCertManUIContainerTrustedSite::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CCertManUIContainerTrustedSite::SizeChanged()
    {
    iListBox->SetRect( Rect() );
    }

// ---------------------------------------------------------
// CCertManUIContainerTrustedSite::FocusChanged(TDrawNow aDrawNow)
// ---------------------------------------------------------
//
void CCertManUIContainerTrustedSite::FocusChanged( TDrawNow aDrawNow )
    {
    if ( iListBox )
        {
        iListBox->SetFocus( IsFocused(), aDrawNow );
        }
    }

// ---------------------------------------------------------
// CCertManUIContainerTrustedSite::HandleListBoxEventL(
// CEikListBox* /*aListBox*/, TListBoxEvent aEventType)
// ---------------------------------------------------------
//
void CCertManUIContainerTrustedSite::HandleListBoxEventL(
    CEikListBox* /*aListBox*/, TListBoxEvent aEventType )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerTrustedSite::HandleListBoxEventL" );

    switch( aEventType )
        {
        case EEventItemSingleClicked:
        case EEventEnterKeyPressed:
            {
            const CListBoxView::CSelectionIndexArray* selections = iListBox->SelectionIndexes();
            if ( iListItemCount > 0 )
                {
                if ( selections->Count() == 0 )
                    {
                    iKeeper.iCertificateHelper->MessageQueryViewDetailsL(
                            iListBox->CurrentItemIndex(), KCertTypeTrustedSite, iEikonEnv );
                    }
                else
                    {
                    CEikMenuBar* menuBar = iParent.MenuBar();
                    menuBar->SetContextMenuTitleResourceId( R_CERTMANUI_MENUBAR_OPTIONS_MARKED );
                    menuBar->TryDisplayContextMenuBarL();
                    }
                }
            break;
            }

        default:
            {
            break;
            }
        }

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerTrustedSite::HandleListBoxEventL" );
    }

// ---------------------------------------------------------
// CCertManUIContainerTrustedSite::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CCertManUIContainerTrustedSite::CountComponentControls() const
    {
    // return number of controls inside this container
    return 1;
    }

// ---------------------------------------------------------
// CCertManUIContainerTrustedSite::ComponentControl(
// TInt /*aIndex*/) const
// ---------------------------------------------------------
//
CCoeControl* CCertManUIContainerTrustedSite::ComponentControl(
    TInt /*aIndex*/) const
    {
    return iListBox;
    }

// ---------------------------------------------------------
// CCertManUIContainerTrustedSite::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CCertManUIContainerTrustedSite::Draw( const TRect& aRect ) const
    {
    CWindowGc& gc = SystemGc();
    gc.SetPenStyle( CGraphicsContext::ENullPen );
    gc.SetBrushColor( KRgbGray );
    gc.SetBrushStyle( CGraphicsContext::ESolidBrush );
    gc.DrawRect( aRect );
    }

// ---------------------------------------------------------
// CCertManUIContainerTrustedSite::HandleControlEventL(
//    CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/)
// Cannot be changed to non-leaving function.
// L-function is required by the class definition, even if empty.
// ---------------------------------------------------------
//
void CCertManUIContainerTrustedSite::HandleControlEventL(
    CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/)
    {
    }

// ---------------------------------------------------------
// CCertManUIContainerTrustedSite::OfferKeyEventL(
// const TKeyEvent& aKeyEvent, TEventCode aType)
// Handles the key events arrow keys, delete key.
// ---------------------------------------------------------
//
TKeyResponse CCertManUIContainerTrustedSite::OfferKeyEventL(
    const TKeyEvent& aKeyEvent, TEventCode aType )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerTrustedSite::OfferKeyEventL" );

    // If operation is currenly onway, do not continue.
    if ( iKeeper.iWrapper->IsActive() )
        {
        return EKeyWasNotConsumed;
        }

    TKeyResponse retval = EKeyWasNotConsumed;
    if ( aType == EEventKey )
        {
        if ( aKeyEvent.iCode == EKeyRightArrow )
            {
            ((CAknViewAppUi*)iAvkonAppUi)->ActivateLocalViewL( KCertManUIViewPersonalId );
            retval = EKeyWasConsumed;
            }
        else if ( aKeyEvent.iCode == EKeyLeftArrow )
            {
            ((CAknViewAppUi*)iAvkonAppUi)->ActivateLocalViewL( KCertManUIViewAuthorityId );
            retval = EKeyWasConsumed;
            }
        else if ( aKeyEvent.iCode == EKeyDelete
                  || aKeyEvent.iCode == EKeyBackspace )
            {
            TInt currentItemAuthority = iListBox->CurrentItemIndex();

            if ( ( currentItemAuthority >= 0) &&
                ( currentItemAuthority < iKeeper.iPeerLabelEntries.Count()))
                {
                // list is not empty
                if ( iKeeper.iCertificateHelper->
                              ConfirmationQueryDeleteCertL(
                                                    KCertTypeTrustedSite,
                                               iListBox ) ) // == ETrue
                    {

                    iParent.iCurrentPosition = iListBox->CurrentItemIndex();
                    iParent.iTopItem = iListBox->TopItemIndex();

                    DrawListBoxL(iParent.iCurrentPosition, iParent.iTopItem );
                    // goes here if at least one certificate is deleted
                    TRAPD( error, iKeeper.RefreshPeerCertEntriesL() );
                    if ( error != KErrNone )
                        {
                        if ( error == KErrCorrupt )
                            {
                            iKeeper.ShowErrorNoteL( error );
                            User::Exit( KErrNone );
                            }
                        else
                            {
                            // have to call straight away the Exit
                            // showing any error notes would corrupt the display
                            User::Exit( error );
                            }
                        }
                    iParent.UpdateMenuBar();
                    }
                  }
                // Listbox takes all event even if it doesn't use them
                retval = EKeyWasConsumed;
          }
        else
          {
          retval = iListBox->OfferKeyEventL( aKeyEvent, aType );
          }
        }
    else
        {
        retval = iListBox->OfferKeyEventL( aKeyEvent, aType );
        }

    if ( iListBox->SelectionIndexes()->Count() > 0 )
        {
        iParent.MenuBar()->SetContextMenuTitleResourceId( R_CERTMANUI_MENUBAR_OPTIONS_MARKED );
        iParent.UpdateCba( R_CERTMANUI_OPTIONS_CONTEXT_BACK );
        }
    else
        {
        iParent.MenuBar()->SetMenuTitleResourceId( R_CERTMANUI_MENUBAR_VIEW_TRUSTED_SITES );
        iParent.UpdateCba( R_CERTMANUI_OPTIONS_OPEN_BACK );
        }

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerTrustedSite::OfferKeyEventL" );

    return retval;
    }

// ---------------------------------------------------------
// CCertManUIContainerTrustedSite::CreateListBoxL()
// Creates listbox, sets empty listbox text.
// ---------------------------------------------------------
//
void CCertManUIContainerTrustedSite::CreateListBoxL()
  {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerTrustedSite::CreateListBoxL()" );

    iListBox = new( ELeave ) CAknSingleStyleListBox;
    iListBox->SetContainerWindowL( *this );
    iListBox->ConstructL( this, EAknListBoxMarkableList );
    iListBox->CreateScrollBarFrameL( ETrue );
    iListBox->ScrollBarFrame()->
        SetScrollBarVisibilityL( CEikScrollBarFrame::EOff,CEikScrollBarFrame::EAuto );
    iListBox->SetListBoxObserver( this );

    HBufC* stringHolder = StringLoader::LoadLC(
        R_TEXT_RESOURCE_VIEW_TRUSTED_EMPTY_LIST );
    iListBox->View()->SetListEmptyTextL( *stringHolder );
    CleanupStack::PopAndDestroy();  // stringHolder

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerTrustedSite::CreateListBoxL()" );
    }

// ---------------------------------------------------------
// CCertManUIContainerTrustedSite::DrawListBoxL(TInt aCurrentPosition, TInt aTopItem)
// Draws listbox, fetches graphic icons for markable list
// ---------------------------------------------------------
//
void CCertManUIContainerTrustedSite::DrawListBoxL(
    TInt aCurrentPosition, TInt aTopItem )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerTrustedSite::DrawListBoxL" );

    if ( aCurrentPosition >= 0 )
        {
        iListBox->SetTopItemIndex( aTopItem );
        iListBox->SetCurrentItemIndex( aCurrentPosition );
        }
    ActivateL();
    DrawNow();

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerTrustedSite::DrawListBoxL" );
    }

// ---------------------------------------------------------
// CCertManUIContainerTrustedSite::UpdateListBoxL(TInt& aCurrentPosition, TInt& aTopItem)
// Gets list of certificates from CertManAPI, displays them,
// sets No label/No subject text to them if their names are empty
// ---------------------------------------------------------
//
void CCertManUIContainerTrustedSite::UpdateListBoxL(
    TInt& aCurrentPosition, TInt& aTopItem )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerTrustedSite::UpdateListBoxL()" );

    iKeeper.StartWaitDialogL( ECertmanUiWaitDialog );
    CleanupCloseWaitDialogPushL( iKeeper );

    MDesCArray* itemList = iListBox->Model()->ItemTextArray();
    CDesCArray* itemArray = ( CDesCArray* )itemList;

    if ( iKeeper.iCertsDatOK )  // == ETrue
        {
        // database is not corrupted, get count
        iListItemCount = iKeeper.iPeerLabelEntries.Count();
        }
    else
        {
        iListItemCount = 0;
        }

    if (iListItemCount == 0 ) //list is empty, disable the scroll bar 
        {
    	iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(
	        CEikScrollBarFrame::EOff,CEikScrollBarFrame::EOff);
        }
    if ( iListItemCount > 0 )
        {
        if ( ( aCurrentPosition == -1 )
            || ( aCurrentPosition > iListItemCount-1 ) )
            {
            aCurrentPosition = 0;
            aTopItem = 0;
            }

        HBufC* buf = HBufC::NewLC( KMaxLengthTextCertLabel );
        for ( TInt i = 0; i < iListItemCount; i++ )
            {
            buf->Des() = ( iKeeper.iPeerLabelEntries )[i]->iPeerEntryLabel->Des();

            TPtrC trimmedCertLabel = iKeeper.iCertificateHelper->CutCertificateField( buf->Des() );
            buf->Des().Copy( trimmedCertLabel );
            TInt length = buf->Des().Length();
            if ( length == 0 )
                {
                HBufC* stringHolder = NULL;

                stringHolder = StringLoader::LoadLC(
                           R_TEXT_RESOURCE_VIEW_NO_SUBJECT_LIST );

                itemArray->AppendL( *stringHolder );
                CleanupStack::PopAndDestroy();  // stringHolder
                }
            else
                {
                buf->Des().Insert( 0, KCertManUIInsertDescriptor );
                itemArray->AppendL( buf->Des() );
                }
          iListBox->HandleItemAdditionL();
          }
      CleanupStack::PopAndDestroy();  // buf
    }

    CleanupStack::PopAndDestroy();    // closes wait dialog

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerTrustedSite::UpdateListBoxL()" );
  }

// ---------------------------------------------------------
// CCertManUIContainerTrustedSite::HandleResourceChange
//
// ---------------------------------------------------------
//
void CCertManUIContainerTrustedSite::HandleResourceChange( TInt aType )
    {
    CCoeControl::HandleResourceChange( aType );

    if ( aType == KEikDynamicLayoutVariantSwitch )
        {
        TRect mainPaneRect;
        AknLayoutUtils::LayoutMetricsRect( AknLayoutUtils::EMainPane, mainPaneRect );
        SetRect( mainPaneRect );
        DrawNow();
        }
    }

// ---------------------------------------------------------
// CCertManUIContainerTrustedSite::IconArrayResourceId
// Loads Mark/Unmark icons from resource
// ---------------------------------------------------------
//
TInt CCertManUIContainerTrustedSite::IconArrayResourceId() const
    {
    return R_CERTMANUI_CERTIFICATE_LISTBOX_MARK_ICONS;
    }

// ---------------------------------------------------------
// CCertManUIContainerTrustedSite::CreateIconArrayL
// Creates Icon Array
// ---------------------------------------------------------
//
CAknIconArray* CCertManUIContainerTrustedSite::CreateIconArrayL() const
    {
    CAknIconArray* icons = new (ELeave) CAknIconArray( KGranularity );
    CleanupStack::PushL( icons );
    icons->ConstructFromResourceL( IconArrayResourceId() );
    CleanupStack::Pop(); // icons
    return icons;
    }

// ---------------------------------------------------------
// CCertManUIContainerTrustedSite::GetHelpContext
// This function is called when Help application is launched.
// (other items were commented in a header).
// ---------------------------------------------------------
//
void CCertManUIContainerTrustedSite::GetHelpContext(
    TCoeHelpContext& aContext ) const
    {
    aContext.iMajor = KUidCM;
    aContext.iContext = KCM_HLP_TRUSTED_SITE;
    }

// End of File
