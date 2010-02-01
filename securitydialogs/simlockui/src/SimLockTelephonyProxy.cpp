/*
* ============================================================================
*  Name        : SimLockTelephonyProxy.cpp
*  Part of     : Sim Lock UI Telephony Proxy
*  Description : Wrap asynchronous calls to Core Telephony
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
#include <Etel3rdParty.h>           // CTelephony

// User Includes
#include "SimLockTelephonyProxy.h"
#include "SimLockUi.pan"


TInt KSimLockProxyTimeout = 5000000;    // 5 seconds

// ---------------------------------------------------------------------------
// CSimLockDataHandlingDelegate::NewL
// ---------------------------------------------------------------------------
CSimLockTelephonyProxy* CSimLockTelephonyProxy::NewL()
    {
    CSimLockTelephonyProxy* self = new ( ELeave ) CSimLockTelephonyProxy();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// CSimLockTelephonyProxy::CSimLockTelephonyProxy
// ---------------------------------------------------------------------------
CSimLockTelephonyProxy::~CSimLockTelephonyProxy()
    {
    Cancel();
    delete iTelephony;
    delete iSchedulerWait;
    delete iTimer;
    }

// ---------------------------------------------------------------------------
// CSimLockTelephonyProxy::IsCallInProgress
// ---------------------------------------------------------------------------
TBool CSimLockTelephonyProxy::IsCallInProgress()
    {
    iTelephony->GetIndicator( iStatus, iIndicatorPackage );    
    CompleteRequestWithTimeout();

    if ( iStatus != KErrNone )
        {
        // If there is an error obtaining status, assume no call in progress
        }
    else if ( iIndicators.iIndicator & CTelephony::KIndCallInProgress )
        {
        return ETrue;
        }

    return EFalse;
    }

// ---------------------------------------------------------------------------
// CSimLockTelephonyProxy::RunL
// ---------------------------------------------------------------------------
void CSimLockTelephonyProxy::RunL()
    {    
    // Stop the current run sequence so we can continue execution in a
    // synchronous fashion
    iSchedulerWait->AsyncStop();
    }

// ---------------------------------------------------------------------------
// CSimLockTelephonyProxy::DoCancel
// ---------------------------------------------------------------------------
void CSimLockTelephonyProxy::DoCancel()
    {
    // Cancel outstanding request
    iTelephony->CancelAsync( CTelephony::EGetIndicatorCancel );
    iSchedulerWait->AsyncStop();
    }

// ---------------------------------------------------------------------------
// CSimLockTelephonyProxy::CSimLockTelephonyProxy
// ---------------------------------------------------------------------------
CSimLockTelephonyProxy::CSimLockTelephonyProxy()
    : CActive( EPriorityStandard ),
    iIndicatorPackage( iIndicators )
    {
    }

// ---------------------------------------------------------------------------
// CSimLockTelephonyProxy::ConstructL
// ---------------------------------------------------------------------------
void CSimLockTelephonyProxy::ConstructL()
    {
    CActiveScheduler::Add( this );
    iSchedulerWait = new ( ELeave ) CActiveSchedulerWait;
    iTelephony = CTelephony::NewL();    
    iTimer = CPeriodic::NewL(EPriorityHigh);
    }

// ---------------------------------------------------------------------------
// CSimLockTelephonyProxy::TimerElapsed
// ---------------------------------------------------------------------------
TInt CSimLockTelephonyProxy::TimerElapsed(TAny* aClientObject)
    {
    CSimLockTelephonyProxy* clientObject = static_cast<CSimLockTelephonyProxy*>(aClientObject);
    
    // Timeout timer has elapsed.  An asynchronous request timed out.            
    ASSERT(0);

    // Cancel original request
    clientObject->Cancel();        
    return 0;
    }

// ---------------------------------------------------------------------------
// CSimLockTelephonyProxy::CompleteRequestWithTimeout
// ---------------------------------------------------------------------------
void CSimLockTelephonyProxy::CompleteRequestWithTimeout()
    {
                
    if ( iTimer->IsActive() )
        {
        ASSERT(0);
        iTimer->Cancel();
        }    
        
    // Start timer with KSimLockProxyTimeout to protect against requests that
    // do not complete for some reason.
    iTimer->Start(KSimLockProxyTimeout,0,TCallBack(&TimerElapsed, this));  
        
    SetActive();

    // Wait for request to complete.  Response time is expected to be negligible.
    iSchedulerWait->Start();        
    
    iTimer->Cancel();        
    }

// end of file

