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
* Description:  A class implementing two stacks. One is a fifo-stack of
*               reader events, the other a random-access stack of service
*               providers that are not used with a given reader.
*
*/



#ifndef CSCARDEVENTSTACK_H
#define CSCARDEVENTSTACK_H

//  INCLUDES
#include "ScardServerBase.h"
#include "ScardDefs.h"

//  FORWARD DECLARATIONS
class CScardConnector;

// CLASS DECLARATION

/**
*  A class implementing two stacks. 
*  One is a fifo-stack of reader events, the other a random-access stack of
*  service providers that are not used with a given reader.
*
*  @lib Scard.lib
*  @since Series60 2.1
*/
class CScardEventStack : public CBase  
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        * @param aConnector Pointer to connector object
        */
        static CScardEventStack* NewL( CScardConnector* aConnector );

        /**
        * Destructor.
        */
        virtual ~CScardEventStack();
   
    public: // New functions
       
        /**
        * Queue events
        * @param aReaderID ReaderID
        * @param aEventType Event type
        * @return void
        */
        void QueueEvent( const TReaderID aReaderID,
                         const TScardServiceStatus aEventType );

    private:

        /**
        * C++ default constructor.
        */
        CScardEventStack();

        /**
        * By default Symbian 2nd phase constructor is private.
        * @param aConnector Pointer to connector object
        */
        void ConstructL( CScardConnector* aConnector );

    private:    // Data
        // Array of event. Owned.
        CArrayFixFlat<TQueueEvent>*  iEvents;
        // Pointer to connector object. Not owned.
        CScardConnector* iConnector;
    };

#endif      // CSCARDEVENTSTACK_H

// End of File
