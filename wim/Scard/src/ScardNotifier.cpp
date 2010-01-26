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
* Description:  Notification of events
*
*/



// INCLUDE FILES
#include    "ScardServer.h"
#include    "ScardNotifier.h"
#include    "WimTrace.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CScardNotifier::CScardNotifier
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CScardNotifier::CScardNotifier(
    CScardNotifyRegistry* aRegistry, 
    RMessage2& aMessage,
    TRequestStatus* aClientStatus,
    const TReaderID& aReaderID )
    : CActive( EPriorityNormal ), 
      iRegistry( aRegistry ),
      iMessage( aMessage ),
      iClientStatus( aClientStatus ),
      iReaderID( aReaderID ),
      iMessageCompleted( EFalse )
    {
    _WIMTRACE(_L("WIM|Scard|CScardNotifier::CScardNotifier|Begin"));
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CScardNotifier::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CScardNotifier::ConstructL()
    {
    _WIMTRACE(_L("WIM|Scard|CScardNotifier::ConstructL|Begin"));
    //if the notifier is regular, no need to activate the ActiveObject.
    //since the notifier can only be used once and will be deleted after
    //the one-time notification is done.
    if(!iClientStatus)
       {
       iEventStack = new( ELeave ) CArrayFixFlat<TCardEvent>( 1 );
       iStatus = KRequestPending; 	
       SetActive();
       }
    }

// -----------------------------------------------------------------------------
// CScardNotifier::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CScardNotifier* CScardNotifier::NewL(
    CScardNotifyRegistry* aRegistry,
    RMessage2& aMessage,
    TRequestStatus* aClientStatus,
    const TReaderID& aReaderID )
    {
    _WIMTRACE(_L("WIM|Scard|CScardNotifier::NewL|Begin"));
    CScardNotifier* self = new( ELeave ) CScardNotifier( 
        aRegistry, aMessage, aClientStatus, aReaderID );
    
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    return self;
    }

    
// Destructor
CScardNotifier::~CScardNotifier()
    {
    _WIMTRACE(_L("WIM|Scard|CScardNotifier::~CScardNotifier|Begin"));
    if(!iClientStatus)
       {
       delete iEventStack;
       Cancel();
       }
    else
       {
       if ( !iMessageCompleted ) 
          {
          iMessage.Complete( KErrNone );
          }	
       }
    _WIMTRACE(_L("WIM|Scard|CScardNotifier::~CScardNotifier|End"));
    }

// -----------------------------------------------------------------------------
// CScardNotifier::NotifyCardEvent
// Complete our own request, or if that's done already, store the event
// until it can be serviced...
// -----------------------------------------------------------------------------
//
void CScardNotifier::NotifyCardEvent(
    TScardServiceStatus aEvent,
    TReaderID aReaderID )
    {
    _WIMTRACE(_L("WIM|Scard|CScardNotifier::NotifyCardEvent|Begin"));
    
    //regular notifier
    if( iClientStatus )
       {
       _WIMTRACE(_L("WIM|Scard|CScardNotifier::NotifyCardEvent| a regular notifier"));
       if(iMessageCompleted )
          {
       	  return;
          }
       _WIMTRACE(_L("WIM|Scard|CScardNotifier::NotifyCardEvent|iMessage not used"));   
       _WIMTRACE(_L("WIM|Scard|CScardNotifier::NotifyCardEvent|Notify client"));
       iMessage.Complete( aEvent );
       iMessageCompleted = ETrue;
       _WIMTRACE(_L("WIM|Scard|CScardNotifier::NotifyCardEvent|Notify client DONE"));
       return;	
       }
    
    _WIMTRACE(_L("WIM|Scard|CScardNotifier::NotifyCardEvent| Server notifier"));   
    //  No need to store the event, just notify it
    if ( iStatus == KRequestPending )
        {
        TUint code( 0 );
        
        //  If this is the server notifier, 
        //  it needs a different event encoding
        if ( !iClientStatus )
            {
            //  The server needs to know both the event code and 
            //  the reader ID, because it listens to all readers

            //  Place the reader ID in the higher bytes...
            code = aReaderID;
            code <<= 4;
            //  ...and the event code in the low 4 bytes
            code |= aEvent;
            }
        TRequestStatus* sp = &iStatus;
        User::RequestComplete( sp, code );
        }
    //  Well, previous notify hasn't finished, so store the event
    else
        {
        TCardEvent event;
        event.iEvent = aEvent;
        event.iReaderID = aReaderID;
        TRAPD( err, iEventStack->AppendL( event ) );

        if ( err != KErrNone ) // No reason to continue
            {
            User::Panic( _L( "Memory allocation error" ), err );
            }
        }
    }

// -----------------------------------------------------------------------------
// CScardNotifier::RunL
// Asynchronous request has finished
// -----------------------------------------------------------------------------
//
void CScardNotifier::RunL()
    {
    _WIMTRACE2(_L("WIM|Scard|CScardNotifier::RunL|Begin, iClientStatus=%d"), iClientStatus);
    //  if this is a regular notifier, 
    //  then some thing goes wrong.
    //  only server notifier should come to RunL().
    if ( iClientStatus )
        {
        _WIMTRACE(_L("WIM|Scard|CScardNotifier::RunL|Only server notifer will come to RunL"));
        }
    //  Otherwise this is the server notifier, so tell the server
    else
        {
        _WIMTRACE(_L("WIM|Scard|CScardNotifier::RunL|server notifer"));
        // the bit-shifting that was done when notifying is now reversed
        TScardServiceStatus event = static_cast< TScardServiceStatus >
                                                ( iStatus.Int() & 0x0f );
        TReaderID readerID = static_cast< TReaderID >( iStatus.Int() >> 4 );
        iRegistry->Server()->CardEvent( event, readerID );
        _WIMTRACE(_L("WIM|Scard|CScardNotifier::RunL|Server notification DONE"));
        }
    iStatus = KRequestPending;
    SetActive();

    //  If there are any queued events, notify (first one of) them now
    if ( iEventStack->Count() )
        {
        _WIMTRACE(_L("WIM|Scard|CScardNotifier::RunL| Some queued event in server notifier"));
        iRegistry->NotifyCardEvent( iEventStack->At( 0 ).iEvent, 
            iEventStack->At( 0 ).iReaderID );
        iEventStack->Delete( 0 );
        }
    _WIMTRACE(_L("WIM|Scard|CScardNotifier::RunL|Exit"));
    }

// -----------------------------------------------------------------------------
// CScardNotifier::DoCancel
// Active object cancelled
// -----------------------------------------------------------------------------
//
void CScardNotifier::DoCancel()
    {
    _WIMTRACE3(_L("WIM|Scard|CScardNotifier::DoCancel|Begin, status=%d"),
    iStatus.Int(), iClientStatus );
    iRegistry->Cancel( iStatus );
    _WIMTRACE(_L("WIM|Scard|CScardNotifier::DoCancel|Cancel called"));

    // If message not already completed do it here
    if ( !iMessageCompleted && iClientStatus ) 
        {
        iMessage.Complete( KErrCancel );
        _WIMTRACE(_L("WIM|Scard|CScardNotifier::DoCancel|Message completed"));
        }
    
    _WIMTRACE(_L("WIM|Scard|CScardNotifier::DoCancel|End"));
    }

// -----------------------------------------------------------------------------
// CScardNotifier::ReaderID
// Return readerID
// -----------------------------------------------------------------------------
//
TReaderID CScardNotifier::ReaderId()
    {
    return iReaderID;
    }
//  End of File  
