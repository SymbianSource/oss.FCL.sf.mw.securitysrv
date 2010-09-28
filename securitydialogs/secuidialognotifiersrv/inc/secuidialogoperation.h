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
* Description:  Base class for CSecuiDialog operations
*
*/

#ifndef SECUIDIALOGOPERATION_H
#define SECUIDIALOGOPERATION_H

#include <e32base.h>                    // CActive

class MSecuiDialogOperationObserver;


/**
 * CSecuiDialogOperation is a base class for different kinds of operations
 * defined for secui dialogs in secdlgimpldefs.h. TSecurityDialogOperation
 * lists the possible operations. A derived class implements each operation
 * separately. This base class works as common API between different operation
 * classes and the main CSecuiDialog class.
 */
NONSHARABLE_CLASS( CSecuiDialogOperation ) : public CActive
    {
    protected:  // constructor
        CSecuiDialogOperation( MSecuiDialogOperationObserver& aObserver,
                const RMessage2& aMessage, TInt aReplySlot );

    public:     // destructor
        ~CSecuiDialogOperation();

    public:     // new function
        virtual void StartL( const TDesC8& aBuffer ) = 0;
        virtual void CancelOperation() = 0;

    protected:  // from CActive
        TInt RunError( TInt aError );

    protected:  // new functions
        void ShowWarningNoteL( const TDesC& aMessage );

    protected:  // data
        MSecuiDialogOperationObserver& iObserver;
        RMessage2 iMessage;
        const TInt iReplySlot;
    };

#endif  // SECUIDIALOGOPERATION_H

