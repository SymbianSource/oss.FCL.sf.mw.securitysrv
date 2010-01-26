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
* Description:  Notification Registry for SmartCard
*
*/



// INCLUDE FILES
#include    "ScardBase.h"
#include    "ScardNotifyRegistry.h"
#include    "ScardServer.h"
#include    "ScardNotifier.h"
#include    "WimTrace.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CScardNotifyRegistry::CScardNotifyRegistry
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CScardNotifyRegistry::CScardNotifyRegistry() 
    {
    _WIMTRACE(_L("WIM|Scard|CScardNotifyRegistry::CScardNotifyRegistry|Begin"));
    }

// -----------------------------------------------------------------------------
// CScardNotifyRegistry::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CScardNotifyRegistry::ConstructL( CScardServer* aServer )
    {
    _WIMTRACE(_L("WIM|Scard|CScardNotifyRegistry::ConstructL|Begin"));
    iNotifiers = new( ELeave ) CArrayPtrFlat<CScardNotifier>( 1 );

    //  The server is notified of all reader events. 
    RMessage2 message; // Empty message
    CScardNotifier* serverNotifier = CScardNotifier::NewL( this, 
                                                           message,
                                                           NULL,
                                                           KServerNotifier );

    iNotifiers->AppendL( serverNotifier );

    iServer = aServer;
    }

// -----------------------------------------------------------------------------
// CScardNotifyRegistry::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CScardNotifyRegistry* CScardNotifyRegistry::NewL( CScardServer* aServer )
    {
    _WIMTRACE(_L("WIM|Scard|CScardNotifyRegistry::NewL|Begin"));
    CScardNotifyRegistry* self = new( ELeave ) CScardNotifyRegistry;
    
    CleanupStack::PushL( self );
    self->ConstructL( aServer );
    CleanupStack::Pop( self );

    return self;
    }

    
// Destructor
CScardNotifyRegistry::~CScardNotifyRegistry() 
    {
    _WIMTRACE(_L("WIM|Scard|CScardNotifyRegistry::~CScardNotifyRegistry|Begin"));
    if ( iNotifiers )
        {
        iNotifiers->ResetAndDestroy();
        }
    delete iNotifiers;
    }


// -----------------------------------------------------------------------------
// CScardNotifyRegistry::NotifyCardEvent
// Notify all notifiers when event occurs
// -----------------------------------------------------------------------------
//
EXPORT_C void CScardNotifyRegistry::NotifyCardEvent(
    TScardServiceStatus aEvent,
    TReaderID aReaderID )
    {
    _WIMTRACE2(_L("WIM|Scard|CScardNotifyRegistry::NotifyCardEvent|Begin, notifierCount=%d"), iNotifiers->Count());
    // Tell the server of the event
    CScardNotifier* notifier = (*iNotifiers)[0];
    notifier->NotifyCardEvent( aEvent, aReaderID );

    // notify clients (if any)
    for ( TInt i( 1 ); i < iNotifiers->Count(); i++ )
        {
        notifier = (*iNotifiers)[i];
        if ( notifier->ReaderId() == aReaderID )
            {
            _WIMTRACE(_L("WIM|Scard|CScardNotifyRegistry::NotifyCardEvent|NotifyCardEvent Regular starts"));
            notifier->NotifyCardEvent( aEvent, aReaderID );
            _WIMTRACE(_L("WIM|Scard|CScardNotifyRegistry::NotifyCardEvent|NotifyCardEvent Regular DONE"));

            // when creating notifier in AddReaderEventListenerL,
            // the RMessage pointer is passed as parameter to notifier.
            // that will be used later to notify the client about smart 
            // card event.
            // So that each notifier can only be used once.
            // so we must remove the notifier from the array.
            // the scard server's notified can be kept.
            iNotifiers->Delete( i );
            delete notifier;
            }
        }
    }

// -----------------------------------------------------------------------------
// CScardNotifyRegistry::Server
// Return CScardServer object
// -----------------------------------------------------------------------------
//
CScardServer* CScardNotifyRegistry::Server()
    {
    _WIMTRACE(_L("WIM|Scard|CScardNotifyRegistry::Server|Begin"));
    return iServer;
    }

// -----------------------------------------------------------------------------
// CScardNotifyRegistry::Cancel
//
// -----------------------------------------------------------------------------
//
void CScardNotifyRegistry::Cancel( TRequestStatus& aStatus )
    {
    _WIMTRACE(_L("WIM|Scard|CScardNotifyRegistry::Cancel|Begin"));
    TRequestStatus* sp = &aStatus;
    User::RequestComplete( sp, KScErrCancelled );
    }

// -----------------------------------------------------------------------------
// CScardNotifyRegistry::RemoveReaderEventListeners
// Remove event listener from registry and delete it
// -----------------------------------------------------------------------------
//
void CScardNotifyRegistry::RemoveReaderEventListeners(
    const RThread& /*aClient*/ )
    {
    _WIMTRACE(_L("WIM|Scard|CScardNotifyRegistry::RemoveReaderEventListeners|Begin"));
    //find and delete from array
    TInt numOfItems = iNotifiers->Count();

    //do while better here EPR
    for ( TInt i( 1 ); i < numOfItems; i++ )
        {
        iNotifiers->Delete( i );
        }
    _WIMTRACE(_L("WIM|Scard|CScardNotifyRegistry::RemoveReaderEventListeners|End"));
    }

// -----------------------------------------------------------------------------
// CScardNotifyRegistry::RemoveReaderEventListener
// Remove event listener from registry and delete it
// -----------------------------------------------------------------------------
//
void CScardNotifyRegistry::RemoveReaderEventListener(
    const RMessage2& /*aMessage*/ )
    {
    _WIMTRACE(_L("WIM|Scard|CScardNotifyRegistry::RemoveReaderEventListener|Begin"));

    TInt notifierCount = iNotifiers->Count();

    _WIMTRACE2(_L("WIM|Scard|CScardNotifyRegistry::RemoveReaderEventListener|Notifier count=%d"), notifierCount);

    //Delete all but our own listener which is the first
    for ( TInt i( 1 ); i < notifierCount; i++ )
        {
        CScardNotifier* notifier = (*iNotifiers)[i];
        delete notifier;
        iNotifiers->Delete( i );
        break;
        }
    _WIMTRACE(_L("WIM|Scard|CScardNotifyRegistry::RemoveReaderEventListener|END"));

    }

// -----------------------------------------------------------------------------
// CScardNotifyRegistry::AddReaderEventListenerL
// Add new event listener
// -----------------------------------------------------------------------------
//
void CScardNotifyRegistry::AddReaderEventListenerL( const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM|Scard|CScardNotifyRegistry::AddReaderEventListenerL|Begin"));

    RMessage2 message( aMessage );
    // extract the TRequestStatus from message
    TPckgBuf<TRequestStatus*> pckg;

    aMessage.ReadL( 0, pckg );

    TRequestStatus* status = pckg();
    _WIMTRACE2(_L("WIM|Scard|CScardNotifyRegistry::AddReaderEventListenerL|status=%d"), status);
    
    //  extract the reader's friendly name
    TScardReaderName name;
    aMessage.ReadL( 1, name );

    const TReaderID readerID = iServer->ReaderID( name );

    if ( !readerID )
        {
        aMessage.Complete( KScErrUnknownReader );
        _WIMTRACE(_L("WIM|Scard|CScardNotifyRegistry::AddReaderEventListenerL|Leave"));
        User::Leave( KScErrNotSupported );
        }

    // add item to listener table
    CScardNotifier* notifier2 = CScardNotifier::NewL( this,
                                                      message,
                                                      status,
                                                      readerID );
    CleanupStack::PushL( notifier2 );
    iNotifiers->AppendL( notifier2 );
    CleanupStack::Pop( notifier2 );
    _WIMTRACE(_L("WIM|Scard|CScardNotifyRegistry::AddReaderEventListenerL|End"));
    }


//  End of File  
