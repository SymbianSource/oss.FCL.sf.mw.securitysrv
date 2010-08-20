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
* Description:  CSecurityDialogs active object
*
*/

#ifndef SECURITYDIALOGS_H
#define SECURITYDIALOGS_H

#include <e32base.h>                    // CActive
#include <secdlgimpldefs.h>             // TSecurityDialogOperation
#include "securitydialognotifiersrv.h"  // RSecurityDialogNotifierSrv


/**
 * Security Dialogs
 * CSecurityDialogs class provides implementation for the security dialog
 * operations (TSecurityDialogOperation) defined in secdlgimpldefs.h.
 * When RNotifier class is used to open security dialogs, CSecurityDialogNotifier
 * class calls CSecurityDialogs to show the requested dialogs. CSecurityDialogs
 * uses RSecurityDialogNotifierSrv server to do the actual work.
 */
NONSHARABLE_CLASS( CSecurityDialogs ) : public CActive
    {
    public:     // constructors and destructor
        static CSecurityDialogs* NewL( TBool& aIsDeleted );
        ~CSecurityDialogs();

    public:     // new functions
        /**
        * Starts handling the requested operation.
        * @param aBuffer - Data passed from client RNotifier::StartNotifierAndGetResponse() call
        * @param aReplySlot - Identifies which message argument to use for the reply
        * @param aMessage - Encapsulates the client request
        */
        void StartLD( const TDesC8& aBuffer, TInt aReplySlot, const RMessagePtr2& aMessage );

    protected:  // from CActive
        void RunL();
        void DoCancel();
        TInt RunError( TInt aError );

    private:    // new functions
        CSecurityDialogs( TBool& aIsDeleted );

    private:    // data
        TBool& iIsDeleted;

        TInt iRetry;
        TInt iReplySlot;
        RMessagePtr2 iMessagePtr;
        TSecurityDialogOperation iOperation;

        RSecurityDialogNotifierSrv iServer;
        HBufC8* iInputBuffer;
        TAny* iOutputBuffer;
        TPtr8 iOutputPtr;
    };

#endif  // SECURITYDIALOGS_H

