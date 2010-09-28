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
* Description:  Secui dialog notifier server session
*
*/

#ifndef SECUIDIALOGNOTIFIERSESSION_H
#define SECUIDIALOGNOTIFIERSESSION_H

#include <e32base.h>                            // CSession2
#include "secuidialogoperationobserver.h"    // MSecuiDialogOperationObserver

class CSecuiDialogNotifierServer;
class CSecuiDialogOperation;


/**
 * CSecuiDialogNotifierSession is the session object in secui
 * dialog notifier server (CSecuiDialogNotifierServer).
 */
NONSHARABLE_CLASS( CSecuiDialogNotifierSession ) : public CSession2,
        public MSecuiDialogOperationObserver
    {
    public:     // constructor and destructor
        static CSecuiDialogNotifierSession* NewL();
        ~CSecuiDialogNotifierSession();

    public:     // from CSession2
        void CreateL();
        void ServiceL( const RMessage2& aMessage );

    public:     // from MSecuiDialogOperationObserver
        void OperationComplete();

    private:    // new functions
        CSecuiDialogNotifierSession();
        void ConstructL();
        CSecuiDialogNotifierServer& Server();
        void DispatchMessageL( const RMessage2& aMessage );
        TBool IsOperationCancelled( const RMessage2& aMessage );
        void GetInputBufferL( const RMessage2& aMessage );
        void ServerAuthenticationFailureL( const RMessage2& aMessage );
        void BasicPinOperationL( const RMessage2& aMessage );

    private:    // data
        HBufC8* iInputBuffer;
        CSecuiDialogOperation* iOperationHandler;
    };

#endif  // SECUIDIALOGNOTIFIERSESSION_H

