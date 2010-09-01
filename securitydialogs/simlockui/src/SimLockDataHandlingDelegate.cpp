/*
* ============================================================================
*  Name        : SimLockDataHandlingDelegate.cpp
*  Part of     : Sim Lock UI Application
*  Description : Implementation of Sim Lock UI Application
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

// System Include Files
#include <rmmcustomapi.h>           // RMmCustomAPI, RMobilePhone

// User Include Files
#include "simlockdatahandlingdelegate.h"
#include "simlockui.pan"
#include "simlockisaserverdefinitions.h"


TInt KSimLockTimeoutDelay = 5000000;       // 5 seconds

// ===========================================================================
// PUBLIC CONSTRUCTORS
// ===========================================================================

// ---------------------------------------------------------------------------
// CSimLockDataHandlingDelegate::NewL
// ---------------------------------------------------------------------------
CSimLockDataHandlingDelegate* CSimLockDataHandlingDelegate::NewL( RMmCustomAPI& aCustomAPI )
    {
    CSimLockDataHandlingDelegate* self = new ( ELeave ) CSimLockDataHandlingDelegate( aCustomAPI );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// CSimLockDataHandlingDelegate::~CSimLockDataHandlingDelegate
// ---------------------------------------------------------------------------
CSimLockDataHandlingDelegate::~CSimLockDataHandlingDelegate()
    {
    Cancel();
    delete iTimer;
    delete iSchedulerWait;
    }

// ===========================================================================
// PUBLIC MEMBER FUNCTIONS
// ===========================================================================

// ---------------------------------------------------------------------------
// CSimLockDataHandlingDelegate::OpenSimLock
// ---------------------------------------------------------------------------
TInt CSimLockDataHandlingDelegate::OpenSimLock( const TDesC& aPassword )
    {
    // Use ETel custom API to deactive SimLock
    return iCustomAPI.DeActivateSimLock( aPassword, RMmCustomAPI::EOperator );
    }

// ---------------------------------------------------------------------------
// CSimLockDataHandlingDelegate::IsSimLockOpen
// ---------------------------------------------------------------------------
TBool CSimLockDataHandlingDelegate::IsSimLockOpen() const
    {

    ASSERT( iDataHasBeenRead );

    return iLockIsOpen;
    }


// ===========================================================================
// PRIVATE CONSTRUCTORS
// ===========================================================================

// ---------------------------------------------------------------------------
// CSimLockDataHandlingDelegate::CSimLockDataHandlingDelegate
// ---------------------------------------------------------------------------
CSimLockDataHandlingDelegate::CSimLockDataHandlingDelegate( RMmCustomAPI& aCustomAPI )
:   CActive( EPriorityStandard ),
    iCustomAPI( aCustomAPI )
    { //lint !e1403 iIndicators initialized in CBase ctor
    }

// ---------------------------------------------------------------------------
// CSimLockDataHandlingDelegate::ConstructL
// ---------------------------------------------------------------------------
void CSimLockDataHandlingDelegate::ConstructL()
    {
    CActiveScheduler::Add(this);
    iSchedulerWait = new ( ELeave ) CActiveSchedulerWait;
    iTimer = CPeriodic::NewL(EPriorityHigh);
    }

// ===========================================================================
// PRIVATE MEMBER FUNCTIONS
// ===========================================================================

// ---------------------------------------------------------------------------
// CSimLockDataHandlingDelegate::RunL
// ---------------------------------------------------------------------------
void CSimLockDataHandlingDelegate::RunL()
    {

    // Stop the current run sequence
    iSchedulerWait->AsyncStop();
    }

// ---------------------------------------------------------------------------
// CSimLockDataHandlingDelegate::DoCancel
// ---------------------------------------------------------------------------
void CSimLockDataHandlingDelegate::DoCancel()
    {
  
    // Cancel timer
    iTimer->Cancel();

    // Stop the current run sequence so we can continue execution in a
    // synchronous fashion
    iSchedulerWait->AsyncStop();
    }

// ---------------------------------------------------------------------------
// CSimLockTelephonyProxy::TimerElapsed
// ---------------------------------------------------------------------------
TInt CSimLockDataHandlingDelegate::TimerElapsed(TAny* /*aUnused*/)
    {        
    // Some request did not complete while reading Sim Lock data.
    // This is not expected, so Panic.
    Panic( ESimLockUIUnableToReadSimLock );
    
    return 0;
    }

// ---------------------------------------------------------------------------
// CSimLockDataHandlingDelegate::CompleteRequestWithTimeout
// ---------------------------------------------------------------------------
void CSimLockDataHandlingDelegate::CompleteRequestWithTimeout()
    {
    
    ASSERT( ! iTimer->IsActive() );
    
    if ( ! iTimer->IsActive() )
        {        
        iTimer->Cancel();
        }
        
    // Start timer with KSimLockProxyTimeout to protect against requests that
    // do not complete for some reason.
    iTimer->Start(KSimLockTimeoutDelay,0,TCallBack(&TimerElapsed));              
        
    SetActive();

    // Wait for request to complete.  Response time is expected to be negligible.
    iSchedulerWait->Start();
    iTimer->Cancel();
    }


// end of file.

