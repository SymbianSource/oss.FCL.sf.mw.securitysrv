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
* Description:  Publish and subscribe settings listener.
*
*/


// INCLUDE FILES
#include "GSPubSubsListener.h"
#include "MGSSettingPSObserver.h"


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CGSPubSubsListener::CGSPubSubsListener
// C++ constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CGSPubSubsListener::CGSPubSubsListener( const TUid aUid, const TInt aKey, 
                                       MGSSettingPSObserver* aObserver ) 
    : CActive( CActive::EPriorityStandard )
    {
    iUid = aUid;
    iId = aKey;
    iCallback = aObserver;
    }

// -----------------------------------------------------------------------------
// CGSPubSubsListener::~CGSPubSubsListener
// Destructor.
// -----------------------------------------------------------------------------
//
CGSPubSubsListener::~CGSPubSubsListener()
    {
    Cancel();
    iProperty.Close();
    }

// -----------------------------------------------------------------------------
// CGSPubSubsListener::RunL
// From CActive.
// -----------------------------------------------------------------------------
//
void CGSPubSubsListener::RunL()
    {
    const TRequestStatus status( iStatus );
    StartListening();
    iCallback->HandleNotifyPSL( iUid, iId, status );
    }

// -----------------------------------------------------------------------------
// CGSPubSubsListener::DoCancel
// From CActive.
// -----------------------------------------------------------------------------
//
void CGSPubSubsListener::DoCancel()
    {
    iProperty.Cancel();
    }

// -----------------------------------------------------------------------------
// CGSPubSubsListener::RunError
// From CActive.
// -----------------------------------------------------------------------------
//
TInt CGSPubSubsListener::RunError( TInt /*aError*/ )
    {
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGSPubSubsListener::NewL
//
// Symbian OS two phased constructor
// -----------------------------------------------------------------------------
//
CGSPubSubsListener* CGSPubSubsListener::NewL( const TUid aUid, const TInt aKey,
                                              MGSSettingPSObserver* aObserver )
    {
    CGSPubSubsListener* self = new( ELeave ) 
        CGSPubSubsListener( aUid, aKey, aObserver );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// -----------------------------------------------------------------------------
// CGSPubSubsListener::StartListening
// -----------------------------------------------------------------------------
//
void CGSPubSubsListener::StartListening() 
    {
    iProperty.Subscribe( iStatus );
    SetActive();
    }

// -----------------------------------------------------------------------------
// CGSPubSubsListener::ConstructL
//
// Symbian OS default constructor
// -----------------------------------------------------------------------------
//
void CGSPubSubsListener::ConstructL()
    {
    CActiveScheduler::Add( this );
    
    User::LeaveIfError ( iProperty.Attach( iUid, iId, EOwnerThread ) );
    StartListening();
    }

// -----------------------------------------------------------------------------
// CGSPubSubsListener::Get
// Read integer value.
// -----------------------------------------------------------------------------
//
TInt CGSPubSubsListener::Get( TInt& aVal )
    {
    return iProperty.Get( iUid, iId, aVal );
    }

// -----------------------------------------------------------------------------
// CGSPubSubsListener::Get
// Read binary value.
// -----------------------------------------------------------------------------
//
TInt CGSPubSubsListener::Get( TDes8& aVal )
    {
    return iProperty.Get( iUid, iId, aVal );
    }
  
// -----------------------------------------------------------------------------
// CGSPubSubsListener::Get
// Read string value.
// -----------------------------------------------------------------------------
//      
TInt CGSPubSubsListener::Get( TDes16& aVal )
    {
    return iProperty.Get( iUid, iId, aVal );
    }

// End of File
