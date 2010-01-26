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
* Description:  Observes the state of Bluetooth Sap. When the state is
*               changed into connected, connection to smart card is closed and
*               when the state is changed into not connected, initialize
*               connection to smart card back.
*
*/


#ifndef CWIMBTSAPOBSERVER_H
#define CWIMBTSAPOBSERVER_H


//  INCLUDES
#include <e32base.h>
#include <e32property.h> // RProperty

// FORWARD DECLARATIONS
class CWimServer;

// CLASS DECLARATION

/**
*  Publish and subscribe Observer class
*  This class listens P/S events
*
*  @lib SwimReader.lib
*  @since Series60 1.2
*/
class CWimBTSapObserver : public CActive
    {
    public: // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */ 
        static CWimBTSapObserver* NewL();

        /**
        * Destructor.
        */
        virtual ~CWimBTSapObserver();

    public: // New functions
        
        /**
        * Starts listener
        * @return void 
        */
        void Start();

    private:

        /**
        * C++ default constructor.
        */ 
        CWimBTSapObserver();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        /**
        * This function starts listening card state from System agent
        * @return void
        */
        void StartListening();
        /**
        * From CActive. Handle asyncronous event response
        * @return void
        */
        void RunL();

        /**
        * From CActive. Handle Cancel call
        * @return void
        */
        void DoCancel();

    private:    // Data
        // Bluetooth Sap status property
        RProperty       iProperty;
        // Pointer to WimServer.
        CWimServer*     iWimSvr;
    };

#endif      // CWIMBTSAPOBSERVER_H
            
// End of File
