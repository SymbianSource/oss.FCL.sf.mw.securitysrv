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
* Description:  For reading SWIM
*
*/


#ifndef CSWIMREADER_H
#define CSWIMREADER_H

//  INCLUDES
#include "ScardDefs.h"          // KMaxATRHistoricalBytes
#include "SwimEtelApdu.h"       // RApdu
#include "SwimReaderConsts.h"
#include "SwimApduRespHeader.h"
#include "SwimApduReqHeader.h"


//  FORWARD DECLARATIONS
class CSwimReaderIF;

// CLASS DECLARATION

/**
*  Functional level of SwimReader
*  
*
*  @lib SwimReader.lib
*  @since Series60 2.1
*/
class CSwimReader : public CActive
    {
    public:  // Constructors and destructor
    
        /**
        * Two-phased constructor.
        * @param aInterface Pointer to SwimReaderIF
        */
        static CSwimReader* NewL( CSwimReaderIF* aInterface );
         
        /**
        * Destructor.
        */
        virtual ~CSwimReader();
        
    public: // New functions

        /**
        * APDU Request
        * @param aStatus Status of request
        * @param aCommandAPDU Command APDU 
        * @param aResponseAPDU Response APDU
        * @param aServiceType Service type
        * @param aCardReader Card reader
        * @param aAppType Application type
        * @param aPaddingByte Padding byte
        * @return
        */
        void APDUReqL( TRequestStatus& aStatus,
                       const TPtrC8& aCommandAPDU,
                       TPtr8& aResponseAPDU,
                       TUint8 aServiceType = KSendApdu,
                       TUint8 aCardReader = KNoPreferredReader,
                       TUint8 aAppType = KAPDUAppTypeWIM,
                       TUint8 aPaddingByte = 0 );
        
        /**
        * Close Etel and CustomAPI connection
        * @return void
        */
        void Close();
        
        /**
        * Get preferred reader status from PreferredReaderStatus member
        * @param aReaderStatus Reader's status
        * @return void
        */
        void PreferredReaderStatus( TUint8& aReaderStatus );
        
        /**
        * From given status bytes, select a card reader,
        * which is present, and card is also present in the reader.
        * @param aReaderStatuses Reader statys bytes
        * @param aPreferredReaderStatus Preferred reader status
        * @param aOldPreferredReader Old preferred reader
        * @return Reader number which was selected
        */
        static TUint8 SelectCardReader(
            TDesC8&  aReaderStatuses,
            TUint8& aPreferredReaderStatus,
            TUint8  aOldPreferredReader = KNoPreferredReader );
        
        /**
        * Sets iCardInserted member variable according the attribute.
        * @param aIsInserted is the card inserted (ETrue / EFalse)
        * @return void
        */
        void SetCardInserted( TBool aIsInserted );
        
        /**
        * Initializes the reader. Also make APDU_LIST to get all 
        * statuses of card readers attached to the device.
        * @param aStatus Request status
        * @param aHistoricals Historical ATR bytes
        * @return void
        */
        void WakeUpL( TRequestStatus& aStatus, TDes8& aHistoricals );

    private:

        /**
        * C++ default constructor.
        * @param aInterface Pointer to SwimReaderIF
        */
        CSwimReader( CSwimReaderIF* aInterface );
         
        /**
        * Symbian 2nd phase constructor.
        */
        void ConstructL();

        /**
        * APDU request
        * @param aCommandAPDU Command APDU
        * @param aResponseAPDU Response APDU
        * @param aServiceType Service type
        * @param aCardReader Card reader number
        * @param aAppType Application type
        * @param aPaddingByte Padding byte
        * @return void
        */
        void APDUReqL( const TPtrC8& aCommandAPDU,
                       TPtr8& aResponseAPDU,
                       TUint8 aServiceType = KSendApdu,
                       TUint8 aCardReader = KNoPreferredReader,
                       TUint8 aAppType = KAPDUAppTypeWIM,
                       TUint8 aPaddingByte = 0 );

        /**
        * APDU request
        * @param aCommandAPDU Command APDU
        * @param aResponseAPDU Response APDU
        * @param aReqHeader APDU request header
        * @return void
        */
        void APDUReqL( const TPtrC8& aCommandAPDU,
                       TPtr8& aResponseAPDU,
                       TSwimApduReqHeader& aReqHeader );

        /**
        * Cleanup all allocated stuff
        * @return void
        */
        void CleanUp();
        
        /**
        * Handle APDU request
        * @return Scard error code
        */
        TInt HandleAPDUReq();
        
        /**
        * Handle open APDU request
        * @return Scard error code
        */
        TInt HandleOpenAPDUReq();
        
        /**
        * From CActive RunL
        * @return void
        */
        void RunL();

        /**
        * From CActive Cancel request 
        * @return void
        */
        void DoCancel();

    private:    // Data
        // Enumerator for reader phase
        enum TReaderPhase
            {
            EWakeUpAPDURequest,
            EAPDURequest
            };

        // RApdu class
        RApdu                       iApdu;
        // Status for Apdu
        TUint8                      iApduStatus;
        // Is the card inserted
        TBool                       iCardInserted;
        // Pointer for client status. Not owned.
        TRequestStatus*             iClientStatus;
        // Cmd bytes
        TBufC8<1>                   iCmdBytes;
        // Pointer to Etel server. Owned.
        RTelServer*                 iEtelServer;
        // Header for APDU
        TSwimApduReqHeader          iHeader;
        //
        TBuf8<KMaxATRHistoricalBytes> iHistoricals;
        // Pointer for SwimReaderIF object. Not owned.
        CSwimReaderIF*              iInterface;
        // Phase of reader operations
        TReaderPhase                iPhase;
        // Max length of APDU
        TInt                        iMaxLen;
        // Pointer to TApdu message. Owned.
        RMmCustomAPI::TApdu*        iMsg;
        // Is reader open
        TBool                       iOpen;
        // Prefferred reader
        TUint8                      iPreferredReader;
        // Prefferred reader status
        TUint8                      iPreferredReaderStatus;
        // Is reader attached
        TBool                       iReaderAttached;
        // Buffer for reader statusses
        HBufC8*                     iReaderStatuses;
        // Pointer to reader statusses. Owned.
        TPtr8*                      iReaderStatusesPtr;
        // Request header
        TSwimApduReqHeader*         iReqHeader;
        // Pointer to Response APDU. Not owned.
        TPtr8*                      iResponseAPDU;
        // Temporary buffer. Owned.
        HBufC8*                     iTempBuf;
        // Pointer to Temporary buffer
        TPtr8*                      iTempPtr;
    };

#endif      // CSWIMREADER_H

// End of File
