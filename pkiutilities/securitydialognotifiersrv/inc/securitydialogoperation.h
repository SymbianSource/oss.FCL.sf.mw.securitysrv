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
* Description:  Base class for CSecurityDialog operations
*
*/

#ifndef SECURITYDIALOGOPERATION_H
#define SECURITYDIALOGOPERATION_H

#include <e32base.h>                    // CActive

class MSecurityDialogOperationObserver;


/**
 * CSecurityDialogOperation is a base class for different kinds of operations
 * defined for security dialogs in secdlgimpldefs.h. TSecurityDialogOperation
 * lists the possible operations. A derived class implements each operation
 * separately. This base class works as common API between different operation
 * classes and the main CSecurityDialog class.
 */
NONSHARABLE_CLASS( CSecurityDialogOperation ) : public CActive
    {
    protected:  // constructor
        CSecurityDialogOperation( MSecurityDialogOperationObserver& aObserver,
                const RMessage2& aMessage, TInt aReplySlot );

    public:     // destructor
        ~CSecurityDialogOperation();

    public:     // new function
        virtual void StartL( const TDesC8& aBuffer ) = 0;
        virtual void CancelOperation() = 0;

    protected:  // from CActive
        TInt RunError( TInt aError );

    protected:  // new functions
        void ShowWarningNoteL( const TDesC& aMessage );

    protected:  // data
        MSecurityDialogOperationObserver& iObserver;
        RMessage2 iMessage;
        const TInt iReplySlot;
    };

#endif  // SECURITYDIALOGOPERATION_H

