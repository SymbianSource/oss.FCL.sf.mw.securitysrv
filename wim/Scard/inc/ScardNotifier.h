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
* Description:  Notifier for SmartCard.
*
*/



#ifndef CSCARDNOTIFIER_H
#define CSCARDNOTIFIER_H


//  INCLUDES
#include "ScardNotifyRegistry.h"

// CLASS DECLARATION

/**
*  Used to notify smart card server about card events.
*
*  @lib Scard.lib
*  @since Series60 2.1
*/
class CScardNotifier : public CActive
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        * @param aRegistry Pointer to notify registry
        * @param aClient Client thread
        * @param aClientStatus Client request status
        * @param aReaderID Reader ID
        */
        static CScardNotifier* NewL( CScardNotifyRegistry* aRegistry, 
                                     RMessage2& aMessage,
                                     TRequestStatus* aClientStatus, 
                                     const TReaderID& aReaderID );
        
        /**
        * Destructor.
        */
        virtual ~CScardNotifier();
    
    public: // New functions
    
        /**
        * Complete our own request, or if that's done already, 
        * store the event until it can be serviced.
        * @param aEvent Scard event
        * @param aReaderID Reader ID
        * @return void
        */
        void NotifyCardEvent( TScardServiceStatus aEvent, 
                              TReaderID aReaderID );
                              
        TReaderID ReaderId();

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
        * @param aRegistry Pointer to notify registry
        * @param aClient Client thread
        * @param aClientStatus Client request status
        * @param aReaderID Reader ID
        */
        CScardNotifier( CScardNotifyRegistry* aRegistry, 
                        RMessage2& aMessage,
                        TRequestStatus* aClientStatus,
                        const TReaderID& aReaderID );
        
        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    public: // Data
                
    
    private:    // Data
        struct TCardEvent
            {
            TScardServiceStatus iEvent;
            TReaderID           iReaderID;
            };

        // Pointer to notify registry. Not owned.
        CScardNotifyRegistry*      iRegistry;
        // Event stack. Owned.
        CArrayFixFlat<TCardEvent>* iEventStack;
        
        // Client message to be completed when event occurs
        RMessage2       iMessage;

        // Clients request status. Not owned. 
        TRequestStatus* iClientStatus; 
        
        // Reader ID
        TReaderID       iReaderID;
        
        // Flag to tell if message is already completed
        TBool           iMessageCompleted;
 
    };

#endif      // CSCARDNOTIFIER_H

// End of File
