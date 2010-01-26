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
* Description:   Implementation of CCertSaverContainer class
*
*/


// INCLUDE FILES
#include <AknDef.h>
#include <AknsDrawUtils.h>
#include <AknUtils.h>
#include "CertSaverContainer.h"



// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CCertSaverContainer::ConstructL()
// EPOC second phase constructor.
// ----------------------------------------------------------
//
void CCertSaverContainer::ConstructL( const TRect& aRect )
    {
    iBgContext = CAknsBasicBackgroundControlContext::NewL( KAknsIIDQsnBgAreaMain,
                   Rect(), EFalse );

    CreateWindowL();
    SetRect(aRect);
    ActivateL();
    }

// ----------------------------------------------------
// CCertSaverContainer::~CCertSaverAppUi()
// Destructor
// Frees reserved resources
// ----------------------------------------------------
//
CCertSaverContainer::~CCertSaverContainer()
    {
    delete iBgContext;
    }


// ----------------------------------------------------
// CCertSaverContainer::CountComponentControls()
// Return count of control components.
// ----------------------------------------------------
//
TInt CCertSaverContainer::CountComponentControls() const
    {
    return 0;
    }


// ----------------------------------------------------
// CCertSaverContainer::ComponentControl()
// Return control pointer.
// ----------------------------------------------------
//
CCoeControl* CCertSaverContainer::ComponentControl( TInt /*aIndex*/ ) const
    {
    return NULL;
    }

// ----------------------------------------------------
// CCertSaverContainer::Draw()
// Clear whole screen.
// ----------------------------------------------------
//
void CCertSaverContainer::Draw( const TRect& aRect ) const
    {
    CWindowGc& gc = SystemGc();

    MAknsSkinInstance* skin = AknsUtils::SkinInstance();
    MAknsControlContext* cc = AknsDrawUtils::ControlContext( this );

    AknsDrawUtils::Background( skin, cc, this, gc, aRect );
    }


// ----------------------------------------------------
// CCertSaverContainer::HandleResourceChange()
// Notifier for changing language
// ----------------------------------------------------
//
void CCertSaverContainer::HandleResourceChange( TInt aType )
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

// ----------------------------------------------------
// CCertSaverContainer::SizeChanged()
// Control size is set
// ----------------------------------------------------
//
void CCertSaverContainer::SizeChanged()
    {
    iBgContext->SetRect( Rect() );
    }

// ----------------------------------------------------
// CCertSaverContainer::MopSupplyObject()
// Pass skin information if need.
// ----------------------------------------------------
//
TTypeUid::Ptr CCertSaverContainer::MopSupplyObject( TTypeUid aId )
    {
    if( aId.iUid == MAknsControlContext::ETypeId )
      {
      return MAknsControlContext::SupplyMopObject( aId, iBgContext );
      }
    return CCoeControl::MopSupplyObject( aId );
    }

// End of File
