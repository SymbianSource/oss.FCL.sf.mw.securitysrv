/*
* Copyright (c) 2003 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Observer class for System Agent events
*
*/



// INCLUDE FILES
#include    "SwimSysAgentObserver.h"
#include    "SwimReaderIF.h"
#include    "WimTrace.h"

#ifdef RD_STARTUP_CHANGE
#include    <startupdomainpskeys.h> // Property values
#else
#include    <PSVariables.h>         // Property values
#endif


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CSwimSysAgentObserver::CSwimSysAgentObserver
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CSwimSysAgentObserver::CSwimSysAgentObserver( CSwimReaderIF* aReaderIF )
    : CActive( EPriorityNormal )
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimSysAgentObserver::CSwimSysAgentObserver|Begin"));
    CActiveScheduler::Add( this );
    iReaderIF = aReaderIF;
    }

// -----------------------------------------------------------------------------
// CSwimSysAgentObserver::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CSwimSysAgentObserver::ConstructL()
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimSysAgentObserver::ConstructL|Begin"));
    }

// -----------------------------------------------------------------------------
// CSwimSysAgentObserver::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSwimSysAgentObserver* CSwimSysAgentObserver::NewL( CSwimReaderIF* aReaderIF )
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimSysAgentObserver::NewL|Begin"));
    CSwimSysAgentObserver* self = new( ELeave ) CSwimSysAgentObserver ( aReaderIF );
    
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    return self;
    }

    
// Destructor
CSwimSysAgentObserver::~CSwimSysAgentObserver()
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimSysAgentObserver::~CSwimSysAgentObserver|Begin"));
    Cancel();
    iProperty.Close();
    }


// -----------------------------------------------------------------------------
// CSwimSysAgentObserver::Start
// Start listening System agent events
// -----------------------------------------------------------------------------
//
void CSwimSysAgentObserver::Start()
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimSysAgentObserver::Start|Begin"));
    StartListening( ); //Start listening
    }

// -----------------------------------------------------------------------------
// CSwimSysAgentObserver::StartListening
// Start listening card status from System Agent
// -----------------------------------------------------------------------------
//
void CSwimSysAgentObserver::StartListening()
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimSysAgentObserver::StartListening|Begin"));
    iStatus = KRequestPending;
#ifdef RD_STARTUP_CHANGE
    TInt err = iProperty.Attach( KPSUidStartup, KPSSimStatus );
#else
    TInt err = iProperty.Attach( KUidSystemCategory, KPSUidSIMStatusValue );
#endif    
    if ( err == KErrNone )
        {
        iProperty.Subscribe( iStatus );
        }
    SetActive();
    }

// -----------------------------------------------------------------------------
// CSwimSysAgentObserver::RunL
// Got card state event
// -----------------------------------------------------------------------------
//
void CSwimSysAgentObserver::RunL()
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimSysAgentObserver::RunL|Begin"));
    //Notify on card event
    TInt simState;
    iProperty.Get( simState );
#ifdef RD_STARTUP_CHANGE
    switch ( simState )
          {
        case ESimUsable: //Card OK, no actions
            {
            StartListening();
            break;
            }

        // In these states, the SIM card cannot be used by WIM. Notify.
        // 
        case ESimReadable:
        case ESimNotReady:
        case ESimNotPresent:
            {
            iReaderIF->Notify( EScardRemoved ); //Notify Scard server
            StartListening(); //Start listening again
            break;
            }

        // The device does not support any SIM card. WIM cannot be used.
        //
        case ESimNotSupported:
            {
            break;
            }

        // These states are considered as fatal errors.
        //
        case ESimStatusUninitialized:
        default:
            {
            // __ASSERT_DEBUG( PANIC )
            break;
            }
        }
#else
    switch ( simState )
          {
        case EPSSimOk: //Card OK, no actions
            {
            StartListening();
            break;
            }
        case EPSSimNotPresent: //Flow trough
        case EPSSimRejected: // Card removed
            {
            iReaderIF->Notify( EScardRemoved ); //Notify Scard server
            StartListening(); //Start listening again
            break;
            }
        default:
            {
            break;
            }
        }
#endif // RD_STARTUP_CHANGE
    }


// -----------------------------------------------------------------------------
// CSwimSysAgentObserver::DoCancel
// Cancel asynchronous request
// -----------------------------------------------------------------------------
//
void CSwimSysAgentObserver::DoCancel()
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimSysAgentObserver::DoCancel|Begin"));
    iProperty.Cancel();
    }

//  End of File  
