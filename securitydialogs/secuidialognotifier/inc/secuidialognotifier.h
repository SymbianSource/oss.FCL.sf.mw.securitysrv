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
* Description:  Secui dialog notifier
*
*/

#ifndef SECUIDIALOGNOTIFIER_H
#define SECUIDIALOGNOTIFIER_H

#include <eiknotapi.h>                  // MEikSrvNotifierManager

class CSecuiDialogs;


/**
 * Secui Dialog Notifier
 * CSecuiDialogNotifier is a RNotifier that implements secui dialogs
 * defined in secdlgimpldefs.h (see TSecurityDialogOperation operations).
 */
NONSHARABLE_CLASS( CSecuiDialogNotifier ) : public CBase, public MEikSrvNotifierBase2
    {
    public:     // constructors and destructor
        static CSecuiDialogNotifier* NewL();
        ~CSecuiDialogNotifier();

    private:    // from MEikSrvNotifierBase2
        void Release();
        TNotifierInfo RegisterL();
        TNotifierInfo Info() const;
        void StartL( const TDesC8& aBuffer, TInt aReplySlot, const RMessagePtr2& aMessage );
        TPtrC8 StartL( const TDesC8& aBuffer );
        void Cancel();
        TPtrC8 UpdateL( const TDesC8& aBuffer );

    private:    // new functions
        CSecuiDialogNotifier();
        void ConstructL();
        void DoStartL( const TDesC8& aBuffer, TInt aReplySlot, const RMessagePtr2& aMessage );

    private:    // data
        CSecuiDialogs* iSecuiDialogs;
        TBool iIsSecuiDialogsDeleted;
    };

#endif // SECUIDIALOGNOTIFIER_H

