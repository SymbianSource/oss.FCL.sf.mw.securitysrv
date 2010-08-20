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
* Description:  Security dialog notifier
*
*/

#ifndef SECURITYDIALOGNOTIFIER_H
#define SECURITYDIALOGNOTIFIER_H

#include <eiknotapi.h>                  // MEikSrvNotifierManager

class CSecurityDialogs;


/**
 * Security Dialog Notifier
 * CSecurityDialogNotifier is a RNotifier that implements security dialogs
 * defined in secdlgimpldefs.h (see TSecurityDialogOperation operations).
 */
NONSHARABLE_CLASS( CSecurityDialogNotifier ) : public CBase, public MEikSrvNotifierBase2
    {
    public:     // constructors and destructor
        static CSecurityDialogNotifier* NewL();
        ~CSecurityDialogNotifier();

    private:    // from MEikSrvNotifierBase2
        void Release();
        TNotifierInfo RegisterL();
        TNotifierInfo Info() const;
        void StartL( const TDesC8& aBuffer, TInt aReplySlot, const RMessagePtr2& aMessage );
        TPtrC8 StartL( const TDesC8& aBuffer );
        void Cancel();
        TPtrC8 UpdateL( const TDesC8& aBuffer );

    private:    // new functions
        CSecurityDialogNotifier();
        void ConstructL();
        void DoStartL( const TDesC8& aBuffer, TInt aReplySlot, const RMessagePtr2& aMessage );

    private:    // data
        CSecurityDialogs* iSecurityDialogs;
        TBool iIsSecurityDialogsDeleted;
    };

#endif // SECURITYDIALOGNOTIFIER_H

