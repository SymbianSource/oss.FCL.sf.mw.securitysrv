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
* Description:  Timer class
*
*/



// INCLUDE FILES
#include    "WimTimer.h"
#include    "WimConsts.h"
#include    "WimTrace.h"


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWimTimer::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CWimTimer* CWimTimer::NewL( MWimTimerListener* aTimerListener )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTimer::NewL | Begin"));
    CWimTimer *self = new( ELeave ) CWimTimer;
    CleanupStack::PushL( self );
    self->ConstructL( aTimerListener );
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CWimTimer::CWimTimer
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CWimTimer::CWimTimer() : CActive( EPriorityStandard ), iHeartBeatCount( 0 )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTimer::CWimTimer | Begin"));
    }

// Destructor
CWimTimer::~CWimTimer()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTimer::~CWimTimer | Begin"));
    Cancel();
    iTimer.Close();
    delete iHeartbeat;
    }

// -----------------------------------------------------------------------------
// CWimTimer::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CWimTimer::ConstructL( MWimTimerListener* aTimerListener )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTimer::ConstructL | Begin"));
    User::LeaveIfError( iTimer.CreateLocal() );
    iTimerListener = aTimerListener;
    CActiveScheduler::Add( this );
    iDelay = 0;
    iHeartbeat = CHeartbeat::NewL( 0 ); // neutral priority
    }

// -----------------------------------------------------------------------------
// CWimTimer::SetCloseAfter
// Set timeout of timer, delay given in seconds
// -----------------------------------------------------------------------------
//
void CWimTimer::SetCloseAfter( const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTimer::SetCloseAfter | Begin"));
    TInt closeAfter = aMessage.Int0();

    // Check that given value is in limits
    if ( closeAfter < KWimNoTimeout || closeAfter > KWimTimeoutMax )
        {
        aMessage.Complete( KErrArgument );
        }
    else // Value is OK, set timer
        {
        DoSetCloseAfter( closeAfter );
        aMessage.Complete( KErrNone );
        }
    }

// -----------------------------------------------------------------------------
// CWimTimer::SetCloseAfter
// Set timeout of timer, delay given in seconds
// -----------------------------------------------------------------------------
//
void CWimTimer::DoSetCloseAfter( TInt aCloseAfter )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTimer::DoSetCloseAfter | Begin"));
    if ( IsActive() ) // Timeout already set, cancel
        {
        Cancel();
        }
    if ( aCloseAfter == KWimNoTimeout ) // No timeout, cancel timer
        {
        Cancel();
        iDelay = KWimNoTimeout;
        }
    else
        {
        iDelay = aCloseAfter * 1000 * 1000; // Timer takes delay in microseconds
        iTimer.After( iStatus, iDelay );
        SetActive();
        StartHeartBeat();
        }
    }
// -----------------------------------------------------------------------------
// CWimTimer::GetCloseAfterL
// Return delay given in SetCloseAfter in seconds
// If no timeout is set return -1 (KWimNoTimeout)
// -----------------------------------------------------------------------------
//
void CWimTimer::GetCloseAfterL( const RMessage2& aMessage ) const
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTimer::Delay | Begin"));

    TInt delay = 0;

    if ( iDelay.Int() == KWimNoTimeout )
        {
        delay = KWimNoTimeout;
        }
    else
        {
        // Timer takes delay in microseconds
        delay = iDelay.Int() / ( 1000 * 1000 );
        }

    TPckgBuf<TInt> closeAfterPckg( delay  );
    aMessage.WriteL( 0, closeAfterPckg );
    aMessage.Complete( KErrNone );
    }

// -----------------------------------------------------------------------------
// CWimTimer::RunL()
// Handles an active object’s request completion event
// -----------------------------------------------------------------------------
//
void CWimTimer::RunL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTimer::RunL | Begin"));
    if ( iTimerListener )
        {
        iTimerListener->TimerExpired();
        }
    }

// -----------------------------------------------------------------------------
// CWimTimer::DoCancel
// Cancellation of an outstanding request
// -----------------------------------------------------------------------------
//
void CWimTimer::DoCancel()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTimer::DoCancel | Begin"));
    iTimer.Cancel();
    iHeartbeat->Cancel();
    }

// -----------------------------------------------------------------------------
// CWimTimer::ResetTimer
// Resets active timer.
// -----------------------------------------------------------------------------
//
void CWimTimer::ResetTimer()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTimer::ResetTimer | Begin"));
    if ( IsActive() )
        {
        Cancel();
        iTimer.After( iStatus, iDelay );
        SetActive();
        iHeartbeat->Cancel();
        StartHeartBeat();
        }
    }

// -----------------------------------------------------------------------------
// CWimTimer::TimeRemainingL
// Returns remaining time of the timer in seconds. If timer has no timeout
// then KWimNoTimeout (-1) is returned.
// -----------------------------------------------------------------------------
//
void CWimTimer::TimeRemainingL( const RMessage2& aMessage  ) const
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTimer::TimeRemainingL | Begin"));

    TInt timeRemaining;

    if ( iDelay.Int() == KWimNoTimeout )
        {
        timeRemaining = KWimNoTimeout;
        }
    else
        {
        // Timer takes delay in microseconds
        TInt delay = iDelay.Int() / ( 1000 * 1000 );
        timeRemaining = delay - iHeartBeatCount;
        }

    TPckgBuf<TInt> timeRemainingPckg( timeRemaining  );
    aMessage.WriteL( 0, timeRemainingPckg );
    aMessage.Complete( KErrNone );
    }
    
// -----------------------------------------------------------------------------
// CWimTimer::TimeRemaining
// Used inside wimserver
// -----------------------------------------------------------------------------
//
TInt CWimTimer::TimeRemaining() const
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTimer::TimeRemaining | Begin"));

    TInt timeRemaining;

    if ( iDelay.Int() == KWimNoTimeout )
        {
        timeRemaining = KWimNoTimeout;
        }
    else
        {
        // Timer takes delay in microseconds
        TInt delay = iDelay.Int() / ( 1000 * 1000 );
        timeRemaining = delay - iHeartBeatCount;
        }

    return timeRemaining; 
    }

// -----------------------------------------------------------------------------
// CWimTimer::StartHeartBeat
// Start heart beating
// -----------------------------------------------------------------------------
//
void CWimTimer::StartHeartBeat()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTimer::StartHeartBeat | Begin"));
    iHeartbeat->Start( ETwelveOClock, this ); // 1 second intervals
    }

// -----------------------------------------------------------------------------
// CWimTimer::Beat
// Beat has occurred, increase counter
// -----------------------------------------------------------------------------
//
void CWimTimer::Beat()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTimer::Beat | Begin"));
    iHeartBeatCount++;
    }

// -----------------------------------------------------------------------------
// CWimTimer::Synchronize
// Synchronize the timer.
// -----------------------------------------------------------------------------
//
void CWimTimer::Synchronize()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTimer::Synchronize | Begin"));
    }

//  End of File
