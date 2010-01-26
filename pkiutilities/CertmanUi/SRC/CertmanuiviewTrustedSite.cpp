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
* Description:   Implementation of class CCertManUIViewTrustedSite
*                Updates Options list depending of the number of
*                marked and unmarked certificates in the listbox.
*                Handles softkey and Option list commands.
*                Handles certificate deletion including confirmation note.
*                Shows certificate details view with appropriate
*                warning notes if certificate is corrupted/expired/not valid.
*
*/


// INCLUDE FILES
#include <aknmessagequerydialog.h>
#include <aknnotewrappers.h>            // for warning & information notes
#include <X509CertNameParser.h>
#include <ErrorUI.h>
#include <x509cert.h>
#include <pkixcertchain.h>              //for validation
#include <certmanui.rsg>
#include "CertmanuiCertificateHelper.h"
#include "CertmanuiKeeper.h"
#include "CertmanuiviewTrustedSite.h"
#include "CertmanuicontainerTrustedSite.h"
#include "Certmanui.hrh"
#include "CertmanuiSyncWrapper.h"
#include "CertmanuiCommon.h"
#include "CertManUILogger.h"

#include    <hlplch.h>                  // For HlpLauncher
#include    <featmgr.h>                 // For FeatureManager


// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CCertManUIViewTrustedSite::CCertManUIViewTrustedSite(CCertManUIKeeper& aKeeper)
// : iKeeper(aKeeper), iCrMgr(iKeeper.CertManager())
// Constructor
// ---------------------------------------------------------
//
CCertManUIViewTrustedSite::CCertManUIViewTrustedSite( CCertManUIKeeper& aKeeper )
    : iKeeper( aKeeper )
    {
    CERTMANUILOGGER_WRITE_TIMESTAMP(
        "CCertManUIViewTrustedSite::CCertManUIViewTrustedSite" );
    }

// ---------------------------------------------------------
// CCertManUIViewTrustedSite::~CCertManUIViewTrustedSite()
// Destructor
// ---------------------------------------------------------
//
CCertManUIViewTrustedSite::~CCertManUIViewTrustedSite()
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIViewTrustedSite::~CCertManUIViewTrustedSite" );

    if ( iContainerTrustedSite )
        {
        AppUi()->RemoveFromViewStack( *this, iContainerTrustedSite );
        delete iContainerTrustedSite;
        }

     CERTMANUILOGGER_LEAVEFN( " CCertManUIViewTrustedSite::~CCertManUIViewTrustedSite" );
    }

// ---------------------------------------------------------
// CCertManUIViewTrustedSite* CCertManUIViewTrustedSite::NewL(
//      const TRect& /*aRect*/, CCertManUIKeeper& aKeeper)
// ---------------------------------------------------------
//
CCertManUIViewTrustedSite* CCertManUIViewTrustedSite::NewL(
    const TRect& /*aRect*/, CCertManUIKeeper& aKeeper )
    {
    CCertManUIViewTrustedSite* self =
        new ( ELeave ) CCertManUIViewTrustedSite( aKeeper );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// ---------------------------------------------------------
// CCertManUIViewTrustedSite* CCertManUIViewTrustedSite::NewLC(
//  const TRect& /*aRect*/, CCertManUIKeeper& aKeeper)
// ---------------------------------------------------------
//
CCertManUIViewTrustedSite* CCertManUIViewTrustedSite::NewLC(
    const TRect& /*aRect*/, CCertManUIKeeper& aKeeper )
    {
    CCertManUIViewTrustedSite* self =
        new ( ELeave ) CCertManUIViewTrustedSite( aKeeper );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------
// CCertManUIViewTrustedSite::ConstructL()
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CCertManUIViewTrustedSite::ConstructL()
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIViewTrustedSite::ConstructL" );

    BaseConstructL( R_CERTMANUI_VIEW_MAIN_TRUSTED_SITE );

    CERTMANUILOGGER_LEAVEFN( " CCertManUIViewTrustedSite::ConstructL" );
    }

// ---------------------------------------------------------
// CCertManUIViewTrustedSite::DynInitMenuPaneL(
//      TInt aResourceId,CEikMenuPane* aMenuPane)
// Updates Options list with correct items depending on
// whether the listbox is empty or if it has any marked items
// ---------------------------------------------------------
//
void CCertManUIViewTrustedSite::DynInitMenuPaneL(
    TInt aResourceId, CEikMenuPane* aMenuPane )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIViewTrustedSite::DynInitMenuPaneL" );

    if (( iKeeper.iWrapper->IsActive()) ||
        ( iContainerTrustedSite->iListBox->CurrentItemIndex() >= iKeeper.iPeerLabelEntries.Count() ))
        {
        return;
        }

    const CListBoxView::CSelectionIndexArray* selections =
            iContainerTrustedSite->iListBox->SelectionIndexes();
    const TInt markedCount = selections->Count();
    const TInt currentItemIndex = iContainerTrustedSite->iListBox->CurrentItemIndex();

    switch ( aResourceId )
        {
        case R_CERTMANUI_VIEW_MENU_TRUSTED_SITES:
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
                aMenuPane->SetItemDimmed( ECertManUICmdMarkUnmark, ETrue );
                }
            else
                {
                if ( markedCount > 0 )
                    {
                    aMenuPane->SetItemDimmed( ECertManUICmdAppViewDtls, ETrue );
                    }

                if ( !iKeeper.iCertificateHelper->IsOneMarkedCertificateDeletable(
                        iContainerTrustedSite->iListBox, KCertTypeTrustedSite ) )
                    {
                    aMenuPane->SetItemDimmed( ECertManUICmdAppDelete, ETrue );
                    }
                }
            break;
            }

        case R_CERTMANUI_MENUPANE_OPTIONS_MARKED:
            {
            if ( !iKeeper.iCertificateHelper->IsOneMarkedCertificateDeletable(
                    iContainerTrustedSite->iListBox, KCertTypeTrustedSite ) )
                {
                aMenuPane->SetItemDimmed( ECertManUICmdAppDelete, ETrue );
                }

            // not available for trusted site certificates
            aMenuPane->SetItemDimmed( ECertManUICmdMoveToDevice, ETrue );
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
            if ( markedCount == iContainerTrustedSite->iListBox->Model()->NumberOfItems() )
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

     CERTMANUILOGGER_LEAVEFN( " CCertManUIViewTrustedSite::DynInitMenuPaneL" );
    }

// ---------------------------------------------------------
// CCertManUIViewTrustedSite::Id() const
// Returns Authority view id
// ---------------------------------------------------------
//
TUid CCertManUIViewTrustedSite::Id() const
    {
    return KCertManUIViewTrustedSiteId;
    }

// ---------------------------------------------------------
// CCertManUIViewTrustedSite::HandleCommandL(TInt aCommand)
// Handles Softkey and Options list commands
// ---------------------------------------------------------
//
void CCertManUIViewTrustedSite::HandleCommandL( TInt aCommand )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIViewTrustedSite::HandleCommandL" );

    if (( iKeeper.iWrapper->IsActive() ) ||
        ( iContainerTrustedSite->iListBox->CurrentItemIndex() >= iKeeper.iPeerLabelEntries.Count() ))
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
                iContainerTrustedSite->iListBox->CurrentItemIndex(), KCertTypeTrustedSite, iEikonEnv );
            break;
            }
        case ECertManUICmdAppDelete:
            {
            if ( iKeeper.iCertificateHelper->ConfirmationQueryDeleteCertL( KCertTypeTrustedSite,
                                                    iContainerTrustedSite->iListBox ) )
                {
                // goes here if at least one certificate is deleted
                iCurrentPosition = iContainerTrustedSite->iListBox->CurrentItemIndex();
                iTopItem = iContainerTrustedSite->iListBox->TopItemIndex();

                iContainerTrustedSite->DrawListBoxL(iCurrentPosition, iTopItem );
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
                UpdateMenuBar();
                }
            break;
            }
        case ECertManUICmdMark:
        case ECertManUICmdUnmark:
        case ECertManUICmdMarkAll:
        case ECertManUICmdUnmarkAll:
            {
            // Gets pointer of current listbox.
            CEikListBox* listbox = iContainerTrustedSite->iListBox;
            if ( listbox )
                {
                AknSelectionService::HandleMarkableListProcessCommandL(
                    aCommand, listbox );
                iKeeper.iCertificateHelper->HandleMarkableListCommandL(
                                      aCommand, iContainerTrustedSite->iListBox );

                UpdateMenuBar();
                }

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
            AppUi()->HandleCommandL( aCommand );
            break;
            }
        }

    CERTMANUILOGGER_LEAVEFN( " CCertManUIViewTrustedSite::HandleCommandL" );
    }

// ---------------------------------------------------------
// CCertManUIViewTrustedSite::HandleClientRectChange()
// ---------------------------------------------------------
//
void CCertManUIViewTrustedSite::HandleClientRectChange()
    {
    if ( iContainerTrustedSite )
        {
        iContainerTrustedSite->SetRect( ClientRect() );
        }
    }

// ---------------------------------------------------------
// CCertManUIViewTrustedSite::DoActivateL(const TVwsViewId& /*aPrevViewId*/,
//      TUid /*aCustomMessageId*/, const TDesC8& /*aCustomMessage*/)
// Updates the view when opening it
// ---------------------------------------------------------
//
void CCertManUIViewTrustedSite::DoActivateL(
    const TVwsViewId& aPrevViewId,
    TUid /*aCustomMessageId*/, const TDesC8& /*aCustomMessage*/)
    {
     CERTMANUILOGGER_ENTERFN( " CCertManUIViewTrustedSite::DoActivateL" );

     iKeeper.StartWaitDialogL( ECertmanUiWaitDialog );
     CleanupCloseWaitDialogPushL( iKeeper );

     TRAPD ( error, iKeeper.RefreshPeerCertEntriesL() );
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

    if ( iContainerTrustedSite )
        {
        ((CAknViewAppUi*)iAvkonAppUi)->RemoveFromViewStack(
            *this, iContainerTrustedSite );

        delete iContainerTrustedSite;
        iContainerTrustedSite = NULL;
        }
    iContainerTrustedSite = new (ELeave) CCertManUIContainerTrustedSite(
        *this, iKeeper );

    iContainerTrustedSite->SetMopParent( this );
    iContainerTrustedSite->ConstructL(
        ClientRect(), iCurrentPosition, iTopItem );

    if ( !iKeeper.iCertsDatOK )
        {
        iCurrentPosition = 0;
        iTopItem = 0;
        }

    iKeeper.ActivateTitleL( KViewTitleCertManUITrustedSite );

    iContainerTrustedSite->DrawListBoxL( iCurrentPosition, iTopItem );
    ((CAknViewAppUi*)iAvkonAppUi)->AddToStackL( *this, iContainerTrustedSite );

    UpdateMenuBar();

    CleanupStack::PopAndDestroy();    // closes wait dialog

    iKeeper.iStartup = EFalse;

    CERTMANUILOGGER_LEAVEFN( " CCertManUIViewTrustedSite::DoActivateL" );
    }

// ---------------------------------------------------------
// CCertManUIViewTrustedSite::DoDeactivate()
// Saves focus position when closing view
// ---------------------------------------------------------
//
void CCertManUIViewTrustedSite::DoDeactivate()
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIViewTrustedSite::DoDeactivate" );

    if ( iKeeper.iCertsDatOK )
        {
        iCurrentPosition = iContainerTrustedSite->iListBox->CurrentItemIndex();
        iTopItem = iContainerTrustedSite->iListBox->TopItemIndex();
        // do not destroy iContainerTrustedSite here because
        // TrustSettings view needs it !
        }
    else
        {
        iCurrentPosition = 0;
        iTopItem = 0;
        }

    CERTMANUILOGGER_LEAVEFN( " CCertManUIViewTrustedSite::DoDeactivate" );
    }

// ---------------------------------------------------------
// CCertManUIViewTrustedSite::Container()
// Returns iContainerTrustedSite to be used in CertManUiContainerTrustedSite
// ---------------------------------------------------------
//
CCoeControl* CCertManUIViewTrustedSite::Container()
    {
    return iContainerTrustedSite;
    }

// ---------------------------------------------------------
// CCertManUIViewTrustedSite::UpdateMenuBar()
// Updates the menu bar
// ---------------------------------------------------------
//
void CCertManUIViewTrustedSite::UpdateMenuBar()
    {
    CEikButtonGroupContainer*  cba = Cba();

    if ( cba == NULL )
        {
        return;
        }

    if ( iKeeper.iPeerLabelEntries.Count() == 0 )
        {
        // No certificate. Don't show MSK
        UpdateCba( R_CERTMANUI_OPTIONS_OPEN_BACK );
        cba->MakeCommandVisible( ECertManUICmdAppViewDtls, EFalse );
        }
    else
        {
        if ( iContainerTrustedSite->iListBox->SelectionIndexes()->Count() > 0 )
            {
            MenuBar()->SetContextMenuTitleResourceId( R_CERTMANUI_MENUBAR_OPTIONS_MARKED );
            UpdateCba( R_CERTMANUI_OPTIONS_CONTEXT_BACK );
            }
        else
            {
            MenuBar()->SetMenuTitleResourceId( R_CERTMANUI_MENUBAR_VIEW_TRUSTED_SITES );
            UpdateCba( R_CERTMANUI_OPTIONS_OPEN_BACK );
            }
        cba->MakeCommandVisible( ECertManUICmdAppViewDtls, ETrue );
        }
    }

// ---------------------------------------------------------
// CCertManUIViewTrustedSite::UpdateCba( TInt aCbaResourceId )
// Updates cba
// ---------------------------------------------------------
//
void CCertManUIViewTrustedSite::UpdateCba( TInt aCbaResourceId )
    {
    CEikButtonGroupContainer*  cba = Cba();

    if ( cba )
        {
        cba->SetCommandSetL( aCbaResourceId );
        cba->DrawNow();
        }
    }


// End of File



