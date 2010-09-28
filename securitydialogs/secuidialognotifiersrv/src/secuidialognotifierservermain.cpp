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

#include "secuidialognotifierserver.h"   // CSecuiDialogNotifierServer
#include "secuidialognotifierservername.h" // KSecuiDialogNotifierServerName
#include "secuidialogstrace.h"           // TRACE macro


// ---------------------------------------------------------------------------
// MainL()
// ---------------------------------------------------------------------------
//
LOCAL_C void MainL()
    {
    TRACE( "SecuiDialogNotifierServer: MainL" );
    CActiveScheduler* scheduler = new( ELeave ) CActiveScheduler;
    CleanupStack::PushL( scheduler );
    CActiveScheduler::Install( scheduler );

    CSecuiDialogNotifierServer* server = CSecuiDialogNotifierServer::NewLC();
    User::LeaveIfError( User::RenameThread( KSecuiDialogNotifierServerName ) );
    TRACE( "SecuiDialogNotifierServer: MainL, rendezvous" );

    RProcess::Rendezvous( KErrNone );
    TRACE( "SecuiDialogNotifierServer: MainL, activescheduler start" );
    CActiveScheduler::Start();

    TRACE( "SecuiDialogNotifierServer: MainL, cleanup" );
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
    __ASSERT_ALWAYS( !err, User::Panic( KSecuiDialogNotifierServerName, err ) );

    delete cleanup;
    return err;
    }

