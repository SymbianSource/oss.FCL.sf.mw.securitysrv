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
* Description:  Interface to access keyguard.
 *
*/


#include <keyguardaccessapi.h>
#include <lockappclientserver.h>
#include "lockaccessextension.h"

#include <avkondomainpskeys.h>      // KPSUidAvkonDomain, KAknKeyguardStatus, TAknKeyguardStatus
#include <e32property.h> // P&S API
#include <e32debug.h>

// ---------------------------------------------------------------------------
// Standard Symbian OS construction sequence
// ---------------------------------------------------------------------------
EXPORT_C CKeyguardAccessApi* CKeyguardAccessApi::NewL( )
    {
    CKeyguardAccessApi* self = new (ELeave) CKeyguardAccessApi( );
    CleanupStack::PushL( self );
    self->ConstructL( );
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Constructor.
// ---------------------------------------------------------------------------
CKeyguardAccessApi::CKeyguardAccessApi()
    {
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
EXPORT_C CKeyguardAccessApi::~CKeyguardAccessApi( )
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
void CKeyguardAccessApi::ConstructL( )
    {
    iLockAccessExtension = new (ELeave) RLockAccessExtension;
    }

// ---------------------------------------------------------------------------
// Checks if the phone has been locked using P&S key storing lock states
// ---------------------------------------------------------------------------
EXPORT_C TBool CKeyguardAccessApi::IsKeylockEnabled()
    {
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
// Checks if the keyguard is enabled or not
// ---------------------------------------------------------------------------
EXPORT_C TBool CKeyguardAccessApi::IsKeyguardEnabled()
    {
    TInt value;
    TInt err = RProperty::Get(KPSUidAvkonDomain, KAknKeyguardStatus, value);
    if ( err == KErrNone )
        {
        switch( value )
            {
            case EKeyguardLocked:
                return ETrue;
            case EKeyguardAutolockEmulation:
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
// Requests lockapp server to enable keyguard with or without note.
// ---------------------------------------------------------------------------
EXPORT_C TInt CKeyguardAccessApi::EnableKeyguard( TBool aWithNote )
    {
    if ( iLockAccessExtension )
        {
        return iLockAccessExtension->SendMessage( ELockAppEnableKeyguard, aWithNote );
        }
    else
        {
        return KErrNotFound;
        }
    }

// ---------------------------------------------------------------------------
// Requests lockapp server to disable keyguard with or without note.
// ---------------------------------------------------------------------------
EXPORT_C TInt CKeyguardAccessApi::DisableKeyguard( TBool aWithNote )
    {
    if ( iLockAccessExtension )
        {
        return iLockAccessExtension->SendMessage( ELockAppDisableKeyguard, aWithNote );
        }
    else
        {
        return KErrNotFound;
        }
    }

// ---------------------------------------------------------------------------
// Requests lockapp server to offer keyguard to user.
// ---------------------------------------------------------------------------
EXPORT_C TInt CKeyguardAccessApi::OfferKeyguard()
    {
    if ( iLockAccessExtension )
        {
        return iLockAccessExtension->SendMessage( ELockAppOfferKeyguard );
        }
    else
        {
        return KErrNotFound;
        }
    }

// ---------------------------------------------------------------------------
// Shows note informing that keys are locked. Only works if keyguard is already enabled.
// ---------------------------------------------------------------------------
EXPORT_C TInt CKeyguardAccessApi::ShowKeysLockedNote()
    {
    if ( iLockAccessExtension )
        {
        return iLockAccessExtension->SendMessage( ELockAppShowKeysLockedNote );
        }
    else
        {
        return KErrNotFound;
        }
    }



// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
EXPORT_C TInt CKeyguardAccessApi::TestInternal( )
    {
#ifdef _DEBUG
    if ( iLockAccessExtension )
        {
        return iLockAccessExtension->SendMessage( ELockAppTestInternal );
        }
    else
        {
        return KErrNotFound;
        }
#else
    return KErrNotSupported;
#endif
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
EXPORT_C TInt CKeyguardAccessApi::TestDestruct( )
    {
#ifdef _DEBUG
    if ( iLockAccessExtension )
        {
        return iLockAccessExtension->SendMessage( ELockAppTestDestruct );
        }
    else
        {
        return KErrNotFound;
        }
#else
    return KErrNotSupported;
#endif
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
EXPORT_C TInt CKeyguardAccessApi::TestPanicClient( )
    {
#ifdef _DEBUG
    if ( iLockAccessExtension )
        {
        return iLockAccessExtension->SendMessage( ELockAppTestPanicClient );
        }
    else
        {
        return KErrNotFound;
        }
#else
    return KErrNotSupported;
#endif
    }


// End of File
