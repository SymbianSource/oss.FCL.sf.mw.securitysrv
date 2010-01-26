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
* Description:  Observer for ETel security events. Uses SecurityNotifier
*               which calls appropriate dialog to be opened from SecUi.
*
*
*/


#ifndef     __SECURITYOBSERVER_H_
#define     __SECURITYOBSERVER_H_

//  INCLUDES

#include <e32base.h>
#include <etelmm.h>
#include    <e32property.h>
#include "SecObsNotify.h"
#include "SecurityObserver.hrh"
#include "SecSimLockObserver.h"

class CSecurityObserver : public CActive
    {
    public:
        /**
        * Creates instance of the CSecurityObserver class.
        *
        * @return Returns the instance just created.
        */
        static  CSecurityObserver* NewL();
        /**
        * Destructor.
        */
        ~CSecurityObserver();
    public:
        /**
        * Starts asynchronic listening security events
        *
        */
        void StartListen();
        /**
        * Is pin1 required or not
        *
        * @return ETrue: required
        *         EFalse: not required
        */
        inline TBool IsPin1Required() const;
        /**
        * Is puk1 required or not
        *
        * @return ETrue: required
        *         EFalse: not required
        */
        inline TBool IsPuk1Required() const;
        /**
        * Is security code required or not
        *
        * @return ETrue: required
        *         EFalse: not required
        */
        inline TBool IsPassPhraseRequired() const;
        /**
        * Is UPIN code required or not
        *
        * @return ETrue: required
        *         EFalse: not required
        */
        inline TBool IsUPinRequired() const;

        //inline TBool CSecurityObserver::IsUSimAppPinRequired() const
        /**
        * Is UPUK code required or not
        *
        * @return ETrue: required
        *         EFalse: not required
        */
        inline TBool IsUPukRequired() const;
    protected:
        /**
        * Symbian OS constructor.
        */
        void ConstructL();
        /**
        * C++ default constructor.
        */
        CSecurityObserver();
    private:    //  from CActive
        /** @see CActive::RunL() */
        void DoCancel();
        /** @see CActive::RunL() */
        void RunL();
    private:  // data
	/*****************************************************
	*	Series 60 Customer / ETel
	*	Series 60  ETel API
	*****************************************************/
        RTelServer       iServer;
		RMobilePhone	iPhone;
		CSecObsNotify* iNotifierController;
        CSimLockObserver* iSimLockObserver;
		RMobilePhone::TMobilePhoneSecurityEvent iEvent;
        RProperty iProperty;
        TBool iPin1Required;
        TBool iPuk1Required;
        TBool iPassPhraseRequired;
        TBool iUPinRequired;
        TBool iUPukRequired;
        TBool iUSimAppPinRequired;
        };

    inline TBool CSecurityObserver::IsPin1Required() const
        {return iPin1Required;}
    inline TBool CSecurityObserver::IsPuk1Required() const
        {return iPuk1Required;}
    inline TBool CSecurityObserver::IsPassPhraseRequired() const
        {return iPassPhraseRequired;}
    inline TBool CSecurityObserver::IsUPinRequired() const
        {return iUPinRequired;}
    inline TBool CSecurityObserver::IsUPukRequired() const
        {return iUPukRequired;}
#endif

// End of file
