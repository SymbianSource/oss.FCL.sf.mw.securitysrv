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
* Description:   Implementation of class CCertManUIViewDevice
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
#include <ErrorUI.h>
#include <x509cert.h>
#include <pkixcertchain.h>              //for validation
#include <certmanui.rsg>
#include "CertmanuiCertificateHelper.h"
#include "CertmanuiKeeper.h"
#include "CertmanuiviewDevice.h"
#include "CertmanuicontainerDevice.h"
#include "Certmanui.hrh"
#include "Certmanuitrustids.h"
#include "CertmanuiSyncWrapper.h"
#include "CertmanuiCommon.h"
#include "CertManUILogger.h"

#include    <hlplch.h>                  // For HlpLauncher
#include    <featmgr.h>                 // For FeatureManager

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CCertManUIViewDevice::CCertManUIViewDevice(CCertManUIKeeper& aKeeper)
// : iKeeper(aKeeper), iCrMgr(iKeeper.CertManager())
// Constructor
// ---------------------------------------------------------
//
CCertManUIViewDevice::CCertManUIViewDevice( CCertManUIKeeper& aKeeper )
    : iKeeper( aKeeper )
    {
    CERTMANUILOGGER_WRITE_TIMESTAMP(
        "CCertManUIViewDevice::CCertManUIViewDevice" );
    }

// ---------------------------------------------------------
// CCertManUIViewDevice::~CCertManUIViewDevice()
// Destructor
// ---------------------------------------------------------
//
CCertManUIViewDevice::~CCertManUIViewDevice()
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIViewDevice::~CCertManUIViewDevice" );

    if ( iContainerDevice )
        {
        AppUi()->RemoveFromViewStack( *this, iContainerDevice );
        delete iContainerDevice;
        }

    CERTMANUILOGGER_LEAVEFN( " CCertManUIViewDevice::~CCertManUIViewDevice" );
    }

// ---------------------------------------------------------
// CCertManUIViewDevice* CCertManUIViewDevice::NewL(
//      const TRect& /*aRect*/, CCertManUIKeeper& aKeeper)
// ---------------------------------------------------------
//
CCertManUIViewDevice* CCertManUIViewDevice::NewL(
    const TRect& /*aRect*/, CCertManUIKeeper& aKeeper )
    {
    CCertManUIViewDevice* self =
        new ( ELeave ) CCertManUIViewDevice( aKeeper );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// ---------------------------------------------------------
// CCertManUIViewDevice* CCertManUIViewDevice::NewLC(
//  const TRect& /*aRect*/, CCertManUIKeeper& aKeeper)
// ---------------------------------------------------------
//
CCertManUIViewDevice* CCertManUIViewDevice::NewLC(
    const TRect& /*aRect*/, CCertManUIKeeper& aKeeper )
    {
    CCertManUIViewDevice* self =
        new ( ELeave ) CCertManUIViewDevice( aKeeper );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------
// CCertManUIViewDevice::ConstructL()
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CCertManUIViewDevice::ConstructL()
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIViewDevice::ConstructL" );

    BaseConstructL( R_CERTMANUI_VIEW_MAIN_DEVICE );

    CERTMANUILOGGER_LEAVEFN( " CCertManUIViewDevice::ConstructL" );
    }

// ---------------------------------------------------------
// CCertManUIViewDevice::DynInitMenuPaneL(
//      TInt aResourceId,CEikMenuPane* aMenuPane)
// Updates Options list with correct items depending on
// whether the listbox is empty or if it has any marked items
// ---------------------------------------------------------
//
void CCertManUIViewDevice::DynInitMenuPaneL(
    TInt aResourceId, CEikMenuPane* aMenuPane )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIViewDevice::DynInitMenuPaneL" );

    if (( iKeeper.iWrapper->IsActive()) ||
        ( iContainerDevice->iListBox->CurrentItemIndex() >= iKeeper.iDeviceLabelEntries.Count()))
        {
        return;
        }

    const CListBoxView::CSelectionIndexArray* selections =
            iContainerDevice->iListBox->SelectionIndexes();
    const TInt markedCount = selections->Count();
    const TInt currentItemIndex = iContainerDevice->iListBox->CurrentItemIndex();

    switch ( aResourceId )
        {
        case R_CERTMANUI_VIEW_MENU_DEVICE:
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
                aMenuPane->SetItemDimmed( ECertManUICmdMoveToPersonal, ETrue );
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
                    aMenuPane->SetItemDimmed( ECertManUICmdMoveToPersonal, ETrue );
                    }

                if ( !iKeeper.iCertificateHelper->IsOneMarkedCertificateDeletable(
                        iContainerDevice->iListBox, KCertTypeDevice ) )
                    {
                    aMenuPane->SetItemDimmed( ECertManUICmdAppDelete, ETrue );
                    aMenuPane->SetItemDimmed( ECertManUICmdMoveToPersonal, ETrue );
                    }
                }
            break;
            }

        case R_CERTMANUI_MENUPANE_OPTIONS_MARKED:
            {
            if ( markedCount > 1 )
                {
                aMenuPane->SetItemDimmed( ECertManUICmdMoveToPersonal, ETrue );
                }

            if ( !iKeeper.iCertificateHelper->IsOneMarkedCertificateDeletable(
                    iContainerDevice->iListBox, KCertTypeDevice ) )
                {
                aMenuPane->SetItemDimmed( ECertManUICmdAppDelete, ETrue );
                }

            // not available for device certificates
            aMenuPane->SetItemDimmed( ECertManUICmdMoveToDevice, ETrue );
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
            if ( markedCount == iContainerDevice->iListBox->Model()->NumberOfItems() )
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

    CERTMANUILOGGER_LEAVEFN( " CCertManUIViewDevice::DynInitMenuPaneL" );
    }

// ---------------------------------------------------------
// CCertManUIViewDevice::Id() const
// Returns Authority view id
// ---------------------------------------------------------
//
TUid CCertManUIViewDevice::Id() const
    {
    return KCertManUIViewDeviceId;
    }

// ---------------------------------------------------------
// CCertManUIViewDevice::HandleCommandL(TInt aCommand)
// Handles Softkey and Options list commands
// ---------------------------------------------------------
//
void CCertManUIViewDevice::HandleCommandL( TInt aCommand )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIViewDevice::HandleCommandL" );

    if (( iKeeper.iWrapper->IsActive() ) ||
        ( iContainerDevice->iListBox->CurrentItemIndex() >= iKeeper.iDeviceLabelEntries.Count() ))
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
                iContainerDevice->iListBox->CurrentItemIndex(),
                KCertTypeDevice, iEikonEnv );
            break;
            }
        case ECertManUICmdAppDelete:
            {
            if ( iKeeper.iCertificateHelper->ConfirmationQueryDeleteCertL( KCertTypeDevice,
                                                    iContainerDevice->iListBox) )
                {
                // goes here if at least one certificate is deleted
                iCurrentPosition = iContainerDevice->iListBox->CurrentItemIndex();
                iTopItem = iContainerDevice->iListBox->TopItemIndex();

                iContainerDevice->DrawListBoxL(iCurrentPosition, iTopItem );
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
                UpdateMenuBar();
                }
            break;
            }
       case ECertManUICmdMoveToPersonal:
            {
            HBufC* prompt = StringLoader::LoadLC(
                        R_QTN_CM_CONF_MOVE_TO_PERSONAL );
            TInt selection = iKeeper.iCertificateHelper->DisplayConfirmationLD(prompt);
            if ( selection )
                {
                TInt focusPosition = 0;
                
                const CArrayFix<TInt>* array = iContainerDevice->iListBox->SelectionIndexes();
                
                if ( array->Count() == 1 )
                    {
                    // item is marked
                    focusPosition = array->At( 0 );
                    }
                else
                    {
                    // There is no marked item. Select higlighted item 
                    focusPosition = iContainerDevice->iListBox->CurrentItemIndex();
                    }
                                                        
                CCTCertInfo* entry = iKeeper.iDeviceLabelEntries[ focusPosition ]->iDeviceEntry;

                // Move key first
                TCTKeyAttributeFilter keyFilter;
                keyFilter.iKeyId = entry->SubjectKeyId();
                keyFilter.iPolicyFilter =  TCTKeyAttributeFilter::EAllKeys;

                TRAPD( error, iKeeper.iCertificateHelper->MoveKeyL( keyFilter,
                                KCMDeviceKeyStoreTokenUid, KCMFileKeyStoreTokenUid ));

                if ( error == KErrNone )
                    {
                    iKeeper.iCertificateHelper->MoveCertL(
                                                      *entry,
                                                      focusPosition,
                                                      iContainerDevice->iListBox,
                                                      KCMDeviceCertStoreTokenUid,
                                                      KCMFileCertStoreTokenUid );

                    TRAP( error, iKeeper.RefreshDeviceCertEntriesL() );
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

                    MDesCArray* itemList = iContainerDevice->iListBox->Model()->ItemTextArray();
                    CDesCArray* itemArray = ( CDesCArray* )itemList;
                    if (( itemArray != NULL ) && ( itemArray->Count() == 0 ))
                        {
                        iCurrentPosition = 0;
                        iTopItem = 0;
                        iContainerDevice->UpdateListBoxL( iCurrentPosition, iTopItem );
                        }
                     else
                        {
                        iCurrentPosition = iContainerDevice->iListBox->CurrentItemIndex();
                        iTopItem = iContainerDevice->iListBox->TopItemIndex();
                        }

                    iContainerDevice->DrawListBoxL(iCurrentPosition, iTopItem );

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
            CEikListBox* listbox = iContainerDevice->iListBox;
            if ( listbox )
                {
                AknSelectionService::HandleMarkableListProcessCommandL(
                    aCommand, listbox );

                iKeeper.iCertificateHelper->
                                    HandleMarkableListCommandL(
                                                              aCommand,
                                                              iContainerDevice->iListBox
                                                              );

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

    CERTMANUILOGGER_LEAVEFN( " CCertManUIViewDevice::HandleCommandL" );
    }

// ---------------------------------------------------------
// CCertManUIViewDevice::HandleClientRectChange()
// ---------------------------------------------------------
//
void CCertManUIViewDevice::HandleClientRectChange()
    {
    if ( iContainerDevice )
        {
        iContainerDevice->SetRect( ClientRect() );
        }
    }

// ---------------------------------------------------------
// CCertManUIViewDevice::DoActivateL(const TVwsViewId& /*aPrevViewId*/,
//      TUid /*aCustomMessageId*/, const TDesC8& /*aCustomMessage*/)
// Updates the view when opening it
// ---------------------------------------------------------
//
void CCertManUIViewDevice::DoActivateL(
    const TVwsViewId& aPrevViewId,
    TUid /*aCustomMessageId*/, const TDesC8& /*aCustomMessage*/)
    {
     CERTMANUILOGGER_ENTERFN( " CCertManUIViewDevice::DoActivateL" );

     iKeeper.StartWaitDialogL( ECertmanUiWaitDialog );
     CleanupCloseWaitDialogPushL( iKeeper );

     TRAPD ( error, iKeeper.RefreshDeviceCertEntriesL() );
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

    if ( iContainerDevice )
        {
        ((CAknViewAppUi*)iAvkonAppUi)->RemoveFromViewStack(
            *this, iContainerDevice );

        delete iContainerDevice;
        iContainerDevice = NULL;
        }

    iContainerDevice = new (ELeave) CCertManUIContainerDevice(
        *this, iKeeper );

    iContainerDevice->SetMopParent( this );
    iContainerDevice->ConstructL(
        ClientRect(), iCurrentPosition, iTopItem );

    if ( !iKeeper.iCertsDatOK )
        {
        iCurrentPosition = 0;
        iTopItem = 0;
        }

    iKeeper.ActivateTitleL( KViewTitleCertManUIDevice );

    iContainerDevice->DrawListBoxL( iCurrentPosition, iTopItem );
    ((CAknViewAppUi*)iAvkonAppUi)->AddToStackL( *this, iContainerDevice );

    UpdateMenuBar();

    CleanupStack::PopAndDestroy();    // closes wait dialog

    iKeeper.iStartup = EFalse;

     CERTMANUILOGGER_LEAVEFN( " CCertManUIViewDevice::DoActivateL" );
    }

// ---------------------------------------------------------
// CCertManUIViewDevice::DoDeactivate()
// Saves focus position when closing view
// ---------------------------------------------------------
//
void CCertManUIViewDevice::DoDeactivate()
    {
     CERTMANUILOGGER_ENTERFN( " CCertManUIViewDevice::DoDeactivate" );

    if ( iKeeper.iCertsDatOK )
        {
        iCurrentPosition = iContainerDevice->iListBox->CurrentItemIndex();
        iTopItem = iContainerDevice->iListBox->TopItemIndex();
        }
    else
        {
        iCurrentPosition = 0;
        iTopItem = 0;
        }

     CERTMANUILOGGER_LEAVEFN( " CCertManUIViewDevice::DoDeactivate" ); //lint !e539
    }

// ---------------------------------------------------------
// CCertManUIViewDevice::Container()
// Returns iContainerDevice to be used in CertManUiContainerDevice
// ---------------------------------------------------------
//
CCoeControl* CCertManUIViewDevice::Container()
    {
    return iContainerDevice;
    }

// ---------------------------------------------------------
// CCertManUIViewDevice::UpdateMenuBar()
// Updates the menu bar
// ---------------------------------------------------------
//
void CCertManUIViewDevice::UpdateMenuBar()
    {
    CEikButtonGroupContainer*  cba = Cba();

    if ( cba == NULL )
        {
        return;
        }

    if ( iKeeper.iDeviceLabelEntries.Count() == 0 )
        {
        // No certificate. Don't show MSK
        UpdateCba( R_CERTMANUI_OPTIONS_OPEN_BACK );
        cba->MakeCommandVisible( ECertManUICmdAppViewDtls, EFalse );
        }
    else
        {
        if ( iContainerDevice->iListBox->SelectionIndexes()->Count() > 0 )
            {
            MenuBar()->SetContextMenuTitleResourceId( R_CERTMANUI_MENUBAR_OPTIONS_MARKED );
            UpdateCba( R_CERTMANUI_OPTIONS_CONTEXT_BACK );
            }
        else
            {
            MenuBar()->SetMenuTitleResourceId( R_CERTMANUI_MENUBAR_VIEW_DEVICE );
            UpdateCba( R_CERTMANUI_OPTIONS_OPEN_BACK );
            }
         cba->MakeCommandVisible( ECertManUICmdAppViewDtls, ETrue );
         }
    }

// ---------------------------------------------------------
// CCertManUIViewDevice::UpdateCba( TInt aCbaResourceId )
// Updates cba
// ---------------------------------------------------------
//
void CCertManUIViewDevice::UpdateCba( TInt aCbaResourceId )
    {
    CEikButtonGroupContainer*  cba = Cba();

    if ( cba )
        {
        cba->SetCommandSetL( aCbaResourceId );
        cba->DrawNow();
        }
    }

// End of File



