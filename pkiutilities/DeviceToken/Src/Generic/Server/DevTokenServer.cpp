/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Implementation of DevTokenServer
*
*/



#include <e32debug.h>
#include "DevTokenServer.h"
#include "DevTokenUtil.h"
#include "DevCertKeyStoreServer.h"
#include "DevCertKeyStoreSession.h"
#include "DevandTruSrvCertStoreServer.h"
#include "DevandTruSrvCertStoreSession.h"
#include "TrustedSitesServer.h"
#include "TrustedSitesSession.h"
#include "DevTokenServerDebug.h"
#include "DevTokenServerName.h"
#include "DevtokenLog.h"
#include "DevTokenDialog.h"
//  approx 2s
const TInt KServerShutdownDelay = 0x200000; 


// ======== MEMBER FUNCTIONS ========

// CDevTokenServerSession 

// ---------------------------------------------------------------------------
// CDevTokenServerSession::CDevTokenServerSession()
// ---------------------------------------------------------------------------
// 
CDevTokenServerSession::CDevTokenServerSession()
    {
    }


// ---------------------------------------------------------------------------
// CDevTokenServerSession::Server()
// ---------------------------------------------------------------------------
// 
inline CDevTokenServer& CDevTokenServerSession::Server()
    {
    return static_cast<CDevTokenServer&>(const_cast<CServer2&>(*CSession2::Server()));
    }


// ---------------------------------------------------------------------------
// CDevTokenServerSession::CreateL()
// ---------------------------------------------------------------------------
//
void CDevTokenServerSession::CreateL()
    {
    Server().AddSession();
    }


// ---------------------------------------------------------------------------
// CDevTokenServerSession::~CDevTokenServerSession()
// ---------------------------------------------------------------------------
//
CDevTokenServerSession::~CDevTokenServerSession()
    {
    Server().DropSession();
    }



// ---------------------------------------------------------------------------
// CDevTokenServerSession::ServiceL()
// Handle a client request.  Leaving is handled by
// CDevTokenServerSession::ServiceError() which reports the error code to the
// client.
// ---------------------------------------------------------------------------
// 
void CDevTokenServerSession::ServiceL(const RMessage2& aMessage)
    {
    #ifdef _DEBUG
    // OOM testing
    switch (aMessage.Function())
        {
        case EStartOOMTest:
            DevTokenServerDebug::StartOOMTest();
            aMessage.Complete(KErrNone);
            return;

        case EIncHeapFailPoint:
            DevTokenServerDebug::IncHeapFailPoint();
            aMessage.Complete(KErrNone);
            return;

        case EResetHeapFail:
            DevTokenServerDebug::ResetHeapFail();
            aMessage.Complete(KErrNone);
            return;

        case EAllocCount:
            aMessage.Complete(User::CountAllocCells());
            return;
        } 
    #endif

    DoServiceL(aMessage);
    }



// ---------------------------------------------------------------------------
// CDevTokenServerSession::ServiceError()
// Handle an error from ServiceL() A bad descriptor error implies a badly
// programmed client, so panic it - otherwise report the error to the client.
// ---------------------------------------------------------------------------
//  
void CDevTokenServerSession::ServiceError(const RMessage2& aMessage, TInt aError)
    {
    if (aError==KErrBadDescriptor)
        {
        PanicClient(aMessage, EPanicBadDescriptor);
        }

    CSession2::ServiceError(aMessage, aError);
    }


// CDevTokenServer 

// ---------------------------------------------------------------------------
// CDevTokenServer::CDevTokenServer()
// ---------------------------------------------------------------------------
//
inline CDevTokenServer::CDevTokenServer() :
  CServer2(0, ESharableSessions)
    {
    }


// ---------------------------------------------------------------------------
// CDevTokenServer::NewLC()
// ---------------------------------------------------------------------------
//
CServer2* CDevTokenServer::NewLC()
    {
    TRACE_PRINT(" CDevTokenServer::NewLC -->");
    CDevTokenServer* self=new(ELeave) CDevTokenServer;
    CleanupStack::PushL(self);
    self->ConstructL();
    TRACE_PRINT(" CDevTokenServer::NewLC <--");
    return self;
    }


// ---------------------------------------------------------------------------
// CDevTokenServer::~CDevTokenServer()
// ---------------------------------------------------------------------------
//
CDevTokenServer::~CDevTokenServer()
    {
    DevTokenDialog::Cleanup();
    delete iKeyStoreServer;
    delete iCertStoreServer;
    delete iTrustedSitesServer;
    }


// ---------------------------------------------------------------------------
// CDevTokenServer::ConstructL()
// 2nd phase construction - ensure the timer and server objects are running.
// ---------------------------------------------------------------------------
//
void CDevTokenServer::ConstructL()
    {
    TRACE_PRINT(" CDevTokenServer::ConstructL -->");
    TRACE_PRINT(" CDevTokenServer::ConstructL 1");
    DevTokenDialog::InitialiseL();
    StartL( KDevTokenServerName );
    TRACE_PRINT(" CDevTokenServer::ConstructL 2");
    // Ensure that the server still exits even if the 1st client fails to connect
    iShutdown.ConstructL();
    iShutdown.Start();
    }


// ---------------------------------------------------------------------------
// CDevTokenServer::AddSession()
// A new session is being created - cancel the shutdown timer if it was running.
// ---------------------------------------------------------------------------
//
void CDevTokenServer::AddSession()
    {
    ++iSessionCount;
    iShutdown.Cancel();
    }


// ---------------------------------------------------------------------------
// CDevTokenServer::DropSession()
// A session is being destroyed - start the shutdown timer if it is the last session. 
// ---------------------------------------------------------------------------
//
void CDevTokenServer::DropSession()
    {
    if (--iSessionCount==0)
        {
        iShutdown.Start();
        }
    }


// ---------------------------------------------------------------------------
// CDevTokenServer::KeyStoreServerL()
// Lazily create key store server object.
// ---------------------------------------------------------------------------
//
CDevCertKeyStoreServer& CDevTokenServer::KeyStoreServerL() const
    {
    if (!iKeyStoreServer)
        {
        iKeyStoreServer = CDevCertKeyStoreServer::NewL();
        }

    return *iKeyStoreServer;
    }


// ---------------------------------------------------------------------------
// CDevTokenServer::CertStoreServerL()
// Lazily create cert store server object. 
// ---------------------------------------------------------------------------
//
CDevandTruSrvCertStoreServer& CDevTokenServer::CertStoreServerL() const
    {
    if (!iCertStoreServer)
        {
        iCertStoreServer = CDevandTruSrvCertStoreServer::NewL( &TrustedSitesL() );
        }

    return *iCertStoreServer;
    }


// ---------------------------------------------------------------------------
// CDevTokenServer::TrustedSitesL()
// Lazily create trusted site server object. 
// ---------------------------------------------------------------------------
//
CTrustedSitesServer& CDevTokenServer::TrustedSitesL() const
    {
    if( !iTrustedSitesServer )
        {
        iTrustedSitesServer = CTrustedSitesServer::NewL((CDevTokenServer*)this);
        }
    return *iTrustedSitesServer;    
    }


// ---------------------------------------------------------------------------
// CDevTokenServer::NewSessionL()
// Create a new client session.
// ---------------------------------------------------------------------------
//
CSession2* CDevTokenServer::NewSessionL(const TVersion& aVersion, 
                                        const RMessage2& /*aMessage*/) const
    {
    // The token the client wants to talk to is encoded in the major version number
    EDevTokenEnum token = static_cast<EDevTokenEnum>(aVersion.iMajor);  

    // The minor version number represents the version of the protocol
    if ( aVersion.iMinor != KDevTokenProtolVersion )
        {
        User::Leave(KErrNotSupported);
        }

    CDevTokenServerSession* result = NULL;

    switch (token)
        {
        case EDevCertKeyStore:
            result = KeyStoreServerL().CreateSessionL();
            break;

        case EDevCertStore:
        case ETruSrvCertStore:
            result = CertStoreServerL().CreateSessionL();
            break;

        case ETruSitesStore:
            result = TrustedSitesL().CreateSessionL();
            break;
        default:
            User::Leave(KErrNotSupported);
            break;
        }

    return result;
    }


// CShutdown

// ---------------------------------------------------------------------------
// CShutdown::CShutdown()
// ---------------------------------------------------------------------------
//
inline CShutdown::CShutdown() :
  CTimer(-1)
    {
    CActiveScheduler::Add(this);
    }


// ---------------------------------------------------------------------------
// CShutdown::ConstructL()
// ---------------------------------------------------------------------------
//
inline void CShutdown::ConstructL()
    {
    CTimer::ConstructL();
    }


// ---------------------------------------------------------------------------
// CShutdown::Start()
// ---------------------------------------------------------------------------
//
inline void CShutdown::Start()
    {
    After(KServerShutdownDelay);
    }


// ---------------------------------------------------------------------------
// CShutdown::RunL()
// Initiate server exit when the timer expires.
// ---------------------------------------------------------------------------
//
void CShutdown::RunL()
    {
    CActiveScheduler::Stop();
    }


// ======== LOCAL FUNCTIONS ========

// Server startup 

// ---------------------------------------------------------------------------
// RunServerL()
// Perform all server initialisation, in particular creation of the scheduler
// and server and then run the scheduler.
// ---------------------------------------------------------------------------
//
static void RunServerL()
    {
    TRACE_PRINT("RunServerL 1");
    // Naming the server thread after the server helps to debug panics
    User::LeaveIfError(User::RenameThread( KDevTokenServerName ));
    TRACE_PRINT("RunServerL 2");
    // Create and install the active scheduler we need
    CActiveScheduler* s=new(ELeave) CActiveScheduler;
    CleanupStack::PushL(s);
    CActiveScheduler::Install(s);
    TRACE_PRINT("RunServerL 3");
    // Create the server and leave it on the cleanup stack
    CDevTokenServer::NewLC();
    TRACE_PRINT("RunServerL 4");
    // Before starting the server, notify client that initialisation is
    // complete.
    // (note that WINS on EKA1 uses threads since it lacks process emulation)
    RProcess::Rendezvous(KErrNone);
    TRACE_PRINT("RunServerL 5");
    // Ready to run
    CActiveScheduler::Start();

    // Cleanup the server and scheduler
    CleanupStack::PopAndDestroy(2);
    }


// ---------------------------------------------------------------------------
// E32Main()
// Server process entry point.
// ---------------------------------------------------------------------------
//
TInt E32Main()
    {
    #ifdef _DEBUG
    DevTokenServerDebug::HeapCheckStart();
    #endif

    CTrapCleanup* cleanup=CTrapCleanup::New();
    TInt r=KErrNoMemory;
    if (cleanup)
        {
        TRAP(r,RunServerL());
        delete cleanup;
        }

    #ifdef _DEBUG
    DevTokenServerDebug::HeapCheckEnd();
    #endif
    return r;
    }
    
//EOF

