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
* Description:  Interface to access devicelock.
 *
*/


#include <devicelockaccessapi.h>
#include <lockappclientserver.h>
#include "lockaccessextension.h"

#include <avkondomainpskeys.h>  // KPSUidAvkonDomain, KAknKeyguardStatus, TAknKeyguardStatus
#include <e32property.h> // P&S API
#include <e32debug.h>

// ---------------------------------------------------------------------------
// Standard Symbian OS construction sequence
// ---------------------------------------------------------------------------
EXPORT_C CDevicelockAccessApi* CDevicelockAccessApi::NewL( )
    {
    RDEBUG("0", 0);
    CDevicelockAccessApi* self = new (ELeave) CDevicelockAccessApi( );
    CleanupStack::PushL( self );
    self->ConstructL( );
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Constructor.
// ---------------------------------------------------------------------------
CDevicelockAccessApi::CDevicelockAccessApi()
    {
    RDEBUG("0", 0);
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
EXPORT_C CDevicelockAccessApi::~CDevicelockAccessApi( )
    {
    if ( iLockAccessExtension )
        {
        iLockAccessExtension->Close( );
        delete iLockAccessExtension;
        iLockAccessExtension = NULL;
        }
    }

// ---------------------------------------------------------------------------
// Second phase construction
// ---------------------------------------------------------------------------
void CDevicelockAccessApi::ConstructL( )
    {
    iLockAccessExtension = new (ELeave) RLockAccessExtension;
    }

// ---------------------------------------------------------------------------
// Checks if the phone has been locked using P&S key storing lock states
// ---------------------------------------------------------------------------
EXPORT_C TBool CDevicelockAccessApi::IsKeylockEnabled()
    {
    RDEBUG("0", 0);
    TInt value;
    TInt err = RProperty::Get(KPSUidAvkonDomain, KAknKeyguardStatus, value);
    if ( err == KErrNone )
        {
        switch( value )
            {
            case EKeyguardLocked:
            case EKeyguardAutolockEmulation:
            return ETrue;
            case EKeyguardNotActive:
            default:
            return EFalse;
            }
        }
    else
        {
        return EFalse;
        }
    }

// ---------------------------------------------------------------------------
// Checks if the devicelock is enabled or not
// ---------------------------------------------------------------------------
EXPORT_C TBool CDevicelockAccessApi::IsDevicelockEnabled()
    {
    RDEBUG("0", 0);
    TInt value;
    TInt err = RProperty::Get(KPSUidAvkonDomain, KAknKeyguardStatus, value);
    if ( err == KErrNone )
        {
        switch( value )
            {
            case EKeyguardAutolockEmulation:
                return ETrue;
            case EKeyguardLocked:
            case EKeyguardNotActive:
            default:
                return EFalse;
            }
        }
    else
        {
        return EFalse;
        }
    }

// ---------------------------------------------------------------------------
// Requests lockapp server to enable devicelock.
// ---------------------------------------------------------------------------
EXPORT_C TInt CDevicelockAccessApi::EnableDevicelock( TDevicelockReason aReason )
    {
    RDEBUG("aReason", aReason);
    if ( iLockAccessExtension )
        {
        switch (aReason)
            {
            case EDevicelockManual:
            case EDevicelockRemote:
                return iLockAccessExtension->SendMessage( ELockAppEnableDevicelock, aReason );
            default:
                return KErrNotSupported;
            }
        }
    else
        {
        return KErrNotFound;
        }
    }

// ---------------------------------------------------------------------------
// Requests lockapp server to offer devicelock to user.
// ---------------------------------------------------------------------------
EXPORT_C TInt CDevicelockAccessApi::OfferDevicelock()
    {
    RDEBUG("0", 0);
    if ( iLockAccessExtension )
        {
        return iLockAccessExtension->SendMessage( ELockAppOfferDevicelock );
        }
    else
        {
        return KErrNotFound;
        }
    }

// ---------------------------------------------------------------------------
// TODO: remove this method!
// Requests lockapp server to disable devicelock.
// ---------------------------------------------------------------------------
EXPORT_C TInt CDevicelockAccessApi::DisableDevicelock()
    {
    RDEBUG("0", 0);
    if ( iLockAccessExtension )
        {
        return iLockAccessExtension->SendMessage( ELockAppDisableDevicelock );
        }
    else
        {
        return KErrNotFound;
        }
    }

// End of File
