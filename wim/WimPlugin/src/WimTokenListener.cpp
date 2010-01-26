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
* Description:  Listens removal event from WimServer
*
*/


// INCLUDE FILES

#include "WimTokenListener.h"
#include "WimTrace.h"
//#include "WimDebug.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWimTokenListener::CWimTokenListener()
// Default constructor
// -----------------------------------------------------------------------------
//
CWimTokenListener::CWimTokenListener( CWimSecModule* aWimSecModule )
                                    : CActive ( EPriorityNormal ),
                                      iWimSecModule( aWimSecModule ),
                                      iClientStatus( NULL )
    {
    }

// -----------------------------------------------------------------------------
// CWimTokenListener::ConstructL()
// Second phase constructor
// -----------------------------------------------------------------------------
//
void CWimTokenListener::ConstructL()
    {
    _WIMTRACE ( _L( "CWimTokenListener::ConstructL()" ) );
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CWimTokenListener::NewL()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CWimTokenListener* CWimTokenListener::NewL( CWimSecModule* aWimSecModule )
    {
    _WIMTRACE ( _L( "CWimTokenListener::NewL()" ) );
    CWimTokenListener* self = new( ELeave ) CWimTokenListener( aWimSecModule );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CWimTokenListener::~CWimTokenListener()
// Destructor
// -----------------------------------------------------------------------------
//
CWimTokenListener::~CWimTokenListener()
    {
    _WIMTRACE ( _L( "CWimTokenListener::~CWimTokenListener()" ) );
    ClearClientStatus(); // Complete UI's request if needed
    if ( iWimSecModule && ( iTokenStatus == KRequestPending ) )
        {
        iWimSecModule->CancelNotifyOnRemoval(); // Cancel listening in WimServer   
        }
    Cancel();
    }

// -----------------------------------------------------------------------------
// CWimTokenListener::StartListening()
// Start listening of Scard removal status
// -----------------------------------------------------------------------------
//
void CWimTokenListener::StartListening()
    {
    _WIMTRACE ( _L( "CWimTokenListener::StartListening()" ) );
    iStatus = KRequestPending;
    iTokenStatus = KRequestPending;
    iWimSecModule->NotifyOnRemoval( iStatus );
    SetActive();    
    }

// -----------------------------------------------------------------------------
// CWimTokenListener::SetClientStatus()
// Store client status
// -----------------------------------------------------------------------------
//
void CWimTokenListener::SetClientStatus( TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimTokenListener::SetClientStatus()" ) );
    iClientStatus = &aStatus;
    }

// -----------------------------------------------------------------------------
// CWimTokenListener::ClearClientStatus()
// If client request is still pending complete request with KErrCancel
// -----------------------------------------------------------------------------
//
void CWimTokenListener::ClearClientStatus()
    {
    _WIMTRACE ( _L( "CWimTokenListener::ClearClientStatus()" ) );
    // If request is still pending, complete it with KErrCancel
    // RequestComplete sets iClientStatus to NULL
    if ( iClientStatus )
        {
        User::RequestComplete( iClientStatus, KErrCancel );
        }
    }

// -----------------------------------------------------------------------------
// CWimTokenListener::RunL()
// Complete asyncronous request
// -----------------------------------------------------------------------------
//
void CWimTokenListener::RunL()
    {
    switch ( iStatus.Int() )
        {
        case KErrNone:
            {
            iTokenStatus = KErrHardwareNotAvailable;
            break;
            }
        case KErrCancel:
            {
            iTokenStatus = KErrCancel;
            break;
            }
        default:
            {
            iTokenStatus = iStatus.Int();
            break;
            }
        }

    if ( iClientStatus ) // Complete the request if pending
        {
        User::RequestComplete( iClientStatus, iStatus.Int() );
        }
    }

// -----------------------------------------------------------------------------
// CWimTokenListener::DoCancel()
// Nothing to do here
// -----------------------------------------------------------------------------
//
void CWimTokenListener::DoCancel()
    {
    _WIMTRACE ( _L( "CWimTokenListener::DoCancel()" ) );
    }

// -----------------------------------------------------------------------------
// CWimTokenListener::TokenStatus()
// Return the staus of token
// -----------------------------------------------------------------------------
//
TInt CWimTokenListener::TokenStatus()
    {
    _WIMTRACE ( _L( "CWimTokenListener::TokenStatus()" ) );
    return iTokenStatus;
    }
