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
* Description:   Implementation of class CCertManUIViewPersonal
*                Updates Options list depending of the number of
*                marked and unmarked certificates in the listbox.
*                Handles softkey and Option list commands.
*                Handles certificate deletion including confirmation and
*                read-only notes.
*                Shows certificate details view with appropriate
*                warning notes if certificate is corrupted/expired/not valid.
*
*/


// INCLUDE FILES

#include <aknmessagequerydialog.h>
#include <aknnotewrappers.h>            // for warning & information notes
#include <certmanui.rsg>
#include <ErrorUI.h>
#include "CertmanuiCertificateHelper.h"
#include "CertmanuiKeeper.h"
#include "CertmanuicontainerPersonal.h"
#include "CertmanuiSyncWrapper.h"
#include "Certmanui.hrh"
#include "CertmanuiCommon.h"
#include "CertManUILogger.h"

#include    <hlplch.h>                  // For HlpLauncher
#include    <featmgr.h>                 // For FeatureManager


// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CCertManUIViewPersonal::CCertManUIViewPersonal(CCertManUIKeeper& aKeeper)
// : iKeeper(aKeeper), iCrMgr(iKeeper.CertManager())
// Constructor
// ---------------------------------------------------------
//
CCertManUIViewPersonal::CCertManUIViewPersonal( CCertManUIKeeper& aKeeper )
: iKeeper( aKeeper )
    {
    CERTMANUILOGGER_WRITE_TIMESTAMP( "CCertManUIViewPersonal::CCertManUIViewPersonal" );
    }

// ---------------------------------------------------------
// CCertManUIViewPersonal::~CCertManUIViewPersonal()
// Destructor
// ---------------------------------------------------------
//
CCertManUIViewPersonal::~CCertManUIViewPersonal()
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUIViewPersonal::~CCertManUIViewPersonal" );

    if ( iContainerPersonal )
        {
        AppUi()->RemoveFromViewStack( *this, iContainerPersonal );
        delete iContainerPersonal;
        }

    CERTMANUILOGGER_LEAVEFN( "CCertManUIViewPersonal::~CCertManUIViewPersonal" );
    }

// ---------------------------------------------------------
// CCertManUIViewPersonal* CCertManUIViewPersonal::NewL(const TRect& /*aRect*/,
//                                CCertManUIKeeper& aKeeper)
// ---------------------------------------------------------
//
CCertManUIViewPersonal* CCertManUIViewPersonal::NewL( const TRect& /*aRect*/,
                                CCertManUIKeeper& aKeeper )
    {
    CCertManUIViewPersonal* self = new ( ELeave ) CCertManUIViewPersonal( aKeeper );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// ---------------------------------------------------------
// CCertManUIViewPersonal* CCertManUIViewPersonal::NewLC(const TRect& /*aRect*/,
//                                 CCertManUIKeeper& aKeeper)
// ---------------------------------------------------------
//
CCertManUIViewPersonal* CCertManUIViewPersonal::NewLC( const TRect& /*aRect*/,
                                 CCertManUIKeeper& aKeeper )
    {
    CCertManUIViewPersonal* self = new ( ELeave ) CCertManUIViewPersonal( aKeeper );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------
// CCertManUIViewPersonal::ConstructL()
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CCertManUIViewPersonal::ConstructL()
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUIViewPersonal::ConstructL" );

    BaseConstructL( R_CERTMANUI_VIEW_MAIN_PERSONAL );

    CERTMANUILOGGER_LEAVEFN( "CCertManUIViewPersonal::ConstructL" );
    }

// ---------------------------------------------------------
// CCertManUIViewPersonal::DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane)
// Updates Options list with correct items depending on
// whether the listbox is empty or if it has any marked items
// ---------------------------------------------------------
//
void CCertManUIViewPersonal::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane)
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUIViewPersonal::DynInitMenuPaneL" );

    if (( iKeeper.iWrapper->IsActive() ) ||
        ( iContainerPersonal->iListBox->CurrentItemIndex() >= iKeeper.iUserLabelEntries.Count() ))
        {
        return;
        }

    const CListBoxView::CSelectionIndexArray* selections =
            iContainerPersonal->iListBox->SelectionIndexes();
    const TInt markedCount = selections->Count();
    const TInt currentItemIndex = iContainerPersonal->iListBox->CurrentItemIndex();

    switch ( aResourceId )
        {
        case R_CERTMANUI_VIEW_MENU_PERSONAL:
            {
            if( !FeatureManager::FeatureSupported( KFeatureIdHelp ) )
                {
                aMenuPane->DeleteMenuItem( EAknCmdHelp );
                }

            // the certificate list is empty, all except Exit are dimmed
            if ( currentItemIndex == -1 )
                {
                aMenuPane->SetItemDimmed( ECertManUICmdAppViewDtls, ETrue );
                aMenuPane->SetItemDimmed( ECertManUICmdAppDelete, ETrue );
                aMenuPane->SetItemDimmed( ECertManUICmdMoveToDevice, ETrue );
                aMenuPane->SetItemDimmed( ECertManUICmdMarkUnmark, ETrue );
                }
            else
                {
                if ( markedCount == 1 )
                    {
                    aMenuPane->SetItemDimmed( ECertManUICmdAppViewDtls, ETrue );
                    }

                if ( markedCount > 1 )
                    {
                    aMenuPane->SetItemDimmed( ECertManUICmdAppViewDtls, ETrue );
                    aMenuPane->SetItemDimmed( ECertManUICmdMoveToDevice, ETrue );
                    }

                if ( !iKeeper.iCertificateHelper->IsOneMarkedCertificateDeletable(
                        iContainerPersonal->iListBox, KCertTypePersonal ) )
                    {
                    aMenuPane->SetItemDimmed( ECertManUICmdAppDelete, ETrue );
                    aMenuPane->SetItemDimmed( ECertManUICmdMoveToDevice, ETrue );
                    }
                }
            break;
            }

        case R_CERTMANUI_MENUPANE_OPTIONS_MARKED:
            {
            if ( markedCount > 1 )
                {
                aMenuPane->SetItemDimmed( ECertManUICmdMoveToDevice, ETrue );
                }

            if ( !iKeeper.iCertificateHelper->IsOneMarkedCertificateDeletable(
                    iContainerPersonal->iListBox, KCertTypePersonal ) )
                {
                aMenuPane->SetItemDimmed( ECertManUICmdAppDelete, ETrue );
                }

            // not available for personal certificates
            aMenuPane->SetItemDimmed( ECertManUICmdMoveToPersonal, ETrue );
            }
            // FALLTHROUGH

        case R_CERTMANUI_MENU_PANE_MARK_UNMARK:
            {
            TInt index = 0;
            TKeyArrayFix key( 0, ECmpTInt );
            if ( selections->Find( currentItemIndex, key, index ) )
                {   // focus is on unmarked item
                aMenuPane->SetItemDimmed( ECertManUICmdUnmark, ETrue );
                }
            else
                {   // focus is on marked item
                aMenuPane->SetItemDimmed( ECertManUICmdMark, ETrue );
                }

            // if all items are marked, dim "mark all" option
            if ( markedCount == iContainerPersonal->iListBox->Model()->NumberOfItems() )
                {
                aMenuPane->SetItemDimmed( ECertManUICmdMarkAll, ETrue );
                }

            // if no items are marked, dim "unmark all" option
            if ( markedCount == 0 )
                {
                aMenuPane->SetItemDimmed( ECertManUICmdUnmarkAll, ETrue );
                }
            break;
            }

        default:
            break;
        }

  CERTMANUILOGGER_LEAVEFN( "CCertManUIViewPersonal::DynInitMenuPaneL" );
  }

// ---------------------------------------------------------
// CCertManUIViewPersonal::Id() const
// Returns Personal view id
// ---------------------------------------------------------
//
TUid CCertManUIViewPersonal::Id() const
    {
    return KCertManUIViewPersonalId;
    }

// ---------------------------------------------------------
// CCertManUIViewPersonal::HandleCommandL(TInt aCommand)
// Handles Softkey and Options list commands
// ---------------------------------------------------------
//
void CCertManUIViewPersonal::HandleCommandL(TInt aCommand)
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUIViewPersonal::HandleCommandL" );

    if (( iKeeper.iWrapper->IsActive() ) ||
        ( iContainerPersonal->iListBox->CurrentItemIndex() >= iKeeper.iUserLabelEntries.Count() ))
        {
        return;
        }

    switch ( aCommand )
        {
        case EAknSoftkeyBack:
            {
            // Activate main view
            AppUi()->ActivateLocalViewL( KCertManUIViewMainId );
            break;
            }
        // put the view specific menu commands here
        case EAknCmdExit:
            {
            ((CAknViewAppUi*)iAvkonAppUi)->HandleCommandL( EAknCmdExit );
            break;
            }
        case ECertManUICmdAppViewDtls:
            {
            iKeeper.iCertificateHelper->MessageQueryViewDetailsL(
                iContainerPersonal->iListBox->CurrentItemIndex(), KCertTypePersonal, iEikonEnv );
            break;
            }
        case ECertManUICmdAppDelete:
            {
            if ( iKeeper.iCertificateHelper->ConfirmationQueryDeleteCertL( KCertTypePersonal,
                                                      iContainerPersonal->iListBox ))
                {
                // goes here if at least one certificate is deleted
                iCurrentPosition = iContainerPersonal->iListBox->CurrentItemIndex();
                iTopItem = iContainerPersonal->iListBox->TopItemIndex();

                iContainerPersonal->DrawListBoxL(iCurrentPosition, iTopItem );
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
                UpdateMenuBar();
                }
            break;
            }
       case ECertManUICmdMoveToDevice:
            {
            HBufC* prompt = StringLoader::LoadLC(
                        R_QTN_CM_CONF_MOVE_TO_DEVICE );
            TInt selection = iKeeper.iCertificateHelper->DisplayConfirmationLD(prompt);
            if ( selection )
                {                
                TInt focusPosition = 0;
                
                const CArrayFix<TInt>* array = iContainerPersonal->iListBox->SelectionIndexes();
                
                if ( array->Count() == 1 )
                    {
                    // item is marked
                    focusPosition = array->At( 0 );
                    }
                else
                    {
                    // There is no marked item. Select higlighted item 
                    focusPosition = iContainerPersonal->iListBox->CurrentItemIndex();
                    }
                                                
                CCTCertInfo* entry = iKeeper.iUserLabelEntries[ focusPosition ]->iUserEntry;

                // Move key first
                TCTKeyAttributeFilter keyFilter;
                keyFilter.iKeyId = entry->SubjectKeyId();
                keyFilter.iPolicyFilter =  TCTKeyAttributeFilter::EAllKeys;

                TRAPD( error, iKeeper.iCertificateHelper->MoveKeyL( keyFilter,
                                KCMFileKeyStoreTokenUid, KCMDeviceKeyStoreTokenUid ));

                if ( error == KErrNone )
                    {
                    // Move certificate
                    iKeeper.iCertificateHelper->MoveCertL( *entry, focusPosition, iContainerPersonal->iListBox,
                                    KCMFileCertStoreTokenUid, KCMDeviceCertStoreTokenUid );

                    TRAP( error, iKeeper.RefreshUserCertEntriesL() );
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

                    MDesCArray* itemList = iContainerPersonal->iListBox->Model()->ItemTextArray();
                    CDesCArray* itemArray = ( CDesCArray* )itemList;
                    if (( itemArray != NULL ) && ( itemArray->Count() == 0 ))
                        {
                        iCurrentPosition = 0;
                        iTopItem = 0;
                        iContainerPersonal->UpdateListBoxL( iCurrentPosition, iTopItem );
                        }
                     else
                        {
                        iCurrentPosition = iContainerPersonal->iListBox->CurrentItemIndex();
                        iTopItem = iContainerPersonal->iListBox->TopItemIndex();
                        }

                    iContainerPersonal->DrawListBoxL(iCurrentPosition, iTopItem );

                    UpdateMenuBar();
                    }
                }
            break;
            }
        case ECertManUICmdMark:
        case ECertManUICmdUnmark:
        case ECertManUICmdMarkAll:
        case ECertManUICmdUnmarkAll:
            {
            // Gets pointer of current listbox.
            CEikListBox* listbox = iContainerPersonal->iListBox;
            if ( listbox )
                {
                AknSelectionService::HandleMarkableListProcessCommandL(
                    aCommand, listbox );
                iKeeper.iCertificateHelper->HandleMarkableListCommandL( aCommand,
                                                iContainerPersonal->iListBox  );

                UpdateMenuBar();
                }
            break;
            }

        case EAknCmdHelp:
            {
            if( FeatureManager::FeatureSupported( KFeatureIdHelp ) )
                {
                HlpLauncher::LaunchHelpApplicationL( iEikonEnv->WsSession(), AppUi()->AppHelpContextL() );
                }
            break;
            }

        default:
            {
            AppUi()->HandleCommandL( aCommand );
            break;
            }
        }

    CERTMANUILOGGER_LEAVEFN( "CCertManUIViewPersonal::HandleCommandL" );
    }

// ---------------------------------------------------------
// CCertManUIViewPersonal::HandleClientRectChange()
// ---------------------------------------------------------
//
void CCertManUIViewPersonal::HandleClientRectChange()
    {
    if ( iContainerPersonal )
        {
        iContainerPersonal->SetRect( ClientRect() );
        }
    }

// ---------------------------------------------------------
// CCertManUIViewPersonal::DoActivateL(const TVwsViewId& /*aPrevViewId*/,TUid /*aCustomMessageId*/, const TDesC8& /*aCustomMessage*/)
// Updates the view when opening it
// ---------------------------------------------------------
//
void CCertManUIViewPersonal::DoActivateL(const TVwsViewId& aPrevViewId, TUid /*aCustomMessageId*/, const TDesC8& /*aCustomMessage*/)
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUIViewPersonal::DoActivateL" );

    iKeeper.StartWaitDialogL( ECertmanUiWaitDialog );
    CleanupCloseWaitDialogPushL( iKeeper );

    TRAPD ( error, iKeeper.RefreshUserCertEntriesL() );
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

    // If this view is being activated from main view,
    // current position is 0. The declaration of KCertManUIViewMainId is
    // located in certmanuiviewid.h which is not public header --> constant
    // is being used.
    if( aPrevViewId.iViewUid == KCertManUIViewMainId )
        {
        iCurrentPosition = 0;
        }

    if ( iContainerPersonal )
        {
        ((CAknViewAppUi*)iAvkonAppUi)->RemoveFromViewStack( *this, iContainerPersonal );
        delete iContainerPersonal;
        iContainerPersonal = NULL;
        }
    iContainerPersonal = new ( ELeave ) CCertManUIContainerPersonal( *this, iKeeper );
    iContainerPersonal->SetMopParent(this);
    iContainerPersonal->ConstructL( ClientRect(), iCurrentPosition, iTopItem );

    iKeeper.ActivateTitleL( KViewTitleCertManUIPersonal );

    iContainerPersonal->DrawListBoxL( iCurrentPosition, iTopItem );
    ((CAknViewAppUi*)iAvkonAppUi)->AddToStackL( *this, iContainerPersonal );

    UpdateMenuBar();

    CleanupStack::PopAndDestroy();    // closes wait dialog

    CERTMANUILOGGER_LEAVEFN( "CCertManUIViewPersonal::DoActivateL" );
    }

// ---------------------------------------------------------
// CCertManUIViewPersonal::DoDeactivate()
// Saves focus position when closing view
// ---------------------------------------------------------
//
void CCertManUIViewPersonal::DoDeactivate()
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUIViewPersonal::DoDeactivate" );

    if( iContainerPersonal )
        {
        CEikColumnListBox* listBox = iContainerPersonal->iListBox;
        if( listBox )
            {
            iCurrentPosition = listBox->CurrentItemIndex();
            iTopItem = listBox->TopItemIndex();
            }

        iAvkonAppUi->RemoveFromViewStack( *this, iContainerPersonal );

        delete iContainerPersonal;
        iContainerPersonal = NULL;
        }

    CERTMANUILOGGER_LEAVEFN( "CCertManUIViewPersonal::DoDeactivate" );
    }

// ---------------------------------------------------------
// CCertManUIViewPersonal::Container()
// Returns iContainerPersonal to be used in CertManUIContainerPersonal
// ---------------------------------------------------------
//
CCoeControl* CCertManUIViewPersonal::Container()
    {
    return iContainerPersonal;
    }

// ---------------------------------------------------------
// CCertManUIViewPersonal::UpdateMenuBar()
// Updates the menu bar
// ---------------------------------------------------------
//
void CCertManUIViewPersonal::UpdateMenuBar()
    {
    CEikButtonGroupContainer*  cba = Cba();

    if ( cba == NULL )
        {
        return;
        }

    if ( iKeeper.iUserLabelEntries.Count() == 0 )
        {
        // No certificate. Don't show MSK
        UpdateCba( R_CERTMANUI_OPTIONS_OPEN_BACK );
        cba->MakeCommandVisible( ECertManUICmdAppViewDtls, EFalse );
        }
    else
        {
        if ( iContainerPersonal->iListBox->SelectionIndexes()->Count() > 0 )
            {
            MenuBar()->SetContextMenuTitleResourceId( R_CERTMANUI_MENUBAR_OPTIONS_MARKED );
            UpdateCba( R_CERTMANUI_OPTIONS_CONTEXT_BACK );
            }
        else
            {
            MenuBar()->SetMenuTitleResourceId( R_CERTMANUI_MENUBAR_VIEW_PERSONAL );
            UpdateCba( R_CERTMANUI_OPTIONS_OPEN_BACK );
            }
         cba->MakeCommandVisible( ECertManUICmdAppViewDtls, ETrue );
         }
    }

// ---------------------------------------------------------
// CCertManUIViewPersonal::UpdateCba( TInt aCbaResourceId )
// Updates cba
// ---------------------------------------------------------
//
void CCertManUIViewPersonal::UpdateCba( TInt aCbaResourceId )
    {
    CEikButtonGroupContainer*  cba = Cba();

    if ( cba )
        {
        cba->SetCommandSetL( aCbaResourceId );
        cba->DrawNow();
        }
    }

// End of File

