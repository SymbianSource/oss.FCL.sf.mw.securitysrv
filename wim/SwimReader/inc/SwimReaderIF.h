/*
* Copyright (c) 2003-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  SWIM reader interface
*
*/



#ifndef CSWIMREADERIF_H
#define CSWIMREADERIF_H

//  INCLUDES
#include "ScardReader.h"
#include "ScardNotifyObserver.h"

//  CONSTANTS  
const TUint8 KGetStatus = 0xf0;

//  FORWARD DECLARATIONS
class CSwimReader;
class CSwimReaderLauncher;
class CSwimSysAgentObserver;

// CLASS DECLARATION

/**
*  Implements SwimReader interface class
*  
*
*  @lib SwimReader.lib
*  @since Series60 2.1
*/
class CSwimReaderIF : public CActive, public MScardReader
    {
    public: // Constructors and destructor
       
        /**
        * Two-phased constructor.
        * @param aNotifyObserver Pointer to notify observer
        * @param aReaderID Reader ID
        * @param aLauncher Pointer to reader launcher
        */    
        static CSwimReaderIF* NewL( MScardNotifyObserver* aNotifyObserver,
                                    TReaderID aReaderID,
                                    CSwimReaderLauncher* aLauncher );
        
        /**
        * Destructor.
        */
        virtual ~CSwimReaderIF();
    

    public: // New functions
        
        /**
        * Cancel transmit
        * @return void
        */
        void CancelTransmit();
        
        /**
        * Close SwimReader IF
        * @return KErrNone
        */
        TInt Close();
        
        /**
        * Get the ATR bytes
        * @param aAtr ATR bytes
        * @return KScErrNotSupported
        */
        TInt GetATR( TScardATR& aAtr );
        
        /**
        * Get card status or presence bits according to aTag parameter
        * @param aStatus Clients request status
        * @param aTag Tag for request type
        * @param aValue Value of tag
        * @param aTimeout Timeout for GetCapabilities request
        * @return ETrue
        */        
        TBool GetCapabilities( TRequestStatus& aStatus,
                               const TInt32 aTag,
                               TPtr8& aValue,
                               const TInt32 aTimeout );
        
        /**
        * Notifies Scard Server about card event
        * @param aStatus TScardServiceStatus
        * @return void
        */
        void Notify( TScardServiceStatus aStatus );
        
        /**
        * Open connection to Etel server
        * @param  aStatus TRequestStatus
        * @return void
        */
        void OpenAsync( TRequestStatus& aStatus );

        /**
        * Send a CommandAPDU on a slot and return ResponseAPDU from slot
        * @param aStatus TRequestStatus
        * @param aCommandAPDU Command APDU
        * @param aResponceAPDU Response APDU
        * @param aTimeout Timeout
        * @return void
        */
        void TransmitToCard( TRequestStatus& aStatus,
                             const TPtrC8& aCommandAPDU,
                             TPtr8& aResponseAPDU,
                             const TInt32 aTimeout);

    private:
        
        /**
        * C++ default constructor.
        * @param aNotifyObserver Pointer to notify observer
        * @param aReaderID Reader ID
        * @param aLauncher Pointer to reader launcher
        */ 
        CSwimReaderIF( MScardNotifyObserver* aNotifyObserver, 
                       TReaderID aReaderID,
                       CSwimReaderLauncher* aLauncher );
        
        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        /**
        * Handle open APDU
        * @return void
        */
        void HandleOpen();

        /**
        * From CActive, RunL
        * @return void
        */
        void RunL();

        /**
        * Cancel the asyncronous request
        * @return void
        */
        void DoCancel();
    
    private: // Data
        // Enumerator for reader IF phases
        enum TReaderIFPhase
            {
            EOpen,
            EGetATR,
            ETransmitToCard
            };
        // error code
        TInt                            iErr;
        // Pointer to client status. Not owned.
        TRequestStatus*                 iClientStatus;
        // Pointer to SwimReader object. Owned.
        CSwimReader*                    iFunctionalLevel;
        // Historical bytes
        TBuf8<KMaxATRHistoricalBytes>   iHistoricals;
        // Reader ID
        TReaderID                       iID;
        // Reader IF phase
        TReaderIFPhase                  iIFPhase;
        // Pointer to ReaderLauncher object. Not owned. 
        CSwimReaderLauncher*            iLauncher;
        // Pointer to NotifyObserver object
        MScardNotifyObserver*           iNotifyObserver;
         // Pointer to System Agent Observer. Owned.
        CSwimSysAgentObserver*          iSysAgentObserver;
        
    private:    // Friend classes
        friend class CSwimReaderLauncher;
    
    };

#endif      // CSWIMREADERIF_H

// End of file
