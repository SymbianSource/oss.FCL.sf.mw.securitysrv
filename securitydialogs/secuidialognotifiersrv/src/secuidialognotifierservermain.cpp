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
    RDEBUG("0", 0);
    CActiveScheduler* scheduler = new( ELeave ) CActiveScheduler;
    CleanupStack::PushL( scheduler );
    CActiveScheduler::Install( scheduler );

    CSecuiDialogNotifierServer* server = CSecuiDialogNotifierServer::NewLC();
    User::LeaveIfError( User::RenameThread( KSecuiDialogNotifierServerName ) );
    RDEBUG("Rendezvous 0", 0);

    RProcess::Rendezvous( KErrNone );
    RDEBUG("CActiveScheduler 0", 0);
    CActiveScheduler::Start();

    RDEBUG("PopAndDestroy 0", 0);
    CleanupStack::PopAndDestroy( server );
    CleanupStack::PopAndDestroy( scheduler );
    RDEBUG("0x99", 0x99);
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

