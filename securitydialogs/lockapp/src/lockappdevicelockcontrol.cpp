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
* Description:  Devicelock UI
 *
*/


#include "lockappdevicelockcontrol.h"
#include "lockappdevicelockcontainer.h"
#include "lockappstatecontrolinterface.h"
#include "lockapppubsubobserver.h"
#include "lockappcenrepobserver.h"
#include "lockapputils.h"
#include "lockappwait.h"
#include "lockappkeycapturecontroller.h"

#include <lockapp.rsg> // general avkon resources
#include <avkon.rsg> // general avkon resources
#include <aknnotpi.rsg> // keyguard spesific resources
#include <AknUtils.h>
#include <akntitle.h> // CAknTitlePane
#include <activitymanager.h>
#include <e32property.h>
#include <etelmm.h>
#include <mmtsy_names.h>
#include <featmgr.h>
#include <secui.h>
#include <secuisecurityhandler.h>

#include <settingsinternalcrkeys.h>  // KSettingsAutolockStatus
#include <coreapplicationuisdomainpskeys.h>
#include <securityuisprivatepskeys.h>
#include <startupdomainpskeys.h>
#include <startupdomaincrkeys.h>
#include <ctsydomainpskeys.h>

#ifdef __SAP_TERMINAL_CONTROL_FW
#include <SCPClient.h>
#endif // __SAP_TERMINAL_CONTROL_FW

const TInt KAutoDeviceLockOff( 60000 );
const TInt KPhoneIndex( 0 );
const TInt KTriesToConnectServer( 2 );
const TInt KTimeBeforeRetryingServerConnection( 50000 );

// ---------------------------------------------------------------------------
// Standard Symbian OS construction sequence
// ---------------------------------------------------------------------------
CLockAppDevicelockControl* CLockAppDevicelockControl::NewL(MLockAppStateControl& aStateControl, RWindowGroup& aWg )
    {
    CLockAppDevicelockControl* self = new (ELeave) CLockAppDevicelockControl( aStateControl );
    INFO_4( "%s %s (%u) self=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, self );

    CleanupStack::PushL( self );
    self->ConstructL( aWg );
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Constructor passes the reference to the main state control.
// ---------------------------------------------------------------------------
CLockAppDevicelockControl::CLockAppDevicelockControl(MLockAppStateControl& aStateControl) :
    CLockAppBaseControl(aStateControl), iShowingSecCodeQuery(EFalse)
    {
    	INFO_4( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );

    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
CLockAppDevicelockControl::~CLockAppDevicelockControl( )
    {
    	INFO_4( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );

    /*****************************************************
    *    S60 Customer / ETel
    *    S60  ETel API
    *****************************************************/
    /*****************************************************
    *    S60 Customer / TSY
    *    Needs customer TSY implementation
    *****************************************************/
    INFO_4( "%s %s (%u) iCustomPhoneInitialized=%x -> -1 ", __FILE__, __PRETTY_FUNCTION__, __LINE__, iCustomPhoneInitialized );
    iCustomPhoneInitialized=-1;
    if ( iCustomPhone.SubSessionHandle( ) )
        {
        iCustomPhone.Close( );
        INFO_4( "%s %s (%u) iCustomPhoneInitialized=%x -> -2 ", __FILE__, __PRETTY_FUNCTION__, __LINE__, iCustomPhoneInitialized );
        iCustomPhoneInitialized=-2;
        }

		INFO_4( "%s %s (%u) iPhoneInitialized=%x -> -1 ", __FILE__, __PRETTY_FUNCTION__, __LINE__, iPhoneInitialized );
		iPhoneInitialized=-1;
    if ( iPhone.SubSessionHandle( ) )
        {
        iPhone.Close( );
        INFO_4( "%s %s (%u) iPhoneInitialized=%x -> -2 ", __FILE__, __PRETTY_FUNCTION__, __LINE__, iPhoneInitialized );
				iPhoneInitialized=-2;
        }

    INFO_4( "%s %s (%u) iTelServerInitialized=%x -> -1 ", __FILE__, __PRETTY_FUNCTION__, __LINE__, iTelServerInitialized );
    iTelServerInitialized=-1;
    if ( iTelServer.Handle( ) )
        {
        iTelServer.UnloadPhoneModule( KMmTsyModuleName );
        iTelServer.Close( );
        INFO_4( "%s %s (%u) iTelServerInitialized=%x -> -2 ", __FILE__, __PRETTY_FUNCTION__, __LINE__, iTelServerInitialized );
        iTelServerInitialized=-2;
        }
    if ( iActivityManager )
        {
        iActivityManager->Cancel( );
        delete iActivityManager;
        iActivityManager = NULL;
        }
    // CenRep observers
    if ( iCRAutoLockTime )
        {
        delete iCRAutoLockTime;
        iCRAutoLockTime = NULL;
        }
    if ( iCRAutoLockStatus )
        {
        delete iCRAutoLockStatus;
        iCRAutoLockStatus = NULL;
        }
    // PuSub observers
    if ( iPSAutolockState )
        {
        delete iPSAutolockState;
        iPSAutolockState = NULL;
        }
    if ( iContainer )
        {
        delete iContainer;
        iContainer = NULL;
        }
    }

// ---------------------------------------------------------------------------
// Devicelock UI constructor reserves localized resources, configures itself
// using CenRep and FeatureManager and reserves child controls.
// ---------------------------------------------------------------------------
void CLockAppDevicelockControl::ConstructL( RWindowGroup& aWg )
    {
    INFO( "CLockAppDevicelockControl::ConstructL started" );
    CLockAppBaseControl::ConstructL( );

    // feature manager is used for determining if the phone is a slider
    FeatureManager::InitializeLibL( );
    iFeatureProtocolCdma = FeatureManager::FeatureSupported( KFeatureIdProtocolCdma );
    INFO_1("CLockAppDevicelockControl::ConstructL - iFeatureProtocolCdma: %d", iFeatureProtocolCdma);
    FeatureManager::UnInitializeLib( );

    // Cba control
    iCba = CEikButtonGroupContainer::NewL( CEikButtonGroupContainer::ECba,
            CEikButtonGroupContainer::EHorizontal, this,
            R_KEYLOCK_SOFTKEYS_UNLOCK_EMPTY );

    TRect screenRect;
    AknLayoutUtils::LayoutMetricsRect( AknLayoutUtils::EScreen, screenRect );
    iCba->SetBoundingRect( screenRect );
    iCba->MakeVisible( EFalse );

    // Set up the status pane
    CEikStatusPane* sp = iAvkonAppUi->StatusPane();
    if ( sp )
        {
        // Switch the layout to show analog clock
        TRAPD(err, sp->SwitchLayoutL( R_AVKON_STATUS_PANE_LAYOUT_IDLE ) )
        ERROR(err, "CLockAppDevicelockControl::ConstructL - failed to switch status pane layout");
        if ( sp->PaneCapabilities(TUid::Uid(EEikStatusPaneUidTitle)).IsPresent( ) )
            {
            // Get the title pane control from status pane
            CAknTitlePane* titlePane = NULL;
            TRAPD(err2, titlePane = static_cast<CAknTitlePane*>( sp->ControlL( TUid::Uid( EEikStatusPaneUidTitle) ) ) )
            ERROR(err2, "CLockAppDevicelockControl::ConstructL - failed to get status title pane");
            if ( titlePane )
                {
                // Read localized "Locked" text from resource.
                HBufC* lockedString = iCoeEnv->AllocReadResourceL( R_TITLE_PANE_LOCKED );
                // Set as title pane text - titlepane takes ownership of the string
                titlePane->SetText( lockedString );
                }
            }
        }

    INFO( "CLockAppDevicelockControl::ConstructL completed" );
    }

void CLockAppDevicelockControl::ConnectToPhoneL( RWindowGroup& aWg )
		{
    INFO( "CLockAppDevicelockControl::ConnectToPhoneL - connecting to etel server" );
    // All server connections are tried to be made KTiesToConnectServer times because
    // occasional fails on connections are possible at least on some servers
    TInt err( KErrGeneral);
    TInt thisTry( 0);
    while ( ( err = iTelServer.Connect() ) != KErrNone && ( thisTry++ ) <= KTriesToConnectServer )
        {
        User::After( KTimeBeforeRetryingServerConnection );
        }
    ERROR(err, "CLockAppDevicelockControl::ConnectToPhoneL - connecting to etel server");
    User::LeaveIfError( err );
    INFO_4( "%s %s (%u) iTelServerInitialized=%x -> 2 ", __FILE__, __PRETTY_FUNCTION__, __LINE__, iTelServerInitialized );
    iTelServerInitialized=2;


    /*****************************************************
    *    S60 Customer / ETel
    *    S60  ETel API
    *****************************************************/
    /*****************************************************
    *    S60 Customer / TSY
    *    Needs customer TSY implementation
    *****************************************************/
    INFO( "CLockAppDevicelockControl::ConnectToPhoneL - loading TSY");
    err = iTelServer.LoadPhoneModule( KMmTsyModuleName );
    if ( err != KErrAlreadyExists )
        {
        // may return also KErrAlreadyExists if some other
        // is already loaded the tsy module. And that is not an error.
        ERROR(err, "CLockAppDevicelockControl::ConnectToPhoneL - loading TSY");
        User::LeaveIfError( err );
        }
    INFO_4( "%s %s (%u) iTelServerInitialized=%x -> 3 ", __FILE__, __PRETTY_FUNCTION__, __LINE__, iTelServerInitialized );
    iTelServerInitialized=3;

    /*****************************************************
    *    S60 Customer / ETel
    *    S60  ETel API
    *****************************************************/
    INFO( "CLockAppDevicelockControl::ConnectToPhoneL - opening phone");
    RTelServer::TPhoneInfo PhoneInfo;
    User::LeaveIfError( iTelServer.SetExtendedErrorGranularity( RTelServer::EErrorExtended ) );
    INFO_4( "%s %s (%u) iTelServerInitialized=%x -> 5 ", __FILE__, __PRETTY_FUNCTION__, __LINE__, iTelServerInitialized );
    iTelServerInitialized=5;
    User::LeaveIfError( iTelServer.GetPhoneInfo( KPhoneIndex, PhoneInfo ) );
    INFO_4( "%s %s (%u) iTelServerInitialized=%x -> 6 ", __FILE__, __PRETTY_FUNCTION__, __LINE__, iTelServerInitialized );
    iTelServerInitialized=6;
    User::LeaveIfError( iPhone.Open( iTelServer, PhoneInfo.iName ) );
		INFO_4( "%s %s (%u) iPhoneInitialized=%x -> 2 ", __FILE__, __PRETTY_FUNCTION__, __LINE__, iPhoneInitialized );
		iPhoneInitialized=2;
    User::LeaveIfError( iCustomPhone.Open( iPhone ) );
		INFO_4( "%s %s (%u) iCustomPhoneInitialized=%x -> 2 ", __FILE__, __PRETTY_FUNCTION__, __LINE__, iCustomPhoneInitialized );
		iCustomPhoneInitialized=2;

    INFO( "CLockAppDevicelockControl::ConnectToPhoneL - phone opened");


    TBool systemLocked(EFalse);

    // Set up CenRep observers
    iCRAutoLockTime = CLockAppCenRepObserver::NewL(this, KCRUidSecuritySettings, KSettingsAutoLockTime);
    iCRAutoLockStatus = CLockAppCenRepObserver::NewL(this, KCRUidSecuritySettings, KSettingsAutolockStatus);

    #ifndef __WINS__

    /*****************************************************
    *    S60 Customer / ETel
    *    S60 ETel API
    *****************************************************/

    RMobilePhone::TMobilePhoneLock lockType = RMobilePhone::ELockPhoneDevice;
    RMobilePhone::TMobilePhoneLockInfoV1 lockInfo;
    RMobilePhone::TMobilePhoneLockInfoV1Pckg lockInfoPkg(lockInfo);

    TRequestStatus getLockInfoStatus;
    iPhone.GetLockInfo( getLockInfoStatus, lockType, lockInfoPkg );
		INFO_4( "%s %s (%u) iPhoneInitialized=%x -> 3 ", __FILE__, __PRETTY_FUNCTION__, __LINE__, iPhoneInitialized );
		iPhoneInitialized=3;
    User::WaitForRequest( getLockInfoStatus );

    TInt lockValue(0);
    TInt cRresult = iCRAutoLockStatus->GetValue( lockValue );
    INFO_2( "CLockAppDevicelockControl::ConnectToPhoneL - CR autolockstatus=%d , res=%d", lockValue, cRresult);
    TBool hiddenReset = IsHiddenReset( );
    INFO_1( "CLockAppDevicelockControl::ConnectToPhoneL - hiddenReset=%d", hiddenReset );
    if ( lockInfo.iSetting == RMobilePhone::ELockSetDisabled )
        {
        INFO( "CLockAppDevicelockControl::ConnectToPhoneL - ELockSetDisabled");
        iCRAutoLockTime->SetValue( 0 );
        if ( iFeatureProtocolCdma )
            {
            iCRAutoLockTime->SetKeyValue( KSettingsLockOnPowerUp, 0 );
            }
        }
    else
        {
        if ( iFeatureProtocolCdma || (hiddenReset && (lockValue == 1)) )
            {
            // In CDMA, the system can stay locked on after the boot-up sequence.
            INFO( "CLockAppDevicelockControl::ConnectToPhoneL - Hidden reset when locked");
            systemLocked = ETrue;
            }
        }
    #endif //__WINS__

    // Create devicelock UI container
    INFO_1( "CLockAppDevicelockControl::ConnectToPhoneL - creating CLockAppDevicelockContainer=%d", 1 );
    iContainer = CLockAppDevicelockContainer::NewL( aWg );
    INFO_1( "CLockAppDevicelockControl::ConnectToPhoneL - creating CLockAppDevicelockContainer=%d done", 1 );

    INFO_1( "CLockAppDevicelockControl::ConnectToPhoneL - creating DefinePubSubKeysL=%d", 1 );
    DefinePubSubKeysL( );
    INFO_1( "CLockAppDevicelockControl::ConnectToPhoneL - creating DefinePubSubKeysL=%d", 1 );

    // The following sequence is used to validate the configuration on SCP server.
    // This is needed on the first boot (initial or RFS) or if the C-drive
    // has been formatted (3-button format) and Autolock is not active.
#ifdef __SAP_TERMINAL_CONTROL_FW
		// This seems to be defined always.
    INFO( "CLockAppDevicelockControl::ConnectToPhoneL - Validate SCP Config" );
    RSCPClient scpClient;
    if ( scpClient.Connect() == KErrNone )
        {
        CleanupClosePushL( scpClient );
        TInt confStatus = scpClient.CheckConfiguration( KSCPInitial );
        if ( confStatus == KErrAccessDenied )
            {
#ifndef __WINS__
            if ( ( lockInfo.iSetting == RMobilePhone::ELockSetDisabled ) )
#else // __WINS__
            if ( ETrue ) // DOS lock is never active in WINS
#endif // __WINS__
                {
                // DOS lock is not active. Note that if DOS is locked, checking the code here will
                // mess up the query sequence. On initial startup DOS is not locked.
                TInt finalConfStatus = scpClient.CheckConfiguration( KSCPComplete );
                if ( finalConfStatus == KErrAccessDenied )
                    {
#ifdef __WINS__
                    ERROR(finalConfStatus, "CLockAppDevicelockControl::ConnectToPhoneL - DOS validation FAILED in WINS!");
#else // !__WINS__
                    // The SCP server is out of sync and Autolock is not active. (c-drive formatted)
                    // We must ask the security code. ( Note that it is very rare that this is executed )
                    INFO( "CLockAppDevicelockControl::ConnectToPhoneL - Lock setting disabled, calling setlocksetting");

                    // Wait here until the startup is complete
                    TInt tarmErr(KErrNone);
                    while ( tarmErr == KErrNone )
                        {
                        TInt sysState;
                        tarmErr = RProperty::Get(KPSUidStartup, KPSGlobalSystemState, sysState);
                        if ((sysState == ESwStateNormalRfOn) ||
                            (sysState == ESwStateNormalRfOff) ||
                            (sysState == ESwStateNormalBTSap))
                            {
                            break;
                            }
                        User::After(500000); // half a second
                        }

                    // Just change the lock setting again to disabled to request the security code.
                    // Set the TARM flag so SecUi knows it should display the "login" query.
                    TInt tarmFlag;
                    TInt tRet = RProperty::Get( KSCPSIDAutolock, SCP_TARM_ADMIN_FLAG_UID, tarmFlag );
                    if ( tRet == KErrNone )
                        {
                        tarmFlag |= KSCPFlagResyncQuery;
                        tRet = RProperty::Set( KSCPSIDAutolock, SCP_TARM_ADMIN_FLAG_UID, tarmFlag );
                        }
                    ERROR(tRet, "CLockAppDevicelockControl::ConnectToPhoneL - FAILED to set TARM Admin Flag" );

                    TRequestStatus setLockSettingStatus;
                    RMobilePhone::TMobilePhoneLockSetting lockChange = RMobilePhone::ELockSetDisabled;
                    iPhone.SetLockSetting( setLockSettingStatus, lockType, lockChange );
                    User::WaitForRequest( setLockSettingStatus );
#endif // __WINS__
                    }
                }
            }
        CleanupStack::PopAndDestroy(); // calls Close() on scpClient
        }
#endif // __SAP_TERMINAL_CONTROL_FW

    // Set up P&S observers
    iPSAutolockState = CLockAppPubSubObserver::NewL( this, KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus );

    if ( systemLocked )
        {
        INFO( "CLockAppDevicelockControl::ConnectToPhoneL - Lock system");
        INFO_4( "%s %s (%u) 1=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 1 );
        iStateControl.EnableDevicelockL( EDevicelockManual );
        INFO( "CLockAppDevicelockControl::ConnectToPhoneL - Lock system. Done");
        }

    // Activity manager
    iActivityManager = CUserActivityManager::NewL( CActive::EPriorityStandard );
    StartActivityMonitoringL( );

    // Setup key pattern matcher
    if ( !SetupKeyPatternsWithPolicyL( EPolicyDevicelockQuery ) )
        {
        INFO( "CLockAppDevicelockControl::ConnectToPhoneL - No CenRep policy defined" );
        iKeyPattern->AddPattern( EStdKeyDevice0, 0 ); // '0' = second key code doesnt matter
        }
#ifdef __WINS__
    iKeyPattern->AddPattern( EStdKeyDevice0, 0 ); // LSK + *
#endif

    INFO( "CLockAppDevicelockControl::ConstructL completed" );
    }

// ---------------------------------------------------------------------------
// Define internal P&S autolock status key
// ---------------------------------------------------------------------------
void CLockAppDevicelockControl::DefinePubSubKeysL()
    {
    INFO( "CLockAppDevicelockControl::DefinePubSubKeysL" );

    // Create the write policy. Also processes with write device data can write the value.
    TSecurityPolicy writePolicy( ECapabilityWriteDeviceData );
    // Create the read policy. Also processes with read device data can read the value.
    TSecurityPolicy readPolicy( ECapabilityReadDeviceData );

    TInt ret = RProperty::Define( KPSUidSecurityUIs,
            KSecurityUIsSecUIOriginatedQuery, RProperty::EInt, readPolicy, writePolicy );
    ERROR(ret, "CLockAppDevicelockControl::DefinePubSubKeysL - FAILED to define the SECUI query Flag");

    ret = RProperty::Define( KPSUidSecurityUIs,
            KSecurityUIsQueryRequestCancel, RProperty::EInt, readPolicy, writePolicy );
    ERROR(ret, "CLockAppDevicelockControl::DefinePubSubKeysL - FAILED to define the SECUI query request state Flag");

    _LIT_SECURITY_POLICY_PASS(KReadPolicy);
    _LIT_SECURITY_POLICY_C1(KWritePolicy, ECapabilityWriteDeviceData);
    ret = RProperty::Define( KPSUidCoreApplicationUIs,
            KCoreAppUIsAutolockStatus, RProperty::EInt, KReadPolicy, KWritePolicy);
    RProperty::Set(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, EAutolockOff);
    if (ret != KErrAlreadyExists)
        {
        ERROR(ret, "CLockAppDevicelockControl::DefinePubSubKeysL - FAILED to set autolock status");
        User::LeaveIfError(ret);
        }

    #ifdef __SAP_TERMINAL_CONTROL_FW
    // Define the TARM admin flag.
    ret = RProperty::Define( KSCPSIDAutolock,
            SCP_TARM_ADMIN_FLAG_UID, RProperty::EInt, readPolicy, writePolicy );
    ERROR(ret, "CLockAppDevicelockControl::DefinePubSubKeysL - FAILED to define the TARM Admin Flag");
    #endif // __SAP_TERMINAL_CONTROL_FW
    }

// ----------------------------------------------------------
// Checks whether we are booting from a Hidden Reset
// ----------------------------------------------------------
TBool CLockAppDevicelockControl::IsHiddenReset( )
    {
    	    	INFO_4( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );

    TBool ret( EFalse );
    TInt startupReason( ENormalStartup );
    TInt err( KErrNone);
    err = RProperty::Get( KPSUidStartup, KPSStartupReason, startupReason );
    ERROR(err, "CLockAppDevicelockControl::IsHiddenReset - error getting startup reason" );
    ret = (startupReason != ENormalStartup);
    INFO_1( "CLockAppDevicelockControl::IsHiddenReset = %d", ret );
		#ifdef _DEBUG
			// test to simulate HiddenReset
			RFs fs;
			_LIT(KTestHiddenReset,"f:\\TestHiddenReset.txt");
			RFile file;
			User::LeaveIfError(fs.Connect());
			err=file.Open(fs, KTestHiddenReset, EFileStreamText|EFileRead|EFileShareReadersOnly);
			if(err==KErrNone)
				{
				INFO_4( "%s %s (%u) ??? TestHiddenReset=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 1 );
				ret = ETrue;
				}
			file.Close();
			fs.Close();
		#endif
    return ret;
    }

// ----------------------------------------------------------
// Checks whether PIN1/UPIN is blocked
// ----------------------------------------------------------
TBool CLockAppDevicelockControl::IsPinBlocked( )
    {
    INFO( "CLockAppDevicelockControl::IsPinBlocked" );
    TBool ret( EFalse );
    RMmCustomAPI::TSecurityCodeType secCodeType;
#if defined(__PROTOCOL_WCDMA) || defined(__UPIN)
    RMobilePhone::TMobilePhoneSecurityCode activePin;
    iCustomPhone.GetActivePin( activePin );
    if ( activePin == RMobilePhone::ESecurityUniversalPin )
        secCodeType = RMmCustomAPI::ESecurityUniversalPin;
    else
        secCodeType = RMmCustomAPI::ESecurityCodePin1;
#else
    secCodeType = RMmCustomAPI::ESecurityCodePin1;
#endif //(__PROTOCOL_WCDMA) || defined(__UPIN)
    iCustomPhone.IsBlocked( secCodeType, ret );
    INFO_1( "CLockAppDevicelockControl::IsPinBlocked = %d", ret );
    return ret;
    }

// ----------------------------------------------------------
// Try to get (and optionally unset) the TARM Admin Flag
// ----------------------------------------------------------
TBool CLockAppDevicelockControl::TarmAdminFlag( TBool unSetFlag )
    {
    INFO_1( "CLockAppDevicelockControl::TarmAdminFlag(unSet = %d)", unSetFlag );
    TBool ret(EFalse);
#ifdef __SAP_TERMINAL_CONTROL_FW
    // Get the TARM admin flag value
    TInt tarmFlag;
    TInt err = RProperty::Get( KSCPSIDAutolock, SCP_TARM_ADMIN_FLAG_UID, tarmFlag );
    if ( err != KErrNone )
        {
        ERROR(err, "CLockAppDevicelockControl::TarmAdminFlag - Failed to get TARM flag" );
        }
    else
        {
        INFO_1( "CLockAppDevicelockControl::TarmAdminFlag - TARM flag: %d", tarmFlag );
        }

    if ( tarmFlag & KSCPFlagAdminLock )
        {
        ret = ETrue;
        // Unset the TARM admin flag if set
        if ( unSetFlag )
            {
            tarmFlag &= ~KSCPFlagAdminLock;
            err = RProperty::Set( KSCPSIDAutolock, SCP_TARM_ADMIN_FLAG_UID, tarmFlag );
            ERROR(err, "CLockAppDevicelockControl::TarmAdminFlag - FAILED to unset TARM flag" );
            }
        }
#endif // __SAP_TERMINAL_CONTROL_FW
    INFO_1( "CLockAppDevicelockControl::TarmAdminFlag = %d", ret );
    return ret;
    }

// ---------------------------------------------------------------------------
// Check ETEL lock info and ask sec code if neccessary
// ---------------------------------------------------------------------------
TBool CLockAppDevicelockControl::ETelActivationAllowed( )
    {
    INFO( "CLockAppDevicelockControl::ETelActivationAllowed" );
    TBool ret(EFalse);
#ifdef __WINS__
    ret = ETrue;
#else //__WINS__

    /*****************************************************
    *    S60 Customer / ETel
    *    S60  ETel API
    *****************************************************/


		INFO_4( "%s %s (%u) 111 value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );

    // INFO_4( "%s %s (%u) iStateControl=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iStateControl );

		INFO_4( "%s %s (%u) checking iPhoneInitialized=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
		INFO_4( "%s %s (%u) iPhoneInitialized=%x -> 4 ", __FILE__, __PRETTY_FUNCTION__, __LINE__, iPhoneInitialized );
		iPhoneInitialized=4;

    RMobilePhone::TMobilePhoneLock lockType = RMobilePhone::ELockPhoneDevice;
    RMobilePhone::TMobilePhoneLockInfoV1 lockInfo;
    RMobilePhone::TMobilePhoneLockInfoV1Pckg lockInfoPkg(lockInfo);
    RMobilePhone::TMobilePhoneLockSetting lockChange(RMobilePhone::ELockSetDisabled);

		INFO_4( "%s %s (%u) before getLockInfoStatus=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    TRequestStatus getLockInfoStatus;
		INFO_4( "%s %s (%u) after getLockInfoStatus=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
		
		INFO_4( "%s %s (%u) iPhoneInitialized=%x -> 5 ", __FILE__, __PRETTY_FUNCTION__, __LINE__, iPhoneInitialized );
		iPhoneInitialized=5;
    iPhone.GetLockInfo( getLockInfoStatus, lockType, lockInfoPkg );
		INFO_4( "%s %s (%u) iPhoneInitialized=%x -> 6 ", __FILE__, __PRETTY_FUNCTION__, __LINE__, iPhoneInitialized );
		iPhoneInitialized=6;
    User::WaitForRequest( getLockInfoStatus );

    INFO_1( "CLockAppDevicelockControl::ETelActivationAllowed - GetLockInfo status: %d ", getLockInfoStatus.Int() );
    if ( getLockInfoStatus.Int( )== KErrNone )
        {
        if ( lockInfo.iSetting == RMobilePhone::ELockSetDisabled )
            {
            // ask code
            INFO( "CLockAppDevicelockControl::ETelActivationAllowed - ETel ELockSetDisabled, Ask Code" );
            // Raise a flag to indicate that the UPIN request coming from ETEL
            // has originated from SecUi and not from Engine.
            RProperty::Set( KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery, ESecurityUIsSystemLockOriginated );
            TRequestStatus setLockSettingStatus;
            lockChange = RMobilePhone::ELockSetEnabled;
            iPhone.SetLockSetting( setLockSettingStatus, lockType, lockChange );
            User::WaitForRequest( setLockSettingStatus );
            INFO_1( "CLockAppDevicelockControl::ETelActivationAllowed - SetLockSetting status: %d ", setLockSettingStatus.Int() );
            // Lower the flag
            RProperty::Set( KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery, ESecurityUIsETelAPIOriginated );
            if (setLockSettingStatus.Int() == KErrNone)
                {
                ret = ETrue;
                }
            }
        else
            {
            INFO( "CLockAppDevicelockControl::ETelActivationAllowed - Lock System Ok" );
            ret = ETrue;
            }
        }
    else
        {
        INFO( "CLockAppDevicelockControl::ETelActivationAllowed - Error getting LockInfo - Ask code (SLS)" );
        // Raise a flag to indicate that the UPIN request coming from ETEL
        // has originated from SecUi and not from Engine.
        RProperty::Set( KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery, ESecurityUIsSystemLockOriginated );
        TRequestStatus setLockSettingStatus;
        lockChange = RMobilePhone::ELockSetEnabled;
				INFO_4( "%s %s (%u) iPhoneInitialized=%x -> 8 ", __FILE__, __PRETTY_FUNCTION__, __LINE__, iPhoneInitialized );
				iPhoneInitialized=8;
        iPhone.SetLockSetting( setLockSettingStatus, lockType, lockChange );
				INFO_4( "%s %s (%u) iPhoneInitialized=%x -> 9 ", __FILE__, __PRETTY_FUNCTION__, __LINE__, iPhoneInitialized );
				iPhoneInitialized=9;
        User::WaitForRequest(setLockSettingStatus);
        INFO_1( "CLockAppDevicelockControl::ETelActivationAllowed - SetLockSetting status: %d ", setLockSettingStatus.Int() );
        // Lower the flag
        RProperty::Set( KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery, ESecurityUIsETelAPIOriginated );
        if (setLockSettingStatus.Int() == KErrNone)
            {
            ret = ETrue;
            }
        }
#endif
    INFO_1( "CLockAppDevicelockControl::ETelActivationAllowed = %d", ret );
    return ret;
    }

// ---------------------------------------------------------------------------
// Check weather its allowed to activate the control
// ---------------------------------------------------------------------------
TBool CLockAppDevicelockControl::ActivationAllowedL( TDevicelockReason aReason )
    {
    INFO_1( "CLockAppDevicelockControl::ActivationAllowedL aReason= %d", aReason );
    
    if (aReason==ETimerLocked)
    	{
 			 { // REQ 414-5466 Prevention of device lock in context of Hands Free Voice UI
	     TInt vuiValue = 0;
	     TUid KHFVuiModePSUid = { 0x102818E7 };
	     enum THFVuiModePSKeys
	         {
	         EHFVuiPSModeId = 1000
	         };
	     TInt tRet = RProperty::Get(KHFVuiModePSUid, EHFVuiPSModeId, vuiValue);  // also 0 if can't get because permissions or because doesn't exists
	     #if defined(_DEBUG)
	         INFO_4( "%s %s (%u) getting KHFVuiModePSUid+EHFVuiPSModeId=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, tRet );
	         INFO_4( "%s %s (%u) vuiValue=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, vuiValue );
	     #endif
	     if(vuiValue)
	         {
	         #if defined(_DEBUG)
	         RDebug::Print(_L("(LOCKAPP)CLockAppDevicelockControl::ActivationAllowedL() Voice functions active. No locking possible."));
	         #endif
	         return EFalse;
	         }
	     }
	     { // if another query is displayed, the future un-lock query will crash. Don't allow time-lock in this case.
	     TInt secQueryStatus = ESecurityQueryUninitialized;
	     TInt tRet = RProperty::Get(KPSUidStartup, KStartupSecurityCodeQueryStatus, secQueryStatus);  // also 0 if can't get because permissions or because doesn't exists
	     #if defined(_DEBUG)
	         INFO_4( "%s %s (%u) getting KPSUidStartup+KStartupSecurityCodeQueryStatus=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, tRet );
	         INFO_4( "%s %s (%u) secQueryStatus=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, secQueryStatus );
	     #endif
	     if(secQueryStatus==ESecurityQueryActive)
	         {
	         #if defined(_DEBUG)
	         RDebug::Print(_L("(LOCKAPP)CLockAppDevicelockControl::ActivationAllowedL() Asking some other security code. No locking possible."));
	         #endif
	         return EFalse;
	         }
	     }
    	}
    
    TInt lightStatus=EForcedLightsUninitialized; 
    RProperty::Get(KPSUidCoreApplicationUIs,KLightsVTForcedLightsOn,lightStatus ); 
    //If display is forced on. don't lock 
    if(lightStatus == EForcedLightsOn ) 
        { 
        #if defined(_DEBUG) 
        RDebug::Print(_L("(LOCKAPP)CLockAppDevicelockControl::ActivationAllowedL() Display is forced on deivce not locked")); 
        #endif 
        return EFalse;
        } 

    // first check ETEL side - and ask sec code if needed
    if ( !ETelActivationAllowed() )
        {
        return EFalse;
        }

    if ( IsPinBlocked( ) )
        {
        return EFalse;
        }

    TBool ret(EFalse);
    TInt sysState(0);
    RProperty::Get( KPSUidStartup, KPSGlobalSystemState, sysState );
    //If NOT in CDMA the Autolock should come up only after the phone has booted up.
    if ( iFeatureProtocolCdma || IsHiddenReset( ) )
        {
        if ( sysState == ESwStateNormalRfOn ||
             sysState == ESwStateNormalRfOff ||
             sysState == ESwStateCriticalPhaseOK )
            {
            INFO( "CLockAppDevicelockControl::ActivationAllowedL - Locked after Hidden Reset" );
            ret = ETrue;
            }
        }
    else
        {
        if ( sysState == ESwStateNormalRfOn || sysState == ESwStateNormalRfOff )
            {
            ret = ETrue;
            }
        }

    if ( IsBitFieldSet( iStateControl.EnvironmentStatus( ), KLockAppEnvPhonecallOngoing ) && !TarmAdminFlag(EFalse) )
        {
        if ( aReason == EDevicelockRemote )
            {
            INFO( "CLockAppDevicelockControl::ActivationAllowedL - remote lock allowed" );
            ret = ETrue;
            }
        }
    return ret;
    }

// ---------------------------------------------------------------------------
// Check weather its allowed to deactivate the control
// ---------------------------------------------------------------------------
TBool CLockAppDevicelockControl::DeActivationAllowedL()
    {
    if ( iShowingSecCodeQuery )
        {
        return ETrue;
        }
    else
        {
        return EFalse;
        }
    }

// ---------------------------------------------------------------------------
// Set the devicelocking reason
// ---------------------------------------------------------------------------
void CLockAppDevicelockControl::SetLockingReason( TDevicelockReason aReason )
    {
#ifndef RD_REMOTELOCK
    iPSAutolockState->SetKeyValue( EAutolockOn );
#else
    switch ( aReason )
        {
        case EDevicelockManual:
            iPSAutolockState->SetKeyValue( EManualLocked );
            break;
        case EDevicelockRemote:
            iPSAutolockState->SetKeyValue( ERemoteLocked );
            break;
        case EDevicelockTimer:
            iPSAutolockState->SetKeyValue( ETimerLocked );
            break;
        default:
            DoPanic( ELockIllegalState );
        }
#endif // RD_REMOTELOCK
    }

// ---------------------------------------------------------------------------
// Activate control
// ---------------------------------------------------------------------------
void CLockAppDevicelockControl::HandleActivateEventL( TUint aEnvMask )
    {
    INFO_1("CLockAppDevicelockControl::HandleActivateEventL - aEnvMask: %d", aEnvMask);
    
    CLockAppBaseControl::HandleActivateEventL( aEnvMask );

    if ( IsBitFieldSet( aEnvMask, KLockAppEnvScreenSaverOn ) )
        {
        // if screensaver is on - capture primary keys
        CapturePrimaryKeys( ETrue );
        }
    // capture keys
    CLockAppKeyCaptureController::CaptureKey( EStdKeyApplication0, EKeyApplication0, EKeyCaptureAllEvents ); // App key
    CLockAppKeyCaptureController::CaptureKey( EStdKeyDevice2, EKeyDevice2, EKeyCaptureAllEvents ); // Power key (for lights)
    CLockAppKeyCaptureController::CaptureKey( EStdKeyDevice6, EKeyDevice6, EKeyCaptureAllEvents ); // Voice key (for lights)
    CLockAppKeyCaptureController::CaptureKey( EStdKeyNo, EKeyNo, EKeyCaptureAllEvents ); // End key (for Rosetta lights)
    CLockAppKeyCaptureController::CaptureKey( EStdKeyDeviceF, EKeyDeviceF, EKeyCaptureAllEvents ); // switch key (for touch devices)

    SetPointerEventCapture( ETrue );
    SetKeyguardIndicatorStateL( ETrue );
    iContainer->MakeVisible( ETrue ); // maybe not needed - as its in different windowgroup
    ShowCba( ETrue );
    ShowStatusPane( ETrue );
    // close task-list in case it is displayed : fast-swap window
    iEikonEnv->DismissTaskList( );
    iCRAutoLockStatus->SetValue( ETrue );
    }

// ---------------------------------------------------------------------------
// DeActivate control
// ---------------------------------------------------------------------------
void CLockAppDevicelockControl::HandleDeActivateEventL( TUint aEnvMask )
    {
    INFO_1("CLockAppDevicelockControl::HandleDeActivateEventL - aEnvMask: %d", aEnvMask);
    
    CLockAppBaseControl::HandleDeActivateEventL( aEnvMask );

    if ( IsBitFieldSet( aEnvMask, KLockAppEnvScreenSaverOn ) )
        {
        // if screensaver is on - uncapture primary keys
        CapturePrimaryKeys( EFalse );
        }

    // uncapture keys
    CLockAppKeyCaptureController::ReleaseKey( EStdKeyApplication0 );
    CLockAppKeyCaptureController::ReleaseKey( EStdKeyDevice2 );
    CLockAppKeyCaptureController::ReleaseKey( EStdKeyDevice6 );
    CLockAppKeyCaptureController::ReleaseKey( EStdKeyNo );
    CLockAppKeyCaptureController::ReleaseKey( EStdKeyDeviceF );
    
    SetPointerEventCapture( EFalse );
    SetKeyguardIndicatorStateL( EFalse );
    iContainer->MakeVisible( EFalse ); // maybe not needed - as its in different windowgroup
    ShowCba( EFalse );
    ShowStatusPane( EFalse );
    iPSAutolockState->SetKeyValue( EAutolockOff );
    iCRAutoLockStatus->SetValue( EFalse );
    }

// ---------------------------------------------------------------------------
// Handle environment changes (Screensaver, Telephony, etc.)
// ---------------------------------------------------------------------------
void CLockAppDevicelockControl::HandleEnvironmentChange( TUint aEnvMask, TUint aEventMask )
    {
    INFO_4( "%s %s (%u) aEnvMask=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aEnvMask );
    INFO_4( "%s %s (%u) aEventMask=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aEventMask );

    if ( IsBitFieldSet( aEventMask, KLockAppEnvScreenSaverOn ) )
        {
        // screen saver state changed
        CapturePrimaryKeys( IsBitFieldSet( aEnvMask, KLockAppEnvScreenSaverOn ) );
        }
    if ( IsBitFieldSet( aEventMask, KLockAppEnvFPS ) )
        {
        TInt lockValue=0;
    		iPSAutolockState->GetKeyValue( lockValue );
    		INFO_4( "%s %s (%u) lockValue=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, lockValue );
    		if(lockValue>EAutolockOff)	// device is locked and query is not open
    			{
    			// indicate to TARM that it should not ask for password
    			TInt secUiOriginatedQuery(ESecurityUIsSecUIOriginatedUninitialized);
    			RProperty::Get(KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery, secUiOriginatedQuery);
    			INFO_4( "%s %s (%u) secUiOriginatedQuery=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, secUiOriginatedQuery );
    			RProperty::Set( KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery, ESecurityUIsFpsOriginated );

        	HandleUnlockCommandL( );
    			RProperty::Set( KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery, secUiOriginatedQuery );	// reset to initial
        	}
        else												// device needs to be locked. Same happens in keyguard control becasue probably this is never called
        	iStateControl.EnableDevicelockL( EDevicelockManual );
        }
    if ( IsBitFieldSet( aEnvMask, KLockAppEnvGrip ) )
    	{
      INFO_4( "%s %s (%u) iShowingSecCodeQuery=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iShowingSecCodeQuery );
	    if ( IsBitFieldSet( aEventMask, KLockAppEnvGrip ) )	//Grip opened
	        {
	        if(iShowingSecCodeQuery==EFalse)
	        	{
	        		TInt lockValue=0;
	        		iPSAutolockState->GetKeyValue( lockValue );
	        		INFO_4( "%s %s (%u) lockValue=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, lockValue );
	        		if(lockValue>EAutolockOff)
	        			{
				        // ask unlock code by sending the menu key. This works on touch?
			        	TApaTaskList tasklist( iEikonEnv->WsSession() );
			        	#define KAknCapServerUid TUid::Uid( 0x10207218 )
			        	TApaTask capserver = tasklist.FindApp( KAknCapServerUid );
    						INFO_4( "%s %s (%u) KAknCapServerUid=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, KAknCapServerUid );
			        	if( capserver.Exists() )
			        	    {
	        	        INFO_4( "%s %s (%u) capserver.Exists() EStdKeyDevice0=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, EStdKeyDevice0 );
			        	    TKeyEvent key;
			        	    key.iCode = EKeyDevice0;
			        	    key.iModifiers = 0;
			        	    key.iRepeats = 0;
			        	    key.iScanCode = EStdKeyDevice0;
			        	    capserver.SendKey( key );
			        	    }
			        	}
			       }
        	}
			else
					{
					if(iShowingSecCodeQuery==EFalse)
	        	{
            //the device lock query is on top
	        	//generate cancel key event
	        	const TInt KCancelKeyCode( 165 );
	        	INFO_4( "%s %s (%u) KCancelKeyCode=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, KCancelKeyCode );

	        	TRawEvent rawEvent;
	        	rawEvent.Set( TRawEvent::EKeyDown, KCancelKeyCode );
	        	iEikonEnv->WsSession().SimulateRawEvent( rawEvent );        	
						}
					}
			}
    }

// ---------------------------------------------------------------------------
// Handle all Central Repository observer callbacks.
// ---------------------------------------------------------------------------
void CLockAppDevicelockControl::HandleCenRepNotify(TUid /*aCenRepUid*/, TUint32 aKeyId, TInt aValue )
    {
    INFO_4( "%s %s (%u) aKeyId=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aKeyId );
    INFO_4( "%s %s (%u) aValue=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aValue );
    switch ( aKeyId )
        {
        case KSettingsAutoLockTime:
            {
            INFO_1( "CLockAppDevicelockControl::HandleCenRepNotify - KSettingsAutoLockTime = %d", aValue );
            ResetInactivityTimeout( );
            }
            break;
        default:
            break;
        }
    }

// ---------------------------------------------------------------------------
// Handle all Publish & Subscribe observer callbacks.
// ---------------------------------------------------------------------------
void CLockAppDevicelockControl::HandlePubSubNotify(TUid aPubSubUid, TUint aKeyId, TInt aValue )
    {
    INFO_4( "%s %s (%u) KPSUidCoreApplicationUIs=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, KPSUidCoreApplicationUIs );
    INFO_4( "%s %s (%u) aKeyId=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aKeyId );
    INFO_4( "%s %s (%u) aValue=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aValue );
    if ( aPubSubUid == KPSUidCoreApplicationUIs )
        {
        switch ( aKeyId )
            {
            case KCoreAppUIsAutolockStatus:
                {
                INFO_1( "CLockAppDevicelockControl::HandlePubSubNotify - KCoreAppUIsAutolockStatus = %d", aValue );
                // Autolock used to react to this PubSub key - but its unsafe and
                // in future API will be used, and the key will be published by Lockapp
                INFO_1( "CLockAppDevicelockControl::HandlePubSubNotify - nothing done. LockApp reacts only to API = %d", aValue );
                }
                break;
            default:
                break;
            }
        }
    }

// ---------------------------------------------------------------------------
// Devicelock UI key events are handled trough here.
// ---------------------------------------------------------------------------
TKeyResponse CLockAppDevicelockControl::OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType )
    {
    INFO_4( "%s (%u) aKeyEvent.iCode=%x aType=%x", __FILE__, __LINE__, aKeyEvent.iCode, aType );
    INFO_4( "%s %s (%u) iActive=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, iActive );
    if ( iActive )
        {
    		if(AknLayoutUtils::PenEnabled())
    			{
    			if(aKeyEvent.iCode == EKeyDeviceF)	// any Type
    				{
    				HandleUnlockCommandL( );
    				}
    			}
        if ( aType == EEventKeyDown )
            {
            if ( !iShowingSecCodeQuery )
                {
                switch ( iKeyPattern->HandleKeyEvent( aKeyEvent.iScanCode ) )
                    {
                    case EPatternPrimaryMatch:
                        HandleUnlockCommandL( );
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    return EKeyWasConsumed;
    }

// ---------------------------------------------------------------------------
// Handle unlock command
// ---------------------------------------------------------------------------
void CLockAppDevicelockControl::HandleUnlockCommandL( )
    {
    INFO( "CLockAppDevicelockControl::HandleUnlockCommandL" );
    // inform sysap to put lights on left soft key press
    SendMessageToSysAp( EEikKeyLockLightsOnRequest );
		INFO_4( "%s %s (%u) iPhoneInitialized=%x -> 10 ", __FILE__, __PRETTY_FUNCTION__, __LINE__, iPhoneInitialized );
		iPhoneInitialized=10;
    CSecurityHandler* handler = new (ELeave) CSecurityHandler( iPhone );
		INFO_4( "%s %s (%u) got handler=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 1 );
    CleanupStack::PushL( handler );
    TSecUi::InitializeLibL( );
		INFO_4( "%s %s (%u) got TSecUi::InitializeLibL=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 1 );
    iShowingSecCodeQuery = ETrue;
    TRAPD(err, {
								INFO_4( "%s %s (%u) before AskSecCodeInAutoLockL=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 1 );
                TBool ret = handler->AskSecCodeInAutoLockL();
								INFO_4( "%s %s (%u) after AskSecCodeInAutoLockL=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, ret );
                INFO_1( "CLockAppDevicelockControl::HandleUnlockCommandL - AskSecCodeInAutoLockL = %d", ret );
                if ( ret )
                    {
                    iStateControl.DisableDevicelockL();
                    }
                })
    ERROR(err, "CLockAppDevicelockControl::HandleUnlockCommandL - AskSecCodeInAutoLockL");
    iShowingSecCodeQuery = EFalse;
    TSecUi::UnInitializeLib( );
    CleanupStack::PopAndDestroy( handler );
    }

// ---------------------------------------------------------------------------
// Get autolock timeout (in seconds)
// ---------------------------------------------------------------------------
TInt CLockAppDevicelockControl::GetAutoLockTimeout( )
    {
    TInt timeoutInMinutes = 0;
    iCRAutoLockTime->GetValue( timeoutInMinutes );
    return timeoutInMinutes * 60;
    }

// ----------------------------------------------------------
// Starts monitoring user activity
// ----------------------------------------------------------
void CLockAppDevicelockControl::StartActivityMonitoringL( )
    {
    __ASSERT_DEBUG( iActivityManager, DoPanic(ELockIllegalState));
    if ( iActivityManager && !iActivityManager->IsActive() )
        {
        TInt value = GetAutoLockTimeout( );
        INFO_1( "CLockAppDevicelockControl::StartActivityMonitoringL - %d sec", value);
        if ( value )
            {
            iActivityManager->Start( value,
                    TCallBack( HandleInactiveEventL, this ),
                    TCallBack( HandleActiveEventL, this ) );
            }
        else
            {
            iActivityManager->Start( KAutoDeviceLockOff,
                    TCallBack(HandleInactiveEventL, this ),
                    TCallBack(HandleActiveEventL, this ) );
            }
        }
    }

// ----------------------------------------------------------
// Gets autolock period and starts monitoring user activity
// ----------------------------------------------------------
void CLockAppDevicelockControl::ResetInactivityTimeout( )
    {
    __ASSERT_DEBUG( iActivityManager, DoPanic(ELockIllegalState));
    if ( iActivityManager )
        {
        TInt value = GetAutoLockTimeout( );
        INFO_1( "CLockAppDevicelockControl::ResetInactivityTimeout - %d sec", value);
        if ( value )
            {
            iActivityManager->SetInactivityTimeout( value );
            }
        else
            {
            iActivityManager->SetInactivityTimeout( KAutoDeviceLockOff );
            }
        }
    }

// ----------------------------------------------------------
// Stop monitoring user activity.
// ----------------------------------------------------------
void CLockAppDevicelockControl::StopActivityMonitoring( )
    {
    __ASSERT_DEBUG( iActivityManager, DoPanic(ELockIllegalState));
    if ( iActivityManager )
        {
        iActivityManager->Cancel( );
        }
    }

// ----------------------------------------------------------
// Handle Active event. Called by ActivityManager
// ----------------------------------------------------------
TInt CLockAppDevicelockControl::HandleActiveEventL(TAny* /*aPtr*/)
    {
    return KErrNone;
    }

// ----------------------------------------------------------
// Handles InActive event. Called by ActivityManager
// ----------------------------------------------------------
TInt CLockAppDevicelockControl::HandleInactiveEventL(TAny* aPtr )
    {
    CLockAppDevicelockControl* devicelock = STATIC_CAST(CLockAppDevicelockControl*, aPtr);
    if ( devicelock->GetAutoLockTimeout( ) )
        {
        devicelock->iStateControl.EnableDevicelockL( EDevicelockTimer );
        }
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Set custom status pane visible
// ---------------------------------------------------------------------------
void CLockAppDevicelockControl::ShowStatusPane( const TBool aVisible )
    {
    CEikStatusPane* statuspane = iAvkonAppUi->StatusPane();
    if ( statuspane )
        {
        statuspane->MakeVisible( aVisible );
        }
    }

// ---------------------------------------------------------------------------
// Handle UI commands received from the child controls
// ---------------------------------------------------------------------------
void CLockAppDevicelockControl::ProcessCommandL(TInt aCommandId)
    {
    INFO_1("CLockAppDevicelockControl::ProcessCommandL : %d ", aCommandId );
    }

TInt CLockAppDevicelockControl::CountComponentControls() const
    {
    return 2;
    }

CCoeControl* CLockAppDevicelockControl::ComponentControl(TInt aIndex ) const
    {
    switch ( aIndex )
        {
        case 0:
            return iCba;
        case 1:
            return iContainer;
        default:
            return NULL;
        }
    }

// ---------------------------------------------------------------------------
// Notification if layout changes.
// ---------------------------------------------------------------------------
void CLockAppDevicelockControl::HandleResourceChange(TInt aType )
    {
    if ( aType == KEikDynamicLayoutVariantSwitch && iCba )
        {
        TRect screenRect;
        AknLayoutUtils::LayoutMetricsRect( AknLayoutUtils::EScreen, screenRect );
        iCba->SetBoundingRect( screenRect );
        }
    CCoeControl::HandleResourceChange( aType );
    }

// END OF FILE
