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
* Description:  Smart card message stack.
*
*/



#ifndef CSCARDMESSAGESTACK_H
#define CSCARDMESSAGESTACK_H

//  INCLUDES
#include "ScardServerBase.h"
#include "ScardClsv.h"
#include "ScardCommandTimer.h"
#include "ScardDefs.h"

// FORWARD DECLARATIONS
class CScardCommandTimer;

// CLASS DECLARATION

/**
*  Message stack.
*  This class establishes a connection with a SC reader handler for a session. 
*  Creates (if necessary) MScardReader objects, and determines when one of 
*  them satisfies the parameters.
*
*  @lib Scard.lib
*  @since Series60 2.1
*/
class CScardMessageStack : public CBase
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CScardMessageStack* NewL();

        /**
        * Destructor.
        */
        virtual ~CScardMessageStack();
    
    public: // New functions 
        
        /**
        * Cancel all messages waiting for service. Delete all others 
        * except the first one.
        * @param aReason Reason code for cancellation
        * @return void
        */
        void CancelAll( const TInt aReason );
        
        /**
        * Cancel all operations for this session ID.
        * @param aSessionID Session ID
        * @param aReason Reason code for cancellation
        * @return void
        */
        void CancelAll( const TInt aSessionID,
                        const TInt aReason );

        /**
        * Effectively the same as CancelAll(const TInt aReason), 
        * except that does not cancel OpenReader commands.
        * @return void
        */
        void CancelCardOperations();

        /**
        * Cancels the messages using the given timer.
        * @param aTimer Timer object
        * @return void
        */
        void CancelByTimeOut( const CScardCommandTimer* aTimer );

        /**
        * Puts the message to the top of the stack.
        * @param aMessage Message
        * @return Error code
        */
        TInt PushToTop( const TMessageHandle& aMessage );

        /**
        * Puts the message to the bottom of the stack.
        * @param aMessage Message
        * @return Error code
        */
        TInt PushToBottom( const TMessageHandle& aMessage );

        /**
        * Returns the topmost message from the stack.
        * @return Handle to topmost message
        */
        const TMessageHandle FromTop();
        
        /**
        * This function is called when a channel is freed. It gives priority
        * for applications who are waiting to reserve either this channel 
        * or any channel.
        * @param aChannel Channel
        * @return Handle to message
        */        
        const TMessageHandle NextReservation( const TInt8 aChannel );

        /**
        * Returns the message from given position in the stack.
        * @param aPosition Position from which we want the message
        * @return Handle to message
        */ 
        const TMessageHandle FromPositionL( const TInt aPosition );

        /**
        * Push message to given position in the stack.
        * @param aPosition Position to which we want to push the msg
        * @param aHandle Message handle
        * @return Handle to message
        */
        void PushToPositionL( const TInt aPosition,
                              TMessageHandle& aHandle );

        /**
        * Returns the number of messages in the stack.
        * @return Count of messagages in the stack
        */
        inline const TInt Count();

    private:
        
        /**
        * C++ default constructor.
        */
        CScardMessageStack();
        
        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        /**
        * Cancel one message
        * @param aIndex Index of message
        * @param aReason Reason code for cancellation
        * @param aCancelTimer Do we want to cancel timer
        * @return void
        */
        inline void CancelOne( const TInt aIndex,
                               const TInt aReason, 
                               const TBool aCancelTimer = ETrue );

    private:    // Data
        // Array of messages. Owned.
        CArrayFixFlat<TMessageHandle>* iMessages;
    };

#include "ScardMessageStack.inl"

#endif      // CSCARDMESSAGESTACK_H

// End of File
