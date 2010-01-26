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
* Description:  Observer for phone events. Used to re-active
*               code query after emergency call
*
*
*/

#ifndef     __SECPHONEOBSERVER_H
#define     __SECPHONEOBSERVER_H

#include <e32svr.h>
#include    <e32property.h>
#include "SecurityObserver.hrh"

class CSecObsNotify;
class CSecurityObserver;

class   CPhoneObserver: public CActive
    {
    public:
        /**
        * Creates instance of the CPhoneObserve class.
        *
        * @param aNotifierController CSecObsNotify* pointer to launch differend security
        *  notifiers
        * @param aObserver CSecurityObserver* pointer to securityobserver
        * @return Returns the instance just created.
        */
        static CPhoneObserver* NewL(CSecObsNotify* aNotifierController,CSecurityObserver* aObserver);
        /**
        * Destructor.
        */
        ~CPhoneObserver();
    public:
        /**
        * Starts asynchronic listening of KUidCurrentCall event
        *
        * @param aNotifier TSecurityNotifier: notifier which is activated when call has ended
        * @return KErrNone: if no errors
        * @return KErrInUse: if already listening
        */
        TInt Start(TSecurityNotifier aNotifier);
        /**
        * Stops asynchronic listening of KUidCurrentCall event
        */
        void Stop();
    private: // constructors
        /**
        * C++ default constructor.
        *
        * @param aNotifierController CSecObsNotify* pointer to launch differend security
        *  notifiers
        * @param aObserver CSecurityObserver* pointer to securityobserver
        */
        CPhoneObserver(CSecObsNotify* aNotifierController,CSecurityObserver* aObserver);
        /**
        * Symbian OS constructor.
        */
        void ConstructL();
    private: // from CActive
        /** @see CActive::RunL() */
        void RunL();
        /** @see CActive::DoCancel() */
        void DoCancel();
    private: // data
        CSecObsNotify*       iNotifierController; // not owned !
        CSecurityObserver*   iObserver; //not owned !
        TSecurityNotifier    iNotifier;
        RProperty            iProperty;
    };

#endif

// End of file
