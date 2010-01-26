/*
* Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   Implementation of the CSecModUIPlugin class
*
*/


// Includes
#include <aknViewAppUi.h>
#include <bautils.h>

#include <SecModUI.rsg>
#include "SecModUIModel.h"
#include "SecModUIPlugin.h"
#include "SecModUIPluginContainer.h"
#include "SecModUIViewAccess.h"
#include "SecModUIViewCode.h"
#include "SecModUIViewMain.h"
#include "SecModUIViewSignature.h"
#include "StringLoader.h"


// Constants

// ========================= MEMBER FUNCTIONS ================================


// ---------------------------------------------------------------------------
// CSecModUIPlugin::CSecModUIPlugin()
// Constructor
//
// ---------------------------------------------------------------------------
//
CSecModUIPlugin::CSecModUIPlugin():
    iResourceLoader( *CCoeEnv::Static() ),
    iPrevViewId()
    {

    }


// ---------------------------------------------------------------------------
// CSecModUIPlugin::~CSecModUIPlugin()
// Destructor
//
// ---------------------------------------------------------------------------
//
CSecModUIPlugin::~CSecModUIPlugin()
    {
    delete iContainer;
    delete iSecUiModel;
    }


// ---------------------------------------------------------------------------
// CSecModUIPlugin::ConstructL(const TRect& aRect)
// Symbian OS two-phased constructor
//
// ---------------------------------------------------------------------------
//
void CSecModUIPlugin::ConstructL()
    {

    BaseConstructL();
    
    iSecUiModel = CSecModUIModel::NewL();

    CSecModUIViewMain* viewMain = CSecModUIViewMain::NewLC(*iSecUiModel);
    AppUi()->AddViewL(viewMain);
    CleanupStack::Pop(viewMain);

    CSecModUIViewCode* viewCode = CSecModUIViewCode::NewLC(*iSecUiModel);
    AppUi()->AddViewL(viewCode);
    CleanupStack::Pop(viewCode);

    CSecModUIViewAccess* viewAccess = CSecModUIViewAccess::NewLC(*iSecUiModel);
    AppUi()->AddViewL(viewAccess);
    CleanupStack::Pop(viewAccess);

    CSecModUIViewSignature* viewSignature = CSecModUIViewSignature::NewLC(*iSecUiModel);
    AppUi()->AddViewL(viewSignature);
    CleanupStack::Pop(viewSignature);

    }
// ---------------------------------------------------------------------------
// CSecModUIPlugin::NewL()
// Static constructor
//
// ---------------------------------------------------------------------------
//
CSecModUIPlugin* CSecModUIPlugin::NewL( TAny* /*aInitParams*/ )
    {
    CSecModUIPlugin* self = new( ELeave ) CSecModUIPlugin();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// CSecModUIPlugin::Id()
//
//
// ---------------------------------------------------------------------------
//
TUid CSecModUIPlugin::Id() const
    {
    return KSecModUIPluginUid;
    }



// ---------------------------------------------------------
// CApSettingsPlugin::HandleClientRectChange
// ---------------------------------------------------------
void CSecModUIPlugin::HandleClientRectChange()
    {
    if ( iContainer )
        {
        iContainer->SetRect( ClientRect() );
        }
    }

// ---------------------------------------------------------------------------
// CSecModUIPlugin::DoActivateL()
//
//
// ---------------------------------------------------------------------------
//
void CSecModUIPlugin::DoActivateL( const TVwsViewId& aPrevViewId,
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

    iContainer = new( ELeave ) CSecModUIPluginContainer;
    iContainer->SetMopParent( this );
    iContainer->ConstructL( ClientRect() );

    AppUi()->AddToViewStackL( *this, iContainer );

    iContainer->MakeVisible( ETrue );
    iContainer->ActivateL();
    iContainer->DrawNow();

    if ( iPrevViewId != aPrevViewId )
      {
      // Activate parent view Security View
      AppUi()->ActivateLocalViewL( iPrevViewId.iViewUid );
      }
    else
      {
      // Activate Security Module Main View
      AppUi()->ActivateLocalViewL( KSecModUIViewMainId );
      }
    }


// ---------------------------------------------------------------------------
// CSecModUIPlugin::DoDeactivate()
//
//
// ---------------------------------------------------------------------------
//
void CSecModUIPlugin::DoDeactivate()
    {
    if( iContainer )
       {
       AppUi()->RemoveFromViewStack( *this, iContainer );
       delete iContainer;
       iContainer = NULL;
       }
    }


// ---------------------------------------------------------------------------
// CSecModUIPlugin::GetCaptionL()
//
//
// ---------------------------------------------------------------------------
//
void CSecModUIPlugin::GetCaptionL( TDes& aCaption ) const
    {
    StringLoader::Load( aCaption, R_QTN_WIM_SETTINGS  );
    }


// ---------------------------------------------------------------------------
// CSecModUIPlugin::Visible()
//
//
// ---------------------------------------------------------------------------
//
TBool CSecModUIPlugin::Visible() const
    {
    TBool visible( EFalse );
    TRAPD( err, visible = DoIsVisibleL() );
    if( !err && visible )
        {
        return ETrue;
        }
    return EFalse;
    }


// ---------------------------------------------------------------------------
// CSecModUIPlugin::DoIsVisibleL()
//
//
// ---------------------------------------------------------------------------
//
TBool CSecModUIPlugin::DoIsVisibleL() const
    {
    TBool isVisible( EFalse );
    const TInt KItemCount = 4;
    CDesCArray* itemArray = new (ELeave) CDesCArrayFlat( KItemCount );
    CleanupStack::PushL( itemArray );
    if( iSecUiModel )
        {
        isVisible = ( iSecUiModel->SecurityModuleCount() != 0 );
        }
    CleanupStack::PopAndDestroy( itemArray );
    return isVisible;
    }


// End of file
