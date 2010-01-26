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
* Description:  LockApp server session requested by lockclient
 *
*/


#include "lockapptrace.h"
#include "lockappsession.h"
#include "lockappappui.h"
#include "lockappstatecontrolinterface.h"
#include "lockapputils.h"
#include <lockappclientserver.h>

// ---------------------------------------------------------------------------
// Default Constructor.
// ---------------------------------------------------------------------------
CLockAppSession::CLockAppSession( )
    {

    // no implementation required
    }

// ---------------------------------------------------------------------------
// Default Destructor.
// ---------------------------------------------------------------------------
CLockAppSession::~CLockAppSession( )
    {
    // no implementation required
    }

// ---------------------------------------------------------------------------
// From @c CApaAppServiceBase. Handles possible error in the lock app session
// Method is empty.
// ---------------------------------------------------------------------------
void CLockAppSession::ServiceError( const RMessage2& aMessage, TInt aError )
    {

    if ( aError == KErrNotReady )
        {
        // initialization not yet ready
        aMessage.Complete( KErrNotReady );
        ERROR( aError, "LockApp initialization not yet ready, KErrNotReady" );
        }
    else
        {
        ERROR_1( aError, "Service request has caused a leave, message: %d", aMessage.Function());
        CApaAppServiceBase::ServiceError( aMessage, aError );
#ifdef _DEBUG
        if ( aMessage.Function( )== ELockAppTestDestruct )
            {
            User::Leave( KErrGeneral );
            }
#endif
        }
    }

// ---------------------------------------------------------------------------
// From @c CApaAppServiceBase. Handles a client request. Leaving is handled by
// CLockAppSession::ServiceError() which reports the error code to the client
// ---------------------------------------------------------------------------
void CLockAppSession::ServiceL(const RMessage2& aMessage )
    {  
		RThread clientThread;
		TInt err=aMessage.Client(clientThread);
		// RDebug::Print( _L("called by %S"), &(clientThread.FullName()) );
		// RDebug::Print( clientThread.FullName() );

    TInt response = KErrNone;
    // log the message
    // whole provided server API is implemented here

    switch ( aMessage.Function( ) )
        {
        case ELockAppEnableKeyguard:
            // enable with or without note (depends on first parameter)
            response = DoEnableKeyguardL( aMessage.Int0() );
            break;
        case ELockAppDisableKeyguard:
            // disable with or without note (depends on first parameter)
            response = DoDisableKeyguardL( aMessage.Int0() );
            break;
        case ELockAppEnableDevicelock:
            // check that client has needed capabilities to enable devicelock
            if ( aMessage.HasCapability( ECapabilityWriteDeviceData ) )
                {
                // first parameter contains the reason
                response = DoEnableDevicelockL( (TDevicelockReason) aMessage.Int0() );
                }
            else
                {
                response = KErrPermissionDenied;
                }
            break;
        case ELockAppDisableDevicelock:
            // check that client has needed capabilities to disable devicelock
            if ( aMessage.HasCapability( ECapabilityWriteDeviceData ) )
                {
                response = DoDisableDevicelockL( );
                }
            else
                {
                response = KErrPermissionDenied;
                }
            break;
        case ELockAppOfferKeyguard:
            // offer to lock keyguard
            response = DoOfferKeyguardL( );
            break;
        case ELockAppOfferDevicelock:
            // TODO implement this someday if needed
            response = KErrNotSupported;
            break;
        case ELockAppShowKeysLockedNote:
            response = DoShowKeysLockedNoteL( );
            break;
#ifdef _DEBUG
        case ELockAppTestDestruct:
            // test for leave
            User::Leave( KErrGeneral );
            break;
        case ELockAppTestInternal:
            // test internal functions (which cannot be tested trough client API)
            response = StateControlL()->ExecuteInternalTest( );
            break;
#endif
        default:
            // illegal message call, panic the client process
            PanicClient( aMessage, ELockPanicIllegalMessage );
            return;
        }

    INFO_2( "CLockAppSession received service message: %d. Result: %d", aMessage.Function(), response );
    aMessage.Complete( response );
    }

// ---------------------------------------------------------------------------
// Request lockapp to enable keyguard on device (might not be approved, e.g.
// devicelock activated).
// ---------------------------------------------------------------------------
TInt CLockAppSession::DoEnableKeyguardL( TBool aShowNote )
    {
    // calls the main control

    return StateControlL()->EnableKeyguardL( aShowNote );
    }

// ---------------------------------------------------------------------------
// Request lockapp to disable keyguard on device (might not be approved, e.g.
// already disabled).
// ---------------------------------------------------------------------------
TInt CLockAppSession::DoDisableKeyguardL( TBool aShowNote )
    {
    // calls the main control

    return StateControlL()->DisableKeyguardL( aShowNote );
    }

// ---------------------------------------------------------------------------
// Request lockapp to enable devicelock on device (might not be approved).
// ---------------------------------------------------------------------------
TInt CLockAppSession::DoEnableDevicelockL( TDevicelockReason aReason )
    {
    // calls the main control

    return StateControlL()->EnableDevicelockL( aReason );
    }

// ---------------------------------------------------------------------------
// Request lockapp to disable devicelock on device (might not be approved).
// ---------------------------------------------------------------------------
TInt CLockAppSession::DoDisableDevicelockL( )
    {
    // TODO Investigate if this should be allowed in API or not
    // calls the main control
    // StateControlL()->DisableDevicelockL( );
    return KErrPermissionDenied;
    }

// ---------------------------------------------------------------------------
// Request to offer keyguard on device (might not be approved, e.g.
// already locked).
// ---------------------------------------------------------------------------
TInt CLockAppSession::DoOfferKeyguardL( )
    {
    // calls the main control
    return StateControlL()->OfferKeyguard( );
    }

// ---------------------------------------------------------------------------
// Request to show keys locked note, only works if keyguard is enabled
// ---------------------------------------------------------------------------
TInt CLockAppSession::DoShowKeysLockedNoteL( )
    {
    // calls the main control

    return StateControlL()->ShowKeysLockedNote( );
    }

// ---------------------------------------------------------------------------
// Return the main state control to the server session.
// Application appui and the main state control might not be constructed yet.
// ---------------------------------------------------------------------------
MLockAppStateControl* CLockAppSession::StateControlL( )
    {

    // other parts of lockapp construction may not be finished
    if ( !iStateControl )
        {
        CLockAppAppUi* appUi = (CLockAppAppUi*) CEikonEnv::Static()->EikAppUi();
        if ( appUi )
            {
            iStateControl = appUi->StateControl( );
            if ( !iStateControl )
                {
                User::Leave( KErrNotReady );
                }
            }
        else
            {
            User::Leave( KErrNotReady );
            }
        }
    return iStateControl;
    }
