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
* Description:  Registry of smart card resources
*
*/



// INCLUDE FILES
#include    "ScardResourceRegistry.h"
#include    "ScardServer.h"
#include    "WimTrace.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CScardResourceRegistry::CScardResourceRegistry
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CScardResourceRegistry::CScardResourceRegistry()
    : iServer( NULL )
    {
    _WIMTRACE(_L("WIM|Scard|CScardResourceRegistry::CScardResourceRegistry|Begin"));
    }

// -----------------------------------------------------------------------------
// CScardResourceRegistry::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CScardResourceRegistry::ConstructL( CScardServer* aServer )
    {
    _WIMTRACE(_L("WIM|Scard|CScardResourceRegistry::ConstructL|Begin"));
    iServer = aServer;
    }

// -----------------------------------------------------------------------------
// CScardResourceRegistry::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CScardResourceRegistry* CScardResourceRegistry::NewL( CScardServer* aServer )
    {
    _WIMTRACE(_L("WIM|Scard|CScardResourceRegistry::NewL|Begin"));
    CScardResourceRegistry* self = new( ELeave ) CScardResourceRegistry;
    
    CleanupStack::PushL( self );
    self->ConstructL( aServer );
    CleanupStack::Pop( self );

    return self;
    }

    
// Destructor
CScardResourceRegistry::~CScardResourceRegistry()
    {
    _WIMTRACE(_L("WIM|Scard|CScardResourceRegistry::~CScardResourceRegistry|Begin"));
    }


// -----------------------------------------------------------------------------
// CScardResourceRegistry::CardEventL
// If a card is inserted / removed, we need to update the list of cards in use
// -----------------------------------------------------------------------------
//
void CScardResourceRegistry::CardEvent(
    const TScardServiceStatus aEvent, 
    const TReaderID aReaderID )
    {
    _WIMTRACE(_L("WIM|Scard|CScardResourceRegistry::CardEventL|Begin"));
    //  First get the access controller for the reader
    CScardAccessControl* control = iServer->FindAccessControl( aReaderID );

    // Due to asynchronity, some readers may have closed before the events 
    // are processed
    if ( !control )
        {
        return;
        }

    //  Then the actual reader
    MScardReader* reader = control->Reader();
    __ASSERT_ALWAYS( reader, User::Panic( _L( "Unknown reader" ), 
        KScServerPanicBadRequest ) );

    switch ( aEvent )
        {
        case EScardRemoved: // Flow through 
        case EReaderRemoved:
            {
            TScardATR atr;

            //  Inform the sessions attached to the reader
            control->CardEvent( aEvent, atr );

            break;
            }
        case EScardInserted:
            {
            //  A new card was inserted, so get the atr bytes
            TScardATR atr;
            TInt err = reader->GetATR( atr );
            if ( !err )
                {
                }
            //  Inform sessions
            control->CardEvent( aEvent, atr );
            break;
            }
        default:
            break;
        }
    }

//  End of File  
