/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   The header file of DTClient
*
*/



#ifndef __DTCLIENT_H__
#define __DTCLIENT_H__

#include <e32base.h>
#include "DevTokenCliServ.h"

class MCTToken;
class RDevTokenClientSession;

/**
 * Base class for implementations of ctframework interfaces.  Derived classes
 * inherit from this and also implement the appropriate interface.
 * It provides asynchronous functionality for sending requests to the filetokens
 * server and processing the responses.
 * Some methods are const so they can be called by the cert apps client, whose
 * interface contains several const methods.  These methods need to create
 * internal buffers, and this leads to iRequestDataBuf and iRequestPtr being
 * mutable.
 *
 *  @lib DevTokenClient.dll
 *  @since S60 v3.2
 */
class CDTClient : public CActive
    {
    public:
        virtual ~CDTClient();

    protected:
        
        CDTClient(TInt aUID, MCTToken& aToken, RDevTokenClientSession& aClient);
        
        TInt AllocRequestBuffer(TInt aReqdSize) const;
        
        void FreeRequestBuffer() const;
        
        void SendSyncRequestAndHandleOverflowL(TDevTokenMessages aMessage,
                                               TInt aInitialBufSize,
                                               const TIpcArgs& aArgs) const;

    protected:
        // From CActive
        virtual void DoCancel();  
        
        virtual TInt RunError(TInt aError);

    protected:
        /**
        * Maintains state for async requests to device tokens server, and completes
        * caller when required.
        */
        class TAsyncRequest
            {
            public:
               
                void operator()(TDevTokenMessages aRequest, TRequestStatus* aStatus);
               
                void Complete(TInt aCompletionResult);      
               
                void Cancel();
               
                inline TDevTokenMessages OutstandingRequest() { return iRequest; }
            
            public:
              
                ~TAsyncRequest();
            
            private:
              
                TDevTokenMessages iRequest;
              
                TRequestStatus* iClientStatus;
            };
        
        //The outstanding server request
        TAsyncRequest iCurrentRequest;    

    private:
        //Buffer for messages passed to the server
        mutable HBufC8* iRequestDataBuf;    
    
    protected:
        // The token we belong to
        MCTToken& iToken;        
       
        // UID of the cryptoken interface we implement
        TInt iInterfaceUID;        
       
        // Client session object for sending messages to the server
        RDevTokenClientSession& iClientSession; 
       
        // Buffer pointer for derived classes to use
        mutable TPtr8 iRequestPtr;
    };

#endif  //  __DTCLIENT_H__

//EOF

