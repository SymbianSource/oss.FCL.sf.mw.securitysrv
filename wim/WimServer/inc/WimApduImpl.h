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
* Description:  APDU Interface between C and Symbian code. Implementations
*               for WimApdu callbacks.
*
*/


#ifndef CWIMAPDU_H
#define CWIMAPDU_H

//  INCLUDES
#include "Scard.h"
#include "WimScardListener.h"


// CONSTANTS
const TInt KMaxApduLen     = 262;
const TInt KMaxReaderCount = 8;

const TInt KDefaulCloseChannelTimeout = 0;  //Timeout in microseconds
const TUint8 KManageChannelIns        = 0x70;
const TUint8 KManageChannelParamOpen  = 0x00;
const TUint8 KManageChannelParamClose = 0x80;


// FORWARD DECLARATIONS
class CScardComm;
class RScard;


// CLASS DECLARATION

/**
*  WIM APDU interface.
*  Interface between C and Symbian code. 
*  Methods for sending and retrieve APDUs.
*
*  @since Series60 2.1
*/
class CWimApdu : public CActive
    {
    public:  // Constructors and destructor

        /**
        * Destructor.
        */
         virtual ~CWimApdu();

        /**
        * Two-phased constructor.
        */
        static CWimApdu* NewL();


    public: // New functions

        /**
        * Opens a connection to a reader.
        * @param aUiReaderId Reader ID
        * @return status of the operation.
        */
        TUint8 Open( TUint8 aUiReaderId );

        /**
        * Closes a connection to a reader.
        * @param aUiReaderId Reader ID
        * @return status of the operation.
        */
        TUint8 Close( TUint8 aUiReaderId );

        /**
        * Sends an ISO 7816 APDU to reader.
        * @param aUiReaderId Reader ID
        * @param aApdu APDU to be sent
        * @param aUiApduLength legth of the APDU
        * @return status of the operation
        */
        TUint8 SendAPDU( TUint8  aUiReaderId,
                         TUint8* aApdu,
                         TUint16 aUiApduLength );
        /**
        * Cancel apdu sending
        */                 
        void CancelApduSending();                 

        /**
        * Gets a list of reader statusses.
        * @return status of the operation
        */
        TUint8 RequestListL();

        /**
        * Returns response APDU from latest SendAPDU call, 
        * does not return SW-bytes.
        * @return response APDU
        */
        TPtrC8 ResponseApdu() const;

        /**
        * Returns SW-bytes from latest SendApdu call.
        * @return SW bytes
        */
        TUint16 ResponseSW() const;

        /**
        * Returns a list of reader statusses from 
        * latest RequestList call.
        * @return reader status list
        */
        const TBuf8<KMaxReaderCount>& StatusList() const;

        /**
        * Returns the length of reader status list.
        * @return lenght of reader status list.
        */
        TUint8 StatusListLength() const;

    private:

        /**
        * C++ default constructor.
        */
        CWimApdu();
        
        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        /**
        * From CActive. Stops ActiveSchedulerWait when got APDU response.
        * @return void
        */
        void RunL();

        /**
        * From CActive. Cancellation function
        * @return void
        */
        void DoCancel();

        /**
        * Wait until asynchronous call is completed
        * @return void
        */
        void SetActiveAndWait();

    private:    // Data

        // APDU responses are stored here. Owned.
        HBufC8*                           iResponseBuffer;
        // SW-bytes of the response.
        TUint16                           iResponseSW;
        // Statusses of the readers found
        TBuf8<KMaxReaderCount>            iReaderStatusses;
        // How many readers found.
        TUint8                            iReaderStatusLength;
        // Status of the response. Possible values in WimDefs.h.
        TUint8                            iResponseStatus;
        // Array containing CSardComm* object for each reader found. Owned.
        CArrayFixFlat<CScardComm*>*       iReaderComm;
        // Array of supported reader names
        CArrayFixFlat<TScardReaderName>*  iReaderNames;
        //Pointer to CScard
        RScard*                           iServer;
        // Array of Scard listeners
        CArrayPtrFlat<CWimScardListener>* iWimScardListenerArray;
        // Synchronous wait for async request
        CActiveSchedulerWait              iWait;
        
        TUint8                            iUiReaderId;

    };

#endif      // CWIMAPDU_H

// End of File
