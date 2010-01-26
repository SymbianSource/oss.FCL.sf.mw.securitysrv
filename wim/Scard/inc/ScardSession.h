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
* Description:  Session for the Scard server, to a single client-side session.
*
*/



#ifndef CSCARDSESSION_H
#define CSCARDSESSION_H

//  INCLUDES
#include "ScardDefs.h"
#include <f32file.h>    // f32file header needed for loading dlls

//  CONSTANTS  
const TInt KOpenChannelResponseLength = 3;
const TInt KApduHeaderLength          = 4;

const TInt KCLA    = 0;    //  class byte
const TInt KINS    = 1;    //  instruction code
const TInt KP1     = 2;    //  control parameter 1
const TInt KP2     = 3;    //  control parameter 2
const TInt KLcHigh = 4;    //  command length high byte
const TInt KLcLow  = 5;    //  command length low byte

const TInt8 KNoChannel     = 0;
const TInt8 KChannelByCard = 1;
const TInt8 KChannelByCID  = 2;


//  FORWARD DECLARATIONS
class CScardAccessControl;
class CScardServer;
class MScardReader;
class CScardNotifyRegistry;

// CLASS DECLARATION

/**
*  Session for the Scard server, to a single client-side session
*
*  @lib Scard.lib
*  @since Series60 2.1
*/
class CScardSession : public CSession2
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        * @param aServer Pointer to ScardServer object
        */
        static CScardSession* NewL( CScardServer* aServer );  

        /**
        * Destructor.
        */
        virtual ~CScardSession();
    
    public: // New functions

        /**
        * Clean up the reader event listeners.
        * @param aMessage Client message
        * @return void
        */
        void CloseSession( const RMessage2& aMessage );

        /**
        * Wrapper for the actual ServiceL() (this needed only because of
        * connector class)
        * @param aMessageHandle Handle to client message
        * @return void
        */
        virtual void ServiceL( const TMessageHandle& aMessageHandle );

        /**
        * Asynchronous operation completed
        * @param aMessageHandle Handle to client message
        * @param aErrorCode Error code
        * @return void
        */
        virtual void AsynchronousServiceComplete( 
            const TMessageHandle& aMessageHandle,
            const TInt aErrorCode );
    
        /**
        * This is here only for derivative class CScardConnector
        * It implements and needs this function
        * @param aServiceStatus Service status
        * @param aATR ATR bytes
        * @param aReaderID Reader ID from which event comes
        * @return void
        */
        virtual void CardEvent( const TScardServiceStatus aServiceStatus,
                                const TScardATR& aATR,
                                const TReaderID& aReaderID );

        /**
        * A reader has finally been contacted.
        * @param aReaderID Reader ID
        * @param aMessage Client essage
        * @param aErrorCode Error code
        * @return void
        */
        void ConnectionDone( const TReaderID& aReaderID, 
                             const RMessage2& aMessage,
                             const TInt aErrorCode );
    
        /**
        * Detach this session from the reader
        * @return void
        */
        void DisconnectFromReader();

    protected:
   
        /**
        * C++ default constructor.
        */
        CScardSession();
    
    protected:  // Functions from base classes
        
        /**
        * From CSharableSession. Service a request message
        * @param aMessage Message
        * @return void
        */
        void ServiceL( const RMessage2& aMessage );

    private:
        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL( CScardServer* aServer );

        /**
        * Connect to reader
        * @param aMessage Message
        * @return void
        */
        void ConnectToReaderL( const RMessage2& aMessage );

        /**
        * Get reader/card capabilities
        * @param aMessage Message
        * @return void
        */
        void GetCapabilities( const RMessage2& aMessage );

        /**
        * Send data to card
        * @param aMessage Message
        * @return void
        */
        void TransmitToCard( const RMessage2& aMessage );
    
        /**
        * Cancel everything
        * @return void
        */
        void CancelTransmitL();

        /**
        * Open/close channel, get channel status
        * @param aMessage Message
        * @return void
        */
        void ManageChannel( const RMessage2& aMessage );
    
        /**
        * Helper function for ManageChannel. Create and send a 
        * manage channel apdu according to parameters
        * @param aMessage Message
        * @param aCommand Command
        * @param aP2 Control parameter 2
        * @param aCommandLength Command APDU length
        * @param aResponseLength Response APDU length
        * @param aTimeOut Timeout
        * @param aChannel Channel used to transmit APDU
        * @return void
        */
        void DoCommandAndTransmit( const RMessage2& aMessage,
                                   const TUint8 aCommand, 
                                   const TUint8 aP2,
                                   const TInt8 aCommandLength, 
                                   const TUint8 aResponseLength,
                                   const TInt32 aTimeOut,
                                   const TUint8 aChannel );

        /**
        * Helper function for ManageChannel. Create and send a 
        * manage channel apdu according to parameters
        * @param aMessage Message
        * @param aCommand Command
        * @param aP2 Control parameter 2
        * @param aCommandLength Command APDU length
        * @param aResponseLength Response APDU length
        * @param aTimeOut Timeout
        * @param aChannel Channel used to transmit APDU
        * @return void
        */
        void DoCommandAndTransmitL( const RMessage2& aMessage,
                                   const TUint8 aCommand, 
                                   const TUint8 aP2,
                                   const TInt8 aCommandLength, 
                                   const TUint8 aResponseLength,
                                   const TInt32 aTimeOut,
                                   const TUint8 aChannel );

        /**
        * Write descriptors to client thread. 8 bit version.
        * @param aMessage Message
        * @param aResponseBuffer Response Buffer
        * @param aPtrIndex Pointer index
        * @return void
        */
        void WriteToClient( const RMessage2& aMessage,
                            const TDesC8& aResponseBuffer,
                            const TInt aPtrIndex );

        /**
        * Read descriptors from client thread. 8 bit version.
        * @param aMessage Message
        * @param aBuffer Buffer for data
        * @param aPointer Pointer to buffer data
        * @param aPtrIndex Pointer index
        * @return Symbian or Scard error code
        */
        TInt ReadFromClient( const RMessage2& aMessage,
                                   HBufC8*& aBuffer, 
                                   TPtr8*& aPointer,
                                   const TInt aPtrIndex );

        /**
        * Write descriptors to client thread. 16 bit version.
        * @param aMessage Message
        * @param aResponseBuffer
        * @param aPtrIndex
        * @return void
        */
        void WriteToClient( const RMessage2& aMessage,
                            const TDesC16& aResponseBuffer,
                            const TInt aPtrIndex );

        /**
        * Allocate space for the internal response buffer. The lenght of the
        * necessary space is deduced from the message.
        * @param aMessage Message
        * @param aPtrIndex Pointer index of message.
        * @return TInt
        */
        TInt AllocateResponse( const RMessage2& aMessage,
                                     const TInt aPtrIndex );

        /**
        * Allocate space for the internal response buffer. The lenght of the
        * necessary space is deduced from the message.
        * @param aDesLen Length of buffer to be allocated.
        * @return void
        */
        void AllocateResponseL( TInt aDesLen );

        /**
        * Delete all internal buffers
        * @return void
        */
        void DeleteBuffers();

        /**
        * Return pointer to wanted slot of client message 
        * @param aMessage Message
        * @param aPtrIndex Pointer index of message.
        * @return Pointer to message slot
        */
        TAny* MessagePointer( const RMessage2& aMessage, const TInt aPtrIndex );
        
        /**
        * Return length of descriptor in client message 
        * @param aMessage Message
        * @param aPtrIndex Pointer index of message.
        * @param aMaxLength ETrue: get maxlength of descriptor
        *                   EFalse: get length of descriptor
        * @return Length of descriptor (MaxLength or just Length)
        */
        TInt DesLength( const RMessage2& aMessage,
                        const TInt aPtrIndex, 
                        const TBool aMaxLength  );

    private:    // Data
        // Pointer to owning server. Not owned.
        CScardServer*         iServer; 
        //  Pointer to notify registry. Owned by server
        CScardNotifyRegistry* iNotifyRegistry;
        //  Reader handler associated with this session
        MScardReader*         iReader;
        //  Reader access controller for this session's reader
        CScardAccessControl*  iAccessCtrl; 
        //  This is, obviously, the session's ID number
        TInt                  iSessionID;
        //  Pointers to command and response parameters. Owned.
        TPtr8*                iResponse;
        TPtr8*                iCommand;

        //  Heap pointers containing the buffers. Owned.
        HBufC8*               iResponseBuffer;
        HBufC8*               iCommandBuffer;

        // Flag to tell if connected to readed
        TBool                 iConnectedToReader;
        
        // Client thread
        RThread iClient;

    };

#endif      // CSCARDSESSION_H

// End of File
