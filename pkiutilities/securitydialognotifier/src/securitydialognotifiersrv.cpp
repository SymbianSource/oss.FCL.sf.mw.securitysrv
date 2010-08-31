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

#include "securitydialognotifiersrv.h"      // RSecurityDialogNotifierSrv
#include "../../securitydialognotifiersrv/inc/securitydialognotifierservername.h"
#include "securitydialogstrace.h"           // TRACE macro


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// RSecurityDialogNotifierSrv::RSecurityDialogNotifierSrv()
// ---------------------------------------------------------------------------
//
RSecurityDialogNotifierSrv::RSecurityDialogNotifierSrv()
    {
    }

// ---------------------------------------------------------------------------
// RSecurityDialogNotifierSrv::~RSecurityDialogNotifierSrv()
// ---------------------------------------------------------------------------
//
RSecurityDialogNotifierSrv::~RSecurityDialogNotifierSrv()
    {
    }

// ---------------------------------------------------------------------------
// RSecurityDialogNotifierSrv::Connect()
// ---------------------------------------------------------------------------
//
TInt RSecurityDialogNotifierSrv::Connect()
    {
    TRACE( "RSecurityDialogNotifierSrv::Connect" );
    const TInt KMaxCreateSessionAttepmts = 2;
    TInt retry = KMaxCreateSessionAttepmts;
    FOREVER
        {
        TInt err = CreateSession( KSecurityDialogNotifierServerName, Version() );
        TRACE( "RSecurityDialogNotifierSrv::Connect, create session err=%d", err );
        if( err != KErrNotFound && err != KErrServerTerminated )
            {
            return err;
            }

        if( --retry == 0 )
            {
            return err;
            }

        err = StartServer();
        if( err != KErrNone && err != KErrAlreadyExists )
            {
            return err;
            }
        }
    }

// ---------------------------------------------------------------------------
// RSecurityDialogNotifierSrv::Version()
// ---------------------------------------------------------------------------
//
TVersion RSecurityDialogNotifierSrv::Version() const
    {
    return TVersion( KSecurityDialogNotifierServerMajorVersionNumber,
            KSecurityDialogNotifierServerMinorVersionNumber,
            KSecurityDialogNotifierServerBuildVersionNumber );
    }

// ---------------------------------------------------------------------------
// RSecurityDialogNotifierSrv::SecurityDialogOperationL()
// ---------------------------------------------------------------------------
//
void RSecurityDialogNotifierSrv::SecurityDialogOperation(
        TSecurityDialogOperation aOperation,
        const TDesC8& aInputBuffer, TDes8& aOutputBuffer,
        TRequestStatus& aStatus )
    {
    TRACE( "RSecurityDialogNotifierSrv::SecurityDialogOperation" );
    iArgs = TIpcArgs( &aInputBuffer, &aOutputBuffer );
    SendReceive( aOperation, iArgs, aStatus );
    }

// ---------------------------------------------------------------------------
// RSecurityDialogNotifierSrv::CancelOperation()
// ---------------------------------------------------------------------------
//
void RSecurityDialogNotifierSrv::CancelOperation()
    {
    TRACE( "RSecurityDialogNotifierSrv::CancelOperation" );
    SendReceive( KSecurityDialogCancelOperation );
    }

// ---------------------------------------------------------------------------
// RSecurityDialogNotifierSrv::StartServer()
// ---------------------------------------------------------------------------
//
TInt RSecurityDialogNotifierSrv::StartServer()
{
    TRACE( "RSecurityDialogNotifierSrv::StartServer, begin" );
    RProcess server;
    TInt err = server.Create( KSecurityDialogNotifierServerName, KNullDesC );
    if( err )
        {
        TRACE( "RSecurityDialogNotifierSrv::StartServer, create failed, err=%d", err );
        return err;
        }

    TRequestStatus status;
    server.Rendezvous( status );
    if( status == KRequestPending )
        {
        server.Resume();
        }
    else
        {
        server.Kill( KErrNone );
        }

    TRACE( "RSecurityDialogNotifierSrv::StartServer, waiting rendezvous" );
    User::WaitForRequest( status );
    if( server.ExitType() == EExitPanic )
        {
        err = KErrGeneral;
        }
    else
        {
        err = status.Int();
        }
    server.Close();

    TRACE( "RSecurityDialogNotifierSrv::StartServer, end err=%d", err );
    return err;
}

