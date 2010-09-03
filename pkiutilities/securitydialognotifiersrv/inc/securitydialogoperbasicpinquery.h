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
* Description:  Basic PIN query operation in security dialog
*
*/

#ifndef SECURITYDIALOGOPERBASICPINQUERY_H
#define SECURITYDIALOGOPERBASICPINQUERY_H

#include "securitydialogoperation.h"    // CSecurityDialogOperation
#include <secdlgimpldefs.h>             // TPINValue

class MSecurityDialogOperationObserver;
class CSecQueryUi;


/**
 * CBasicPinQueryOperation is a CSecurityDialogOperation that handles
 * EEnterPIN, EEnablePIN, EChangePIN, and EDisablePIN operations.
 */
NONSHARABLE_CLASS( CBasicPinQueryOperation ) : public CSecurityDialogOperation
    {
    public:     // constructors and destructor
        static CBasicPinQueryOperation* NewL( MSecurityDialogOperationObserver& aObserver,
                const RMessage2& aMessage, TInt aReplySlot );
        ~CBasicPinQueryOperation();

    public:     // from CSecurityDialogOperation
        void StartL( const TDesC8& aBuffer );
        void CancelOperation();

    protected:  // from CActive (via CSecurityDialogOperation)
        void RunL();
        void DoCancel();

    private:    // new functions
        CBasicPinQueryOperation( MSecurityDialogOperationObserver& aObserver,
                const RMessage2& aMessage, TInt aReplySlot );
        void ReturnResultL( TInt aErrorCode );

    private:    // data
        CSecQueryUi* iQueryUi;
        const TPINInput* iPinInput;
        TPINValue iPinValue;
    };

#endif  // SECURITYDIALOGOPERBASICPINQUERY_H

