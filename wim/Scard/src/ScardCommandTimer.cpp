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
* Description:  Smart card command timer
*
*/



// INCLUDE FILES
#include    "ScardAccessControl.h"
#include    "ScardBase.h"
#include    "ScardCommandTimer.h"
#include    "WimTrace.h"

#ifdef _DEBUG // for logging
#include    "ScardLogs.h"
#include    <flogger.h> 
#endif

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CScardCommandTimer::CScardCommandTimer
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CScardCommandTimer::CScardCommandTimer()
    : CActive( EPriorityNormal )
    {
    _WIMTRACE(_L("WIM|Scard|CScardCommandTimer::CScardCommandTimer|Begin"));
    }

// -----------------------------------------------------------------------------
// CScardCommandTimer::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CScardCommandTimer::ConstructL(
    TInt32 aTimeOut, 
    CScardAccessControl* aController )
    {
    _WIMTRACE(_L("WIM|Scard|CScardCommandTimer::ConstructL|Begin"));
    iTimeOut = aTimeOut;
    iController = aController;
    iTimer = new( ELeave ) RTimer;
    iTimer->CreateLocal();
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CScardCommandTimer::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CScardCommandTimer* CScardCommandTimer::NewL(
    TInt32 aTimeOut, 
    CScardAccessControl* aController )
    {
    _WIMTRACE(_L("WIM|Scard|CScardCommandTimer::NewL|Begin"));
    __ASSERT_ALWAYS( aTimeOut >= 0, User::Leave( KScErrBadArgument ) );
    CScardCommandTimer* self = new( ELeave ) CScardCommandTimer;
    
    CleanupStack::PushL( self );
    self->ConstructL( aTimeOut, aController );
    CleanupStack::Pop( self );

    return self;
    }

    
// Destructor
CScardCommandTimer::~CScardCommandTimer()
    {
    _WIMTRACE(_L("WIM|Scard|CScardCommandTimer::~CScardCommandTimer|Begin"));
    if ( iTimer )
        {
        Cancel();
        delete iTimer;
        }
    }

// -----------------------------------------------------------------------------
// CScardCommandTimer::StartTiming
// Start the clock
// -----------------------------------------------------------------------------
//
void CScardCommandTimer::StartTiming()
    {
    _WIMTRACE(_L("WIM|Scard|CScardCommandTimer::StartTiming|Begin"));
#ifdef _DEBUG    
    RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
        EFileLoggingModeAppend, 
        _L( "CScardCommandTimer: start timer\n" ) );
#endif
    iTimer->After( iStatus, iTimeOut );
    SetActive();
    }

// -----------------------------------------------------------------------------
// CScardCommandTimer::RunL
// The timer finished, i.e. the command has timed out
// -----------------------------------------------------------------------------
//
void CScardCommandTimer::RunL()
    {
    _WIMTRACE(_L("WIM|Scard|CScardCommandTimer::RunL|Begin"));
#ifdef _DEBUG    
    RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
        EFileLoggingModeAppend, 
        _L( "CScardCommandTimer: timer expired\n" ) );
#endif
    iController->CancelByTimeOut( this );
    delete iTimer;
    iTimer = NULL;
    }

// -----------------------------------------------------------------------------
// CScardCommandTimer::DoCancel
// -----------------------------------------------------------------------------
//
void CScardCommandTimer::DoCancel()
    {
    _WIMTRACE(_L("WIM|Scard|CScardCommandTimer::DoCancel|Begin"));
#ifdef _DEBUG    
    RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
        EFileLoggingModeAppend, 
        _L( "CScardCommandTimer: cancel timer\n" ) );
#endif
    iTimer->Cancel();
    }

//  End of File  
