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
* Description:  Publish & Subscribe key observer
 *
*/


#include "lockapppubsubobserver.h"
#include "lockapputils.h"

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
CLockAppPubSubObserver* CLockAppPubSubObserver::NewL(MLockAppObserverInterface* aObserver,
        TUid aPubSubUid, TUint32 aKeyId )
    {
    CLockAppPubSubObserver* self = new (ELeave) CLockAppPubSubObserver(aObserver, aPubSubUid, aKeyId);
    CleanupStack::PushL( self );
    self->ConstructL( );
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
CLockAppPubSubObserver::CLockAppPubSubObserver(MLockAppObserverInterface* aObserver,
        TUid aPubSubUid, TUint32 aKeyId ) :
    CActive( 0 ),
    iObserver( aObserver ),
    iPubSubUid( aPubSubUid ),
    iKeyId( aKeyId ),
    iValue( NULL )
    {
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
CLockAppPubSubObserver::~CLockAppPubSubObserver( )
    {
    Cancel( );
    iProperty.Close( );
    }

// ---------------------------------------------------------------------------
// Symbian OS default constructor
// ---------------------------------------------------------------------------
void CLockAppPubSubObserver::ConstructL( )
    {
    // Add this active object to the scheduler.
    CActiveScheduler::Add( this );
    TInt err = iProperty.Attach( iPubSubUid, iKeyId );
    ERROR_2(err, "CLockAppPubSubObserver::ConstructL - Property(%d,%d) attach", iPubSubUid, iKeyId);
    User::LeaveIfError( err );
    Start( );
    }

// ---------------------------------------------------------------------------
// Starts listening
// ---------------------------------------------------------------------------
TInt CLockAppPubSubObserver::Start( )
    {
    if ( !IsActive( ) )
        {
        iStatus = KRequestPending;
        iProperty.Subscribe( iStatus );
        SetActive( );
        return KErrNone;
        }
    else
        {
        return KErrInUse;
        }
    }

// ---------------------------------------------------------------------------
// Stops listening
// ---------------------------------------------------------------------------
void CLockAppPubSubObserver::Stop( )
    {
    if ( IsActive( ) )
        {
        Cancel( );
        iProperty.Cancel( );
        }
    }

// ---------------------------------------------------------------------------
// Gets value of the key from P&S.
// ---------------------------------------------------------------------------
TInt CLockAppPubSubObserver::GetKeyValue(TInt& aValue )
    {
    RDebug::Printf( "%s %s (%u) 1=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 1 );
    return iProperty.Get( aValue );
    }

// ---------------------------------------------------------------------------
// Sets a value for the key in P&S.
// ---------------------------------------------------------------------------
TInt CLockAppPubSubObserver::SetKeyValue(TInt aValue )
    {
    RDebug::Printf( "%s %s (%u) aValue=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aValue );
    return iProperty.Set( aValue );
    }

// ---------------------------------------------------------------------------
// CLockAppPubSubObserver::RunL()
// ---------------------------------------------------------------------------
void CLockAppPubSubObserver::RunL( )
    {
    TInt value( NULL);
    iProperty.Get( value );
    if ( iValue != value )
        {
        // on value change
        iValue = value;
        if ( iObserver )
            {
            iObserver->HandlePubSubNotify( iPubSubUid, iKeyId, iValue );
            }
        }
    // re-subscribe to events
    Start( );
    }

// ---------------------------------------------------------------------------
// Cancels event listening
// ---------------------------------------------------------------------------
void CLockAppPubSubObserver::DoCancel( )
    {
    iProperty.Cancel( );
    }

// End of file
