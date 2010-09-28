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
* Description:  Secui dialog notifier server
*
*/

#ifndef SECUIDIALOGNOTIFIERSERVER_H
#define SECUIDIALOGNOTIFIERSERVER_H

#include <e32base.h>                    // CPolicyServer


/**
 * CSecuiDialogNotifierServer is the work-horse for TSecurityDialogOperation
 * operations  defined for secui dialogs in secdlgimpldefs.h. It's client
 * is CSecuiDialogs class in secuidialognotifier component.
 */
NONSHARABLE_CLASS( CSecuiDialogNotifierServer ) : public CPolicyServer
    {
    public:     // constructor and destructor
        static CSecuiDialogNotifierServer* NewLC();
        ~CSecuiDialogNotifierServer();

    public:     // new functions
        void AddSession();
        void RemoveSession();

    private:    // from CServer2 (via CPolicyServer)
        CSession2* NewSessionL(const TVersion &aVersion, const RMessage2& aMessage) const;

    private:    // new functions
        CSecuiDialogNotifierServer();
        void ConstructL();

    private:    // data
        TInt iSessionCount;
    };

#endif  // SECUIDIALOGNOTIFIERSERVER_H

