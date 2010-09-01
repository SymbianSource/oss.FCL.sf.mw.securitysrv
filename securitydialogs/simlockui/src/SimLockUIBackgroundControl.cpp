/*
* ============================================================================
*  Name        : SimLockUIBackgroundControl.cpp
*  Part of     : Sim Lock UI Application
*  Description : Implementation of Sim Lock UI Background
*  Version     :
*  
* Copyright (c) 2005-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   Build info file for Ado domain appinstall 
* ============================================================================
*/


// System Includes
#include <coemain.h>
#include <AknsDrawUtils.h>                      // AknsDrawUtils
#include <AknsSkinInstance.h>                   // MAknsSkinInstance
#include <AknsControlContext.h>                 // MAknsControlContext
#include <AknsBasicBackgroundControlContext.h>  // CAknsBasicBackgroundControlContext
#include <aknlayoutscalable_avkon.cdl.h>        // AknLayoutScalable_Avkon
#include <AknUtils.h>                           // AknsUtils

// User Includes
#include "simlockuibackgroundcontrol.h"
#include "simlockisaserverdefinitions.h"
#include <simlockui.rsg>

static const TInt KSkinLayoutOption = 2;

// ---------------------------------------------------------------------------
// CSimLockUIBackgroundControl::NewL
// ---------------------------------------------------------------------------
CSimLockUIBackgroundControl* CSimLockUIBackgroundControl::NewL()
    {
    CSimLockUIBackgroundControl* self = CSimLockUIBackgroundControl::NewLC();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// CSimLockUIBackgroundControl::NewLC
// ---------------------------------------------------------------------------
CSimLockUIBackgroundControl* CSimLockUIBackgroundControl::NewLC()
    {
    CSimLockUIBackgroundControl* self = new ( ELeave ) CSimLockUIBackgroundControl;
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------------------------
// CSimLockUIBackgroundControl::~CSimLockUIBackgroundControl
// ---------------------------------------------------------------------------
CSimLockUIBackgroundControl::~CSimLockUIBackgroundControl()
    {
    delete iBackgroundSkinContext;
    delete iDisplayText;
    }

// ---------------------------------------------------------------------------
// CSimLockUIBackgroundControl::SetBackgroundText
// ---------------------------------------------------------------------------
void CSimLockUIBackgroundControl::SetBackgroundText( HBufC* aText )
    {
    delete iDisplayText;

    iDisplayText = aText;

    DrawDeferred();
    }

// ---------------------------------------------------------------------------
// CSimLockUIBackgroundControl::ConstructL
// ---------------------------------------------------------------------------
void CSimLockUIBackgroundControl::ConstructL()
    {
    // Create a window for this application view
    CreateWindowL();
    SetExtentToWholeScreen();

    iBackgroundSkinContext = CAknsBasicBackgroundControlContext::NewL(
            KAknsIIDQsnBgAreaMain, Rect(), EFalse );

    // Activate the window, which makes it ready to be drawn
    ActivateL();
    }

// ---------------------------------------------------------------------------
// CSimLockUIBackgroundControl::SizeChanged
// ---------------------------------------------------------------------------
void CSimLockUIBackgroundControl::SizeChanged()
    {
    // Background skin.
    if ( iBackgroundSkinContext )
        iBackgroundSkinContext->SetRect( Rect() );
    }

// ---------------------------------------------------------------------------
// CSimLockUIBackgroundControl::HandleResourceChange()
// ---------------------------------------------------------------------------
//
void CSimLockUIBackgroundControl::HandleResourceChange( TInt aType )
    {
    CCoeControl::HandleResourceChange( aType );

    if ( aType == KEikDynamicLayoutVariantSwitch )
        {
        SetExtentToWholeScreen();
        }
    }


// ---------------------------------------------------------------------------
// CSimLockUIBackgroundControl::Draw
// ---------------------------------------------------------------------------
void CSimLockUIBackgroundControl::Draw( const TRect& /*aRect*/ ) const
    {
    // Draw background skin
    CWindowGc& graphicsContext = SystemGc();
    MAknsSkinInstance* skin = AknsUtils::SkinInstance();
    MAknsControlContext* controlContext = AknsDrawUtils::ControlContext( this );
    AknsDrawUtils::Background( skin, controlContext, this, graphicsContext, Rect() );

    // Draw background text
    DisplayText( graphicsContext );
    }

// ---------------------------------------------------------------------------
// CSimLockUIBackgroundControl::MopSupplyObject
// ---------------------------------------------------------------------------
TTypeUid::Ptr CSimLockUIBackgroundControl::MopSupplyObject( TTypeUid aId )
    {
    if ( aId.iUid == MAknsControlContext::ETypeId )
        {
        // Return the background skin object
        return MAknsControlContext::SupplyMopObject( aId, iBackgroundSkinContext );
        }

    return CCoeControl::MopSupplyObject( aId );
    }

// ---------------------------------------------------------------------------
// CSimLockUIBackgroundControl::DisplayText
// ---------------------------------------------------------------------------
void CSimLockUIBackgroundControl::DisplayText( CGraphicsContext& aGc ) const
    {
    if ( iDisplayText )
        {
        // Set up text and layout
        TAknLayoutText line;
        TAknTextLineLayout layout = AknLayoutScalable_Avkon::main_pane_empty_t1( KSkinLayoutOption ).LayoutLine();

        line.LayoutText( Rect(), layout );

        // Get color from skin instance.
        TRgb color( line.Color() );

        if ( AknsUtils::AvkonSkinEnabled() )
            {
            TRgb skinColor;

            MAknsSkinInstance* skin = AknsUtils::SkinInstance();

            TInt err = AknsUtils::GetCachedColor( skin,
                                                  skinColor,
                                                  KAknsIIDQsnTextColors,
                                                  EAknsCIQsnTextColorsCG6 );
            if ( err == KErrNone )
                {
                color = skinColor;
                }
            }

        // Actually display the text
        line.DrawText( aGc, *iDisplayText, ETrue, color );
        }
    }

// End of file.

