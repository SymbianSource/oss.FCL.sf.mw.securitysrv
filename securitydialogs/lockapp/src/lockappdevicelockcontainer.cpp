/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Devicelock Background UI (window owning control)
 *
*/


// INCLUDE FILES
#include "lockappdevicelockcontainer.h"
#include "lockapputils.h"

#include <eikenv.h>
#include <lockapp.mbg>
#include <avkon.hrh>
#include <aknlayoutscalable_avkon.cdl.h>
#include <aknlayoutscalable_apps.cdl.h>
#include <AknsLayeredBackgroundControlContext.h>
#include <AknsDrawUtils.h>
#include <AknBitmapAnimation.h>
#include <AknsUtils.h>
#include <AknUtils.h>
#include <aknappui.h>

#include <layoutmetadata.cdl.h>
#include <data_caging_path_literals.hrh>
#include <eikdef.h>

_LIT(LOCKBITMAPNAME, "AutoLock.mbm"); // TODO change filename

// CONSTANTS

#ifdef RD_FULLSCREEN_WALLPAPER
enum TAutolockBgLayers
    {
    EAutolockBgLayerWallpaper = 0,
    EAutolockBgLayerBackground = 1,
    EAutolockBgLayersN = 2
    };
#else
enum TAutolockBgLayers
    {
    EAutolockBgLayerBackground = 0,
    EAutolockBgLayerWallpaper = 1,
    EAutolockBgLayersN = 2
    };
#endif //RD_FULLSCREEN_WALLPAPER

// ---------------------------------------------------------------------------
// Standard Symbian OS construction sequence
// ---------------------------------------------------------------------------
CLockAppDevicelockContainer* CLockAppDevicelockContainer::NewL( RWindowGroup& aWg )
    {
    CLockAppDevicelockContainer* self = new (ELeave) CLockAppDevicelockContainer( );
    CleanupStack::PushL( self );
    self->ConstructL( aWg );
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------
// Symbian OS two phased constructor
// ---------------------------------------------------------
void CLockAppDevicelockContainer::ConstructL( RWindowGroup& aWg )
    {
    INFO( "CLockAppDevicelockContainer::ConstructL started" );

    CreateWindowL( aWg );
    MakeVisible( EFalse );

    HBufC* bitMapPath = HBufC::NewLC( KMaxPath );
    TPtr BitmapFile( bitMapPath->Des( ) );
    BitmapFile.Append( _L("Z:") );
    BitmapFile.Append( KDC_APP_BITMAP_DIR );
    BitmapFile.Append( LOCKBITMAPNAME );

    TRect mainPaneRect = GetMainPaneRect( );

    iClock = NULL;
    iEikBitmap = NULL;
    AknsUtils::CreateIconL( AknsUtils::SkinInstance( ), KAknsIIDQgnGrafPhoneLocked, iBitmap, iMask,
            BitmapFile, EMbmLockappQgn_graf_phone_locked, EMbmLockappQgn_graf_phone_locked_mask );

#ifdef RD_FULLSCREEN_WALLPAPER
    TSize screenSize = iCoeEnv->ScreenDevice()->SizeInPixels( );
    TRect wallpaperRect( TPoint( 0, 0 ), screenSize);
    iBgContext = CAknsLayeredBackgroundControlContext::NewL( KAknsIIDWallpaper, wallpaperRect,
            ETrue, EAutolockBgLayersN );
#else
    // Create background control context for skins. Use parent absolute mode,
    // as this is window owning control
    iBgContext = CAknsLayeredBackgroundControlContext::NewL(
            KAknsIIDQsnBgAreaMainIdle, mainPaneRect, ETrue, EAutolockBgLayersN );
    iBgContext->SetLayerImage( EAutolockBgLayerWallpaper, KAknsIIDWallpaper );
    iBgContext->SetLayerRect( EAutolockBgLayerWallpaper, mainPaneRect );
#endif // RD_FULLSCREEN_WALLPAPER

    SetRect( mainPaneRect );
    CleanupStack::PopAndDestroy( bitMapPath ); //bitMapPath
    INFO( "CLockAppDevicelockContainer::ConstructL completed" );
    }

// ---------------------------------------------------------
// Destructor
// ---------------------------------------------------------
CLockAppDevicelockContainer::~CLockAppDevicelockContainer( )
    {
    delete iBgContext;
    delete iBitmap;
    delete iMask;
    delete iClock;
    }

// ---------------------------------------------------------
// Return the rectangle for this control
// ---------------------------------------------------------
TRect CLockAppDevicelockContainer::GetMainPaneRect( )
    {
    TRect screen( iAvkonAppUi->ApplicationRect());
    TAknLayoutRect applicationWindow;
    applicationWindow.LayoutRect( screen, AknLayoutScalable_Avkon::application_window( 0 ) );
    TInt mainPaneVariety = ( Layout_Meta_Data::IsLandscapeOrientation() ? 4 : 3 );
    TAknLayoutRect mainPane;
    mainPane.LayoutRect( applicationWindow.Rect( ),
            AknLayoutScalable_Avkon::main_pane( mainPaneVariety ) );
    return mainPane.Rect( );
    }

// ---------------------------------------------------------
// Called by framework when the view size is changed
// ---------------------------------------------------------
void CLockAppDevicelockContainer::SizeChanged()
    {
    INFO( "CLockAppDevicelockContainer::SizeChanged" );
    TRect mainPaneRect( Rect( ));
    TAknLayoutRect idleTradPane;
    idleTradPane.LayoutRect( mainPaneRect, AknLayoutScalable_Avkon::main_idle_trad_pane( ) );

    TInt variety = ( Layout_Meta_Data::IsLandscapeOrientation() ? 2 : 3 );

    TAknLayoutRect idlePaneG2;
    idlePaneG2.LayoutRect( idleTradPane.Rect( ),
            AknLayoutScalable_Avkon::main_idle_pane_g2( variety ) );
    AknIconUtils::SetSize( iBitmap, idlePaneG2.Rect().Size( ) );

    TPoint parentPos( 0, 0);
#ifdef RD_FULLSCREEN_WALLPAPER
    TSize screenSize = iCoeEnv->ScreenDevice()->SizeInPixels( );
    TRect wallpaperRect( TPoint( 0, 0 ), screenSize);
    iBgContext->SetLayerRect( EAutolockBgLayerBackground, Rect( ) );
    iBgContext->SetLayerRect( EAutolockBgLayerWallpaper, wallpaperRect );
#else
    iBgContext->SetLayerRect( EAutolockBgLayerBackground, Rect() );
    iBgContext->SetLayerRect( EAutolockBgLayerWallpaper, Rect() );
    //parent must be set when using parent absolute mode.
    parentPos = PositionRelativeToScreen();
#endif //RD_FULLSCREEN_WALLPAPER

    iBgContext->SetParentPos( parentPos );

    if ( !Layout_Meta_Data::IsLandscapeOrientation() )
        {
        // the screen is in portrait mode
        if ( iClock )
            {
            // remove clock, if it exists
            delete iClock;
            iClock = NULL;
            }
        RRegion autolockRegion;
#ifdef  RD_FULLSCREEN_WALLPAPER
        autolockRegion.AddRect( wallpaperRect );
#else
        autolockRegion.AddRect( Rect() );
#endif //RD_FULLSCREEN_WALLPAPER
        Window().SetShape( autolockRegion );
        autolockRegion.Close( );
        }
    else
        {
        // the screen is in landscape mode.
        // get the correct area from layout utils.
        TAknLayoutRect popupClockDigitalAnalogueWindowLayoutRect;
        popupClockDigitalAnalogueWindowLayoutRect.LayoutRect( idleTradPane.Rect( ),
                AknLayoutScalable_Avkon::popup_clock_digital_analogue_window( 3 ) );
        TRect popupClockDigitalAnalogueWindowRect(popupClockDigitalAnalogueWindowLayoutRect.Rect( ));

        // since the clock is not shown in app shell when the screen is in landscape,
        // we'll show a clock here.
        if ( !iClock )
            {
            TRAP_IGNORE({
                        CAknSkinnableClock* clock = CAknSkinnableClock::NewL( this, ETrue, EFalse );
                        CleanupStack::PushL( clock );
                        clock->SetRect( popupClockDigitalAnalogueWindowRect );
                        clock->ActivateL();
                        CleanupStack::Pop( clock );
                        iClock = clock;
                        });
            }
        else
            {
            iClock->SetRect( popupClockDigitalAnalogueWindowRect );
            }
        RRegion autolockRegion;
#ifdef RD_FULLSCREEN_WALLPAPER
        autolockRegion.AddRect( wallpaperRect );
#else
        autolockRegion.AddRect( Rect() );
#endif //RD_FULLSCREEN_WALLPAPER
        autolockRegion.AddRect( iClock->Rect( ) );
        Window().SetShape( autolockRegion );
        autolockRegion.Close( );
        }
     }

// ---------------------------------------------------------
// CLockAppDevicelockContainer::CountComponentControls() const
// ---------------------------------------------------------
TInt CLockAppDevicelockContainer::CountComponentControls( ) const
    {
    TInt controlCount = 0;
    if ( Layout_Meta_Data::IsLandscapeOrientation( ) )
        {
        if ( iClock )
            {
            controlCount++;
            }
        }
    return controlCount;
    }

// ---------------------------------------------------------
// CLockAppDevicelockContainer::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
CCoeControl* CLockAppDevicelockContainer::ComponentControl(TInt /*aIndex*/) const
    {
    return iClock;
    }

// ---------------------------------------------------------
// CLockAppDevicelockContainer::Draw(const TRect& aRect) const
// ---------------------------------------------------------
void CLockAppDevicelockContainer::Draw(const TRect& aRect) const
    {
    CWindowGc& gc = SystemGc( );
    gc.SetPenStyle( CGraphicsContext::ENullPen );
    gc.SetBrushColor( KRgbWhite );
    gc.SetBrushStyle( CGraphicsContext::ESolidBrush );
    MAknsSkinInstance* skin = AknsUtils::SkinInstance( );
    MAknsControlContext* cc = AknsDrawUtils::ControlContext( this );
    AknsDrawUtils::Background( skin, cc, this, gc, aRect );

    if ( iBitmap )
        {
        // Draw "lock" icon centered to this control
        TInt x = (Rect().Width( ) - iBitmap->SizeInPixels().iWidth) / 2;
        TInt y = (Rect().Height( ) - iBitmap->SizeInPixels().iHeight) / 2;
        if ( iMask )
            {
            gc.BitBltMasked( TPoint( x, y ), iBitmap, TRect( TPoint( 0, 0 ),
                    iBitmap->SizeInPixels( ) ), iMask, ETrue );
            }
        else
            {
            gc.BitBlt( TPoint( x, y ), iBitmap );
            }
        }
    }

// ---------------------------------------------------------
// CLockAppDevicelockContainer::HandleControlEventL( CCoeControl* aControl,TCoeEvent aEventType)
// ---------------------------------------------------------
void CLockAppDevicelockContainer::HandleControlEventL(CCoeControl* /*aControl*/, TCoeEvent /*aEventType*/)
    {
    }

// ---------------------------------------------------------
// CLockAppDevicelockContainer::MopSupplyObject
// ---------------------------------------------------------
TTypeUid::Ptr CLockAppDevicelockContainer::MopSupplyObject( TTypeUid aId )
    {
    if ( aId.iUid == MAknsControlContext::ETypeId )
        {
        return MAknsControlContext::SupplyMopObject( aId, iBgContext );
        }
    return CCoeControl::MopSupplyObject( aId );
    }

// End of File
