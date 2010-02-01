/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:
*
*/


// INCLUDE FILES
#include "AutolockContainer.h"
#include <eikenv.h>
#include <autolock.mbg>
#include <avkon.hrh>
#include <aknlayoutscalable_avkon.cdl.h>
#include <aknlayoutscalable_apps.cdl.h>
#include <layoutmetadata.cdl.h>
#include <data_caging_path_literals.hrh>
#include "AutolockAppUiPS.h"
#include <Autolock.rsg>
#include "autolock.hrh"
#include <eikdef.h>

#include <coreapplicationuisdomainpskeys.h>

#include <AknsLayeredBackgroundControlContext.h>
#include <AknsDrawUtils.h>
#include <AknBitmapAnimation.h>
#include    <AknsUtils.h>
#include	<AknUtils.h>

#include <e32property.h>
#include <PSVariables.h>   // Property values
#include <coreapplicationuisdomainpskeys.h>
#include <startupdomainpskeys.h>
#include <ctsydomainpskeys.h>
_LIT(BitmapName,"AutoLock.mbm");

// CONSTANTS

#ifdef RD_FULLSCREEN_WALLPAPER
enum TAutolockBgLayers
    {
    EAutolockBgLayerWallpaper      = 0,    
    EAutolockBgLayerBackground     = 1,
    EAutolockBgLayersN             = 2
    };
#else
enum TAutolockBgLayers
    {
    EAutolockBgLayerBackground     = 0,
    EAutolockBgLayerWallpaper      = 1,
    EAutolockBgLayersN             = 2
    };
#endif //RD_FULLSCREEN_WALLPAPER

        TInt aCallRect_x;
        TInt aCallRect_y;
        TInt aCallRect_width;
        TInt aCallRect_height;
// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CAutolockContainer::ConstructL(const TRect& aRect)
// Symbian OS two phased constructor
// ---------------------------------------------------------
//
void CAutolockContainer::ConstructL(const TRect& aRect)
    {
    #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockContainer::ConstructL"));
    #endif
    CreateWindowL();

    HBufC* bitMapPath = HBufC::NewLC(KMaxPath);
    TPtr BitmapFile(bitMapPath->Des());
    BitmapFile.Append(_L("Z:"));
    BitmapFile.Append(KDC_APP_BITMAP_DIR);
    BitmapFile.Append(BitmapName);

    iEikBitmap = 0;
    iEikBitmapCall = 0;
    AknsUtils::CreateIconL( 
        AknsUtils::SkinInstance(), 
        KAknsIIDQgnGrafPhoneLocked,
        iBitmap,
        iMask,
        BitmapFile,
        EMbmAutolockQgn_graf_phone_locked, 
        EMbmAutolockQgn_graf_phone_locked_mask );
    AknsUtils::CreateIconL( 
        AknsUtils::SkinInstance(), 
        KAknsIIDQgnGrafPhoneLocked,
        iBitmapCall,
        iMaskCall,
        BitmapFile,
        EMbmAutolockQgn_indi_button_end_call, 
        EMbmAutolockQgn_indi_button_end_call_mask );
    
#ifdef RD_FULLSCREEN_WALLPAPER
    TSize screenSize = iCoeEnv->ScreenDevice()->SizeInPixels();
    TRect wallpaperRect( TPoint(0,0), screenSize );    
    iBgContext = CAknsLayeredBackgroundControlContext::NewL(
    KAknsIIDWallpaper, wallpaperRect, ETrue, EAutolockBgLayersN );
    SetRect(aRect);
#else    
    // Create background control context for skins. Use parent absolute mode,
    // as this is window owning control
    iBgContext = CAknsLayeredBackgroundControlContext::NewL(
        KAknsIIDQsnBgAreaMainIdle, aRect, ETrue, EAutolockBgLayersN );
    iBgContext->SetLayerImage( EAutolockBgLayerWallpaper, KAknsIIDWallpaper );
    iBgContext->SetLayerRect( EAutolockBgLayerWallpaper, aRect );
    SetRect(aRect);
#endif // RD_FULLSCREEN_WALLPAPER
    ActivateL();

    CleanupStack::PopAndDestroy(bitMapPath); //bitMapPath
    #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockContainer::ConstructL END"));
    #endif
    }


// ---------------------------------------------------------
// CAutolockContainer::~CAutolockContainer()
// Destructor
// ---------------------------------------------------------
//
CAutolockContainer::~CAutolockContainer()
    {
    delete iBgContext;
    delete iBitmap;
    delete iMask;
    delete iBitmapCall;
    delete iMaskCall;

	}
// ---------------------------------------------------------
// CAutolockContainer::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CAutolockContainer::SizeChanged()
    {
    #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockContainer::SizeChanged()"));
    #endif
    
    TRect mainPaneRect(Rect());
    TAknLayoutRect idleTradPane;
    idleTradPane.LayoutRect(mainPaneRect, AknLayoutScalable_Avkon::main_idle_trad_pane());

    TInt variety = 3;
    if (Layout_Meta_Data::IsLandscapeOrientation())
        {
        variety = 2;
        }


    TAknLayoutRect idlePaneG2;
    idlePaneG2.LayoutRect(idleTradPane.Rect(), AknLayoutScalable_Avkon::main_idle_pane_g2(variety));

    TInt callState = 0;
	  RProperty::Get( KPSUidCtsyCallInformation, KCTsyCallState, callState );
    if ( callState == EPSCTsyCallStateNone || callState == EPSCTsyCallStateUninitialized )
    	{
    AknIconUtils::SetSize( iBitmap, idlePaneG2.Rect().Size() );
    	}
    else
    	{
		  TSize lockSize = TSize(0.8 * idlePaneG2.Rect().Size().iWidth, 0.8 * idlePaneG2.Rect().Size().iHeight);
	    AknIconUtils::SetSize( iBitmap, lockSize );
    	}
		TSize callSize = TSize(60,60);
    AknIconUtils::SetSize( iBitmapCall, callSize );
    
#ifdef  RD_FULLSCREEN_WALLPAPER
    TSize screenSize = iCoeEnv->ScreenDevice()->SizeInPixels();
    TRect wallpaperRect( TPoint(0,0), screenSize );    
    iBgContext->SetLayerRect( EAutolockBgLayerBackground, Rect() ) ;
    iBgContext->SetLayerRect( EAutolockBgLayerWallpaper, wallpaperRect ) ;
    
    TPoint origo( 0, 0);
    iBgContext->SetParentPos(origo);
#else

    iBgContext->SetLayerRect( EAutolockBgLayerBackground, Rect() ) ;
    iBgContext->SetLayerRect( EAutolockBgLayerWallpaper, Rect() ) ;
     
    TPoint positionRelativeToScreen = PositionRelativeToScreen();
    //parent must be set when using parent absolute mode.
    iBgContext->SetParentPos(positionRelativeToScreen);
#endif //RD_FULLSCREEN_WALLPAPER

        RRegion autolockRegion;
#ifdef  RD_FULLSCREEN_WALLPAPER    	
    	autolockRegion.AddRect( wallpaperRect);
#else    	
    	autolockRegion.AddRect(Rect());
#endif //RD_FULLSCREEN_WALLPAPER   
        Window().SetShape(autolockRegion);
        autolockRegion.Close();
    
     #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockContainer::SizeChanged() END"));
    #endif
	 }
// ---------------------------------------------------------
// CAutolockContainer::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CAutolockContainer::CountComponentControls() const
    {
    TInt controlCount = 0;

    return controlCount;
    }

// ---------------------------------------------------------
// CAutolockContainer::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* CAutolockContainer::ComponentControl(TInt /*aIndex*/) const
    {
    return NULL;
	} 
 // ---------------------------------------------------------
// CAutolockContainer::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CAutolockContainer::Draw(const TRect& aRect) const
    {
    #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockContainer::Draw"));
    #endif
    if (AknLayoutUtils::PenEnabled() )
    	{
			TInt value = 0;
			RProperty::Get(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, value);	    
			if(value <= EAutolockOff)
				{	// Avoid displaying the icon
				#if defined(_DEBUG)
				RDebug::Printf( "%s %s (%u) no Draw value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, value );
				#endif
				return;
				}
			}

    CWindowGc& gc = SystemGc();
    gc.SetPenStyle(CGraphicsContext::ENullPen);
    gc.SetBrushColor(KRgbWhite); 
    gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
    MAknsSkinInstance* skin = AknsUtils::SkinInstance();
    MAknsControlContext* cc = AknsDrawUtils::ControlContext( this );
    AknsDrawUtils::Background( skin, cc, this, gc, aRect );

    // Draw "lock" icon centered to this control    
    // fist calculate the correct x coordinate based on this 
    // controls rect and bitmap width    
    TInt x = Rect().Width()/2 - iBitmap->SizeInPixels().iWidth /2;
    // and do same with y
    TInt y = Rect().Height()/2 - iBitmap->SizeInPixels().iHeight/2;
		TInt width =iBitmap->SizeInPixels().iWidth;
		TInt height =iBitmap->SizeInPixels().iHeight;
      TInt callState = 0;
  	  RProperty::Get( KPSUidCtsyCallInformation, KCTsyCallState, callState );
    if ( callState == EPSCTsyCallStateNone || callState == EPSCTsyCallStateUninitialized )
    	{
    	}
    else
    	{
    	y-=100;
    	}
    
    if (iBitmap && iMask)
        {
          // gc.BitBltMasked(TPoint(x,y),iBitmap,TRect(TPoint(0,0),iBitmap->SizeInPixels()), iMask, ETrue);
          gc.BitBltMasked(TPoint(x,y),iBitmap,TRect(TPoint(0,0),TPoint(width,height)), iMask, ETrue);
    	  if ( callState == EPSCTsyCallStateNone || callState == EPSCTsyCallStateUninitialized )
    	  	{
    	  }
    	  else
    	  	{
   	    	aCallRect_x=Rect().Width()/2 - iBitmapCall->SizeInPixels().iWidth/2;
		    	aCallRect_y=Rect().Height() * 0.75;
		    	aCallRect_width=iBitmapCall->SizeInPixels().iWidth;
		    	aCallRect_height=iBitmapCall->SizeInPixels().iHeight;
    	  	TSize cornerSize(20,20); 
    	  	TSize cornerEllipseSize(cornerSize.iHeight*2,cornerSize.iWidth*2);
    	  	TRect box(aCallRect_x-3*10	, aCallRect_y-3*10, aCallRect_x+aCallRect_width+3*10, aCallRect_y+aCallRect_height+3*10);
					TRect cornerRectTl(box.iTl,cornerEllipseSize);
					gc.SetBrushColor(KRgbRed); 
					gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
					gc.SetPenStyle(CGraphicsContext::EDottedPen);
					gc.DrawRoundRect(box,cornerSize);
        	gc.BitBltMasked(TPoint(aCallRect_x,aCallRect_y),iBitmapCall,TRect(TPoint(0,0),TPoint(aCallRect_width,aCallRect_height)), iMaskCall, ETrue);
    	  	aCallRect_y+=100;	// coordinates are realtive to TRect, not to Screen
        	}
        }
    else if (iBitmap && !iMask)
        {
        gc.BitBlt(TPoint(x,y),iBitmap);
    	  if ( callState == EPSCTsyCallStateNone || callState == EPSCTsyCallStateUninitialized )
    	  	{
    	  	}
    	  else
    	  	{
        	gc.BitBlt(TPoint(x,y+iBitmap->SizeInPixels().iHeight),iBitmapCall);
        	}
        }
    #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockContainer::Draw END"));
    #endif
    }

// ---------------------------------------------------------
// CAutolockContainer::HandleControlEventL(
//     CCoeControl* aControl,TCoeEvent aEventType)
// ---------------------------------------------------------
//
void CAutolockContainer::HandleControlEventL(
    CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/)
    {
    }

// ---------------------------------------------------------
// CAutolockContainer::MopSupplyObject
// 
// ---------------------------------------------------------
//
TTypeUid::Ptr CAutolockContainer::MopSupplyObject( TTypeUid aId )
    {
    if (aId.iUid == MAknsControlContext::ETypeId)
        {
        return MAknsControlContext::SupplyMopObject( aId, iBgContext );
        }
    return CCoeControl::MopSupplyObject( aId );
    }
void CAutolockContainer::GiveCoords( TRect& aRect )
    {
    	aRect.iBr.iX=aCallRect_x;
    	aRect.iBr.iY=aCallRect_y;
    	aRect.iTl.iX=aCallRect_width+2*3*10;
    	aRect.iTl.iY=aCallRect_height+2*3*10;
    }
// End of File  
