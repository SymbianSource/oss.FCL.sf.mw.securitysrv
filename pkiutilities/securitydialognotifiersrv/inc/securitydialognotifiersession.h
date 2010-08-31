/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Security dialog notifier server session
*
*/

#ifndef SECURITYDIALOGNOTIFIERSESSION_H
#define SECURITYDIALOGNOTIFIERSESSION_H

#include <e32base.h>                            // CSession2
#include "securitydialogoperationobserver.h"    // MSecurityDialogOperationObserver

class CSecurityDialogNotifierServer;
class CSecurityDialogOperation;


/**
 * CSecurityDialogNotifierSession is the session object in security
 * dialog notifier server (CSecurityDialogNotifierServer).
 */
NONSHARABLE_CLASS( CSecurityDialogNotifierSession ) : public CSession2,
        public MSecurityDialogOperationObserver
    {
    public:     // constructor and destructor
        static CSecurityDialogNotifierSession* NewL();
        ~CSecurityDialogNotifierSession();

    public:     // from CSession2
        void CreateL();
        void ServiceL( const RMessage2& aMessage );

    public:     // from MSecurityDialogOperationObserver
        void OperationComplete();

    private:    // new functions
        CSecurityDialogNotifierSession();
        void ConstructL();
        CSecurityDialogNotifierServer& Server();
        void DispatchMessageL( const RMessage2& aMessage );
        TBool IsOperationCancelled( const RMessage2& aMessage );
        void ServerAuthenticationFailureL( const RMessage2& aMessage );
        void GetInputBufferL( const RMessage2& aMessage );

    private:    // data
        HBufC8* iInputBuffer;
        CSecurityDialogOperation* iOperationHandler;
    };

#endif  // SECURITYDIALOGNOTIFIERSESSION_H

