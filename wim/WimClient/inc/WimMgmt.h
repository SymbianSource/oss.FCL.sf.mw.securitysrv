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
* Description:  Startup place for client & server, message handling.
*
*/


#ifndef WIMMGMT_H
#define WIMMGMT_H

//  INCLUDES
#include "WimClient.h"
#include "WimCertTClassWrappers.h"
#include <secdlg.h>

/**
*  RWimMgmt 
*  This class inherits RWimClient and communicates with it.
*  @lib WimClient
*  @since Series 60 2.1
*/
class RWimMgmt: public RWimClient
    {
    public:

        /**
        * Destructor 
        */
        virtual ~RWimMgmt();

        /**
        * Returns  TUint - the number of WIMs
        * @return TUint
        */
        TUint WIMCount();
        
        /**
        * Gets the array of WIM structures.
        * @param aWimAddrLst - Pointer to a list of allocated WIM structures
        * @param aCount  - is the number of WIMs
        * @return TInt -Error code
        */
        TInt WIMRefs( TWimAddressList aWimAddrLst, TUint8 aCount );


        /**
        * Tests whether WIM is open or not.
        * @param aWimAddr - Address to the WIM.
        * @return Boolean
        */
        TBool IsOpen( const TWimAddress aWimAddr );

        /**
        * Closes the WIM and returns the status of the WIMI_CloseWIM()
        * @param aWimAddr - Address to the WIM 
        * @return TInt the return status of Close   
        */
        TInt CloseWIM( const TWimAddress aWimAddr );

        /**
        * Gets the WIM information in to aTWimSecModuleStruct
        * @param aWimAddr -Address to the WIM structure (IN)
        * @param aTWimSecModuleStruct -Reference to the Structure to contain 
        *                                WIM  information (OUT)
        * @return TInt -Error code
        */
        TInt WIMInfo( const TWimAddress aWimAddr, 
                      TWimSecModuleStruct& aTWimSecModuleStruct );
  
        /**
        * Returns the time which is set as WIM closing time
        * @return TUint -closing time
        */
        TInt CloseAfter();

        /**
        * Returns TUint - the timeout in minutes, which tells how long WIM 
        * Security Module will be open
        * @return TUint -timeout
        */
        TInt TimeRemaining();
  
        /*
        * Sets closing time for WIM.
        * @param aCloseAfter
        * @return void
        */
        void SetCloseAfter( const TUint aCloseAfter );
        
        /**
        * Returns the count of PINs
        * @param aWimAddr- Address to the WIM structure (IN)
        * @return TUint -the count of PINs
        */  
        TUint PINCount( const TWimAddress aWimAddr );

        /**
        * Gets the array of PIN structures.
        * @param aWimAddr -Address to the WIM structure
        * @param aPinLstAddr -Reference to PIN list
        * @param aPinAddrLst    -Address to PIN address list
        * @param aCount -Count of PINs
        * @return TInt -Error code
        */
        TInt PINRefs( const TWimAddress aWimAddr, 
                      TUint32& aPinLstAddr, 
                      TPinAddressList aPinAddrLst, 
                      TUint8 aCount );
    
        /**
        * Get all of the pins information on a given SecMod asynchronously.
        * @ param aPinInfoLst array to store the PINs information 
        * return void 
        */
        void PINsInfo( const TWimAddress aWimAddr, 
                        CArrayFixFlat<TWimPinStruct>& aPinInfoLst,
                        TUint8 aCount,
                        TRequestStatus& aStatus );
        /**
        * Get all of the pins information on a given SecMod synchronously.
        * @ param aPinInfoLst array to store the PINs information 
        * return void 
        */                
        TInt PINsInfo( const TWimAddress aWimAddr, 
               CArrayFixFlat<TWimPinStruct>& aPinInfoLst,
               TUint8 aCount );                
        
        /**
        * Gets the PIN information in to aPinStruct
        * @param aPinAddr -Address to the PIN structure (IN)
        * @param aPinStruct -Reference to the Structure to containing PIN 
        *                    information (OUT)
        * @return TInt -Error code
        */  
        void PINInfo( const TPinAddress aPinAddr,
                      TWimPinStruct& aPinStruct,
                      TRequestStatus& aStatus );

        /**
        * Used to check is PIN blocked or not.
        * @param aPinAddr -Address to the PIN struct
        * @return TInt -Error code
        */  
        TInt IsBlocked( const TPinAddress aPinAddr );
        
        /**
        * Used to check is Disabled PIN blocked or not.
        * @param aPinAddr -Address to the PIN struct
        * @return TInt -Error code
        */ 
        TInt IsDisabledPINBlocked( const TPinAddress aPinAddr );

        /** 
        * Handles Enable and Disable PIN requests. Server
        * knows wanted operation by flag which is set in TPINStateRequest.
        * @param aPinAddr   -Address to the PIN struct.
        * @param aPinStateRequest   -Enable/Disable flag
        * @param aPinParams     -General information about PIN.
        * @param aStatus    -Caller's status
        * @return void   
        */
        void EnableDisablePinQueryL(
                            const TPinAddress aPinAddr,
                            const TPINStateRequest& aPinStateRequest,
                            const TPINParams& aPinParams,
                            TRequestStatus& aStatus );
        
        /** 
        * Cancel Enable and Disable PIN requests. Server
        * knows wanted operation by flag which is set in TPINStateRequest.
        * @param aPinAddr   -Address to the PIN struct.
        * @return void   
        */
        void CancelEnableDisablePin( const TPinAddress aPinAddr );
        
        /** 
        * PinOperationL handles several PIN operations: it can be
        * verify PIN request, change PIN request or unblock PIN request. The 
        * type of request is handled via aOpCode.
        * @param aPinAddr   -Address to the PIN struct
        * @param aPinParams -General information about PIN
        * @param aOpCode    -Code that informs the type of operation
        * @param aStatus    -Caller's status
        * @return void
        */
        void PinOperationL( const TPinAddress aPinAddr,
                            const TPINParams aPinParams,
                            const TWimServRqst aOpCode,
                            TRequestStatus& aStatus );
        
        /** 
        * Cancel PinOperationL handles several PIN operations: it can be
        * verify PIN request, change PIN request or unblock PIN request. The 
        * type of request is handled via aOpCode.
        * @param aPinAddr   -Address to the PIN struct
        * @param aOpCode    -Code that informs the type of operation
        * @return void
        */
        void CancelPinOperation( const TPinAddress aPinAddr, const TWimServRqst aOpCode );                     

        /**
        * A static method to get the client session.
        * @Return RWimMgmt*
        */  
        static RWimMgmt* ClientSessionL();

        /** 
        * Deallocates memory from pckgBuf member variables
        * @return void
        */
        void DeallocMemoryFromEnableDisablePinQuery();

    private:

        /**
        * Constructor 
        */
        RWimMgmt();

        /** 
        * Copy constructor
        * @param aSource -Reference to class object.
        */
        RWimMgmt( const RWimMgmt& aSource );

        /** 
        * Assigment operator
        * @param aParam -Reference to class object.
        */
        RWimMgmt& operator = ( const RWimMgmt& aParam );

    private:

        //PckgBuffer to hold PIN state information
        CWimCertPckgBuf<TPINStateRequest>*  iPinStateRequestBuf;

        //PckgBuffer to hold PIN parameters information
        CWimCertPckgBuf<TPINParams>*        iPinParamsBuf;

        //Indicates whether iPinStateRequestBuf is allocated or not
        TBool                               iPinStateRequestBufAllocated;

        //Indicates whether iPinParamsBuf is allocated or not
        TBool                               iPinParamsBufAllocated;
        
        //The pointer descriptor to pin info list
        TPtr8*                              iPinInfoLstPtr; 
        
        TPckg<TWimPinStruct>*               iPinModule;


    };


#endif
