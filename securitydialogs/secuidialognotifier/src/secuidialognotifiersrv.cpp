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
* Description:  Secui dialog notifier server, client-side API.
*
*/

#include "secuidialognotifiersrv.h"      // RSecuiDialogNotifierSrv
#include "../../secuidialognotifiersrv/inc/secuidialognotifierservername.h"
#include "secuidialogstrace.h"           // TRACE macro


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// RSecuiDialogNotifierSrv::RSecuiDialogNotifierSrv()
// ---------------------------------------------------------------------------
//
RSecuiDialogNotifierSrv::RSecuiDialogNotifierSrv()
    {
    }

// ---------------------------------------------------------------------------
// RSecuiDialogNotifierSrv::~RSecuiDialogNotifierSrv()
// ---------------------------------------------------------------------------
//
RSecuiDialogNotifierSrv::~RSecuiDialogNotifierSrv()
    {
    }

// ---------------------------------------------------------------------------
// RSecuiDialogNotifierSrv::Connect()
// ---------------------------------------------------------------------------
//
TInt RSecuiDialogNotifierSrv::Connect()
    {
    RDEBUG("0", 0);
    const TInt KMaxCreateSessionAttepmts = 2;
    TInt retry = KMaxCreateSessionAttepmts;
    FOREVER
        {
        TInt err = CreateSession( KSecuiDialogNotifierServerName, Version() );
        RDEBUG("err", err);
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
// RSecuiDialogNotifierSrv::Version()
// ---------------------------------------------------------------------------
//
TVersion RSecuiDialogNotifierSrv::Version() const
    {
    return TVersion( KSecuiDialogNotifierServerMajorVersionNumber,
            KSecuiDialogNotifierServerMinorVersionNumber,
            KSecuiDialogNotifierServerBuildVersionNumber );
    }

// ---------------------------------------------------------------------------
// RSecuiDialogNotifierSrv::SecuiDialogOperationL()
// ---------------------------------------------------------------------------
//
void RSecuiDialogNotifierSrv::SecuiDialogOperation(
        TSecurityDialogOperation aOperation,
        const TDesC8& aInputBuffer, TDes8& aOutputBuffer,
        TRequestStatus& aStatus )
    {
    RDEBUG("0", 0);
    iArgs = TIpcArgs( &aInputBuffer, &aOutputBuffer );
    SendReceive( aOperation, iArgs, aStatus );
    }

// ---------------------------------------------------------------------------
// RSecuiDialogNotifierSrv::CancelOperation()
// ---------------------------------------------------------------------------
//
void RSecuiDialogNotifierSrv::CancelOperation()
    {
    RDEBUG("0", 0);
    SendReceive( KSecuiDialogCancelOperation );
    }

// ---------------------------------------------------------------------------
// RSecuiDialogNotifierSrv::StartServer()
// ---------------------------------------------------------------------------
//
TInt RSecuiDialogNotifierSrv::StartServer()
{
    RDEBUG("0", 0);
    RProcess server;
    TInt err = server.Create( KSecuiDialogNotifierServerName, KNullDesC );
    if( err )
        {
        RDEBUG("err", err);
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

    RDEBUG("WaitForRequest", 0);
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

    RDEBUG("err", err);
    return err;
}

