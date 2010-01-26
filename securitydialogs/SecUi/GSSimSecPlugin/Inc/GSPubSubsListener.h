/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Listener for Publish and subscribe data.
*
*/


#ifndef GSPUBSUBSLISTENER_H
#define GSPUBSUBSLISTENER_H

//  INCLUDES
#include <e32base.h>
#include <e32std.h>
#include <e32property.h>

// FORWARD DECLARATIONS
class MGSSettingPSObserver;

// CLASS DECLARATION

/**
*  RProperty poller.
*
*  @lib gs.lib
*  @since Series 60 3.0
*/
NONSHARABLE_CLASS( CGSPubSubsListener ) : public CActive
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        * @param aUid the Uid to use
        * @param aKey item's key
        * @param aObserver callback interface for notification
        * @return instance of CGSPubSubsListener
        */
        static CGSPubSubsListener* NewL( const TUid aUid, const TInt aKey, 
                                         MGSSettingPSObserver* aObserver );
        
        /**
        * Destructor.
        */
        virtual ~CGSPubSubsListener();
        
    public: // New functions.

        /**
        * Gets integer value from P & S.
        * @param aVal a value in return
        * @return error code
        */       
        TInt Get( TInt& aVal );

        /**
        * Gets 8 bit string value from P&S.
        * @param aVal a value in return
        * @return error code
        */
        TInt Get( TDes8& aVal );
        
        /**
        * Gets 16 bit descriptor value from P&S.
        * @param aVal a value in return
        * @return error code
        */
        TInt Get( TDes16& aVal );
        
    private: // From CActive.

        /** @see CActive::RunL */
        virtual void RunL();

        /** @see CActive::Cancel */
        virtual void DoCancel();

        /** @see CActive::RunError */
        virtual TInt RunError( TInt aError );

        /**
        * Starts the listening (RunL).
        */
        void StartListening();

    private:

        /**
        * C++ default constructor.
        * @param aUid the Uid to use
        * @param aKey item's key
        * @param aObserver callback interface for notification
        */
        CGSPubSubsListener( const TUid aUid,  TInt aKey, 
                            MGSSettingPSObserver* aObserver );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();
    
    private:    // Data
    
        // UID of the monitored item.
        TUid        iUid;
        
        // ID of the monitored item.
        TInt        iId;
        
        // Property to subscribe to.
        RProperty   iProperty;
        
        // The notification interface.
        MGSSettingPSObserver* iCallback;
    };

#endif // GSPUBSUBSLISTENER_H

// End of File
