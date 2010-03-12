/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Predefined contacts engine (state machine)
*
*/


#include <startupdomainpskeys.h>
#include "DevEncStarterEngine.h"
#include "DevEncStarterDef.h"
#include "DevEncLog.h"
#include "DevEncStarterMmcObserver.h"
#include "DevEncUids.hrh"
#include "DevEncStarterUtils.h"
#include "DevEncProtectedPSKey.h"

// ---------------------------------------------------------------------------
// CDevEncStarterEngine::NewLC
// C++ constructor
// ---------------------------------------------------------------------------
//
CDevEncStarterEngine* CDevEncStarterEngine::NewLC()
	{
	DFLOG( "CDevEncStarterAppUi::NewLC" );
	CDevEncStarterEngine* self = new( ELeave ) CDevEncStarterEngine();
	CleanupStack::PushL( self );
	self->ConstructL();
	return self;
    }

// ---------------------------------------------------------------------------
// CDevEncStarterEngine::CDevEncStarterEngine
// C++ constructor
// ---------------------------------------------------------------------------
//
CDevEncStarterEngine::CDevEncStarterEngine()
    : CActive( EPriorityNormal )
    {
    }

// ---------------------------------------------------------------------------
// CDevEncStarterEngine::ConstructL
// ---------------------------------------------------------------------------
//
void CDevEncStarterEngine::ConstructL()
    {
    DFLOG( "CDevEncStarterAppUi::ConstructL" );
    iUtils = CDevEncStarterUtils::NewL();
    iMmcObserver = CDevEncStarterMmcObserver::NewL( iUtils );
        
    const TSecurityPolicy policy( ECapability_None );
    RProperty::Define( KDevEncProtectedUid, KDevEncOperationKey, RProperty::EInt, policy, policy );
    
    iProperty.Attach( KPSUidStartup, KPSGlobalSystemState );
    CActiveScheduler::Add( this );
    
    iProperty.Subscribe( iStatus );
    SetActive();
    }

// ---------------------------------------------------------------------------
// CDevEncStarterEngine::~CDevEncStarterEngine
// Destructor
// ---------------------------------------------------------------------------
//
CDevEncStarterEngine::~CDevEncStarterEngine()
    {
    DFLOG( "CDevEncStarterAppUi::~CDevEncStarterAppUi" );
    
    delete iMmcObserver;
    delete iUtils;
    
    RProperty::Delete( KDevEncProtectedUid, KDevEncOperationKey );
    
    Cancel();
    iProperty.Close();
    }

// -----------------------------------------------------------------------------
// CDevEncStarterEngine::DoCancel
// -----------------------------------------------------------------------------
//
void CDevEncStarterEngine::DoCancel()
    {
    DFLOG( "CDevEncStarterEngine::DoCancel()" );
    iProperty.Cancel();
    }

// -----------------------------------------------------------------------------
// CDevEncStarterEngine::RunL
// -----------------------------------------------------------------------------
//
void CDevEncStarterEngine::RunL()
    {
    DFLOG( "CDevEncStarterEngine::RunL()" );
    TInt value( 0 );
    iProperty.Get( value );
    
    if( value == ESwStateShuttingDown )
        {
        CActiveScheduler::Stop();
        }
    else
        {
        iProperty.Subscribe( iStatus );
        SetActive();
        }
	}

// End of file


