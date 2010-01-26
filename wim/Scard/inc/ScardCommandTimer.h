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
* Description:  Smart card command timer
*
*/



#ifndef CSCARDCOMMANDTIMER_H
#define CSCARDCOMMANDTIMER_H

// FORWARD DECLARATIONS
class CScardAccessControl;

// CLASS DECLARATION

/**
*  Timer class.
*  Timer class, which is used when smart card is accessed after connection 
*  is established.
*
*  @lib Scard.lib
*  @since Series60 2.1
*/
class CScardCommandTimer : public CActive  
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        * @param aTimeOut Timeout
        * @param aController Pointer to access controller
        */
        static CScardCommandTimer* NewL( TInt32 aTimeOut, 
                                         CScardAccessControl* aController );
        
        /**
        * Destructor.
        */
        virtual ~CScardCommandTimer();

    public: // New functions

        /**
        * Starts the timer.
        * @return void
        */
        void StartTiming();
        
    public: // Functions from base classes
    
        /**
        * From CActive DoCancel.
        */
        void DoCancel();

        /**
        * From CActive RunL.
        */
        void RunL();

    private:

        /**
        * C++ default constructor.
        */
        CScardCommandTimer();

        /**
        * By default Symbian 2nd phase constructor is private.
        * @param aTimeOut Timeout
        * @param aController Pointer to access controller
        */
        void ConstructL( TInt32 aTimeOut,
                         CScardAccessControl* aController );

    private:
        // Pointer to RTimer object. Owned.
        RTimer* iTimer;
        // Pointer to Controller. Not owned.
        CScardAccessControl* iController;
        // Timeout
        TInt32 iTimeOut;
    };

#endif      // CSCARDCOMMANDTIMER_H

// End of File
