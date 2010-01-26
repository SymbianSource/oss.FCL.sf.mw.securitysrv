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
* Description:  Timer for connections to smartcard
*
*/



// INCLUDE FILES
#include    "ScardConnector.h"
#include    "ScardConnectionTimer.h"
#include    "WimTrace.h"

#ifdef _DEBUG // for logging
#include    "ScardLogs.h"
#include    <flogger.h> 
#endif


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CScardConnectionTimer::CScardConnectionTimer
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CScardConnectionTimer::CScardConnectionTimer(
    CScardConnector* aConnector, 
    TInt32 aTimeOut )
    : CActive( EPriorityNormal ), 
      iTimeOut( aTimeOut ), 
      iConnector( aConnector )
    {
    _WIMTRACE(_L("WIM|Scard|CScardConnectionTimer::CScardConnectionTimer|Begin"));
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CScardConnectionTimer::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CScardConnectionTimer::ConstructL()
    {
    _WIMTRACE(_L("WIM|Scard|CScardConnectionTimer::ConstructL|Begin"));
    iTimer = new( ELeave ) RTimer;
    iTimer->CreateLocal();
#ifdef _DEBUG    
    RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
        EFileLoggingModeAppend, 
        _L( "CScardConnectionTimer::ConstructL\n" ) );
#endif
    iTimer->After( iStatus, iTimeOut );
    SetActive();
    }

// -----------------------------------------------------------------------------
// CScardConnectionTimer::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CScardConnectionTimer* CScardConnectionTimer::NewL(
    CScardConnector* aConnector, 
    TInt32 aTimeOut )
    {
    _WIMTRACE(_L("WIM|Scard|CScardConnectionTimer::NewL|Begin"));
    __ASSERT_ALWAYS( aTimeOut >= 0, User::Leave( KScErrBadArgument ) );
    CScardConnectionTimer* self = new( ELeave ) CScardConnectionTimer(
        aConnector, aTimeOut );
    
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    return self;
    }

    
// Destructor
CScardConnectionTimer::~CScardConnectionTimer()
    {
    _WIMTRACE(_L("WIM|Scard|CScardConnectionTimer::~CScardConnectionTimer|Begin"));
#ifdef _DEBUG    
    RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
        EFileLoggingModeAppend, 
        _L( "CScardConnectionTimer::Destructor\n" ) );
#endif
    Cancel();
    delete iTimer;    
    }


// -----------------------------------------------------------------------------
// CScardConnectionTimer::RunL
// -----------------------------------------------------------------------------
//
void CScardConnectionTimer::RunL()
    {
    _WIMTRACE(_L("WIM|Scard|CScardConnectionTimer::RunL|Begin"));
#ifdef _DEBUG    
    RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
        EFileLoggingModeAppend, 
        _L( "CScardConnectionTimer::RunL\n" ) );
#endif
    iConnector->ConnectionTimedOut();
    }

// -----------------------------------------------------------------------------
// CScardConnectionTimer::DoCancel
// -----------------------------------------------------------------------------
//
void CScardConnectionTimer::DoCancel()
    {
    _WIMTRACE(_L("WIM|Scard|CScardConnectionTimer::DoCancel|Begin"));
#ifdef _DEBUG    
    RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
        EFileLoggingModeAppend, 
        _L( "CScardConnectionTimer::DoCancel\n" ) );
#endif
    iTimer->Cancel();
    }

//  End of File  
