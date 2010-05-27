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

// #include "xqservicelog.h"

#include <QApplication>
#include <QKeyEvent>
#include <QEvent>
#include <QLabel>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QImageReader>
#include <QTimer>
#include <QPushButton>
#include <QLineEdit>
#include <QListView>
#include <QMessageBox>
#include <QToolButton>
#include <QSymbianEvent>
#include <QMainWindow>
#include <qvaluespacesubscriber.h>
#include <qvaluespacepublisher.h>

QTM_USE_NAMESPACE

#include <HbIndicator>
#include <hbdevicemessagebox.h>

#include <lockappclientserver.h>

#include <xqsharablefile.h>

#include <QGraphicsLinearLayout>

#include <hblabel.h>

#include "AutolockSrv.h"
#include <xqserviceutil.h>

#include "autolockuseractivityservice.h"

#include <settingsinternalcrkeys.h>		// CenRep keys
#include <w32std.h>
#include <eikenv.h>

#include <qapplication.h>

#include <secuisecuritysettings.h>
#include <secui.h>
#include <secuisecurityhandler.h>
#include <etelmm.h>
#include <rmmcustomapi.h>

#include <qvaluespacesubscriber.h>

#include <hwrmlightdomaincrkeys.h>
#include <ProfileEngineSDKCRKeys.h>
#include <e32property.h>
#include <coreapplicationuisdomainpskeys.h>
#include "../../../inc/securityuisprivatepskeys.h"
#include <avkondomainpskeys.h>
#include <hwrmdomainpskeys.h>

#include <hbdevicedialog.h>

const TInt KPhoneIndex(0);
const TInt KTriesToConnectServer(2);
const TInt KTimeBeforeRetryingServerConnection(50000);

#define ESecUiTypeLock					0x00100000

_LIT( KMmTsyModuleName, "PhoneTsy");

AutolockSrv::AutolockSrv(QWidget *parent, Qt::WFlags f) :
    QWidget(parent, f), mService(NULL)
    {
    RDEBUG("start autolocksrv", 0);
    mService = new AutolockSrvService(this);

    /* Adjust the palette */
#if defined(Q_WS_S60)
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

    QPushButton *unlockButton = new QPushButton(tr("Unlock"));
    QPushButton *unguardButton = new QPushButton(tr("Unguard"));
    QPushButton *lockButton = new QPushButton(tr("Lock"));
    QPushButton *quitButton = new QPushButton(tr("Quit"));
    QPushButton *test1Button = new QPushButton(tr("Autolock 10 seconds"));
    QPushButton *test2Button = new QPushButton(tr("Autolock never"));
    QPushButton *test3Button = new QPushButton(tr("SecUiTest"));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(quit()));
    connect(lockButton, SIGNAL(clicked()), this, SLOT(lockAction()));
    connect(unlockButton, SIGNAL(clicked()), this, SLOT(unlockAction()));
    connect(unguardButton, SIGNAL(clicked()), this, SLOT(unguardAction()));
    connect(test1Button, SIGNAL(clicked()), this, SLOT(test1Action()));
    connect(test2Button, SIGNAL(clicked()), this, SLOT(test2Action()));
    connect(test3Button, SIGNAL(clicked()), this, SLOT(test3Action()));
    RDEBUG("connect", 1);

    /* there's no use for this */
    /*
     bool isService = XQServiceUtil::isService();
     
     QString t = "SERVICEAPP:\n";
     t = t + (isService ?  "    LAUNCHED AS SERVICE\n" : "    LAUNCHED NORMALLY\n");
     t = t + (XQServiceUtil::isEmbedded() ? "    EMBEDDED\n" : "    NOT EMBEDDED\n");
     
     QStringList args = QApplication::arguments();
     foreach (QString arg, args)
     {
     t += "cmdline arg=" + arg + "\n";
     }
     
     QLabel *title = new QLabel(t);
     */

    mLabel = new QLabel("");
    mNumberEdit = new QLineEdit("");

    QVBoxLayout *vl = new QVBoxLayout;
    RDEBUG("QVBoxLayout", 1);

    vl->setMargin(0);
    vl->setSpacing(0);
    RDEBUG("setSpacing", 1);

    vl->addWidget(lockButton);
    vl->addWidget(unlockButton);
    vl->addWidget(unguardButton);
    vl->addWidget(quitButton);
    vl->addWidget(test1Button);
    vl->addWidget(test2Button);
    vl->addWidget(test3Button);
    RDEBUG("added test3Button", 1);

    mLabelIcon = new QToolButton;
    mLabelIcon->setIcon(QIcon(
            ":/AutolockSrv_hbicon/qtg_large_device_lock.svg"));
    mLabelIcon->setIconSize(QSize(300, 300));

    vl->addWidget(mLabelIcon);
    RDEBUG("added mLabelIcon", 1);

    /*    vl->addItem(title);
     vl->addWidget(mLabel);
     vl->addWidget(mNumberEdit);
     */
    setLayout(vl);

#if defined(Q_WS_X11) || defined(Q_WS_WIN)
    setFixedSize(QSize(360,640)); // nHD
#elif defined(Q_WS_S60)
    // showMaximized();
    showFullScreen();
#endif
    mLabelIcon_visible = 1;
    serviceKeyguard = new AutolockUserActivityService();
    serviceDevicelock = new AutolockUserActivityService();

    TInt lockValue = 0;
    TInt lightsTimeout = 0;
    CRepository* repository = NULL;
    TInt cRresult = 0;

    iLockStatusPrev = ELockNotActive;
    iLockStatus = ELockNotActive;
    QT_TRAP_THROWING( repository = CRepository::NewL(KCRUidSecuritySettings) );
    cRresult = repository->Get(KSettingsAutolockStatus, lockValue);
    RDEBUG("KSettingsAutolockStatus", KSettingsAutolockStatus);
    RDEBUG("cRresult", cRresult);
    RDEBUG("lockValue", lockValue);
    iLockStatus = lockValue;
    // the settings says to lock
    delete repository;

    adjustInactivityTimers(0);

    QT_TRAP_THROWING( repository = CRepository::NewL(KCRUidProfileEngine) );
    cRresult = repository->Get(KProEngActiveProfile, lightsTimeout);
    // this value is not used for now
    delete repository;

    QT_TRAP_THROWING( repository = CRepository::NewL(KCRUidLightSettings) );
    cRresult = repository->Get(KDisplayLightsTimeout, lightsTimeout);
    // this value is not used for now
    delete repository;

    // subscribe to settings changes
    subscriberKSettingsAutolockStatus = new QValueSpaceSubscriber(
            "/KCRUidSecuritySettings/KSettingsAutolockStatus", this);
    connect(subscriberKSettingsAutolockStatus, SIGNAL(contentsChanged()),
            this, SLOT(subscriberKSettingsAutolockStatusChanged()));
    subscriberKSettingsAutoLockTime = new QValueSpaceSubscriber(
            "/KCRUidSecuritySettings/KSettingsAutoLockTime", this);
    connect(subscriberKSettingsAutoLockTime, SIGNAL(contentsChanged()), this,
            SLOT(subscriberKSettingsAutoLockTimeChanged()));
    subscriberKSettingsAutomaticKeyguardTime = new QValueSpaceSubscriber(
            "/KCRUidSecuritySettings/KSettingsAutomaticKeyguardTime", this);
    connect(subscriberKSettingsAutomaticKeyguardTime, SIGNAL(
            contentsChanged()), this, SLOT(
            subscriberKSettingsAutomaticKeyguardTimeChanged()));
    subscriberKDisplayLightsTimeout = new QValueSpaceSubscriber(
            "/KCRUidLightSettings/KDisplayLightsTimeout", this);
    connect(subscriberKDisplayLightsTimeout, SIGNAL(contentsChanged()), this,
            SLOT(subscriberKDisplayLightsTimeoutChanged()));
    subscriberKProEngActiveProfile = new QValueSpaceSubscriber(
            "/KCRUidProfileEngine/KProEngActiveProfile", this);
    connect(subscriberKProEngActiveProfile, SIGNAL(contentsChanged()), this,
            SLOT(subscriberKProEngActiveProfileChanged()));

    // subscribe to environment changes
    subscriberKHWRMGripStatus = new QValueSpaceSubscriber(
            "/KPSUidHWRM/KHWRMGripStatus", this);
    connect(subscriberKHWRMGripStatus, SIGNAL(contentsChanged()), this,
            SLOT(subscriberKHWRMGripStatusChanged()));

    subscriberKAknKeyguardStatus = new QValueSpaceSubscriber(
            "/KPSUidAvkonDomain/KAknKeyguardStatus", this);
    connect(subscriberKAknKeyguardStatus, SIGNAL(contentsChanged()), this,
            SLOT(subscriberKAknKeyguardStatusChanged()));

    subscriberKCoreAppUIsAutolockStatus = new QValueSpaceSubscriber(
            "/KPSUidCoreApplicationUIs/KCoreAppUIsAutolockStatus", this);
    connect(subscriberKCoreAppUIsAutolockStatus, SIGNAL(contentsChanged()), this,
            SLOT(subscriberKCoreAppUIsAutolockStatusChanged()));

		/////////////
    TSecurityPolicy readPolicy(ECapabilityReadDeviceData);
    TSecurityPolicy writePolicy(ECapabilityWriteDeviceData);
    TInt ret = RProperty::Define(KPSUidSecurityUIs,
            KSecurityUIsSecUIOriginatedQuery, RProperty::EInt, readPolicy,
            writePolicy);
    RDEBUG("defined KSecurityUIsSecUIOriginatedQuery", ret);
    ret = RProperty::Define(KPSUidSecurityUIs,
            KSecurityUIsQueryRequestCancel, RProperty::EInt, readPolicy,
            writePolicy);
    RDEBUG("defined KSecurityUIsQueryRequestCancel", ret);

    _LIT_SECURITY_POLICY_PASS( KReadPolicy);
    _LIT_SECURITY_POLICY_C1(KWritePolicy, ECapabilityWriteDeviceData);
    ret = RProperty::Define(KPSUidCoreApplicationUIs,
            KCoreAppUIsAutolockStatus, RProperty::EInt, KReadPolicy,
            KWritePolicy);
    //User::LeaveIfError( RProperty::Set(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus,	
    //               EAutolockOff));        
    RProperty::Set(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus,	
                   EAutolockOff);
    RDEBUG("defined KCoreAppUIsAutolockStatus", ret);

    ret = RProperty::Define(KPSUidAvkonDomain, KAknKeyguardStatus,
            RProperty::EInt, TSecurityPolicy(TSecurityPolicy::EAlwaysPass),
            KWritePolicy);
    RDEBUG("defined KAknKeyguardStatus", ret);

    ret = RProperty::Define(KPSUidSecurityUIs,
            KSecurityUIsLockInitiatorUID, RProperty::EInt, readPolicy,
            writePolicy);
    RDEBUG("defined KSecurityUIsLockInitiatorUID", ret);

    ret = RProperty::Define(KPSUidSecurityUIs,
            KSecurityUIsLockInitiatorTimeHigh, RProperty::EInt, readPolicy,
            writePolicy);
    RDEBUG("defined KSecurityUIsLockInitiatorTimeHigh", ret);
    ret = RProperty::Define(KPSUidSecurityUIs,
            KSecurityUIsLockInitiatorTimeLow, RProperty::EInt, readPolicy,
            writePolicy);
    RDEBUG("defined KSecurityUIsLockInitiatorTimeLow", ret);

    ret = RProperty::Define(KPSUidSecurityUIs, KSecurityUIsLights,
            RProperty::EInt, TSecurityPolicy(TSecurityPolicy::EAlwaysPass),
            TSecurityPolicy(TSecurityPolicy::EAlwaysPass));
    RDEBUG("defined KAknKeyguardStatus", ret);

    ret = RProperty::Define(KPSUidSecurityUIs, KSecurityUIsDismissDialog,
            RProperty::EInt, TSecurityPolicy(TSecurityPolicy::EAlwaysPass),
            TSecurityPolicy(TSecurityPolicy::EAlwaysPass));
    RDEBUG("defined KSecurityUIsDismissDialog", ret);

    // inactivity
    connect(serviceKeyguard, SIGNAL(active()), this, SLOT(activeKeyguard()));
    connect(serviceKeyguard, SIGNAL(notActive()), this, SLOT(
            notActiveKeyguard()));
    connect(serviceDevicelock, SIGNAL(active()), this, SLOT(
            activeDevicelock()));
    connect(serviceDevicelock, SIGNAL(notActive()), this, SLOT(
            notActiveDevicelock()));

    RWindowGroup& groupWin = CEikonEnv::Static()->RootWin();
    RDEBUG("got groupWin", 1);
    // TODO if I want to release, I should do:   mKeyCaptureHandle = env->RootWin().CaptureKey(EKeyBackspace, 0, 0);
    groupWin.CaptureKey(EKeyBackspace, 0, 0);
    groupWin.CaptureKey(EKeyDeviceF, 0, 0);
    groupWin.CaptureKey(EKeyBell, 0, 0);
    groupWin.CaptureKey(EKeyTab, 0, 0);
    groupWin.CaptureKey(EKeyInsert, 0, 0);
    RDEBUG("got mKeyCaptureHandle", 1);

    iSecQueryUiCreated = -1;
    iDeviceDialogCreated = -1;
    // TODO for now, always starts unlocked
    // TryChangeStatus(iLockStatus);
    TryChangeStatus( ELockNotActive);
    lower();
    hide();
    // not needed:   new AutolockSrvService(this);
    }

AutolockSrv::~AutolockSrv()
    {
    RDEBUG("0", 0);
    delete mService;
    }

void AutolockSrv::adjustInactivityTimers(int aReason)
    {
    RDEBUG("aReason", aReason);
    TInt keyguardTime = 0;
    TInt lockTime = 0;
    CRepository* repository = NULL;
    TInt cRresult = 0;

    QT_TRAP_THROWING( repository = CRepository::NewL(KCRUidSecuritySettings) );
    cRresult = repository->Get(KSettingsAutoLockTime, lockTime);	// in minutes, handled internally as seconds
    lockTime *= 60;
    RDEBUG("KSettingsAutoLockTime", KSettingsAutoLockTime);
    RDEBUG("cRresult", cRresult);
    RDEBUG("lockTime", lockTime);
    if (lockTime > 60 && lockTime < 24*60*60)	// lock timer can't be bigger than 1 day
        {
        serviceDevicelock->setInactivityPeriod(lockTime);
        }
    else
        {
        serviceDevicelock->setInactivityPeriod(12 * 31 * 24 * 60 * 60);	// 0x1ea6e00
        }

    cRresult = repository->Get(KSettingsAutomaticKeyguardTime, keyguardTime);	// in seconds
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
    delete repository;
    }
void AutolockSrv::quit()
    {
    RDEBUG("0", 0);
    qApp->quit();
    }

// from the button
void AutolockSrv::unlockAction()
    {
    RDEBUG("0", 0);
    TInt err = TryChangeStatus( ELockAppDisableDevicelock);
    RDEBUG("err", err);
    }

void AutolockSrv::unguardAction()
    {
    RDEBUG("0", 0);
    TInt err = TryChangeStatus( ELockAppDisableKeyguard);
    RDEBUG("err", err);
    }

void AutolockSrv::test1Action()
    {
    RDEBUG("Set(KSettingsAutoLockTime, 2)", 2);

    CRepository* repositorySet = NULL;
    QT_TRAP_THROWING( repositorySet = CRepository::NewL(KCRUidSecuritySettings) );
    repositorySet->Set(KSettingsAutoLockTime, 2);	// in minutes
    delete repositorySet;
    }

void AutolockSrv::test2Action()
    {
    RDEBUG("Set(KSettingsAutoLockTime, 0)", 0);

    CRepository* repositorySet = NULL;
    QT_TRAP_THROWING( repositorySet = CRepository::NewL(KCRUidSecuritySettings) );
    repositorySet->Set(KSettingsAutoLockTime, 0);	// minutes
    delete repositorySet;
    }
void AutolockSrv::test3Action()
    {
    RDEBUG("SecUiTest", 0);
    RDEBUG("This should start SecUiTest, but it's not done yet", 0);
    }

int AutolockSrv::AskValidSecCode(int aReason)
    {
    RDEBUG("aReason", aReason);
    RMobilePhone iPhone; // NULL in emulator

#ifdef __WINS__1
    return KErrNone;
#endif


    TInt err(KErrGeneral);
    TBool validCode(EFalse);
    TInt thisTry(0);
    RTelServer iTelServer;

    err = RProperty::Set(KPSUidSecurityUIs,
            KSecurityUIsLights, ESecurityUIsLightsQueryOnRequest);
		RDEBUG("KSecurityUIsLights err", err);

    RMmCustomAPI iCustomPhone;
    while ((err = iTelServer.Connect()) != KErrNone && (thisTry++)
            <= KTriesToConnectServer)
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
        QT_TRAP_THROWING( validCode = handler->AskSecCodeInAutoLockL() ); // this returns true/false
        // TODO should this also do iPhone.SetLockSetting(status, lockType, lockChange); ???
        }
    else if (aReason == ELockAppEnableDevicelock)
        {
        // check whether code is really needed
        RMobilePhone::TMobilePhoneLock lockType =
                RMobilePhone::ELockPhoneDevice;
        RMobilePhone::TMobilePhoneLockInfoV1 lockInfo;
        RMobilePhone::TMobilePhoneLockInfoV1Pckg lockInfoPkg(lockInfo);
        RMobilePhone::TMobilePhoneLockSetting lockChange(
                RMobilePhone::ELockSetDisabled);
        TRequestStatus status = KRequestPending;
        TInt ret = KErrNone;
        RDEBUG("GetLockInfo", 0);
        iPhone.GetLockInfo(status, lockType, lockInfoPkg);
        RDEBUG("WaitForRequest", 0);
        User::WaitForRequest(status);
        ret = status.Int();
        RDEBUG("WaitForRequest ret", ret);
#ifdef __WINS__
        if(ret==KErrTimedOut)
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
                QT_TRAP_THROWING( iWait = CWait::NewL() );
                iWait->SetRequestType(EMobilePhoneSetLockSetting);
                iPhone.SetLockSetting(iWait->iStatus, lockType, lockChange); // ask for PassPhraseRequiredL
                RDEBUG("WaitForRequestL", 0);
                QT_TRAP_THROWING( ret = iWait->WaitForRequestL() );
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
                if(ret==KErrTimedOut)
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
         handler->HandleEventL(iEvent, result);	// this is supposed to wait
         validCode = false;
         if(result)
         validCode = true;
         RDEBUG( "result", result );
         */
        }
    // TODO this doesn't wait on WINS , so how do I get the Acceptation?
    RDEBUG("validCode", validCode);
    if (validCode>0)
        return KErrNone;

		// no valid code -> switch off the lights
    err = RProperty::Set(KPSUidSecurityUIs,
            KSecurityUIsLights, ESecurityUIsLightsLockOffRequest);
		RDEBUG("KSecurityUIsLights err", err);
    return KErrCancel;
    }

void AutolockSrv::lockAction()
    {
    RDEBUG("0", 0);
    TryChangeStatus( ELockAppOfferDevicelock);
    }

void AutolockSrv::handleAnswerDelivered()
    {
    RDEBUG("0", 0);
    // quit();

    }

void AutolockSrv::endCall()
    {
    RDEBUG("This should never be called", 0);
    //QVBoxLayout *vl = qobject_cast<QVBoxLayout *>(layout()) ;
    //vl->removeWidget(mEndCallButton);

    //XQServiceUtil::toBackground(true);
    }

void AutolockSrv::setLabelNumber(QString label, QString number)
    {
    RDEBUG("0", 0);
    mLabel->setText("mLabel=" + label);
    mNumberEdit->setText("mNumberEdit=" + number);
    }

void AutolockSrv::DebugRequest(int aReason)
    {
    switch (aReason)
        {
        case ELockAppEnableKeyguard:
            RDEBUG("ELockAppEnableKeyguard", aReason);
            break;
        case ELockAppDisableKeyguard:
            RDEBUG("ELockAppDisableKeyguard", aReason);
            break;
        case ELockAppEnableDevicelock:
            RDEBUG("ELockAppEnableDevicelock", aReason);
            break;
        case ELockAppDisableDevicelock:
            RDEBUG("ELockAppDisableDevicelock", aReason);
            break;
        case ELockAppOfferKeyguard:
            RDEBUG("ELockAppOfferKeyguard", aReason);
            break;
        case ELockAppOfferDevicelock:
            RDEBUG("ELockAppOfferDevicelock", aReason);
            break;
        case ELockAppShowKeysLockedNote:
            RDEBUG("ELockAppShowKeysLockedNote", aReason);
            break;
        default:
            RDEBUG("default", aReason);
            break;
        }
    }
void AutolockSrv::DebugStatus(int aReason)
    {
    switch (aReason)
        {
        case ELockNotActive:
            RDEBUG("ELockNotActive", aReason);
            break;
        case EKeyguardActive:
            RDEBUG("EKeyguardActive", aReason);
            break;
        case EDevicelockActive:
            RDEBUG("EDevicelockActive", aReason);
            break;
        default:
            RDEBUG("default", aReason);
            break;
        }
    }
void AutolockSrv::DebugError(int aReason)
    {
    switch (aReason)
        {
        case KErrNone:
            RDEBUG("KErrNone", aReason);
            break;
        case KErrPermissionDenied:	// caller doesn't have enough capabilities
            RDEBUG("KErrPermissionDenied", aReason);
            break;
        case KErrAlreadyExists:	// this particular lock is already enabled
            RDEBUG("KErrAlreadyExists", aReason);
            break;
        case KErrInUse:	// the dialog is already shown
            RDEBUG("KErrInUse", aReason);
            break;
        default:
            RDEBUG("default", aReason);
            break;
        }
    }

int AutolockSrv::CheckIfLegal(int aReason)
    {
    RDEBUG("aReason", aReason);
    // check whether a dialog is already displayed. This is to prevent timeout-lock and timeout-keyguard for activated on top of a PIN query, in particular at boot-up (TODO Starter)
    TInt secUiOriginatedQuery(ESecurityUIsSecUIOriginatedUninitialized);
    TInt err = RProperty::Get(KPSUidSecurityUIs,
            KSecurityUIsSecUIOriginatedQuery, secUiOriginatedQuery);
    RDEBUG("err", err);
    RDEBUG("secUiOriginatedQuery", secUiOriginatedQuery);
    switch (aReason)
        {
        case ELockAppEnableKeyguard:
            {
            RDEBUG("ELockAppEnableKeyguard iLockStatus", iLockStatus);
            if (secUiOriginatedQuery
                    != ESecurityUIsSecUIOriginatedUninitialized)
                return KErrInUse; // PIN on top. Don't keyguard
            switch (iLockStatus)
                {
                case ELockNotActive:
                    if (1 == 0) // !CKeyLockPolicyApi::KeyguardAllowed() )
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
            if (secUiOriginatedQuery
                    != ESecurityUIsSecUIOriginatedUninitialized
                    && secUiOriginatedQuery
                            != ESecurityUIsSystemLockOriginated)
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
            if (secUiOriginatedQuery
                    != ESecurityUIsSecUIOriginatedUninitialized
                    && secUiOriginatedQuery
                            != ESecurityUIsSystemLockOriginated)
                {
                // PIN on top and trying to display unlock-code. This is valid
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
            if (secUiOriginatedQuery
                    != ESecurityUIsSecUIOriginatedUninitialized
                    && secUiOriginatedQuery
                            != ESecurityUIsSystemLockOriginated)
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
            if (secUiOriginatedQuery
                    != ESecurityUIsSecUIOriginatedUninitialized
                    && secUiOriginatedQuery
                            != ESecurityUIsSystemLockOriginated)
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
            if (secUiOriginatedQuery
                    != ESecurityUIsSecUIOriginatedUninitialized
                    && secUiOriginatedQuery
                            != ESecurityUIsSystemLockOriginated)
                return KErrInUse; // PIN on top. Don't display
            switch (iLockStatus)
                {
                case ELockNotActive:
                    return KErrPermissionDenied;
                case EKeyguardActive:
                    return KErrNone;
                case EDevicelockActive:
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

int AutolockSrv::publishStatus(int aReason)
    {
    RDEBUG("aReason", aReason);
    TInt err;
    // can't use include file because it's private file. However it gives permissions
    const TUid KCRUidCoreApplicationUIsSysAp = { 0x101F8765 };
    const TUint32 KSysApKeyguardActive = 0x00000001;
    CRepository* repositoryDevicelock = NULL;
    CRepository* repositoryKeyguard = NULL;
    QT_TRAP_THROWING( repositoryDevicelock = CRepository::NewL(KCRUidSecuritySettings) );
    QT_TRAP_THROWING( repositoryKeyguard = CRepository::NewL(KCRUidCoreApplicationUIsSysAp) );
    TInt cRresult = KErrNone;
    if (1 == 1) // this is a quick way to disable this functionality, for testing
        {
        RDEBUG("publishing", aReason);
        if (aReason == ELockNotActive)
            {
            err = RProperty::Set(KPSUidAvkonDomain, KAknKeyguardStatus,
                    EKeyguardNotActive);
            RDEBUG("err", err);
            err = RProperty::Set(KPSUidCoreApplicationUIs,
                    KCoreAppUIsAutolockStatus, EAutolockOff);
            RDEBUG("err", err);
            // lights are required ?

            // cRresult = repositoryDevicelock->Set(KSettingsAutolockStatus, 0);	// the settings remains. Only ISA changes, as well as the P&S
            cRresult = repositoryKeyguard->Set(KSysApKeyguardActive, 0);
            RDEBUG("cRresult", cRresult);
            }
        if (1 == 1) // this is a quick way to disable this functionality, for testing
            {
            if (aReason == EKeyguardActive)
                {
                err = RProperty::Set(KPSUidAvkonDomain, KAknKeyguardStatus,
                        EKeyguardLocked);
            		RDEBUG("KAknKeyguardStatus err", err);
                err = RProperty::Set(KPSUidCoreApplicationUIs,
                        KCoreAppUIsAutolockStatus, EAutolockOff);
            		RDEBUG("KCoreAppUIsAutolockStatus err", err);
                err = RProperty::Set(KPSUidSecurityUIs,
                        KSecurityUIsLights, ESecurityUIsLightsLockOffRequest);	// same for keyguard and devicelock
            		RDEBUG("KSecurityUIsLights err", err);
                // cRresult = repositoryDevicelock->Set(KSettingsAutolockStatus, 0);
                cRresult = repositoryKeyguard->Set(KSysApKeyguardActive, 1);
                RDEBUG("cRresult", cRresult);
                }
            else if (aReason >= EDevicelockActive)
                {
                err = RProperty::Set(KPSUidAvkonDomain, KAknKeyguardStatus,
                        EKeyguardAutolockEmulation); // Other candidates might be: EKeyguardLocked and EKeyguardNotActive
            		RDEBUG("KAknKeyguardStatus err", err);
                err = RProperty::Set(KPSUidCoreApplicationUIs,
                        KCoreAppUIsAutolockStatus, EManualLocked);
            		RDEBUG("KCoreAppUIsAutolockStatus err", err);
                err = RProperty::Set(KPSUidSecurityUIs,
                        KSecurityUIsLights, ESecurityUIsLightsLockOffRequest);
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
int AutolockSrv::showNoteIfRequested(int aReason)
    {
    RDEBUG("aReason", aReason);

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
int AutolockSrv::TryChangeStatus(int aReason)
    {
    RDEBUG("aReason", aReason);
    int ret = aReason;
    int errorInProcess = KErrNone;
    DebugRequest(ret);

    TInt err = RProperty::Set(KPSUidSecurityUIs,
            KSecurityUIsDismissDialog,
            ESecurityUIsDismissDialogOn);
		RDEBUG("err", err);
    switch (ret)
        {
        case ELockAppEnableKeyguard: // 1
            {
            errorInProcess = CheckIfLegal(ret);
            DebugError(errorInProcess);
            if (errorInProcess == KErrNone)
                {
                setLabelIcon( EKeyguardActive);
                updateIndicator(EKeyguardActive);
                publishStatus(EKeyguardActive);
                showNoteIfRequested(EKeyguardActive);
                }
            }
            break;
        case ELockAppDisableKeyguard: // 2
            {
            errorInProcess = CheckIfLegal(ret);
            DebugError(errorInProcess);
            if (errorInProcess == KErrNone)
                {
                setLabelIcon( ELockNotActive);
                updateIndicator(ELockNotActive);
                publishStatus(ELockNotActive);
                showNoteIfRequested(ELockNotActive);
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
                break;	// so that "disable while disabled" doesn't mess up
                }
            if (errorInProcess == KErrNone)
                {
                RDEBUG(" calling HbDeviceMessageBox::question", 0);
                bool value = HbDeviceMessageBox::question("Disable Lock?");	// this doesn't block other events, so after return everything might be messed up.
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
                publishStatus(ELockNotActive);
                }
            if (errorInProcess != KErrNone)
                { // re-lock. For example, if password is wrong
               	if( iLockStatus >=EDevicelockActive)	// this skips the case when unlocking, although it wan't locked.
                	setLockDialog(aReason, 1);
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
                bool value = HbDeviceMessageBox::question("Enable Keyguard?");	// this doesn't block other events, so after return everything might be messed up.
                // TODO what about a nice icon?
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
            setLockDialog(aReason, 0); // hide temporarilly because AskValidSecCode doesn't get in top of the Lock-icon. Thus, dismiss it.
            errorInProcess = AskValidSecCode(ELockAppEnableDevicelock);
            if (errorInProcess == KErrNone)
                {
                RDEBUG("ELockAppOfferDevicelock calling ELockAppEnableDevicelock", ELockAppEnableDevicelock);
                errorInProcess = TryChangeStatus(ELockAppEnableDevicelock);
                }
            // this never shows a note. Perhaps ELockAppEnableDevicelock does.
            }
            break;
        case ELockAppShowKeysLockedNote:
            {
            iShowKeyguardNote = 1; // this is not sent as parameter, so we need to fake it: ON
            showNoteIfRequested( EKeyguardActive);
            }
            break;
        default:
            RDEBUG("default", ret);
            errorInProcess = KErrNotSupported;
            break;

        }
    return errorInProcess;
    }
/**************************/
int AutolockSrv::setLockDialog(int aReason, int status)
    {
    RDEBUG("aReason", aReason);
    RDEBUG("status", status);
    RDEBUG("iDeviceDialogCreated", iDeviceDialogCreated);
    TInt secUiOriginatedQuery(ESecurityUIsSecUIOriginatedUninitialized);
    TInt err = RProperty::Get(KPSUidSecurityUIs,
            KSecurityUIsSecUIOriginatedQuery, secUiOriginatedQuery);
    RDEBUG("err", err);
    RDEBUG("secUiOriginatedQuery", secUiOriginatedQuery);

    if (status == 0) // hide
        {
        // secUiOriginatedQuery should be ESecurityUIsSystemLockOriginated . If not, this is not correctly setting it
        if (iDeviceDialogCreated > 0)
            {
            iDeviceDialogCreated = 0;
            iDeviceDialog->cancel();
            err = iDeviceDialog->error();
            RDEBUG("err", err);
            TInt err = RProperty::Set(KPSUidSecurityUIs,
                    KSecurityUIsSecUIOriginatedQuery,
                    ESecurityUIsSecUIOriginatedUninitialized);
            RDEBUG("err", err);
            }
        }
    else if (status == 1) // show
        {
        // secUiOriginatedQuery should be ESecurityUIsSecUIOriginatedUninitialized . If not, the validation is not correctly filtering it
        QVariantMap params;
        TBool err;
#define ESecUiTypeDeviceLock		0x00100000
#define ESecUiTypeKeyguard			0x00200000

        if (aReason == EKeyguardActive)
            params.insert("type", ESecUiTypeKeyguard);
        else if (aReason >= EDevicelockActive)
            params.insert("type", ESecUiTypeDeviceLock);
        else
            {
            RDEBUG("error. status=1 but aReason", aReason);
            }
        // no need for title. Icon should be explicit enough
        // params.insert("title", "Locked");
        if (iDeviceDialogCreated <= 0)
            {
            RDEBUG("creating iDeviceDialog", 0);
            iDeviceDialog = new HbDeviceDialog(HbDeviceDialog::NoFlags, this);
            iDeviceDialogCreated = 1;
            }
        else
            {
            RDEBUG("raising iDeviceDialog", 0);
            // confirm that dialog is present
            err = iDeviceDialog->error();
            RDEBUG("err", err);
            iDeviceDialogCreated = 2;
            }
        const QString KSecQueryUiDeviceDialog("com.nokia.secuinotificationdialog/1.0");
        RDEBUG("pre show", aReason);
        err = iDeviceDialog->show(KSecQueryUiDeviceDialog, params); // and continue processing
        RDEBUG("post show. err", err);
        err = iDeviceDialog->error();
        RDEBUG("err", err);
        // This won't be needed when screensaver is in place, as the dialogs will be different, and therefore both can be present
        // Somehow this should be handled by Orbit, but unfortunatelly they don't allow the same dialog twice
        err = RProperty::Set(KPSUidSecurityUIs,
                KSecurityUIsSecUIOriginatedQuery,
                ESecurityUIsSecUIOriginatedUninitialized);
        RDEBUG("err", err);

        }
    else
        {
        RDEBUG("unknown status", status);
        return KErrNotSupported;
        }
    return KErrNone;
    }
void AutolockSrv::setLabelIcon(int aReason)
    {
    RDEBUG("aReason", aReason);

    if (aReason == ELockNotActive)
        {
        mLabelIcon->setVisible(false);
        setLockDialog(aReason, 0); // TODO isn't this done already at TryChangeStatus ???
        lower();
        hide();
        }
    else if (aReason == EKeyguardActive)
        {
        mLabelIcon->setVisible(true);
        setLockDialog(aReason, 1);
        raise(); // not repaint(), not show() , not setVisible(true), not showFullScreen() , not ???
        show();
        }
    else if (aReason == EDevicelockActive)
        {
        mLabelIcon->setVisible(true);
        raise();
        show();
        }
    else
        {
        RDEBUG("error: aReason", aReason);
        }
    }

int AutolockSrv::updateIndicator(int aReason)
    {
    RDEBUG("aReason", aReason);
    QList<QVariant> list;
    list.insert(0, 1);
    list.insert(1, "dummy");
    list.insert(2, 2);

    HbIndicator indicator;
    bool success;
    if (aReason == ELockNotActive)
        success = indicator.deactivate(
                "com.nokia.hb.indicator.autolocksrv.autolocksrv_8/1.0"); // maybe it's already deactivated. Not a problem
    else if (aReason == EKeyguardActive)
        success = indicator.activate(
                "com.nokia.hb.indicator.autolocksrv.autolocksrv_8/1.0");
    else if (aReason == EDevicelockActive)
        success = indicator.activate(
                "com.nokia.hb.indicator.autolocksrv.autolocksrv_8/1.0"); // same. We have just 1 indicator
    else
        success = 0;
    RDEBUG("success", success);
    return success;
    }

void AutolockSrv::activeKeyguard()
    {
    // activity while keyguarded. Nothing to do
    RDEBUG("0", 0);
    }

void AutolockSrv::notActiveKeyguard()
    {
    // inactivity. Keyguard should be activated
    RDEBUG("0", 0);
    int ret = KErrNone;
    DebugStatus( iLockStatus);
    if (iLockStatus == ELockNotActive) // not possible if it's keyguarded, or locked
        {
        ret = TryChangeStatus(ELockAppEnableKeyguard);
        }
    RDEBUG("ret", ret);
    }
/* Some activity detected while the deviceLock is enabled */
void AutolockSrv::activeDevicelock()
    {
    // nothing to do
    RDEBUG("0", 0);
    }

void AutolockSrv::notActiveDevicelock()
    {
    // inactivity. Devicelock should be activated
    RDEBUG("0", 0);
    int ret = KErrNone;
    DebugStatus( iLockStatus);
    if (iLockStatus == ELockNotActive || iLockStatus == EKeyguardActive) // not possible if it's locked
        {
        ret = TryChangeStatus(ELockAppEnableDevicelock);
        }
    RDEBUG("ret", ret);

    }

// some key is pressed
bool AutolockSrv::event(QEvent *ev)
    {
    if (ev)
        {
        int isSwitchKey = 0;
        // on device, this doesn't seem to get the EKeyDeviceF key: only 1ffffff
        if (ev->type() == QEvent::KeyPress)
            {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *> (ev);
            RDEBUG("KeyPress", keyEvent->key());
            if ((keyEvent->key() & 0xFF) == (EKeyInsert & 0xFF))
                {
                RDEBUG("EKeyInsert", EKeyInsert);
                // only reacts on release, not on press
                isSwitchKey = 1;
                }
            if ((keyEvent->key() & 0xFF) == (EKeyTab & 0xFF))
                {
                RDEBUG("EKeyTab", EKeyTab);
                }
            if ((keyEvent->key() & 0xFF) == (EKeyDeviceF & 0xFF))
                {
                RDEBUG("EKeyDeviceF", EKeyDeviceF);
                }
            if (keyEvent->key() == 0x1ffffff)
                {
                RDEBUG("0x1ffffff", 0x1ffffff);
                isSwitchKey = 1;
                }
            }
        else if (ev->type() == QEvent::KeyRelease)
            {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *> (ev);
            RDEBUG("KeyRelease", keyEvent->key());
            if (keyEvent->key() == 0x1ffffff)
                {
                RDEBUG("released EKeyDeviceF-1ffffff", 1);
                // isSwitchKey=1; this should happen is   pressed  was not processed (hint: if it is/was in background)
                }
            }
        RDEBUG("isSwitchKey", isSwitchKey);
        if (isSwitchKey)
            {
            int ret = KErrNone;
            DebugStatus( iLockStatus);
            if (iLockStatus == ELockNotActive)
                {
                iShowKeyguardNote = 1; // note on enable keyguard
                ret = TryChangeStatus(ELockAppEnableKeyguard); // this should not ask confirmation
                }
            else if (iLockStatus == EKeyguardActive)
                {
                iShowKeyguardNote = 1; // note on disable keyguard
                ret = TryChangeStatus(ELockAppDisableKeyguard);
                }
            else if (iLockStatus == EDevicelockActive)
                {
                ret = TryChangeStatus(ELockAppDisableDevicelock);
                }
            else
                {
                RDEBUG("unknown iLockStatus", iLockStatus);
                }
            RDEBUG("ret", ret);
            } // isSwitchKey
        } // ev
    // Process if not done before. For example, redraw or quit
    return QWidget::event(ev);
    }

bool AutolockSrv::eventFilter(QObject *o, QEvent *ev)
    {
    // this never happens
    RDEBUG("0", 0);
    return QWidget::eventFilter(o, ev);

    }
// some setting changed
void AutolockSrv::subscriberKSettingsAutolockStatusChanged()
    {
    RDEBUG("0", 0);
    QVariant v = subscriberKSettingsAutolockStatus->value(
            "/KCRUidSecuritySettings/KSettingsAutolockStatus");
    adjustInactivityTimers( KSettingsAutolockStatus);
    }
void AutolockSrv::subscriberKSettingsAutoLockTimeChanged()
    {
    RDEBUG("0", 0);
    QVariant v = subscriberKSettingsAutoLockTime->value(
            "/KCRUidSecuritySettings/KSettingsAutoLockTime");
    adjustInactivityTimers( KSettingsAutoLockTime);
    }
void AutolockSrv::subscriberKSettingsAutomaticKeyguardTimeChanged()
    {
    RDEBUG("0", 0);
    QVariant v = subscriberKSettingsAutomaticKeyguardTime->value(
            "/KCRUidSecuritySettings/KSettingsAutomaticKeyguardTime");
    adjustInactivityTimers( KSettingsAutoLockTime);
    }
void AutolockSrv::subscriberKDisplayLightsTimeoutChanged()
    {
    RDEBUG("0", 0);
    QVariant v = subscriberKDisplayLightsTimeout->value(
            "/KCRUidLightSettings/KDisplayLightsTimeout");
    // nothing to do
    }
void AutolockSrv::subscriberKProEngActiveProfileChanged()
    {
    RDEBUG("0", 0);
    QVariant v = subscriberKProEngActiveProfile->value(
            "/KCRUidProfileEngine/KProEngActiveProfile");
    // nothing to do
    }
// some environment changed
void AutolockSrv::subscriberKAknKeyguardStatusChanged()
    {
    RDEBUG("Error only AutolockSrv should be able to change it", 0);
  	}
void AutolockSrv::subscriberKCoreAppUIsAutolockStatusChanged()
    {
    RDEBUG("Error only AutolockSrv should be able to change it", 0);
  	}
void AutolockSrv::subscriberKHWRMGripStatusChanged()
    {
    TInt ret;
    RDEBUG("0", 0);
    TInt aGripStatus;
    TInt err = RProperty::Get(KPSUidHWRM, KHWRMGripStatus, aGripStatus );
    RDEBUG("err", err);
    RDEBUG("value", aGripStatus);
    if( aGripStatus == EPSHWRMGripOpen )
    	{
      if (iLockStatus == EKeyguardActive)
          {
          iShowKeyguardNote = 1; // note on disable keyguard
          ret = TryChangeStatus(ELockAppDisableKeyguard);
          }
      else if (iLockStatus == EDevicelockActive)
          {
          ret = TryChangeStatus(ELockAppDisableDevicelock);
          }
      }
    else if( aGripStatus == EPSHWRMGripClosed )
    	{
      if (iLockStatus == ELockNotActive)
          {
          iShowKeyguardNote = 1; // note on enable keyguard
          ret = TryChangeStatus(ELockAppEnableKeyguard);
          }
    	}
    }

// ----------AutolockSrvService---------------

AutolockSrvService::AutolockSrvService(AutolockSrv* parent) :
    XQServiceProvider(QLatin1String(
            "com.nokia.services.AutolockSrv.AutolockSrv"), parent),
            mAutolockSrv(parent), mAsyncReqId(-1), mAsyncAnswer(false)
    {
    RDEBUG("0", 0);
    publishAll();
    connect(this, SIGNAL(returnValueDelivered()), parent, SLOT(
            handleAnswerDelivered()));
    }

AutolockSrvService::~AutolockSrvService()
    {
    RDEBUG("0", 0);
    }

void AutolockSrvService::complete(QString number)
    {
    RDEBUG("0", 0);
    if (mAsyncReqId == -1)
        return;
    completeRequest(mAsyncReqId, number.toInt());
    }

// got API request from lockaccessextension
int AutolockSrvService::service(const QString& number,
        const QString& aParam1, const QString& aParam2)
    {
    RDEBUG("0", 0);
    TInt err = KErrNone;
    RDEBUG("number", number.toInt());
    RDEBUG("aParam1", aParam1.toInt());
    RDEBUG("aParam2", aParam2.toInt());
    mAsyncAnswer = false;
    XQRequestInfo info = requestInfo();
    QSet<int> caps = info.clientCapabilities();

    mAutolockSrv->callerHasECapabilityWriteDeviceData = 0;
    if (caps.contains(ECapabilityWriteDeviceData))
        {
        RDEBUG("callerHasECapabilityWriteDeviceData",
                ECapabilityWriteDeviceData);
        mAutolockSrv->callerHasECapabilityWriteDeviceData = 1;
        }

    QString label = "AutolockSrv::service:\n";
    label += QString("number=%1\n").arg(number);

    mNumber = number;
    mAutolockSrv->setLabelNumber(label, number);
    int ret = 0;
    ret = number.toInt();

    mAutolockSrv->mParam1 = aParam1.toInt();
    mAutolockSrv->iShowKeyguardNote = aParam1.toInt();
    mAutolockSrv->mParam2 = aParam2.toInt();
    TTime myTime;
    myTime.HomeTime();
    TInt myTimeHigh = 0;
    TInt myTimeLow = 0;
    err = RProperty::Get(KPSUidSecurityUIs, KSecurityUIsLockInitiatorTimeHigh, myTimeHigh );
    RDEBUG("err", err);
    RDEBUG("myTimeHigh", myTimeHigh);
    err = RProperty::Get(KPSUidSecurityUIs, KSecurityUIsLockInitiatorTimeLow, myTimeLow );
    RDEBUG("err", err);
    RDEBUG("myTimeLow", myTimeLow);
    
    myTimeHigh = I64HIGH( myTime.Int64() );
    myTimeLow = I64LOW( myTime.Int64() );
    RDEBUG("myTimeHigh", myTimeHigh);
    RDEBUG("myTimeLow", myTimeLow);
    err = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsLockInitiatorTimeHigh, myTimeHigh );
    err = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsLockInitiatorTimeLow, myTimeLow );


    ret = mAutolockSrv->TryChangeStatus(ret);
    RDEBUG("ret", ret);
    return ret;
    }

void AutolockSrvService::handleClientDisconnect()
    {
    RDEBUG("0", 0);
    // Just quit service application if client ends
    mAsyncAnswer = false;
    RDEBUG("0", 0);
    // mAutolockSrv->quit();
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

