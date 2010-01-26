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
* Description:  Implements the functionality of RApdu class
*
*/



// INCLUDE FILES 
#include    "SwimEtelApdu.h"
#include    "etelext.h"         // EEtelPanicNullHandle
#include    "WimTrace.h"        // for trace logging
 
#ifdef _DEBUG
#include    <flogger.h>
#endif

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// RApdu::RApdu
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
RApdu::RApdu()
    {
    _WIMTRACE(_L("WIM|SwimReader|RApdu::RApdu|Begin"));
    }

// -----------------------------------------------------------------------------
// RApdu::ConstructL
// Symbian 2nd phase constructor can leave. 
// Virtual function which extensions may overload. Called in Open()
// -----------------------------------------------------------------------------
//
void RApdu::ConstructL()
    {
    _WIMTRACE(_L("WIM|SwimReader|RApdu::ConstructL|Begin"));
    //Create new CustomAPI instance
    iCustomApi = new( ELeave ) RMmCustomAPI;
    }


// -----------------------------------------------------------------------------
// RApdu::APDUReq
// Message Transmission
// -----------------------------------------------------------------------------
//
void RApdu::APDUReq( TRequestStatus& aStatus, RMmCustomAPI::TApdu& aMsg )
    {
    _WIMTRACE(_L("WIM|SwimReader|RApdu::APDUReq|Begin"));
    aStatus = KRequestPending;
    // Send APDU to Etel through CustomAPI
    iCustomApi->SendAPDUReq( aStatus, aMsg );
    }

// -----------------------------------------------------------------------------
// RApdu::CancelAPDUReq
// Cancel Message Transmission
// -----------------------------------------------------------------------------
//
void RApdu::CancelAPDUReq()
    {
	iCustomApi->CancelAsyncRequest( ECustomSendAPDUReqIPC );
    }

// -----------------------------------------------------------------------------
// RApdu::Destruct
// Destructor. Called in Close()
// -----------------------------------------------------------------------------
//
void RApdu::Destruct()
    {
    _WIMTRACE(_L("WIM|SwimReader|RApdu::Destruct|Begin"));
    if ( iCustomApi )
    	{
    	iCustomApi->Close();
        delete iCustomApi;
        iCustomApi = NULL;
    	} 
    }

// -----------------------------------------------------------------------------
// RApdu::Close
// Close a phone and connection to CustomAPI
// -----------------------------------------------------------------------------
//
void RApdu::Close()
    {
    _WIMTRACE(_L("WIM|SwimReader|RApdu::Close|Begin"));
    CloseSubSession( EEtelClose );
    iPhone.Close();
    if ( iCustomApi )
    	{
        iCustomApi->Close();
    	}   
    Destruct();
    }

// -----------------------------------------------------------------------------
// RApdu::Open
// Open a phone by name and CustomAPI session
// -----------------------------------------------------------------------------
//
TInt RApdu::Open( RTelServer& aServer, const TDesC &aName )
    {
    _WIMTRACE(_L("WIM|SwimReader|RApdu::Open|Begin"));
    __ASSERT_ALWAYS( aServer.Handle() != 0, 
        PanicClient( EEtelPanicNullHandle ) );
    __ASSERT_ALWAYS( aName.Length() != 0, PanicClient( KErrBadName ) );

    TRAPD( ret, ConstructL() );
    if ( ret )
        {
        Destruct();
        return ret;
        }

    //RMobilePhone phone;

    // Open phone
    ret = iPhone.Open( aServer, aName );
            
    if ( ret != KErrNone )
        {
#ifdef _DEBUG        
        RFileLogger::WriteFormat( KSwimReaderLogDir, KSwimReaderLogFileName,
        EFileLoggingModeAppend, 
        _L( "EtelApdu::Open: phone open failed: %d" ), 
        ret );
#endif
        Destruct();
        iPhone.Close();
        return ret;
        }
    
    // Open new CustomAPI session for phone
    ret = iCustomApi->Open( iPhone );
    
    if ( ret != KErrNone )
        {
#ifdef _DEBUG        
        RFileLogger::WriteFormat( KSwimReaderLogDir, KSwimReaderLogFileName,
        EFileLoggingModeAppend, 
        _L( "EtelApdu::Open: CustomAPI open failed: %d" ), 
        ret );
#endif
        Destruct();
        iPhone.Close();
        return ret;
        }

    return ret;
    }

// -----------------------------------------------------------------------------
// PanicClient
// Panic the client on client side
// -----------------------------------------------------------------------------
//
void PanicClient( TInt aFault )
    {
    _WIMTRACE(_L("WIM|SwimReader|RApdu::PanicClient|Begin"));
    _LIT( KETelClientFault,"Etel Client Fault" );
    User::Panic( KETelClientFault, aFault );
    }

// End of file
