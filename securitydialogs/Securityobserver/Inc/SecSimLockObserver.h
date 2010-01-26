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
* Description:  Observer for Sim Locking events.
*               Shows "Sim restriction on" note if necessary.
*
*
*/

#ifndef     __SECSIMLOCKOBSERVER_H
#define     __SECSIMLOCKOBSERVER_H

#include <e32svr.h>
#include    <e32property.h>

class CSecObsNotify;

class   CSimLockObserver: public CActive
    {
    public:
        /* Creates instance of the CSimLockObserve class.
        *
        * @param aNotifierController CSecObsNotify* pointer to launch differend security
        *  notifiers
        * @return Returns the instance just created.
        */
        static CSimLockObserver* NewL(CSecObsNotify* aNotifierController);
        /**
        * Destructor.
        */
        ~CSimLockObserver();
    public:
        /**
        * Starts asynchronic listening of Sim lock events
        *
        * @return KErrNone: if no errors
        * @return KErrInUse: if already listening
        */
        TInt Start();
        /**
        * Stops asynchronic listening of Sim lock events
        *
        */
        void Stop();
    private: // constructors
        /**
        * C++ default constructor.
        *
        * @param aNotifierController CSecObsNotify* pointer to launch differend security
        *  notifiers
        */
        CSimLockObserver(CSecObsNotify* aNotifierController);
        /**
        * Symbian OS constructor.
        */
        void ConstructL();
    private: // from CActive
        /** @see CActive::RunL() */
        void RunL();
        /** @see CActive::DoCancel() */
        void DoCancel();
    private:  //data
        CSecObsNotify*       iNotifierController; // not owned !
        RProperty            iProperty;
    };

#endif

// End of file
