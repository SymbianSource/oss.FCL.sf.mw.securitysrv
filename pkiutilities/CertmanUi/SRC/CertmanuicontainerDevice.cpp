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
* Description:   Implementation of class CCertManUIContainerDevice
*                Reads & updates certificate list
*                Handles markable list for certificates
*
*/


// INCLUDE FILES
#include <eikclbd.h>
#include <AknIconArray.h>
#include <certmanui.rsg>
#include <csxhelp/cm.hlp.hrh>   // for help context of CertManUI

#include "CertmanuiCertificateHelper.h"
#include "CertmanuiKeeper.h"
#include "CertmanuicontainerDevice.h"
#include "CertmanuiSyncWrapper.h"
#include "Certmanui.hrh"
#include "CertManUILogger.h"
#include "CertmanuiCommon.h"

#include "Certmanuidll.h"       // for applicationUID

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CCertManUIContainerDevice::CCertManUIContainerDevice(
// CCertManUIViewAuthority& aParent, CCertManUIKeeper& aKeeper)
// Constructor with parent
// ---------------------------------------------------------
//
CCertManUIContainerDevice::CCertManUIContainerDevice(
    CCertManUIViewDevice& aParent, CCertManUIKeeper& aKeeper ) :
    iParent ( aParent ), iKeeper ( aKeeper )
    {
    CERTMANUILOGGER_WRITE_TIMESTAMP(
        "CCertManUIContainerDevice::CCertManUIContainerDevice" );
    }

// ---------------------------------------------------------
// CCertManUIContainerDevice::~CCertManUIContainerDevice()
// Destructor
// ---------------------------------------------------------
//
CCertManUIContainerDevice::~CCertManUIContainerDevice()
    {
    CERTMANUILOGGER_ENTERFN(
        " CCertManUIContainerDevice::~CCertManUIContainerDevice" );

    delete iListBox;

    CERTMANUILOGGER_LEAVEFN(
        " CCertManUIContainerDevice::~CCertManUIContainerDevice" );
    }

// ---------------------------------------------------------
// CCertManUIContainerDevice::ConstructL(
// const TRect& aRect, TInt& aCurrentPosition, TInt& aTopItem)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CCertManUIContainerDevice::ConstructL(
    const TRect& aRect, TInt& aCurrentPosition, TInt& aTopItem )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerDevice::ConstructL" );

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

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerDevice::ConstructL" );
    }

// ---------------------------------------------------------
// CCertManUIContainerDevice::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CCertManUIContainerDevice::SizeChanged()
    {
    iListBox->SetRect( Rect() );
    }

// ---------------------------------------------------------
// CCertManUIContainerDevice::FocusChanged(TDrawNow aDrawNow)
// ---------------------------------------------------------
//
void CCertManUIContainerDevice::FocusChanged( TDrawNow aDrawNow )
    {
    if ( iListBox )
        {
        iListBox->SetFocus( IsFocused(), aDrawNow );
        }
    }

// ---------------------------------------------------------
// CCertManUIContainerDevice::HandleListBoxEventL(
// CEikListBox* /*aListBox*/, TListBoxEvent aEventType)
// ---------------------------------------------------------
//
void CCertManUIContainerDevice::HandleListBoxEventL(
    CEikListBox* /*aListBox*/, TListBoxEvent aEventType )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerDevice::HandleListBoxEventL" );

    switch ( aEventType )
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
                             iListBox->CurrentItemIndex(), KCertTypeDevice, iEikonEnv );
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

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerDevice::HandleListBoxEventL" );
    }

// ---------------------------------------------------------
// CCertManUIContainerDevice::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CCertManUIContainerDevice::CountComponentControls() const
    {
    // return number of controls inside this container
    return 1;
    }

// ---------------------------------------------------------
// CCertManUIContainerDevice::ComponentControl(
// TInt /*aIndex*/) const
// ---------------------------------------------------------
//
CCoeControl* CCertManUIContainerDevice::ComponentControl(
    TInt /*aIndex*/) const
    {
    return iListBox;
    }

// ---------------------------------------------------------
// CCertManUIContainerDevice::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CCertManUIContainerDevice::Draw( const TRect& aRect ) const
    {
    CWindowGc& gc = SystemGc();
    gc.SetPenStyle( CGraphicsContext::ENullPen );
    gc.SetBrushColor( KRgbGray );
    gc.SetBrushStyle( CGraphicsContext::ESolidBrush );
    gc.DrawRect( aRect );
    }

// ---------------------------------------------------------
// CCertManUIContainerDevice::HandleControlEventL(
//    CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/)
// Cannot be changed to non-leaving function.
// L-function is required by the class definition, even if empty.
// ---------------------------------------------------------
//
void CCertManUIContainerDevice::HandleControlEventL(
    CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/)
    {
    }

// ---------------------------------------------------------
// CCertManUIContainerDevice::OfferKeyEventL(
// const TKeyEvent& aKeyEvent, TEventCode aType)
// Handles the key events arrow keys, delete key.
// ---------------------------------------------------------
//
TKeyResponse CCertManUIContainerDevice::OfferKeyEventL(
    const TKeyEvent& aKeyEvent, TEventCode aType )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerDevice::OfferKeyEventL" );

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
            ((CAknViewAppUi*)iAvkonAppUi)->ActivateLocalViewL( KCertManUIViewAuthorityId );
            retval = EKeyWasConsumed;
            }
        else if ( aKeyEvent.iCode == EKeyLeftArrow )
            {
            ((CAknViewAppUi*)iAvkonAppUi)->ActivateLocalViewL( KCertManUIViewPersonalId );
            retval = EKeyWasConsumed;
            }
        else if ( aKeyEvent.iCode == EKeyDelete
                  || aKeyEvent.iCode == EKeyBackspace )
            {
            TInt currentItemAuthority = iListBox->CurrentItemIndex();

            if ((currentItemAuthority >= 0) &&
                ( currentItemAuthority < iKeeper.iDeviceLabelEntries.Count()))
                {
                // list is not empty
                if ( iKeeper.iCertificateHelper->ConfirmationQueryDeleteCertL(
                                               KCertTypeDevice, iListBox ) ) // == ETrue
                    {
                    iParent.iCurrentPosition = iListBox->CurrentItemIndex();
                    iParent.iTopItem = iListBox->TopItemIndex();

                    DrawListBoxL(iParent.iCurrentPosition, iParent.iTopItem );
                    // goes here if at least one certificate is deleted
                    TRAPD( error, iKeeper.RefreshDeviceCertEntriesL() );
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
        iParent.MenuBar()->SetMenuTitleResourceId( R_CERTMANUI_MENUBAR_VIEW_DEVICE );
        iParent.UpdateCba( R_CERTMANUI_OPTIONS_OPEN_BACK );
        }

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerDevice::OfferKeyEventL" );

    return retval;
    }

// ---------------------------------------------------------
// CCertManUIContainerDevice::CreateListBoxL()
// Creates listbox, sets empty listbox text.
// ---------------------------------------------------------
//
void CCertManUIContainerDevice::CreateListBoxL()
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerDevice::CreateListBoxL()" );

    iListBox = new( ELeave ) CAknSingleStyleListBox;
    iListBox->SetContainerWindowL( *this );
    iListBox->ConstructL( this, EAknListBoxMarkableList );
    iListBox->CreateScrollBarFrameL( ETrue );
    iListBox->ScrollBarFrame()->
    SetScrollBarVisibilityL( CEikScrollBarFrame::EOff,CEikScrollBarFrame::EAuto );
    iListBox->SetListBoxObserver( this );

    HBufC* stringHolder = StringLoader::LoadLC(
            R_TEXT_RESOURCE_VIEW_DEVICE_EMPTY_LIST );
    iListBox->View()->SetListEmptyTextL( *stringHolder );
    CleanupStack::PopAndDestroy();  // stringHolder

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerDevice::CreateListBoxL()" );
    }

// ---------------------------------------------------------
// CCertManUIContainerDevice::DrawListBoxL(TInt aCurrentPosition, TInt aTopItem)
// Draws listbox, fetches graphic icons for markable list
// ---------------------------------------------------------
//
void CCertManUIContainerDevice::DrawListBoxL(
    TInt aCurrentPosition, TInt aTopItem )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerDevice::DrawListBoxL" );

    if ( aCurrentPosition >= 0 )
        {
        iListBox->SetTopItemIndex( aTopItem );
        iListBox->SetCurrentItemIndex( aCurrentPosition );
        }
    ActivateL();
    DrawNow();

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerDevice::DrawListBoxL" );
  }

// ---------------------------------------------------------
// CCertManUIContainerDevice::UpdateListBoxL(TInt& aCurrentPosition, TInt& aTopItem)
// Gets list of certificates from CertManAPI, displays them,
// sets No label/No subject text to them if their names are empty
// ---------------------------------------------------------
//
void CCertManUIContainerDevice::UpdateListBoxL(
    TInt& aCurrentPosition, TInt& aTopItem )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerDevice::UpdateListBoxL()" );

    iKeeper.StartWaitDialogL( ECertmanUiWaitDialog );
    CleanupCloseWaitDialogPushL( iKeeper );

    MDesCArray* itemList = iListBox->Model()->ItemTextArray();
    CDesCArray* itemArray = ( CDesCArray* )itemList;

    if ( iKeeper.iCertsDatOK )  // == ETrue
        {
        // database is not corrupted, get count
        iListItemCount = iKeeper.iDeviceLabelEntries.Count();
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
          buf->Des() = ( iKeeper.iDeviceLabelEntries )[i]->iDeviceEntryLabel->Des();

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

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerDevice::UpdateListBoxL()" );
    }

// ---------------------------------------------------------
// CCertManUIContainerDevice::HandleResourceChange
//
// ---------------------------------------------------------
//
void CCertManUIContainerDevice::HandleResourceChange( TInt aType )
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
// CCertManUIContainerDevice::IconArrayResourceId
// Loads Mark/Unmark icons from resource
// ---------------------------------------------------------
//
TInt CCertManUIContainerDevice::IconArrayResourceId() const
    {
    return R_CERTMANUI_CERTIFICATE_LISTBOX_MARK_ICONS;
    }

// ---------------------------------------------------------
// CCertManUIContainerDevice::CreateIconArrayL
// Creates Icon Array
// ---------------------------------------------------------
//
CAknIconArray* CCertManUIContainerDevice::CreateIconArrayL() const
    {
    CAknIconArray* icons = new (ELeave) CAknIconArray( KGranularity );
    CleanupStack::PushL( icons );
    icons->ConstructFromResourceL( IconArrayResourceId() );
    CleanupStack::Pop(); // icons
    return icons;
    }

// ---------------------------------------------------------
// CCertManUIContainerDevice::GetHelpContext
// This function is called when Help application is launched.
// (other items were commented in a header).
// ---------------------------------------------------------
//
void CCertManUIContainerDevice::GetHelpContext(
    TCoeHelpContext& aContext ) const
    {
    aContext.iMajor = KUidCM;
    aContext.iContext = KCM_HLP_DEVICE;
    }

// End of File
