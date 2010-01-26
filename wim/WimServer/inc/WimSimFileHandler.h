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
* Description:  WIM SIM Certificate handler. Handles WIMI's ReadFile requests
*               to card not supporting logical channels
*
*/



#ifndef CWIMSIMFILEHANDLER_H
#define CWIMSIMFILEHANDLER_H

//  INCLUDES
#include <rmmcustomapi.h>

// CONSTANTS
const TUint KMaxSimFileSize = 10000; // Maximum size allocated for file

// CLASS DECLARATION

/**
*  Class for handling SIM file reading. Calls CustomAPI's function
*  to read the file.
*  
*  @since Series60 2.1
*/
class CWimSimFileHandler : public CActive
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CWimSimFileHandler* NewL();
        
        /**
        * Destructor.
        */
        virtual ~CWimSimFileHandler();

    public: // New functions
        
        /**
        * Reads SIM file
        * @param aReader     Reader ID
        * @param aPath       Path to the file to read
        * @param aOffset     Offset from the beginning of the file.
        * If the Elementary File pointed to by FileID is linear fixed or cyclic 
        * (this is the case for EF DIR), then the least significant byte of 
        * aOffSet denotes the record number, while the most significant byte in
        * aOffSet denotes the start byte within that record. The record number 
        * starts fron 1 and therefore 0x0100 is sent as the offset.
        * @param aSize       Amount of data to read, aSize = 0 means that whole
        *                    file must be read
        * @return void
        */
        void ReadSimFileL( TUint8 aReader,
                           const TDes8& aPath,
                           TUint16 aOffset,
                           TUint16 aSize );
        
    private:

        /**
        * C++ default constructor.
        */
        CWimSimFileHandler();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        /**
        * Open CustomAPI for communication
        * @return void
        */
        void OpenCustomApiL();

        /**
        * Handle received file
        * @return void
        */
        void FileReceived();

        /**
        * Wait until asynchronous call is completed
        * @return void
        */
        void SetActiveAndWait();

        /**
        * From CActive. Handles asynchrounous file reading response
        * @return void
        */
        void RunL();

        /**
        * From CActive. Cancellation function
        * @return void
        */
        void DoCancel();

        
    private:    // Data
        // Pointer to CustomAPI instance. Owned.
        RMmCustomAPI*               iCustomApi;
        // Pointer to Etel server. Owned.
        RTelServer*                 iEtelServer;
        // Pointer to response bytes buffer. Owned.
        HBufC8*                     iResponseBytesBuf;
        // Pointer to response bytes pointer. Owned.
        TPtr8*                      iResponseBytesPtr;
        // Reader number
        TUint8                      iReaderNumber;
        // Synchronous wait for async request
        CActiveSchedulerWait        iWait;
        // SIM file info structure
        RMmCustomAPI::TSimFileInfo  iSimFileInfo;

    };

#endif      // CWIMSIMFILEHANDLER_H
    
//End of File
