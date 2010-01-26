/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  API for managing WIM Security Module
*
*/



#ifndef WIMSECMODULE_H
#define WIMSECMODULE_H

//  INCLUDES
#include "WimClsv.h"
#include <e32base.h>

// FORWARD DECLARATIONS
class CWimPin;
class RWimMgmt;


// CLASS DECLARATION

/**
*  API for managing WIM Security Module.
*  Provides methods to manage one WIM Security Module:
*  check statuses, change settings, use functionality 
*  provide it and list PINs.
*
*  @lib WimClient
*  @since Series 60 2.1
*/
class CWimSecModule: public CActive
    {
    public:  // Constructors and destructor
                    
        /**
        * Two-phased constructor.
        * @param aWimAddr
        */
        IMPORT_C static CWimSecModule* NewL( TWimAddress aWimAddr ); 
        
        /**
        * Destructor.
        */
        IMPORT_C virtual ~CWimSecModule();

    public: // New functions
            
        /**
        * Checks is WIM already open.
        * @return ETrue if WIM's is open (=PIN-G verified)
        *         else EFalse  
        */
        IMPORT_C TBool IsOpen(); 

        /**
        * Returns the time which is set as WIM closeout time
        * @return Timeout in minutes
        */
        IMPORT_C TInt CloseAfter();

        /**
        * Returns the timeout which tells how 
        * long WIM Security Module will be open 
        * after it is opened.
        * @return Timeout in minutes
        */
        IMPORT_C TInt TimeRemaining();

        
        /**
        * Returns WIM Security Module's keys' PIN-NR objects in array.
        * @param aPinNRs -Array of pointers to CWimPin objects which
        *                are initialized as PIN-NR. (IN/OUT)
        * @return TInt  -Error code
        */
        IMPORT_C TInt PinNrEntriesL( const CArrayPtrFlat<CWimPin>*& aPinNRs, TRequestStatus& aStatus  );
     
        /**
        * Returns WIM Security Module's keys' PIN-NR objects in array.
        * @param aPinNRs -Array of pointers to CWimPin objects which
        *                are initialized as PIN-NR. (IN/OUT)
        * @return TInt  -Error code
        */
        IMPORT_C TInt PinNrEntriesL( const CArrayPtrFlat<CWimPin>*& aPinNRs );
        
        /**
        * Returns WIM's version. This will return KErrNone according to PKC15. 
        * @return Pointer descriptor containing version.
        */
        IMPORT_C const TDesC& Version();

        /**
        * Returns WIM's label.
        * @return Pointer descriptor containing label.
        */
        IMPORT_C TPtrC Label();

        /**
        * Returns WIM's manufacturer.
        * @return Pointer descriptor containing manufacturer.
        */
        IMPORT_C const TDesC& Manufacturer();

        /**
        * Returns WIM's serial number which identifies it.
        * @return Pointer descriptor containing serial number
        */
        IMPORT_C const TDesC& SerialNumber();

        /**
        * Closes the parts of WIM which is opened with PIN-G.
        * @return TInt Error code
        */
        IMPORT_C TInt Close();

        /**
        * Sets the timeout which defines the time after WIM 
        * is automatically closed.
        * @param aTimeout, timeout in minutes.
        * @return void
        */
        IMPORT_C void SetCloseAfter( const TUint aTimeout );


        /**
        * Notifies the client when the token has been removed.
        * @param aStatus -TRequestStatus
        * @return void
        */
        IMPORT_C void NotifyOnRemoval( TRequestStatus& aStatus );

        /**
        * Cancels the notifyOnRemoval request.
        * @return void
        */
        IMPORT_C void CancelNotifyOnRemoval();

        /**
        * Returns the number of the slot, where current token is connected
        * @return TUint8
        */
        IMPORT_C TUint8 TokenNumber();

        /**
        * Sets pointer RWimMgmt object to modifier iClientSession
        * @param aClientSession Client session
        * @return void
        */
        void SetClientSession( RWimMgmt* aClientSession );


    private:

        /**
        * C++ constructor.
        * @param aWimAddr
        */
        CWimSecModule( TWimAddress aWimAddr );

        /**
        * By default EPOC constructor is private.
        */
        void ConstructL();


        /**
        * Gets label,manufacturer,serialnumber and the number of 
        * the slot where card is.
        * @return TInt
        */
        TInt GetWIMInfoL(); 

        /** 
        * Handles cleanup for an object which is not derived from CBase
        * @param aObject -object to be cleaned up
        */
        static void Cleanup( TAny* aObject );

        /** 
        * Converts serialnumber to 8-bit form. Conversion reads
        * memory area from orginal serialnumber and captures its significant
        * bits to a new 8-bit -modifier. 
        * @param aSerialNumber -number to be converted
        * @return void
        */
        void ConvertSerialNumberL( HBufC*& aSerialNumber );
        
    private:  
        
        /**
        * Gets PIN references and creates new CWimPin objects
        * @return TInt  -Error code
        */
        TInt GetPINModulesL( TRequestStatus& aStatus );
        
        /**
        * Gets PIN references and creates new CWimPin objects
        * @return TInt  -Error code
        */
        TInt GetPINModulesL();
        
    private: //from CActive
        
        /**
        * Different phases are handled here.
        * @return void
        */
        void RunL();
        
        /**
        * Cancellation function
        * Deallocates member variables and completes client status with
        * KErrCancel error code.
        * @return void
        */
        void DoCancel();

        /**
        * The active scheduler calls this function if this active 
        * object's RunL() function leaves. 
        * Handles neccessary cleanup and completes request with
        * received error code.
        * @param aError -Error code which caused this event
        * @return TInt  -Error code to active scheduler, is always KErrNone.
        */ 
        TInt RunError( TInt aError );    
  
    private: 

        // Reference to WIM. 
        TWimAddress             iReference;  
        // Array of pointers to PIN-NRs.This class owns this and
        // this is valid as long as CWimSecModule is valid.
        CArrayPtrFlat<CWimPin>* iPinNRs; 
        //Contains tokens label.Owned
        HBufC*                  iLabel;
        //Contains tokens manufacturer.Owned
        HBufC*                  iManufacturer;
        //Contains tokens SerialNumber.Owned
        HBufC*                  iSerialNumber;
        //Contains tokens version. Owned
        HBufC*                  iVersion;
        //The "slot" where WIM-card is inserted
        TUint8                  iReader;        
        //pointer to RWIMMgmt. Owned by CWimSecModuleMgr.
        RWimMgmt*               iClientSession;
        //Boolean value 
        TBool                   iWimInfoConstructed;
        //TUint32, reference to pinlist.
        TUint32                 iPinLstAddr;
        //TUint32, reference to ping.
        TPinAddress             iRefPing;
        
        TWimPinStruct*          iPinStructLst; //Owned
        
        TRequestStatus*         iClientStatus;
        
        TUint                   iPinCount;
        
        CArrayFixFlat<TWimPinStruct>* iPinInfoLst; //Owned
        //ETrue if iPinNRs is initialized
        TBool                   iPinNRsInit;

    };

#endif      // WIMSECMODULE_H   
            
// End of File
