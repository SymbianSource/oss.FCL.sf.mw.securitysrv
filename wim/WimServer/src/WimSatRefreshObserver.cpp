/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Observes 'SIM file changed' events from SAT. If changed file
*               is WIM related, WimiLib's cache is cleared and WimiLib closed.
*               Prevents WIM from operating with out-dated cache.
*
*/


// INCLUDE FILES
#include "WimSatRefreshObserver.h"
#include "WimServer.h"
#include "WimSession.h"
#include "WimTrace.h"
#include <rsatrefresh.h>
#include <rsatsession.h>
#include <etelsat.h>
#if defined SAT_REFRESH_TIMER_TRIGGER
#include "SatRefreshTimerTrigger.h" // for simulated sat refresh
#endif

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWimSatRefreshObserver::CWimSatRefreshObserver
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CWimSatRefreshObserver::CWimSatRefreshObserver()
    {
    _WIMTRACE(_L("WIM | CWimSatRefreshObserver | C'tor"));
    }

// -----------------------------------------------------------------------------
// CWimSatRefreshObserver::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CWimSatRefreshObserver::ConstructL()
    {
    _WIMTRACE(_L("WIM | CWimSatRefreshObserver::ConstructL | Begin"));

    iWimSvr = CWimServer::Server();

#if defined SAT_REFRESH_TIMER_TRIGGER
    // simulate SAT file changed events
    iTimerTrigger = CSatRefreshTimerTrigger::NewL( this );
    iTimerTrigger->Start();
    _WIMTRACE(_L("WIM | CWimSatRefreshObserver::ConstructL | Using timer triggered SAT refresh."));
#else
    // real implementation
    iRefreshClient = new( ELeave ) RSatRefresh( *this );
    _WIMTRACE(_L("WIM | CWimSatRefreshObserver::ConstructL | iRefreshClient"));
    iSatSession = new( ELeave ) RSatSession;
    _WIMTRACE(_L("WIM | CWimSatRefreshObserver::ConstructL | iSatSession"));
    iSatSession->ConnectL();
    _WIMTRACE(_L("WIM | CWimSatRefreshObserver::ConstructL | ConnectL"));
    iRefreshClient->OpenL( *iSatSession );
    _WIMTRACE(_L("WIM | CWimSatRefreshObserver::ConstructL | OpenL"));
    iRefreshClient->NotifyFileChangeL(); // start receiving notifications
    _WIMTRACE(_L("WIM | CWimSatRefreshObserver::ConstructL | NotifyFileChangeL"));
#endif // SAT_REFRESH_TIMER_TRIGGER
    _WIMTRACE(_L("WIM | CWimSatRefreshObserver::ConstructL | End"));
    }

// -----------------------------------------------------------------------------
// CWimSatRefreshObserver::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CWimSatRefreshObserver* CWimSatRefreshObserver::NewL()
    {
    _WIMTRACE(_L("WIM | CWimSatRefreshObserver::NewL | Begin"));
    CWimSatRefreshObserver* self = new( ELeave ) CWimSatRefreshObserver();

    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    _WIMTRACE(_L("WIM | CWimSatRefreshObserver::NewL | End"));
    return self;
    }


// -----------------------------------------------------------------------------
// CWimSatRefreshObserver::~CWimSatRefreshObserver
// Destructor.
// -----------------------------------------------------------------------------
//
CWimSatRefreshObserver::~CWimSatRefreshObserver()
    {
    _WIMTRACE(_L("WIM | CWimSatRefreshObserver | D'tor"));
    if ( iRefreshClient )
        {
        iRefreshClient->Close();
        delete iRefreshClient;
        }
    if ( iSatSession )
        {
        iSatSession->Close();
        delete iSatSession;
        }
#if defined SAT_REFRESH_TIMER_TRIGGER
    if ( iTimerTrigger )
        {
        delete iTimerTrigger;
        }
#endif
    }


// -----------------------------------------------------------------------------
// CWimSatRefreshObserver::AllowRefresh
// Query from SAT: Is it OK to refresh files on SIM.
// -----------------------------------------------------------------------------
//
TBool CWimSatRefreshObserver::AllowRefresh( TSatRefreshType /*aType*/,
                                            const TSatRefreshFiles& /*aFiles*/ )
    {
    _WIMTRACE(_L("WIM | CWimSatRefreshObserver::AllowRefresh | Begin"));

    TBool allowRefresh( ETrue ); // allow by default
    
    if ( iWimSvr->IsAccessingToken()) // check if we are accessing card
        {
        allowRefresh = EFalse; // deny if we are accessing card
        }

    _WIMTRACE2(_L("WIM | CWimSatRefreshObserver::AllowRefresh | End | allowRefresh=%d"), allowRefresh);
    return allowRefresh;
    }

// -----------------------------------------------------------------------------
// CWimSatRefreshObserver::Refresh
// Notification of completed refresh event from SAT.
//
// -----------------------------------------------------------------------------
//
void CWimSatRefreshObserver::Refresh( TSatRefreshType /*aType*/,
                                      const TSatRefreshFiles& /*aFiles*/ )
    {
    _WIMTRACE(_L("WIM | CWimSatRefreshObserver::Refresh | Begin"));

    iRefreshClient->RefreshEFRead( EFalse );
    // WIM is doing stg with the card -> postpone WIMI closing.
    if ( iWimSvr->IsAccessingToken() )
        {
        // Tell server to re-initialize at earliest convenience.
        iWimSvr->SetRefreshNotificationReceived( ETrue );
        _WIMTRACE(_L("WIM | CWimSatRefreshObserver::Refresh | WIMI closing postponed."));
        }
    else if ( CWimServer::iWimInitialized )
        {
        // Wim initialized, can close WIMI immediately.
        // NOTE: Is it ok to run lengthy operations here?
        RArray<CWimSession*> sessions;
        TRAPD( err, iWimSvr->GetSessionsL( sessions ) );

        if ( !err )  // Got sessions correctly
            {
            // Loop through all sessions and notify all clients that are
            // requesting the notification
            for ( TInt i( 0 ); i < sessions.Count(); i++ )
                {
                sessions[i]->NotifyComplete();
                }
            sessions.Reset();
            // Close WIMI
            WIMI_CloseDownReq();
            _WIMTRACE(_L("WIM | CWimSatRefreshObserver::Refresh | WIMI closed."));
            }
        }
    else
        {
        _WIMTRACE(_L("WIM | CWimSatRefreshObserver::Refresh | WIMI already closed."));
        }

    _WIMTRACE(_L("WIM | CWimSatRefreshObserver::Refresh | End"));
    }

//  End of File
