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
* Description:  Security dialog notifier server
*
*/

#ifndef SECURITYDIALOGNOTIFIERSERVER_H
#define SECURITYDIALOGNOTIFIERSERVER_H

#include <e32base.h>                    // CPolicyServer


/**
 * CSecurityDialogNotifierServer is the work-horse for TSecurityDialogOperation
 * operations  defined for security dialogs in secdlgimpldefs.h. It's client
 * is CSecurityDialogs class in securitydialognotifier component.
 */
NONSHARABLE_CLASS( CSecurityDialogNotifierServer ) : public CPolicyServer
    {
    public:     // constructor and destructor
        static CSecurityDialogNotifierServer* NewLC();
        ~CSecurityDialogNotifierServer();

    public:     // new functions
        void AddSession();
        void RemoveSession();

    private:    // from CServer2 (via CPolicyServer)
        CSession2* NewSessionL(const TVersion &aVersion, const RMessage2& aMessage) const;

    private:    // new functions
        CSecurityDialogNotifierServer();
        void ConstructL();

    private:    // data
        TInt iSessionCount;
    };

#endif  // SECURITYDIALOGNOTIFIERSERVER_H

