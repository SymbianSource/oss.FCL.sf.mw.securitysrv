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
#include    "Scard.h"
#include    "ScardBase.h"
#include    "ScardListener.h"
#include    "ScardClsv.h"
#include    "WimTrace.h"


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
EXPORT_C CScardListener::CScardListener( RScard* aScard ) 
    : CActive( CActive::EPriorityStandard )
    {
    _WIMTRACE(_L("WIM|Scard|CScardListener::CScardListener|Begin"));
    __ASSERT_ALWAYS( aScard, User::Panic( _L( "NULL pointer" ), 
        KErrArgument ) );
    iScard = aScard;
    CActiveScheduler::Add( this );
    }

// Destructor
EXPORT_C CScardListener::~CScardListener()
    {
    _WIMTRACE(_L("WIM|Scard|CScardListener::~CScardListener|Begin"));
    Cancel(); // Make sure we're cancelled
    _WIMTRACE(_L("WIM|Scard|CScardListener::~CScardListener|End"));
    }


// -----------------------------------------------------------------------------
// CScardListener::ListenCardEvents
// Start listening card events
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CScardListener::ListenCardEvents(
    const TScardReaderName& aReaderName )
    {
    _WIMTRACE(_L("WIM|Scard|CScardListener::ListenCardEvents|Begin"));
    iReaderName.Copy( aReaderName );
    TInt err = NotifyChange( iStatus, aReaderName );
    SetActive();
    return err;
    }

// -----------------------------------------------------------------------------
// CScardListener::DoCancel
// Listening cancelled
// -----------------------------------------------------------------------------
//
EXPORT_C void CScardListener::DoCancel()
    {   
    _WIMTRACE(_L("WIM|Scard|CScardListener::DoCancel|Begin"));
    CancelNotifyChange(iReaderName );
    }

// -----------------------------------------------------------------------------
// CScardListener::RunL
// Handle event we have got
// -----------------------------------------------------------------------------
//
EXPORT_C void CScardListener::RunL()
    {
    _WIMTRACE(_L("WIM|Scard|CScardListener::RunL|Begin"));
    TScardServiceStatus event = (TScardServiceStatus) iStatus.Int();
    //iStatus = KRequestPending;
    //SetActive();
    ProcessEvent( event );
    _WIMTRACE(_L("WIM|Scard|CScardListener::RunL|END"));
    }

// -----------------------------------------------------------------------------
// CScardListener::NotifyChange
// Send NotifyChange opcode to server
// -----------------------------------------------------------------------------
//
TInt CScardListener::NotifyChange(
    TRequestStatus& aStatus,
    const TScardReaderName& aFriendlyName )
    {
    _WIMTRACE(_L("WIM|Scard|CScardListener::NotifyChange|Begin"));
    aStatus = KRequestPending;
    iPckg = &aStatus;
    //TPckgBuf<TRequestStatus*> pckg( &aStatus );

    TIpcArgs args( &iPckg, &aFriendlyName );
    iScard->SendReceive( EScardServerNotifyChange, args, aStatus );
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CScardListener::CancelNotifyChange
// Send CancelNotifyChange opcode to server
// -----------------------------------------------------------------------------
//
/*void CScardListener::CancelNotifyChange(
    TRequestStatus& aStatus,
    const TScardReaderName& aFriendlyName )
    {
    _WIMTRACE(_L("WIM|Scard|CScardListener::CancelNotifyChange|Begin"));
    
    //TPckgBuf<TRequestStatus*> pckg( &aStatus );
    iPckg = &aStatus;
    
    TIpcArgs args( &iPckg, &aFriendlyName );
    
    iScard->SendReceive( EScardServerCancelNotifyChange, args );
    _WIMTRACE(_L("WIM|Scard|CScardListener::CancelNotifyChange|End"));
    }*/
    

TInt CScardListener::CancelNotifyChange( const TScardReaderName& aFriendlyName )
    {
    _WIMTRACE(_L("WIM|Scard|CScardListener::CancelNotifyChange|Begin"));
    
    //this parameter is not used in server side
    //It will be useful if there is a multiple scard listener
    //currently only wimserver is the scard listener, so by calling
    //this function, the array of listener will be cleaned.
    TIpcArgs args( &iPckg, &aFriendlyName );
    
    return iScard->SendReceive( EScardServerCancelNotifyChange, args );
    _WIMTRACE(_L("WIM|Scard|CScardListener::CancelNotifyChange|End"));
    }

//  End of File  
