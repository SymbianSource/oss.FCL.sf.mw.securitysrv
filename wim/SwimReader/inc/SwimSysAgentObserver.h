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
* Description:  Implement SIM state observer for card removed event
*
*/



#ifndef CSWIMSYSAGENTOBSERVER_H
#define CSWIMSYSAGENTOBSERVER_H


//  INCLUDES
#include <e32base.h>
#include <e32property.h> // RProperty

//  FORWARD DECLARATIONS
class CSwimReaderIF;

// CLASS DECLARATION

/**
*  Publish and subscribe Observer class
*  This class listens P/S events
*
*  @lib SwimReader.lib
*  @since Series60 1.2
*/
class CSwimSysAgentObserver : public CActive
    {
    public: // Constructors and destructor
        
        /**
        * Two-phased constructor.
        * @param aReaderIF Pointer to ReaderIF owning this observer
        */ 
        static CSwimSysAgentObserver* NewL( CSwimReaderIF* aReaderIF );

        /**
        * Destructor.
        */
        virtual ~CSwimSysAgentObserver();

    public: // New functions
        
        /**
        * Starts listener
        * @return void 
        */
        void Start();

    private: 

        /**
        * C++ default constructor.
        * @param aReaderIF Pointer to ReaderIF owning this observer
        */ 
        CSwimSysAgentObserver( CSwimReaderIF* aReaderIF );

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
        // SIM status property
        RProperty       iProperty;
        // Pointer to ReaderIF owning this observer. Not owned.
        CSwimReaderIF*  iReaderIF;
    };

#endif      // CSWIMSYSAGENTOBSERVER_H
            
// End of File
