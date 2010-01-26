/*
* Copyright (c) 2003-2009 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Server accepts requests from client threads and forwards them
*               to the relevant server-side client session. It also handles the
*               creation of server-side client sessions as a result of requests
*               for connection from client threads.
*
*/



#ifndef CWIMSERVER_H
#define CWIMSERVER_H

//  INCLUDES
#include "Wimi.h"       // WIMI definitions
#include "WimClsv.h"
#include "WimTimer.h"

// CONSTANTS
_LIT( KWimServer, "CWimServer" );

/*
* ==============================================================================
    WimServer's policy
* ==============================================================================
*/
// Total number of policy ranges
const TUint KWimServerRangeCount = 16;

/**
* Definition of the ranges of IPC numbers in Wim server.
* NOTE: Adding new functions to server will mess up policy checking.
* @see TWimServRqst
*/
const TInt wimRanges[KWimServerRangeCount] =
    {
    ENon,             // Range 0;  'ENon', Opcode has not been assigned yet.
    EWimInitialize,   // Range 1, functions 1-5; WIM mgmt functions EAlwaysPass
    ECloseWIMAfter,   // Range 2, function 6;    Set closing time for WIM
    EGetCloseWIMAfter,// Range 3, functions 7-8; Get WIM closing time info
    EWIMClose,        // Range 4, function 9;    Closes the WIM
    ENotifyOnRemoval, // Range 5, funcs 10-14; Token removal & free mem (internal)
    EGetPINCount,     // Range 6, functions 15-18; PIN info functions
    EChangePINReq,    // Range 7, functions 19-21; PIN & Key mgmt, PIN modif.
    EVerifyPinReq,    // Range 8, functions 22-25; PIN & Key mgmt, Pin&Key read
    EGetWIMCertLst,   // Range 9, functions 26-27; Cert info
    EGetWIMCertDetails,// Range 10, funcs 28-33; Cert read & manip, PK export, sign text
    EGetOMAFileSize,   // Range 11, functions 34-35; OMA provisioning
    EGetTrustSettings, // Range 12, functions 36;    Get trust settings
    ESetApplicability, // Range 13, function 37-40;  Modify trust settings
    ECancelTrustSettings,  // Range 14, function 41; Cancel any trust operation
    ECancelTrustSettings+1 // Range 15, 42-; N/A function/end-of-range check
    };                                    // -> ENotSupported

// Policy to implement for each of the ranges in range table.
const TUint8 wimElementsIndex[KWimServerRangeCount] =
    {
    CPolicyServer::ENotSupported, // applies to 0th range
    CPolicyServer::EAlwaysPass,   // applies to 1st range
    3,                            // Policy "3" applies to 2nd range
    2,                            // Policy "2" applies to 3rd range
    3,                            // Policy "3" applies to 4th range
    CPolicyServer::EAlwaysPass,   // applies to 5th range
    2,                            // Policy "2" applies to 6th range
    3,                            // Policy "3" applies to 7th range
    2,                            // Policy "2" applies to 8th range
    CPolicyServer::EAlwaysPass,   // applies to 9th range
    CPolicyServer::ECustomCheck,  // applies to 10th range
    2,                            // Policy "2" applies to 11th range
    CPolicyServer::EAlwaysPass,   // Applies to 12th range (get trust)
    CPolicyServer::ECustomCheck,  // Applies to 13th range (modify trust)
    CPolicyServer::EAlwaysPass,   // Applies to 14th range (cancel trust)
    CPolicyServer::ENotSupported  // applies to 15th range (out of range IPC)
    };

// Specific capability checks.
const CPolicyServer::TPolicyElement wimElements[] =
    {
        // policy "0"; fail call if ReadUserData not present
        {
        _INIT_SECURITY_POLICY_C1( ECapabilityReadUserData ),
        CPolicyServer::EFailClient
        },
        // policy "1"; fail call if WriteUserData not present
        {
        _INIT_SECURITY_POLICY_C1( ECapabilityWriteUserData ),
        CPolicyServer::EFailClient
        },
        // policy "2"; fail call if ReadDeviceData not present
        {
        _INIT_SECURITY_POLICY_C1( ECapabilityReadDeviceData ),
        CPolicyServer::EFailClient
        },
        // policy "3"; fail call if WriteDeviceData not present
        {
        _INIT_SECURITY_POLICY_C1( ECapabilityWriteDeviceData ),
        CPolicyServer::EFailClient
        }
    };

// Packs all the security definitions into a policy.
const CPolicyServer::TPolicy wimPolicy =
    {
    CPolicyServer::EAlwaysPass, // all connect attempts should pass
    KWimServerRangeCount,       // number of ranges
    wimRanges,                  // ranges array
    wimElementsIndex,           // elements<->ranges index
    wimElements,                // array of elements
    };

// FORWARD DECLARATIONS
class CWimSession;
class CWimMemMgmt;
class CWimSessionRegistry;
class CWimTrustSettingsStore;
class CWimSatRefreshObserver;
class CWimBTSapObserver;

// CLASS DECLARATION

#ifdef WIMSERVER_SHUTDOWN
/**
 * Implements shutdown of the server.  When the last client disconnects, this
 * class is activated, and when the timer expires, causes the server to
 * close.
 *
 *  @lib
 *  @since S60 
 */
class CShutdown : public CTimer
    {  
    public:
    
        inline CShutdown();
        
        inline void ConstructL();
        
        inline void Start();
        
    private:
        
        void RunL();
    };

#endif //WIMSERVER_SHUTDOWN


/**
*  Main class of the server.
*
*  @since Series60 2.1
*/

class CWimServer : public CPolicyServer, public MWimTimerListener   // for policy checking
    {

    public:  // Constructor and destructor

        /**
        * Two-phased constructor.
        */
        static CWimServer* NewL();

        /**
        * Destructor.
        */
        virtual ~CWimServer();


    public: // New functions

        /**
        * Creates a new session.
        * @param aVersion Version of the server.
        * @return Pointer to sharable session.
        */
        virtual CSession2* NewSessionL( const TVersion &aVersion,
                                        const RMessage2& aMessage ) const;

        /**
        * Panics the server
        * @param aPanic Server panic reasons
        * @return void
        */
        static void PanicServer( TWimServerPanic aPanic );

        /**
        * Get all sessions of the server
        * @param aSessions Array of sessions. Empty array constructed in
        *        a calling function
        * @return void
        */
        void GetSessionsL( RArray<CWimSession*>& aSessions ) const;

        /**
        * Get pointer to WimSessionRegistry
        * @return pointer to WimSessionRegistry
        */
        CWimSessionRegistry* WimSessionRegistry();

        /**
        * Get pointer to WimServer object
        * @return Pointer to WimServer object
        */
        static CWimServer* Server();

        /**
        * Initializes WIMI if not yet initialized.
        * In initalization WIM data from card is fetched to WIMI.
        * @return void
        */
        void WimInitialize( const RMessage2& aMessage );
        
        /**
        * Cancel Initializes WIMI if not yet initialized.
        * Stop the apdu sending
        * @return void
        */
        void CancelWimInitialize( const RMessage2& aMessage );

        /**
        * Set WimInitialized flag
        * @param aInitialized ETrue/EFalse whether WIM is initialized
        * @return void
        */
        static void SetWimInitialized( TBool aInitialized, TInt aStatus  );

        /**
        * Get TrustSettingsStore pointer
        * @param void
        * @return pointer to TrustSettingsStore
        */

        CWimTrustSettingsStore* WimTrustSettingsStore();

        /**
        * Tells if currently requested service is accessing physical token.
        * @return  TBool  Is current service request accessing token or not.
        */
        TBool IsAccessingToken();

        /**
        * Tell that currently requested service is accessing physical token.
        * @param  TBool  Is current service request accessing token.
        */
        void SetIsAccessingToken( TBool aValue );

        /**
        * Tells if there's been completed SIM Refresh operation.
        * Returns ETrue if SIM Refresh notification is received in the middle
        * of service request that accesses physical token.
        * Server shall re-initialize itself at earliest convenience.
        * @see IsAccessingToken()
        * @return  TBool  Event from SAT received during service.
        */
        TBool RefreshNotificationReceived();

        /**
        * Tell about completed SIM Refresh operation.
        * Set ETrue if SIM Refresh notification is received in the middle of
        * service request that accesses physical token.
        * Server shall re-initialize itself at earliest convenience.
        * @see IsAccessingToken()
        * @param  TBool  Refresh event received from SAT.
        */
        void SetRefreshNotificationReceived( TBool aValue );
        
        /**
        * Return the pointer of Timer
        */
        CWimTimer* WimTimer();
        
        /**
        * Interface derived from MWimTimerListener
        */
        void TimerExpired();
        
#ifdef WIMSERVER_SHUTDOWN
        void AddSession();

        void DropSession();
#endif //WIMSERVER_SHUTDOWN

    private:

        /**
        * C++ default constructor.
        */
        CWimServer();

        /**
        * Constructor
        * @param aPriority The priority of this active object.
        * @return void
        */
        CWimServer( TInt aPriority );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        /**
        * Initializes callback functions.
        */
        void InitializeCallbackFunctions();

        /**
        * From CPolicyServer. Checks client's capabilities.
        * @param aMsg       Message from client.
        * @param aAction    IN/OUT; What to do in case of failure.
        * @param aMissing   IN/OUT; A list of missing security attributes.
        * @return           EPass if checking was passed, EFail otherwise.
        */
            CPolicyServer::TCustomResult
        CustomSecurityCheckL( const RMessage2& aMsg,
                              TInt& aAction,
                              TSecurityInfo& aMissing );

        /**
        * Resolves usage (CA/User) for a certificate.
        * @param aMsg       Message from client
        * @param aUsage     Certificate usage (CA/User).
        * @return           WIMI_Ok if ok, error code otherwise.
        */
        WIMI_STAT ResolveCertUsage( const RMessage2& aMsg,
                                    TUint8& aUsage );

        /**
        * Fetches certificate info. Wrapper for WIMI call.
        * @param aMsg       Message from client
        * @param aUsage     Certificate usage (CA/User).
        * @return           WIMI_Ok if ok, error code otherwise.
        */
        WIMI_STAT GetCertificateInfo( WIMI_Ref_pt aCertRef,
                                      TUint8& aUsage );

        /**
        * Resolves which kind of certificate (CA/User) client is accessing.
        * @param aMsg       Message from client
        * @param aUsage     Certificate usage (CA/User).
        * @return void
        */
        void ResolveStoreCertReqL( const RMessage2& aMsg,
                                   TUint8& aUsage );

        /**
        * Resolves which kind of certificate client is accessing.
        * @param aMsg       Message from client
        * @param aUsage     Certificate usage (CA/User).
        * @return           WIMI_Ok if ok, error code otherwise.
        */
        WIMI_STAT ResolveRemoveCertReqL( const RMessage2& aMsg,
                                         TUint8& aUsage );

        /**
        * Resolves which kind of certificate client is accessing.
        * @param aMsg       Message from client
        * @param aUsage     Certificate usage (CA/User).
        * @return ETrue if successful, EFalse otherwise.
        */
        TBool ResolveTrustModficationReqL( const RMessage2& aMsg,
                                           TUint8& aUsage );

        /**
        * Resolves which kind of key (CA/User) client requests.
        * @param aMsg       Message from client
        * @param aUsage     Certificate usage (CA/User).
        * @return           WIMI_Ok if ok, error code otherwise.
        */
        WIMI_STAT ResolveExportPublicReqL( const RMessage2& aMsg,
                                           TUint8& aUsage );

        /**
        * Check if client has sufficient read capability for service in question.
        * @param aMsg       Message from client
        * @param aUsage     Certificate usage (CA/User).
        * @return           EPass if checking was passed, EFail otherwise
        */
            CPolicyServer::TCustomResult
        CheckReadCapsForUsage( const RMessage2& aMsg,
                               TUint8 aUsage );

        /**
        * Check if client has sufficient write capability for service in question.
        * @param aMsg       Message from client
        * @param aUsage     Certificate usage (CA/User).
        * @return           EPass if checking was passed, EFail otherwise
        */
            CPolicyServer::TCustomResult
        CheckWriteCapsForUsage( const RMessage2& aMsg,
                                TUint8 aUsage );

    public:     // Data

        // Status of the last initialization action
        static TInt iWimStatus;
        // Flag to tell if WIM is already initialized
        static TBool iWimInitialized;

        WIMI_Callbacks_t     iWimCallBack;
    private:    // Data
        // Pointer to Trust Settings Store instance. Owned.
        CWimTrustSettingsStore* iWimTrustSettingsStore;
        // Session counter
        TInt                 iSessionCount;
        // Pointer to CWimSessionRegistry
        CWimSessionRegistry* iWimSessionRegistry;
        // Pointer to CServer object
        static CWimServer*   iWimServer;
        // Callback structure for WIMI
       
        /**
        * Current service is accessing physical token.
        * Used to determine if SIM Refresh can be allowed.
        */
        TBool                iIsAccessingToken;
        /**
        * Informs server of completed SIM Refresh operation.
        * Is set if SIM Refresh notification is received in the middle of
        * service request that accesses physical token.
        * Server shall re-initialize itself at earliest convenience.
        */
        TBool                iRefreshNotificationReceived;
        /**
        * SAT refresh event observer. Owned.
        *
        */
   		CWimSatRefreshObserver* iRefreshObserver;
        /**
        * BT Sap refresh event observer. Owned.
        *
        */
   		CWimBTSapObserver* iBTSapObserver;
   		  
   		  // Pointer to Timer. Owned
   		  CWimTimer* iWimTimer;

#ifdef WIMSERVER_SHUTDOWN   		  
   		CShutdown iShutdown;
#endif   		
    };

#endif      // CWIMSERVER_H

// End of File
