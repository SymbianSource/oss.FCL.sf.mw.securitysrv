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
* Description:   Implementation of class CCertManUIContainerAuthority
*                Reads & updates certificate list
*                Handles markable list for certificates
*
*/


// INCLUDE FILES
#include <eikclbd.h>
#include <AknIconArray.h>
#include <certmanui.rsg>
#include <csxhelp/cm.hlp.hrh>    // for help context of CertManUI

#include "CertmanuiCertificateHelper.h"
#include "CertmanuiKeeper.h"
#include "CertmanuicontainerAuthority.h"
#include "CertmanuiSyncWrapper.h"
#include "Certmanui.hrh"
#include "CertManUILogger.h"
#include "CertmanuiCommon.h"
#include "Certmanuidll.h"        // for applicationUID

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CCertManUIContainerAuthority::CCertManUIContainerAuthority(
// CCertManUIViewAuthority& aParent, CCertManUIKeeper& aKeeper )
// Constructor with parent
// ---------------------------------------------------------
//
CCertManUIContainerAuthority::CCertManUIContainerAuthority(
    CCertManUIViewAuthority& aParent, CCertManUIKeeper& aKeeper ) :
    iParent ( aParent ), iKeeper ( aKeeper )
    {
    CERTMANUILOGGER_WRITE_TIMESTAMP(
        "CCertManUIContainerAuthority::CCertManUIContainerAuthority" );
    }

// ---------------------------------------------------------
// CCertManUIContainerAuthority::~CCertManUIContainerAuthority()
// Destructor
// ---------------------------------------------------------
//
CCertManUIContainerAuthority::~CCertManUIContainerAuthority()
    {
    CERTMANUILOGGER_ENTERFN(
        " CCertManUIContainerAuthority::~CCertManUIContainerAuthority" );

    delete iListBox;

    CERTMANUILOGGER_LEAVEFN(
        " CCertManUIContainerAuthority::~CCertManUIContainerAuthority" );
    }

// ---------------------------------------------------------
// CCertManUIContainerAuthority::ConstructL(
// const TRect& aRect, TInt& aCurrentPosition, TInt& aTopItem)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CCertManUIContainerAuthority::ConstructL(
    const TRect& aRect, TInt& aCurrentPosition, TInt& aTopItem )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerAuthority::ConstructL" );

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

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerAuthority::ConstructL" );
    }

// ---------------------------------------------------------
// CCertManUIContainerAuthority::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CCertManUIContainerAuthority::SizeChanged()
    {
    iListBox->SetRect( Rect() );
    }

// ---------------------------------------------------------
// CCertManUIContainerAuthority::FocusChanged( TDrawNow aDrawNow )
// ---------------------------------------------------------
//
void CCertManUIContainerAuthority::FocusChanged( TDrawNow aDrawNow )
    {
    if ( iListBox )
        {
        iListBox->SetFocus( IsFocused(), aDrawNow );
        }
    }

// ---------------------------------------------------------
// CCertManUIContainerAuthority::HandleListBoxEventL(
// CEikListBox* /*aListBox*/, TListBoxEvent aEventType)
// ---------------------------------------------------------
//
void CCertManUIContainerAuthority::HandleListBoxEventL(
    CEikListBox* /*aListBox*/, TListBoxEvent aEventType )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerAuthority::HandleListBoxEventL" );

    switch  ( aEventType )
        {
        case EEventItemDoubleClicked:
        case EEventItemSingleClicked:
        case EEventEnterKeyPressed:
            {
            const CListBoxView::CSelectionIndexArray* selections = iListBox->SelectionIndexes();
            if ( iListItemCount > 0 )
                {
                TInt currentItemIndex = iListBox->CurrentItemIndex();
                if ( selections->Count() == 0 )
                    {
                    iKeeper.iCertificateHelper->MessageQueryViewDetailsL(
                          currentItemIndex, KCertTypeAuthority, iEikonEnv );
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

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerAuthority::HandleListBoxEventL" );
    }

// ---------------------------------------------------------
// CCertManUIContainerAuthority::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CCertManUIContainerAuthority::CountComponentControls() const
    {
    // return number of controls inside this container
    return 1;
    }

// ---------------------------------------------------------
// CCertManUIContainerAuthority::ComponentControl(
// TInt /*aIndex*/) const
// ---------------------------------------------------------
//
CCoeControl* CCertManUIContainerAuthority::ComponentControl(
    TInt /*aIndex*/) const
    {
    return iListBox;
    }

// ---------------------------------------------------------
// CCertManUIContainerAuthority::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CCertManUIContainerAuthority::Draw( const TRect& aRect ) const
    {
    CWindowGc& gc = SystemGc();
    gc.SetPenStyle( CGraphicsContext::ENullPen );
    gc.SetBrushColor( KRgbGray );
    gc.SetBrushStyle( CGraphicsContext::ESolidBrush );
    gc.DrawRect( aRect );
    }

// ---------------------------------------------------------
// CCertManUIContainerAuthority::HandleControlEventL(
//    CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/)
// Cannot be changed to non-leaving function.
// L-function is required by the class definition, even if empty.
// ---------------------------------------------------------
//
void CCertManUIContainerAuthority::HandleControlEventL(
    CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/)
    {
    }

// ---------------------------------------------------------
// CCertManUIContainerAuthority::OfferKeyEventL(
// const TKeyEvent& aKeyEvent, TEventCode aType)
// Handles the key events arrow keys, delete key.
// ---------------------------------------------------------
//
TKeyResponse CCertManUIContainerAuthority::OfferKeyEventL(
    const TKeyEvent& aKeyEvent, TEventCode aType )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerAuthority::OfferKeyEventL" );

    // If operation is currenly onway, do not continue.
    if ( iKeeper.iWrapper->IsActive() )
        {
        return EKeyWasNotConsumed;
        }

    TKeyResponse retval = EKeyWasNotConsumed;
    if ( aType==EEventKey )
        {
        if ( aKeyEvent.iCode == EKeyRightArrow )
            {
            ((CAknViewAppUi*)iAvkonAppUi)->ActivateLocalViewL(
                                      KCertManUIViewTrustedSiteId );
            retval = EKeyWasConsumed;
            }
      else if ( aKeyEvent.iCode == EKeyLeftArrow )
            {
            ((CAknViewAppUi*)iAvkonAppUi)->ActivateLocalViewL(
                                            KCertManUIViewDeviceId );
            retval = EKeyWasConsumed;
            }
      else if ( aKeyEvent.iCode == EKeyDelete
                  || aKeyEvent.iCode == EKeyBackspace )
            {
            TInt currentItemAuthority = iListBox->CurrentItemIndex();

            if ( ( currentItemAuthority >= 0 ) &&
                ( currentItemAuthority < iKeeper.iCALabelEntries.Count()))
                {
                // list is not empty
                if ( iKeeper.iCertificateHelper->ConfirmationQueryDeleteCertL(
                                               KCertTypeAuthority, iListBox ) ) // == ETrue
                    {

                    iParent.iCurrentPosition = iListBox->CurrentItemIndex();
                    iParent.iTopItem = iListBox->TopItemIndex();

                    DrawListBoxL(iParent.iCurrentPosition, iParent.iTopItem );
                    // goes here if at least one certificate is deleted
                    TRAPD( error, iKeeper.RefreshCAEntriesL() );
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
        iParent.MenuBar()->SetContextMenuTitleResourceId(
                                          R_CERTMANUI_MENUBAR_OPTIONS_MARKED );
        iParent.UpdateCba( R_CERTMANUI_OPTIONS_CONTEXT_BACK );
        }
    else
        {
        iParent.MenuBar()->SetMenuTitleResourceId( R_CERTMANUI_MENUBAR_VIEW_AUTHORITY );
        iParent.UpdateCba( R_CERTMANUI_OPTIONS_OPEN_BACK );
        }

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerAuthority::OfferKeyEventL" );

    return retval;
    }

// ---------------------------------------------------------
// CCertManUIContainerAuthority::CreateListBoxL()
// Creates listbox, sets empty listbox text.
// ---------------------------------------------------------
//
void CCertManUIContainerAuthority::CreateListBoxL()
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerAuthority::CreateListBoxL()" );

    iListBox = new( ELeave ) CAknSingleStyleListBox;
    iListBox->SetContainerWindowL( *this );
    iListBox->ConstructL( this, EAknListBoxMarkableList );
    iListBox->CreateScrollBarFrameL( ETrue );
    iListBox->ScrollBarFrame()->
        SetScrollBarVisibilityL( CEikScrollBarFrame::EOff,CEikScrollBarFrame::EAuto );
    iListBox->SetListBoxObserver( this );

    HBufC* stringHolder = StringLoader::LoadLC(
        R_TEXT_RESOURCE_VIEW_AUTHORITY_EMPTY_LIST );
    iListBox->View()->SetListEmptyTextL( *stringHolder );
    CleanupStack::PopAndDestroy();  // stringHolder

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerAuthority::CreateListBoxL()" );
    }

// ---------------------------------------------------------
// CCertManUIContainerAuthority::DrawListBoxL(TInt aCurrentPosition, TInt aTopItem)
// Draws listbox, fetches graphic icons for markable list
// ---------------------------------------------------------
//
void CCertManUIContainerAuthority::DrawListBoxL(
    TInt aCurrentPosition, TInt aTopItem )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerAuthority::DrawListBoxL" );

    if ( aCurrentPosition >= 0 )
        {
        iListBox->SetTopItemIndex( aTopItem );
        iListBox->SetCurrentItemIndex( aCurrentPosition );
        }

    ActivateL();
    DrawNow();

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerAuthority::DrawListBoxL" );
    }

// ---------------------------------------------------------
// CCertManUIContainerAuthority::UpdateListBoxL(TInt& aCurrentPosition, TInt& aTopItem)
// Gets list of certificates from CertManAPI, displays them,
// sets No label/No subject text to them if their names are empty
// ---------------------------------------------------------
//
void CCertManUIContainerAuthority::UpdateListBoxL(
    TInt& aCurrentPosition, TInt& aTopItem )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerAuthority::UpdateListBoxL()" );

    iKeeper.StartWaitDialogL( ECertmanUiWaitDialog );
    CleanupCloseWaitDialogPushL( iKeeper );

    MDesCArray* itemList = iListBox->Model()->ItemTextArray();
    CDesCArray* itemArray = ( CDesCArray* )itemList;

    // List CA certificates
    if ( iKeeper.iCertsDatOK )  // == ETrue
        {
        // cacerts.dat is not corrupted, get count
        iListItemCount = iKeeper.iCALabelEntries.Count();
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
              buf->Des() = ( iKeeper.iCALabelEntries )[i]->iCAEntryLabel->Des();

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
                  buf->Des().Insert(0, KCertManUIInsertDescriptor);
                  itemArray->AppendL(buf->Des());
                  }
              iListBox->HandleItemAdditionL();
              }
              CleanupStack::PopAndDestroy();  // buf
          }

    CleanupStack::PopAndDestroy();    // closes wait dialog

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerAuthority::UpdateListBoxL()" );
    }

// ---------------------------------------------------------
// CCertManUIContainerAuthority::HandleResourceChange
//
// ---------------------------------------------------------
//
void CCertManUIContainerAuthority::HandleResourceChange( TInt aType )
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
// CCertManUIContainerAuthority::IconArrayResourceId
// Loads Mark/Unmark icons from resource
// ---------------------------------------------------------
//
TInt CCertManUIContainerAuthority::IconArrayResourceId() const
    {
    return R_CERTMANUI_CERTIFICATE_LISTBOX_MARK_ICONS;
    }

// ---------------------------------------------------------
// CCertManUIContainerAuthority::CreateIconArrayL
// Creates Icon Array
// ---------------------------------------------------------
//
CAknIconArray* CCertManUIContainerAuthority::CreateIconArrayL() const
    {
    CAknIconArray* icons = new (ELeave) CAknIconArray( KGranularity );
    CleanupStack::PushL( icons );
    icons->ConstructFromResourceL( IconArrayResourceId() );
    CleanupStack::Pop(); // icons
    return icons;
    }

// ---------------------------------------------------------
// CCertManUIContainerAuthority::GetHelpContext
// This function is called when Help application is launched.
// (other items were commented in a header).
// ---------------------------------------------------------
//
void CCertManUIContainerAuthority::GetHelpContext(
    TCoeHelpContext& aContext ) const
    {
    aContext.iMajor = KUidCM;
    aContext.iContext = KCM_HLP_AUTHORITY;
    }

// End of File
