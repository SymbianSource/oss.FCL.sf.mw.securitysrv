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
* Description:  Represents a session for a client thread on the server-side.
*
*/

#ifndef CWIMSESSION_H
#define CWIMSESSION_H

#include "WimClsv.h"
#include "WimTimer.h"
#include "Wimi.h"            //WIMI definitions

// FORWARD DECLARATION
class CWimServer;
class CWimCertHandler;
class CWimAuthObjHandler;
class CWimTokenHandler;
class CWimKeyMgmtHandler;
class CWimSignTextHandler;
class CWimMemMgmt;
class CWimUtilityFuncs;
class CWimUtilityFuncs;
class CWimOmaProvisioning;
class CWimJavaProvisioning;
class CWimTrustSettingsHandler;

/**
*  A session acts as a channel of communication between the client and
*  the server.
*  A client thread can have multiple concurrent sessions with a server.
*
*  @since Series60 2.1
*/
class CWimSession : public CSession2, public MWimTimerListener
    {

    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CWimSession* NewL( CWimServer* aWimServer );

        /**
        * Destructor.
        */
        virtual ~CWimSession();

    public: // New functions

        /**
        * Handles the servicing of client requests to the server.
        * @param    aMessage Encapsulates a client request.
        * @return   void
        */
        virtual void ServiceL( const RMessage2& aMessage );

        /**
        * Complete NotifyOnRemoval message, meaning that card is removed.
        * @return   void
        */
        void NotifyComplete();

    private:

        /**
        * C++ default constructor.
        */
        CWimSession();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL( CWimServer* aWimServer );

        /**
        * Handles client requests and forwards
        * them to appropriate handler class.
        * @param    aMessage Encapsulates a client request.
        * @return   void
        */
        void DispatchMessageL( const RMessage2& aMessage );

        /**
        * Fetches the list of the references of WIMs currently associated
        * with readers.
        * @return   void
        */
        void GetWimRefListL();

        /**
        * Fetches the count of WIM cards in use.
        * @return   void
        */
        void GetWimCountL();

        /**
        * Timer for closing connection to the WIM card.
        * @return   void
        */
        void TimerExpired();

        /**
        * NotifyOnRemoval message received. Store the message for completing.
        * @return   void
        */
        void NotifyOnRemovalL();

        /**
        * Cancel NotifyOnRemoval. Complete notification message with KErrCancel.
        * @return   void
        */
        void CancelNotifyOnRemoval();

        /**
        * Check if current request accesses HW.
        * @param	TInt	Request ID
        * @return   TBool
        */
        TBool RequestAccessesHW( TInt aFunction );

        /**
        * Re-initializes WIMlib.
        * Executed when card data has been changed.
        * @return   void
        */
        void RefreshWimi();

    private:    // Data
        // total number of resources allocated
        TInt                      iResourceCount;
        // Pointer to the server. Not owned.
        CWimServer*               iWimSvr;
        // Pointer for memory management. Owned.
        CWimMemMgmt*              iWimMgmt;
        // Pointer to timer. Owned.
        //CWimTimer*                iTimer;
        // Certificate Handler class pointer. Owned.
        CWimCertHandler*          iWimCertHandler;
        // Authentication Object Handler class pointer. Owned.
        CWimAuthObjHandler*       iWimAuthObjHandler;
        // Token Handler class pointer. Owned.
        CWimTokenHandler*         iWimTokenHandler;
        // Key Management Handler class pointer. Owned.
        CWimKeyMgmtHandler*       iWimKeyMgmtHandler;
        // Pointer to Sign Text handler. Owned.
        CWimSignTextHandler*      iWimSignTextHandler;
        // Utility functions. Owned.
        CWimUtilityFuncs*         iWimUtilFuncs;
        // Pointer to OMA Provisioning class. Owned.
        CWimOmaProvisioning*      iWimOmaProvisioning;
        
        CWimJavaProvisioning*     iWimJavaProvisioning;
        // Pointer to WimTrustSettingsHandler. Owned.
        CWimTrustSettingsHandler* iWimTrustSettingsHandler;
        // NotifyOnRemoval Message, to be completed. Owned.
        RMessage2*                iNotifyMessage;
        // Message to be server. Owned.
        RMessage2*                iMessage;
    };

#endif      // CWIMSESSION_H

// End of File
