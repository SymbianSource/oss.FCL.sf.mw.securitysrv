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

#include "securitydialognotifierserver.h"   // CSecurityDialogNotifierServer
#include "securitydialognotifierservername.h" // KSecurityDialogNotifierServerName
#include "securitydialogstrace.h"           // TRACE macro


// ---------------------------------------------------------------------------
// MainL()
// ---------------------------------------------------------------------------
//
LOCAL_C void MainL()
    {
    TRACE( "SecurityDialogNotifierServer: MainL" );
    CActiveScheduler* scheduler = new( ELeave ) CActiveScheduler;
    CleanupStack::PushL( scheduler );
    CActiveScheduler::Install( scheduler );

    CSecurityDialogNotifierServer* server = CSecurityDialogNotifierServer::NewLC();
    User::LeaveIfError( User::RenameThread( KSecurityDialogNotifierServerName ) );
    TRACE( "SecurityDialogNotifierServer: MainL, rendezvous" );

    RProcess::Rendezvous( KErrNone );
    TRACE( "SecurityDialogNotifierServer: MainL, activescheduler start" );
    CActiveScheduler::Start();

    TRACE( "SecurityDialogNotifierServer: MainL, cleanup" );
    CleanupStack::PopAndDestroy( server );
    CleanupStack::PopAndDestroy( scheduler );
    }

// ---------------------------------------------------------------------------
// E32Main()
// ---------------------------------------------------------------------------
//
GLDEF_C TInt E32Main()
    {
    CTrapCleanup* cleanup = CTrapCleanup::New();

    TRAPD( err, MainL() );
    __ASSERT_ALWAYS( !err, User::Panic( KSecurityDialogNotifierServerName, err ) );

    delete cleanup;
    return err;
    }

