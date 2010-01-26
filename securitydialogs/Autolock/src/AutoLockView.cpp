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

#include <e32std.h>
#include <eiklabel.h>
#include <eikfutil.h>
#include <aknconsts.h>
#include <akntitle.h>
#include <avkon.mbg>
#include <aknview.h>
#include <aknViewAppUi.h>
#include <Autolock.rsg>
#include "autolock.hrh"
#include <aknlayoutscalable_avkon.cdl.h>
#include <layoutmetadata.cdl.h>
#include <AknUtils.h>
#include "AutolockAppUiPS.h"
#include "AutoLockModelPS.h"
#include "AutolockView.h"
#include "AutolockContainer.h"




// CONSTANTS

const TUid KUidStartUp = { 0x100058F4 };
const TInt KPhoneAppOrdinalPosition = 1; // used to pull phone app closer to foreground

// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------
// CAutolockView::ConstructL()
// Symbian OS default constructor can leave..
// ----------------------------------------------------
//
void CAutolockView::ConstructL()
	{
	#if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockView::ConstructL()"));
    #endif
    BaseConstructL();
	}
// ----------------------------------------------------
// CAutolockView::~CAutolockView()
// Destructor
// ----------------------------------------------------
//
CAutolockView::~CAutolockView()
	{
	#if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockView::~CAutolockView()"));
    #endif
    CAutolockContainer* view = iView;
    iView = NULL;
	if (view)
        {
		AppUi()->RemoveFromStack(view);
        }
	delete view;
	}
// ----------------------------------------------------
// CAutolockView::Id()
// Returns view Id
// ----------------------------------------------------
//
TUid CAutolockView::Id() const
    {
    return KAutoLockViewId;
    }
// ----------------------------------------------------
// CAutolockView::HandleCommandL()
// Handles user inputs
// ----------------------------------------------------
//
void CAutolockView::HandleCommandL(TInt aCommand)
    {
    #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockView::HandleCommandL()"));
    #endif   
    AppUi()->HandleCommandL(aCommand);
    #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockView::HandleCommandL END()"));
    #endif
    }
void CAutolockView::HandleCall(TInt aCommand, TRect &aRect)
    {
    	RDebug::Printf( "%s %s (%u) aCommand=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aCommand );
    if (iView)
    	{
        TRect cr = ClientRect();
        iView->SetRect( cr );
    		RDebug::Printf( "%s %s (%u) got 2 cr=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aCommand );
        iView->DrawNow( );
        iView->GiveCoords( aRect );
			}
    else
			{
			RDebug::Printf( "%s %s (%u) !iView ???=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iView );
			}
    }
void CAutolockView::MakeVisible(TBool aVisibility)
    {
    #if defined(_DEBUG)
    RDebug::Printf( "%s %s (%u) aVisibility=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aVisibility );
    #endif   
    if (iView)
	iView->MakeVisible( aVisibility );
    else
	{
	#if defined(_DEBUG)
	RDebug::Printf( "%s %s (%u) !iView ???=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iView );
	#endif   
	}
    #if defined(_DEBUG)
    RDebug::Printf( "%s %s (%u) aVisibility=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aVisibility );
    #endif
    }
// ----------------------------------------------------
// CAutolockView::HandleStatusPaneSizeChange()
// Handles StatusPane Size Change
// ----------------------------------------------------
//
void CAutolockView::HandleStatusPaneSizeChange()
	{
	#if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockView::HandleStatusPaneSizeChange()"));
    #endif
    if (iView)
        {
        #if defined(_DEBUG)
        RDebug::Print(_L("(AUTOLOCK)CAutolockView::HandleStatusPaneSizeChange() SET RECT"));
        #endif
        TRect cr = ClientRect();
        iView->SetRect( cr );
        }
    #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockView::HandleStatusPaneSizeChange() END"));
    #endif
	}
// ----------------------------------------------------
// CAutolockView::DoActivateL
// Activates the view
// ----------------------------------------------------
//
void CAutolockView::DoActivateL(const TVwsViewId& /*aPrevViewId*/,TUid aCustomMessageId,const TDesC8& /*aCustomMessage*/)
	{
	#if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockView::DoActivateL"));
    #endif	
	iView = new(ELeave) CAutolockContainer;

    TRect screen( iAvkonAppUi->ApplicationRect() );
    TAknLayoutRect applicationWindow;
    applicationWindow.LayoutRect(screen, AknLayoutScalable_Avkon::application_window(0));
    TInt mainPaneVariety = 0;
    if((AknLayoutUtils::PenEnabled()) && !(Layout_Meta_Data::IsLandscapeOrientation()))
        {//Use main pane without softkey area
            mainPaneVariety = 5;
        }
    else if (Layout_Meta_Data::IsLandscapeOrientation())
        {
            mainPaneVariety = 4;
        }
    else
        {
            mainPaneVariety = 3;
        }

    
    
    TAknLayoutRect mainPane;
    mainPane.LayoutRect(applicationWindow.Rect(), AknLayoutScalable_Avkon::main_pane(mainPaneVariety));     	
	iView->ConstructL( mainPane.Rect() );
    AppUi()->AddToStackL(*this,iView);

    // Message comes from start-up. We need to lock the device.
    if ( aCustomMessageId == KUidStartUp )
        {
        static_cast<CAutolockAppUi*>(AppUi())->Model()->LockSystemL();

        // Set phone app window group to position 2. This is the same position 
        // as the phone app set is self when it goes to background.
        TVwsViewId phoneAppId;
        TApaTaskList taskList( iEikonEnv->WsSession() );
        User::LeaveIfError( AknDef::GetPhoneIdleViewId(phoneAppId) );
        const TInt phoneWgId = taskList.FindApp( phoneAppId.iAppUid ).WgId();
        User::LeaveIfError( iCoeEnv->WsSession().SetWindowGroupOrdinalPosition( phoneWgId, KPhoneAppOrdinalPosition ) );
        }
    #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockView::DoActivateL END"));
    #endif
    }
// ----------------------------------------------------
// CAutolockView::DoDeActivateL
// Deactivates the view
// ----------------------------------------------------
//
void CAutolockView::DoDeactivate()
	{
	#if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockView::DoDeActivateL"));
    #endif
    CAutolockContainer* view = iView;
    iView = NULL;
	if (view)
        {
		AppUi()->RemoveFromStack(view);
        }
	delete view;
    }
// ----------------------------------------------------
// CAutolockView::ScreenDeviceChanged()
// Handles screen layout changes
// ----------------------------------------------------
//
void CAutolockView::ScreenDeviceChanged()
{
 if (Layout_Meta_Data::IsLandscapeOrientation() && AknLayoutUtils::PenEnabled())
        {//do not change layout in touch device to conserve battery
        #if defined(_DEBUG)
    		RDebug::Print(_L("(AUTOLOCK)CAutolockView::ScreenDeviceChanged(): Do Nothing"));
    		#endif
        return;
        }
        
 if (iView)
    {
    TRect screen( iAvkonAppUi->ApplicationRect() );
    TAknLayoutRect applicationWindow;
    applicationWindow.LayoutRect(screen, AknLayoutScalable_Avkon::application_window(0));

    TInt mainPaneVariety = 0;
    if((AknLayoutUtils::PenEnabled()) && !(Layout_Meta_Data::IsLandscapeOrientation()))
        {//Use main pane without softkey area
            mainPaneVariety = 5;
        }
    else if (Layout_Meta_Data::IsLandscapeOrientation())
        {
            mainPaneVariety = 4;
        }
    else
        {
            mainPaneVariety = 3;
        }
    #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockView::ScreenDeviceChanged() variety: %d"), mainPaneVariety);
    #endif
    TAknLayoutRect mainPane;
    mainPane.LayoutRect(applicationWindow.Rect(), AknLayoutScalable_Avkon::main_pane(mainPaneVariety));     
    iView->SetRect(mainPane.Rect());        
    }
}
// end of file

