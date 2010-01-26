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
* Description:  A class implementing two stacks. One is a fifo-stack of
*               reader events, the other a random-access stack of service
*               providers that are not used with a given reader
*
*/



// INCLUDE FILES
#include    "ScardBase.h"
#include    "ScardEventStack.h"
#include    "ScardConnector.h"
#include    "WimTrace.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CScardEventStack::CScardEventStack
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CScardEventStack::CScardEventStack()
    {
    }

// -----------------------------------------------------------------------------
// CScardEventStack::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CScardEventStack::ConstructL( CScardConnector* aConnector )
    {
    _WIMTRACE(_L("WIM|Scard|CScardEventStack::ConstructL|Begin"));
    iConnector = aConnector;
    iEvents = new ( ELeave ) CArrayFixFlat<TQueueEvent>( 1 );
    }

// -----------------------------------------------------------------------------
// CScardEventStack::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CScardEventStack* CScardEventStack::NewL( CScardConnector* aConnector )
    {
    _WIMTRACE(_L("WIM|Scard|CScardEventStack::NewL|Begin"));
    CScardEventStack* self = new( ELeave ) CScardEventStack;
    
    CleanupStack::PushL( self );
    self->ConstructL( aConnector );
    CleanupStack::Pop( self );

    return self;
    }

    
// Destructor
CScardEventStack::~CScardEventStack()
    {
    _WIMTRACE(_L("WIM|Scard|CScardEventStack::~CScardEventStack|Begin"));
    if ( iEvents )
        {
        iEvents->Reset();
        }
    delete iEvents;    
    }

// -----------------------------------------------------------------------------
// CScardEventStack::QueueEvent
// Queue event
// -----------------------------------------------------------------------------
//
void CScardEventStack::QueueEvent(
    const TReaderID aReaderID, 
    const TScardServiceStatus aEventType )
    {
    _WIMTRACE(_L("WIM|Scard|CScardEventStack::QueueEvent|Begin"));
    //  Remove any old events for this reader to ensure that no out-of-date 
    //  events are in the stack
    for ( TInt i( 0 ); i < iEvents->Count(); i++ )
        {
        if ( iEvents->At( i ).iReaderID == aReaderID )
            {
            iEvents->Delete( i );
            break;
            }
        }

    //  Add event to the stack
    TQueueEvent event;
    event.iEventType = aEventType;
    event.iReaderID = aReaderID;
    TRAPD( mem_ok, iEvents->AppendL( event ) );
    __ASSERT_MEMORY( mem_ok );
    }

//  End of File  
