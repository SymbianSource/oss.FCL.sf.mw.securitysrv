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
* Description: 
*		CSecObsNotify - starts and cancels notifier plug-ins. 
*
*
*/


#ifndef     __SECOBSNOTIFY_H
#define     __SECOBSNOTIFY_H

//  INCLUDES
#include    <e32base.h>
#include    <e32property.h>
#include "SecPhoneObserver.h"
#include "SecurityObserver.hrh"

class CSecurityObserver;

//  CLASS DEFINITIONS
class   CSecObsNotify : public CActive
    {
    public:
        /**
        * Creates instance of the CSecObsNotify class.
        *
        * @param aObserver CSecurityObserver* pointer to securityobserver
        * @return Returns the instance just created.
        */
        static CSecObsNotify* NewL(CSecurityObserver* aObserver);
        /**
        * Destructor.
        */
        ~CSecObsNotify();
    public:
        /**
        * Starts given notifier asyncronously
        *
        * @param aNotifierToStart TSecurityNotifier
        */
        void StartNotifier(TSecurityNotifier aNotifierToStart);
        /**
        * Cancels active notifier.
        */
        void CancelActiveNotifier();
    private:
        /**
        * C++ default constructor.
        *
        * @param aObserver CSecurityObserver* pointer to securityobserver
        */
        CSecObsNotify(CSecurityObserver* aObserver);
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
        RNotifier iNotifier;
        RProperty iProperty;
        CPhoneObserver* iPhoneObserver;
        TBuf8<1> iDummy;
        CSecurityObserver* iObserver;
        TSecurityNotifier iActiveNotifier;
        TSecurityNotifier iPendingNotifier;
   };
#endif

// End of File
