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
* Description:  Handles messaging with stack
*
*/



// INCLUDE FILES
#include    "ScardBase.h"
#include    "ScardMessageStack.h"
#include    "ScardConnector.h"
#include    "WimTrace.h"

#ifdef _DEBUG // for logging
#include    "ScardLogs.h"
#include    <flogger.h> 
#endif


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CScardMessageStack::CScardMessageStack
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CScardMessageStack::CScardMessageStack()
    {
    _WIMTRACE(_L("WIM|Scard|CScardMessageStack::CScardMessageStack|Begin"));
    }

// -----------------------------------------------------------------------------
// CScardMessageStack::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CScardMessageStack::ConstructL()
    {
    _WIMTRACE(_L("WIM|Scard|CScardMessageStack::ConstructL|Begin"));
    iMessages = new( ELeave ) CArrayFixFlat<TMessageHandle>( 3 );
    }

// -----------------------------------------------------------------------------
// CScardMessageStack::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CScardMessageStack* CScardMessageStack::NewL()
    {
    _WIMTRACE(_L("WIM|Scard|CScardMessageStack::NewL|Begin"));
    CScardMessageStack* self = new( ELeave ) CScardMessageStack();
    
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    return self;
    }

    
// Destructor
CScardMessageStack::~CScardMessageStack()
    {
    _WIMTRACE(_L("WIM|Scard|CScardMessageStack::~CScardMessageStack|Begin"));
    CancelAll( KScErrCancelled );
    delete iMessages;
    }


// -----------------------------------------------------------------------------
// CScardMessageStack::CancelAll
// Cancel all operations for this session ID.
// -----------------------------------------------------------------------------
//
void CScardMessageStack::CancelAll(
    const TInt aSessionID, 
    const TInt aReason )
    {
    _WIMTRACE(_L("WIM|Scard|CScardMessageStack::CancelAll|Begin"));
    if ( iMessages->Count() )
        {
        //  First check if the message currently serviced is one of 
        //  the cancelled ones (it requires special treatment)
        if ( (*iMessages)[0].iSessionID == aSessionID ) 
            {
            CancelOne( 0, aReason );
            }
        //  Then pop out all the others
        for ( TInt i( 1 ); i < iMessages->Count(); i++ ) 
            {
            if ( (*iMessages)[i].iSessionID == aSessionID )
                {
                CancelOne( i, aReason );
                iMessages->Delete( i );
                i--;
                } // if
            } // for
        } // if
    }

// -----------------------------------------------------------------------------
// CScardMessageStack::CancelAll
// Cancel all messages waiting for service. Delete all others except the 
// first one
// -----------------------------------------------------------------------------
//
void CScardMessageStack::CancelAll( const TInt aReason )
    {
    _WIMTRACE(_L("WIM|Scard|CScardMessageStack::CancelAll|Begin"));
    if ( iMessages->Count() )
        {
        //  Just cancel the first
        CancelOne( 0, aReason );
    
        //  Then pop out all the others
        for ( TInt i( 1 ); i < iMessages->Count(); i++ )
            {
            CancelOne( i, aReason );
            iMessages->Delete( i );
            i--;
            }
        }
    }

// -----------------------------------------------------------------------------
// CScardMessageStack::CancelCardOperations
// Effectively the same as the above, except that does not cancel 
// OpenReader commands...
// -----------------------------------------------------------------------------
//
void CScardMessageStack::CancelCardOperations()
    {
    _WIMTRACE(_L("WIM|Scard|CScardMessageStack::CancelCardOperations|Begin"));
    if ( iMessages->Count() )
        {
        //  Just cancel the first
        if ( iMessages->At( 0 ).iAdditionalParameter != KOpenReader )
            {
            CancelOne( 0, KScReaderErrNoCard );
            }
    
        //  Then pop out all the others
        for ( TInt i( 1 ); i < iMessages->Count(); i++ )
            {
            if ( iMessages->At( i ).iAdditionalParameter != KOpenReader )
                {
                CancelOne( i, KScReaderErrNoCard );
                iMessages->Delete( i );
                i--;
                }
            }
        }
    }

// -----------------------------------------------------------------------------
// CScardMessageStack::CancelByTimeOut
// Cancel message by timeout
// -----------------------------------------------------------------------------
//
void CScardMessageStack::CancelByTimeOut( const CScardCommandTimer* aTimer )
    {
    _WIMTRACE(_L("WIM|Scard|CScardMessageStack::CancelByTimeOut|Begin"));
#ifdef _DEBUG
    RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
        EFileLoggingModeAppend, _L( "CScardMessageStack::CancelByTimeOut\n" ) );
#endif

    if ( iMessages->Count() )
        {
        //  First check if the message currently serviced is the cancelled one
        //  (it requires special treatment)
        if ( (*iMessages)[0].iTimer == aTimer )
            {
            CancelOne( 0, KScErrTimeOut, EFalse );
            return;
            }

        //  Then check out all the others
        for ( TInt i( 1 ); i < iMessages->Count(); i++ )
            {
            if ( (*iMessages)[i].iTimer == aTimer )
                {
                CancelOne( i, KScErrTimeOut, EFalse );
                iMessages->Delete( i );
                return;
                } // if
            } // for
        } // if
    }

// -----------------------------------------------------------------------------
// CScardMessageStack::PushToTop
// Push message to top
// -----------------------------------------------------------------------------
//
TInt CScardMessageStack::PushToTop( const TMessageHandle& aMessage )
    {
    _WIMTRACE(_L("WIM|Scard|CScardMessageStack::PushToTop|Begin"));
    TRAPD( err, iMessages->InsertL( 0, aMessage ) );
    _WIMTRACE2(_L("WIM|Scard|CScardMessageStack::PushToTop|End|err=%d"), err);
    return err;
    }

// -----------------------------------------------------------------------------
// CScardMessageStack::FromTop
// Message from top
// -----------------------------------------------------------------------------
//
const TMessageHandle CScardMessageStack::FromTop()
    {
    _WIMTRACE(_L("WIM|Scard|CScardMessageStack::FromTop|Begin"));
    if ( !iMessages->Count() )
        {
        //  returns the default, which is interpreted as an error
        TMessageHandle a;
        return a;
        }
    TMessageHandle tmp = (*iMessages)[0];
    iMessages->Delete( 0 );
    return tmp;
    }

// -----------------------------------------------------------------------------
// CScardMessageStack::PushToBottom
// Push message to bottom
// -----------------------------------------------------------------------------
//
TInt CScardMessageStack::PushToBottom( const TMessageHandle& aMessage )
    {
    _WIMTRACE(_L("WIM|Scard|CScardMessageStack::PushToBottom|Begin"));
    TRAPD( err, iMessages->AppendL( aMessage ) );
    return err;
    }

// -----------------------------------------------------------------------------
// CScardMessageStack::NextReservation
// This function is called when a channel is freed. It gives priority
// for applications who are waiting to reserve either this channel or
// any channel
// -----------------------------------------------------------------------------
//
const TMessageHandle CScardMessageStack::NextReservation( const TInt8 aChannel )
    {
    _WIMTRACE(_L("WIM|Scard|CScardMessageStack::NextReservation|Begin"));
    TMessageHandle tmp;
    for ( TInt i( 0 ); i < iMessages->Count(); i++ )
        {
        tmp = (*iMessages)[i];
        if ( ( tmp.iAdditionalParameter & KReservation ) && 
           ( tmp.iChannel == aChannel || tmp.iChannel == KAllChannels ) )
            {
            iMessages->Delete( i );
            return tmp;
            }
        }
    tmp.iSessionID = ENoSession;
    return tmp;
    }

// -----------------------------------------------------------------------------
// CScardMessageStack::FromPositionL
// Get message from given position
// -----------------------------------------------------------------------------
//
const TMessageHandle CScardMessageStack::FromPositionL( const TInt aPosition )
    {
    _WIMTRACE(_L("WIM|Scard|CScardMessageStack::FromPositionL|Begin"));
    __ASSERT_ALWAYS( aPosition >= 0 && aPosition < iMessages->Count(), 
        User::Leave( KScErrBadArgument ) );
    TMessageHandle r = iMessages->At( aPosition );
    iMessages->Delete( aPosition );
    return r;
    }

// -----------------------------------------------------------------------------
// CScardMessageStack::PushToPositionL
// Push message to given position
// -----------------------------------------------------------------------------
//
void CScardMessageStack::PushToPositionL(
    const TInt aPosition, 
    TMessageHandle& aHandle )
    {
    _WIMTRACE(_L("WIM|Scard|CScardMessageStack::PushToPositionL|Begin"));
    __ASSERT_ALWAYS( aPosition >= 0 && aPosition <= iMessages->Count(), 
        User::Leave( KScErrBadArgument ) );
    iMessages->InsertL( aPosition, aHandle );
    }

//  End of File
