/*
* Copyright (c) 2003-2009 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Registry of WimServer sessions
*
*/



// INCLUDE FILES
#include    "WimSessionRegistry.h"
#include    "WimTrace.h"


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CWimSessionRegistry::CWimSessionRegistry()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSessionRegistry::CWimSessionRegistry | Begin"));
    }

// Destructor
CWimSessionRegistry::~CWimSessionRegistry()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSessionRegistry::~CWimSessionRegistry | Begin"));
    iSessions.Close();
    }

// -----------------------------------------------------------------------------
// CWimSessionRegistry::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CWimSessionRegistry* CWimSessionRegistry::NewL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSessionRegistry::NewL | Begin"));
    CWimSessionRegistry* self = new( ELeave ) CWimSessionRegistry();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CWimSessionRegistry::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CWimSessionRegistry::ConstructL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSessionRegistry::ConstructL | Begin"));
    }

// -----------------------------------------------------------------------------
// CWimSessionRegistry::AddSession
// Add session to array
// -----------------------------------------------------------------------------
//
void CWimSessionRegistry::AddSessionL( CWimSession* aSession )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSessionRegistry::AddSessionL | Begin"));
    iSessions.AppendL( aSession );
    }

// -----------------------------------------------------------------------------
// CWimSessionRegistry::GetSessionsL
// Get array of sessions
// -----------------------------------------------------------------------------
//
void CWimSessionRegistry::GetSessionsL( RArray<CWimSession*>& aSessions ) const
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSessionRegistry::GetSessionsL | Begin"));
    TInt count = iSessions.Count();
    for( TInt i = 0; i < count; ++i )
        {
        aSessions.AppendL( iSessions[ i ] );
        }
    }

// -----------------------------------------------------------------------------
// CWimSessionRegistry::RemoveSession
// Remove session from array
// -----------------------------------------------------------------------------
//
void CWimSessionRegistry::RemoveSession( CWimSession* aSession )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSessionRegistry::RemoveSession | Begin"));
	TInt sessionCount = iSessions.Count();
    for( TInt i = 0; i < sessionCount; ++i )
        {
        if( iSessions[ i ] == aSession )
            {
            iSessions.Remove( i );
            break;
            }
        }
    iSessions.Compress();
    }

// End of File
