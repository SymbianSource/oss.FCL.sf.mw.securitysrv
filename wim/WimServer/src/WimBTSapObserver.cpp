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
* Description:  Observes the state of Bluetooth Sap. When the state is
*               changed into connected, connection to smart card is closed and
*               when the state is changed into not connected, initialize
*               connection to smart card back.
*
*/



// INCLUDE FILES
#include    "WimBTSapObserver.h"
#include    "WimServer.h"
#include    "WimSession.h"
#include    "WimTrace.h"

#if defined BT_SAP_TEST_BY_CHARGER
#include    <PSVariables.h>
#else
//#include    <BTSapInternalPSKeys.h>         // Property values
#include    <BTSapDomainPSKeys.h>
#endif // BT_SAP_TEST_BY_CHARGER


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWimBTSapObserver::CWimBTSapObserver
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CWimBTSapObserver::CWimBTSapObserver()
    : CActive( EPriorityNormal )
    {
    _WIMTRACE(_L("WIM|CWimBTSapObserver::CWimBTSapObserver|Begin"));
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CWimBTSapObserver::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CWimBTSapObserver::ConstructL()
    {
    _WIMTRACE(_L("WIM|CWimBTSapObserver::ConstructL|Begin"));
    iWimSvr = CWimServer::Server();
    }

// -----------------------------------------------------------------------------
// CWimBTSapObserver::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CWimBTSapObserver* CWimBTSapObserver::NewL()
    {
    _WIMTRACE(_L("WIM|CWimBTSapObserver::NewL|Begin"));
    CWimBTSapObserver* self = new( ELeave ) CWimBTSapObserver ();
    
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    return self;
    }

    
// Destructor
CWimBTSapObserver::~CWimBTSapObserver()
    {
    _WIMTRACE(_L("WIM|CWimBTSapObserver::~CWimBTSapObserver|Begin"));
    Cancel();
    iProperty.Close();
    }

// -----------------------------------------------------------------------------
// CWimBTSapObserver::Start
// Start listening publish and subscribe events
// -----------------------------------------------------------------------------
//
void CWimBTSapObserver::Start()
    {
    _WIMTRACE(_L("WIM|CWimBTSapObserver::Start|Begin"));
    if ( !IsActive() ) // If already active, don't activate
        {
        StartListening(); //Start listening
        }    
    }

// -----------------------------------------------------------------------------
// CWimBTSapObserver::StartListening
// Start listening card status from publish and subscribe
// -----------------------------------------------------------------------------
//
void CWimBTSapObserver::StartListening()
    {
    _WIMTRACE(_L("WIM|CWimBTSapObserver::StartListening|Begin"));
    iStatus = KRequestPending;
#if defined BT_SAP_TEST_BY_CHARGER
    TInt err = iProperty.Attach( KUidSystemCategory, 
                                 KPSUidChargerStatusValue );
 
#else
    TInt err = iProperty.Attach( KPSUidBluetoothSapConnectionState,
                                 KBTSapConnectionState );
#endif // BT_SAP_TEST_BY_CHARGER
    if ( err == KErrNone )
        {
        iProperty.Subscribe( iStatus );
        SetActive();
        }
    }

// -----------------------------------------------------------------------------
// CWimBTSapObserver::RunL
// Got event from publish and subscribe
// -----------------------------------------------------------------------------
//
void CWimBTSapObserver::RunL()
    {
    _WIMTRACE(_L("WIM|CWimBTSapObserver::RunL|Begin"));
    //Notify on bluetooth event
    TInt btState;
    iProperty.Get( btState );
    switch ( btState )
        {
#if defined BT_SAP_TEST_BY_CHARGER
        case EPSChargerStatusUninitialized: // Flow through
        case EPSChargerDisconnected:        // Flow through
        case EPSChargerNotCharging: // Charger not charging, no actions
#else
        case EBTSapNotConnected:    // Flow through
        case EBTSapConnecting:      // BT Sap not connected, no actions
#endif // BT_SAP_TEST_BY_CHARGER
            {
            StartListening();
            break;
            }
#if defined BT_SAP_TEST_BY_CHARGER
        case EPSChargerConnected:
#else
        case EBTSapConnected:
#endif // BT_SAP_TEST_BY_CHARGER
            {
            if ( CWimServer::iWimInitialized )
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

                sessions.Close();
                }
            else
                {
                _WIMTRACE(_L("WIM | CWimSatRefreshObserver::Refresh | WIMI already closed."));
                }
            }
        }
    _WIMTRACE(_L("WIM|CWimBTSapObserver::RunL|End"));
    }

// -----------------------------------------------------------------------------
// CWimBTSapObserver::DoCancel
// Cancel asynchronous request
// -----------------------------------------------------------------------------
//
void CWimBTSapObserver::DoCancel()
    {
    _WIMTRACE(_L("WIM|CWimBTSapObserver::DoCancel|Begin"));
    iProperty.Cancel();
     _WIMTRACE(_L("WIM|CWimBTSapObserver::DoCancel|End"));
    }

//  End of File  
