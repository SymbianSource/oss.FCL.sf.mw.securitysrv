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
* Description:   Implementation of class CCertManUIPlugin
*
*/


// INCLUDE FILES
#include <aknViewAppUi.h>
#include <bautils.h>

#include <certmanui.rsg>

#include "CertmanuiKeeper.h"
#include "Certmanuiviewid.h"
#include "CertmanuiPlugin.h"
#include "CertmanuiviewMain.h"
#include "CertmanuiviewTrust.h"
#include "CertmanuiviewPersonal.h"
#include "CertmanuiviewDevice.h"
#include "CertmanuiviewTrustedSite.h"
#include "CertmanuicontainerAuthority.h"
#include "CertmanuiPluginContainer.h"
#include "StringLoader.h"


// Constants
_LIT( KResourceFile, "z:CertManUi.rsc" );

// ========================= MEMBER FUNCTIONS ================================


// ---------------------------------------------------------------------------
// CCertManUIPlugin::CCertManUIPlugin()
// Constructor
//
// ---------------------------------------------------------------------------
//
CCertManUIPlugin::CCertManUIPlugin():
    iResourceLoader( *CCoeEnv::Static() ),
    iPrevViewId()
    {
    }


// ---------------------------------------------------------------------------
// CCertManUIPlugin::~CCertManUIPlugin()
// Destructor
//
// ---------------------------------------------------------------------------
//
CCertManUIPlugin::~CCertManUIPlugin()
    {
    iResourceLoader.Close();

    if ( iContainer != NULL )
        {
        AppUi()->RemoveFromViewStack( *this, iContainer );
        }

    delete iContainer;
    delete iCertManUIKeeper;
    }


// ---------------------------------------------------------------------------
// CCertManUIPlugin::ConstructL(const TRect& aRect)
// Symbian OS two-phased constructor
//
// ---------------------------------------------------------------------------
//
void CCertManUIPlugin::ConstructL()
    {
    BaseConstructL();

    AddResourceFileL();

    iCertManUIKeeper = new ( ELeave )CCertManUIKeeper();

    iCertManUIInitialized = EFalse;

    CCertManUIViewMain* viewMain =
        CCertManUIViewMain::NewLC( ClientRect(), *iCertManUIKeeper );
    AppUi()->AddViewL(viewMain);

    CCertManUIViewAuthority* viewAuthority =
        CCertManUIViewAuthority::NewLC( ClientRect(), *iCertManUIKeeper );
    AppUi()->AddViewL(viewAuthority);

    CCertManUIViewTrustedSite* viewTrustedSite =
        CCertManUIViewTrustedSite::NewLC( ClientRect(), *iCertManUIKeeper );
    AppUi()->AddViewL(viewTrustedSite);

    CCertManUIViewPersonal* viewPersonal =
        CCertManUIViewPersonal::NewLC( ClientRect(), *iCertManUIKeeper );
    AppUi()->AddViewL(viewPersonal);

    CCertManUIViewDevice* viewDevice =
        CCertManUIViewDevice::NewLC( ClientRect(), *iCertManUIKeeper );
    AppUi()->AddViewL(viewDevice);

    CCertManUIViewTrust* viewTrust =
        CCertManUIViewTrust::NewLC( ClientRect(), *iCertManUIKeeper );
    AppUi()->AddViewL( viewTrust );

    CleanupStack::Pop( 6 ); // viewMain, viewAuthority, viewTrustedSite, viewPersonal, viewDevice, viewTrust
    }
// ---------------------------------------------------------------------------
// CCertManUIPlugin::NewL()
// Static constructor
//
// ---------------------------------------------------------------------------
//
CCertManUIPlugin* CCertManUIPlugin::NewL( TAny* /*aInitParams*/ )
    {
    CCertManUIPlugin* self = new( ELeave ) CCertManUIPlugin();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// CCertManUIPlugin::Id()
//
//
// ---------------------------------------------------------------------------
//
TUid CCertManUIPlugin::Id() const
    {
    return KCertManUIPluginUid;
    }


// ---------------------------------------------------------
// CCertManUIKeeper::AddResourceFileL()
//
// ---------------------------------------------------------
//
void CCertManUIPlugin::AddResourceFileL()
    {
    TFileName fileName;
    TParse parse;
    parse.Set( KResourceFile, &KDC_RESOURCE_FILES_DIR, NULL );

    fileName = parse.FullName();
    iResourceLoader.OpenL( fileName );
    }


// ---------------------------------------------------------
// CApSettingsPlugin::HandleClientRectChange
// ---------------------------------------------------------
void CCertManUIPlugin::HandleClientRectChange()
    {
    if ( iContainer )
        {
        iContainer->SetRect( ClientRect() );
        }
    }

// ---------------------------------------------------------------------------
// CCertManUIPlugin::DoActivateL()
//
//
// ---------------------------------------------------------------------------
//
void CCertManUIPlugin::DoActivateL( const TVwsViewId& aPrevViewId,
                                   TUid /*aCustomMessageId*/,
                                   const TDesC8& /*aCustomMessage*/ )
    {
    if (iPrevViewId.iViewUid.iUid == 0)
      {
      // This is Security View
      iPrevViewId = aPrevViewId;
      }

    if( iContainer )
        {
        AppUi()->RemoveFromViewStack( *this, iContainer );
        delete iContainer;
        iContainer=NULL;
        }

    iContainer = new( ELeave ) CCertManUIPluginContainer;
    iContainer->SetMopParent( this );
    iContainer->ConstructL( ClientRect() );

    AppUi()->AddToViewStackL( *this, iContainer );

    if ( iPrevViewId != aPrevViewId )
        {
        // Activate parent view Security View
        AppUi()->ActivateLocalViewL( iPrevViewId.iViewUid );
        }
    else
        {
        if (( iCertManUIKeeper != NULL ) && ( !iCertManUIInitialized ))
            {
            iCertManUIKeeper->InitializeL( ClientRect(), KCertManUIPluginUid );
            iCertManUIInitialized = ETrue;
            }

        // Activate CertMan Authority view
        AppUi()->ActivateLocalViewL( KCertManUIViewMainId );
        }
    }


// ---------------------------------------------------------------------------
// CCertManUIPlugin::DoDeactivate()
//
//
// ---------------------------------------------------------------------------
//
void CCertManUIPlugin::DoDeactivate()
    {
    if( iContainer )
       {
       AppUi()->RemoveFromViewStack( *this, iContainer );
       delete iContainer;
       iContainer = NULL;
       }
    }


// ---------------------------------------------------------------------------
// CCertManUIPlugin::GetCaptionL()
//
//
// ---------------------------------------------------------------------------
//
void CCertManUIPlugin::GetCaptionL( TDes& aCaption ) const
    {
    StringLoader::Load( aCaption, R_GS_CERTMANUI_CAPTION  );
    }

// End of file
