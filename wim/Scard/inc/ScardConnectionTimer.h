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
* Description:  Timer for connections to smartcard.
*
*/



#ifndef CSCARDCONNECTIONTIMER_H
#define CSCARDCONNECTIONTIMER_H

// INCLUDES
#include "ScardBase.h"

// FORWARD DECLARATIONS
class CScardConnector;

/**
*  Timer class, which is used while establishing connection to smart card.
*
*  @lib Scard.lib
*  @since Series60 2.1
*/
class CScardConnectionTimer : public CActive
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        * @param aConnector Pointer to connector object
        * @param aTimeOut Timeout
        */
        static CScardConnectionTimer* NewL( CScardConnector* aConnector,
                                            TInt32 aTimeOut );
        
        /**
        * Destructor.
        */
        virtual ~CScardConnectionTimer();
    
    public: // Functions from base classes        
        
        /**
        * From CActive RunL.
        */
        void RunL();
        
        /**
        * From CActive DoCancel.
        */
        void DoCancel();

    private:
        
        /**
        * C++ default constructor.
        * @param aConnector Pointer to connector object
        * @param aTimeOut Timeout
        */
        CScardConnectionTimer( CScardConnector* aConnector,
                               TInt32 aTimeOut );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private:    // Data
        // Timeout
        TInt32  iTimeOut;
        
        // Pointer to timer. Owned.
        RTimer* iTimer;

        // Pointer to connector. Not owned.
        CScardConnector* iConnector;
    };

#endif      // CSCARDCONNECTIONTIMER_H

// End of File
