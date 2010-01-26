/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  
*
*/



#ifndef __DEVENCSTARTERUTILS_H__
#define __DEVENCSTARTERUTILS_H__

// INCLUDES
#include <e32std.h>
#include <eikenv.h>
#include <w32std.h>
//#include <akncapserverclient.h>	// for RAknUiServer

#include "DevEncStarterdef.h"
#include "DevEncStarterAppStarter.h"

// FORWARD DECLARATIONS

// CLASS DECLARATION
/**
 * Callback class for CDevEncStarterUtils.
 */
class MUtilsCallback
    {
    public:
        /**
         * Called when an application is started using CDevEncStarterUtils.
         * @param aUid The UID of the started application.
         */
    	virtual void AppStarted( const TUint32& aUid ) = 0;
    };

/**
 * Containts various utility functions.
 */
class CDevEncStarterUtils : public CBase,
                            public MAppStarterCallback
    {
    public:
        /**
         * Symbian two-phased construction.
         * @param aEikEnv Reference to the Eikon environment.
         */
        static CDevEncStarterUtils* NewL();

        /*
        * Destructor
        */
        virtual ~CDevEncStarterUtils();

        /*
        * Sends a key event to the application that currently has focus.
        * @param aUid the UID of the application to start
        * @param aCallback Reference to callback class. If specified, the
        * class's AppStarted function will be called on completion of the
        * request.
        * @leave Symbian error code.
        */
        void StartAppL( const TUint32& aUid, MUtilsCallback* aCallback = 0 );

        /*
        * From MAppStarterCallback. Calls MUtilsCallback::AppStarted if
        * specified.
        * @param aUid The UID of the started application
        */
        void AppStarted( const TUint32& aUid );

    private:
    // Functions

        /*
        * Second-phase constructor.
        */
        void ConstructL();

        /*
        * C++ constructor.
        */
        CDevEncStarterUtils();

    // Data
        /*
        * Application starter. Owned
        */
        CDevEncStarterStarter* iAppStarter;

        /*
        * Reference to the callback object. Not owned.
        */
        MUtilsCallback* iCallback;
};

#endif //__DEVENCSTARTERUTILS_H__

// End of file
