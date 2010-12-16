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
*   Notifier class for Security code dialogs. Notifier calls appropriate
*   dialog to be opened from SecUi.
*
*/

#ifndef     __SECURITYNOTIFIER_H
#define     __SECURITYNOTIFIER_H

//  INCLUDES
#include <e32std.h>
#include <e32base.h>
#include <eiknotapi.h>
#include <etelmm.h>
#include <securitynotification.h>


// CONSTANTS

const TUid KSecurityNotifierChannel = {0x1000598F};
const TUid KSecurityNotifierAppServerUid = {0x102071FB};

IMPORT_C CArrayPtr<MEikSrvNotifierBase2>* NotifierArray();   // Notifier array (entry point)


// CLASS DECLARATION

/**
*  CSecurityNotifier class
*
*
*/
class CSecurityNotifier : public CActive,public MEikSrvNotifierBase2
    {
    public:
        /**
        * Creates instance of the CSecurityNotifier class.
        *
        * @return Returns the MEikSrvNotifierBase2 instance just created.
        */
        static MEikSrvNotifierBase2* NewL();
        /**
        * Destructor.
        */
        ~CSecurityNotifier();
    public: // from MEikSrvNotifierBase2
        /**
        * Called when all resources allocated by notifiers should be freed.
        */
        void Release();
        /**
        * From MEikSrvNotifierBase2 - method for registering the EikSrv plugin
        *
        * @return TNotifierInfo - struct containing the plugin priorities etc.
        */
        TNotifierInfo RegisterL();
         /**
        * From MEikSrvNotifierBase2 - method for getting the notifier info
        *
        * @return TNotifierInfo - struct containing the plugin priorities etc.
        */
        TNotifierInfo Info() const;
         /**
        * From MEikSrvNotifierBase2 - method for starting the plugin. Synchronous version
        * @param aBuffer - buffer containing the parameters for the plugin, packaged in TPckg<SAknSoftNotificationParams>
        * @return TPtrC8 - return value to the client, this method will return KNullDesC
        */
        TPtrC8 StartL(const TDesC8& aBuffer);
        /**
        * From MEikSrvNotifierBase2 - method for starting the plugin. Asynchronous version.
        * @param aBuffer - buffer containing parameters for the plugin, packaged in TPckg<SAknSoftNotificationParams>
        * @param aReturnVal - return value of any type. Not used in this case.
        * @param aMessage - RMessage to notify that the execution has ended
        */
        void StartL(const TDesC8& aBuffer,TInt aReturnVal, const RMessagePtr2& aMessage);
        /**
        * From MEikSrvNotifierBase2 - method for cancelling the plugin execution.
        */
        void Cancel();
         /**
        * From MEikSrvNotifierBase2 - method for updating the plugin while execution is in progress
        * @param aBuffer - buffer containing parameters for the plugin, packaged in TPckg<SAknSoftNotificationParams>
        * @return not used, return KNullDesC
        */
        TPtrC8 UpdateL(const TDesC8& /*aBuffer*/);
    public: // From CActive
        /** @see CActive::RunL() */
        void RunL();
        /** @see CActive::DoCancel() */
        void DoCancel();
        /** @see CActive::RunError() */
        TInt RunError( TInt aError );
    private:
        /**
        * C++ default constructor.
        */
        CSecurityNotifier();
        /**
        * Initialize parameters and jumps to RunL
        * @param aBuffer - buffer containing parameters for the plugin, packaged in TPckg<SAknSoftNotificationParams>
        * @param aReturnVal - return value of any type. Not used in this case.
        * @param aMessage - RMessage to notify that the execution has ended
        */
        void GetParamsL(const TDesC8& aBuffer, TInt aReturnVal, const RMessagePtr2& aMessage);
    private: // data
        TNotifierInfo   iInfo;
	/*****************************************************
	*	Series 60 Customer / ETel
	*	Series 60  ETel API
	*****************************************************/
        RTelServer      iServer;
		RMobilePhone	iPhone;
		RMobilePhone::TMobilePhoneSecurityEvent iEvent;
    protected:
        RMessagePtr2    iMessage;               // Received message
        TInt            iReturnVal;             // Return value
        TBool           iStartup;               // System state: true means we are in the middle of a boot.
    };

#endif

// end of file
