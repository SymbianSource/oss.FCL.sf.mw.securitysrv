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
* Description:  Listens for smard card events
*
*/



// INCLUDE FILES
#include    "ScardBase.h"
#include    "WimScardListener.h"
#include    "WimServer.h"
#include    "WimSession.h"
#include    "WimTrace.h"


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CWimScardListener::CWimScardListener( 
    RScard* aScard,
    TUint8 aUiReaderID,
    const TScardReaderName& aName )
    : CScardListener( aScard ),
      iUiReaderID( aUiReaderID ),
      iName( aName )
      
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimScardListener::CWimScardListener | Begin"));
    }

// Destructor
CWimScardListener::~CWimScardListener()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimScardListener::~CWimScardListener | Begin"));
    }

// -----------------------------------------------------------------------------
// CWimScardListener::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CWimScardListener* CWimScardListener::NewL(
    RScard* aScard,
    TUint8 aUiReaderID,
    const TScardReaderName& aName )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimScardListener::NewL | Begin"));
    CWimScardListener* self = new( ELeave ) CWimScardListener( aScard,
        aUiReaderID, aName );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CWimScardListener::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CWimScardListener::ConstructL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimScardListener::ConstructL | Begin"));
    User::LeaveIfError( ListenCardEvents( iName ) ); // Start listening events
    iServer = CWimServer::Server();
    }

// -----------------------------------------------------------------------------
// CWimScardListener::ProcessEvent
// Card has been inserted/removed. From this point you have to inform 
// WIMI & client what has happened. 
// -----------------------------------------------------------------------------
//
void CWimScardListener::ProcessEvent( const TScardServiceStatus aEvent )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimScardListener::ProcessEvent | Begin"));

    switch ( aEvent )
        {
        case EScardInserted:
            {
             _WIMTRACE(_L("WIM | WIMServer | CWimScardListener::ProcessEvent Inserted"));
            break;
            }

        case EScardRemoved:
            {
            _WIMTRACE(_L("WIM | WIMServer | CWimScardListener::ProcessEvent Removed"));
            RArray<CWimSession*> sessions;
            TRAPD( err, iServer->GetSessionsL( sessions ) );

            if ( !err )  // Got sessions correctly
                {
                // Loop through all sessions and notify all clients that are
                // requesting the notification
                for ( TInt i( 0 ); i < sessions.Count(); i++ )
                    {
                    sessions[i]->NotifyComplete();
                    }
                sessions.Reset();
                // Close WIMI
                if( CWimServer::iWimInitialized )
                    {
                    WIMI_CloseDownReq();   
                    }
               CWimServer::SetWimInitialized( EFalse, KErrHardwareNotAvailable );
                }
            break;
            }

        default:
            {
            _WIMTRACE(_L("WIM | WIMServer | CWimScardListener::ProcessEvent Unexpected smart card event"));
            break;
            }
        }
    _WIMTRACE(_L("WIM | WIMServer | CWimScardListener::ProcessEvent | End"));
    }

// End of File
