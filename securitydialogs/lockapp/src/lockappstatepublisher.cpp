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
* Description:  Publishes LockApp states to other applications
 *
*/


#include "lockappstatepublisher.h"
#include "lockapputils.h"

// defines keys KPSUidAvkonDomain, KAknKeyguardStatus, TAknKeyguardStatus
#include <avkondomainpskeys.h>

// ---------------------------------------------------------------------------
// Standard Symbian OS construction sequence
// ---------------------------------------------------------------------------
CLockAppStatePublisher* CLockAppStatePublisher::NewL( )
    {
    CLockAppStatePublisher* self = new (ELeave) CLockAppStatePublisher();
    CleanupStack::PushL( self );
    self->ConstructL( );
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Standard C++ constructor
// ---------------------------------------------------------------------------
CLockAppStatePublisher::CLockAppStatePublisher()
    {
    // no implementation required
    }

// ---------------------------------------------------------------------------
// Closes the property handle.
// ---------------------------------------------------------------------------
CLockAppStatePublisher::~CLockAppStatePublisher()
    {
    iStatusProperty.Close();
    }

// ---------------------------------------------------------------------------
// Defines the P&S key storing keyguard status
// ---------------------------------------------------------------------------
void CLockAppStatePublisher::ConstructL()
    {
    // for some reason P&S capability policy macros do not work with codetest
#ifdef _LOCKAPP_CODETEST_DEBUG
    RProperty::Define(
        KPSUidAvkonDomain,
        KAknKeyguardStatus,
        RProperty::EInt,
        TSecurityPolicy(TSecurityPolicy::EAlwaysPass),
        TSecurityPolicy(TSecurityPolicy::EAlwaysPass));
#else
    // defines the P&S key
    _LIT_SECURITY_POLICY_C1(KWritePolicy, ECapabilityWriteDeviceData);
    RProperty::Define(
        KPSUidAvkonDomain,
        KAknKeyguardStatus,
        RProperty::EInt,
        TSecurityPolicy(TSecurityPolicy::EAlwaysPass),
        KWritePolicy);
#endif
    // creates handle to property
    iStatusProperty.Attach(KPSUidAvkonDomain, KAknKeyguardStatus);
    }

// ---------------------------------------------------------------------------
// From @c CLockAppStateObserver. Used to publish lockapp state
// to external parties.
// ---------------------------------------------------------------------------
void CLockAppStatePublisher::HandleLockStatusChangedL( TLockStatus aLockStatus )
    {
    switch ( aLockStatus )
        {
        case ELockNotActive:
            {
            iStatusProperty.Set(EKeyguardNotActive);
            }
            break;
        case EDevicelockActive:
            {
            iStatusProperty.Set(EKeyguardAutolockEmulation);
            }
            break;
        case EKeyguardActive:
            {
            iStatusProperty.Set(EKeyguardLocked);
            }
            break;
        default:
            DoPanic(ELockUnknownValue);
            break;
        }
    }
