/*
 * Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies). 
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
 * Description:  Provides api for handling security events.
 *
 *
 */
#include <e32notif.h>
#include <aknnotedialog.h>
#include <etelmm.h>
// #include <SecUi.rsg>
#include <exterror.h>
#include <textresolver.h>

#ifdef __COVER_DISPLAY
#include <aknmediatorfacade.h>
#endif //__COVER_DISPLAY
#include <centralrepository.h> 
#include <e32property.h>

#include <PSVariables.h>   // Property values
#include <coreapplicationuisdomainpskeys.h>
#include <startupdomainpskeys.h>
// #include <uikon/eiksrvui.h>
#include <settingsinternalcrkeys.h>
#include <securityuisprivatepskeys.h>
// #include <AknNotiferAppServerApplication.h>

#include <SCPClient.h>
#include <securitynotification.h>
#include "secui.hrh"
#include "SecUi.h"
#include "secuisecurityhandler.h"
#include "secuicodequerydialog.h"
#include "secuisecuritysettings.h"
#include "SecUiWait.h"
// #include "SecUiLockObserver.h"
#include <RemoteLockSettings.h>
#include <StringLoader.h>
#include <featmgr.h>
#include <hb/hbcore/hbtextresolversymbian.h>

#include "SecQueryUi.h"

//  LOCAL CONSTANTS AND MACROS
const TInt KMaxNumberOfPUKAttempts(10);
const TInt KMaxNumberOfPINAttempts(3);
// not used
// const TInt KLastRemainingInputAttempt(1);

const TInt KTriesToConnectServer(2);
const TInt KTimeBeforeRetryingRequest(50000);

// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CSecurityHandler::CSecurityHandler()
// C++ constructor
// ----------------------------------------------------------
// qtdone
EXPORT_C CSecurityHandler::CSecurityHandler(RMobilePhone& aPhone) :
    iPhone(aPhone), iQueryCanceled(ETrue), iSecurityDlg(NULL), iNoteDlg(NULL)
    {
    RDEBUG("0", 0);

    TInt result = iCustomPhone.Open(aPhone);
    RDEBUG("result", result);
    TRAP_IGNORE(FeatureManager::InitializeLibL()); //Shouldn't this panic if FM does not initialise??

    _LIT(KFileName, "secui_");
    _LIT(KPath, "z:/resource/qt/translations/");
    RDEBUG("HbTextResolverSymbian", 0);
    result = HbTextResolverSymbian::Init(KFileName, KPath);
    RDEBUG("result", result);
    }

//
// ----------------------------------------------------------
// CSecurityHandler::~CSecurityHandler()
// Destructor
// ----------------------------------------------------------
// qtdone
EXPORT_C CSecurityHandler::~CSecurityHandler()
    {
    RDEBUG("0", 0);

    if (iDestroyedPtr)
        {
        *iDestroyedPtr = ETrue;
        iDestroyedPtr = NULL;
        }
    RDEBUG("calling CancelOpenQuery", 0);
    TInt err = CancelOpenQuery(-1);
    RDEBUG("err", err);
    iCustomPhone.Close();
    FeatureManager::UnInitializeLib();
    RDEBUG("1", 1);
    }
//
// ----------------------------------------------------------
// CSecurityHandler::HandleEventL()
// Handles different security events
// ----------------------------------------------------------
// qtdone
EXPORT_C void CSecurityHandler::HandleEventL(RMobilePhone::TMobilePhoneSecurityEvent aEvent)
    {
    RDEBUG("0", 0);

    TInt result = KErrNone;
    HandleEventL(aEvent, result);
    }

//
// ----------------------------------------------------------
// CSecurityHandler::HandleEventL()
// Handles different security events
// ----------------------------------------------------------
// qtdone
EXPORT_C void CSecurityHandler::HandleEventL(RMobilePhone::TMobilePhoneSecurityEvent aEvent, TBool aStartup, TInt& aResult)
    {
    RDEBUG("0", 0);
	RDEBUG("TBool aStartup", aStartup);

    iStartup = aStartup;
    HandleEventL(aEvent, aResult);
    }

//
// ----------------------------------------------------------
// CSecurityHandler::HandleEventL()
// Handles different security events
// ----------------------------------------------------------
// qtdone
EXPORT_C void CSecurityHandler::HandleEventL(RMobilePhone::TMobilePhoneSecurityEvent aEvent, TInt& aResult)
    {
    RDEBUG("0", 0);

    /*****************************************************
     *    Series 60 Customer / ETel
     *    Series 60  ETel API
     *****************************************************/
    TBool wcdmaSupported(FeatureManager::FeatureSupported(KFeatureIdProtocolWcdma));
    TBool upinSupported(FeatureManager::FeatureSupported(KFeatureIdUpin));
    RDEBUG("aEvent", aEvent);

    switch (aEvent)
        {
        case RMobilePhone::EPin1Required:
            RDEBUG("RMobilePhone::EPin1Required", 0)
            ;
            aResult = Pin1RequiredL();
            break;
        case RMobilePhone::EPuk1Required:
            RDEBUG("RMobilePhone::EPuk1Required", 0)
            ;
            Puk1RequiredL();
            break;
        case RMobilePhone::EPin2Required:
            Pin2RequiredL();
            break;
        case RMobilePhone::EPuk2Required:
            Puk2RequiredL();
            break;
        case RMobilePhone::EUniversalPinRequired:
            if (wcdmaSupported || upinSupported)
                {
                aResult = UPinRequiredL();
                }
            else
                aResult = KErrNotSupported;
            break;
        case RMobilePhone::EUniversalPukRequired:
            if (wcdmaSupported || upinSupported)
                {
                aResult = UPukRequiredL();
                }
            else
                aResult = KErrNotSupported;
            break;
        case RMobilePhone::EPhonePasswordRequired+0x100:	// from Autolock
        case RMobilePhone::EPhonePasswordRequired:
            aResult = PassPhraseRequiredL();
            break;
        case RMobilePhone::EICCTerminated:
            SimLockEventL();
            break;
        default:
            RDEBUG("default", aEvent)
            ;
            break;
        }
    RDEBUG("aResult", aResult);
    }
//
// ----------------------------------------------------------
// CSecurityHandler::AskSecCodeL()
// For asking security code e.g in settings
// ----------------------------------------------------------
// qtdone
EXPORT_C TBool CSecurityHandler::AskSecCodeL()
    {
    /*****************************************************
     *    Series 60 Customer / ETel
     *    Series 60  ETel API
     *****************************************************/
    RDEBUG("0", 0);
    // if code is still not initialized, then there's no need to ask it. This fixes the error when the RFS requests the code.
    const TUid KCRUidSCPLockCode =
        {
        0x2002677B
        };
    const TUint32 KSCPLockCodeDefaultLockCode = 0x00000001;

    CRepository* repository = CRepository::NewL(KCRUidSCPLockCode);
    TInt currentLockStatus = -1;
    TInt res = -1;
    TInt lAlphaSupported = 0;
    TInt lCancelSupported = 0;
    RMobilePhone::TMobilePassword iSecUi_password;

    res = repository->Get(KSCPLockCodeDefaultLockCode, currentLockStatus);	// 0x3039 = 12345
    RDEBUG("res", res);
    RDEBUG("currentLockStatus", currentLockStatus);
    delete repository;
    if (res == 0 && (currentLockStatus == 1 || currentLockStatus == 12345) )
        {
        // code is the default one
        RDEBUG("code is the default one; supply as default", 1);
        iSecUi_password.Copy(_L("12345"));
        // RDEBUG("code is the default one; no need to request it", 1);
        // return ETrue;
        }
    // end check for default code

    RMobilePhone::TMobilePassword required_fourth;

    TInt ret = KErrNone;
    TInt status = KErrNone;

    TInt queryAccepted = KErrCancel;

    while (queryAccepted != KErrNone)
        {
        RMobilePhone::TMobilePhoneSecurityCode secCodeType;
        secCodeType = RMobilePhone::ESecurityCodePhonePassword;

        /* request PIN using QT */
        CSecQueryUi *iSecQueryUi;
        RDEBUG("CSecQueryUi", 0);
        iSecQueryUi = CSecQueryUi::NewL();
        iQueryCanceled = EFalse;
        lAlphaSupported = ESecUiAlphaSupported;
        lCancelSupported = ESecUiCancelSupported;
        TBuf<0x100> title;
        title.Zero();
        HBufC* stringHolder = HbTextResolverSymbian::LoadLC(_L("txt_pin_code_dialog_sec_code"));
        title.Append(stringHolder->Des());
        CleanupStack::PopAndDestroy(stringHolder);
        title.Append(_L("$"));
        title.AppendNum(-1);	// Don't know the number of remaining attempts
        queryAccepted = iSecQueryUi->SecQueryDialog(title, iSecUi_password, SEC_C_SECURITY_CODE_MIN_LENGTH, SEC_C_SECURITY_CODE_MAX_LENGTH, ESecUiSecretSupported | lAlphaSupported
                | lCancelSupported | secCodeType);
        RDEBUG("iSecUi_password", 0);
        RDEBUGSTR(iSecUi_password);
        RDEBUG("delete", 0);
        iQueryCanceled = ETrue;
        delete iSecQueryUi;
        RDEBUG("queryAccepted", queryAccepted);
        /* end request PIN using QT */
        if (queryAccepted != KErrNone)
            {
            ret = EFalse;
            return ret;
            }

		// Validate the code using SCP. This is needed to get hash
    RDEBUG("AbortSecurityCode", 0);
    RSCPClient scpClient;
    RDEBUG("scpClient.Connect", 0);
    TInt tRet = scpClient.Connect();
    RDEBUG("tRet", tRet);

    CleanupClosePushL(scpClient);

    RArray<TDevicelockPolicies> aFailedPolicies;
    TInt retLockcode = KErrNone;
    RMobilePhone::TMobilePassword aISACode;
    TInt scpFlags = 0;
    RDEBUG("scpClient.VerifyCurrentLockcode", 0);
    // this validate on ISA . No need to do iPhone.VerifySecurityCode
    retLockcode = scpClient.VerifyCurrentLockcode(iSecUi_password, aISACode, aFailedPolicies, scpFlags);
    RDEBUG("retLockcode", retLockcode);

    RDEBUG("aISACode", 0);
    RDEBUGSTR(aISACode);

    RDEBUG("aFailedPolicies.Count()", aFailedPolicies.Count());
    RDEBUG("Close", 0);
    scpClient.Close();
    RDEBUG("PopAndDestroy", 0);
    CleanupStack::PopAndDestroy(); //scpClient

        CWait* wait = CWait::NewL();
        RDEBUG("iSecUi_password", 0);
        RDEBUGSTR(iSecUi_password);
        RDEBUG("aISACode", 0);
        RDEBUGSTR(aISACode);
        RDEBUG("VerifySecurityCode", 0);
        iPhone.VerifySecurityCode(wait->iStatus, secCodeType, aISACode /* not iSecUi_password !!! */, required_fourth);
        RDEBUG("WaitForRequestL", 0);
        status = wait->WaitForRequestL();
        RDEBUG("status", status);
        delete wait;
#ifdef __WINS__
        if (status == KErrNotSupported || status == KErrTimedOut)
            {
            RDEBUG("status", status);
            status = KErrNone;
            }
#endif

        ret = EFalse;
        queryAccepted = KErrCancel; // because it's not yet validated
        switch (status)
            {
            case KErrNone:
                {
                if (FeatureManager::FeatureSupported(KFeatureIdSapTerminalControlFw) && !(FeatureManager::FeatureSupported(KFeatureIdSapDeviceLockEnhancements)))
                    {
                    RDEBUG("calling RSCPClient", 0);
                    RSCPClient scpClient;
                    User::LeaveIfError(scpClient.Connect());
                    CleanupClosePushL(scpClient);

                    TSCPSecCode newCode;
                    RDEBUG("iSecUi_password", 1);
                    RDEBUGSTR(iSecUi_password);
                    newCode.Copy(iSecUi_password);	// this might fail if lenght=10 , because TSCPSecCode=8, while SEC_C_SECURITY_CODE_MAX_LENGTH=10
                    RDEBUG(
                            "!!!!!!! ***** deprecated **** !!!!! scpClient.StoreCode",
                            0);
                    scpClient.StoreCode(newCode);
                    RDEBUG("called StoreCode", 1);

                    CleanupStack::PopAndDestroy(); //scpClient
                    queryAccepted = KErrNone;
                    }

                iQueryCanceled = ETrue;
                return ETrue;
                }
            case KErrGsmSSPasswordAttemptsViolation:
            case KErrLocked:
                {
                // security code blocked! 
                CSecuritySettings::ShowResultNoteL(R_SEC_BLOCKED, CAknNoteDialog::EErrorTone);
                break;
                }
            case KErrGsm0707IncorrectPassword:
            case KErrAccessDenied:
                {
                // code was entered erroneusly
                CSecuritySettings::ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
                break;
                }
            default:
                {
                CSecuritySettings::ShowResultNoteL(status, CAknNoteDialog::EErrorTone);
                }
            }
        RDEBUG("while AskSecCodeL", 1);
        iSecUi_password.Copy(_L(""));	// clear password so that the next time, it shows empty
        } // while

    iQueryCanceled = ETrue;
    RDEBUG("ret", ret);
    return ret;
    }
//
// ----------------------------------------------------------
// Cancels all security code queries
// aStatus = -1     from destructor
// aStatus =  1     from API. Will kill all dialogs through signal P&S
// ----------------------------------------------------------
// qtdone
TInt CSecurityHandler::CancelOpenQuery(TInt aStatus)
    {
    RDEBUG("aStatus", aStatus);
    RDEBUG("iQueryCanceled", iQueryCanceled);
    TInt res = 0;

    if (aStatus == 1) // also signal all other dialogs
        {
        RDEBUG(
                "set KSecurityUIsDismissDialog to ESecurityUIsDismissDialogOn",
                ESecurityUIsDismissDialogOn);
        TInt err = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsDismissDialog, ESecurityUIsDismissDialogOn);
        RDEBUG("err", err);
        res += 1;
        }

    if (!iQueryCanceled)
        {
        // notify all dialogs, in particular SecUiNotificationDialog::subscriberKSecurityUIsDismissDialogChanged
        // this will cancel only the dialog which was opened by same client.
        res += 0x10;
        iQueryCanceled = ETrue;
        if (iSecurityDlg != NULL)
            {
            RDEBUG("deleting iSecurityDlg", 0);
            res += 0x100;
            delete iSecurityDlg;
            }
        if (iNoteDlg != NULL)
            {
            RDEBUG("deleting iNoteDlg", 0);
            res += 0x1000;
            delete iNoteDlg;
            }
        iNoteDlg = NULL;
        iSecurityDlg = NULL;
        }
    res += 0x10000;
    RDEBUG("res", res);
    return res;
    }

//
// ----------------------------------------------------------
// CSecurityHandler::CancelSecCodeQuery()    
// Cancels PIN2 and security code queries
// this is used by rfsHandler
// ----------------------------------------------------------
// qtdone
EXPORT_C void CSecurityHandler::CancelSecCodeQuery()
    {
    RDEBUG("0", 0);

    TInt err = CancelOpenQuery(1);

    RDEBUG("err", err);
    }
//
// ----------------------------------------------------------
// CSecurityHandler::AskSecCodeInAutoLock()
// for asking security code in autolock
// ----------------------------------------------------------
// qtdone
EXPORT_C TBool CSecurityHandler::AskSecCodeInAutoLockL()
    {
    /*****************************************************
     *    Series 60 Customer / ETel
     *    Series 60  ETel API
     *****************************************************/

    RDEBUG("0", 0);
    TInt res;
    CWait* wait;

    RMobilePhone::TMobilePhoneLockSetting lockChange(RMobilePhone::ELockSetDisabled);
    RMobilePhone::TMobilePhoneLock lockType = RMobilePhone::ELockPhoneDevice;

    // get autolock period from Central Repository.
    CRepository* repository = CRepository::NewL(KCRUidSecuritySettings);
    TInt period = 0;
    res = repository->Get(KSettingsAutoLockTime, period);
    delete repository;

    RDEBUG("res", res);
    RDEBUG("period", period);

    if (res == KErrNone)
        {
        // disable autolock in Domestic OS side too if autolock period is 0.
        if (period == 0)
            {
            RDEBUG("period", period);

            // If remote lock is enabled, don't disable the domestic OS device lock
            // since that would render the RemoteLock useless.
            // Instead just re-set the DOS lock to enabled which as a side effect
            // requests the security code from the user.

            TBool remoteLockStatus(EFalse);
            CRemoteLockSettings* remoteLockSettings = CRemoteLockSettings::NewL();

            if (remoteLockSettings->GetEnabled(remoteLockStatus))
                {
                RDEBUG("0", 0);
                if (remoteLockStatus)
                    {
                    // Remote lock is enabled
                    lockChange = RMobilePhone::ELockSetEnabled;
                    RDEBUG("lockChange", lockChange);
                    }
                else
                    {
                    // Remote lock is disabled
                    lockChange = RMobilePhone::ELockSetDisabled;
                    RDEBUG("lockChange", lockChange);
                    }
                }
            else
                {
                // Failed to get remote lock status
                RDEBUG("Failed", lockChange);
                }

            delete remoteLockSettings;
            remoteLockSettings = NULL;

            RDEBUG("lockChange", lockChange);
            wait = CWait::NewL();
            RDEBUG("0", 0);
            // this also calls PassPhraseRequiredL ???
            RDEBUG("SetLockSetting", 1);
            iPhone.SetLockSetting(wait->iStatus, lockType, lockChange);
            res = KErrNone;
            RDEBUG("WaitForRequestL", 0);
            res = wait->WaitForRequestL();
            RDEBUG("res", res);
            delete wait;
            } // from   period == 0
        else
            { // ask security code
            RDEBUG("codeQueryNotifier 0", 0);
            RNotifier codeQueryNotifier;
            User::LeaveIfError(codeQueryNotifier.Connect());
            CWait* wait = CWait::NewL();
            CleanupStack::PushL(wait);
            TInt queryResponse = 0;	// TODO will be changed by SecurityObserver
            TPckg<TInt> response(queryResponse);
            RDEBUG("0", 0);
            TSecurityNotificationPckg params;
            params().iEvent = static_cast<TInt> (0x100+RMobilePhone::EPhonePasswordRequired);
            params().iStartup = EFalse;
            RDEBUG("queryResponse", queryResponse);

            RDEBUG("StartNotifierAndGetResponse", 0);
            codeQueryNotifier.StartNotifierAndGetResponse(wait->iStatus, KSecurityNotifierUid, params, response);
            // this will eventually call PassPhraseRequiredL
            RDEBUG("WaitForRequestL", 0);
            res = wait->WaitForRequestL();
            RDEBUG("WaitForRequestL", 1);
            RDEBUG("res", res);
            CleanupStack::PopAndDestroy(); // wait
            if (res == KErrNone)
                res = queryResponse;
            } // from   else period == 0
        RDEBUG("0", 0);
        }
    else
        { // can't read repository for KSettingsAutoLockTime
        RDEBUG("KERRSOMETHING:Call SetLockSetting", 0);

        // If remote lock is enabled, don't disable the domestic OS device lock
        // since that would render the RemoteLock useless.
        // Instead just re-set the DOS lock to enabled which as a side effect
        // requests the security code from the user.

        TBool remoteLockStatus(EFalse);
        CRemoteLockSettings* remoteLockSettings = CRemoteLockSettings::NewL();

        if (remoteLockSettings->GetEnabled(remoteLockStatus))
            {
            if (remoteLockStatus)
                {
                // Remote lock is enabled
                lockChange = RMobilePhone::ELockSetEnabled;
                RDEBUG("lockChange", lockChange);
                }
            else
                {
                // Remote lock is disabled
                lockChange = RMobilePhone::ELockSetDisabled;
                RDEBUG("lockChange", lockChange);
                }
            }
        else
            {
            // Failed to get remote lock status
            RDEBUG("Failed", lockChange);
            }

        delete remoteLockSettings;
        remoteLockSettings = NULL;

        RDEBUG("0", 0);
        wait = CWait::NewL();
        RDEBUG("SetLockSetting", 0);
        iPhone.SetLockSetting(wait->iStatus, lockType, lockChange);
        RDEBUG("WaitForRequestL", 0);
        res = wait->WaitForRequestL();
        RDEBUG("WaitForRequestL res", res);
        delete wait;
        }

    RDEBUG("res", res);
    switch (res)
        {
        case KErrNone:
            {
            return ETrue;
            }
        case KErrGsmSSPasswordAttemptsViolation:
        case KErrLocked:
        case KErrGsm0707IncorrectPassword:
        case KErrAccessDenied:
            {
            RDEBUG("KErrAccessDenied", KErrAccessDenied);
            return AskSecCodeInAutoLockL();
            }
        case KErrInUse:
            {
            RDEBUG("KErrInUse", KErrInUse);
            return EFalse;
            }
        case KErrDied:
            {
            RDEBUG("KErrDied ", KErrDied);
            return EFalse;
            }
        case KErrServerTerminated:
            {
            RDEBUG("KErrServerTerminated ", KErrServerTerminated);
            return EFalse;
            }
        case KErrServerBusy:
            {
            RDEBUG("KErrServerBusy ", KErrServerBusy);
            return EFalse;
            }
        case KErrAbort:
            {
            RDEBUG("KErrAbort", KErrAbort);
            return EFalse;
            }
        case KErrCancel:
            {
            RDEBUG("KErrCancel", KErrCancel);
            // user pressed "cancel"
            return EFalse;
            }
        default:
            {
            RDEBUG("default", res);
            return AskSecCodeInAutoLockL();
            }
        }
    }
//
// ----------------------------------------------------------
// CSecurityHandler::PassPhraseRequired()    
// Handles PassPhraseRequired event
// ----------------------------------------------------------
// qtdone
TInt CSecurityHandler::PassPhraseRequiredL()
    {
    /*****************************************************
     *    Series 60 Customer / ETel
     *    Series 60  ETel API
     *****************************************************/
    askPassPhraseRequiredL:
    RDEBUG("0", 0);
    TBool StartUp = iStartup;

    RMobilePhone::TMobilePassword iSecUi_password;
    iSecUi_password.Copy(_L(""));
    RMobilePhone::TMobilePassword required_fourth;
    TInt queryAccepted = KErrCancel;

    TInt autolockState = 0;
    TInt lCancelSupported = 0;
    TInt lEmergencySupported = 0;

    TInt err(KErrGeneral);
    err = RProperty::Get(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, autolockState);
    RDEBUG("StartUp", StartUp);
    RDEBUG("err", err);
    if (!StartUp)
        {
        RDebug::Printf("%s %s (%u) might leave if StartUp=0 and err=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, err);
        User::LeaveIfError(err);
        }
    TBool isConditionSatisfied = EFalse;
    TInt tarmFlag = 0;
    if (FeatureManager::FeatureSupported(KFeatureIdSapTerminalControlFw))
        {
        TInt tRet = RProperty::Get(KSCPSIDAutolock, SCP_TARM_ADMIN_FLAG_UID, tarmFlag);

        if (tRet != KErrNone)
            {
            RDEBUG("Warning: failed to get TARM Admin Flag state", tRet);
            }
        else
            {
            RDEBUG("TARM flag", tarmFlag);
            }

        if ((StartUp) || (tarmFlag & KSCPFlagResyncQuery))
            isConditionSatisfied = ETrue;
        }
    else
        {
        if (StartUp)
            isConditionSatisfied = ETrue;
        }

    // Security code at bootup: No "cancel" softkey; Emergency calls enabled.
    RMobilePhone::TMobilePhoneSecurityCode secCodeTypeToAsk = RMobilePhone::ESecurityCodePhonePassword;
    RDEBUG("isConditionSatisfied", isConditionSatisfied);
    if (isConditionSatisfied)
        {
        // starter or special TARM. NoCancel+Emergency
        lCancelSupported = ESecUiCancelNotSupported;
        lEmergencySupported = ESecUiEmergencySupported;
        }
    else if (autolockState > EAutolockOff)
        {
        // from unlock. Cancel+Emergency
        lCancelSupported = ESecUiCancelSupported;
        lEmergencySupported = ESecUiEmergencySupported;
        }
    else
        {
        // from settings. Cancel+NoEmergency
        lCancelSupported = ESecUiCancelSupported;
        lEmergencySupported = ESecUiEmergencyNotSupported;
        }

    CSecQueryUi *iSecQueryUi;
    iSecQueryUi = CSecQueryUi::NewL();
    iQueryCanceled = EFalse;
    TInt lType = ESecUiSecretSupported | ESecUiAlphaSupported | lCancelSupported | lEmergencySupported | secCodeTypeToAsk;
    RDEBUG("lType", lType);
    TBuf<0x100> title;
    title.Zero();
    HBufC* stringHolder = HbTextResolverSymbian::LoadLC(_L("txt_pin_code_dialog_sec_code"));
    title.Append(stringHolder->Des());
    CleanupStack::PopAndDestroy(stringHolder);
    queryAccepted = iSecQueryUi->SecQueryDialog(title, iSecUi_password, SEC_C_SECURITY_CODE_MIN_LENGTH, SEC_C_SECURITY_CODE_MAX_LENGTH, lType);
    RDEBUG("iSecUi_password", 0);
    RDEBUGSTR(iSecUi_password);
    RDEBUG("queryAccepted", queryAccepted);
    iQueryCanceled = ETrue;
    delete iSecQueryUi;

    TBool wasCancelledOrEmergency = EFalse;
    RDEBUG("KFeatureIdSapDeviceLockEnhancements",
            KFeatureIdSapDeviceLockEnhancements);
    if ((queryAccepted == KErrAbort /* =emergency */) || (queryAccepted == KErrCancel))
        wasCancelledOrEmergency = ETrue;
    RDEBUG("wasCancelledOrEmergency", wasCancelledOrEmergency);
    if (wasCancelledOrEmergency)
        {
        RDEBUG("StartUp", StartUp);
        if (!StartUp)
            {
            RDEBUG("AbortSecurityCode", 0);
            iPhone.AbortSecurityCode(RMobilePhone::ESecurityCodePhonePassword);
            RDEBUG("AbortSecurityCode", 1);
            }
        return KErrCancel;
        }

    RMobilePhone::TMobilePhoneSecurityCode secCodeType = RMobilePhone::ESecurityCodePhonePassword;
    CWait* wait = NULL;
    TInt status = KErrNone;

		// Validate the code using SCP. This is needed to check expiration
    RDEBUG("AbortSecurityCode", 0);
    RSCPClient scpClient;
    RDEBUG("scpClient.Connect", 0);
    TInt tRet = scpClient.Connect();
    RDEBUG("tRet", tRet);

    CleanupClosePushL(scpClient);

    RArray<TDevicelockPolicies> aFailedPolicies;
    TDevicelockPolicies failedPolicy;
    TInt retLockcode = KErrNone;
    RMobilePhone::TMobilePassword aISACode;
    TInt scpFlags = 0;
    RDEBUG("scpClient.VerifyCurrentLockcode", 0);
    // this validate on ISA . No need to do iPhone.VerifySecurityCode
    retLockcode = scpClient.VerifyCurrentLockcode(iSecUi_password, aISACode, aFailedPolicies, scpFlags);
    RDEBUG("retLockcode", retLockcode);

    RDEBUG("aISACode", 0);
    RDEBUGSTR(aISACode);

    RDEBUG("aFailedPolicies.Count()", aFailedPolicies.Count());
    RDEBUG("EDeviceLockPasscodeExpiration", EDeviceLockPasscodeExpiration); // 8
    TInt failedEDeviceLockPasscodeExpiration = 0;
    for (TInt i = 0; i < aFailedPolicies.Count(); i++)
        {
        failedPolicy = aFailedPolicies[i];
        RDEBUG("failedPolicy", failedPolicy);
        if (failedPolicy == EDeviceLockPasscodeExpiration)
            failedEDeviceLockPasscodeExpiration = EDeviceLockPasscodeExpiration;
        }
    RDEBUG("Close", 0);
    scpClient.Close();
    RDEBUG("PopAndDestroy", 0);
    CleanupStack::PopAndDestroy(); //scpClient

		RDEBUG("failedEDeviceLockPasscodeExpiration", failedEDeviceLockPasscodeExpiration);
    if (failedEDeviceLockPasscodeExpiration > 0)
        {
    		// the password has expired. Note that it has NOT been validated. It will be revalidated inside ChangeSecCodeParamsL
        RMobilePhone::TMobilePassword iNewPassword;
        TInt iFlags = 0;
        iNewPassword.Copy(_L(""));
        TBuf<0x80> iCaption;
        iCaption.Copy(_L("ChangeSecCodeL")); // no need to translate because it's not used
        TInt iShowError = 1;
        CSecuritySettings* iSecSettings;
        iSecSettings = CSecuritySettings::NewL();
        TSecUi::InitializeLibL();
        RDEBUG("calling ChangeSecCodeParamsL", 0);
        status = iSecSettings->ChangeSecCodeParamsL(iSecUi_password, iNewPassword, iFlags, iCaption, iShowError);
        // note that it might have been aborted. The error KErrAbort is shown in this method, not inside ChangeSecCodeParamsL
        if(status==KErrAccessDenied || status==KErrGsm0707IncorrectPassword)
        	{
        	// the password expired, but the user typed wrongly. The error was already displayed.
        	RDEBUG("SCP returned ", KErrAccessDenied);
        	status=R_CODES_DONT_MATCH;
        	RDEBUG("SCP changed to R_CODES_DONT_MATCH", R_CODES_DONT_MATCH);
        	}
        RDEBUG("status", status);
        RDEBUG("iNewPassword", 0);
        RDEBUGSTR(iNewPassword);
        TSecUi::UnInitializeLib();
        RDEBUG("deleting iSecSettings", 0);
        delete iSecSettings;
        RDEBUG("deleted iSecSettings", 1);
        }
    else
        {
        RDEBUG( "wait", 0 );
        RDEBUG("iSecUi_password", 0);
        RDEBUGSTR(iSecUi_password);
        RDEBUG("aISACode", 0);
        RDEBUGSTR(aISACode);
        wait = CWait::NewL();
        RDEBUG("VerifySecurityCode", 0);
        iPhone.VerifySecurityCode(wait->iStatus, secCodeType, aISACode /* not iSecUi_password !!! */, required_fourth);
        RDEBUG("WaitForRequestL",
                0);
        status = wait->WaitForRequestL();
        RDEBUG("WaitForRequestL status",
                status);
        delete wait;
#ifdef __WINS__
        if (status == KErrNotSupported || status == KErrTimedOut)
            {
            RDEBUG( "status", status );
            status = KErrNone;
            }
#endif
        }

    TInt returnValue = status;
    RDEBUG("tarmFlag", tarmFlag);
    RDEBUG("StartUp", StartUp);
    RDEBUG("status", status);
    switch (status)
        {
        case KErrNone:
            // code approved
            {
            RDEBUG("KErrNone", KErrNone);
            if( failedEDeviceLockPasscodeExpiration > 0)
            	{
            	RDEBUG("code has just been changed. No need to show Confirmation note. failedEDeviceLockPasscodeExpiration", failedEDeviceLockPasscodeExpiration);
            	}
            else
            	{
            	CSecuritySettings::ShowResultNoteL(R_CONFIRMATION_NOTE, CAknNoteDialog::EConfirmationTone);
            	}
            RDEBUG("R_CONFIRMATION_NOTE", R_CONFIRMATION_NOTE);
            if (FeatureManager::FeatureSupported(KFeatureIdSapTerminalControlFw))
                {
                RDEBUG("KFeatureIdSapTerminalControlFw",
                        KFeatureIdSapTerminalControlFw);
                // Unset the admin flag if set
                if (tarmFlag & KSCPFlagResyncQuery)
                    {
                    TInt tRet = RProperty::Get(KSCPSIDAutolock, SCP_TARM_ADMIN_FLAG_UID, tarmFlag);

                    if (tRet == KErrNone)
                        {
                        tarmFlag &= ~KSCPFlagResyncQuery;
                        tRet = RProperty::Set(KSCPSIDAutolock, SCP_TARM_ADMIN_FLAG_UID, tarmFlag);
                        }

                    if (tRet != KErrNone)
                        {
                        RDEBUG(
                                "FAILED to unset TARM Admin Flag tRet=",
                                tRet);
                        }
                    }
                if (!FeatureManager::FeatureSupported(KFeatureIdSapDeviceLockEnhancements))
                    {
                    RDEBUG("KFeatureIdSapDeviceLockEnhancements", KFeatureIdSapDeviceLockEnhancements);
                    RSCPClient scpClient;
                    RDEBUG("scpClient.Connect", 0);
                    TInt tRet = scpClient.Connect();
                    RDEBUG("tRet", tRet);
                    User::LeaveIfError(tRet);
                    RDEBUG("scpClient.Connect", 1);
                    CleanupClosePushL(scpClient);
                    TSCPSecCode newCode;
                    newCode.Copy(iSecUi_password);	// this might fail if lenght=10 , because TSCPSecCode=8, while SEC_C_SECURITY_CODE_MAX_LENGTH=10
                    RDEBUG(
                            "!!!!!!! ***** TODO deprecated **** !!!!! scpClient.StoreCode",
                            0);
                    scpClient.StoreCode(newCode);
                    RDEBUG("scpClient.StoreCode", 1);
                    // TODO check whether code is expired.
                    // This should be done by TARM, and they should ask to change the code
                    CleanupStack::PopAndDestroy(); //scpClient
                    }

                }
            RDEBUG("StartUp", StartUp);
            if (StartUp)
                {
                // get autolock period from Central Repository.
                CRepository* repository = CRepository::NewL(KCRUidSecuritySettings);
                TInt period = 0;
                TInt res = repository->Get(KSettingsAutoLockTime, period);
                delete repository;
                RDEBUG("res", res);
                RDEBUG("period", period);
                _LIT_SECURITY_POLICY_PASS( KReadPolicy);
                _LIT_SECURITY_POLICY_C1(KWritePolicy,
                        ECapabilityWriteDeviceData);
                RProperty::Define(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, RProperty::EInt, KReadPolicy, KWritePolicy);
                RProperty::Set(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, EAutolockOff);
                RDEBUG("KCoreAppUIsAutolockStatus",
                        KCoreAppUIsAutolockStatus);

                if (res == KErrNone)
                    {
                    // disable autolock in Domestic OS side too if autolock period is 0.
                    if (period == 0)
                        {
                        // Don't disable the domestic OS device lock
                        // since that would render the RemoteLock useless.

                        TBool remoteLockStatus(EFalse);
                        CRemoteLockSettings* remoteLockSettings = CRemoteLockSettings::NewL();

                        if (remoteLockSettings->GetEnabled(remoteLockStatus))
                            {
                            if (!remoteLockStatus)
                                {
                                // Remote lock is disabled
                                RDEBUG(
                                        "Autolock and RemoteLock are disabled -> disable DOS device lock",
                                        0);
                                // Disable DOS device lock setting
                                RDEBUG(
                                        "iCustomPhone.DisablePhoneLock",
                                        0);
                                wait = CWait::NewL();
                                iCustomPhone.DisablePhoneLock(wait->iStatus, iSecUi_password);
                                RDEBUG("WaitForRequestL", 0);
                                status = wait->WaitForRequestL();
                                RDEBUG("WaitForRequestL", status);
                                delete wait;
                                }
                            }
                        else
                            {
                            // Failed to get remote lock status
                            RDEBUG(
                                    "Autolock is disabled, but failed to get RemoteLock status, so do nothing",
                                    0);
                            }

                        delete remoteLockSettings;
                        remoteLockSettings = NULL;

                        }
                    }
                else // error getting repository
                    {
                    RDEBUG("error getting repository", 0);
                    // Don't disable the domestic OS device lock
                    // since that would render the RemoteLock useless.

                    TBool remoteLockStatus(EFalse);
                    CRemoteLockSettings* remoteLockSettings = CRemoteLockSettings::NewL();

                    if (remoteLockSettings->GetEnabled(remoteLockStatus))
                        {
                        if (!remoteLockStatus)
                            {
                            // Remote lock is disabled
                            RDEBUG("iCustomPhone.DisablePhoneLock", 0);
                            wait = CWait::NewL();
                            iCustomPhone.DisablePhoneLock(wait->iStatus, iSecUi_password);
                            RDEBUG("WaitForRequestL", 0);
                            status = wait->WaitForRequestL();
                            RDEBUG("WaitForRequestL status", status);
                            delete wait;
                            }
                        }
                    else
                        {
                        // Failed to get remote lock status
                        RDEBUG(
                                "Failed to get Autolock period and RemoteLock status, so do nothing",
                                0);
                        }

                    delete remoteLockSettings;
                    remoteLockSettings = NULL;

                    }

                } // no Startup
            }
            break;
        case KErrGsmSSPasswordAttemptsViolation:
    				RDEBUG("KErrGsmSSPasswordAttemptsViolation", KErrGsmSSPasswordAttemptsViolation);
    				// and continue
        case KErrLocked:
        		{
            // security code blocked!
            RDEBUG("KErrLocked", KErrLocked)
            ;
            CSecuritySettings::ShowResultNoteL(R_SEC_BLOCKED, CAknNoteDialog::EErrorTone);
          	}
            break;
        case KErrGsm0707IncorrectPassword:
        		{
    				RDEBUG("KErrGsm0707IncorrectPassword", KErrGsm0707IncorrectPassword);
            // The Settings caller might retry
            CSecuritySettings::ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
            if(StartUp)
            	goto askPassPhraseRequiredL;
          	}
            break;
    				// and continue
        case KErrAccessDenied:
        		{
            RDEBUG("KErrAccessDenied", KErrAccessDenied);
            // The Settings caller might retry
            CSecuritySettings::ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
          	}
            break;
        case R_CODES_DONT_MATCH:	// if password was supposed to be changed, but it failed (the initial verification)
        		{
            RDEBUG("R_CODES_DONT_MATCH", R_CODES_DONT_MATCH);
            // Error was already displayed
            returnValue = KErrAccessDenied;
          	}
            break;
        default:
        		{
            RDEBUG("default", status);
            CSecuritySettings::ShowErrorNoteL(status);
            // The Settings caller might retry
          	}
            break;
        }
    RDEBUG("returnValue", returnValue);
    return returnValue;
    }
//
// ----------------------------------------------------------
// CSecurityHandler::Pin1Required()    
// Handles Pin1Required event
// ----------------------------------------------------------
// qtdone
TInt CSecurityHandler::Pin1RequiredL()
    {
    /*****************************************************
     *    Series 60 Customer / ETel
     *    Series 60  ETel API
     *****************************************************/
    RDEBUG("0", 0);

    RMobilePhone::TMobilePassword iSecUi_password;
    TInt lCancelSupported = ESecUiCancelNotSupported;
    TInt lEmergencySupported = ESecUiEmergencyNotSupported;
    TInt queryAccepted = KErrCancel;
    TInt lAlphaSupported = 0;
    RMobilePhone::TMobilePassword required_fourth;
    RMobilePhone::TMobilePhoneSecurityCode secCodeType = RMobilePhone::ESecurityCodePin1;
    RMobilePhone::TMobilePhoneSecurityCodeInfoV5 codeInfo;
    RMobilePhone::TMobilePhoneSecurityCodeInfoV5Pckg codeInfoPkg(codeInfo);
    TBool StartUp = ETrue;
    TInt secUiOriginatedQuery(ESecurityUIsSecUIOriginatedUninitialized);
    TInt err = KErrNone;
    TInt res = KErrGeneral;
    CWait* wait = CWait::NewL();
    CleanupStack::PushL(wait);
    RDEBUG("0", 0);

    StartUp = iStartup;

    RDEBUG("StartUp", StartUp);
    if (!StartUp)
        {
        // read a flag to see whether the query is SecUi originated. For example, from CSecuritySettings::ChangePinRequestParamsL
        err = RProperty::Get(KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery, secUiOriginatedQuery);
        if (err != KErrNone)
            {
            RDEBUG("FAILED to get the SECUI query Flag err", err);
            }
        }
    RDEBUG("err", err);
    RDEBUG("secUiOriginatedQuery", secUiOriginatedQuery);
    RDEBUG("ESecurityUIsSecUIOriginated", ESecurityUIsSecUIOriginated);
    if (StartUp || (secUiOriginatedQuery != ESecurityUIsSecUIOriginated) || (err != KErrNone))
        {
        RDEBUG("0", 0);
        lCancelSupported = ESecUiCancelNotSupported;
        lEmergencySupported = ESecUiEmergencySupported;
        }
    else
        {
        lCancelSupported = ESecUiCancelSupported;
        lEmergencySupported = ESecUiEmergencyNotSupported;
        // it will be RMobilePhone::ESecurityCodePin1 , equivalent to ESecUiNone
        }
    wait->SetRequestType(EMobilePhoneGetSecurityCodeInfo);
    RDEBUG("GetSecurityCodeInfo", 0);
    iPhone.GetSecurityCodeInfo(wait->iStatus, secCodeType, codeInfoPkg);
    res = wait->WaitForRequestL();

    RDEBUG("res", res);
#ifdef __WINS__
    RDEBUG("emulator can't read PIN attempts", res);
    res = KErrNone;
    codeInfo.iRemainingEntryAttempts = 3;
#endif
		RDEBUG("KErrPermissionDenied", KErrPermissionDenied);
    User::LeaveIfError(res);

    RDEBUG("codeInfo.iRemainingEntryAttempts",
            codeInfo.iRemainingEntryAttempts);
    if (codeInfo.iRemainingEntryAttempts == KMaxNumberOfPINAttempts)
        codeInfo.iRemainingEntryAttempts = -1;

    /* request PIN using QT */
    CSecQueryUi *iSecQueryUi;
    RDEBUG("CSecQueryUi", 0);
    iSecQueryUi = CSecQueryUi::NewL();
    iQueryCanceled = EFalse;
    RDEBUG("SecQueryDialog", 1);
    // ESecUiCodeEtelReqest/ESecUiNone might be useful
    lAlphaSupported = ESecUiAlphaNotSupported;
    TBuf<0x100> title;
    title.Zero();
    HBufC* stringHolder = HbTextResolverSymbian::LoadLC(_L("txt_pin_code_dialog_pin_code"));
    title.Append(stringHolder->Des());
    CleanupStack::PopAndDestroy(stringHolder);
    title.Append(_L("$"));
    title.AppendNum(codeInfo.iRemainingEntryAttempts);
    TInt amode = ESecUiSecretSupported | lAlphaSupported | lCancelSupported | lEmergencySupported | secCodeType;
    RDEBUG("amode", amode);
    queryAccepted = iSecQueryUi->SecQueryDialog(title, iSecUi_password, SEC_C_PIN_CODE_MIN_LENGTH, SEC_C_PIN_CODE_MAX_LENGTH, amode);
    RDEBUG("iSecUi_password", 0);
    RDEBUGSTR(iSecUi_password);
    iQueryCanceled = ETrue;
    delete iSecQueryUi;
    RDEBUG("queryAccepted", queryAccepted);
    /* end request PIN using QT */

    if (queryAccepted == KErrAbort) // emergency call
        {
        RDEBUG("queryAccepted", queryAccepted);
        CleanupStack::PopAndDestroy(wait); // this is needed
        return KErrCancel;
        }
    if (lCancelSupported && (queryAccepted == KErrCancel))
        {
        // cancel code request
        RDEBUG("AbortSecurityCode", 0);
        iPhone.AbortSecurityCode(RMobilePhone::ESecurityCodePin1);
        RDEBUG("AbortSecurityCode", 1);
        CleanupStack::PopAndDestroy(wait); // this is needed
        return KErrCancel;
        }

    RDEBUG("iSecUi_password", 0);
    RDEBUGSTR(iSecUi_password);
    RDEBUG("VerifySecurityCode", 0);
    iPhone.VerifySecurityCode(wait->iStatus, secCodeType, iSecUi_password, required_fourth);
    RDEBUG("WaitForRequestL", 0);
    res = wait->WaitForRequestL();
    RDEBUG("WaitForRequestL res", res);
    CleanupStack::PopAndDestroy(wait);

    TInt returnValue = res;
    switch (res)
        {
        case KErrNone:
            // code approved
            RDEBUG("code approved", res)
            ;
            if (lCancelSupported == ESecUiCancelNotSupported)
                { // OK note is not displayed in boot-up, to make things faster
                CSecuritySettings::ShowResultNoteL(R_CONFIRMATION_NOTE, CAknNoteDialog::EConfirmationTone);
                }
            break;
        case KErrGsm0707IncorrectPassword:
        case KErrAccessDenied:
            // code was entered erroneously
            CSecuritySettings::ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
           	RDEBUG("StartUp", StartUp);
            if (StartUp)
                {
                returnValue = Pin1RequiredL();
                }
            break;
        case KErrGsmSSPasswordAttemptsViolation:
        case KErrLocked:
            // code blocked; show error note and terminate.
            // what if not during Startup? Probably it's Ok since the SIM would had also failed at StartUp
           	RDEBUG("StartUp", StartUp);
            if (StartUp)
                CSecuritySettings::ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
            break;
        case KErrGsm0707SimWrong:
            // sim lock active
            // no error? This is strange
            break;
        default:
        		{
            CSecuritySettings::ShowErrorNoteL(res);
            if (StartUp)
                {
                returnValue = Pin1RequiredL();
                }
            }
            break;
        }
    return returnValue;
    }
//
// ----------------------------------------------------------
// CSecurityHandler::Puk1Required()
// Handles Puk1Required event
// First asks the PUK1, then verifies, then the newPIN + re-type , and then writes . This is odd, but the API needs the PUK and newPIN in same step.
// Afterwards, the PIN1 might be asked (because the initial program still needs it)
// ----------------------------------------------------------
// qtdone
TInt CSecurityHandler::Puk1RequiredL()
    {
    /*****************************************************
     *    Series 60 Customer / ETel
     *    Series 60  ETel API
     *****************************************************/
		askPuk1RequiredL:
    RDEBUG("0", 0);
    TInt queryAccepted = KErrCancel;
    RMobilePhone::TMobilePassword puk1_password;
    RMobilePhone::TMobilePassword aNewPinPassword;
    RMobilePhone::TMobilePhoneSecurityCodeInfoV5 codeInfo;
    RMobilePhone::TMobilePhoneSecurityCodeInfoV5Pckg codeInfoPkg(codeInfo);
    RMobilePhone::TMobilePhoneSecurityCode blockCodeType;
    blockCodeType = RMobilePhone::ESecurityCodePuk1;
    CWait* wait = CWait::NewL();
    CleanupStack::PushL(wait);

    TBool StartUp(ETrue);
    StartUp = iStartup;

    TInt res(KErrCancel); // for the first try
    wait->SetRequestType(EMobilePhoneGetSecurityCodeInfo);

    TInt thisTry = 0;

    // If there was a problem (as there might be in case we're dropping off SIM Access Profile); try again a couple of times.
    while (res != KErrNone && (thisTry++) <= KTriesToConnectServer)
        {
        if (thisTry > 0)
            User::After(KTimeBeforeRetryingRequest);
        RDEBUG("GetSecurityCodeInfo", 0);
        iPhone.GetSecurityCodeInfo(wait->iStatus, blockCodeType, codeInfoPkg);
        RDEBUG("WaitForRequestL", 0);
        res = wait->WaitForRequestL();
        RDEBUG("WaitForRequestL res", res);
        }
    RDEBUG("res", res);
    // If there's still an error we're doomed. Bail out.
    User::LeaveIfError(res);

    RDEBUG("StartUp", StartUp);
    RDEBUG("codeInfo.iRemainingEntryAttempts",
            codeInfo.iRemainingEntryAttempts);
    TInt attempts(codeInfo.iRemainingEntryAttempts);
    RDEBUG("attempts", attempts);
    // show the last "Code Error" note of PIN verify result here so it won't be left under the PUK1 dialog
    if (!StartUp && (attempts == KMaxNumberOfPUKAttempts))
        CSecuritySettings::ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);

    if (attempts == KMaxNumberOfPINAttempts)
        attempts = -1;

    CSecQueryUi *iSecQueryUi;
    RDEBUG("CSecQueryUi", 0);
    iSecQueryUi = CSecQueryUi::NewL();
    iQueryCanceled = EFalse;
    RDEBUG("SecQueryDialog", 1);
    // ESecUiCodeEtelReqest/ESecUiNone might be useful
    TBuf<0x100> title;
    title.Zero();
    HBufC* stringHolder = HbTextResolverSymbian::LoadLC(_L("txt_pin_code_dialog_puk_code"));
    title.Append(stringHolder->Des());
    CleanupStack::PopAndDestroy(stringHolder);
    title.Append(_L("$"));
    title.AppendNum(attempts);
    TInt lSecUiCancelSupported = ESecUiCancelSupported | ESecUiEmergencyNotSupported;
    RDEBUG("StartUp", 0);
    if (StartUp) // how to know whether PUK comes from failing at Starter, or failing at any other PIN (i.e. changing PIN, or changing PIN-request) ???
    		{
        lSecUiCancelSupported = ESecUiCancelNotSupported | ESecUiEmergencySupported;
        RDEBUG("new ", lSecUiCancelSupported);
      	}
    queryAccepted = iSecQueryUi->SecQueryDialog(title, puk1_password, SEC_C_PUK_CODE_MIN_LENGTH, SEC_C_PUK_CODE_MAX_LENGTH, ESecUiSecretNotSupported | ESecUiAlphaNotSupported
            | lSecUiCancelSupported | ESecUiPukRequired);
    RDEBUG("puk1_password", 0);
    RDEBUGSTR(puk1_password);
    iQueryCanceled = ETrue;
    delete iSecQueryUi;
    RDEBUG("queryAccepted", queryAccepted);

    if ((queryAccepted == KErrAbort) || (queryAccepted == KErrCancel))
        {
        CleanupStack::PopAndDestroy(wait); // this is needed
        return KErrCancel;
        }

    // send code
    // first we verify the puk. For this, we reset the PIN to the same as the PUK
    // Hopefully this will never fail in the case "new PIN too long"
    RDEBUG("VerifySecurityCode", 0);
    iPhone.VerifySecurityCode(wait->iStatus, blockCodeType, puk1_password, puk1_password);
    RDEBUG("WaitForRequestL", 0);
    res = wait->WaitForRequestL();
    RDEBUG("WaitForRequestL res", res);
    CleanupStack::PopAndDestroy(wait);

    TInt returnValue = res;
    switch (res)
        {
        case KErrNone:
            // code approved -> note . The process continue and new-pin is requested
            CSecuritySettings::ShowResultNoteL(res, CAknNoteDialog::EConfirmationTone);
            break;
        case KErrGsm0707IncorrectPassword:
        case KErrAccessDenied:
            // wrong PUK code -> note -> ask PUK code again        
            CSecuritySettings::ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
           	RDEBUG("goto askPuk1RequiredL", 0);
            goto askPuk1RequiredL;
            // break;
        case KErrGsm0707SimWrong:
            // sim lock active
            // no message ?
            break;
        case KErrGsmSSPasswordAttemptsViolation:
        case KErrLocked:
            // sim card rejected.
            break;
        default:
            CSecuritySettings::ShowErrorNoteL(res);
           	RDEBUG("goto askPuk1RequiredL", 0);
            goto askPuk1RequiredL;
            // break;
        }

    // Now the PUK1 is validated. It's time for asking the new PIN1
    RDEBUG("new wait", 0);
    wait = CWait::NewL();
    CleanupStack::PushL(wait);
        {
        // new-pin query
        CSecQueryUi * iSecQueryUi;
        RDEBUG("CSecQueryUi", 0);
        iSecQueryUi = CSecQueryUi::NewL();
        iQueryCanceled = EFalse;
        RDEBUG("SecQueryDialog", 1);
        TBuf<0x100> title;
        title.Zero();
        HBufC* stringHolder = HbTextResolverSymbian::LoadLC(_L("txt_pin_code_dialog_new_pin_code"));
        title.Append(stringHolder->Des());
        CleanupStack::PopAndDestroy(stringHolder);
        title.Append(_L("|"));
        HBufC* stringHolder2 = HbTextResolverSymbian::LoadLC(_L("txt_pin_code_dialog_verify_new_pin_code"));
        title.Append(stringHolder2->Des());
        CleanupStack::PopAndDestroy(stringHolder2);
        lSecUiCancelSupported = ESecUiCancelNotSupported; 	// initialy it was ESecUiCancelSupported , but an error said "Becaouse, if it is can be canceled, why desigh this step about newpincode confirm"
        																										// Somehow both scenarios make sense: User should not cancel because the code has been already changed to PUK.
        																										// On the other hand, this happened because user forgot the PIN. Now you know it: same as PUK. So user can cancel.
        RDEBUG("StartUp", 0);
		    if (StartUp) // how to know whether PUK comes from failing at Starter, or failing at any other PIN (i.e. changing PIN, or changing PIN-request) ???
		    		{
		    		lSecUiCancelSupported = ESecUiCancelNotSupported;
		    		}
        queryAccepted = iSecQueryUi->SecQueryDialog(title, aNewPinPassword, SEC_C_PIN_CODE_MIN_LENGTH, SEC_C_PIN_CODE_MAX_LENGTH, ESecUiAlphaNotSupported | lSecUiCancelSupported
                | ESecUiPukRequired);
        RDEBUG("aNewPinPassword", 0);
        RDEBUGSTR(aNewPinPassword);
        iQueryCanceled = ETrue;
        delete iSecQueryUi;
        RDEBUG("queryAccepted", queryAccepted);
        }

    if ((queryAccepted == KErrAbort) || (queryAccepted == KErrCancel))
        {
        CleanupStack::PopAndDestroy(wait);
        return KErrCancel;
        }

    // send code again, now with the user pin
    RDEBUG("VerifySecurityCode", 0);
    iPhone.VerifySecurityCode(wait->iStatus, blockCodeType, aNewPinPassword, puk1_password);	// TODO why not ChangeSecurityCode ?
    RDEBUG("WaitForRequestL", 0);
    res = wait->WaitForRequestL();
    RDEBUG("WaitForRequestL res", res);
    CleanupStack::PopAndDestroy(wait);

		// this can't fail, because PUK1 was just verified
    returnValue = res;
    switch (res)
        {
        case KErrNone:
            // code approved -> note
            CSecuritySettings::ShowResultNoteL(R_PIN_CODE_CHANGED_NOTE, CAknNoteDialog::EConfirmationTone);
            break;
        case KErrGsm0707IncorrectPassword:
        case KErrAccessDenied:
            // wrong PUK code -> note -> ask PUK code again        
            CSecuritySettings::ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
            returnValue = Puk1RequiredL();
            break;
        case KErrGsm0707SimWrong:
            // sim lock active
            // no message ?
            break;
        case KErrGsmSSPasswordAttemptsViolation:
        case KErrLocked:
            // sim card rejected.
            break;
        default:
            CSecuritySettings::ShowErrorNoteL(res);
            returnValue = Puk1RequiredL();
            break;
        }
    RDEBUG("returnValue", returnValue);
    return returnValue;
    }
//
// ----------------------------------------------------------
// CSecurityHandler::Pin2Required()
// Handles Pin2Required event
// ----------------------------------------------------------
// qtdone
void CSecurityHandler::Pin2RequiredL()
    {
    /*****************************************************
     *    Series 60 Customer / ETel
     *    Series 60  ETel API
     *****************************************************/

    RDEBUG("0", 0);
    TInt queryAccepted = KErrCancel;
    RMobilePhone::TMobilePassword iSecUi_password;
    RMobilePhone::TMobilePassword required_fourth;
    RMobilePhone::TMobilePhoneSecurityCode secCodeType(RMobilePhone::ESecurityCodePin2);
    RMobilePhone::TMobilePhoneSecurityCodeInfoV5 codeInfo;
    RMobilePhone::TMobilePhoneSecurityCodeInfoV5Pckg codeInfoPkg(codeInfo);
    CWait* wait = CWait::NewL();
    CleanupStack::PushL(wait);

    wait->SetRequestType(EMobilePhoneGetSecurityCodeInfo);
    RDEBUG("GetSecurityCodeInfo", 0);
    iPhone.GetSecurityCodeInfo(wait->iStatus, secCodeType, codeInfoPkg);
    RDEBUG("WaitForRequestL", 0);
    TInt ret = wait->WaitForRequestL();
    RDEBUG("WaitForRequestL ret", ret);
    User::LeaveIfError(ret);

    RDEBUG("codeInfo.iRemainingEntryAttempts",
            codeInfo.iRemainingEntryAttempts);
    if (codeInfo.iRemainingEntryAttempts == KMaxNumberOfPINAttempts)
        codeInfo.iRemainingEntryAttempts = -1;

    /* request PIN using QT */
    CSecQueryUi *iSecQueryUi;
    RDEBUG("CSecQueryUi", 0);
    iSecQueryUi = CSecQueryUi::NewL();
    iQueryCanceled = EFalse;
    RDEBUG("SecQueryDialog", 1);
    // ESecUiCodeEtelReqest/ESecUiNone might be useful against KLastRemainingInputAttempt

    TBuf<0x100> title;
    title.Zero();
    HBufC* stringHolder = HbTextResolverSymbian::LoadLC(_L("txt_pin_code_dialog_pin2_code"));
    title.Append(stringHolder->Des());
    CleanupStack::PopAndDestroy(stringHolder);
    title.Append(_L("$"));
    title.AppendNum(codeInfo.iRemainingEntryAttempts);
    queryAccepted = iSecQueryUi->SecQueryDialog(title, iSecUi_password, SEC_C_PIN2_CODE_MIN_LENGTH, SEC_C_PIN2_CODE_MAX_LENGTH, ESecUiSecretNotSupported | ESecUiAlphaNotSupported
            | ESecUiCancelSupported | secCodeType);
    RDEBUG("iSecUi_password", 0);
    RDEBUGSTR(iSecUi_password);
    RDEBUG("queryAccepted", queryAccepted);
    iQueryCanceled = ETrue;
    delete iSecQueryUi;

    // If failed or device became locked, any pending request should be cancelled.
    if (queryAccepted != KErrNone)
        {
        RDEBUG("AbortSecurityCode", 0);
        iPhone.AbortSecurityCode(secCodeType);
        RDEBUG("AbortSecurityCode", 1);
        CleanupStack::PopAndDestroy(wait);
        return;
        }

    RDEBUG("VerifySecurityCode", 0);
    iPhone.VerifySecurityCode(wait->iStatus, secCodeType, iSecUi_password, required_fourth);
    RDEBUG("WaitForRequestL", 0);
    TInt status = wait->WaitForRequestL();
    RDEBUG("WaitForRequestL status", status);
    CleanupStack::PopAndDestroy(wait);

    switch (status)
        {
        case KErrNone:
            break;
        case KErrGsm0707IncorrectPassword:
        case KErrAccessDenied:
            // code was entered erroneously
            CSecuritySettings::ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
            break;
        case KErrGsmSSPasswordAttemptsViolation:
        case KErrLocked:
            // blocked
            CSecuritySettings::ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
            break;
        default:
            CSecuritySettings::ShowErrorNoteL(status);
            break;
        }
    }
//
// ----------------------------------------------------------
// CSecurityHandler::Puk2Required()
// Handles Puk2Required event
// ----------------------------------------------------------
// qtdone
void CSecurityHandler::Puk2RequiredL()
    {
    /*****************************************************
     *    Series 60 Customer / ETel
     *    Series 60  ETel API
     *****************************************************/
    askPuk2RequiredL:
    RDEBUG("0", 0);
    TInt queryAccepted = KErrCancel;
    RMobilePhone::TMobilePassword iSecUi_password;
    RMobilePhone::TMobilePassword aNewPassword;
    RMobilePhone::TMobilePassword verifcationPassword;
    RMobilePhone::TMobilePhoneSecurityCodeInfoV5 codeInfo;
    RMobilePhone::TMobilePhoneSecurityCodeInfoV5Pckg codeInfoPkg(codeInfo);

    RMobilePhone::TMobilePhoneSecurityCode secCodeType = RMobilePhone::ESecurityCodePuk2;
    CWait* wait = CWait::NewL();
    CleanupStack::PushL(wait);

    // ask PUK2

    TInt ret(KErrNone);
    wait->SetRequestType(EMobilePhoneGetSecurityCodeInfo);
    RDEBUG("GetSecurityCodeInfo", 0);
    iPhone.GetSecurityCodeInfo(wait->iStatus, secCodeType, codeInfoPkg);
    RDEBUG("WaitForRequestL", 0);
    ret = wait->WaitForRequestL();
    RDEBUG("WaitForRequestL ret", ret);
    User::LeaveIfError(ret);

    /* request PIN using QT */
    CSecQueryUi *iSecQueryUi;
    RDEBUG("CSecQueryUi", 0);
    iSecQueryUi = CSecQueryUi::NewL();
    iQueryCanceled = EFalse;
    RDEBUG("SecQueryDialog", 1);
    // ESecUiCodeEtelReqest/ESecUiNone might be useful

    RDEBUG("codeInfo.iRemainingEntryAttempts",
            codeInfo.iRemainingEntryAttempts);
    if (codeInfo.iRemainingEntryAttempts == KMaxNumberOfPINAttempts) // TODO this might be 10 ?
        codeInfo.iRemainingEntryAttempts = -1;

    TBuf<0x100> title;
    title.Zero();
    HBufC* stringHolder = HbTextResolverSymbian::LoadLC(_L("txt_pin_code_dialog_puk2_code"));
    title.Append(stringHolder->Des());
    CleanupStack::PopAndDestroy(stringHolder);
    title.Append(_L("$"));
    title.AppendNum(codeInfo.iRemainingEntryAttempts);
    queryAccepted = iSecQueryUi->SecQueryDialog(title, iSecUi_password, SEC_C_PUK2_CODE_MIN_LENGTH, SEC_C_PUK2_CODE_MAX_LENGTH, ESecUiSecretNotSupported | ESecUiAlphaNotSupported
            | ESecUiCancelSupported | secCodeType);
    RDEBUG("iSecUi_password", 0);
    RDEBUGSTR(iSecUi_password);
    iQueryCanceled = ETrue;
    delete iSecQueryUi;
    RDEBUG("queryAccepted", queryAccepted);

    if (queryAccepted != KErrNone)
        {
        // cancel "get security unblock code" request
        RDEBUG("AbortSecurityCode", 0);
        iPhone.AbortSecurityCode(secCodeType);
        RDEBUG("AbortSecurityCode", 1);
        CleanupStack::PopAndDestroy(1); //wait
        return;
        }

    RDEBUG("VerifySecurityCode", 0);
    iPhone.VerifySecurityCode(wait->iStatus, secCodeType, iSecUi_password, iSecUi_password);
    // this writes the newPIN2 with the value of PUK2. It's ok, since the user forgot it
    RDEBUG("WaitForRequestL", 0);
    TInt res = wait->WaitForRequestL();
    RDEBUG("WaitForRequestL res", res);
    CleanupStack::PopAndDestroy(wait);

    switch (res)
        {
        case KErrNone:
            // code approved -> note
            CSecuritySettings::ShowResultNoteL(res, CAknNoteDialog::EConfirmationTone);
            break;
        case KErrGsm0707IncorrectPassword:
        case KErrAccessDenied:
            // wrong PUK2 code -> note -> ask PUK2 code again        
            CSecuritySettings::ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
            RDEBUG("goto askPuk2RequiredL", 0);
            goto askPuk2RequiredL;
            // break;
        case KErrGsmSSPasswordAttemptsViolation:
        case KErrLocked:
            // Pin2 features blocked permanently!
            CSecuritySettings::ShowResultNoteL(R_PIN2_REJECTED, CAknNoteDialog::EConfirmationTone);
            break;
        default:
            CSecuritySettings::ShowErrorNoteL(res);
            RDEBUG("goto askPuk2RequiredL", 0);
            goto askPuk2RequiredL;
            // break;
        }

    // now the PUK2 is valid. Time to get the new PIN2
    wait = CWait::NewL();
    CleanupStack::PushL(wait);
        {
        // new pin2 code query
        CSecQueryUi * iSecQueryUi;
        RDEBUG("CSecQueryUi", 0);
        iSecQueryUi = CSecQueryUi::NewL();
        iQueryCanceled = EFalse;
        TBuf<0x100> title;
        title.Zero();
        HBufC* stringHolder = HbTextResolverSymbian::LoadLC(_L("txt_pin_code_dialog_new_pin2_code"));
        title.Append(stringHolder->Des());
        CleanupStack::PopAndDestroy(stringHolder);
        title.Append(_L("|"));
        HBufC* stringHolder2 = HbTextResolverSymbian::LoadLC(_L("Verify"));
        title.Append(stringHolder2->Des());
        CleanupStack::PopAndDestroy(stringHolder2);
        // ESecUiCodeEtelReqest/ESecUiNone might be useful
        queryAccepted = iSecQueryUi->SecQueryDialog(title, aNewPassword, SEC_C_PIN2_CODE_MIN_LENGTH, SEC_C_PIN2_CODE_MAX_LENGTH, ESecUiAlphaNotSupported | ESecUiCancelSupported
                | secCodeType);
        RDEBUG("aNewPassword", 0);
        RDEBUGSTR(aNewPassword);
        iQueryCanceled = ETrue;
        delete iSecQueryUi;
        RDEBUG("queryAccepted", queryAccepted);
        if (queryAccepted != KErrNone)
            {
            // cancel "get security unblock code" request
            RDEBUG("AbortSecurityCode", 0);
            iPhone.AbortSecurityCode(secCodeType);
            RDEBUG("AbortSecurityCode", 1);
            CleanupStack::PopAndDestroy(1); //wait
            return;
            }
        }
    // send code. The code was temporarilly changed before. Thus, this really done to set the new-new one
    RDEBUG("VerifySecurityCode", 0);
    iPhone.VerifySecurityCode(wait->iStatus, secCodeType, aNewPassword, iSecUi_password);
    RDEBUG("WaitForRequestL", 0);
    res = wait->WaitForRequestL();
    RDEBUG("WaitForRequestL res", res);
    CleanupStack::PopAndDestroy(wait);

		// this can't fail, as PUK2 was just verified
    switch (res)
        {
        case KErrNone:
            // code approved -> note
            CSecuritySettings::ShowResultNoteL(R_PIN2_CODE_CHANGED_NOTE, CAknNoteDialog::EConfirmationTone);
            break;
        case KErrGsm0707IncorrectPassword:
        case KErrAccessDenied:
            // wrong PUK2 code -> note -> ask PUK2 code again        
            CSecuritySettings::ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
            Puk2RequiredL();
            break;
        case KErrGsmSSPasswordAttemptsViolation:
        case KErrLocked:
            // Pin2 features blocked permanently!
            CSecuritySettings::ShowResultNoteL(R_PIN2_REJECTED, CAknNoteDialog::EConfirmationTone);
            break;
        default:
            CSecuritySettings::ShowErrorNoteL(res);
            Puk2RequiredL();
            break;
        }
    }

//
// ----------------------------------------------------------
// CSecurityHandler::UPinRequiredL()
// Hendles UniversalPinRequired event
// ----------------------------------------------------------
// qtdone
TInt CSecurityHandler::UPinRequiredL()
    {
    /*****************************************************
     *    Series 60 Customer / ETel
     *    Series 60  ETel API
     *****************************************************/
    TBool wcdmaSupported(FeatureManager::FeatureSupported(KFeatureIdProtocolWcdma));
    TBool upinSupported(FeatureManager::FeatureSupported(KFeatureIdUpin));
    RDEBUG("wcdmaSupported", wcdmaSupported);
    RDEBUG("upinSupported", upinSupported);
    if (wcdmaSupported || upinSupported)
        {
        TInt queryAccepted = KErrCancel;
        TInt lCancelSupported = ESecUiCancelNotSupported;
        RMobilePhone::TMobilePassword iSecUi_password;
        RMobilePhone::TMobilePassword required_fourth;
        RMobilePhone::TMobilePhoneSecurityCodeInfoV5 codeInfo;
        RMobilePhone::TMobilePhoneSecurityCodeInfoV5Pckg codeInfoPkg(codeInfo);
        RMobilePhone::TMobilePhoneSecurityCode secCodeType = RMobilePhone::ESecurityUniversalPin;
        CWait* wait = CWait::NewL();
        CleanupStack::PushL(wait);
        TBool StartUp = ETrue;
        TInt secUiOriginatedQuery(ESecurityUIsSecUIOriginatedUninitialized);
        TInt err = KErrNone;
        TInt res = KErrGeneral;

        StartUp = iStartup;
        RDEBUG("StartUp", StartUp);

        wait->SetRequestType(EMobilePhoneGetSecurityCodeInfo);
        RDEBUG("GetSecurityCodeInfo", 0);
        iPhone.GetSecurityCodeInfo(wait->iStatus, secCodeType, codeInfoPkg);
        RDEBUG("WaitForRequestL", 0);
        res = wait->WaitForRequestL();
        RDEBUG("WaitForRequestL res", res);
        User::LeaveIfError(res);

        if (!StartUp)
            {
            // read a flag to see whether the query is SecUi originated. 
            err = RProperty::Get(KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery, secUiOriginatedQuery);
            }

        /* request PIN using QT */
        CSecQueryUi *iSecQueryUi;
        RDEBUG("CSecQueryUi", 0);
        iSecQueryUi = CSecQueryUi::NewL();
        iQueryCanceled = EFalse;
        // ESecUiCodeEtelReqest/ESecUiNone might be useful
        // TODO also support Emergency
        if (StartUp || (secUiOriginatedQuery != ESecurityUIsSecUIOriginated) || (err != KErrNone))
            lCancelSupported = ESecUiCancelNotSupported;
        else
            lCancelSupported = ESecUiCancelSupported;

        TBuf<0x100> title;
        title.Zero();
        HBufC* stringHolder = HbTextResolverSymbian::LoadLC(_L("txt_pin_code_dialog_upin_code"));
        title.Append(stringHolder->Des());
        CleanupStack::PopAndDestroy(stringHolder);
        title.Append(_L("$"));
        title.AppendNum(codeInfo.iRemainingEntryAttempts);
        queryAccepted = iSecQueryUi->SecQueryDialog(title, iSecUi_password, SEC_C_PIN_CODE_MIN_LENGTH, SEC_C_PIN_CODE_MAX_LENGTH, ESecUiSecretSupported | ESecUiAlphaNotSupported
                | lCancelSupported | ESecUiCodeEtelReqest);
        RDEBUG("iSecUi_password", 0);
        RDEBUGSTR(iSecUi_password);
        iQueryCanceled = ETrue;
        delete iSecQueryUi;
        RDEBUG("queryAccepted", queryAccepted);
        if (queryAccepted != KErrNone)
            {
            CleanupStack::PopAndDestroy(wait);
            RDEBUG("AbortSecurityCode", 0);
            iPhone.AbortSecurityCode(RMobilePhone::ESecurityUniversalPin);
            RDEBUG("AbortSecurityCode", 1);

            return KErrCancel;
            }
        RDEBUG("VerifySecurityCode", 0);
        iPhone.VerifySecurityCode(wait->iStatus, secCodeType, iSecUi_password, required_fourth);
        RDEBUG("WaitForRequestL", 0);
        res = wait->WaitForRequestL();
        RDEBUG("WaitForRequestL res", res);
        CleanupStack::PopAndDestroy(wait);

        TInt returnValue = res;
        switch (res)
            {
            case KErrNone:
                // code approved 
                RDEBUG("code approved ", 0)
                ;
                CSecuritySettings::ShowResultNoteL(R_CONFIRMATION_NOTE, CAknNoteDialog::EConfirmationTone);
                break;
            case KErrGsm0707IncorrectPassword:
            case KErrAccessDenied:
                // code was entered erroneously
                CSecuritySettings::ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
                if (StartUp)
                    {
                    returnValue = UPinRequiredL();
                    }
                break;
            case KErrGsmSSPasswordAttemptsViolation:
            case KErrLocked:
                // code blocked; show error note and terminate.
                if (StartUp)
                    CSecuritySettings::ShowResultNoteL(res, CAknNoteDialog::EErrorTone);
                break;
            case KErrGsm0707SimWrong:
                // sim lock active
                break;
            default: // for example, KErrArgument
                CSecuritySettings::ShowErrorNoteL(res);
                if (StartUp)
                    {
                    returnValue = UPinRequiredL();
                    }
                break;
            }

        return returnValue;
        }
    else
        return KErrNone;
    }
//
// ----------------------------------------------------------
// CSecurityHandler::UPukRequiredL()
// Handles UPukRequired event
// ----------------------------------------------------------
// qtdone
TInt CSecurityHandler::UPukRequiredL()
    {
    TBool wcdmaSupported(FeatureManager::FeatureSupported(KFeatureIdProtocolWcdma));
    TBool upinSupported(FeatureManager::FeatureSupported(KFeatureIdUpin));
    if (wcdmaSupported || upinSupported)
        {
        RDEBUG("0", 0);
        TInt queryAccepted = KErrCancel;
        RMobilePhone::TMobilePassword iSecUi_password;
        RMobilePhone::TMobilePassword aNewPassword;
        RMobilePhone::TMobilePhoneSecurityCodeInfoV5 codeInfo;
        RMobilePhone::TMobilePhoneSecurityCodeInfoV5Pckg codeInfoPkg(codeInfo);

        RMobilePhone::TMobilePhoneSecurityCode blockCodeType;
        blockCodeType = RMobilePhone::ESecurityUniversalPuk;
        CWait* wait = CWait::NewL();
        CleanupStack::PushL(wait);

        TBool StartUp(ETrue);
        StartUp = iStartup;

        TInt res(KErrNone);
        wait->SetRequestType(EMobilePhoneGetSecurityCodeInfo);
        RDEBUG("GetSecurityCodeInfo", 0);
        iPhone.GetSecurityCodeInfo(wait->iStatus, blockCodeType, codeInfoPkg);
        RDEBUG("WaitForRequestL", 0);
        res = wait->WaitForRequestL();
        RDEBUG("WaitForRequestL res", res);
        User::LeaveIfError(res);
        //show last "Code Error" note for UPIN verify result so it won't be left under the PUK1 dialog
        if (!StartUp && (codeInfo.iRemainingEntryAttempts == KMaxNumberOfPUKAttempts))
            CSecuritySettings::ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);

        // ask UPUK code
        /* request PIN using QT */
            {
            CSecQueryUi *iSecQueryUi;
            RDEBUG("CSecQueryUi", 0);
            iSecQueryUi = CSecQueryUi::NewL();
            iQueryCanceled = EFalse;
            // TODO ESecUiCodeEtelReqest/ESecUiNone might be useful
            // TODO also support Emergency

            TBuf<0x100> title;
            title.Zero();
            HBufC* stringHolder = HbTextResolverSymbian::LoadLC(_L("txt_pin_code_dialog_puk_code"));
            title.Append(stringHolder->Des());
            CleanupStack::PopAndDestroy(stringHolder);
            title.Append(_L("$"));
            title.AppendNum(codeInfo.iRemainingEntryAttempts);
            queryAccepted = iSecQueryUi->SecQueryDialog(title, iSecUi_password, SEC_C_PUK_CODE_MIN_LENGTH, SEC_C_PUK_CODE_MAX_LENGTH, ESecUiAlphaNotSupported
                    | ESecUiCancelSupported | ESecUiPukRequired);
            RDEBUG("iSecUi_password", 0);
            RDEBUGSTR(iSecUi_password);
            iQueryCanceled = ETrue;
            delete iSecQueryUi;
            RDEBUG("queryAccepted", queryAccepted);

            if (queryAccepted != KErrNone)
                {
                CleanupStack::PopAndDestroy(wait);
                return KErrCancel;
                }
            }

            {
            /* request PIN using QT */
            CSecQueryUi *iSecQueryUi;
            RDEBUG("CSecQueryUi", 0);
            iSecQueryUi = CSecQueryUi::NewL();
            iQueryCanceled = EFalse;
            // TODO ESecUiCodeEtelReqest/ESecUiNone might be useful
            // TODO also support Emergency
            TBuf<0x100> title;
            title.Zero();
            HBufC* stringHolder = HbTextResolverSymbian::LoadLC(_L("txt_pin_code_dialog_new_upin_code"));
            title.Append(stringHolder->Des());
            CleanupStack::PopAndDestroy(stringHolder);
            title.Append(_L("|"));
            HBufC* stringHolder2 = HbTextResolverSymbian::LoadLC(_L("Verify"));
            title.Append(stringHolder2->Des());
            CleanupStack::PopAndDestroy(stringHolder2);
            queryAccepted = iSecQueryUi->SecQueryDialog(title, aNewPassword, SEC_C_PUK_CODE_MIN_LENGTH, SEC_C_PUK_CODE_MAX_LENGTH, ESecUiAlphaNotSupported | ESecUiCancelSupported
                    | ESecUiPukRequired);
            RDEBUG("aNewPassword", 0);
            RDEBUGSTR(aNewPassword);
            iQueryCanceled = ETrue;
            delete iSecQueryUi;
            RDEBUG("queryAccepted", queryAccepted);
            if (queryAccepted != KErrNone)
                {
                CleanupStack::PopAndDestroy(wait);
                return KErrCancel;
                }
            }

        // send code
        RDEBUG("VerifySecurityCode", 0);
        iPhone.VerifySecurityCode(wait->iStatus, blockCodeType, aNewPassword, iSecUi_password);
        RDEBUG("WaitForRequestL", 0);
        res = wait->WaitForRequestL();
        RDEBUG("WaitForRequestL res", res);
        CleanupStack::PopAndDestroy(wait);

        TInt returnValue = res;
        switch (res)
            {
            case KErrNone:
                // code approved -> note
                CSecuritySettings::ShowResultNoteL(R_UPIN_CODE_CHANGED_NOTE, CAknNoteDialog::EConfirmationTone);
                break;
            case KErrGsm0707IncorrectPassword:
            case KErrAccessDenied:
                // wrong PUK code -> note -> ask UPUK code again        
                CSecuritySettings::ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
                returnValue = UPukRequiredL();
                break;
            case KErrGsm0707SimWrong:
                // sim lock active
                break;
            case KErrGsmSSPasswordAttemptsViolation:
            case KErrLocked:
                // sim card rejected.
                break;
            default:
                CSecuritySettings::ShowErrorNoteL(res);
                returnValue = UPukRequiredL();
                break;
            }

        return returnValue;
        }
    else
        // not wcdmaSupported || upinSupported
        return KErrNone;
    }

//
// ----------------------------------------------------------
// CSecurityHandler::SimLockEventL()
// Shows "SIM restriction on" note
// ----------------------------------------------------------
// qtdone
void CSecurityHandler::SimLockEventL()
    {
    RDEBUG("0", 0);
    CSecuritySettings::ShowResultNoteL(R_SIM_ON, CAknNoteDialog::EConfirmationTone);
    }
// ---------------------------------------------------------
// CSecurityHandler::RemoveSplashScreenL()
// Removes splash screen
// ---------------------------------------------------------
void CSecurityHandler::RemoveSplashScreenL() const
    {

    }

// ---------------------------------------------------------
// CSecurityHandler::ShowGenericErrorNoteL(TInt aStatus)
// Shows a generic error note
// ---------------------------------------------------------
// qtdone
void CSecurityHandler::ShowGenericErrorNoteL(TInt aStatus)
    {
    // Let's create TextResolver instance for error resolving...
    RDEBUG("aStatus", aStatus);
    RDEBUG("!!!!! this should never be called !!!!", 0);

    CSecuritySettings::ShowResultNoteL(aStatus, CAknNoteDialog::EErrorTone);
    }

// End of file
