/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Implementation of RGbaServerSession
*
*/


#include "GbaCommon.h"
#include "GbaServerSession.h"
#include "GBALogger.h"

// Number of message slots to reserve for this client server session.
// In this example we can have one asynchronous request outstanding 
// and one synchronous request in progress.
static const TUint KDefaultMessageSlots = 2;
static const TUid KServerUid3 = {0x20029F0A};


static TInt StartServer();
static TInt CreateServerProcess();

// -----------------------------------------------------------------------------
// RGbaServerSession::RGbaServerSession()
// -----------------------------------------------------------------------------
//
RGbaServerSession::RGbaServerSession()
:   RSessionBase(), iGbaInputBuffer(NULL, 0, 0), iGbaOutputBuffer(NULL, 0, 0)
    {
    // No implementation required.
    }

// -----------------------------------------------------------------------------
// RGbaServerSession::Connect()
// -----------------------------------------------------------------------------
//
TInt RGbaServerSession::Connect()
    {
    TInt error = ::StartServer();
    GBA_TRACE_DEBUG(("RGbaServerSession::Connect"));
    if (KErrNone == error)
        {
        GBA_TRACE_DEBUG(("RGbaServerSession::Connect 1"));
        error = CreateSession(KGbaServerName,
                              Version(),
                              KDefaultMessageSlots);
        }
    
    return error;
    }


// -----------------------------------------------------------------------------
// RGbaServerSession::Version()
// -----------------------------------------------------------------------------
//
TVersion RGbaServerSession::Version() const
    {
    return(TVersion(KGbaServMajorVersionNumber,
                    KGbaServMinorVersionNumber,
                    KGbaServBuildVersionNumber));
    }


// -----------------------------------------------------------------------------
// RGbaServerSession::RequestBootstrap()
// -----------------------------------------------------------------------------
//
void RGbaServerSession::RequestBootstrap( TGBABootstrapInputParams& aInput, TGBABootstrapOutputParams& aOutput, TRequestStatus& aStatus) 
    {
    iGbaInputBuffer.Set(reinterpret_cast <TUint8*>(&aInput), sizeof(TGBABootstrapInputParams), sizeof(TGBABootstrapInputParams));
    iGbaOutputBuffer.Set(reinterpret_cast <TUint8*>(&aOutput), sizeof(TGBABootstrapOutputParams), sizeof(TGBABootstrapOutputParams));
    TIpcArgs messageParameters(&iGbaInputBuffer, &iGbaOutputBuffer);
    SendReceive(EGbaServRequestBootstrap, messageParameters, aStatus);
    }


// -----------------------------------------------------------------------------
// RGbaServerSession::CancelBootstrap()
// -----------------------------------------------------------------------------
//
TInt RGbaServerSession::CancelBootstrap() const
    {
    return SendReceive(EGbaServCancelRequestBootstrap);
    }

// -----------------------------------------------------------------------------
// RGbaServerSession::WriteOption()
// -----------------------------------------------------------------------------
//
TInt RGbaServerSession::WriteOption(TUid optionUid, const TDesC8& aValue) const
    {
    TIpcArgs messageParameters(optionUid.iUid,&aValue); 
    return SendReceive(EGbaServWriteOption,messageParameters);
    }


// -----------------------------------------------------------------------------
// RGbaServerSession::IsGBAUSupported()
// -----------------------------------------------------------------------------
//
TInt RGbaServerSession::IsGBAUSupported( TBool& aIsGBAUSupported )
    {

    TPckg<TBool> pckg( aIsGBAUSupported );
    TIpcArgs messageParameters( &pckg ); 
    TInt err = SendReceive(EGbaServIsGBAUSupported,messageParameters);
    GBA_TRACE_DEBUG_NUM(("returned error is %d"), err );
    return err;
    }


// -----------------------------------------------------------------------------
// StartServer()
// -----------------------------------------------------------------------------
//
static TInt StartServer()
    {
    TInt result;

    TFindServer findGbaServer(KGbaServerName);
    TFullName name;

    result = findGbaServer.Next(name);
    if ( result == KErrNone )
        {
        return KErrNone;
       }

    result = CreateServerProcess();
    if ( result != KErrNone )
        {
        return  result;
        }

    return  KErrNone;
    }


// -----------------------------------------------------------------------------
// CreateServerProcess()
// -----------------------------------------------------------------------------
//
static TInt CreateServerProcess()
    {
    TInt result;
    const TUidType serverUid(KNullUid, KNullUid, KServerUid3);
    RProcess server;
    result = server.Create( KGbaServerName, _L(""), serverUid );
    if ( result != KErrNone )
        {
        return  result;
        }

    TRequestStatus stat;
    server.Rendezvous(stat);
    if (stat!=KRequestPending)
        {
        // abort startup
        server.Kill(0);
        }
    else
        {
        // logon OK - start the server
        server.Resume();
        }
    // wait for start or death
    User::WaitForRequest(stat);
    // we can't use the 'exit reason' if the server panicked as this
    // is the panic 'reason' and may be '0' which cannot be distinguished
    // from KErrNone
    TInt r=(server.ExitType()==EExitPanic) ? KErrGeneral : stat.Int();
    server.Close();

    return  r;
    }
    

//EOF
