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
* Description:  
*
*/


// INCLUDE FILES
#include "DevEncStarterPropertyObserver.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CDevEncStarterPropertyObserver::CPropertyObserver
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CDevEncStarterPropertyObserver::CDevEncStarterPropertyObserver(
                               MDevEncStarterPropertyChangeObserver& aObserver,
                               const TUid& aCategory,
                               const TUint aKey )
    : CActive( CActive::EPriorityStandard ),
    iObserver( aObserver ),
    iCategory( aCategory ),
    iKey( aKey )
    {
    }

// -----------------------------------------------------------------------------
// CDevEncStarterPropertyObserver::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CDevEncStarterPropertyObserver::ConstructL()
    {
    User::LeaveIfError( iProperty.Attach( iCategory, iKey ) );
    CActiveScheduler::Add( this );
    iProperty.Subscribe( iStatus );
    SetActive();
    }

// -----------------------------------------------------------------------------
// CDevEncStarterPropertyObserver::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CDevEncStarterPropertyObserver* CDevEncStarterPropertyObserver::NewL(
                               MDevEncStarterPropertyChangeObserver& aObserver,
                               const TUid& aCategory,
                               const TUint aKey )
    {
    CDevEncStarterPropertyObserver* self =
        new ( ELeave ) CDevEncStarterPropertyObserver( aObserver, aCategory, aKey );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// Destructor
CDevEncStarterPropertyObserver::~CDevEncStarterPropertyObserver()
    {
    Cancel();
    iProperty.Close();
    }

// -----------------------------------------------------------------------------
// CDevEncStarterPropertyObserver::RunL
//
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CDevEncStarterPropertyObserver::RunL()
    {
    iProperty.Subscribe( iStatus );
    SetActive();
    TInt err = iProperty.Get( iValue );
    if ( err )
        {
        iObserver.HandlePropertyChangeErrorL( iCategory, iKey, err );
        }
    else
        {
        iObserver.HandlePropertyChangeL( iCategory, iKey, iValue );
        }
    }

// -----------------------------------------------------------------------------
// CDevEncStarterPropertyObserver::DoCancel
// Cancels an outstanding active request
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CDevEncStarterPropertyObserver::DoCancel()
    {
    iProperty.Cancel();
    }

// -----------------------------------------------------------------------------
// CDevEncStarterPropertyObserver::GetValue
// Returns KErrNone if operation was successful
// Actual value is written to aValue
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CDevEncStarterPropertyObserver::GetValue( TInt& aValue ) const
    {
    aValue = iValue;
    }

// End of File
