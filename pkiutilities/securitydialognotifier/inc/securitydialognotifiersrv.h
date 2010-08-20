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
* Description:  Security dialog notifier server, client-side API.
*
*/

#ifndef SECURITYDIALOGNOTIFIERSRV_H
#define SECURITYDIALOGNOTIFIERSRV_H

#include <e32std.h>             // RSessionBase
#include <secdlgimpldefs.h>     // TSecurityDialogOperation


/**
 * Security Dialog Notifier Server API
 * Security dialogs provide TSecurityDialogOperation functionality defined in secdlgimpldefs.h.
 * Implementation consist of two components: a notifier DLL (CSecurityDialogNotifier, ECom DLL)
 * and a server executable (CSecurityDialogNotifierSrv). CSecurityDialogNotifier runs in UIKON
 * server that has limited capabilities. It passes the requests to the server component
 * (CSecurityDialogNotifierSrv) that provides the actual functionality.
 */
NONSHARABLE_CLASS( RSecurityDialogNotifierSrv ) : public RSessionBase
    {
    public:     // constructors and destructor
        RSecurityDialogNotifierSrv();
        ~RSecurityDialogNotifierSrv();

    public:     // new functions
        TInt Connect();
        TVersion Version() const;
        void SecurityDialogOperation( TSecurityDialogOperation aOperation,
                const TDesC8& aInputBuffer, TDes8& aOutputBuffer,
                TRequestStatus& aStatus );
        void CancelOperation();

    private:    // new functions
        TInt StartServer();

    private:    // data
        TIpcArgs iArgs;
    };

#endif // SECURITYDIALOGNOTIFIERSRV_H

