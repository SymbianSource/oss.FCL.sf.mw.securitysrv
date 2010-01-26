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
* Description:  Async-Sync utility class
 *
*/


#include <e32base.h>
#include <eikenv.h>
#include <eikappui.h>
#include "lockappwait.h"

// ----------------------------------------------------------
// Standard Symbian OS construction sequence
// ----------------------------------------------------------
//
CWait* CWait::NewL( )
    {
    CWait* self = new (ELeave) CWait( );
    CleanupStack::PushL( self );
    self->ConstructL( );
    CleanupStack::Pop( self );
    return self;
    }

// ----------------------------------------------------------
// 2nd phase constructor.
// ----------------------------------------------------------
//
void CWait::ConstructL( )
    {
    CActiveScheduler::Add( this );
    }

// ----------------------------------------------------------
// Constructor
// ----------------------------------------------------------
//
CWait::CWait( ) :
    CActive(0)
    {
    // no implementation required
    }

// ----------------------------------------------------------
// Destructor.
// ----------------------------------------------------------
//
CWait::~CWait( )
    {
    Cancel( );
    }

// ----------------------------------------------------------
//
// ----------------------------------------------------------
//
TInt CWait::WaitForRequestL( )
    {
    CWaitAbsorbingControl* absorbing = CWaitAbsorbingControl::NewLC( );
    SetActive( );
    iWait.Start( );
    CleanupStack::PopAndDestroy( absorbing );
    return iStatus.Int( );
    }

// ----------------------------------------------------------
//
// ----------------------------------------------------------
//
void CWait::RunL( )
    {
    if ( iWait.IsStarted( ) )
        {
        iWait.AsyncStop( );
        }
    }

// ----------------------------------------------------------
// Cancels code request
// ----------------------------------------------------------
//
void CWait::DoCancel( )
    {
    if ( iWait.IsStarted( ) )
        {
        iWait.AsyncStop( );
        }
    }

// ----------------------------------------------------------
// Sets active request type
// ----------------------------------------------------------
//
void CWait::SetRequestType(TInt aRequestType )
    {
    iRequestType = aRequestType;
    }

// ----------------------------------------------------------
// Gets active request type
// ----------------------------------------------------------
//
TInt CWait::GetRequestType( )
    {
    return iRequestType;
    }

// ----------------------------------------------------------
// Standard Symbian OS construction sequence
// ----------------------------------------------------------
//
CWaitAbsorbingControl* CWaitAbsorbingControl::NewLC( )
    {
    CWaitAbsorbingControl* self= new (ELeave) CWaitAbsorbingControl();
    CleanupStack::PushL( self );
    self->ConstructL( );
    return self;
    }

// ----------------------------------------------------------
//
// ----------------------------------------------------------
//
CWaitAbsorbingControl::CWaitAbsorbingControl( )
    {
    // no implementation required
    }

// ----------------------------------------------------------
// Destructor.
// ----------------------------------------------------------
//
CWaitAbsorbingControl::~CWaitAbsorbingControl( )
    {
    if ( iCoeEnv && iAppUi )
        iAppUi->RemoveFromStack( this );
    }

// ----------------------------------------------------------
//
// ----------------------------------------------------------
//
void CWaitAbsorbingControl::ConstructL( )
    {
    CreateWindowL( );
    SetExtent( TPoint( 0, 0 ), TSize( 0, 0 ) );
    ActivateL( );
    SetPointerCapture( ETrue );
    ClaimPointerGrab( ETrue );
    iAppUi = iEikonEnv->EikAppUi();
    iAppUi->AddToStackL( this, ECoeStackPriorityEnvironmentFilter );
    }

// ----------------------------------------------------------
//
// ----------------------------------------------------------
//
TKeyResponse CWaitAbsorbingControl::OfferKeyEventL(const TKeyEvent& /*aKeyEvent*/, TEventCode /*aType*/)
    {
    	RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 1 );
    return EKeyWasConsumed;
    }

// End of file
