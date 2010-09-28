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
* Description:  Basic PIN query operation in secui dialog
*
*/

#ifndef SECUIDIALOGOPERBASICPINQUERY_H
#define SECUIDIALOGOPERBASICPINQUERY_H

#include "secuidialogoperation.h"    // CSecuiDialogOperation
#include <secdlgimpldefs.h>             // TPINValue

class MSecuiDialogOperationObserver;
class CSecQueryUi;


/**
 * CBasicPinQueryOperation is a CSecuiDialogOperation that handles
 * EEnterPIN, EEnablePIN, EChangePIN, and EDisablePIN operations.
 */
NONSHARABLE_CLASS( CBasicPinQueryOperation ) : public CSecuiDialogOperation
    {
    public:     // constructors and destructor
        static CBasicPinQueryOperation* NewL( MSecuiDialogOperationObserver& aObserver,
                const RMessage2& aMessage, TInt aReplySlot );
        ~CBasicPinQueryOperation();

    public:     // from CSecuiDialogOperation
        void StartL( const TDesC8& aBuffer );
        void CancelOperation();

    protected:  // from CActive (via CSecuiDialogOperation)
        void RunL();
        void DoCancel();

    private:    // new functions
        CBasicPinQueryOperation( MSecuiDialogOperationObserver& aObserver,
                const RMessage2& aMessage, TInt aReplySlot );
        void ReturnResultL( TInt aErrorCode );

    private:    // data
        const TPINInput* iPinInput;
        TPINValue iPinValue;
        TBool iStartUp;
    };

#endif  // SECUIDIALOGOPERBASICPINQUERY_H

