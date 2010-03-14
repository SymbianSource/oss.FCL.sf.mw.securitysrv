/*
* Copyright (c) 2003-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   Implementation of class CCertManUIContainerPersonal
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
#include "CertmanuicontainerPersonal.h"
#include "CertmanuiSyncWrapper.h"
#include "Certmanui.hrh"
#include "CertManUILogger.h"
#include "CertmanuiCommon.h"
#include "Certmanuidll.h"       // for applicationUID

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CCertManUIContainerPersonal::CCertManUIContainerPersonal(
// CCertManUIViewPersonal& aParent, CCertManUIKeeper& aKeeper)
// Constructor with parent
// ---------------------------------------------------------
//
CCertManUIContainerPersonal::CCertManUIContainerPersonal(
    CCertManUIViewPersonal& aParent, CCertManUIKeeper& aKeeper )
: iParent( aParent ), iKeeper( aKeeper )
    {
    CERTMANUILOGGER_WRITE_TIMESTAMP(
        "CCertManUIContainerPersonal::CCertManUIContainerPersonal" );
    }

// ---------------------------------------------------------
// CCertManUIContainerPersonal::~CCertManUIContainerPersonal()
// Destructor
// ---------------------------------------------------------
//
CCertManUIContainerPersonal::~CCertManUIContainerPersonal()
    {
    CERTMANUILOGGER_ENTERFN(
        " CCertManUIContainerPersonal::~CCertManUIContainerPersonal" );

    delete iListBox;

    CERTMANUILOGGER_LEAVEFN(
        " CCertManUIContainerPersonal::~CCertManUIContainerPersonal" );
    }

// ---------------------------------------------------------
// CCertManUIContainerPersonal::ConstructL(
// const TRect& aRect, TInt aCurrentPosition, TInt aTopItem)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CCertManUIContainerPersonal::ConstructL(
    const TRect& aRect, TInt aCurrentPosition, TInt aTopItem )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerPersonal::ConstructL" );

    CreateWindowL();
    CreateListBoxL();
    UpdateListBoxL( aCurrentPosition, aTopItem );
    iListBox->ItemDrawer()->ColumnData()->SetIconArray( iIconHandler->CreateIconArrayL() );
    SetRect( aRect );

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerPersonal::ConstructL" );
    }

// ---------------------------------------------------------
// CCertManUIContainerPersonal::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CCertManUIContainerPersonal::SizeChanged()
    {
    iListBox->SetRect( Rect() );
    }

// ---------------------------------------------------------
// CCertManUIContainerPersonal::FocusChanged(TDrawNow aDrawNow)
// ---------------------------------------------------------
//
void CCertManUIContainerPersonal::FocusChanged( TDrawNow aDrawNow )
    {
    if ( iListBox )
        {
        iListBox->SetFocus( IsFocused(), aDrawNow );
        }
    }

// ---------------------------------------------------------
// CCertManUIContainerPersonal::HandleListBoxEventL(
// CEikListBox* /*aListBox*/, TListBoxEvent aEventType)
// ---------------------------------------------------------
//
void CCertManUIContainerPersonal::HandleListBoxEventL(
    CEikListBox* /*aListBox*/, TListBoxEvent aEventType )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerPersonal::HandleListBoxEventL" );

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
                        iListBox->CurrentItemIndex(), KCertTypePersonal, iEikonEnv );
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

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerPersonal::HandleListBoxEventL" );
    }


// ---------------------------------------------------------
// CCertManUIContainerPersonal::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CCertManUIContainerPersonal::CountComponentControls() const
    {
    // return number of controls inside this container
    return 1;
    }

// ---------------------------------------------------------
// CCertManUIContainerPersonal::ComponentControl(TInt /*aIndex*/) const
// ---------------------------------------------------------
//
CCoeControl* CCertManUIContainerPersonal::ComponentControl( TInt /*aIndex*/ ) const
    {
    return iListBox;
    }

// ---------------------------------------------------------
// CCertManUIContainerPersonal::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CCertManUIContainerPersonal::Draw( const TRect& aRect ) const
    {
    CWindowGc& gc = SystemGc();
    gc.SetPenStyle( CGraphicsContext::ENullPen );
    gc.SetBrushColor( KRgbGray );
    gc.SetBrushStyle( CGraphicsContext::ESolidBrush );
    gc.DrawRect( aRect );
    }

// ---------------------------------------------------------
// CCertManUIContainerPersonal::HandleControlEventL(
//    CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/)
// Cannot be changed to non-leaving function.
// L-function is required by the class definition, even if empty.
// ---------------------------------------------------------
//
void CCertManUIContainerPersonal::HandleControlEventL(
    CCoeControl* /*aControl*/, TCoeEvent /*aEventType*/ )
    {
    }

// ---------------------------------------------------------
// CCertManUIContainerPersonal::OfferKeyEventL(
// const TKeyEvent& aKeyEvent, TEventCode aType)
// Handles the key events arrow keys, delete key.
// ---------------------------------------------------------
//
TKeyResponse CCertManUIContainerPersonal::OfferKeyEventL(
    const TKeyEvent& aKeyEvent, TEventCode aType )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerPersonal::OfferKeyEventL" );

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
            ((CAknViewAppUi*)iAvkonAppUi)->ActivateLocalViewL( KCertManUIViewDeviceId );
            retval = EKeyWasConsumed;
            }
          else if ( aKeyEvent.iCode == EKeyLeftArrow )
            {
            ((CAknViewAppUi*)iAvkonAppUi)->ActivateLocalViewL( KCertManUIViewTrustedSiteId );
            retval = EKeyWasConsumed;
            }
          else if ( aKeyEvent.iCode == EKeyDelete
                  || aKeyEvent.iCode == EKeyBackspace )
            {
            TInt currentItemPersonal = iListBox->CurrentItemIndex();

            if ( ( currentItemPersonal >= 0 ) &&
                ( currentItemPersonal < iKeeper.iUserLabelEntries.Count() ))
              {
              // list is not empty
              if ( iKeeper.iCertificateHelper->ConfirmationQueryDeleteCertL(
                     KCertTypePersonal, iListBox ) ) // == ETrue
                  {
                  iParent.iCurrentPosition = iListBox->CurrentItemIndex();
                  iParent.iTopItem = iListBox->TopItemIndex();

                  DrawListBoxL( iParent.iCurrentPosition, iParent.iTopItem );
                  // goes here if at least one certificate is deleted
                  TRAPD( error, iKeeper.RefreshUserCertEntriesL() );
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
        iParent.MenuBar()->SetMenuTitleResourceId( R_CERTMANUI_MENUBAR_VIEW_PERSONAL );
        iParent.UpdateCba( R_CERTMANUI_OPTIONS_OPEN_BACK );
        }

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerPersonal::OfferKeyEventL" );

    return retval;
    }

// ---------------------------------------------------------
// CCertManUIContainerPersonal::CreateListBoxL()
// Creates listbox, sets empty listbox text.
// ---------------------------------------------------------
//
void CCertManUIContainerPersonal::CreateListBoxL()
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerPersonal::CreateListBoxL" );

    iListBox = new( ELeave ) CAknSingleStyleListBox;
    iListBox->SetContainerWindowL( *this );
    iListBox->ConstructL( this, EAknListBoxMarkableList );
    iListBox->CreateScrollBarFrameL( ETrue );
    iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(
        CEikScrollBarFrame::EOff,CEikScrollBarFrame::EAuto );
    iListBox->SetListBoxObserver( this );

    HBufC* stringHolder = StringLoader::LoadLC( R_TEXT_RESOURCE_VIEW_PERSONAL_EMPTY_LIST );
    iListBox->View()->SetListEmptyTextL( *stringHolder );
    CleanupStack::PopAndDestroy();  // stringHolder

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerPersonal::CreateListBoxL" );
    }

// ---------------------------------------------------------
// CCertManUIContainerPersonal::DrawListBoxL(TInt aCurrentPosition, TInt aTopItem)
// Draws listbox, fetches graphic icons for markable list
// ---------------------------------------------------------
//
void CCertManUIContainerPersonal::DrawListBoxL( TInt aCurrentPosition, TInt aTopItem )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerPersonal::DrawListBoxL" );

    if ( aCurrentPosition >= 0 )
        {
        iListBox->SetTopItemIndex( aTopItem );
        iListBox->SetCurrentItemIndex( aCurrentPosition );
        }
    ActivateL();
    DrawNow();

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerPersonal::DrawListBoxL" );
    }

// ---------------------------------------------------------
// CCertManUIContainerPersonal::UpdateListBoxL(TInt& aCurrentPosition, TInt& aTopItem)
// Gets list of certificates from CertManAPI, displays them,
// sets No label text to them if their names are empty
// ---------------------------------------------------------
//
void CCertManUIContainerPersonal::UpdateListBoxL( TInt& aCurrentPosition, TInt& aTopItem )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerPersonal::UpdateListBoxL" );

    iKeeper.StartWaitDialogL( ECertmanUiWaitDialog );
    CleanupCloseWaitDialogPushL( iKeeper );

    MDesCArray* itemList = iListBox->Model()->ItemTextArray();
    CDesCArray* itemArray = ( CDesCArray* )itemList;

    // List certificates
    if ( iKeeper.iCertsDatOK )  // == ETrue
        {
        // cacerts.dat is not corrupted, get count
        iListItemCount = iKeeper.iUserLabelEntries.Count();
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
          buf->Des() = ( iKeeper.iUserLabelEntries )[i]->iUserEntryLabel->Des();

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

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerPersonal::UpdateListBoxL" );
    }

// ---------------------------------------------------------
// CCertManUIContainerAuthority::HandleResourceChange
//
// ---------------------------------------------------------
//
void CCertManUIContainerPersonal::HandleResourceChange( TInt aType )
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
// CCertManUIContainerPersonal::IconArrayResourceId
// Loads Mark/Unmark icons from resource
// ---------------------------------------------------------
//
TInt CCertManUIContainerPersonal::IconArrayResourceId() const
    {
    return R_CERTMANUI_CERTIFICATE_LISTBOX_MARK_ICONS;
    }

// ---------------------------------------------------------
// CCertManUIContainerPersonal::CreateIconArrayL
// Creates Icon Array
// ---------------------------------------------------------
//
CAknIconArray* CCertManUIContainerPersonal::CreateIconArrayL() const
    {
    CAknIconArray* icons = new ( ELeave ) CAknIconArray( KGranularity );
    CleanupStack::PushL( icons );
    icons->ConstructFromResourceL( IconArrayResourceId() );
    CleanupStack::Pop(); // icons
    return icons;
    }


// ---------------------------------------------------------
// CCertManUIContainerPersonal::GetHelpContext
// This function is called when Help application is launched.
// (other items were commented in a header).
// ---------------------------------------------------------
//
void CCertManUIContainerPersonal::GetHelpContext( TCoeHelpContext& aContext ) const
    {
    aContext.iMajor = KUidCM;
    aContext.iContext = KCM_HLP_PERSONAL;
    }

// End of File
