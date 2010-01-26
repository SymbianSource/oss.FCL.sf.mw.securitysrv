/*
* Copyright (c) 2003 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Smart card server
*
*/



// INCLUDE FILES
#include    "ScardBase.h"
#include    "ScardClsv.h"
#include    "ScardServerBase.h"
#include    "ScardServer.h"
#include    "WimTrace.h"

// MODULE DATA STRUCTURES
struct TThreadParameter
    {
    RSemaphore  iSemaphore;
    TInt        iPanicCode;
    };

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CScardServer::CScardServer
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CScardServer::CScardServer()
    : CServer2( EPriorityStandard )
    {
    _WIMTRACE(_L("WIM|Scard|CScardServer::CScardServer|Begin"));
    }

// -----------------------------------------------------------------------------
// CScardServer::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CScardServer::ConstructL()
    {
    _WIMTRACE(_L("WIM|Scard|CScardServer::ConstructL|Begin"));
    iNotifyRegistry = CScardNotifyRegistry::NewL( this );
    iControlRegistry = CScardAccessControlRegistry::NewL( this );
    iReaderFactoryRegistry = CScardReaderRegistry::NewL( this );
    iResourceRegistry = CScardResourceRegistry::NewL( this );
    iConnectionRegistry = CScardConnectionRegistry::NewL( this );
    }

// -----------------------------------------------------------------------------
// CScardServer::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CScardServer* CScardServer::NewL()
    {
    _WIMTRACE(_L("WIM|Scard|CScardServer::NewL|Begin"));
    CScardServer* self = new( ELeave ) CScardServer;
    
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    TPtrC tempName;
    tempName.Set( KScardServerName );
    HBufC* pName = tempName.Alloc();
    //Again these panics EPR
    __ASSERT_ALWAYS( pName,
        PanicServer( KScServerPanicSvrCreateServer ) );
    
    self->iName = pName; // Ownership moves

    // Start the server
    TPtr name = self->iName->Des();     
    TInt r = self->Start( name );
    __ASSERT_ALWAYS( r == KErrNone,
        PanicServer( KScServerPanicSvrCreateServer ) );

    return self;
    }

    
// Destructor
CScardServer::~CScardServer()
    {
    _WIMTRACE(_L("WIM|Scard|CScardServer::~CScardServer|Begin"));
    delete iNotifyRegistry;
    delete iControlRegistry;
    delete iReaderFactoryRegistry;
    delete iResourceRegistry;
    delete iConnectionRegistry;
    }


// -----------------------------------------------------------------------------
// CScardServer::PanicServer
// Panics the server
// -----------------------------------------------------------------------------
//
void CScardServer::PanicServer( const TInt aPanic )
    {
    _WIMTRACE(_L("WIM|Scard|CScardServer::PanicServer|Begin"));
    User::Panic( _L( "ScardServer" ), aPanic );
    }

// -----------------------------------------------------------------------------
// CScardServer::NewSessionL
// Create a new client session for this server.
// -----------------------------------------------------------------------------
//
CSession2* CScardServer::NewSessionL(
    const TVersion& aVersion, const RMessage2& /*aMessage*/ ) const
    {
    _WIMTRACE(_L("WIM|Scard|CScardServer::NewSessionL|Begin"));
    // check version is ok
    TVersion v( KScardServMajorVersionNumber, KScardServMinorVersionNumber, 
        KScardServBuildVersionNumber );
    if ( !User::QueryVersionSupported( v, aVersion ) )
        {
        User::Leave( KErrNotSupported );
        }

    // make new session
    return CScardSession::NewL( const_cast< CScardServer* >( this ) );
    }

// -----------------------------------------------------------------------------
// CScardServer::ThreadFunction
// Start active scheduler and do other new thread things
// -----------------------------------------------------------------------------
//
GLDEF_C TInt CScardServer::ThreadFunction( TAny* aStarted )
    {
    _WIMTRACE(_L("WIM|Scard|CScardServer::ThreadFunction|Begin"));
    //  convert argument to initial data
    TThreadParameter& data = *(static_cast< TThreadParameter* >( aStarted ) );
    RSemaphore& started = data.iSemaphore;

    data.iPanicCode = KErrNone;
    
    // create cleanup stack
    if ( !CTrapCleanup::New() )
        {
        data.iPanicCode = KScServerPanicCreateTrapCleanup;
        started.Signal();
        PanicServer( KScServerPanicCreateTrapCleanup );
        }

    // construct and install active scheduler
    if ( !CActiveScheduler::Current() ) //Check if scheduler already started
        {
        CActiveScheduler* scheduler = new CActiveScheduler;
        if ( !scheduler )
            {
            data.iPanicCode = KScServerPanicMainSchedulerError;
            started.Signal();
            PanicServer( KScServerPanicMainSchedulerError );
            }
        CActiveScheduler::Install( scheduler );
        }
    // construct server, an active object
    TRAPD( err, CScardServer::NewL() );

    if ( err )
        {
        data.iPanicCode = KScServerPanicSvrCreateServer;
        started.Signal();
        PanicServer( KScServerPanicSvrCreateServer );
        }

    // signal everything has started
    started.Signal(); // now started ok

    // start handling requests
    CActiveScheduler::Start();
    return KErrNone;
    }

// ========================== OTHER EXPORTED FUNCTIONS =========================

// -----------------------------------------------------------------------------
// StartThread Starts the server thread.
// Returns: TInt:   error code
// -----------------------------------------------------------------------------
//
EXPORT_C TInt StartThread()
    {
    _WIMTRACE(_L("WIM|Scard|CScardServer::StartThread|Begin"));
    TInt res = KErrNone;

    // check server not already started
    TFindServer findSCardServer( KScardServerName );
    TFullName name;
    if ( findSCardServer.Next( name ) != KErrNone )
        {
        // create server thread
        RThread thread;
        TThreadParameter threadData;
        threadData.iSemaphore.CreateLocal( 0 );

        res = thread.Create(
            KScardServerName, // name of thread
            CScardServer::ThreadFunction, // thread function
            KDefaultStackSize,
            KDefaultMinHeapSize,
            KDefaultMaxHeapSize,
            &threadData // parameter to thread function
            );

        //  If creation was successful, try starting the server
        if ( res == KErrNone )
            {
            // now start thread
            thread.SetPriority( EPriorityNormal ); // set its priority
            thread.Resume(); // kick it into life
            threadData.iSemaphore.Wait(); // wait until it's got going

            if ( threadData.iPanicCode )   
                {
                //  The server did not start properly
                thread.Kill( threadData.iPanicCode );
                res = threadData.iPanicCode; // set the response
                return res;
                }

            // tidy up
            thread.Close(); // no longer interested in that thread
            }
        else
            {
            thread.Close();         
            }

        threadData.iSemaphore.Close(); // or semaphore
        }

    // all well
    return res;
    }

//  End of File  
