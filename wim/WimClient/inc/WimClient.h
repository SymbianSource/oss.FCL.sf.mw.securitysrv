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


#ifndef WIMCLIENT_H
#define WIMCLIENT_H

//  INCLUDES
#include "WimClsv.h"
#include <f32file.h>
#include <e32base.h> 

//FORWARD DECLARATIONS
class TWIMSrvStartSignal;

//The number of message slots represents the number of operations that
//can be outstanding at once. If you wish to have more asynchronous operations 
//outstanding than this, use more message slots. 
//You should always provide an extra slot for the cancel operation.
const TInt KMessageSlotsNum = 5;


// CLASS DECLARATION

/**
*  RWimClient, this creates new connection to server and sends 
*  syncronous & asyncronous messages to server. 
*  
*  @lib WimClient
*  @since Series 60 2.1
*/
class RWimClient: public RSessionBase
    {
    public:

        /**
        * Constructor 
        */
        RWimClient();

        /**
        * Destructor 
        */
        virtual ~RWimClient();

        /**
        * Connect to WIMI server
        * @return TInt
        */
        TInt Connect();

        /** 
        * Starts Server
        * @return TInt -error code
        */
        TInt StartWim();


        /** 
        * Sends initialization command to Server.
        * @param aStatus -Client status
        * @return void
        */
        void Initialize( TRequestStatus& aStatus );
        
        /** 
        * Cancel initialization command to Server.
        * @return void
        */
        void CancelInitialize();

        /**
        * Return version
        * @return TVersion
        */
        TVersion Version() const;

        /**
        * Sends data synchronoysly to server
        * @param aFn the WIMRequest operation 
        * @param aIpcArgs IPC parameters
        * @return TInt
        */
        TInt SendReceiveData( TWimServRqst aFn, TIpcArgs& aIpcArcs );

        /**
        * Sends data asynchronoysly to server
        * @param aFn the WIMRequest operation 
        * @param aIpcArgs IPC parameters
        * @param aStatus -Caller's status
        * @return void
        */
        void SendReceiveData( TWimServRqst aFn, 
                              TIpcArgs& aIpcArgs,
                              TRequestStatus& aStatus );

        /**
        * Free the address list  
        * @param addrLst -Address of a list to be freed
        * @return void
        */
        void FreeAddrLst( const TUint32 addrLst );

        /**
        * Free the address list 
        * @param addrLst    -Reference to WIMI
        * @param aSize      -Size to be freed
        * return void
        */
        void FreeWIMAddrLst( const TWimAddressList addrLst, const TUint aSize );

        /**
        * Free the address list  
        * @param aAddr  -Referece to WIMI
        * return void
        */
        void FreeWIMAddr( const TWimAddress aAddr );


    private:


        /** 
        * Return the name of the Wim server file
        * @param aServer -descriptor for server filename
        * @return TInt -error code
        */
        TInt FindServerFileName( TFileName& aServer );
    
    
    private:
        
        /** 
        * Copy constructor
        * @param aSource -Reference to class object.
        */
        RWimClient( const RWimClient& aSource );

        /** 
        * Assigment operator
        * @param aParam -Reference to class object.
        */
        RWimClient& operator = ( const RWimClient& aParam );




    };

#endif
