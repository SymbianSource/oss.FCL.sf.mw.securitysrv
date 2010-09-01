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
* Description:   Implementation of class CCertManUIViewAuthority
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
#include <X509CertNameParser.h>
#include <hash.h>                       // SHA-1 fingerprint
#include <sysutil.h>
#include <ErrorUI.h>
#include <x509cert.h>
#include <pkixcertchain.h>              //for validation
#include <certmanui.rsg>
#include "CertmanuiCertificateHelper.h"
#include "CertmanuiKeeper.h"
#include "CertmanuiviewAuthority.h"
#include "CertmanuicontainerAuthority.h"
#include "Certmanui.hrh"
#include "Certmanuitrustids.h"
#include "CertmanuiSyncWrapper.h"
#include "CertmanuiCommon.h"
#include "CertManUILogger.h"
#include    <hlplch.h>                   // For HlpLauncher
#include    <featmgr.h>                  // For HlpLauncher

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CCertManUIViewAuthority::CCertManUIViewAuthority(CCertManUIKeeper& aKeeper)
// : iKeeper(aKeeper), iCrMgr(iKeeper.CertManager())
// Constructor
// ---------------------------------------------------------
//
CCertManUIViewAuthority::CCertManUIViewAuthority( CCertManUIKeeper& aKeeper )
    : iKeeper( aKeeper )
    {
    CERTMANUILOGGER_WRITE_TIMESTAMP(
        "CCertManUIViewAuthority::CCertManUIViewAuthority" );
    }

// ---------------------------------------------------------
// CCertManUIViewAuthority::~CCertManUIViewAuthority()
// Destructor
// ---------------------------------------------------------
//
CCertManUIViewAuthority::~CCertManUIViewAuthority()
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIViewAuthority::~CCertManUIViewAuthority" );

    if ( iContainerAuthority )
        {
        AppUi()->RemoveFromViewStack( *this, iContainerAuthority );
        delete iContainerAuthority;
        }

    CERTMANUILOGGER_LEAVEFN( " CCertManUIViewAuthority::~CCertManUIViewAuthority" );
    }

// ---------------------------------------------------------
// CCertManUIViewAuthority* CCertManUIViewAuthority::NewL(
//      const TRect& /*aRect*/, CCertManUIKeeper& aKeeper)
// ---------------------------------------------------------
//
CCertManUIViewAuthority* CCertManUIViewAuthority::NewL(
    const TRect& /*aRect*/, CCertManUIKeeper& aKeeper )
    {
    CCertManUIViewAuthority* self =
        new ( ELeave ) CCertManUIViewAuthority( aKeeper );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// ---------------------------------------------------------
// CCertManUIViewAuthority* CCertManUIViewAuthority::NewLC(
//  const TRect& /*aRect*/, CCertManUIKeeper& aKeeper)
// ---------------------------------------------------------
//
CCertManUIViewAuthority* CCertManUIViewAuthority::NewLC(
    const TRect& /*aRect*/, CCertManUIKeeper& aKeeper )
    {
    CCertManUIViewAuthority* self =
        new ( ELeave ) CCertManUIViewAuthority( aKeeper );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------
// CCertManUIViewAuthority::ConstructL()
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CCertManUIViewAuthority::ConstructL()
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIViewAuthority::ConstructL" );

    BaseConstructL( R_CERTMANUI_VIEW_MAIN_AUTHORITY );

    CERTMANUILOGGER_LEAVEFN( " CCertManUIViewAuthority::ConstructL" );
    }

// ---------------------------------------------------------
// CCertManUIViewAuthority::DynInitMenuPaneL(
//      TInt aResourceId,CEikMenuPane* aMenuPane)
// Updates Options list with correct items depending on
// whether the listbox is empty or if it has any marked items
// ---------------------------------------------------------
//
void CCertManUIViewAuthority::DynInitMenuPaneL(
    TInt aResourceId, CEikMenuPane* aMenuPane )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIViewAuthority::DynInitMenuPaneL" );

    if (( iKeeper.iWrapper->IsActive()) ||
        ( iContainerAuthority->iListBox->CurrentItemIndex() >= iKeeper.iCALabelEntries.Count()) )
        {
        return;
        }

    const CListBoxView::CSelectionIndexArray* selections =
            iContainerAuthority->iListBox->SelectionIndexes();
    const TInt markedCount = selections->Count();
    const TInt currentItemIndex = iContainerAuthority->iListBox->CurrentItemIndex();

    switch ( aResourceId )
        {
        case R_CERTMANUI_VIEW_MENU_AUTHORITY:
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
                aMenuPane->SetItemDimmed( ECertManUICmdAppTrustSet, ETrue );
                aMenuPane->SetItemDimmed( ECertManUICmdMarkUnmark, ETrue );
                }
            else
                {
                if ( markedCount == 1 )
                    {
                    aMenuPane->SetItemDimmed( ECertManUICmdAppViewDtls, ETrue );
                    }

                if ( markedCount > 0 )
                    {
                    aMenuPane->SetItemDimmed( ECertManUICmdAppViewDtls, ETrue );
                    aMenuPane->SetItemDimmed( ECertManUICmdAppTrustSet, ETrue );
                    }

                if ( !iKeeper.iCertificateHelper->IsOneMarkedCertificateDeletable(
                        iContainerAuthority->iListBox, KCertTypeAuthority ) )
                    {
                    aMenuPane->SetItemDimmed( ECertManUICmdAppDelete, ETrue );
                    }
                }
            break;
            }

        case R_CERTMANUI_MENUPANE_OPTIONS_MARKED:
            {
            if ( !iKeeper.iCertificateHelper->IsOneMarkedCertificateDeletable(
                    iContainerAuthority->iListBox, KCertTypeAuthority ) )
                {
                aMenuPane->SetItemDimmed( ECertManUICmdAppDelete, ETrue );
                }

            // not available for authority certificates
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
            if ( markedCount == iContainerAuthority->iListBox->Model()->NumberOfItems() )
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

    CERTMANUILOGGER_LEAVEFN( " CCertManUIViewAuthority::DynInitMenuPaneL" );
    }

// ---------------------------------------------------------
// CCertManUIViewAuthority::Id() const
// Returns Authority view id
// ---------------------------------------------------------
//
TUid CCertManUIViewAuthority::Id() const
    {
    return KCertManUIViewAuthorityId;
    }

// ---------------------------------------------------------
// CCertManUIViewAuthority::HandleCommandL(TInt aCommand)
// Handles Softkey and Options list commands
// ---------------------------------------------------------
//
void CCertManUIViewAuthority::HandleCommandL( TInt aCommand )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIViewAuthority::HandleCommandL" );

    if (( iKeeper.iWrapper->IsActive() ) ||
        ( iContainerAuthority->iListBox->CurrentItemIndex() >= iKeeper.iCALabelEntries.Count() ))
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
                iContainerAuthority->iListBox->CurrentItemIndex(),
                KCertTypeAuthority, iEikonEnv );
            break;
            }
        case ECertManUICmdAppDelete:
            {
            if ( iKeeper.iCertificateHelper->ConfirmationQueryDeleteCertL( KCertTypeAuthority,
                                                    iContainerAuthority->iListBox ) )
                {
                // goes here if at least one certificate is deleted
                iCurrentPosition = iContainerAuthority->iListBox->CurrentItemIndex();
                iTopItem = iContainerAuthority->iListBox->TopItemIndex();

                iContainerAuthority->DrawListBoxL(iCurrentPosition, iTopItem );
                TRAPD( error, iKeeper.RefreshCAEntriesL() );
                if ( error != KErrNone )
                    {
                    if ( error == KErrCorrupt )
                        {
                        iKeeper.ShowErrorNoteL( error);
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
            CEikListBox* listbox = iContainerAuthority->iListBox;
            if ( listbox )
                {
                AknSelectionService::HandleMarkableListProcessCommandL(
                    aCommand, listbox );

                iKeeper.iCertificateHelper->HandleMarkableListCommandL(
                                      aCommand, iContainerAuthority->iListBox );

                UpdateMenuBar();
                }
            break;
            }
        case ECertManUICmdAppTrustSet:
            {
            TInt helpIndex3 = iContainerAuthority->iListBox->CurrentItemIndex();
            CEikStatusPane* sp = STATIC_CAST(
                CAknAppUi*, iEikonEnv->EikAppUi())->StatusPane();
            // Fetch pointer to the default title pane control
            CAknTitlePane* title = STATIC_CAST(
                CAknTitlePane*,
                sp->ControlL( TUid::Uid( EEikStatusPaneUidTitle ) ));

            // Creating the correct title text for Trust Settings view
            HBufC* buf = HBufC::NewLC( KMaxLengthTextCertLabel );

            // Get Cert entries
            buf->Des() =
                (iKeeper.iCALabelEntries)[helpIndex3]->iCAEntryLabel->Des();
            buf->Des().TrimLeft();
            // Cut CertLabel after fourth semi colon
            TPtrC trimmedCertLabel = iKeeper.iCertificateHelper->CutCertificateField( buf->Des() );
            buf->Des().Copy( trimmedCertLabel );
            TInt length = buf->Des().Length();
            if (length == 0)
                {
                HBufC* stringHolder = NULL;

                stringHolder = StringLoader::LoadLC(
                       R_TEXT_RESOURCE_VIEW_NO_SUBJECT_TITLE );

                title->SetTextL(stringHolder->Des());
                CleanupStack::PopAndDestroy();  // stringHolder
                }
            else
                {
                title->SetTextL( buf->Des() );
                }
            CleanupStack::PopAndDestroy();  // buf

            iKeeper.iCurrentCACertForTrustSettings = helpIndex3;
            ((CAknViewAppUi*)iAvkonAppUi)->ActivateLocalViewL(
                KCertManUIViewTrustId, TUid::Uid( helpIndex3), KNullDesC8 );
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

    CERTMANUILOGGER_LEAVEFN( " CCertManUIViewAuthority::HandleCommandL" );
    }

// ---------------------------------------------------------
// CCertManUIViewAuthority::HandleClientRectChange()
// ---------------------------------------------------------
//
void CCertManUIViewAuthority::HandleClientRectChange()
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIViewAuthority::HandleClientRectChange" );

    if ( iContainerAuthority )
        {
        iContainerAuthority->SetRect( ClientRect() );
        }

    CERTMANUILOGGER_LEAVEFN( " CCertManUIViewAuthority::HandleClientRectChange" );
    }

// ---------------------------------------------------------
// CCertManUIViewAuthority::DoActivateL(const TVwsViewId& /*aPrevViewId*/,
//      TUid /*aCustomMessageId*/, const TDesC8& /*aCustomMessage*/)
// Updates the view when opening it
// ---------------------------------------------------------
//
void CCertManUIViewAuthority::DoActivateL(
    const TVwsViewId& aPrevViewId,
    TUid /*aCustomMessageId*/, const TDesC8& /*aCustomMessage*/)
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIViewAuthority::DoActivateL" );

    iKeeper.StartWaitDialogL( ECertmanUiWaitDialog );
    CleanupCloseWaitDialogPushL( iKeeper );

    TRAPD ( error, iKeeper.RefreshCAEntriesL() );
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

    if ( iContainerAuthority )
        {
        ((CAknViewAppUi*)iAvkonAppUi)->RemoveFromViewStack(
            *this, iContainerAuthority );

        delete iContainerAuthority;
        iContainerAuthority = NULL;
        }
    iContainerAuthority = new (ELeave) CCertManUIContainerAuthority(
        *this, iKeeper );

    iContainerAuthority->SetMopParent( this );
    iContainerAuthority->ConstructL(
        ClientRect(), iCurrentPosition, iTopItem );

    if ( !iKeeper.iCertsDatOK )
        {
        iCurrentPosition = 0;
        iTopItem = 0;
        }

    iKeeper.ActivateTitleL( KViewTitleCertManUIAuthority );

    iContainerAuthority->DrawListBoxL( iCurrentPosition, iTopItem );
    ((CAknViewAppUi*)iAvkonAppUi)->AddToStackL( *this, iContainerAuthority );

    UpdateMenuBar();

    CleanupStack::PopAndDestroy();    // closes wait dialog

    iKeeper.iStartup = EFalse;

    CERTMANUILOGGER_LEAVEFN( " CCertManUIViewAuthority::DoActivateL" );
    }

// ---------------------------------------------------------
// CCertManUIViewAuthority::DoDeactivate()
// Saves focus position when closing view
// ---------------------------------------------------------
//
void CCertManUIViewAuthority::DoDeactivate()
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIViewAuthority::DoDeactivate" );

    if ( iKeeper.iCertsDatOK )
        {
        iCurrentPosition = iContainerAuthority->iListBox->CurrentItemIndex();
        iTopItem = iContainerAuthority->iListBox->TopItemIndex();
        // do not destroy iContainerAuthority here because
        // TrustSettings view needs it !
        }
    else
        {
        iCurrentPosition = 0;
        iTopItem = 0;
        }

    CERTMANUILOGGER_LEAVEFN( " CCertManUIViewAuthority::DoDeactivate" ); //lint !e539
    }

// ---------------------------------------------------------
// CCertManUIViewAuthority::Container()
// Returns iContainerAuthority to be used in CertManUIContainerAuthority
// ---------------------------------------------------------
//
CCoeControl* CCertManUIViewAuthority::Container()
    {
    return iContainerAuthority;
    }

// ---------------------------------------------------------
// CCertManUIViewAuthority::UpdateMenuBar()
// Updates the menu bar
// ---------------------------------------------------------
//
void CCertManUIViewAuthority::UpdateMenuBar()
    {
    CEikButtonGroupContainer*  cba = Cba();

    if ( cba == NULL )
        {
        return;
        }

    if ( iKeeper.iCALabelEntries.Count() == 0 )
        {
        // No certificate. Don't show MSK
        UpdateCba( R_CERTMANUI_OPTIONS_OPEN_BACK );
        cba->MakeCommandVisible( ECertManUICmdAppViewDtls, EFalse );
        }
    else
        {
        if ( iContainerAuthority->iListBox->SelectionIndexes()->Count() > 0 )
            {
            MenuBar()->SetContextMenuTitleResourceId( R_CERTMANUI_MENUBAR_OPTIONS_MARKED );
            UpdateCba( R_CERTMANUI_OPTIONS_CONTEXT_BACK );
            }
        else
            {
            MenuBar()->SetMenuTitleResourceId( R_CERTMANUI_MENUBAR_VIEW_AUTHORITY );
            UpdateCba( R_CERTMANUI_OPTIONS_OPEN_BACK );
            }
         }
    }

// ---------------------------------------------------------
// CCertManUIViewAuthority::UpdateCba( TInt aCbaResourceId )
// Updates cba
// ---------------------------------------------------------
//
void CCertManUIViewAuthority::UpdateCba( TInt aCbaResourceId )
    {
    CEikButtonGroupContainer*  cba = Cba();

    if ( cba )
        {
        cba->SetCommandSetL( aCbaResourceId );
        cba->DrawNow();
        }
    }


// End of File



