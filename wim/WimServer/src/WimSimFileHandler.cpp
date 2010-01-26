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
* Description:  Handler for Wimlib request for SIM file reading. Calls 
*               services from CustomAPI to read file and returns read file
*               to WIMI (and Wimlib)
*
*/



// INCLUDE FILES
#include    "WimSimFileHandler.h"
#include    "Wimi.h"                // WIMI
#include    "WimTrace.h"
#include    <mmtsy_names.h>         // TSY and Phone name


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWimSimFileHandler::CWimSignTextHandler
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CWimSimFileHandler::CWimSimFileHandler() : CActive( EPriorityStandard )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSimFileHandler::CWimSimFileHandler | Begin"));
    }

// -----------------------------------------------------------------------------
// CWimSimFileHandler::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CWimSimFileHandler::ConstructL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSimFileHandler::ConstructL | Begin"));
    CActiveScheduler::Add( this );

    //Create new CustomAPI instance
    iCustomApi = new( ELeave ) RMmCustomAPI;
    OpenCustomApiL(); // Open connection to CustomAPI
    }

// -----------------------------------------------------------------------------
// CWimSimFileHandler::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CWimSimFileHandler* CWimSimFileHandler::NewL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSimFileHandler::NewL | Begin"));
    CWimSimFileHandler* self = new( ELeave ) CWimSimFileHandler;
    
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    _WIMTRACE(_L("WIM | WIMServer | CWimSimFileHandler::NewL | End"));
    return self;
    }
  
// Destructor
CWimSimFileHandler::~CWimSimFileHandler()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSimFileHandler::~CWimSimFileHandler | Begin"));
    if ( iCustomApi )
        {
        iCustomApi->Close();
        }
    if ( iEtelServer )
        {
        iEtelServer->Close();
        }
    delete iCustomApi;
    delete iEtelServer;
    Cancel();
    delete iResponseBytesBuf;
    delete iResponseBytesPtr;
    _WIMTRACE(_L("WIM | WIMServer | CWimSimFileHandler::~CWimSimFileHandler | End"));
    }

// -----------------------------------------------------------------------------
// CWimSimFileHandler::OpenCustomApiL
// Open CustomAPI for communication
// -----------------------------------------------------------------------------
//  
void CWimSimFileHandler::OpenCustomApiL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSimFileHandler::OpenCustomApiL | Begin"));
    iEtelServer = new( ELeave ) RTelServer();
    
    RMobilePhone phone;

    // Connect to the Etel server.
    TInt ret = iEtelServer->Connect();
    
    if ( ret != KErrNone )
        {
        _WIMTRACE2(_L("WIM | WIMServer | CWimSimFileHandler::OpenCustomApiL | Connection to etel server failed: %d"), ret);
        User::Leave( ret );
        }

    // Load phone module.
    ret = iEtelServer->LoadPhoneModule( KMmTsyModuleName );
    
    if ( ret != KErrNone )
        {
        _WIMTRACE2(_L("WIM | WIMServer | CWimSimFileHandler::OpenCustomApiL | loading phone module failed: %d"), ret);
        User::Leave( ret );
        }
    
    // Open phone
    ret = phone.Open( *iEtelServer, KMmTsyPhoneName );
         
    if ( ret != KErrNone )
        {
        _WIMTRACE2(_L("WIM | WIMServer | CWimSimFileHandler::OpenCustomApiL | phone.Open failed: %d"), ret);
        User::Leave( ret );
        }
    
    // Open new CustomAPI session for phone
    ret = iCustomApi->Open( phone );

    phone.Close(); // Close phone

    if ( ret != KErrNone )
        {
        _WIMTRACE2(_L("WIM | WIMServer | CWimSimFileHandler::OpenCustomApiL | iCustomApi->Open failed: %d"), ret);
        User::Leave( ret );
        }
    }

// -----------------------------------------------------------------------------
// CWimSimFileHandler::ReadSimFileL
// Read SIM file using CustomAPI's ReadSimFile() method.
// -----------------------------------------------------------------------------
//
void CWimSimFileHandler::ReadSimFileL(
    TUint8 aReader,
    const TDes8& aPath,
    TUint16 aOffset,
    TUint16 aSize )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSimFileHandler::ReadSimFileL | Begin"));

    iReaderNumber = aReader;

    TInt size;
    if ( aSize == 0 ) // Read whole file
        {
        size = KMaxSimFileSize;
        }
    else
        {
        // If size is bigger than maximum file size declared in header file use
        // declared max size
        if ( aSize > KMaxSimFileSize )
            {
            size = KMaxSimFileSize;
            }
        else
            {
            size = aSize;
            }
        }

    iSimFileInfo.iPath = aPath;
    iSimFileInfo.iOffSet = aOffset;
    iSimFileInfo.iSize = ( TUint16 )size;

    RMmCustomAPI::TSimFileInfoPckg simFileInfoPckg( iSimFileInfo );

    iResponseBytesBuf = HBufC8::NewL( size );
    iResponseBytesPtr = new( ELeave ) TPtr8( NULL, 0, size );
    iResponseBytesPtr->Set( iResponseBytesBuf->Des() );

    iStatus = KRequestPending;

    _WIMTRACE9(_L("WIM | WIMServer | CWimSimFileHandler::ReadSimFileL | Path = %02x%02x %02x%02x %02x%02x %02x%02x "),
        iSimFileInfo.iPath[0], iSimFileInfo.iPath[1], iSimFileInfo.iPath[2], iSimFileInfo.iPath[3],
        iSimFileInfo.iPath[4], iSimFileInfo.iPath[5], iSimFileInfo.iPath[6], iSimFileInfo.iPath[7] );

    _WIMTRACE2(_L("WIM | WIMServer | CWimSimFileHandler::ReadSimFileL | Offset = 0x%04x"), iSimFileInfo.iOffSet );

    iCustomApi->ReadSimFile( iStatus, simFileInfoPckg, *iResponseBytesPtr );
    
    SetActiveAndWait(); // Wait request to be completed

    _WIMTRACE(_L("WIM | WIMServer | CWimSimFileHandler::ReadSimFileL | End"));
    }

// -----------------------------------------------------------------------------
// CWimSimFileHandler::FileReceivedL
// Handle received file
// -----------------------------------------------------------------------------
//
void CWimSimFileHandler::FileReceived()
    {
    _WIMTRACE2(_L("WIM | WIMServer | CWimSimFileHandler::FileReceived | Begin, error: %d"), iStatus.Int());
    WIMI_BinData_t responseBytes;
    
    TInt error = iStatus.Int();
    TUint8 status;

    switch ( error )
        {
        case KErrPathNotFound:
            {
            _WIMTRACE(_L("WIM | WIMServer | CWimSimFileHandler::FileReceived | File not found"));
            break;
            }
        case KErrNotFound:
            {
            _WIMTRACE(_L("WIM | WIMServer | CWimSimFileHandler::FileReceived | No data found from file"));
            break;
            }
        case KErrUnknown:
            {
            _WIMTRACE(_L("WIM | WIMServer | CWimSimFileHandler::FileReceived | Unknown error"));
            break;
            }
        default:
            {
            _WIMTRACE2(_L("WIM | WIMServer | CWimSimFileHandler::FileReceived | Error: %d"), error);
            break;
            }
        }

    if ( error != KErrNone ) // Error occurred in SIM file reading
        {
        responseBytes.pb_buf = NULL;
        responseBytes.ui_buf_length = 0;
        status = WIMI_Err;
        }
    else  // SIM file read succesfully
        {
        responseBytes.pb_buf = ( TUint8* )iResponseBytesPtr->Ptr();
        responseBytes.ui_buf_length = ( TUint16 )iResponseBytesPtr->Length();
        status = WIMI_Ok;
        }

    _WIMTRACE2(_L("WIM | WIMServer | CWimSimFileHandler::FileReceived | FileLength = %d"), responseBytes.ui_buf_length);
    
    // Pass data to WIMI
    WIMI_SIM_ReadFileResp( iReaderNumber, status, &responseBytes );

    delete iResponseBytesBuf;
    iResponseBytesBuf = NULL;
    delete iResponseBytesPtr;
    iResponseBytesPtr = NULL;
    
    _WIMTRACE(_L("WIM | WIMServer | CWimSimFileHandler::FileReceived | End"));
    }

// -----------------------------------------------------------------------------
// CWimSimFileHandler::RunL
// Handle CustomAPI response
// -----------------------------------------------------------------------------
//
void CWimSimFileHandler::RunL()
    {
    _WIMTRACE2(_L("WIM | WIMServer | CWimSimFileHandler::RunL, status: %d"), iStatus.Int());
    iWait.AsyncStop();
    FileReceived();
    }

// -----------------------------------------------------------------------------
// CWimSimFileHandler::DoCancel
// Cancel asyncronous request
// -----------------------------------------------------------------------------
//
void CWimSimFileHandler::DoCancel()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSimFileHandler::DoCancel | Begin"));
    delete iResponseBytesBuf;
    iResponseBytesBuf = NULL;
    delete iResponseBytesPtr;
    iResponseBytesPtr = NULL;
    }

// -----------------------------------------------------------------------------
// CWimSimFileHandler::SetActiveAndWait
// Wait until asynchronous call is completed
// -----------------------------------------------------------------------------
//
void CWimSimFileHandler::SetActiveAndWait()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSimFileHandler::SetActiveAndWait | Begin"));
    SetActive();
    iWait.Start();
    _WIMTRACE(_L("WIM | WIMServer | CWimSimFileHandler::SetActiveAndWait | End"));
    }

//  End of File
