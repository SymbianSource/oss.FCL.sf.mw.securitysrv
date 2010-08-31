/*
 * Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 2.1 of the License.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, 
 * see "http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html/".
 *
 * Description:
 *
 */

#include <QCoreApplication>
#include <QKeyEvent>
#include <QEvent>
#include <QTimer>
#include <QMessageBox>
#include <QSymbianEvent>
#include <QMainWindow>
#include <qvaluespacesubscriber.h>
#include <qvaluespacepublisher.h>

QTM_USE_NAMESPACE

#include <HbIndicator>
#include <hbdevicemessagebox.h>

#include <lockappclientserver.h>

#include "Autolock.h"
#include <xqserviceutil.h>
#include "autolockuseractivityservice.h"
#include <settingsinternalcrkeys.h>     // CenRep keys
#include <w32std.h>
#include <eikenv.h>
#include <aknsoundsystem.h>

#include <secuisecuritysettings.h>
#include <secui.h>
#include <secuisecurityhandler.h>
#include <etelmm.h>
#include <rmmcustomapi.h>
#include <keylockpolicyapi.h>
#include <qvaluespacesubscriber.h>

#include <hwrmlightdomaincrkeys.h>
#include <ProfileEngineSDKCRKeys.h>
#include <e32property.h>
#include <coreapplicationuisdomainpskeys.h>
#include "../PubSub/securityuisprivatepskeys.h"
#include <avkondomainpskeys.h>
#include <hwrmdomainpskeys.h>
#include <ctsydomainpskeys.h>

#include <hbdevicedialog.h>
#include <power_save_display_mode.h>

const TInt KPhoneIndex(0);
const TInt KTriesToConnectServer(2);
const TInt KTimeBeforeRetryingServerConnection(50000);
#define ESecUiTypeLock                  0x00100000
_LIT( KMmTsyModuleName, "PhoneTsy");

// Constant definitions to control screensaver view types
static const char *KSnsrViewTypeKey = "view_type";
enum TSnsrViewType
    {
    ESnsrViewTypeInitial = 0, // default initial state, depends on the configuration
    ESnsrViewTypeActive,
    ESnsrViewTypeStandby,
    ESnsrViewTypeDisabled,
    };
static const char *KSnsrCmdUnlock = "unlock";
static const char *KSnsrCmdSwitchLights = "switch_lights";
static const char *KSnsrCmdSwitchLowPower = "switch_low_power";

Autolock::Autolock(QWidget *parent, Qt::WFlags f) :
    QWidget(parent, f), mService(NULL)
    {
    RDEBUG("start autolock", 0);

    // The very first thing is to define the properties, so that others can use them.
    TSecurityPolicy readPolicy(ECapabilityReadDeviceData);
    TSecurityPolicy writePolicy(ECapabilityWriteDeviceData);
    TInt ret = RProperty::Define(KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery, RProperty::EInt, readPolicy, writePolicy);
    RDEBUG("defined KSecurityUIsSecUIOriginatedQuery", ret);
    ret = RProperty::Define(KPSUidSecurityUIs, KSecurityUIsQueryRequestCancel, RProperty::EInt, readPolicy, writePolicy);
    RDEBUG("defined KSecurityUIsQueryRequestCancel", ret);
    _LIT_SECURITY_POLICY_PASS( KReadPolicy);
    _LIT_SECURITY_POLICY_C1(KWritePolicy, ECapabilityWriteDeviceData);
    ret = RProperty::Define(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, RProperty::EInt, KReadPolicy, KWritePolicy);
    RDEBUG("defined KCoreAppUIsAutolockStatus", ret);

    // This is important: we set the status through a property
    TInt autolockState;
    ret = RProperty::Get(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, autolockState);
    RDEBUG("Get KCoreAppUIsAutolockStatus", ret);
    RDEBUG("autolockState", autolockState);
    if (autolockState == EAutolockStatusUninitialized)
        {
        autolockState = EAutolockOff; // not-initialized means taht the unlock-query hasn't been displayed. Therefore the device should not stay locked.
        }
    ret = RProperty::Set(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, autolockState); // this might re-set it. That's not bad. It will re-notify all listeners.
    RDEBUG("Set KCoreAppUIsAutolockStatus", ret);

    ret = RProperty::Define(KPSUidAvkonDomain, KAknKeyguardStatus, RProperty::EInt, TSecurityPolicy(TSecurityPolicy::EAlwaysPass), KWritePolicy);
    RDEBUG("defined KAknKeyguardStatus", ret);

    ret = RProperty::Define(KPSUidSecurityUIs, KSecurityUIsLockInitiatorUID, RProperty::EInt, TSecurityPolicy(TSecurityPolicy::EAlwaysPass), writePolicy);
    RDEBUG("defined KSecurityUIsLockInitiatorUID", ret);

    ret = RProperty::Define(KPSUidSecurityUIs, KSecurityUIsLockInitiatorTimeHigh, RProperty::EInt, readPolicy, writePolicy);
    RDEBUG("defined KSecurityUIsLockInitiatorTimeHigh", ret);
    ret = RProperty::Define(KPSUidSecurityUIs, KSecurityUIsLockInitiatorTimeLow, RProperty::EInt, readPolicy, writePolicy);
    RDEBUG("defined KSecurityUIsLockInitiatorTimeLow", ret);

    ret = RProperty::Define(KPSUidSecurityUIs, KSecurityUIsLights, RProperty::EInt, TSecurityPolicy(TSecurityPolicy::EAlwaysPass), TSecurityPolicy(TSecurityPolicy::EAlwaysPass));
    RDEBUG("defined KAknKeyguardStatus", ret);

    ret = RProperty::Define(KPSUidSecurityUIs, KSecurityUIsDismissDialog, RProperty::EInt, TSecurityPolicy(TSecurityPolicy::EAlwaysPass), TSecurityPolicy(
            TSecurityPolicy::EAlwaysPass));
    RDEBUG("defined KSecurityUIsDismissDialog", ret);

    mService = new AutolockService(this);

    /* Adjust the palette */
#if defined(Q_WS_S60)
    // this is defined.
    RDEBUG( "Q_WS_S60", 1 );
    QPalette p = qApp->palette();
    QColor color(192,192,192);
    QColor bg(201,250,250);
    p.setColor(QPalette::Highlight, color.lighter(200));
    p.setColor(QPalette::Text, Qt::black);
    p.setColor(QPalette::Base, bg);
    p.setColor(QPalette::WindowText, Qt::black);
    p.setColor(QPalette::Window, bg);
    p.setColor(QPalette::ButtonText, Qt::black);
    p.setColor(QPalette::Button, color.lighter(150));
    p.setColor(QPalette::Link, QColor(240,40,40));

    qApp->setPalette(p);
#endif

    RDEBUG("connect", 1);

#if defined(Q_WS_X11) || defined(Q_WS_WIN)
    setFixedSize(QSize(360,640)); // nHD
#elif defined(Q_WS_S60)
    // this doesn't work well
    // showMaximized();
    showFullScreen();
#endif

    serviceKeyguard = new AutolockUserActivityService();
    serviceDevicelock = new AutolockUserActivityService();

    TInt lockValue = 0;
    TInt lightsTimeout = 0;
    CRepository* repository = NULL;
    TInt cRresult = 0;
    iLockCodeQueryInDisplay = EFalse;
    Q_UNUSED(cRresult);

		iProcessingEvent = -1;
    iLockStatusPrev = ELockNotActive;
    iLockStatus = ELockNotActive;
    QT_TRAP_THROWING(repository = CRepository::NewL(KCRUidSecuritySettings));
    cRresult = repository->Get(KSettingsAutolockStatus, lockValue);
    RDEBUG("KSettingsAutolockStatus", KSettingsAutolockStatus);
    RDEBUG("cRresult", cRresult);
    RDEBUG("lockValue", lockValue);
    iLockStatus = lockValue;
    // the settings says to lock
    delete repository;

    adjustInactivityTimers(0);

    QT_TRAP_THROWING(repository = CRepository::NewL(KCRUidProfileEngine));
    cRresult = repository->Get(KProEngActiveProfile, lightsTimeout);
    // this value is not used for now
    delete repository;

    QT_TRAP_THROWING(repository = CRepository::NewL(KCRUidLightSettings));
    cRresult = repository->Get(KDisplayLightsTimeout, lightsTimeout);
    // this value is not used for now
    delete repository;

    // subscribe to settings changes
    subscriberKSettingsAutolockStatus = new QValueSpaceSubscriber("/KCRUidSecuritySettings/KSettingsAutolockStatus", this);
    connect(subscriberKSettingsAutolockStatus, SIGNAL(contentsChanged()), this, SLOT(subscriberKSettingsAutolockStatusChanged()));
    subscriberKSettingsAutoLockTime = new QValueSpaceSubscriber("/KCRUidSecuritySettings/KSettingsAutoLockTime", this);
    connect(subscriberKSettingsAutoLockTime, SIGNAL(contentsChanged()), this, SLOT(subscriberKSettingsAutoLockTimeChanged()));
    subscriberKSettingsAutomaticKeyguardTime = new QValueSpaceSubscriber("/KCRUidSecuritySettings/KSettingsAutomaticKeyguardTime", this);
    connect(subscriberKSettingsAutomaticKeyguardTime, SIGNAL(contentsChanged()), this, SLOT(subscriberKSettingsAutomaticKeyguardTimeChanged()));
    subscriberKDisplayLightsTimeout = new QValueSpaceSubscriber("/KCRUidLightSettings/KDisplayLightsTimeout", this);
    connect(subscriberKDisplayLightsTimeout, SIGNAL(contentsChanged()), this, SLOT(subscriberKDisplayLightsTimeoutChanged()));
    subscriberKProEngActiveProfile = new QValueSpaceSubscriber("/KCRUidProfileEngine/KProEngActiveProfile", this);
    connect(subscriberKProEngActiveProfile, SIGNAL(contentsChanged()), this, SLOT(subscriberKProEngActiveProfileChanged()));

    subscriberKSecurityUIsDismissDialog = new QValueSpaceSubscriber("/KPSUidSecurityUIs/KSecurityUIsDismissDialog", this);
    connect(subscriberKSecurityUIsDismissDialog, SIGNAL(contentsChanged()), this, SLOT(subscriberKSecurityUIsDismissDialogChanged()));

    // subscribe to environment changes
    subscriberKHWRMGripStatus = new QValueSpaceSubscriber("/KPSUidHWRM/KHWRMGripStatus");
    connect(subscriberKHWRMGripStatus, SIGNAL(contentsChanged()), this, SLOT(subscriberKHWRMGripStatusChanged()));

    subscriberKAknKeyguardStatus = new QValueSpaceSubscriber("/KPSUidAvkonDomain/KAknKeyguardStatus");
    connect(subscriberKAknKeyguardStatus, SIGNAL(contentsChanged()), this, SLOT(subscriberKAknKeyguardStatusChanged()));

    subscriberKCoreAppUIsAutolockStatus = new QValueSpaceSubscriber("/KPSUidCoreApplicationUIs/KCoreAppUIsAutolockStatus");
    connect(subscriberKCoreAppUIsAutolockStatus, SIGNAL(contentsChanged()), this, SLOT(subscriberKCoreAppUIsAutolockStatusChanged()));

    subscriberKCTsyCallState = new QValueSpaceSubscriber("/KPSUidCtsyCallInformation/KCTsyCallState");
    connect(subscriberKCTsyCallState, SIGNAL(contentsChanged()), this, SLOT(subscriberKCTsyCallStateChanged()));

    // inactivity
    connect(serviceKeyguard, SIGNAL(active()), this, SLOT(activeKeyguard()));
    connect(serviceKeyguard, SIGNAL(notActive()), this, SLOT(notActiveKeyguard()));
    connect(serviceDevicelock, SIGNAL(active()), this, SLOT(activeDevicelock()));
    connect(serviceDevicelock, SIGNAL(notActive()), this, SLOT(notActiveDevicelock()));

    RWindowGroup& groupWin = CEikonEnv::Static()->RootWin();
    RDEBUG("got groupWin", 1);
    // if I want to release, I should do:   mKeyCaptureHandle = env->RootWin().CaptureKey(EKeyBackspace, 0, 0);
    mEKeyDeviceFCaptureHandle = groupWin.CaptureKey(EKeyDeviceF, 0, 0);
    RDEBUG("mEKeyDeviceFCaptureHandle", mEKeyDeviceFCaptureHandle);
    mEKeyBellCaptureHandle = groupWin.CaptureKey(EKeyBell, 0, 0);
    RDEBUG("mEKeyBellCaptureHandle", mEKeyBellCaptureHandle);
    RDEBUG("got mKeyCaptureHandle", 1);

    iSecQueryUiCreated = -1;
    iDeviceDialogCreated = EDeviceDialogUninitialized;
    RDEBUG("new iDeviceDialogCreated", iDeviceDialogCreated);
    iDeviceDialog = NULL;
    // for now, always starts unlocked. This is correct because if locked, the unlock-query (from starter) is on top
    // TryChangeStatus(iLockStatus);
    TryChangeStatus( ELockNotActive);
    lower();
    hide();

    iDeviceDialog = new HbDeviceDialog(HbDeviceDialog::NoFlags, this);
    connect(iDeviceDialog, SIGNAL(dataReceived(QVariantMap)), SLOT(handleMessageFromScreensaver(QVariantMap)));
    connect(iDeviceDialog, SIGNAL(deviceDialogClosed()), SLOT(handleScreensaverClosed()));

    // screensaver standby mode timer
    mScreensaverModeTimer = new QTimer(this);
    mScreensaverModeTimer->setInterval(5 * 1000); // milliseconds, TODO: read from cenrep
    connect(mScreensaverModeTimer, SIGNAL(timeout()), SLOT(switchScreensaverToPowerSaveMode()));
    // screensaver AMOLED low power mode support
    mScreensaverPowerSave = CPowerSaveDisplayMode::NewL();
    mScreensaverPowerSavePixelBuffer = HBufC16::NewL(360 * 640);
    mScreensaverPowerSavePixelBuffer->Des().Fill(0);
    }

Autolock::~Autolock()
    {
    RDEBUG("0", 0);
    delete mService;
    delete mScreensaverPowerSave;
    delete mScreensaverPowerSavePixelBuffer;
    RDEBUG("1", 1);
    }

void Autolock::adjustInactivityTimers(int aReason)
    {
    RDEBUG("aReason", aReason);
    TInt keyguardTime = 0;
    TInt lockTime = 0;
    CRepository* repository = NULL;
    TInt cRresult = 0;
    Q_UNUSED(cRresult);

    QT_TRAP_THROWING(repository = CRepository::NewL(KCRUidSecuritySettings));
    cRresult = repository->Get(KSettingsAutomaticKeyguardTime, keyguardTime); // in seconds
    RDEBUG("KSettingsAutomaticKeyguardTime", KSettingsAutomaticKeyguardTime);
    RDEBUG("cRresult", cRresult);
    RDEBUG("keyguardTime", keyguardTime);
    if (keyguardTime > 0 && keyguardTime < 1000)
        {
        serviceKeyguard->setInactivityPeriod(keyguardTime);
        }
    else
        {
        serviceKeyguard->setInactivityPeriod(12 * 31 * 24 * 60 * 60);
        }

    cRresult = repository->Get(KSettingsAutoLockTime, lockTime); // in minutes, handled internally as seconds
    lockTime *= 60;
    RDEBUG("KSettingsAutoLockTime", KSettingsAutoLockTime);
    RDEBUG("cRresult", cRresult);
    RDEBUG("lockTime", lockTime);
    if (lockTime == 65535 * 60) // Special setting "lock at same time as keyguard" at CpDeviceLockPluginView::GetAutoLockIndex which uses this magic number
        lockTime = keyguardTime - 2; // lock 2 seconds before before, to avoid keyguard->devicelock sequence
    if (lockTime > 60 && lockTime < 24 * 60 * 60) // lock timer can't be bigger than 1 day
        {
        serviceDevicelock->setInactivityPeriod(lockTime);
        }
    else
        {
        serviceDevicelock->setInactivityPeriod(12 * 31 * 24 * 60 * 60); // 0x1ea6e00
        }

    delete repository;
    }
void Autolock::quit()
    {
    RDEBUG("0", 0);
    qApp->quit();
    }

int Autolock::AskValidSecCode(int aReason)
    {
    RDEBUG("aReason", aReason);
    RMobilePhone iPhone; // NULL in emulator

#ifdef __WINS__1
    return KErrNone;
#endif

    TInt err(KErrGeneral);
    Q_UNUSED(err);
    TBool validCode(EFalse);
    TInt thisTry(0);
    RTelServer iTelServer;

    err = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsLights, ESecurityUIsLightsQueryOnRequest);
    RDEBUG("KSecurityUIsLights err", err);

    RMmCustomAPI iCustomPhone;
    while ((err = iTelServer.Connect()) != KErrNone && (thisTry++) <= KTriesToConnectServer)
        {
        User::After( KTimeBeforeRetryingServerConnection);
        }
    err = iTelServer.LoadPhoneModule(KMmTsyModuleName);
    RTelServer::TPhoneInfo PhoneInfo;
    RDEBUG("LoadPhoneModule err", err);
    err = iTelServer.SetExtendedErrorGranularity(RTelServer::EErrorExtended);
    RDEBUG("SetExtendedErrorGranularity err", err);
    err = iTelServer.GetPhoneInfo(KPhoneIndex, PhoneInfo);
    RDEBUG("GetPhoneInfo err", err);
    err = iPhone.Open(iTelServer, PhoneInfo.iName);
    RDEBUG("Open err", err);
    err = iCustomPhone.Open(iPhone);
    RDEBUG("Open2 err", err);

    RDEBUG("CSecurityHandler", 0);
    CSecurityHandler* handler = new (ELeave) CSecurityHandler(iPhone);
    if (aReason == ELockAppDisableDevicelock)
        {
        RDEBUG("calling AskSecCodeInAutoLockL", 0);
        iLockCodeQueryInDisplay = ETrue;
        QT_TRAP_THROWING(validCode = handler->AskSecCodeInAutoLockL()); // this returns true/false
        iLockCodeQueryInDisplay = EFalse;
        // TODO should this also do iPhone.SetLockSetting(status, lockType, lockChange); ???
        }
    else if (aReason == ELockAppEnableDevicelock)
        {
        // check whether code is really needed
        RMobilePhone::TMobilePhoneLock lockType = RMobilePhone::ELockPhoneDevice;
        RMobilePhone::TMobilePhoneLockInfoV1 lockInfo;
        RMobilePhone::TMobilePhoneLockInfoV1Pckg lockInfoPkg(lockInfo);
        RMobilePhone::TMobilePhoneLockSetting lockChange(RMobilePhone::ELockSetDisabled);
        TRequestStatus status = KRequestPending;
        TInt ret = KErrNone;
        RDEBUG("GetLockInfo", 0);
        iPhone.GetLockInfo(status, lockType, lockInfoPkg);
        RDEBUG("WaitForRequest", 0);
        User::WaitForRequest(status);
        ret = status.Int();
        RDEBUG("WaitForRequest ret", ret);
#ifdef __WINS__
        if (ret == KErrTimedOut)
            {
            ret = KErrNone;
            lockInfo.iSetting = RMobilePhone::ELockSetDisabled; // ask password only if there's no timeout.
            }
#endif
        if (ret == KErrNone)
            {
            RDEBUG("lockInfo.iSetting", lockInfo.iSetting);
            RDEBUG("RMobilePhone::ELockSetDisabled",
                    RMobilePhone::ELockSetDisabled);
            if (lockInfo.iSetting == RMobilePhone::ELockSetDisabled) // ask password only if there's no timeout
                {
                lockChange = RMobilePhone::ELockSetEnabled;
                RDEBUG("SetLockSetting lockChange", lockChange);
#define OLD_METHODx
#ifdef OLD_METHOD
                status = KRequestPending;
                RDEBUG( "SetLockSetting", 0 );
                iPhone.SetLockSetting(status, lockType, lockChange); // ask for PassPhraseRequiredL
                RDEBUG( "WaitForRequestL", 0 );
                User::WaitForRequest( status ); // TODO this waits 33 seconds in WINS and returns ffffffdf = KErrTimedOut
                ret = status.Int();
                RDEBUG( "WaitForRequestL ret", ret );
#else
                RDEBUG("! OLD_METHOD", 0);
                CWait *iWait = NULL;
                QT_TRAP_THROWING(iWait = CWait::NewL());
                iWait->SetRequestType(EMobilePhoneSetLockSetting);
                iPhone.SetLockSetting(iWait->iStatus, lockType, lockChange); // ask for PassPhraseRequiredL
                RDEBUG("WaitForRequestL", 0);
                QT_TRAP_THROWING(ret = iWait->WaitForRequestL());
                RDEBUG("WaitForRequestL ret", ret);
                if (iWait)
                    {
                    RDEBUG("IsActive", 0);
                    if (iWait->IsActive())
                        {
                        RDEBUG("CancelAsyncRequest", 0);
                        iPhone.CancelAsyncRequest(iWait->GetRequestType());
                        RDEBUG("cancelled", 0);
                        }
                    }
                delete iWait;
#endif

                RDEBUG("WaitForRequestL ret", ret);
#ifdef __WINS__
                if (ret == KErrTimedOut)
                    ret = KErrNone;
#endif
                if (ret == KErrNone)
                    validCode = 1;
                else
                    validCode = 0;
                }
            else
                {
                RDEBUG("RMobilePhone::ELockSetEnabled", RMobilePhone::ELockSetEnabled);
                RDEBUG("lockInfo.iSetting = RMobilePhone::ELockSetEnabled", RMobilePhone::ELockSetEnabled);
                validCode = 0x20;
                }
            }
        else
            {
            RDEBUG("Error: ret", ret);
            validCode = 0x21;
            }

        /* Alternative way to ask for password
         RMobilePhone::TMobilePhoneSecurityEvent iEvent;
         TInt result = KErrNone;
         iEvent = RMobilePhone::EPhonePasswordRequired;
         RDEBUG( "calling HandleEventL", 0 );
         handler->HandleEventL(iEvent, result); // this is supposed to wait
         validCode = false;
         if(result)
         validCode = true;
         RDEBUG( "result", result );
         */
        }
    RDEBUG("validCode", validCode);
    if (validCode > 0)
        return KErrNone;

    // no valid code -> switch off the lights
    err = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsLights, ESecurityUIsLightsLockOffRequest);
    RDEBUG("KSecurityUIsLights err", err);
    return KErrCancel;
    }

void Autolock::handleAnswerDelivered()
    {
    RDEBUG("0", 0);
    // quit();

    }

void Autolock::setLabelNumber(QString label, QString number)
    {
    RDEBUG("0", 0);
    }

void Autolock::DebugRequest(int aReason)
    {
    switch (aReason)
        {
        case ELockAppEnableKeyguard:
            RDEBUG("ELockAppEnableKeyguard", aReason)
            ;
            break;
        case ELockAppDisableKeyguard:
            RDEBUG("ELockAppDisableKeyguard", aReason)
            ;
            break;
        case ELockAppEnableDevicelock:
            RDEBUG("ELockAppEnableDevicelock", aReason)
            ;
            break;
        case ELockAppDisableDevicelock:
            RDEBUG("ELockAppDisableDevicelock", aReason)
            ;
            break;
        case ELockAppOfferKeyguard:
            RDEBUG("ELockAppOfferKeyguard", aReason)
            ;
            break;
        case ELockAppOfferDevicelock:
            RDEBUG("ELockAppOfferDevicelock", aReason)
            ;
            break;
        case ELockAppShowKeysLockedNote:
            RDEBUG("ELockAppShowKeysLockedNote", aReason)
            ;
            break;
        default:
            RDEBUG("default", aReason)
            ;
            break;
        }
    }
void Autolock::DebugStatus(int aReason)
    {
    switch (aReason)
        {
        case ELockNotActive:
            RDEBUG("ELockNotActive", aReason)
            ;
            break;
        case EKeyguardActive:
            RDEBUG("EKeyguardActive", aReason)
            ;
            break;
        case EDevicelockActive:
            RDEBUG("EDevicelockActive", aReason)
            ;
            break;
        default:
            RDEBUG("default", aReason)
            ;
            break;
        }
    }
void Autolock::DebugError(int aReason)
    {
    switch (aReason)
        {
        case KErrNone:
            RDEBUG("KErrNone", aReason)
            ;
            break;
        case KErrPermissionDenied: // caller doesn't have enough capabilities
            RDEBUG("KErrPermissionDenied", aReason)
            ;
            break;
        case KErrAlreadyExists: // this particular lock is already enabled
            RDEBUG("KErrAlreadyExists", aReason)
            ;
            break;
        case KErrInUse: // the dialog is already shown
            RDEBUG("KErrInUse", aReason)
            ;
            break;
        default:
            RDEBUG("default", aReason)
            ;
            break;
        }
    }

int Autolock::CheckIfLegal(int aReason)
    {
    RDEBUG("aReason", aReason);
    // check whether a dialog is already displayed. This is to prevent timeout-lock and timeout-keyguard activated on top of a PIN query, in particular at boot-up (TODO check for Starter)
    TInt secUiOriginatedQuery(ESecurityUIsSecUIOriginatedUninitialized);
    TInt err = RProperty::Get(KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery, secUiOriginatedQuery);
    RDEBUG("err", err);
    RDEBUG("secUiOriginatedQuery", secUiOriginatedQuery);
    switch (aReason)
        {
        case ELockAppEnableKeyguard:
            {
            RDEBUG("ELockAppEnableKeyguard iLockStatus", iLockStatus);
            if (secUiOriginatedQuery != ESecurityUIsSecUIOriginatedUninitialized)
                return KErrInUse; // PIN on top. Don't keyguard
            switch (iLockStatus)
                {
                case ELockNotActive:
                    if (!CKeyLockPolicyApi::KeyguardAllowed())
                        return KErrPermissionDenied;
                    else
                        return KErrNone;
                case EKeyguardActive:
                    return KErrAlreadyExists;
                case EDevicelockActive:
                    return KErrPermissionDenied;
                }
            }
            break;
        case ELockAppDisableKeyguard:
            {
            RDEBUG("ELockAppDisableKeyguard iLockStatus", iLockStatus);
            // no matter whether PIN is displayed
            switch (iLockStatus)
                {
                case ELockNotActive:
                    return KErrAlreadyExists;
                case EKeyguardActive:
                    return KErrNone;
                case EDevicelockActive:
                    return KErrPermissionDenied;
                }
            }
            break;
        case ELockAppEnableDevicelock:
            {
            RDEBUG("ELockAppEnableDevicelock iLockStatus", iLockStatus);
            if (secUiOriginatedQuery != ESecurityUIsSecUIOriginatedUninitialized && secUiOriginatedQuery != ESecurityUIsSystemLockOriginated)
                return KErrInUse; // PIN on top. Don't devicelock
            switch (iLockStatus)
                {
                case ELockNotActive:
                    return KErrNone;
                case EKeyguardActive:
                    return KErrNone;
                case EDevicelockActive:
                    return KErrAlreadyExists;
                }
            }
            break;
        case ELockAppDisableDevicelock:
            {
            RDEBUG("ELockAppDisableDevicelock iLockStatus", iLockStatus);
            if (iLockCodeQueryInDisplay)
                {
                // PIN on top and trying to display unlock-code. This is valid
                return KErrAlreadyExists;
                }
            switch (iLockStatus)
                {
                case ELockNotActive:
                    return KErrAlreadyExists;
                case EKeyguardActive:
                    return KErrPermissionDenied;
                case EDevicelockActive:
                    return KErrNone;
                }
            }
            break;
        case ELockAppOfferKeyguard:
            {
            RDEBUG("ELockAppOfferKeyguard iLockStatus", iLockStatus);
            if (secUiOriginatedQuery != ESecurityUIsSecUIOriginatedUninitialized && secUiOriginatedQuery != ESecurityUIsSystemLockOriginated)
                return KErrInUse; // PIN on top. Don't offer
            switch (iLockStatus)
                {
                case ELockNotActive:
                    return KErrNone;
                case EKeyguardActive:
                    return KErrPermissionDenied;
                case EDevicelockActive:
                    return KErrPermissionDenied;
                }
            }
            break;
        case ELockAppOfferDevicelock:
            {
            RDEBUG("ELockAppOfferDevicelock iLockStatus", iLockStatus);
            if (secUiOriginatedQuery != ESecurityUIsSecUIOriginatedUninitialized && secUiOriginatedQuery != ESecurityUIsSystemLockOriginated)
                return KErrInUse; // PIN on top. Don't offer
            switch (iLockStatus)
                {
                case ELockNotActive:
                    return KErrNone;
                case EKeyguardActive:
                    return KErrNone;
                case EDevicelockActive:
                    return KErrAlreadyExists;
                }
            }
            break;
        case ELockAppShowKeysLockedNote:
            {
            RDEBUG("ELockAppShowKeysLockedNote iLockStatus", iLockStatus);
            if (secUiOriginatedQuery != ESecurityUIsSecUIOriginatedUninitialized && secUiOriginatedQuery != ESecurityUIsSystemLockOriginated)
                return KErrInUse; // PIN on top. Don't display
            switch (iLockStatus)
                {
                case ELockNotActive:
                    return KErrPermissionDenied;
                case EKeyguardActive:
                    return KErrNone;
                case EDevicelockActive: // TODO confirm this. CMMKeyBearer::ReceivedKeyEvent expects KErrNone if keypad is locked
                    return KErrPermissionDenied;
                }
            }
            break;
        default:
            {
            RDEBUG("default iLockStatus", iLockStatus);
            return KErrPermissionDenied;
            }
            // break;
        } // switch
    return KErrPermissionDenied;
    }

int Autolock::publishStatus(int aReason)
    {
    RDEBUG("aReason", aReason);
    Q_UNUSED(aReason);
    TInt err = KErrNone;
    Q_UNUSED(err);
    // can't use include file because it's private file. However it gives permissions
    const TUid KCRUidCoreApplicationUIsSysAp =
        {
        0x101F8765
        };
    const TUint32 KSysApKeyguardActive = 0x00000001;
    CRepository* repositoryDevicelock = NULL;
    CRepository* repositoryKeyguard = NULL;
    QT_TRAP_THROWING(repositoryDevicelock = CRepository::NewL(KCRUidSecuritySettings));
    QT_TRAP_THROWING(repositoryKeyguard = CRepository::NewL(KCRUidCoreApplicationUIsSysAp));
    TInt cRresult = KErrNone;
    Q_UNUSED(cRresult);
    if (1 == 1) // this is a quick way to disable this functionality, for testing
        {
        RDEBUG("publishing", aReason);
        if (aReason == ELockNotActive)
            {
            err = RProperty::Set(KPSUidAvkonDomain, KAknKeyguardStatus, EKeyguardNotActive);
            RDEBUG("err", err);
            err = RProperty::Set(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, EAutolockOff);
            RDEBUG("err", err);
            err = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsLights, ESecurityUIsLightsLockOnRequest); // lights are required
            RDEBUG("KSecurityUIsLights err", err);

            // cRresult = repositoryDevicelock->Set(KSettingsAutolockStatus, 0);    // the settings remains. Only ISA changes, as well as the P&S
            cRresult = repositoryKeyguard->Set(KSysApKeyguardActive, 0);
            RDEBUG("cRresult", cRresult);
            }
        if (1 == 1) // this is a quick way to disable this functionality, for testing
            {
            if (aReason == EKeyguardActive)
                {
                err = RProperty::Set(KPSUidAvkonDomain, KAknKeyguardStatus, EKeyguardLocked);
                RDEBUG("KAknKeyguardStatus err", err);
                err = RProperty::Set(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, EAutolockOff);
                RDEBUG("KCoreAppUIsAutolockStatus err", err);
                err = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsLights, ESecurityUIsLightsLockOffRequest); // same for keyguard and devicelock
                RDEBUG("KSecurityUIsLights err", err);
                // cRresult = repositoryDevicelock->Set(KSettingsAutolockStatus, 0);
                cRresult = repositoryKeyguard->Set(KSysApKeyguardActive, 1);
                RDEBUG("cRresult", cRresult);
                }
            else if (aReason >= EDevicelockActive)
                {
                err = RProperty::Set(KPSUidAvkonDomain, KAknKeyguardStatus, EKeyguardAutolockEmulation); // Other candidates might be: EKeyguardLocked and EKeyguardNotActive
                RDEBUG("KAknKeyguardStatus err", err);
                err = RProperty::Set(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, EManualLocked);
                RDEBUG("KCoreAppUIsAutolockStatus err", err);
                err = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsLights, ESecurityUIsLightsLockOffRequest);
                RDEBUG(" KSecurityUIsLights err", err);
                // cRresult = repositoryDevicelock->Set(KSettingsAutolockStatus, 1);
                cRresult = repositoryKeyguard->Set(KSysApKeyguardActive, 0); // keyguard disabled, so that user can type and DeviceF can be captured
                RDEBUG("cRresult", cRresult);
                }
            }
        }
    delete repositoryDevicelock;
    delete repositoryKeyguard;
    // this is the real point where everything is done.
    iLockStatusPrev = iLockStatus;
    iLockStatus = aReason;
    RDEBUG("setting iLockStatus", iLockStatus);
    return KErrNone;
    }
/********************/
int Autolock::showNoteIfRequested(int aReason)
    {
    RDEBUG("aReason", aReason);
    // The notes are displayed. This is useful only for the API withNote, and ELockAppShowKeysLockedNote
    // However, it has some inconvenients:
    // - the screensaver already displays/hides
    // - the notes are shown behing the icon because they have lower priorities
    // - they are annoying
    // TODO translate
    if (aReason == ELockNotActive)
        {
        if (iShowKeyguardNote == 1)
            {
            HbDeviceMessageBox::information("Keyguard deactivated");
            }
        }
    else if (aReason == EKeyguardActive)
        {
        if (iShowKeyguardNote == 1)
            {
            HbDeviceMessageBox::information("Keyguard activated");
            // this already waits a bit because the lock-icon takes some time to display. So the note is visible before the lock-icon.
            }
        }
    else if (aReason >= EDevicelockActive) // this doesn't happen, but we are prepared nevertheless
        {
        if (iShowKeyguardNote == 1)
            {
            HbDeviceMessageBox::information("Devicelock activated");
            }
        }
    return KErrNone;
    }
/***********************/
int Autolock::TryChangeStatus(int aReason)
    {
    RDEBUG("aReason", aReason);
    int ret = aReason;
    int errorInProcess = KErrNone;
    DebugRequest(ret);

    // this will cancel any previous dialog, i.e. PIN query, or any previous code-request.
    // not sure about the screensaver, but nevertheless will be cancelled when the status is changed.
    TInt err = KErrNone;
    Q_UNUSED(err);
    RDEBUG("Not setting KSecurityUIsDismissDialog ESecurityUIsDismissDialogOn", ESecurityUIsDismissDialogOn);
    //// err = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsDismissDialog, ESecurityUIsDismissDialogOn);
    RDEBUG("err", err);
    switch (ret)
        {
        case ELockAppEnableKeyguard: // 1
            {
            errorInProcess = CheckIfLegal(ret);
            DebugError(errorInProcess);
            if (errorInProcess == KErrNone)
                {
                showNoteIfRequested( EKeyguardActive);
                setLabelIcon(EKeyguardActive);
                updateIndicator(EKeyguardActive);
                publishStatus(EKeyguardActive);
                setLockDialog(aReason, 1);
                }
            }
            break;
        case ELockAppDisableKeyguard: // 2
            {
            errorInProcess = CheckIfLegal(ret);
            DebugError(errorInProcess);
            if (errorInProcess == KErrNone)
                {
                showNoteIfRequested( ELockNotActive);
                setLabelIcon(ELockNotActive);
                updateIndicator(ELockNotActive);
                publishStatus(ELockNotActive);
                setLockDialog(aReason, 0);
                }
            }
            break;
        case ELockAppEnableDevicelock: // 3
            {
            errorInProcess = CheckIfLegal(ret);
            DebugError(errorInProcess);
            if (errorInProcess == KErrNone)
                {
                if (!callerHasECapabilityWriteDeviceData) // check permissions for calling process, because doesn't AskValidSecCode
                    errorInProcess = KErrPermissionDenied;
                DebugError(errorInProcess);
                }
            if (errorInProcess == KErrNone)
                {
                updateIndicator( EDevicelockActive);
                publishStatus(EDevicelockActive);
                setLabelIcon(EDevicelockActive);
                setLockDialog(aReason, 1);
                }
            // aParam1 is aReason : EDevicelockManual, EDevicelockRemote
            // this never shows a note
            }
            break;
        case ELockAppDisableDevicelock: // 4
            {
            errorInProcess = CheckIfLegal(ret);
            DebugError(errorInProcess);
            if (errorInProcess == KErrNone)
                {
                if (!callerHasECapabilityWriteDeviceData) // check permissions for calling process, because doesn't AskValidSecCode
                    errorInProcess = KErrPermissionDenied;
                DebugError(errorInProcess);
                }
            if (errorInProcess == KErrNone)
                {
                setLockDialog(aReason, 0); // hide temporarilly because HbDeviceMessageBox doesn't get in top of the Lock-icon. Thus, dismiss it.
                // do I need to enable touch? it seems to work without it
                // in any case, lights are needed
                err = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsLights, ESecurityUIsLightsQueryOnRequest);
                RDEBUG("calling HbDeviceMessageBox::question", 0);
                bool value = true;
                // not sure whether this question is really needed. The UI doesn't say anything, so I remove it for now.
                // HbDeviceMessageBox::question("Disable Lock?");   // this doesn't block other events, so after return everything might be messed up.
                RDEBUG("value", value);
                if (!value)
                    errorInProcess = KErrCancel;
                }
            if (errorInProcess == KErrNone)
                {
                setLockDialog(aReason, 0); // hide temporarilly because AskValidSecCode doesn't get in top of the Lock-icon. Thus, dismiss it.
                RDEBUG("calling AskValidSecCode", 0);
                errorInProcess = AskValidSecCode(ELockAppDisableDevicelock);
                RDEBUG("errorInProcess", errorInProcess);
                }
            if (errorInProcess == KErrNone)
                {
                setLabelIcon( ELockNotActive);
                updateIndicator(ELockNotActive);
                setLockDialog(aReason, 0);
                publishStatus(ELockNotActive);
                }
            if (errorInProcess != KErrNone)
                { // re-lock. For example, if password is wrong
                if (iLockStatus >= EDevicelockActive) // this skips the case "unlocking although it wasn't locked"
                    setLockDialog(ELockAppEnableDevicelock, 1);
                }
            // this never shows a note
            }
            break;
        case ELockAppOfferKeyguard: // 5
            {
            errorInProcess = CheckIfLegal(ret);
            DebugError(errorInProcess);
            if (errorInProcess == KErrNone)
                {
                // no need to dismiss screensaver, because it's not active
                // what about any PIN-query, i.e. from settings ? In this case, the query will show after the PIN query is finished. somehow this is good because PIN is more important
                bool value = HbDeviceMessageBox::question("Enable Keyguard?"); // this doesn't block other events, so after return everything might be messed up.
                // what about a nice icon?
                RDEBUG("value", value);
                if (!value)
                    errorInProcess = KErrCancel;
                }
            if (errorInProcess == KErrNone)
                {
                errorInProcess = TryChangeStatus(ELockAppEnableKeyguard);
                }
            // this never shows a note
            }
            break;
        case ELockAppOfferDevicelock: // 6
            {
            errorInProcess = CheckIfLegal(ret);
            DebugError(errorInProcess);
            if (errorInProcess == KErrNone)
                {
                setLockDialog(aReason, 0); // hide temporarilly because AskValidSecCode doesn't get in top of the Lock-icon. Thus, dismiss it.
                errorInProcess = AskValidSecCode(ELockAppEnableDevicelock);
                }
            if (errorInProcess == KErrNone)
                {
                RDEBUG("ELockAppOfferDevicelock calling ELockAppEnableDevicelock", ELockAppEnableDevicelock);
                errorInProcess = TryChangeStatus(ELockAppEnableDevicelock);
                }
            // this never shows a note. Perhaps ELockAppEnableDevicelock does.
            }
            break;
        case ELockAppShowKeysLockedNote: // 7
            {
            errorInProcess = CheckIfLegal(ret); // it will not be legal if the keyguard is not enabled and the devicelock is not enabled.
            DebugError(errorInProcess);
            if (errorInProcess == KErrNone)
                {
                iShowKeyguardNote = 1; // this is not sent as parameter, so we need to fake it: ON
                showNoteIfRequested( EKeyguardActive);
                }
            }
            break;
        default:
            {
            RDEBUG("default", ret);
            errorInProcess = KErrNotSupported;
            }
            break;

        }
    return errorInProcess;
    }
/**************************/
int Autolock::setLockDialog(int aReason, int status)
    {
    RDEBUG("aReason", aReason);
    RDEBUG("status", status);
    RDEBUG("iDeviceDialogCreated", iDeviceDialogCreated);
    TInt secUiOriginatedQuery(ESecurityUIsSecUIOriginatedUninitialized);
    TInt err = RProperty::Get(KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery, secUiOriginatedQuery);
    RDEBUG("err", err);
    RDEBUG("secUiOriginatedQuery", secUiOriginatedQuery);

    RWindowGroup& groupWin = CEikonEnv::Static()->RootWin();

    if (status == 0) // hide
        {
        // aReason is not important here, but let's check nevertheless
        RDEBUG("ReleaseContext", R_AVKON_DEFAULT_SKEY_LIST);
        static_cast<CAknAppUi*>(CEikonEnv::Static()->EikAppUi())->KeySounds()->ReleaseContext();
        RDEBUG("PopContext", 0x90);
        static_cast<CAknAppUi*>(CEikonEnv::Static()->EikAppUi())->KeySounds()->PopContext();
        RDEBUG("ReleaseContext done", 0x92);

        if (aReason != ELockAppDisableKeyguard && aReason != ELockAppDisableDevicelock)
            {
            RDEBUG("!!!!****!!!!! error. status=0 but aReason", aReason);
            }
        // secUiOriginatedQuery should be ESecurityUIsSystemLockOriginated . If not, this is not correctly setting it
        if (iDeviceDialogCreated >= EDeviceDialogCreated)
            {
            iDeviceDialogCreated = EDeviceDialogDestroyed;
            RDEBUG("new iDeviceDialogCreated", iDeviceDialogCreated);

            RDEBUG("calling switchScreensaverMode", ESnsrViewTypeDisabled);
            switchScreensaverMode( ESnsrViewTypeDisabled);

            disconnect(iDeviceDialog, SIGNAL(dataReceived(QVariantMap)), this, SLOT(handleMessageFromScreensaver(QVariantMap)));
            disconnect(iDeviceDialog, SIGNAL(deviceDialogClosed()), this, SLOT(handleScreensaverClosed()));
            RDEBUG("signal disconnected", err);
            // TODO this crashes for EDeviceDialogScreenSaver
            err = iDeviceDialog->cancel();
            RDEBUG("cancel (bool: 1= wellCancelled) err", err);
            err = iDeviceDialog->error();
            RDEBUG("err", err);
            RDEBUG("calling iDeviceDialog->waitForClosed()", 0);
            err = iDeviceDialog->waitForClosed();
            RDEBUG("cancel (bool: 1= well waitForClosed) err", err);
            err = iDeviceDialog->error();
            RDEBUG("err", err);

            TInt err = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery, ESecurityUIsSecUIOriginatedUninitialized);
            RDEBUG("reset KSecurityUIsSecUIOriginatedQuery. err", err);
            // Cancel power key and application key capturing
            groupWin.CancelCaptureKey(mPowerKeyCaptureHandle);
            groupWin.CancelCaptureKey(mApplicationKeyCaptureHandle);
            groupWin.CancelCaptureLongKey(mApplicationLongKeyCaptureHandle);
            RDEBUG("done CancelCaptureKey", 1);
            if (mEKeyYesCaptureHandle)
                {
                RDEBUG("mEKeyYesCaptureHandle", mEKeyYesCaptureHandle);
                groupWin.CancelCaptureKey(mEKeyYesCaptureHandle);
                mEKeyYesCaptureHandle = NULL;
                }
            if (mEKeyNoCaptureHandle)
                {
                RDEBUG("mEKeyNoCaptureHandle", mEKeyNoCaptureHandle);
                groupWin.CancelCaptureKey(mEKeyNoCaptureHandle);
                mEKeyNoCaptureHandle = NULL;
                }

            RDEBUG("deleting iDeviceDialog", 0);
            delete iDeviceDialog;
            RDEBUG("deleted iDeviceDialog", 1);
            iDeviceDialog = NULL;
            }
        }
    else if (status == 1) // show
        {
        RDEBUG("PushContextL", R_AVKON_DEFAULT_SKEY_LIST);
        static_cast<CAknAppUi*>(CEikonEnv::Static()->EikAppUi())->KeySounds()->PushContextL(R_AVKON_SILENT_SKEY_LIST);
        RDEBUG("BringToForeground", 0x90);
        static_cast<CAknAppUi*>(CEikonEnv::Static()->EikAppUi())->KeySounds()->BringToForeground();
        RDEBUG("LockContext", 0x91);
        static_cast<CAknAppUi*>(CEikonEnv::Static()->EikAppUi())->KeySounds()->LockContext();
        RDEBUG("PushContextL Done", 0x92);

        // secUiOriginatedQuery should be ESecurityUIsSecUIOriginatedUninitialized . If not, the validation is not correctly filtering it
        QVariantMap params;
        TBool err;
#define ESecUiTypeDeviceLock        0x00100000
#define ESecUiTypeKeyguard          0x00200000

        if (aReason == ELockAppEnableKeyguard)
            params.insert("type", ESecUiTypeKeyguard);
        else if (aReason == ELockAppEnableDevicelock)
            params.insert("type", ESecUiTypeDeviceLock);
        else
            {
            RDEBUG("error. status=1 but aReason", aReason);
            }
        // no need for title. Icon should be explicit enough
        // params.insert("title", "Locked");
        if (iDeviceDialogCreated < EDeviceDialogCreated)
            {
            if (iDeviceDialog != NULL)
                {
                RDEBUG("!!!!!!*********!!!!!!!! error: iDeviceDialog != NULL", 0);
                }
            RDEBUG("creating iDeviceDialog", 0);
            iDeviceDialog = new HbDeviceDialog(HbDeviceDialog::NoFlags, this);
            // in theory this is needed only for screensaver, not for LockIcon. But it doesn't harm
            connect(iDeviceDialog, SIGNAL(dataReceived(QVariantMap)), SLOT(handleMessageFromScreensaver(QVariantMap)));
            connect(iDeviceDialog, SIGNAL(deviceDialogClosed()), SLOT(handleScreensaverClosed()));
            iDeviceDialogCreated = EDeviceDialogCreated;
            RDEBUG("new iDeviceDialogCreated", iDeviceDialogCreated);
            }
        else
            {
            RDEBUG("raising iDeviceDialog", 0);
            // confirm that dialog is present
            err = iDeviceDialog->error();
            RDEBUG("err", err);
            iDeviceDialogCreated = EDeviceDialogRaised;
            RDEBUG("new iDeviceDialogCreated", iDeviceDialogCreated);
            }
        const QString KScreensaverDeviceDialog("com.nokia.screensaver.snsrdevicedialogplugin/1.0");
        RDEBUG("pre show", aReason);
        bool launchSuccesful = iDeviceDialog->show(KScreensaverDeviceDialog, params); // and continue processing
        RDEBUG("post show. bool launchSuccesful", launchSuccesful);
        err = iDeviceDialog->error();
        RDEBUG("iDeviceDialog->error", err);
        if (launchSuccesful) // TODO && !error ???
            {
            iDeviceDialogCreated = EDeviceDialogScreenSaver;
            RDEBUG("new iDeviceDialogCreated", iDeviceDialogCreated);
            // TODO this is needed if creating dialog for first time? switchScreensaverToActiveMode(); // this is needed in case the dialog was existing.
            // Start standy mode timer. The initial state may already be the standby
            // mode but setting screensaver again to standby doesn't hurt.
            mScreensaverModeTimer->start();
            }
        else // some err. Usually 3 (not existing)
            {
            // screensaver has failed. Probably because it's not installed. Then, try the standard lock-icon
            iDeviceDialogCreated = EDeviceDialogLockIcon;
            RDEBUG("new iDeviceDialogCreated", iDeviceDialogCreated);
            const QString KSecQueryUiDeviceDialog("com.nokia.secuinotificationdialog/1.0");
            err = iDeviceDialog->show(KSecQueryUiDeviceDialog, params); // and continue processing
            RDEBUG("post show. err", err);
            }
        err = iDeviceDialog->error();
        RDEBUG("err", err);
        // This won't be needed when screensaver is in place, as the dialogs will be different, and therefore both can be present
        // Somehow this should be handled by Orbit, but unfortunatelly they don't allow the same dialog twice
        err = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery, ESecurityUIsSecUIOriginatedUninitialized); // this could also be ESecurityUIsSystemLockOriginated
        RDEBUG("err", err);
        // Capture power and application keys while screen saver is open. Also works for LockIcon
        mPowerKeyCaptureHandle = groupWin.CaptureKey(EKeyDevice2, 0, 0);
        mApplicationKeyCaptureHandle = groupWin.CaptureKey(EKeyApplication0, 0, 0);
        mApplicationLongKeyCaptureHandle = groupWin.CaptureLongKey(EKeyApplication0, EKeyApplication0, 0, 0, 0, ELongCaptureShortEventImmediately | ELongCaptureRepeatEvents);
        mEKeyYesCaptureHandle = groupWin.CaptureKey(EKeyYes, 0, 0);
        mEKeyNoCaptureHandle = groupWin.CaptureKey(EKeyNo, 0, 0);
        RDEBUG("keys captured", err);
        }
    else
        {
        RDEBUG("unknown status", status);
        return KErrNotSupported;
        }
    return KErrNone;
    }

void Autolock::setLabelIcon(int aReason)
    {
    RDEBUG("aReason", aReason);

    if (aReason == ELockNotActive)
        {
        // setLockDialog(aReason, 0); // This is done already at TryChangeStatus
        lower();
        hide();
        }
    else if (aReason == EKeyguardActive)
        {
        // setLockDialog(aReason, 1); // This is done already at TryChangeStatus
        // this shows the Autolock Application. not needed
        }
    else if (aReason == EDevicelockActive)
        {
        }
    else
        {
        RDEBUG("error: aReason", aReason);
        }
    }

int Autolock::updateIndicator(int aReason)
    {
    RDEBUG("aReason", aReason);
    QList<QVariant> list;
    list.insert(0, 1);
    list.insert(1, "dummy");
    list.insert(2, 2);

    HbIndicator indicator;
    bool success;
    if (aReason == ELockNotActive)
        success = indicator.deactivate("com.nokia.hb.indicator.autolock.autolock_8/1.0"); // maybe it's already deactivated. Not a problem
    else if (aReason == EKeyguardActive)
        success = indicator.activate("com.nokia.hb.indicator.autolock.autolock_8/1.0");
    else if (aReason == EDevicelockActive)
        success = indicator.activate("com.nokia.hb.indicator.autolock.autolock_8/1.0"); // same. We have just 1 indicator
    else
        success = 0;
    RDEBUG("success", success);
    return success;
    }

void Autolock::activeKeyguard()
    {
    // activity while keyguarded. Nothing to do
    RDEBUG("0", 0);
    }

void Autolock::notActiveKeyguard()
    {
    // inactivity. Keyguard should be activated
    RDEBUG("0", 0);
    int ret = KErrNone;
    DebugStatus( iLockStatus);
    if (iLockStatus == ELockNotActive) // not possible if it's keyguarded, or locked
        {
        TInt callState = EPSCTsyCallStateNone;
        int err = RProperty::Get(KPSUidCtsyCallInformation, KCTsyCallState, callState);
        RDEBUG("err", err);
        RDEBUG("callState", callState);
        // if there's an active call, don't auto-keyguard (manual is still possible)
        if (callState <= EPSCTsyCallStateNone)
            {
            ret = TryChangeStatus(ELockAppEnableKeyguard);
            RDEBUG("ret", ret);
            }
        else
            {
            RDEBUG("not ELockAppEnableKeyguard because callState", callState);
            }
        }
    else
        {
        // restart screensaver. The scenario is: keyguard->screensaver->CriticalNote->lightsOn . Therefore we need lightsOff
        // this is done when keyguard can't be enabled because it's already enabled
        // Note that this happens only once, so it's fine to show the screensaver in full light.
        if (ret == KErrAlreadyExists)
            switchScreensaverMode( ESnsrViewTypeInitial);
        }
    RDEBUG("ret", ret);
    }
/* Some activity detected while the deviceLock is enabled */
void Autolock::activeDevicelock()
    {
    // nothing to do
    RDEBUG("0", 0);
    }

void Autolock::notActiveDevicelock()
    {
    // inactivity. Devicelock should be activated
    RDEBUG("0", 0);
    int ret = KErrNone;
    Q_UNUSED(ret);
    DebugStatus( iLockStatus);
    if (iLockStatus == ELockNotActive || iLockStatus == EKeyguardActive) // not possible if it's locked
        {
        // this is independent of the call status
        ret = TryChangeStatus(ELockAppEnableDevicelock);
        }
    RDEBUG("ret", ret);

    }

void Autolock::switchScreensaverMode(int mode)
    {
    RDEBUG("mode", mode);
    if (iDeviceDialog)
        {
        // maybe this can be used to tell the dialog to dismiss itself?
        RDEBUG("got iDeviceDialog. iDeviceDialogCreated", iDeviceDialogCreated);
        QVariantMap params;
        // this is for LockIcon. Doesn't harm screensaver
        // need to send again so that the existing dialog still knows what is its type
        if (iLockStatus == EKeyguardActive)
            params.insert("type", ESecUiTypeKeyguard);
        else if (iLockStatus >= EDevicelockActive)
            params.insert("type", ESecUiTypeDeviceLock);

        // this is for screensaver. Doesn't harm LockIcon
        params.insert(KSnsrViewTypeKey, mode);
        iDeviceDialog->update(params);

        if (mScreensaverModeTimer)
            {
            if (mode == ESnsrViewTypeStandby || mode == ESnsrViewTypeDisabled)
                {
                RDEBUG("stop mScreensaverModeTimer", 0);
                mScreensaverModeTimer->stop();
                }
            else
            		{
                RDEBUG("start mScreensaverModeTimer", 0);
            		mScreensaverModeTimer->start();
            		}
            }
        }
    RDEBUG("1", 1);
    }

void Autolock::switchScreensaverToPowerSaveMode()
    {
    RDEBUG("0", 0);
    switchScreensaverMode( ESnsrViewTypeStandby);
    RDEBUG("1", 1);
    }

void Autolock::handleMessageFromScreensaver(const QVariantMap &data)
    {
    RDEBUG("0", 0);
    RDEBUG("iLockStatus", iLockStatus);
    QVariantMap::const_iterator it = data.find(KSnsrCmdUnlock);
    if (it != data.end() && iLockStatus != ELockNotActive)
        {
        RDEBUG("calling handleLockSwitch", 0);
        handleLockSwitch();
        }

    it = data.find(KSnsrCmdSwitchLights);
    if (it != data.end())
        {
        int lightStatus = it.value().toInt();
        RDEBUG("switching screen lights . lightStatus", lightStatus);
        int err = RProperty::Set(KPSUidCoreApplicationUIs, KLightsSSForcedLightsOn, lightStatus);
        RDEBUG("err", err);
        }

    it = data.find(KSnsrCmdSwitchLowPower);
    if (it != data.end())
        {
        QVariantList lowPowerLimits = it.value().toList();
        if (lowPowerLimits.count() == 2)
            {
            // switch on screen power save mode when start and end lines are given
            int startRow = lowPowerLimits.at(0).toInt();
            int endRow = lowPowerLimits.at(1).toInt();
            // TODO: The pixel buffer passed to CPowerSaveDisplayMode is supposed to
            // contain the pixels shown in the active area of the screen. Currently, we
            // pass a buffer filled with 0s, and this works fine on the reference
            // hardware. However, according to display driver people, this doesn't
            // work if the display component of the device doesn't have internal memory
            // which may be the case in some of our target devices.
            TUint16 *ptr = const_cast<TUint16 *> (mScreensaverPowerSavePixelBuffer->Ptr());
            RDEBUG("switching screen power save on, number of visible rows", endRow-startRow);
            int err = mScreensaverPowerSave->Set(startRow, endRow, ptr);
            RDEBUG("err", err);
            }
        else
            {
            // any other case is interpreted as an order to switch off the power save and
            // return to the normal mode
            RDEBUG("switching screen power save off 0", 0);
            int err = mScreensaverPowerSave->Exit();
            RDEBUG("err", err);
            }
        }
    }

void Autolock::handleScreensaverClosed()
    {
    RDEBUG("0", 0);
    int err(0);
    // Screensaver (usually) cannot send anymore any commands when
    // it's being closed. Thus, we need to ensure here that screen has lights and
    // full-power mode once screensaver is closed.
    if (mScreensaverPowerSave)
        {
        RDEBUG("switching screen power save off", 0);
        err = mScreensaverPowerSave->Exit();
        RDEBUG("error=", err);
        }
    RDEBUG("switching screen lights on", 1);
    err = RProperty::Set(KPSUidCoreApplicationUIs, KLightsSSForcedLightsOn, 30);
    RDEBUG("err", err);
    }

// some key is pressed
// TODO perhaps need a way to stop switch-key while asking unlock-code?
// Not clear what to do here. dismiss query? 
bool Autolock::event(QEvent *ev)
    {
    if (ev)
        {
        int isSwitchKey = 0;
        if (ev->type() == QEvent::KeyPress)
            {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *> (ev);
            RDEBUG("KeyPress", keyEvent->key());
            RDEBUG("KeyPress nativeScanCode", keyEvent->nativeScanCode());
            RDEBUG("EStdKeyDeviceF", EStdKeyDeviceF);
            RDEBUG("EKeyDeviceF", EKeyDeviceF);
            RDEBUG("keyEvent->isAutoRepeat()", keyEvent->isAutoRepeat());
            // note that the key was EKeyApplication0 and EKeyDevice2
            // yes=0xc4 app=0xb4 no=0xa6
            if (keyEvent->nativeScanCode() == EStdKeyApplication0 || keyEvent->nativeScanCode() == EStdKeyDevice2 || keyEvent->nativeScanCode() == EStdKeyYes
                    || keyEvent->nativeScanCode() == EStdKeyNo)
                {
                // switch to active screensaver if application key or power key pressed while screensaver active
                // This key can be repeated, so that the screensaver remains as long as key is pushed
                // TODO what happens when holding Power key ?
                switchScreensaverMode( ESnsrViewTypeActive);
                }
            else if (!keyEvent->isAutoRepeat())
                {
                if (keyEvent->key() == EKeyInsert)
                    {
                    // on WINS, this is handled with the "Ins" key in the numeric keypad
                    RDEBUG("EKeyInsert", EKeyInsert);
                    // only reacts on release, not on press
                    isSwitchKey = 1;
                    }
                else if (keyEvent->key() == EKeyDeviceF)
                    {
                    // this never seem to happen. Nevertheless we are not doing anything
                    RDEBUG("EKeyDeviceF", EKeyDeviceF);
                    }
                else if (keyEvent->nativeScanCode() == EStdKeyDeviceF)
                    {
                    RDEBUG("got EStdKeyDeviceF", EStdKeyDeviceF);
                    isSwitchKey = 1;
                    }
                else if (keyEvent->nativeScanCode() == EKeyBell) // this might be sent by others, i.e. PCFW
                    {
                    RDEBUG("got EKeyBell", EKeyBell);
                    isSwitchKey = 1;
                    }
                else if (keyEvent->key() == 0x1ffffff)
                    {
                    RDEBUG("0x1ffffff", 0x1ffffff); // some unknown key is received. Nothing to do
                    }
                }
            }
        else if (ev->type() == QEvent::KeyRelease)
            {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *> (ev);
            RDEBUG("KeyRelease", keyEvent->key());
            RDEBUG("KeyRelease native", keyEvent->nativeScanCode());
            if (keyEvent->nativeScanCode() == EStdKeyDeviceF && !keyEvent->isAutoRepeat())
                {
                RDEBUG("released EStdKeyDeviceF", 1);
                // isSwitchKey=1; this should happen is   pressed  was not processed (hint: if it is/was in background)
                }
            }
        RDEBUG("isSwitchKey", isSwitchKey);
        if (isSwitchKey)
            {
            handleLockSwitch();
            RDEBUG("0", 0);
            } // isSwitchKey
        } // ev
    // Process if not done before. For example, redraw or quit
    TBool widgetEventSuccesful = QWidget::event(ev);
    RDEBUG("9", 9);
    return widgetEventSuccesful;
    }

int Autolock::handleLockSwitch()
    {
    RDEBUG("0", 0);
    int ret = KErrNone;
   	RDEBUG("iProcessingEvent", iProcessingEvent);
   	// iProcessingEvent is used to avoid concurrent processing, i.e. 2 side-switch presses; because it needs time to create (and dismiss) the dialog
   	// However this disallows keyguard during unlock-quey.
   	if(iProcessingEvent < 1)
   		{
	    iProcessingEvent = 1;
	    Q_UNUSED(ret);
	    DebugStatus( iLockStatus);
	    if (iLockStatus == ELockNotActive)
	        {
	        iShowKeyguardNote = 0; // no note on enable keyguard
	        callerHasECapabilityWriteDeviceData = 1;
	        ret = TryChangeStatus(ELockAppEnableKeyguard); // this should not ask confirmation
	        }
	    else if (iLockStatus == EKeyguardActive)
	        {
	        iShowKeyguardNote = 0; // no note on disable keyguard
	        callerHasECapabilityWriteDeviceData = 1;
	        ret = TryChangeStatus(ELockAppDisableKeyguard);
	        }
	    else if (iLockStatus == EDevicelockActive)
	        {
	        callerHasECapabilityWriteDeviceData = 1;
	        ret = TryChangeStatus(ELockAppDisableDevicelock);
	        }
	    else
	        {
	        RDEBUG("unknown iLockStatus", iLockStatus);
	        }
    	iProcessingEvent = 0;
	    }
    RDEBUG("ret", ret);
    return ret;
    }

bool Autolock::eventFilter(QObject *o, QEvent *ev)
    {
    // this never happens
    RDEBUG("0", 0);
    return QWidget::eventFilter(o, ev);
    }
// some setting changed
void Autolock::subscriberKSettingsAutolockStatusChanged()
    {
    RDEBUG("0", 0);
    QVariant v = subscriberKSettingsAutolockStatus->value("/KCRUidSecuritySettings/KSettingsAutolockStatus");
    adjustInactivityTimers( KSettingsAutolockStatus);
    }
void Autolock::subscriberKSettingsAutoLockTimeChanged()
    {
    RDEBUG("0", 0);
    QVariant v = subscriberKSettingsAutoLockTime->value("/KCRUidSecuritySettings/KSettingsAutoLockTime");
    adjustInactivityTimers( KSettingsAutoLockTime);
    }
void Autolock::subscriberKSettingsAutomaticKeyguardTimeChanged()
    {
    RDEBUG("0", 0);
    QVariant v = subscriberKSettingsAutomaticKeyguardTime->value("/KCRUidSecuritySettings/KSettingsAutomaticKeyguardTime");
    adjustInactivityTimers( KSettingsAutoLockTime);
    }
void Autolock::subscriberKDisplayLightsTimeoutChanged()
    {
    RDEBUG("0", 0);
    QVariant v = subscriberKDisplayLightsTimeout->value("/KCRUidLightSettings/KDisplayLightsTimeout");
    // nothing to do
    }
void Autolock::subscriberKProEngActiveProfileChanged()
    {
    RDEBUG("0", 0);
    QVariant v = subscriberKProEngActiveProfile->value("/KCRUidProfileEngine/KProEngActiveProfile");
    // nothing to do
    }
// some environment changed
void Autolock::subscriberKAknKeyguardStatusChanged()
    {
    RDEBUG("Error only Autolock should be able to change it", 0);
    }
void Autolock::subscriberKCoreAppUIsAutolockStatusChanged()
    {
    RDEBUG("Error only Autolock should be able to change it", 0);
    }
void Autolock::subscriberKHWRMGripStatusChanged()
    {
    TInt ret = KErrNone;
    Q_UNUSED(ret);
    RDEBUG("0", 0);
    TInt aGripStatus;
    TInt err = RProperty::Get(KPSUidHWRM, KHWRMGripStatus, aGripStatus);
    RDEBUG("err", err);
    RDEBUG("value", aGripStatus);
    if (aGripStatus == EPSHWRMGripOpen)
        {
        if (iLockStatus == EKeyguardActive)
            {
            iShowKeyguardNote = 0; // no note on disable keyguard
            callerHasECapabilityWriteDeviceData = 1;
            ret = TryChangeStatus(ELockAppDisableKeyguard);
            }
        else if (iLockStatus == EDevicelockActive)
            {
            callerHasECapabilityWriteDeviceData = 1;
            ret = TryChangeStatus(ELockAppDisableDevicelock);
            }
        }
    else if (aGripStatus == EPSHWRMGripClosed)
        {
        if (iLockStatus == ELockNotActive)
            {
            iShowKeyguardNote = 0; // no note on enable keyguard
            callerHasECapabilityWriteDeviceData = 1;
            ret = TryChangeStatus(ELockAppEnableKeyguard);
            }
        }
    RDEBUG("ret", ret);
    }
/**************/
void Autolock::subscriberKCTsyCallStateChanged()
    {
    RDEBUG("0", 0);
    TInt aCallStatus;
    TInt err = RProperty::Get(KPSUidCtsyCallInformation, KCTsyCallState, aCallStatus);
    RDEBUG("err", err);
    RDEBUG("aCallStatus", aCallStatus);
    RWindowGroup& groupWin = CEikonEnv::Static()->RootWin();
    if (aCallStatus == EPSCTsyCallStateUninitialized || aCallStatus == EPSCTsyCallStateNone)
        {
        RDEBUG("call is inactive", aCallStatus);
        RDEBUG("iLockStatus", iLockStatus);
        if (iLockStatus == EKeyguardActive || iLockStatus == EDevicelockActive)
            {
            if (!mEKeyYesCaptureHandle)
                mEKeyYesCaptureHandle = groupWin.CaptureKey(EKeyYes, 0, 0);
            if (!mEKeyNoCaptureHandle)
                mEKeyNoCaptureHandle = groupWin.CaptureKey(EKeyNo, 0, 0);
            }
        }
    else
        {
        RDEBUG("call is active", aCallStatus);
        // Telephone should be on top, but nevertheless we give End/Send to them
        // This should be done only if locked, but it doesn't harm
        if (mEKeyYesCaptureHandle)
            {
            RDEBUG("mEKeyYesCaptureHandle", mEKeyYesCaptureHandle);
            groupWin.CancelCaptureKey(mEKeyYesCaptureHandle);
            mEKeyYesCaptureHandle = NULL;
            }
        if (mEKeyNoCaptureHandle)
            {
            RDEBUG("mEKeyNoCaptureHandle", mEKeyNoCaptureHandle);
            groupWin.CancelCaptureKey(mEKeyNoCaptureHandle);
            mEKeyNoCaptureHandle = NULL;
            }
        }
    RDEBUG("9", 9);
    }
// SecUiNotificationDialog::subscriberKSecurityUIsDismissDialogChanged()
// A way for Autolock to dismiss any possible PIN dialog
// ----------------------------------------------------------------------------
//
void Autolock::subscriberKSecurityUIsDismissDialogChanged()
    {
    RDEBUG("0", 0);
    TInt aDismissDialog = ESecurityUIsDismissDialogUninitialized;
    TInt err = RProperty::Get(KPSUidSecurityUIs, KSecurityUIsDismissDialog, aDismissDialog);
    RDEBUG("err", err);
    RDEBUG("aDismissDialog", aDismissDialog);
    }
// ----------AutolockService---------------

AutolockService::AutolockService(Autolock* parent) :
    XQServiceProvider(QLatin1String("com.nokia.services.Autolock.Autolock"), parent), mAutolock(parent), mAsyncReqId(-1), mAsyncAnswer(false)
    {
    RDEBUG("0", 0);
    publishAll();
    connect(this, SIGNAL(returnValueDelivered()), parent, SLOT(handleAnswerDelivered()));
    }

AutolockService::~AutolockService()
    {
    RDEBUG("0", 0);
    }

void AutolockService::complete(QString number)
    {
    RDEBUG("0", 0);
    if (mAsyncReqId == -1)
        return;
    completeRequest(mAsyncReqId, number.toInt());
    }

// got API request from lockaccessextension
int AutolockService::service(const QString& number, const QString& aParam1, const QString& aParam2)
    {
    RDEBUG("0", 0);
    TInt err = KErrNone;
    RDEBUG("number", number.toInt());
    RDEBUG("aParam1", aParam1.toInt());
    RDEBUG("aParam2", aParam2.toInt());
    mAsyncAnswer = false;
    XQRequestInfo info = requestInfo();
    QSet<int> caps = info.clientCapabilities();

    mAutolock->callerHasECapabilityWriteDeviceData = 0;
    if (caps.contains(ECapabilityWriteDeviceData))
        {
        RDEBUG("callerHasECapabilityWriteDeviceData",
                ECapabilityWriteDeviceData);
        mAutolock->callerHasECapabilityWriteDeviceData = 1;
        }

    QString label = "Autolock::service:\n";
    label += QString("number=%1\n").arg(number);

    mNumber = number;
    mAutolock->setLabelNumber(label, number);
    int ret = 0;
    ret = number.toInt();

    mAutolock->mParam1 = aParam1.toInt();
    mAutolock->iShowKeyguardNote = aParam1.toInt();
    mAutolock->mParam2 = aParam2.toInt();
    TTime myTime;
    myTime.HomeTime();
    TInt myTimeHigh = 0;
    TInt myTimeLow = 0;
    err = RProperty::Get(KPSUidSecurityUIs, KSecurityUIsLockInitiatorTimeHigh, myTimeHigh);
    RDEBUG("err", err);
    RDEBUG("myTimeHigh", myTimeHigh);
    err = RProperty::Get(KPSUidSecurityUIs, KSecurityUIsLockInitiatorTimeLow, myTimeLow);
    RDEBUG("err", err);
    RDEBUG("myTimeLow", myTimeLow);

    myTimeHigh = I64HIGH( myTime.Int64() );
    myTimeLow = I64LOW( myTime.Int64() );
    RDEBUG("myTimeHigh", myTimeHigh);
    RDEBUG("myTimeLow", myTimeLow);
    err = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsLockInitiatorTimeHigh, myTimeHigh);
    err = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsLockInitiatorTimeLow, myTimeLow);

    ret = mAutolock->TryChangeStatus(ret);
    RDEBUG("ret", ret);

    TInt myInitiatorUID = 0;
    err = RProperty::Get(KPSUidSecurityUIs, KSecurityUIsLockInitiatorUID, myInitiatorUID);
    RDEBUG("ret", ret);
    RDEBUG("myInitiatorUID", myInitiatorUID);
    myInitiatorUID = info.clientSecureId();
    err = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsLockInitiatorUID, myInitiatorUID);
    RDEBUG("ret", ret);
    RDEBUG("myInitiatorUID", myInitiatorUID);

    return ret;
    }

void AutolockService::handleClientDisconnect()
    {
    RDEBUG("0", 0);
    // Just quit service application if client ends
    mAsyncAnswer = false;
    RDEBUG("0", 0);
    // mAutolock->quit();
    }

/****************/
CWait* CWait::NewL()
    {
    CWait* self = new (ELeave) CWait();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }
void CWait::ConstructL()
    {
    CActiveScheduler::Add(this);
    }
CWait::CWait() :
    CActive(0)
    {
    }
CWait::~CWait()
    {
    Cancel();
    }
TInt CWait::WaitForRequestL()
    {
    SetActive();
    iWait.Start();
    return iStatus.Int();
    }
void CWait::RunL()
    {
    if (iWait.IsStarted())
        iWait.AsyncStop();
    }
void CWait::DoCancel()
    {
    if (iWait.IsStarted())
        iWait.AsyncStop();
    }
void CWait::SetRequestType(TInt aRequestType)
    {
    iRequestType = aRequestType;
    }
TInt CWait::GetRequestType()
    {
    return iRequestType;
    }

