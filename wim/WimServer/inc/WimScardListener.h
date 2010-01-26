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
* Description:  Listens for Smard Card events
*
*/

#ifndef CWIMSCARDLISTENER_H
#define CWIMSCARDLISTENER_H

//  INCLUDES
#include <e32std.h>
#include "Scard.h"
#include "ScardListener.h"

// FORWARD DECLARATIONS
class CWimServer;


// CLASS DECLARATION

/**
*  A class for listening card events.
*  Each listener can listen only one reader at time, so every reader
*  has its own CrWimScardListener.
*  The maximum amount of listeners is 8.
*
*  @since Series60 2.1
*/
class CWimScardListener : public CScardListener
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        * @param aScard Pointer to RScard object
        * @param aUiReaderID ReaderID
        * @param aName Reader name
        * @return void
        */
        static CWimScardListener* NewL( RScard* aScard,
                                        TUint8 aUiReaderID,
                                        const TScardReaderName& aName );

        /**
        * Destructor.
        */
        ~CWimScardListener();

    private:

        /**
        * C++ default constructor.
        * @param aScard Pointer to RScard object
        * @param aUiReaderID ReaderID
        * @param aName Reader name
        * @return void
        */
        CWimScardListener( RScard* aScard,
                           TUint8 aUiReaderID,
                           const TScardReaderName& aName );
        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        /**
        * From CScardListener. Process event notified by Scard listener
        * @param aEvent Event from Scard
        * @return void
        */
        virtual void ProcessEvent( const TScardServiceStatus aEvent );

    private:
        // Reader ID
        TUint8       iUiReaderID;
        // Name of reader we want the listen
        TScardReaderName iName;
        // Pointer to CWimServer
        CWimServer* iServer;
        
    };

#endif      // CWIMSCARDLISTENER_H

