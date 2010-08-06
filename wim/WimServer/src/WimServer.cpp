/*
* Copyright (c) 2003-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Server startup and initialization
*
*/



// INCLUDE FILES
#include    "WimServer.h"
#include    "WimSession.h"
#include    "WimSig.h"
#include    "WimSessionRegistry.h"
#include    "WimTrustSettingsStore.h"
#include    "WimDefs.h"
#include    "WimTrace.h"
#include    "WimUtilityFuncs.h"
#include    "WimCallbackImpl.h"
#include    <s32file.h>         // UserSvr

#include    "WimCertInfo.h"
#include    "Wimi.h"            // WIMI definitions
#include    "WimSatRefreshObserver.h"
#include    "WimBTSapObserver.h"

#include    <e32property.h>     // RProperty
#include    <PSVariables.h>     // Property values

#ifdef WIMSERVER_SHUTDOWN
const TInt KServerShutdownDelay = 0x200000;
#endif
// Initialize static variables. These variables are static because static
// callback functions in CWimCallBack.
CWimServer* CWimServer::iWimServer = NULL;
TInt CWimServer::iWimStatus = KErrNone;
TBool CWimServer::iWimInitialized = EFalse;


// ============================= LOCAL FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// StartServer
// Server startup method. Creates active scheduler for the server.
// Returns: TInt: error code
// -----------------------------------------------------------------------------
//
static void StartServerL()
    {
    _WIMTRACE(_L("WIM | WIMServer | StartServer | Enter"));
    User::LeaveIfError( User::RenameThread( KWIMServerName ) );
    _WIMTRACE(_L("WIM | WIMServer | StartServer | Create AS"));

    CActiveScheduler* s = new(ELeave) CActiveScheduler;
    CleanupStack::PushL( s );

    CActiveScheduler::Install( s );

    _WIMTRACE(_L("WIM | WIMServer | StartServer | Create server"));

    CWimServer::NewL();

    _WIMTRACE(_L("WIM | WIMServer | StartServer | Notify client"));
    // Notify Client that server has starter
    RProcess::Rendezvous( KErrNone );

    _WIMTRACE(_L("WIM | WIMServer | StartServer | Start ActiveScheduler"));
    CActiveScheduler::Start();

    CleanupStack::PopAndDestroy( s );
    }


// -----------------------------------------------------------------------------
// E32Main
// Server entry point
// Returns: TInt: error code
// -----------------------------------------------------------------------------
//
GLDEF_C TInt E32Main()
    {
    __UHEAP_MARK;
    _WIMTRACE(_L("WIM | WIMServer | E32Main | Begin"));

    CTrapCleanup* cleanup=CTrapCleanup::New();
    TInt r=KErrNoMemory;

    if ( cleanup )
        {
        TRAP( r, StartServerL() );
        delete cleanup;
        }
    _WIMTRACE(_L("WIM | WIMServer | E32Main Memory leak checking line"));
    __UHEAP_MARKEND;
    _WIMTRACE(_L("WIM | WIMServer | E32Main Memory checking passed"));
    return r;
    }


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWimServer::CWimServer
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CWimServer::CWimServer( TInt aPriority )
    : CPolicyServer( aPriority, wimPolicy, ESharableSessions )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimServer::CWimServer | Begin"));
    }

// -----------------------------------------------------------------------------
// CWimServer::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CWimServer::ConstructL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimServer::ConstructL | Begin"));
    StartL( KWIMServerName );
    _WIMTRACE(_L("WIM | WIMServer | CWimServer::ConstructL | StartL"));

    iWimServer = this;
    InitializeCallbackFunctions();  // Initialize WIMI callback functions
    iWimSessionRegistry = CWimSessionRegistry::NewL();
    iWimTrustSettingsStore = CWimTrustSettingsStore::NewL();
    iWimTimer = CWimTimer::NewL( this );
#ifdef WIMSERVER_SHUTDOWN
    iShutdown.ConstructL();
    iShutdown.Start();
#endif
     _WIMTRACE(_L("WIM | WIMServer | CWimServer::ConstructL | End"));
    }

// -----------------------------------------------------------------------------
// CWimServer::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CWimServer* CWimServer::NewL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimServer::NewL | Begin"));
    CWimServer* self = new( ELeave ) CWimServer( EPriorityHigh );
    CleanupStack::PushL( self );
    self->ConstructL() ;
    CleanupStack::Pop( self );
    return self;
    }

// Destructor
CWimServer::~CWimServer()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimServer::~CWimServer | Begin"));
    if ( iRefreshObserver )
        {
        delete iRefreshObserver;
        iRefreshObserver = NULL;
        }
    if ( iBTSapObserver )
        {
        delete iBTSapObserver;
        iBTSapObserver = NULL;
        }
    // Call close down request only if WIM is initialized
    if ( iWimInitialized )
        {
        WIMI_CloseDownReq();
        _WIMTRACE(_L("WIM | WIMServer | CWimServer::~CWimServer | WIMI_CloseDownReq"));
        }

    delete iWimTrustSettingsStore;
    delete iWimSessionRegistry;
    delete iWimTimer;
    CWimCallBack::DeletegApdu();
    _WIMTRACE(_L("WIM | WIMServer | CWimServer::~CWimServer | delete iWimSessionRegistry"));
    }

// -----------------------------------------------------------------------------
// CWimServer::WimInitialize
// Initialize WIMI API, fetches WIM data to WIMI.
// -----------------------------------------------------------------------------
//
void CWimServer::WimInitialize( const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimServer::WimInitialize | Begin"));

    // Create observer for bluetooth sap event.
    // This is checked every time we enter here.
    if ( !iBTSapObserver )
        {
        TRAPD( err, iBTSapObserver = CWimBTSapObserver::NewL());
        if ( KErrNone != err )
            {
            // creation failed
            iBTSapObserver = NULL; // to be sure
            _WIMTRACE(_L("WIM|CWimServer::WimInitialize | BT Sap observer NOT created."));
            }
        }
    // If BT SAP observer exists, start it
    if ( iBTSapObserver )
        {
        iBTSapObserver->Start(); //If already started, this doesn't start again
        }

    // Get state of smart card
    TInt simState = CWimUtilityFuncs::SimState();

    // Check whether state of Smart Card is OK and whether BT SAP is connected
    if ( simState != KErrNone ) // Card is not OK
        {
        SetWimInitialized( EFalse, simState );
        iWimStatus = simState;
        aMessage.Complete( iWimStatus );
        }
    else // Smart Card is ready
        {
        // Initialize WIMI if not yet done
        // iWimInitialized is set in callback function InitOKResp()
        if ( !iWimInitialized )
            {
            TInt initStatus = CWimUtilityFuncs::MapWIMError(
                                      WIMI_InitializeReq( &iWimCallBack ) );

            _WIMTRACE2(_L("WIM|WIMServer|CWimServer::WimInitialize|initStatus=%d"), initStatus );

            // In case WIMI_InitializeReq returs error
            // iWimStatus may not be set
            if ( initStatus != KErrNone )
                {
                SetWimInitialized( EFalse, initStatus  );
                iWimStatus = KErrNotReady;
                }
            }
        else // WIMI already initialized
            {
            _WIMTRACE(_L("WIM|WIMServer|CWimServer::WimInitialize|Wim already initialized"));
            iWimStatus = KErrNone;
            }

        // Create observer for SAT refresh event.
        // This is checked every time we enter here. It may have failed before
        // because SAT Server wasn't started.
        if ( !iRefreshObserver )
            {
            TRAPD( err, iRefreshObserver = CWimSatRefreshObserver::NewL());
            if ( KErrNone != err )
                {
                // creation failed
                iRefreshObserver = NULL; // to be sure
                _WIMTRACE(_L("WIM|CWimServer::WimInitialize | SAT Refresh observer NOT created."));
                }
            }

        aMessage.Complete( iWimStatus );
        }
    }

// -----------------------------------------------------------------------------
// CWimServer::SetWimInitialized
// Set iWimInitialized flag
// -----------------------------------------------------------------------------
//
void CWimServer::SetWimInitialized( TBool aInitialized, TInt aWimStatus )
    {
    _WIMTRACE2(_L("WIM|WIMServer|CWimServer::SetWimInitialized|Value=%d"), aInitialized);
    iWimInitialized = aInitialized;

    if ( !aInitialized )
        {
        iWimStatus = aWimStatus;
        }
    }

// -----------------------------------------------------------------------------
// CWimServer::NewSessionL
// Creates a new sharable session.
// -----------------------------------------------------------------------------
//
CSession2* CWimServer::NewSessionL(
    const TVersion& aVersion,
    const RMessage2& /*aMessage*/ ) const
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimServer::NewSessionL | Begin"));
    TVersion v( KWIMServMajorVersionNumber, KWIMServMinorVersionNumber,
        KWIMServBuildVersionNumber );

    if ( !User::QueryVersionSupported( v, aVersion ) )
        {
        User::Leave( KErrNotSupported );
        }

    CWimSession* wimSession;
    wimSession = CWimSession::NewL( ( CWimServer* )this );
    iWimSessionRegistry->AddSessionL( wimSession );

    return wimSession;
    }

// -----------------------------------------------------------------------------
// CWimServer::InitializeCallbackFunctions
// WIMI API callback initialization.
// -----------------------------------------------------------------------------
//
void CWimServer::InitializeCallbackFunctions()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSession::InitializeCallbackFunctions | Begin"));
    _WIMTRACE2(_L("WIM | WIMServer | CWimSession::InitializeCallbackFunctions | iWimCallBack = %d"), iWimCallBack );

    iWimCallBack.InitOkResp = CWimCallBack::InitOkResp;
    iWimCallBack.CloseDownResp = CWimCallBack::CloseDownResp;
    iWimCallBack.SignResp = CWimCallBack::SignResp;
    iWimCallBack.CertificateResp = CWimCallBack::CertificateResp;
    iWimCallBack.CertificateDeleteResp = CWimCallBack::CertificateDeleteResp;
    iWimCallBack.CertificateStoreResp = CWimCallBack::CertificateStoreResp;
    iWimCallBack.VerifyPINResp = CWimCallBack::VerifyPINResp;
    iWimCallBack.ChangePINResp = CWimCallBack::ChangePINResp;
    iWimCallBack.UnblockPINResp = CWimCallBack::UnblockPINResp;
    iWimCallBack.EnablePINResp = CWimCallBack::EnablePINResp;
    iWimCallBack.CardInsertedResp = CWimCallBack::CardInsertedResp;
    iWimCallBack.Open = CWimCallBack::Open;
    iWimCallBack.Close = CWimCallBack::Close;
    iWimCallBack.APDUReq = CWimCallBack::APDUReq;
    iWimCallBack.GetATRReq = CWimCallBack::GetATRReq;
    iWimCallBack.GetReaderListReq = CWimCallBack::GetReaderListReq;
    iWimCallBack.ReadSimFileReq = CWimCallBack::ReadSimFileReq;
    iWimCallBack.ProvSCFileResp = CWimCallBack::OmaProvisioningResp;

     _WIMTRACE(_L("WIM | WIMServer | CWimSession::InitializeCallbackFunctions | END"));
    }

// -----------------------------------------------------------------------------
// CWimServer::PanicServer
// Panics the server.
// -----------------------------------------------------------------------------
//
void CWimServer::PanicServer( TWimServerPanic aPanic )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimServer::PanicServer | Begin"));

    // Set server state to EWimServerNotRunning.
    (void)RProperty::Set( KUidSystemCategory, KWimServerUid.iUid, EWimServerNotRunning );

    User::Panic( KWIMServerName, aPanic );
    }

// -----------------------------------------------------------------------------
// CWimServer::Server
// Return static iServer
// -----------------------------------------------------------------------------
//
CWimServer* CWimServer::Server()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimServer::Server | Begin"));
    return CWimServer::iWimServer;
    }

// -----------------------------------------------------------------------------
// CWimServer::WimSessionRegistry
// Get pointer to WimSessionRegistry
// -----------------------------------------------------------------------------
//
CWimSessionRegistry* CWimServer::WimSessionRegistry()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimServer::WimSessionRegistry | Begin"));
    return iWimSessionRegistry;
    }

// -----------------------------------------------------------------------------
// CWimServer::GetSessions
// Return all sessions of server
// -----------------------------------------------------------------------------
//
void CWimServer::GetSessionsL( RArray<CWimSession*>& aSessions ) const
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimServer::GetSessionsL | Begin"));
    iWimSessionRegistry->GetSessionsL( aSessions );
    }

// -----------------------------------------------------------------------------
// CWimServer::WimTrustSettingsStore
// Return TrustSettingsStore pointer
// -----------------------------------------------------------------------------
//
CWimTrustSettingsStore* CWimServer::WimTrustSettingsStore()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimServer::WimTrustSettingsStore | Begin"));
    return iWimTrustSettingsStore;
    }

// -----------------------------------------------------------------------------
// CWimServer::CustomSecurityCheckL
// Custom policy check for requests of certain WimServer services.Called by
// framework.
// -----------------------------------------------------------------------------
//
    CPolicyServer::TCustomResult
CWimServer::CustomSecurityCheckL( const RMessage2& aMsg,
                                  TInt& /*aAction*/,
                                  TSecurityInfo& /*aMissing*/ )
    {
    _WIMTRACE2(_L("CWimServer::CustomSecurityCheckL | Begin, heap=%d"),
               User::CountAllocCells());

    WIMI_STAT callStatus;
    TUint8 usage( 255 ); // init stg unused
    // Set initial return value to failed
    CPolicyServer::TCustomResult test( EFail );
    TInt function = aMsg.Function();

    _WIMTRACE4(_L("CWimServer::CustomSecurityCheckL: SID=%d,VID=%d, Function=%d"),
               aMsg.SecureId().iId, aMsg.VendorId().iId, function);

    switch ( function )
        {
        case EGetWIMCertDetails:
            {
            //Leave the capability checking to Certificate Handler object
            test = EPass;
            break;
            }

        case EGetCertExtras:
            {
            // Int2 contains usage of required cert
            TWimEntryType certEntryType = ( TWimEntryType ) aMsg.Int2();

            switch ( certEntryType )
                {
                case EWimEntryTypeCA:
                    {
                    // CA cert reading does not require caps
                    _WIMTRACE(_L("CWimServer::CustomSecurityCheckL: CA cert read req, OK"));
                    test = EPass;
                    break;
                    }
                case EWimEntryTypePersonal: //lint -fallthrough
                case EWimEntryTypeAll:
                    {
                    // User cert reading requires caps
                    test = CheckReadCapsForUsage( aMsg, WIMI_CU_Client );
                    break;
                    }
                default:
                    {
                    test = EFail;
                    break;
                    }
                }
            break;
            }

        case EStoreCertificate:
            {
            ResolveStoreCertReqL( aMsg, usage );
            test = CheckWriteCapsForUsage( aMsg, usage );
            break;
            }

        case ERemoveCertificate:
            {
            callStatus = ResolveRemoveCertReqL( aMsg, usage );
            if ( WIMI_Ok == callStatus )
                {
                test = CheckWriteCapsForUsage( aMsg, usage );
                }
            break;
            }

        case ESignTextReq:
            {
            test = CheckReadCapsForUsage( aMsg, WIMI_CU_Client );
            break;
            }

        case ESetApplicability:         //lint -fallthrough
        case ESetTrust:                 //lint -fallthrough
        case ERemoveTrustSettings:
            {
            if ( ResolveTrustModficationReqL( aMsg, usage ))
                {
                test = CheckWriteCapsForUsage( aMsg, usage );
                }
            break;
            }

        case ESetDefaultTrustSettings:
            {
            _WIMTRACE( _L("CWimServer::CustomSecurityCheckL, ESetDefaultTrustSettings exception") );
            if ( ResolveTrustModficationReqL( aMsg, usage ))
                {
                test = CheckReadCapsForUsage( aMsg, usage );
                }
            }
            break;
            
        case EExportPublicKey:
            {
            ResolveExportPublicReqL( aMsg, usage );
            test = CheckReadCapsForUsage( aMsg, usage );
            break;
            }

        default:
            {
            _WIMTRACE2(_L("CWimServer::CustomSecurityCheckL: UNKNOWN FUNCTION %d"),
                        function);
            // Inconsistent policy, should not be here. Panic.
            PanicServer( EWimIncorrectPolicy );
            break;
            }
        } // switch ( function )

    _WIMTRACE2(_L("CWimServer::CustomSecurityCheckL: Result=%d"), test);
    _WIMTRACE2(_L("CWimServer::CustomSecurityCheckL | End, heap=%d"),
               User::CountAllocCells());

    return test;
    }


// -----------------------------------------------------------------------------
// CWimServer::ResolveCertUsage
// Resolves usage (CA/User) for a certificate.
// -----------------------------------------------------------------------------
//
WIMI_STAT CWimServer::ResolveCertUsage( const RMessage2& aMsg,
                                        TUint8& aUsage )
    {
    _WIMTRACE(_L("CWimServer::ResolveCertUsage | Begin"));

    // aMsg.Ptr0 contains reference to certificate
    WIMI_Ref_pt pCertRef = const_cast< WIMI_Ref_pt >( aMsg.Ptr0() );

    WIMI_STAT callStatus = GetCertificateInfo( pCertRef, aUsage );

    _WIMTRACE(_L("CWimServer::ResolveCertUsage | End"));
    return callStatus;
    }


// -----------------------------------------------------------------------------
// CWimServer::GetCertificateInfo
// Fetches certificate info. Wrapper for WIMI call.
// -----------------------------------------------------------------------------
//
WIMI_STAT CWimServer::GetCertificateInfo( WIMI_Ref_pt aCertRef,
                                          TUint8& aUsage )
    {
    _WIMTRACE(_L("CWimServer::GetCertificateInfo | Begin"));

    WIMI_Ref_pt     pWimRef = NULL;
    WIMI_BinData_t  ptLabel;
    WIMI_BinData_t  ptKeyID;
    WIMI_BinData_t  ptCAID;
    WIMI_BinData_t  ptIssuerHash;
    WIMI_BinData_t  ptTrustedUsage;
    TUint8          uiCDFRefs;
    TUint8          type;
    TUint16         certLen;
    TUint8          modifiable = 0;

    WIMI_STAT callStatus = WIMI_GetCertificateInfo(
                                aCertRef,
                                &pWimRef,
                                &ptLabel,
                                &ptKeyID, // Key Id (hash)
                                &ptCAID,
                                &ptIssuerHash,
                                &ptTrustedUsage,
                                &uiCDFRefs,
                                &aUsage,  // 0 = client, 1 = CA
                                &type,    // WTLSCert(1),
                                          // X509Cert(2),
                                          // X968Cert(3),
                                          // CertURL(4)
                                &certLen, // cert. content or URL length
                                &modifiable );

    if ( callStatus == WIMI_Ok )
        {
        free_WIMI_Ref_t( pWimRef );
        WSL_OS_Free( ptLabel.pb_buf );
        WSL_OS_Free( ptKeyID.pb_buf );
        WSL_OS_Free( ptCAID.pb_buf );
        WSL_OS_Free( ptIssuerHash.pb_buf );
        WSL_OS_Free( ptTrustedUsage.pb_buf );
        }

    _WIMTRACE(_L("CWimServer::GetCertificateInfo | End"));
    return callStatus;
    }


// -----------------------------------------------------------------------------
// CWimServer::ResolveStoreCertReqL
// Resolves which kind of certificate (CA/User) client is accessing.
// -----------------------------------------------------------------------------
//
void CWimServer::ResolveStoreCertReqL( const RMessage2& aMsg,
                                       TUint8& aUsage )
    {
    _WIMTRACE(_L("CWimServer::ResolveStoreCertReqL | Begin"));
    TPckgBuf<TWimCertAddParameters> certInfoPckg;
    aMsg.ReadL( 1, certInfoPckg );

    // Convert from TWimCertType to TUint8
    if ( certInfoPckg().iUsage == EWimCertTypeCA )
        {
        aUsage = WIMI_CU_CA;     // 1
        _WIMTRACE(_L("CWimServer::ResolveStoreCertReqL | Usage=CA"));
        }
    else
        {
        aUsage = WIMI_CU_Client; // 0
        _WIMTRACE(_L("CWimServer::ResolveStoreCertReqL | Usage=Client"));
        }
    _WIMTRACE(_L("CWimServer::ResolveStoreCertReqL | End"));
    }


// -----------------------------------------------------------------------------
// CWimServer::ResolveRemoveCertReqL
// Resolves which kind of certificate client is accessing.
// -----------------------------------------------------------------------------
//
WIMI_STAT CWimServer::ResolveRemoveCertReqL( const RMessage2& aMsg,
                                             TUint8& aUsage )
    {
    _WIMTRACE2(_L("CWimServer::ResolveRemoveCertReqL | Begin, heap=%d"),
               User::CountAllocCells());

    WIMI_STAT callStatus = WIMI_Ok;
    TWimCertRemoveAddr dataStruct = {0};

    TPckgBuf<TWimCertRemoveAddr> wimCertRemoveAddrPckg( dataStruct );
    aMsg.ReadL( 0, wimCertRemoveAddrPckg );
    WIMI_Ref_pt pCertRef = ( void* )wimCertRemoveAddrPckg().iCertAddr;

    callStatus = GetCertificateInfo( pCertRef, aUsage );

    _WIMTRACE2(_L("CWimServer::ResolveRemoveCertReqL | End, heap=%d"),
               User::CountAllocCells());

    return callStatus;
    }


// -----------------------------------------------------------------------------
// CWimServer::ResolveTrustModficationReqL
// Resolves which kind of certificate client is accessing.
// -----------------------------------------------------------------------------
//
TBool CWimServer::ResolveTrustModficationReqL( const RMessage2& aMsg,
                                               TUint8& aUsage )
    {
    _WIMTRACE(_L("CWimServer::ResolveTrustModficationReqL | Begin"));
    TBool rc( EFalse );
    TPckgBuf<TWimCertInfoPckg> pckg;
    aMsg.ReadL( 0, pckg );

    TInt owner = pckg().iCertificateOwnerType;

    switch ( owner )
        {
        case ECACertificate:
            {
            aUsage = WIMI_CU_CA;
            rc = ETrue;
            _WIMTRACE(_L("CWimServer::ResolveTrustModficationReqL | Usage=CA"));
            break;
            }
        case EUserCertificate:     //lint -fallthrough
        case EPeerCertificate:
            {
            aUsage = WIMI_CU_Client;
            rc = ETrue;
            _WIMTRACE(_L("CWimServer::ResolveTrustModficationReqL | Usage=Client"));
            break;
            }
        default:
            {
            rc = EFalse;
            _WIMTRACE(_L("CWimServer::ResolveTrustModficationReqL | FAIL"));
            break;
            }
        }
    return rc;
    }


// -----------------------------------------------------------------------------
// CWimServer::ResolveExportPublicReqL
// Resolves which kind of key (CA/User) client requests.
// -----------------------------------------------------------------------------
//
WIMI_STAT CWimServer::ResolveExportPublicReqL( const RMessage2& aMsg,
                                               TUint8& aUsage )
    {
    _WIMTRACE2(_L("CWimServer::ResolveExportPublicReqL | Begin, heap=%d"),
               User::CountAllocCells());

    WIMI_STAT callStatus = WIMI_Ok;

    TPckgBuf<TExportPublicKey> keyExportPckg;
    aMsg.ReadL( 0, keyExportPckg );

    TBuf8<KKeyIdLen> keyIdBuf = keyExportPckg().iKeyId;
    TUint8 clicertCount = 0;
    WIMI_RefList_t clicertRefList = NULL;
    const TUint16 KInvalidUsage = 10;
    TUint16 usage = KInvalidUsage;

    callStatus = WIMI_GetCertificateListByKeyHash( ( TUint8* ) keyIdBuf.Ptr(),
                                            WIMI_CU_Client,
                                            &clicertCount,
                                            &clicertRefList );
    if( callStatus != WIMI_Ok )
        {
    	return callStatus;
        }
    free_WIMI_RefList_t( clicertRefList );

    if ( clicertCount > 0 )
        {
    	usage = WIMI_CU_Client;
        }
    else
        {
    	TUint8 cacertCount = 0;
        WIMI_RefList_t cacertRefList = NULL;

        callStatus = WIMI_GetCertificateListByKeyHash( ( TUint8* ) keyIdBuf.Ptr(),
                                            WIMI_CU_CA,
                                            &cacertCount,
                                            &cacertRefList );
        if ( callStatus != WIMI_Ok )
            {
            return callStatus;
            }
        free_WIMI_RefList_t( cacertRefList );

        if ( cacertCount > 0 )
            {
    	    usage = WIMI_CU_CA;
            }
        else
            {
        	aMsg.Complete( KErrBadHandle );
            }
        }

    aUsage = static_cast< TUint8 >( usage );

    _WIMTRACE2(_L("CWimServer::ResolveExportPublicReqL | End, heap=%d"),
               User::CountAllocCells());

    return callStatus;
    }


// -----------------------------------------------------------------------------
// CWimServer::CheckReadCapsForUsage
// Check if client has sufficient read capability for service in question.
// -----------------------------------------------------------------------------
//
    CPolicyServer::TCustomResult
CWimServer::CheckReadCapsForUsage( const RMessage2& aMsg,
                                   TUint8 aUsage )
    {
    _WIMTRACE2(_L("CWimServer::CheckReadCapsForUsage | Begin, usage=%d"), aUsage );

    CPolicyServer::TCustomResult rc( EFail );

    switch ( aUsage )
        {
        case WIMI_CU_CA:
            {
            // CA certificate reading doesn't require any capability.
            _WIMTRACE(_L("CWimServer::CheckReadCapsForUsage: CA cert read req, OK."));
            rc = EPass;
            break;
            }
        case WIMI_CU_Client:
            {
            // User certificate reading requires ReadUserData capability.
            if ( aMsg.HasCapability( ECapabilityReadUserData ))
                {
                rc = EPass;
                _WIMTRACE(_L("CWimServer::CheckReadCapsForUsage: User cert read capa PASS"));
                }
            else
                {
                rc = EFail;
                _WIMTRACE(_L("CWimServer::CheckReadCapsForUsage: User cert read capa FAIL"));
                }
            break;
            }
        default:
            {
            _WIMTRACE(_L("CWimServer::CheckReadCapsAccordingToUsage: FAIL:Unknown usage."));
            rc = EFail;
            break;
            }
        }
    _WIMTRACE(_L("CWimServer::CheckReadCapsForUsage | End"));
    return rc;
    }


// -----------------------------------------------------------------------------
// CWimServer::CheckWriteCapsForUsage
// Check if client has sufficient write capability for service in question.
// -----------------------------------------------------------------------------
//
    CPolicyServer::TCustomResult
CWimServer::CheckWriteCapsForUsage( const RMessage2& aMsg,
                                    TUint8 aUsage )
    {
    _WIMTRACE2(_L("CWimServer::CheckWriteCapsForUsage | Begin, usage=%d"), aUsage);

    CPolicyServer::TCustomResult rc( EFail );

    switch ( aUsage )
        {
        case WIMI_CU_CA:
            {
            // CA certificate writing/modifying requires WriteDeviceData
            // capability.
            if ( aMsg.HasCapability( ECapabilityWriteDeviceData ))
                {
                rc = EPass;
                _WIMTRACE(_L("CWimServer::CheckWriteCapsForUsage: CA cert write capa PASS"));
                }
            else
                {
                rc = EFail;
                _WIMTRACE(_L("CWimServer::CheckWriteCapsForUsage: CA cert write capa FAIL"));
                }
            break;
            }
        case WIMI_CU_Client:
            {
            // User certificate writing/modifying requires WriteUserData
            // capability.
            if ( aMsg.HasCapability( ECapabilityWriteUserData ))
                {
                rc = EPass;
                _WIMTRACE(_L("CWimServer::CheckWriteCapsForUsage: User cert write capa PASS"));
                }
            else
                {
                rc = EFail;
                _WIMTRACE(_L("CWimServer::CheckWriteCapsForUsage: User cert write capa FAIL"));
                }
            break;
            }
        default:
            {
            _WIMTRACE(_L("CWimServer::CheckWriteCapsForUsage: FAIL:Unknown usage."));
            rc = EFail; // Unknown cert usage -> fail to be sure
            break;
            }
        }
    _WIMTRACE(_L("CWimServer::CheckWriteCapsForUsage | End"));
    return rc;
    }


// -----------------------------------------------------------------------------
// CWimServer::IsAccessingToken
// Tells if currently requested service is accessing physical token.
// -----------------------------------------------------------------------------
//
TBool CWimServer::IsAccessingToken()
    {
    return iIsAccessingToken;
    }

// -----------------------------------------------------------------------------
// CWimServer::SetIsAccessingToken
// Tell that currently requested service is accessing physical token.
// -----------------------------------------------------------------------------
//
void CWimServer::SetIsAccessingToken( TBool aValue )
    {
    iIsAccessingToken = aValue;
    }

// -----------------------------------------------------------------------------
// CWimServer::RefreshNotificationReceived
// Informs server about completed SIM Refresh operation. Returns true if SIM
// Refresh notification is received in the middle of service request that
// accesses physical token. See IsAccessingToken().
// Server shall re-initialize itself at earliest convenience.
// -----------------------------------------------------------------------------
//
TBool CWimServer::RefreshNotificationReceived()
    {
    return iRefreshNotificationReceived;
    }

// -----------------------------------------------------------------------------
// CWimServer::SetRefreshNotificationReceived
// Tell server about completed SIM Refresh operation. Is set if SIM
// Refresh notification is received in the middle of service request
// that accesses physical token. See IsAccessingToken().
// Server shall re-initialize itself at earliest convenience.
// -----------------------------------------------------------------------------
//
void CWimServer::SetRefreshNotificationReceived( TBool aValue )
    {
    iRefreshNotificationReceived = aValue;
    }

// -----------------------------------------------------------------------------
// CWimServer::TimerExpired()
// Interface derived from MWimTimerListener
// -----------------------------------------------------------------------------
//
void CWimServer::TimerExpired()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimServer::TimerExpired | Begin"));
    WIMI_Ref_pt pWimRefTemp = NULL;

    pWimRefTemp = WIMI_GetWIMRef( 0 );

    if ( pWimRefTemp )  // Close the WIM
        {
        WIMI_CloseWIM( pWimRefTemp );
        free_WIMI_Ref_t( pWimRefTemp );
        }
    }

// -----------------------------------------------------------------------------
// CWimServer::WimTimer()
// Return the pointer of Timer
// -----------------------------------------------------------------------------
//
CWimTimer* CWimServer::WimTimer()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimServer::WimTimer | Begin"));
    return iWimTimer;
    }

// -----------------------------------------------------------------------------
// CWimServer::CancelWimInitialize()
// Cancel Wim Initialize
// -----------------------------------------------------------------------------
//
void CWimServer::CancelWimInitialize( const RMessage2& aMessage )
    {
	 if ( !iWimInitialized )
		 {
		 CWimCallBack::CancelWimInitialize();
		 }
	aMessage.Complete( KErrNone );
    }


#ifdef WIMSERVER_SHUTDOWN

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
     _WIMTRACE(_L("WIM | WIMServer |  CShutdown::RunL"));
    CActiveScheduler::Stop();
    _WIMTRACE(_L("WIM | WIMServer |  CShutdown::End"));
    }

// ---------------------------------------------------------------------------
// CWimServer::DropSession()
// ---------------------------------------------------------------------------
//
void CWimServer::AddSession()
    {
    ++iSessionCount;
    iShutdown.Cancel();
    }


// ---------------------------------------------------------------------------
// CWimServer::DropSession()
// ---------------------------------------------------------------------------
//
void CWimServer::DropSession()
    {
    if (--iSessionCount==0)
        {
        iShutdown.Start();
        }
    }

#endif //WIMSERVER_SHUTDOWN
