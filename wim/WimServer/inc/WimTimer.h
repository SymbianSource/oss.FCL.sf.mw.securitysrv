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
* Description:  A timer class that calls the function TimerExpired for the 
*               registered classes of type MWimTimerListener
*
*/



#ifndef CWIMTIMER_H
#define CWIMTIMER_H

//  INCLUDES
#include <e32base.h>

// Interface for the listener
class MWimTimerListener
    {
    public:
        virtual void TimerExpired() = 0;
        virtual ~MWimTimerListener(){}
    };  


// CLASS DECLARATION

/**
*  Timer class.
*  
*  @since Series60 2.6
*/
class CWimTimer : public CActive, public MBeating
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CWimTimer* NewL( MWimTimerListener* aTimerListener );
        
        /**
        * Destructor.
        */
        virtual ~CWimTimer();

    public: // New functions
        
        /**
        * Set timeout value for closing WIM connection
        * @param aMessage Client message
        * @return void
        */
        void SetCloseAfter( const RMessage2& aMessage );

        /**
        * Resets timer
        * @return void
        */
        void ResetTimer();

        /**
        * Time remaining. Write value to client.
        * @return void
        */
        void TimeRemainingL( const RMessage2& aMessage ) const;
        
        /**
        * Time reamining, Used in Server side
        */
        TInt TimeRemaining() const;
        
        /**
        * Get delay set by SetCloseAfter. Write value to client.
        * @return void
        */
        void GetCloseAfterL( const RMessage2& aMessage ) const;

    private:

        /**
        * C++ default constructor.
        */
        CWimTimer();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL( MWimTimerListener* aTimerListener );

        /**
        * From CActive Handles an active object’s request completion event
        * @return void
        */
        void RunL();

        /**
        * From CActive Cancellation of an outstanding request
        * @return void
        */
        void DoCancel();

        /**
        * Set timeout value for closing WIM connection
        * @param aCloseAfter Timeout value in seconds
        * @return void
        */
        void DoSetCloseAfter( TInt aCloseAfter );

        /**
        * From MBeating. Timer beat has occurred, increase beat counter.
        * @return void
        */
        void Beat();

        /**
        * From MBeating. Synchronize.
        * @return void
        */
        void Synchronize();

        /**
        * Start heart beating
        * @return void
        */
        void StartHeartBeat();

    private:    // Data
        // Delay in microseconds
        TTimeIntervalMicroSeconds32 iDelay;
        // Interface for calling listeners. Not owned.
        MWimTimerListener*          iTimerListener;
        // Asynchronous timer
        RTimer                      iTimer;
        // Pointer to Heartbeat instance. Owned.
        CHeartbeat*                 iHeartbeat;
        // Heartbeat count
        TUint                       iHeartBeatCount;
         
    };

#endif      // CWIMTIMER_H

// End of File
