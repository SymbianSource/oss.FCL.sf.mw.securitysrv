/*
* Copyright (c) 2003-2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Implementation of class CCertManUIViewTrust
*                Updates Options list depending on the status of the
*                trusted clients (unavailable, located in WIM).
*                Handles Softkey and Options list commands.
*
*/


// INCLUDE FILES
#include <certificateapps.h>
#include <certmanui.rsg>
#include "CertmanuiKeeper.h"
#include "CertmanuiviewTrust.h"
#include "CertmanuicontainerTrust.h"
#include "CertmanuicontainerAuthority.h"
#include "CertmanuiSyncWrapper.h"
#include "Certmanuitrustids.h"
#include "Certmanui.hrh"
#include "CertmanuiCommon.h"
#include "CertManUILogger.h"
#include "securityuisvariant.hrh"
#include "CertmanuiCertificateHelper.h"

#include    <hlplch.h>            // For HlpLauncher
#include    <featmgr.h>           // For FeatureManager



// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CCertManUIViewTrust::CCertManUIViewTrust(CCertManUIKeeper& aKeeper)
// Constructor
// ---------------------------------------------------------
//
CCertManUIViewTrust::CCertManUIViewTrust( CCertManUIKeeper& aKeeper )
    : iKeeper( aKeeper )
    {
    CERTMANUILOGGER_WRITE_TIMESTAMP(
        "CCertManUIViewTrust::CCertManUIViewTrust" );
    }

// ---------------------------------------------------------
// CCertManUIViewTrust::~CCertManUIViewTrust()
// Destructor
// ---------------------------------------------------------
//
CCertManUIViewTrust::~CCertManUIViewTrust()
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIViewTrust::~CCertManUIViewTrust" );

    if ( iContainerTrust )
        {
        AppUi()->RemoveFromViewStack( *this, iContainerTrust );
        delete iContainerTrust;
        }

    CERTMANUILOGGER_LEAVEFN( " CCertManUIViewTrust::~CCertManUIViewTrust" );
    }

// ---------------------------------------------------------
// CCertManUIViewTrust* CCertManUIViewTrust::NewL(const TRect& /*aRect*/,
//      CCertManUIKeeper& aKeeper)
// ---------------------------------------------------------
//
CCertManUIViewTrust* CCertManUIViewTrust::NewL(
    const TRect& /*aRect*/, CCertManUIKeeper& aKeeper )
    {
    CCertManUIViewTrust* self = new (ELeave) CCertManUIViewTrust( aKeeper );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// ---------------------------------------------------------
// CCertManUIViewTrust* CCertManUIViewTrust::NewLC(const TRect& /*aRect*/,
//      CCertManUIKeeper& aKeeper)
// ---------------------------------------------------------
//
CCertManUIViewTrust* CCertManUIViewTrust::NewLC(const TRect& /*aRect*/,
    CCertManUIKeeper& aKeeper)
    {
    CCertManUIViewTrust* self = new (ELeave) CCertManUIViewTrust(aKeeper);
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------
// CCertManUIViewTrust::ConstructL()
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CCertManUIViewTrust::ConstructL()
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIViewTrust::ConstructL" );

    BaseConstructL( R_CERTMANUI_VIEW_TRUST );

    CERTMANUILOGGER_LEAVEFN( " CCertManUIViewTrust::ConstructL" );
  }

// ---------------------------------------------------------
// CCertManUIViewTrust::DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane)
// Updates Options list with correct items depending on
// whether the listbox is empty or if the trust can be changed
// Change trust is not shown if certificate location is not supported
// unsupported certificate locations are ECrCertLocationWIMCard,
//                    ECrCertLocationWIMURL,
//                    ECrCertLocationPhoneMemory,
//                    ECrCertLocationPhoneMemoryURL
// Change trust is not shown if certificate format is not supported
// unsupported certificate formats are ECrX968Certificate
//                    ECrCertificateURL
// ---------------------------------------------------------
//
void CCertManUIViewTrust::DynInitMenuPaneL(
    TInt aResourceId, CEikMenuPane* aMenuPane )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIViewTrust::DynInitMenuPaneL" );

    if ( aResourceId == R_CERTMANUI_VIEW_TRUST_MENU )
        {
        if( !FeatureManager::FeatureSupported( KFeatureIdHelp ) )
             {
             aMenuPane->DeleteMenuItem( EAknCmdHelp );
             }

        TInt currentItemTrust = iContainerTrust->iListBox->CurrentItemIndex();
        if ( currentItemTrust == -1 )
            {
            aMenuPane->SetItemDimmed( ECertManUICmdAppChangeTrust, ETrue );
            }

        CCertManUIViewAuthority* authorityView = STATIC_CAST(
            CCertManUIViewAuthority*, AppUi()->View( KCertManUIViewAuthorityId ) );
        TInt currentItem = authorityView->
            iContainerAuthority->iListBox->CurrentItemIndex();
        CCTCertInfo& entry = *( iKeeper.iCALabelEntries[ currentItem ]->iCAEntry );

        if ( !iKeeper.iCertificateHelper->IsOneMarkedCertificateDeletable(
                authorityView->iContainerAuthority->iListBox, KCertTypeAuthority ) )
                {
                LOG_WRITE( "Read only Certificate" );
                aMenuPane->SetItemDimmed( ECertManUICmdAppChangeTrust, ETrue );
                }

        if ( currentItem >= 0 )
            {
            if( KCertManUIViewTrustApplicationControllerId ==
                iContainerTrust->iClientUids[ currentItemTrust ] &&
                iKeeper.IsLocalFeatureSupported( KSecurityUIsNoNativeAppTrustSettingChange ) )
                {
                aMenuPane->SetItemDimmed( ECertManUICmdAppChangeTrust, ETrue );
                }
            }
        }

    CERTMANUILOGGER_LEAVEFN( " CCertManUIViewTrust::DynInitMenuPaneL" );
    }

// ---------------------------------------------------------
// CCertManUIViewTrust::Id() const
// Returns Trust view id
// ---------------------------------------------------------
//
TUid CCertManUIViewTrust::Id() const
    {
    return KCertManUIViewTrustId;
    }

// ---------------------------------------------------------
// CCertManUIViewTrust::HandleCommandL(TInt aCommand)
// Handles Softkey and Options list commands
// ---------------------------------------------------------
//
void CCertManUIViewTrust::HandleCommandL( TInt aCommand )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIViewTrust::HandleCommandL" );

    if ( iKeeper.iWrapper->IsActive() )
        {
        return;
        }

    switch ( aCommand )
        {
        case EAknSoftkeyBack:
            {
            AppUi()->ActivateLocalViewL( KCertManUIViewAuthorityId );
            break;
            }
        // put the view specific menu commands here
        case EAknCmdExit:
            {
            ((CAknViewAppUi*)iAvkonAppUi)->HandleCommandL( EAknCmdExit );
            break;
            }
        case ECertManUICmdAppChangeTrust:
            {
            PopupTrustChangeSettingPageL();
            break;
            }

        case EAknCmdHelp:
            {
            if( FeatureManager::FeatureSupported( KFeatureIdHelp ) )
                {
                HlpLauncher::LaunchHelpApplicationL( iEikonEnv->WsSession(),
                    AppUi()->AppHelpContextL() );
                }
            break;
            }

        default:
            {
            AppUi()->HandleCommandL( aCommand );
            break;
            }
        }

    CERTMANUILOGGER_LEAVEFN( " CCertManUIViewTrust::HandleCommandL" );
    }

// ---------------------------------------------------------
// CCertManUIViewTrust::HandleClientRectChange()
// ---------------------------------------------------------
//
void CCertManUIViewTrust::HandleClientRectChange()
    {
    if ( iContainerTrust )
        {
        iContainerTrust->SetRect( ClientRect() );
        }
    }

// ---------------------------------------------------------
// CCertManUIViewTrust::DoActivateL(
//      const TVwsViewId& /*aPrevViewId*/,
//      TUid aCustomMessageId, const TDesC8& /*aCustomMessage*/)
// Updates the view when opening it
// ---------------------------------------------------------
//
void CCertManUIViewTrust::DoActivateL( const TVwsViewId& /*aPrevViewId*/,
    TUid aCustomMessageId, const TDesC8& /*aCustomMessage*/ )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIViewTrust::DoActivateL" );

    if ( iContainerTrust )
        {
        ((CAknViewAppUi*)iAvkonAppUi)->RemoveFromViewStack(
            *this, iContainerTrust );
        delete iContainerTrust;
        iContainerTrust = NULL;
        }

    TInt trustViewItem = STATIC_CAST( TInt, aCustomMessageId.iUid );
    iContainerTrust = new(ELeave) CCertManUIContainerTrust( *this, iKeeper );
    // to fix problem of focus if trust view has more than one item
    TInt currentPosition = 0;
    TInt topItem = 0;
    iContainerTrust->SetMopParent( this );
    iContainerTrust->ConstructL( ClientRect());

    UpdateMenuBar();

    UpdateNaviPaneL();

    iContainerTrust->DrawListBoxL( currentPosition, topItem, trustViewItem );
    ((CAknViewAppUi*)iAvkonAppUi)->AddToStackL( *this, iContainerTrust );

    CERTMANUILOGGER_LEAVEFN( " CCertManUIViewTrust::DoActivateL" );
    }

// ---------------------------------------------------------
// CCertManUIViewTrust::DoDeactivate()
// ---------------------------------------------------------
//
void CCertManUIViewTrust::DoDeactivate()
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIViewTrust::DoDeactivate" );

    if ( iContainerTrust )
        {
        ((CAknViewAppUi*)iAvkonAppUi)->RemoveFromViewStack( *this, iContainerTrust );
        delete iContainerTrust;
        iContainerTrust = NULL;
        }

    RemoveNaviPane();

    CERTMANUILOGGER_LEAVEFN( " CCertManUIViewTrust::DoDeactivate" );
    }

// ---------------------------------------------------------
// CCertManUIViewTrust::Container()
// Returns iContainerTrust
// ---------------------------------------------------------
//
CCoeControl* CCertManUIViewTrust::Container()
    {
    return iContainerTrust;
    }

// ---------------------------------------------------------
// CCertManUIViewTrust::ChangeTrustL()
// Calls Change trust method UpdateTrustSettingsViewL in certmanuicontainertrust
// for not supported certificate formats ECrX968Certificate and ECrCertificateURL
// and for not supported certificate locations  ECrCertLocationWIMCard, ECrCertLocationWIMURL,
// ECrCertLocationPhoneMemory, ECrCertLocationPhoneMemoryURL UpdateTrustSettingsViewL is not called
// also UpdateTrustSettingsViewL is not called if there are no clients in Trust view.
// This function is called when user presses joystick key in trusters list.
// ---------------------------------------------------------
//
void CCertManUIViewTrust::ChangeTrustL()
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIViewTrust::ChangeTrustL" );
    CCertManUIViewAuthority* authorityView = STATIC_CAST(
            CCertManUIViewAuthority*, AppUi()->View(KCertManUIViewAuthorityId) );
    // Policy decision: Trusters of read-only certs cannot be changed
    if ( iKeeper.iCertificateHelper->IsOneMarkedCertificateDeletable(
                authorityView->iContainerAuthority->iListBox, KCertTypeAuthority ) )

        {
        TInt currentItem = authorityView->
            iContainerAuthority->iListBox->CurrentItemIndex();

        CCTCertInfo& entry = *( iKeeper.iCALabelEntries[ currentItem ]->iCAEntry );

        TInt currentTruster = iContainerTrust->iListBox->CurrentItemIndex();

        if ( currentItem >= 0 )
            {
            TUid uid = iContainerTrust->iClientUids[ currentTruster ];

            iContainerTrust->ChangeTrustValueL( entry, uid );
            iContainerTrust->UpdateTrustListboxItemL( entry, currentTruster );
            }
        }

    CERTMANUILOGGER_LEAVEFN( " CCertManUIViewTrust::ChangeTrustL" );
    }

// ---------------------------------------------------------
// CCertManUIViewTrust::PopupTrustChangeSettingPageL()
// ---------------------------------------------------------
//
void CCertManUIViewTrust::PopupTrustChangeSettingPageL()
    {
    CCertManUIViewAuthority* authorityView =
    STATIC_CAST( CCertManUIViewAuthority*,
            AppUi()->View( KCertManUIViewAuthorityId ) );
    TInt helpIndex3 = authorityView->
            iContainerAuthority->iListBox->CurrentItemIndex();
    iContainerTrust->PopupTrustChangeSettingPageL( helpIndex3 );
    }

// ---------------------------------------------------------
// CCertManUIViewTrust::UpdateMenuBar()
// Updates the menu bar
// ---------------------------------------------------------
//
void CCertManUIViewTrust::UpdateMenuBar()
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIViewTrust::UpdateMenuBar" );

    CCertManUIViewAuthority* authorityView = STATIC_CAST(
            CCertManUIViewAuthority*, AppUi()->View(KCertManUIViewAuthorityId) );

    CEikButtonGroupContainer*  cba = Cba();

    if (( cba == NULL ) || ( authorityView == NULL ))
        {
        return;
        }

    if ( !iKeeper.iCertificateHelper->IsOneMarkedCertificateDeletable(
                authorityView->iContainerAuthority->iListBox, ETrue ))
        {
        // Read-only certificate
        cba->MakeCommandVisible( ECertManUICmdAppChangeTrust, EFalse );
        }
    else
       {
       cba->MakeCommandVisible( ECertManUICmdAppChangeTrust, ETrue );
       }

    CERTMANUILOGGER_LEAVEFN( " CCertManUIViewTrust::UpdateMenuBar" );
    }

// ---------------------------------------------------------
// CCertManUIViewTrust::UpdateNaviPaneL()
// Updates navi pane
// ---------------------------------------------------------
//
void CCertManUIViewTrust::UpdateNaviPaneL()
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIViewTrust::UpdateNaviPaneL" );

    CEikStatusPane* sp = STATIC_CAST(
        CAknAppUi*, iEikonEnv->EikAppUi())->StatusPane();

    iNaviPane = STATIC_CAST(CAknNavigationControlContainer*,
        sp->ControlL(TUid::Uid(EEikStatusPaneUidNavi)));

    TResourceReader reader;
    iCoeEnv->CreateResourceReaderLC( reader, R_CM_NAVI_PANE_TRUST_SETTINGS );
    iTrustSettingsNaviPane = iNaviPane->CreateMessageLabelL( reader );
    CleanupStack::PopAndDestroy();  // reader

    iNaviPane->PushL( *iTrustSettingsNaviPane );

    CERTMANUILOGGER_LEAVEFN( " CCertManUIViewTrust::UpdateNaviPaneL" );
    }

// ---------------------------------------------------------
// CCertManUIViewTrust::RemoveNaviPane()
// Removes navi pane
// ---------------------------------------------------------
//
void CCertManUIViewTrust::RemoveNaviPane()
  {
  CERTMANUILOGGER_ENTERFN( " CCertManUIViewTrust::RemoveNaviPane" );

  if ( iNaviPane )
    {
    iNaviPane->Pop( iTrustSettingsNaviPane );
    }

  delete iTrustSettingsNaviPane;
  iTrustSettingsNaviPane = NULL;

  CERTMANUILOGGER_LEAVEFN( " CCertManUIViewTrust::RemoveNaviPane" );
  }


// End of File

