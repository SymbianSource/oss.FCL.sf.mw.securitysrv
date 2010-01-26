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
* Description:   Implementation of class CCertManUIViewMain
*
*/


// INCLUDE FILES

#include <aknmessagequerydialog.h>
#include <aknnotewrappers.h>            // for warning & information notes
#include <certmanui.rsg>
#include <ErrorUI.h>
#include "CertmanuiCertificateHelper.h"
#include "CertmanuiKeeper.h"
#include "CertmanuicontainerMain.h"
#include "CertmanuiSyncWrapper.h"
#include "Certmanui.hrh"
#include "CertmanuiCommon.h"
#include "CertManUILogger.h"
#include "CertmanuiPlugin.h"            // for KCertManUIPluginUid

#include    <hlplch.h>                  // For HlpLauncher

const TUid KGSSecurityViewUid = { 0x1020743A };


// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CCertManUIViewMain::CCertManUIViewMain(CCertManUIKeeper& aKeeper)
// : iKeeper(aKeeper), iCrMgr(iKeeper.CertManager())
// Constructor
// ---------------------------------------------------------
//
CCertManUIViewMain::CCertManUIViewMain(CCertManUIKeeper& aKeeper)
    : iKeeper( aKeeper )
    {
    CERTMANUILOGGER_WRITE_TIMESTAMP( "CCertManUIViewMain::CCertManUIViewMain" );
    }

// -----------------------------------------------------------------------------
// CCertManUIViewMain::~CCertManUIViewMain()
// Destructor
// -----------------------------------------------------------------------------
//
CCertManUIViewMain::~CCertManUIViewMain()
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUIViewMain::~CCertManUIViewMain" );

    if ( iContainerMain )
        {
        AppUi()->RemoveFromViewStack( *this, iContainerMain );
        delete iContainerMain;
        }

    CERTMANUILOGGER_LEAVEFN( "CCertManUIViewMain::~CCertManUIViewMain" );
    }

// ---------------------------------------------------------
// CCertManUIViewMain* CCertManUIViewMain::NewL(const TRect& /*aRect*/,
//                                CCertManUIKeeper& aKeeper)
// ---------------------------------------------------------
//
CCertManUIViewMain* CCertManUIViewMain::NewL( const TRect& /*aRect*/,
                                CCertManUIKeeper& aKeeper )
    {
    CCertManUIViewMain* self = new ( ELeave ) CCertManUIViewMain( aKeeper );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// ---------------------------------------------------------
// CCertManUIViewMain* CCertManUIViewMain::NewLC(const TRect& /*aRect*/,
//                                 CCertManUIKeeper& aKeeper)
// ---------------------------------------------------------
//
CCertManUIViewMain* CCertManUIViewMain::NewLC(const TRect& /*aRect*/,
                                 CCertManUIKeeper& aKeeper)
    {
    CCertManUIViewMain* self = new ( ELeave ) CCertManUIViewMain( aKeeper );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------
// CCertManUIViewMain::ConstructL()
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CCertManUIViewMain::ConstructL()
    {
  CERTMANUILOGGER_ENTERFN( "CCertManUIViewMain::ConstructL" );

  BaseConstructL( R_CERTMANUI_VIEW_MAIN );

  CERTMANUILOGGER_LEAVEFN( "CCertManUIViewMain::ConstructL" );
  }

// ---------------------------------------------------------
// CCertManUIViewMain::Id() const
// Returns Personal view id
// ---------------------------------------------------------
//
TUid CCertManUIViewMain::Id() const
    {
    return KCertManUIViewMainId;
    }

// ---------------------------------------------------------
// CCertManUIViewMain::HandleCommandL(TInt aCommand)
// Handles Softkey and Options list commands
// ---------------------------------------------------------
//
void CCertManUIViewMain::HandleCommandL( TInt aCommand )
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUIViewMain::HandleCommandL" );

    switch ( aCommand )
        {
        case EAknSoftkeyBack:
            {
            // Activate Security view
            AppUi()->ActivateLocalViewL( KGSSecurityViewUid );
            break;
            }
        case EAknCmdExit:
            {
            ((CAknViewAppUi*)iAvkonAppUi)->HandleCommandL( EAknCmdExit );
            break;
            }
        case ECertManUICmdAppViewDtls:
        case ECertManUICmdAppOpen:
            {
            TInt index = iContainerMain->iListBox->CurrentItemIndex();

            if ( index == 0)
                {
                AppUi()->ActivateLocalViewL( KCertManUIViewAuthorityId );
                }
            else if ( index == 1)
                {
                AppUi()->ActivateLocalViewL( KCertManUIViewTrustedSiteId );
                }
            else if ( index == 2)
                {
                AppUi()->ActivateLocalViewL( KCertManUIViewPersonalId );
                }
            else if ( index == 3)
                {
                AppUi()->ActivateLocalViewL( KCertManUIViewDeviceId );
                }
            else  // For lint
                {
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

    CERTMANUILOGGER_LEAVEFN( "CCertManUIViewMain::HandleCommandL" );
    }

// ---------------------------------------------------------
// CCertManUIViewMain::HandleClientRectChange()
// ---------------------------------------------------------
//
void CCertManUIViewMain::HandleClientRectChange()
    {
    if ( iContainerMain )
        {
        iContainerMain->SetRect( ClientRect() );
        }
    }

// ---------------------------------------------------------
// CCertManUIViewMain::DoActivateL(const TVwsViewId& aPrevViewId,
//                                 TUid /*aCustomMessageId*/,
//                                 const TDesC8& /*aCustomMessage*/)
// Updates the view when opening it
// ---------------------------------------------------------
//
void CCertManUIViewMain::DoActivateL( const TVwsViewId& aPrevViewId,
                                      TUid /*aCustomMessageId*/,
                                      const TDesC8& /*aCustomMessage*/ )
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUIViewMain::DoActivateL" );

    if ( iContainerMain )
        {
        ((CAknViewAppUi*)iAvkonAppUi)->RemoveFromViewStack(
            *this, iContainerMain );

        delete iContainerMain;
        iContainerMain = NULL;
        }

    if( aPrevViewId.iViewUid == KCertManUIPluginUid )
        {
        iCurrentPosition = 0;
        }

    iContainerMain = new (ELeave) CCertManUIContainerMain(
        *this, iKeeper );

    iContainerMain->SetMopParent( this );
    iContainerMain->ConstructL(
        ClientRect(), iCurrentPosition, iTopItem );


    iContainerMain->DrawListBoxL( iCurrentPosition, iTopItem );
    ((CAknViewAppUi*)iAvkonAppUi)->AddToStackL( *this, iContainerMain );

    CERTMANUILOGGER_LEAVEFN( "CCertManUIViewMain::DoActivateL" );
    }

// ---------------------------------------------------------
// CCertManUIViewMain::DoDeactivate()
// Saves focus position when closing view
// ---------------------------------------------------------
//
void CCertManUIViewMain::DoDeactivate()
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUIViewMain::DoDeactivate" );

    iCurrentPosition = iContainerMain->iListBox->CurrentItemIndex();
    iTopItem = iContainerMain->iListBox->TopItemIndex();

    CERTMANUILOGGER_LEAVEFN( "CCertManUIViewMain::DoDeactivate" );
    }

// ---------------------------------------------------------
// CCertManUIViewMain::Container()
// Returns iContainerMain to be used in CertManUIContainerMain
// ---------------------------------------------------------
//
CCoeControl* CCertManUIViewMain::Container()
    {
    return iContainerMain;
    }


// ---------------------------------------------------------
// CCertManUIViewMain::DynInitMenuPaneL()
// ---------------------------------------------------------
//
void CCertManUIViewMain::DynInitMenuPaneL(
    TInt aResourceId, CEikMenuPane* aMenuPane )
    {
    if ( aResourceId == R_CERTMANUI_VIEW_MENU_MAIN )
        {
        if( !FeatureManager::FeatureSupported( KFeatureIdHelp ) )
            {
            aMenuPane->DeleteMenuItem( EAknCmdHelp ); //remove help
            }
        }
    }

// End of File

