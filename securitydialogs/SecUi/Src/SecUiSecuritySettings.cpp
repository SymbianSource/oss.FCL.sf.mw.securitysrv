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
* Description:  Provides api for changing security settings.
*
*
*/

#include <etelmm.h>
#include <exterror.h>
#include <textresolver.h>
#include <aknnotedialog.h>
#include <mmtsy_names.h>
#include <centralrepository.h> 
#include <gsmerror.h>
#include <SCPClient.h>
#include <StringLoader.h>
#include <e32property.h>
#include <PSVariables.h>   // Property values
#include <securityuisprivatepskeys.h>
#include <startupdomainpskeys.h>
#include "SecUi.h"
#include "secuisecuritysettings.h"
#include "secui.hrh"
#include "secuisecurityhandler.h"
#include "secuicodequerydialog.h"
#include "SecUiWait.h"

#include <aknnotewrappers.h>
#include <StringLoader.h>
#include <RemoteLockSettings.h>
#include <featmgr.h>

#include "SecQueryUi.h"
#include <hb/hbwidgets/hbdevicemessageboxsymbian.h>

/*****************************************************
 *    Series 60 Customer / TSY
 *    Needs customer TSY implementation
 *****************************************************/
//  LOCAL CONSTANTS AND MACROS  

const TInt KTriesToConnectServer(2);
const TInt KTimeBeforeRetryingServerConnection(50000);
const TInt PhoneIndex(0);

const TInt KMaxNumberOfPINAttempts(3); // is this valid also for PIN2 ?

#define ESecUiTypeLock                  0x00100000

// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CSecuritySettings::NewL()
// ----------------------------------------------------------
// qtdone
EXPORT_C CSecuritySettings* CSecuritySettings::NewL()
    {
    CSecuritySettings* self = new (ELeave) CSecuritySettings();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(); //self
    return self;
    }
//
// ----------------------------------------------------------
// CSecuritySettings::CSecuritySettings()
// constructor
// ----------------------------------------------------------
// qtdone
EXPORT_C CSecuritySettings::CSecuritySettings()
    {
    }
//
// ----------------------------------------------------------
// CSecuritySettings::ConstructL()
// Symbian OS constructor.
// ----------------------------------------------------------
// qtdone
EXPORT_C void CSecuritySettings::ConstructL()
    {
    /*****************************************************
     *    Series 60 Customer / ETel
     *    Series 60  ETel API
     *****************************************************/
    /*****************************************************
     *    Series 60 Customer / TSY
     *    Needs customer TSY implementation
     *****************************************************/

		RDEBUG("0", 0);
    TInt err(KErrGeneral);
    TInt thisTry(0);
    iWait = CWait::NewL();
    RTelServer::TPhoneInfo PhoneInfo;
    /* All server connections are tried to be made KTriesToConnectServer times because occasional
     fails on connections are possible, at least on some servers */

    FeatureManager::InitializeLibL();
    // connect to ETel server
    while ((err = iServer.Connect()) != KErrNone && (thisTry++)
            <= KTriesToConnectServer)
        {
        User::After( KTimeBeforeRetryingServerConnection);
        }
    User::LeaveIfError(err);

    // load TSY
    err = iServer.LoadPhoneModule(KMmTsyModuleName);
    if (err != KErrAlreadyExists)
        {
        // May return also KErrAlreadyExists if something else
        // has already loaded the TSY module. And that is
        // not an error.
        User::LeaveIfError(err);
        }

    // open phones
    User::LeaveIfError(iServer.SetExtendedErrorGranularity(
            RTelServer::EErrorExtended));
    User::LeaveIfError(iServer.GetPhoneInfo(PhoneIndex, PhoneInfo));
    User::LeaveIfError(iPhone.Open(iServer, PhoneInfo.iName));
    User::LeaveIfError(iCustomPhone.Open(iPhone));

    iSecurityHandler = new (ELeave) CSecurityHandler(iPhone);
    }
//
// ----------------------------------------------------------
// CSecuritySettings::~CSecuritySettings()
// Destructor
// ----------------------------------------------------------
// qtdone
EXPORT_C CSecuritySettings::~CSecuritySettings()
    {
    /*****************************************************
     *    Series 60 Customer / ETel
     *    Series 60  ETel API
     *****************************************************/
    /*****************************************************
     *    Series 60 Customer / TSY
     *    Needs customer TSY implementation
     *****************************************************/
    delete iSecurityHandler;

    // Cancel active requests
    if (iWait->IsActive())
        {
				RDEBUG("CancelAsyncRequest", 0);
        iPhone.CancelAsyncRequest(iWait->GetRequestType());

        switch (iWait->GetRequestType())
            { //inform query that it has beeen canceled
            case EMobilePhoneSetLockSetting:
            case EMobilePhoneSetFdnSetting:
                RProperty::Set(KPSUidSecurityUIs,
                        KSecurityUIsQueryRequestCancel,
                        ESecurityUIsQueryRequestCanceled);
                break;
            default:
                break;
            }

        }
    // close phone
    if (iPhone.SubSessionHandle())
        iPhone.Close();
    // close custom phone
    if (iCustomPhone.SubSessionHandle())
        iCustomPhone.Close();
    //close ETel connection
    if (iServer.Handle())
        {
        iServer.UnloadPhoneModule(KMmTsyModuleName);
        iServer.Close();
        }
    delete iWait;
    FeatureManager::UnInitializeLib();
    }
//
// ----------------------------------------------------------
// CSecuritySettings::ChangePinL()
// Changes PIN1
// ----------------------------------------------------------
// qtdone
EXPORT_C void CSecuritySettings::ChangePinL()
    {
    RDEBUG("0", 0);
    RMobilePhone::TMobilePassword iOldPassword;
    RMobilePhone::TMobilePassword iNewPassword;
    TInt iFlags = ESecUiTypeLock;
    iOldPassword.Copy(_L(""));
    iNewPassword.Copy(_L(""));

    TBuf<0x80> iCaption;
    iCaption.Copy(_L("ChangePinL"));
    TInt iShowError = 1;
    TInt err = ChangePinParamsL(iOldPassword, iNewPassword, iFlags, iCaption, iShowError);
    RDEBUG("err", err);
    }

//
// ----------------------------------------------------------
// CSecuritySettings::ChangeUPinL()
// Changes Universal PIN
// ----------------------------------------------------------
// qtdone
EXPORT_C void CSecuritySettings::ChangeUPinL()
    {
    RDEBUG("0", 0);
    RMobilePhone::TMobilePassword iOldPassword;
    RMobilePhone::TMobilePassword iNewPassword;
    TInt iFlags = ESecUiTypeLock;
    iOldPassword.Copy(_L(""));
    iNewPassword.Copy(_L(""));

    TBuf<0x80> iCaption;
    iCaption.Copy(_L("ChangeUPinL"));
    TInt iShowError = 1;
    TInt err = ChangeUPinParamsL(iOldPassword, iNewPassword, iFlags, iCaption,
            iShowError);
    RDEBUG("err", err);

    }

//
// ----------------------------------------------------------
// CSecuritySettings::ChangePin2L()
// Changes PIN2
// ----------------------------------------------------------
// qtdone
EXPORT_C void CSecuritySettings::ChangePin2L()
    {
    RDEBUG("0", 0);
    RMobilePhone::TMobilePassword iOldPassword;
    RMobilePhone::TMobilePassword iNewPassword;
    TInt iFlags = ESecUiTypeLock;
    iOldPassword.Copy(_L(""));
    iNewPassword.Copy(_L(""));

    TBuf<0x80> iCaption;
    iCaption.Copy(_L("ChangePin2L"));
    TInt iShowError = 1;
    TInt err = ChangePin2ParamsL(iOldPassword, iNewPassword, iFlags, iCaption,
            iShowError);
    RDEBUG("err", err);
    }
//
// ----------------------------------------------------------
// CSecuritySettings::ChangeSecCodeL()
// Changes security code 
// ----------------------------------------------------------
// qtdone
EXPORT_C void CSecuritySettings::ChangeSecCodeL()
    {
    RDEBUG("0", 0);
    RMobilePhone::TMobilePassword iOldPassword;
    RMobilePhone::TMobilePassword iNewPassword;
    TInt iFlags = 0;
    iOldPassword.Copy(_L(""));
    iNewPassword.Copy(_L(""));

    TBuf<0x80> iCaption;
    iCaption.Copy(_L("ChangeSecCodeL"));
    TInt iShowError = 1;
    TInt err = ChangeSecCodeParamsL(iOldPassword, iNewPassword, iFlags, iCaption,
            iShowError);
    RDEBUG("err", err);
    }
//
// ----------------------------------------------------------
// CSecuritySettings::ChangeAutoLockPeriodL()
// Changes autolock period
// ----------------------------------------------------------
// qtdone
EXPORT_C TInt CSecuritySettings::ChangeAutoLockPeriodL(TInt aPeriod)
    {
    TInt ret = 0;
    RDEBUG("aPeriod", aPeriod);
    RMobilePhone::TMobilePassword iOldPassword;
    TInt iFlags = 0;
    TInt iShowError = 1;
    TBuf<0x80> iCaption;
    iCaption.Copy(_L("ChangeAutoLockPeriodL"));
    iOldPassword.Copy(_L(""));
    ret = ChangeAutoLockPeriodParamsL(aPeriod, iOldPassword, iFlags,
            iCaption, iShowError);
    RDEBUG("ret", ret);
    return ret;
    }

//
// ----------------------------------------------------------
// CSecuritySettings::ChangeRemoteLockStatusL()
// Changes remote lock status (on/off)
// ----------------------------------------------------------
// no qtdone
EXPORT_C TInt CSecuritySettings::ChangeRemoteLockStatusL(
        TBool& aRemoteLockStatus, TDes& aRemoteLockCode, TInt aAutoLockPeriod)
    {
    TInt retValue( KErrNone );
		RDEBUG("aRemoteLockStatus", aRemoteLockStatus);
		RDEBUG("aAutoLockPeriod", aAutoLockPeriod);

    if ( aRemoteLockStatus )
        {
        aRemoteLockStatus = ETrue;

        // If user wishes to enable remote lock
        // a new remote lock code is required.
        // RemoteLockCodeQueryL also 
        retValue = RemoteLockCodeQueryL( aRemoteLockCode );
        }
    else
        {
        aRemoteLockStatus = EFalse;
        retValue = KErrNone;
				// TODO this should calculate aAutoLockPeriod itself, and not trust the input

        // Check whether AutoLock is enabled (timeout value greater 
        // than zero) or not. If AutoLock is enabled the domestic OS 
        // device lock should be left enabled.
        if ( aAutoLockPeriod == 0 )
            {
            // Disable lock setting from domestic OS
            retValue = RemoteLockSetLockSettingL( EFalse );
            }
        else
            {
            // If AutoLock is enabled, don't disable the DOS device lock
            // Re-set (enable) the domestic OS device lock because as a 
            // side effect it requires the security code from the user
            retValue = RemoteLockSetLockSettingL( ETrue );
            }
        }
		RDEBUG( "retValue", retValue );
    return retValue;
    }
//
// ----------------------------------------------------------
// CSecuritySettings::RemoteLockCodeQueryL()
// Pops up remote lock code query. Requires user to enter a new remote lock 
// code (RemoteMsg) twice and if they match enables the domestic OS device lock (which as 
// a side effect pops up security code query).
// Note: if the RemoteMsg is cancelled, nevertheless the lock is activated. This is done because the code is askedirst, and the only way to do so is by enabling the lock.
// This is not a problem, because:
// a) first the RemoteMsg is enable, and this function is used to change it
// b) if lock was disabled, the "change RemoteMsg" menu is not available.
// ----------------------------------------------------------
// qtdone
TInt CSecuritySettings::RemoteLockCodeQueryL(TDes& aRemoteLockCode)
    {

    TInt retValue( KErrNone );

      // This is done because lock-code needs to be asked first.
      // Enable lock setting in domestic OS. It is safe to enable the 
      // lock setting since RemoteLock API requires remote locking to
      // be enabled when changing or setting the remote lock message.
      retValue = RemoteLockSetLockSettingL( ETrue );
		RDEBUG( "retValue", retValue );
		if (retValue != KErrNone)
            return retValue;
    aRemoteLockCode.Zero();

    TInt queryAccepted = KErrCancel;
        queryAccepted = KErrCancel;
        CSecQueryUi * iSecQueryUi;
        iSecQueryUi = CSecQueryUi::NewL();
        // this queries both, and verifies itself
        queryAccepted = iSecQueryUi->SecQueryDialog(
                _L("RemoteMsg-New|RemoteMsg-Ver"), aRemoteLockCode,
                SEC_REMOTELOCK_CODE_MIN_LENGTH, SEC_REMOTELOCK_CODE_MAX_LENGTH,
                ESecUiAlphaSupported | ESecUiCancelSupported
                        | ESecUiEmergencyNotSupported | ESecUiNone);
        RDEBUG("aRemoteLockCode", 0);
        RDebug::Print(aRemoteLockCode);
        RDEBUG("queryAccepted", queryAccepted);
        delete iSecQueryUi;
        if (queryAccepted != KErrNone)
            return KErrAbort;

            // Check that the new remote lock code doesn't match the security code of the device.
            RMobilePhone::TMobilePhoneSecurityCode secCodeType = RMobilePhone::ESecurityCodePhonePassword;
            RMobilePhone::TMobilePassword securityCode;
            RMobilePhone::TMobilePassword unblockCode; // Required here only as a dummy parameter 


                securityCode = aRemoteLockCode;
								RDEBUG( "EMobilePhoneVerifySecurityCode", EMobilePhoneVerifySecurityCode );
                iWait->SetRequestType( EMobilePhoneVerifySecurityCode );
                RDEBUG( "VerifySecurityCode", 0 );
                iPhone.VerifySecurityCode( iWait->iStatus, secCodeType, securityCode, unblockCode );
                RDEBUG( "WaitForRequestL", 0 );
                TInt res = iWait->WaitForRequestL();
                RDEBUG( "WaitForRequestL res", res );
#ifdef __WINS__
                if (res == KErrNotSupported || res == KErrTimedOut)
                res = KErrGsm0707IncorrectPassword;	// KErrGsm0707IncorrectPassword = incorrect code
#endif
								RDEBUG( "KErrGsm0707IncorrectPassword", KErrGsm0707IncorrectPassword );
                if(res == KErrNone)
                    {
                    // The message is also valid as a lock-code, this means that
		                // remote lock code matches the security code 
		                // and that is not allowed
		                RDEBUG( "return KErrCancel because msg matches code", KErrCancel );
		                ShowResultNoteL(R_REMOTELOCK_INVALID_CODE, CAknNoteDialog::EErrorTone);
		                return KErrCancel;
                    }

		RDEBUG( "retValue", retValue );
    return retValue;
    }
//
// ----------------------------------------------------------
// CSecuritySettings::RemoteLockSetLockSettingL()
// Changes lock setting in domestic OS. Changing the domestic OS lock setting
// requires user to enter the security code.
// ----------------------------------------------------------
// qtdone
TInt CSecuritySettings::RemoteLockSetLockSettingL(TBool aLockSetting)
    {
    TInt retValue( KErrNone );
		RDEBUG( "aLockSetting", aLockSetting );


    RMobilePhone::TMobilePhoneLockSetting lockSetting = RMobilePhone::ELockSetEnabled;
    RMobilePhone::TMobilePhoneLock lockType = RMobilePhone::ELockPhoneDevice;


    RMobilePhone::TMobilePhoneLockInfoV1 lockInfo;
    RMobilePhone::TMobilePhoneLockInfoV1Pckg lockInfoPkg(lockInfo);

    //get lock info
    iWait->SetRequestType(EMobilePhoneGetLockInfo);
    RDEBUG("GetLockInfo", 0);
    iPhone.GetLockInfo(iWait->iStatus, lockType, lockInfoPkg);
    RDEBUG("WaitForRequestL", 0);
    TInt status = iWait->WaitForRequestL();
    RDEBUG("WaitForRequestL status", status);

#ifdef __WINS__
    if (status == KErrNotSupported || status == KErrTimedOut)
        {
        lockInfo.iSetting = RMobilePhone::ELockSetDisabled;
        status = KErrNone;
        }
#endif
    User::LeaveIfError(status);
		RDEBUG("current lockInfo.iSetting", lockInfo.iSetting);

		// disabled->disabled	should not happen
		// enabled->enabled		happens because a change of message also forces a code re-validation
    if ( aLockSetting )
        {
        lockSetting = RMobilePhone::ELockSetEnabled;
        }
    else
        {
        lockSetting = RMobilePhone::ELockSetDisabled;
        }
		RDEBUG("future lockSetting", lockSetting);

    iWait->SetRequestType( EMobilePhoneSetLockSetting );
    RProperty::Set(KPSUidSecurityUIs, KSecurityUIsQueryRequestCancel, ESecurityUIsQueryRequestOk);
    RDEBUG( "SetLockSetting", 0 );
    iPhone.SetLockSetting( iWait->iStatus, lockType, lockSetting );	// this will PassPhraseRequiredL
    RDEBUG( "WaitForRequestL", 0 );
    retValue = iWait->WaitForRequestL();
    RDEBUG( "WaitForRequestL retValue", retValue );
#ifdef __WINS__
    if (retValue == KErrNotSupported || retValue == KErrTimedOut)
    retValue = KErrNone;
#endif

    switch( retValue )
        {
        case KErrNone:
        break;

        case KErrGsmSSPasswordAttemptsViolation:
        case KErrLocked:
        case KErrGsm0707IncorrectPassword:
        case KErrAccessDenied:
        // Security code was entered erroneously
        //Error note is shown in CSecurityHandler::PassPhraseRequired()
        break;

        case KErrAbort:
        break;

        default:
        break;
        }

		RDEBUG( "retValue", retValue );
    return retValue;
    }

//
// ----------------------------------------------------------
// CSecuritySettings::ChangeSimSecurityL()
// Changes SIM security
// ----------------------------------------------------------
// qtdone
EXPORT_C TBool CSecuritySettings::ChangeSimSecurityL()
    {
    /*****************************************************
     *    Series 60 Customer / ETel
     *    Series 60  ETel API
     *****************************************************/
		RDEBUG("0", 0);
    RMobilePhone::TMobilePhoneLockInfoV1 lockInfo;
    RMobilePhone::TMobilePhoneLockInfoV1Pckg lockInfoPkg(lockInfo);
    RMobilePhone::TMobilePhoneLock lockType = RMobilePhone::ELockPhoneToICC;
    RMobilePhone::TMobilePhoneLockSetting lockChangeSetting;

    //get lock info
    iWait->SetRequestType(EMobilePhoneGetLockInfo);
    RDEBUG("GetLockInfo", 0);
    iPhone.GetLockInfo(iWait->iStatus, lockType, lockInfoPkg);
    RDEBUG("WaitForRequestL", 0);
    TInt status = iWait->WaitForRequestL();
    RDEBUG("WaitForRequestL status", status);

#ifdef __WINS__
    if (status == KErrNotSupported || status == KErrTimedOut)
        {
        lockInfo.iSetting = RMobilePhone::ELockSetDisabled;
        status = KErrNone;
        }
#endif
    User::LeaveIfError(status);
    TInt currentItem = 0;
		RDEBUG("lockInfo.iSetting", lockInfo.iSetting);
    if (lockInfo.iSetting == RMobilePhone::ELockSetDisabled)
        {
        currentItem = 1; // off
        }

    if (currentItem == 0) // switch the flag
        {
        lockChangeSetting = RMobilePhone::ELockSetDisabled;
        }
    else
        {
        lockChangeSetting = RMobilePhone::ELockSetEnabled;
        }
    RDEBUG("lockChangeSetting", lockChangeSetting);

    iWait->SetRequestType(EMobilePhoneSetLockSetting);
    RProperty::Set(KPSUidSecurityUIs, KSecurityUIsQueryRequestCancel,
            ESecurityUIsQueryRequestOk);
    RDEBUG("SetLockSetting", 0);
    iPhone.SetLockSetting(iWait->iStatus, lockType, lockChangeSetting); // this invokes the handler
    RDEBUG("WaitForRequestL", 0);
    status = iWait->WaitForRequestL();
    RDEBUG("WaitForRequestL status", status);
#ifdef __WINS__
    if (status == KErrNotSupported || status == KErrTimedOut)
    status = KErrNone;
#endif

    // the error was displayed in the handler
		RDEBUG("status", status);
    switch (status)
        {
        case KErrNone:
            {
            break;
            }
        case KErrGsm0707IncorrectPassword:
        case KErrAccessDenied:
            {
            // code was entered erroneously
            return ChangeSimSecurityL();
            }
        case KErrGsmSSPasswordAttemptsViolation:
        case KErrLocked:
            {
            return ChangeSimSecurityL();
            }
        case KErrAbort:
            {
            return EFalse;
            }
        default:
            {
            ShowErrorNoteL(status);
            return ChangeSimSecurityL();
            }
        }

    return ETrue;
    }
//
// ----------------------------------------------------------
// CSecuritySettings::ChangePinRequestL()
// Changes PIN1 request
// ----------------------------------------------------------
// qtdone
EXPORT_C TBool CSecuritySettings::ChangePinRequestL()
    {
    RDEBUG("0", 0);

    RMobilePhone::TMobilePassword iOldPassword;
    TInt iFlags = 0;
    iOldPassword.Copy(_L(""));

    TBuf<0x80> iCaption;
    iCaption.Copy(_L("ChangePinRequestL"));
    TInt iShowError = 1;
    TInt err = ChangePinRequestParamsL(
            1/* it's imposible to know if we want to set or clear*/,
            iOldPassword, iFlags, iCaption, iShowError);
    RDEBUG("err", err);
		if(err==KErrNone)
    	return ETrue;
		else
   		return EFalse;
    }

//
// ----------------------------------------------------------
// CSecuritySettings::ChangeUPinRequestL()
// Changes UPIN request on/off
// ----------------------------------------------------------
// qtdone
EXPORT_C TBool CSecuritySettings::ChangeUPinRequestL()
    {
    TBool wcdmaSupported(
            FeatureManager::FeatureSupported( KFeatureIdProtocolWcdma));
    TBool upinSupported(FeatureManager::FeatureSupported( KFeatureIdUpin));
		RDEBUG("wcdmaSupported", wcdmaSupported);
		RDEBUG("upinSupported", upinSupported);
    if (wcdmaSupported || upinSupported)
        {
        TInt simState;
        TInt err(KErrGeneral);
        err = RProperty::Get(KPSUidStartup, KPSSimStatus, simState);
        User::LeaveIfError(err);
        TBool simRemoved(simState == ESimNotPresent);

        if (simRemoved)
            {
            ShowResultNoteL(R_INSERT_SIM, CAknNoteDialog::EErrorTone);
            return EFalse;
            }

        RMobilePhone::TMobilePhoneLockInfoV1 lockInfo;
        RMobilePhone::TMobilePhoneLockInfoV1Pckg lockInfoPkg(lockInfo);
        RMobilePhone::TMobilePhoneLock lockType =
                RMobilePhone::ELockUniversalPin;

        RMobilePhone::TMobilePhoneLockSetting lockChangeSetting =
                RMobilePhone::ELockSetDisabled;

        //get lock info
        iWait->SetRequestType(EMobilePhoneGetLockInfo);
        RDEBUG("GetLockInfo", 0);
        iPhone.GetLockInfo(iWait->iStatus, lockType, lockInfoPkg);
        RDEBUG("WaitForRequestL", 0);
        TInt status = iWait->WaitForRequestL();
        RDEBUG("WaitForRequestL status", status);
#ifdef __WINS__
        if (status == KErrNotSupported || status == KErrTimedOut)
        status = KErrNone;
#endif
        User::LeaveIfError(status);
        TInt currentItem = 0;

				RDEBUG("lockInfo.iSetting", lockInfo.iSetting);
        if (lockInfo.iSetting == RMobilePhone::ELockSetDisabled)
            {
            currentItem = 1; // off
            }

        if (currentItem == 0) // switch the flag
            {
            lockChangeSetting = RMobilePhone::ELockSetDisabled;
            }
        else
            {
            lockChangeSetting = RMobilePhone::ELockSetEnabled;
            }
				RDEBUG("lockChangeSetting", lockChangeSetting);
        // Raise a flag to indicate that the UPIN
        // request coming from ETEL has originated from SecUi and not from Engine.
        RProperty::Set(KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery,
                ESecurityUIsSecUIOriginated);
        // Change the lock setting
        iWait->SetRequestType(EMobilePhoneSetLockSetting);
        RProperty::Set(KPSUidSecurityUIs, KSecurityUIsQueryRequestCancel,
                ESecurityUIsQueryRequestOk);
        RDEBUG("SetLockSetting", 0);
        iPhone.SetLockSetting(iWait->iStatus, lockType, lockChangeSetting); // this calls something in the handler
        RDEBUG("WaitForRequestL", 0);
        status = iWait->WaitForRequestL();
        RDEBUG("WaitForRequestL status", status);
        // Lower the flag                           
        RProperty::Set(KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery,
                ESecurityUIsSecUIOriginatedUninitialized);
#ifdef __WINS__
        if (status == KErrNotSupported || status == KErrTimedOut)
        status = KErrNone;
#endif

        // no need to show errors because they were displayed in the Handler
        switch (status)
            {
            case KErrNone:
                {
                break;
                }
            case KErrGsm0707OperationNotAllowed:
                {
                // not allowed with this sim
                ShowResultNoteL(R_OPERATION_NOT_ALLOWED,
                        CAknNoteDialog::EErrorTone);
                return EFalse;
                }
            case KErrGsm0707IncorrectPassword:
            case KErrAccessDenied:
                {
                // code was entered erroneously
                return ChangeUPinRequestL();
                }
            case KErrGsmSSPasswordAttemptsViolation:
            case KErrLocked:
                {
                return EFalse;
                }
            case KErrAbort:
                {
                return EFalse;
                }
            default:
                {
                ShowErrorNoteL(status);
                return ChangeUPinRequestL();
                }
            }

        return ETrue;
        }
    else
        return EFalse;

    }

//
// ----------------------------------------------------------
// CSecuritySettings::SwitchPinCodesL()
// Changes the pin code currently in use (PIN/UPIN)
// ----------------------------------------------------------
// qtdone
EXPORT_C TBool CSecuritySettings::SwitchPinCodesL()
    {
    TBool wcdmaSupported(
            FeatureManager::FeatureSupported( KFeatureIdProtocolWcdma));
    TBool upinSupported(FeatureManager::FeatureSupported( KFeatureIdUpin));
				RDEBUG("wcdmaSupported", wcdmaSupported);
				RDEBUG("upinSupported", upinSupported);
    if (wcdmaSupported || upinSupported)
        {
        // If we are in simless offline mode the PIN codes can't obviously be switched
        TInt simState;
        TInt err(KErrGeneral);
        err = RProperty::Get(KPSUidStartup, KPSSimStatus, simState);
        User::LeaveIfError(err);
        TBool simRemoved(simState == ESimNotPresent);

        if (simRemoved)
            {
            ShowResultNoteL(R_INSERT_SIM, CAknNoteDialog::EErrorTone);
            return EFalse;
            }

        RMobilePhone::TMobilePhoneLock lockType =
                RMobilePhone::ELockUniversalPin;
        RMobilePhone::TMobilePhoneLockSetting lockChangeSetting =
                RMobilePhone::ELockReplaced;
        RMobilePhone::TMobilePhoneSecurityCode activeCode;

        iCustomPhone.GetActivePin(activeCode);

        RMobilePhone::TMobilePhoneLockInfoV1 lockInfo;
        RMobilePhone::TMobilePhoneLockInfoV1Pckg lockInfoPkg(lockInfo);
				RDEBUG("EMobilePhoneGetLockInfo", EMobilePhoneGetLockInfo);
        iWait->SetRequestType(EMobilePhoneGetLockInfo);

        if (activeCode == RMobilePhone::ESecurityUniversalPin)
            {
            lockType = RMobilePhone::ELockUniversalPin;
            iPhone.GetLockInfo(iWait->iStatus, lockType, lockInfoPkg);
            RDEBUG("WaitForRequestL", 0);
            TInt res = iWait->WaitForRequestL();
            RDEBUG("WaitForRequestL res", res);
#ifdef __WINS__
            if (res == KErrNotSupported || res == KErrTimedOut)
            res = KErrNone;
#endif
            User::LeaveIfError(res);
            }
        else
            {
            lockType = RMobilePhone::ELockICC;
            iPhone.GetLockInfo(iWait->iStatus, lockType, lockInfoPkg);
            RDEBUG("WaitForRequestL", 0);
            TInt res = iWait->WaitForRequestL();
            RDEBUG("WaitForRequestL res", res);
#ifdef __WINS__
            if (res == KErrNotSupported || res == KErrTimedOut)
            res = KErrNone;
#endif
            User::LeaveIfError(res);
            }

        // code request must be ON to change active code.
				RDEBUG("lockInfo.iSetting", lockInfo.iSetting);
        if (lockInfo.iSetting == RMobilePhone::ELockSetDisabled)
            {

            if (activeCode != RMobilePhone::ESecurityUniversalPin)
                {
                ShowResultNoteL(R_UPIN_NOT_ALLOWED,
                        CAknNoteDialog::EErrorTone);
                }
            else
                {
                ShowResultNoteL(R_PIN_NOT_ALLOWED, CAknNoteDialog::EErrorTone);
                }
            return EFalse;
            }

        iCustomPhone.GetActivePin(activeCode);
        TInt currentItem = 0;

				RDEBUG("activeCode", activeCode);
        if (activeCode == RMobilePhone::ESecurityUniversalPin)
            {
            currentItem = 1; // UPIN
            }
        if (currentItem == 0) // switch the flag
            {
            lockType = RMobilePhone::ELockUniversalPin;
            }
        else
            {
            lockType = RMobilePhone::ELockICC;
            }
				RDEBUG("lockType", lockType);

        // Raise a flag to indicate that the code
        // request coming from ETEL has originated from SecUi and not from Engine.
        RProperty::Set(KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery,
                ESecurityUIsSecUIOriginated);
        // Change the lock setting
        iWait->SetRequestType(EMobilePhoneSetLockSetting);
        RProperty::Set(KPSUidSecurityUIs, KSecurityUIsQueryRequestCancel,
                ESecurityUIsQueryRequestOk);
        RDEBUG("SetLockSetting", 0);
        iPhone.SetLockSetting(iWait->iStatus, lockType, lockChangeSetting); // request from handler
        RDEBUG("WaitForRequestL", 0);
        TInt status = iWait->WaitForRequestL();
        RDEBUG("WaitForRequestL status", status);
        // Lower the flag                            
        RProperty::Set(KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery,
                ESecurityUIsSecUIOriginatedUninitialized);
#ifdef __WINS__
        if (status == KErrNotSupported || status == KErrTimedOut)
        status = KErrNone;
#endif

        // errors are shown in the handler
        switch (status)
            {
            case KErrNone:
                {
                break;
                }
            case KErrGsm0707OperationNotAllowed:
                {
                // not allowed with this sim
                ShowResultNoteL(R_OPERATION_NOT_ALLOWED,
                        CAknNoteDialog::EErrorTone);
                return EFalse;
                }
            case KErrGsm0707IncorrectPassword:
            case KErrAccessDenied:
                {
                // code was entered erroneously
                return SwitchPinCodesL();
                }
            case KErrGsmSSPasswordAttemptsViolation:
            case KErrLocked:
                {
                return EFalse;
                }
            case KErrAbort:
                {
                return EFalse;
                }
            default:
                {
                ShowErrorNoteL(status);
                return SwitchPinCodesL();
                }
            }

        return ETrue;
        }
    else
        return EFalse;
    }

//
// ----------------------------------------------------------
// CSecuritySettings::IsLockEnabledL()
// Return is lock enabled/disabled
// ----------------------------------------------------------
// qtdone
EXPORT_C TBool CSecuritySettings::IsLockEnabledL(
        RMobilePhone::TMobilePhoneLock aLockType)
    {
    TBool ret = EFalse;
    RMobilePhone::TMobilePhoneLockInfoV1 lockInfo;
    //get lock info
    RMobilePhone::TMobilePhoneLockInfoV1Pckg lockInfoPkg(lockInfo);
    iWait->SetRequestType(EMobilePhoneGetLockInfo);
    RDEBUG("GetLockInfo", 0);
    iPhone.GetLockInfo(iWait->iStatus, aLockType, lockInfoPkg);
    RDEBUG("WaitForRequestL", 0);
    TInt res = iWait->WaitForRequestL();
    RDEBUG("WaitForRequestL res", res);

    if (res != KErrNone)
        ret = ETrue;
    //lock is enabled return true
    else if (lockInfo.iSetting == RMobilePhone::ELockSetEnabled)
        {
        ret = ETrue;
        }
    RDEBUG("ret", ret);
    return ret;
    }
//
// ----------------------------------------------------------
// CSecuritySettings::AskSecCodeL()
// For asking security code e.g in settings
// ----------------------------------------------------------
// qtdone
EXPORT_C TBool CSecuritySettings::AskSecCodeL()
    {
    return iSecurityHandler->AskSecCodeL();
    }
//
// ----------------------------------------------------------
// CSecuritySettings::AskPin2L()
// Asks PIN2
// ----------------------------------------------------------
// qtdone
EXPORT_C TBool CSecuritySettings::AskPin2L()
    {
    /*****************************************************
     *    Series 60 Customer / ETel
     *    Series 60  ETel API
     *****************************************************/
    RDEBUG("0", 0);
    TInt retPhone = 0;
    // check if pin2 is blocked...
    RMmCustomAPI::TSecurityCodeType secCodeType =
            RMmCustomAPI::ESecurityCodePin2;
    RMobilePhone::TMobilePhoneSecurityCode etelsecCodeType(
            RMobilePhone::ESecurityCodePin2);
    RMobilePhone::TMobilePhoneSecurityCodeInfoV5 codeInfo;
    RMobilePhone::TMobilePhoneSecurityCodeInfoV5Pckg codeInfoPkg(codeInfo);
    RMobilePhone::TMobilePassword password;
    TBool isBlocked = EFalse;
    TInt queryAccepted = KErrCancel;
    //Check whether PIN2 is blocked
    retPhone = iCustomPhone.IsBlocked(secCodeType, isBlocked);

    RDEBUG("retPhone", retPhone);
    RDEBUG("isBlocked", isBlocked);
#ifdef __WINS__
    if (retPhone == KErrNotSupported || retPhone == KErrTimedOut)
    retPhone = KErrNone;
#endif
    if (isBlocked)
        return EFalse;

    if (retPhone != KErrNone)
        {
        switch (retPhone)
            {
            // PIN2 Blocked.
            case KErrGsm0707SIMPuk2Required:
                break;
            case KErrGsmSSPasswordAttemptsViolation:
            case KErrLocked:
                // Pin2 features blocked permanently!
                ShowResultNoteL(R_PIN2_REJECTED,
                        CAknNoteDialog::EConfirmationTone);
                break;
            case KErrGsm0707SimNotInserted:
                // not allowed with this sim
                ShowResultNoteL(R_OPERATION_NOT_ALLOWED,
                        CAknNoteDialog::EErrorTone);
                break;
            default:
                ShowErrorNoteL(retPhone);
                break;
            }
        return EFalse;
        }
    iWait->SetRequestType(EMobilePhoneGetSecurityCodeInfo);
    RDEBUG("GetSecurityCodeInfo", 0);
    iPhone.GetSecurityCodeInfo(iWait->iStatus, etelsecCodeType, codeInfoPkg);
    RDEBUG("WaitForRequestL", 0);
    retPhone = iWait->WaitForRequestL();
    RDEBUG("WaitForRequestL retPhone", retPhone);
#ifdef __WINS__
    if (retPhone == KErrNotSupported || retPhone == KErrTimedOut)
        {
        retPhone = KErrNone;
        codeInfo.iRemainingEntryAttempts = 3;
        }
#endif
    User::LeaveIfError(retPhone);

    RDEBUG("codeInfo.iRemainingEntryAttempts",
            codeInfo.iRemainingEntryAttempts);
    if (codeInfo.iRemainingEntryAttempts == KMaxNumberOfPINAttempts)
        codeInfo.iRemainingEntryAttempts = -1;

    // ask pin2 code  
    /* request PIN using QT */
    queryAccepted = KErrCancel;
    CSecQueryUi *iSecQueryUi;
    iSecQueryUi = CSecQueryUi::NewL();
    TBuf<0x100> title;
    title.Zero();
    title.Append(_L("PIN2"));
    title.Append(_L("#"));
    title.AppendNum(codeInfo.iRemainingEntryAttempts);
    queryAccepted = iSecQueryUi->SecQueryDialog(title, password,
            SEC_C_PIN_CODE_MIN_LENGTH, SEC_C_PIN_CODE_MAX_LENGTH,
            ESecUiAlphaNotSupported | ESecUiCancelSupported
                    | ESecUiEmergencyNotSupported | secCodeType);
    RDEBUG("password", 0);
    RDebug::Print(password);
    RDEBUG("queryAccepted", queryAccepted);
    delete iSecQueryUi;
    if (queryAccepted != KErrNone)
        return EFalse;

    // verify code
    RMobilePhone::TMobilePassword required_fourth;
    iWait->SetRequestType(EMobilePhoneVerifySecurityCode);
    RDEBUG("VerifySecurityCode", 0);
    iPhone.VerifySecurityCode(iWait->iStatus, etelsecCodeType, password,
            required_fourth);
    RDEBUG("WaitForRequestL", 0);
    retPhone = iWait->WaitForRequestL();
    RDEBUG("WaitForRequestL retPhone", retPhone);
#ifdef __WINS__
    if (retPhone == KErrNotSupported)
    retPhone = KErrNone;
#endif

    switch (retPhone)
        {
        case KErrNone:
            break;
        case KErrGsm0707IncorrectPassword:
        case KErrAccessDenied:
            // code was entered erroneously
            ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
            return AskPin2L();
        case KErrGsm0707OperationNotAllowed:
            // not allowed with this sim
            ShowResultNoteL(R_OPERATION_NOT_ALLOWED,
                    CAknNoteDialog::EErrorTone);
            return EFalse;
        case KErrGsmSSPasswordAttemptsViolation:
        case KErrLocked:
            // code was blocked
            ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
            return EFalse;
        default:
            ShowErrorNoteL(retPhone);
            return AskPin2L();
        }

    return ETrue;
    }
//
// ----------------------------------------------------------
// CSecuritySettings::SetFdnModeL()
// Activates or deactivates Fixed Dialling Numbers (FDN) mode.  
// ----------------------------------------------------------
// not qtdone
EXPORT_C void CSecuritySettings::SetFdnModeL()
    {
    /*****************************************************
     *    Series 60 Customer / ETel
     *    Series 60  ETel API
     *****************************************************/
		RDEBUG("0", 0);
    RMmCustomAPI::TSecurityCodeType secCodeType =
            RMmCustomAPI::ESecurityCodePin2;

    TBool isBlocked = EFalse;
    TInt ret = iCustomPhone.IsBlocked(secCodeType, isBlocked);
    RDEBUG("isBlocked", isBlocked);
    RDEBUG("ret", ret);
    if (isBlocked)
        return;

    if (ret != KErrNone)
        {
        switch (ret)
            {
            // PIN2 Blocked.
            case KErrGsm0707SIMPuk2Required:
                break;
            case KErrGsmSSPasswordAttemptsViolation:
            case KErrLocked:
                // Pin2 features blocked permanently!
                ShowResultNoteL(R_PIN2_REJECTED,
                        CAknNoteDialog::EConfirmationTone);
                break;
            case KErrGsm0707SimNotInserted:
                // not allowed with this sim
                ShowResultNoteL(R_OPERATION_NOT_ALLOWED,
                        CAknNoteDialog::EErrorTone);
                break;
            default:
                ShowErrorNoteL(ret);
                break;
            }
        return;
        }

    TInt status = KErrNone;

    RMobilePhone::TMobilePhoneFdnStatus fdnMode;
    RMobilePhone::TMobilePhoneFdnSetting fdnSet;

    iPhone.GetFdnStatus(fdnMode);

    if (fdnMode == RMobilePhone::EFdnActive)
        {
        fdnSet = RMobilePhone::EFdnSetOff;
        }
    else
        {
        fdnSet = RMobilePhone::EFdnSetOn;
        }
    RDEBUG("fdnSet", fdnSet);
    // Change the FDN setting
    iWait->SetRequestType(EMobilePhoneSetFdnSetting);
    RProperty::Set(KPSUidSecurityUIs, KSecurityUIsQueryRequestCancel,
            ESecurityUIsQueryRequestOk);
    RDEBUG("SetFdnSetting", 0);
    iPhone.SetFdnSetting(iWait->iStatus, fdnSet);
    RDEBUG("WaitForRequestL", 0);
    status = iWait->WaitForRequestL();
    RDEBUG("WaitForRequestL status", status);
#ifdef __WINS__
    if (status == KErrNotSupported)
    status = KErrNone;
#endif

		RDEBUG("status", status);
    switch (status)
        {
        case KErrNone:
            break;
        case KErrGsm0707IncorrectPassword:
        case KErrAccessDenied:
            // code was entered erroneously
            SetFdnModeL();
            break;
        case KErrGsmSSPasswordAttemptsViolation:
        case KErrLocked:
            break;
        case KErrAbort:
            break;
        case KErrGsm0707OperationNotAllowed:
            // not allowed with this sim
            ShowResultNoteL(R_OPERATION_NOT_ALLOWED,
                    CAknNoteDialog::EErrorTone);
            break;
        default:
            ShowErrorNoteL(status);
            break;
        }
    }
//
// ----------------------------------------------------------
// CSecuritySettings::GetFndMode()
// Retrieves the current Fixed Dialling Numbers mode
// ----------------------------------------------------------
// qtdone
EXPORT_C TInt CSecuritySettings::GetFdnMode(
        RMobilePhone::TMobilePhoneFdnStatus& aFdnMode)
    {
    /*****************************************************
     *    Series 60 Customer / ETel
     *    Series 60  ETel API
     *****************************************************/
		RDEBUG("0", 0);
    return iPhone.GetFdnStatus(aFdnMode);
    }

//
// ----------------------------------------------------------
// CSecuritySettings::ShowErrorNoteL()
// Shows error note
// ----------------------------------------------------------
// qtdone
void CSecuritySettings::ShowErrorNoteL(TInt aError)
    {
    RDEBUG("aError", aError);

    ShowResultNoteL(aError, CAknNoteDialog::EErrorTone);
    }

//
// ----------------------------------------------------------
// CSecuritySettings::ShowResultNoteL()
// Shows result note
// ----------------------------------------------------------
// qtdone
void CSecuritySettings::ShowResultNoteL(TInt aResourceID,
        CAknNoteDialog::TTone aTone)
    {
    RDEBUG("aResourceID", aResourceID);

    CHbDeviceMessageBoxSymbian* messageBox =
            CHbDeviceMessageBoxSymbian::NewL(
                    CHbDeviceMessageBoxSymbian::EWarning);
    CleanupStack::PushL(messageBox);
    _LIT(KText, "ShowResultNoteL: ");
    TBuf<0x200> title;
    TBuf<0x200> titleTr;
    title.Zero();
    titleTr.Zero();
    title.Append(KText);
    title.AppendNum(aResourceID);
    _LIT(KSeparator, " ");
    title.Append(KSeparator);
    switch (aResourceID)
        {
        case 0:
            titleTr.Append(_L("OK"));
            title.Append(_L("OK"));
            break;
        case KErrGsm0707IncorrectPassword:
            titleTr.Append(_L("KErrGsm0707IncorrectPassword"));
            title.Append(_L("Incorrect Password"));
            break;
        case KErrAccessDenied:
            titleTr.Append(_L("KErrAccessDenied"));
            title.Append(_L("Access Denied"));
            break;
        case KErrGsmSSPasswordAttemptsViolation:
            titleTr.Append(_L("KErrGsmSSPasswordAttemptsViolation"));
            title.Append(_L("Password Attempts Violation"));
            break;
        case KErrLocked:
            titleTr.Append(_L("KErrLocked"));
            title.Append(_L("Locked"));
            break;
        case KErrGsm0707OperationNotAllowed:
            titleTr.Append(_L("KErrGsm0707OperationNotAllowed"));
            title.Append(_L("Operation Not Allowed"));
            break;
        case KErrAbort:
            titleTr.Append(_L("KErrAbort"));
            title.Append(_L("Abort"));
            break;
        case KErrNotSupported:
            titleTr.Append(_L("KErrNotSupported"));
            title.Append(_L("Not Supported"));
            break;
        case R_SEC_BLOCKED:
            titleTr.Append(_L("R_SEC_BLOCKED"));
            title.Append(_L("BLOCKED"));
            break;
        case R_CODE_ERROR:
            titleTr.Append(_L("R_CODE_ERROR"));
            title.Append(_L("ERROR"));
            break;
        case KErrGsmInvalidParameter:
            titleTr.Append(_L("KErrGsmInvalidParameter"));
            title.Append(_L("Invalid Parameter"));
            break;
        case R_CONFIRMATION_NOTE:
            titleTr.Append(_L("R_CONFIRMATION_NOTE"));
            title.Append(_L("CONFIRMED"));
            break;
        case R_CODES_DONT_MATCH:
            titleTr.Append(_L("R_CODES_DONT_MATCH"));
            title.Append(_L("CODES DONT MATCH"));
            break;
        case R_PIN_CODE_CHANGED_NOTE:
            titleTr.Append(_L("R_PIN_CODE_CHANGED_NOTE"));
            title.Append(_L("PIN CODE CHANGED"));
            break;
        case R_SECURITY_CODE_CHANGED_NOTE:
            titleTr.Append(_L("R_SECURITY_CODE_CHANGED_NOTE"));
            title.Append(_L("SECURITY CODE CHANGED"));
            break;
        case R_SECUI_TEXT_AUTOLOCK_MUST_BE_ACTIVE:
            titleTr.Append(_L("R_SECUI_TEXT_AUTOLOCK_MUST_BE_ACTIVE"));
            title.Append(_L("AUTOLOCK MUST BE ACTIVE"));
            break;
        case KErrServerTerminated:
            titleTr.Append(_L("KErrServerTerminated"));
            title.Append(_L("Server Terminated"));
            break;
        case KErrServerBusy:
            titleTr.Append(_L("KErrServerBusy"));
            title.Append(_L("Server Busy"));
            break;
        case R_PIN2_REJECTED:
            titleTr.Append(_L("R_PIN2_REJECTED"));
            title.Append(_L("PIN2 REJECTED"));
            break;
        case R_OPERATION_NOT_ALLOWED:
            titleTr.Append(_L("R_OPERATION_NOT_ALLOWED"));
            title.Append(_L("OPERATION NOT ALLOWED"));
            break;
        case R_UPIN_NOT_ALLOWED:
            titleTr.Append(_L("R_UPIN_NOT_ALLOWED"));
            title.Append(_L("UPIN NOT ALLOWED"));
            break;
        case R_PIN_NOT_ALLOWED:
            titleTr.Append(_L("R_PIN_NOT_ALLOWED"));
            title.Append(_L("PIN NOT ALLOWED"));
            break;
        case R_INSERT_SIM:
            titleTr.Append(_L("R_INSERT_SIM"));
            title.Append(_L("INSERT SIM"));
            break;
        case R_SIM_ON:
            titleTr.Append(_L("R_SIM_ON"));
            title.Append(_L("SIM ON"));
            break;
        case KErrTimedOut:
            titleTr.Append(_L("KErrTimedOut"));
            title.Append(_L("Timed Out"));
            break;
        case R_PIN2_CODE_CHANGED_NOTE:
            titleTr.Append(_L("R_PIN2_CODE_CHANGED_NOTE"));
            title.Append(_L("PIN2 CODE CHANGED"));
            break;
        case KErrArgument:
            titleTr.Append(_L("KErrArgument"));
            title.Append(_L("Error Argument"));
            break;
        case R_SIM_OFF:
            titleTr.Append(_L("R_SIM_OFF"));
            title.Append(_L("SIM OFF"));
            break;
        case R_SIM_ALLREADY_OFF:
            titleTr.Append(_L("R_SIM_ALLREADY_OFF"));
            title.Append(_L("SIM ALLREADY OFF"));
            break;
        case R_SIM_NOT_ALLOWED:
            titleTr.Append(_L("R_SIM_NOT_ALLOWED"));
            title.Append(_L("SIM NOT ALLOWED"));
            break;
        case R_REMOTELOCK_INVALID_CODE:
            titleTr.Append(_L("R_REMOTELOCK_INVALID_CODE"));
            title.Append(_L("REMOTELOCK INVALID CODE"));
            break;

        default: // " "
            titleTr.Append(_L("Specific Error"));
            title.Append(_L("Specific Error"));
            break;
        }
    messageBox->SetTextL(title);
    RDEBUG("aResourceID", aResourceID);
    RDebug::Print(titleTr);

    _LIT(KIconNameWondering, "qtg_small_smiley_wondering");
    _LIT(KIconNameSmile, "qtg_small_smiley_smile");
    if(aResourceID==0 || aResourceID==R_CONFIRMATION_NOTE)
    	messageBox->SetIconNameL(KIconNameSmile);
    else
    	messageBox->SetIconNameL(KIconNameWondering);

    if (aTone == CAknNoteDialog::EErrorTone) // another case is EConfirmationTone
        {
        messageBox->SetTimeout(messageBox->Timeout() * 2); // errors are displayed double time
        }

    // use default timeout
    messageBox->ShowL();
    CleanupStack::PopAndDestroy(); // messageBox

    }

//
// ----------------------------------------------------------
// CSecuritySettings::IsUpinSupportedL()
// Return is UPIN supported
// ----------------------------------------------------------
// qtdone
EXPORT_C TBool CSecuritySettings::IsUpinSupportedL()
    {
    TBool wcdmaSupported(
            FeatureManager::FeatureSupported( KFeatureIdProtocolWcdma));
    TBool upinSupported(FeatureManager::FeatureSupported( KFeatureIdUpin));
    TBool isSupported = EFalse;
		RDEBUG("wcdmaSupported", wcdmaSupported);
		RDEBUG("upinSupported", upinSupported);
    if (wcdmaSupported || upinSupported)
        {
        RMobilePhone::TMobilePhoneLockInfoV1 lockInfo;

        //get lock info
        RMobilePhone::TMobilePhoneLockInfoV1Pckg lockInfoPkg(lockInfo);
        iWait->SetRequestType(EMobilePhoneGetLockInfo);
        RDEBUG("GetLockInfo", 0);
        iPhone.GetLockInfo(iWait->iStatus, RMobilePhone::ELockUniversalPin,
                lockInfoPkg);
        RDEBUG("WaitForRequestL", 0);
        TInt res = iWait->WaitForRequestL();
        RDEBUG("WaitForRequestL res", res);
        if ((res == KErrNotSupported) || (res == KErrGsmInvalidParameter))
            {
            RDEBUG("0", 0);
            isSupported = EFalse;
            }
        else
            {
            RDEBUG("1", 1);
            isSupported = ETrue;
            }
        }
    else
        isSupported = EFalse;
    RDEBUG("isSupported", isSupported);
    return isSupported;
    }
//
// ----------------------------------------------------------
// CSecuritySettings::IsUpinBlocked()
// Return is a code blocked
// ----------------------------------------------------------
// qtdone
EXPORT_C TBool CSecuritySettings::IsUpinBlocked()
    {
    TBool wcdmaSupported(
            FeatureManager::FeatureSupported( KFeatureIdProtocolWcdma));
    TBool upinSupported(FeatureManager::FeatureSupported( KFeatureIdUpin));
    if (wcdmaSupported || upinSupported)
        {
        RMmCustomAPI::TSecurityCodeType secCodeType;
        secCodeType = RMmCustomAPI::ESecurityUniversalPin;
        TBool isBlocked = EFalse;
        RDEBUG("IsBlocked", 0);
        TInt ret = iCustomPhone.IsBlocked(secCodeType, isBlocked);
        RDEBUG("ret", ret);
        RDEBUG("isBlocked", isBlocked);
        return isBlocked;
        }
    else
        return EFalse;
    }
//
// ----------------------------------------------------------
// CSecuritySettings::IsUpinActive()
// Return the code active in current application (PIN/UPIN)
// ----------------------------------------------------------
// qtdone
EXPORT_C TBool CSecuritySettings::IsUpinActive()
    {
    TBool wcdmaSupported(
            FeatureManager::FeatureSupported( KFeatureIdProtocolWcdma));
    TBool upinSupported(FeatureManager::FeatureSupported( KFeatureIdUpin));
    if (wcdmaSupported || upinSupported)
        {
        RMobilePhone::TMobilePhoneSecurityCode activePin;
        RDEBUG("GetActivePin", 0);
        iCustomPhone.GetActivePin(activePin);
        RDEBUG("activePin", activePin);
        RDEBUG("RMobilePhone::ESecurityUniversalPin",
                RMobilePhone::ESecurityUniversalPin);
        if (activePin == RMobilePhone::ESecurityUniversalPin)
            {
            return ETrue;
            }
        return EFalse;
        }
    else
        return EFalse;
    }

/**************************/
// qtdone
EXPORT_C TInt CSecuritySettings::ChangePinParamsL(
        RMobilePhone::TMobilePassword aOldPassword,
        RMobilePhone::TMobilePassword aNewPassword, TInt aFlags,
        TDes& aCaption, TInt aShowError)
    {
    RDEBUG("aFlags", aFlags);
    RDEBUG("aOldPassword", 0);
    RDebug::Print(aOldPassword);
    RDEBUG("aNewPassword", 0);
    RDebug::Print(aNewPassword);
    RDEBUG("aCaption", 0);
    RDebug::Print(aCaption);
    RDEBUG("aShowError", aShowError);

    /*****************************************************
     *    Series 60 Customer / ETel
     *    Series 60  ETel API
     *****************************************************/

    TInt simState;
    TInt err(KErrGeneral);
    err = RProperty::Get(KPSUidStartup, KPSSimStatus, simState);
    User::LeaveIfError(err);
    TBool simRemoved(simState == ESimNotPresent);

		RDEBUG("simRemoved", simRemoved);
    if (simRemoved)
        {
        ShowResultNoteL(R_INSERT_SIM, CAknNoteDialog::EErrorTone);
        return KErrAccessDenied;
        }
    RMobilePhone::TMobilePhoneSecurityCode secCodeType;
    secCodeType = RMobilePhone::ESecurityCodePin1;

    RMobilePhone::TMobilePassword oldPassword;
    RMobilePhone::TMobilePassword newPassword;
    RMobilePhone::TMobilePhonePasswordChangeV1 passwords;
    RMobilePhone::TMobilePhoneSecurityCodeInfoV5 codeInfo;
    RMobilePhone::TMobilePhoneSecurityCodeInfoV5Pckg codeInfoPkg(codeInfo);
    RMobilePhone::TMobilePassword required_fourth;
    TInt queryAccepted = KErrCancel;

    RDEBUG("0", 0);

    RMobilePhone::TMobilePhoneLock lockType;
    RMobilePhone::TMobilePhoneLockInfoV1 lockInfo;

    lockType = RMobilePhone::ELockICC;

    RMobilePhone::TMobilePhoneLockInfoV1Pckg lockInfoPkg(lockInfo);
    RDEBUG("0", 0);
    iWait->SetRequestType(EMobilePhoneGetLockInfo);
    TInt res = KErrNone;
    RDEBUG("GetLockInfo", 0);
    iPhone.GetLockInfo(iWait->iStatus, lockType, lockInfoPkg);
    RDEBUG("WaitForRequestL", 0);
    res = iWait->WaitForRequestL();
    RDEBUG("WaitForRequestL res", res);
#ifdef __WINS__
    if (res == KErrTimedOut)
        {
        lockInfo.iSetting = RMobilePhone::ELockSetEnabled;
        res = KErrNone;
        }
#endif

    User::LeaveIfError(res);

    if (lockInfo.iSetting == RMobilePhone::ELockSetDisabled)
        {
        RDEBUG("RMobilePhone::ELockSetDisabled",
                RMobilePhone::ELockSetDisabled);
        ShowResultNoteL(R_PIN_NOT_ALLOWED, CAknNoteDialog::EErrorTone);
        return KErrAccessDenied;
        }

    RDEBUG("0", 0);
    iWait->SetRequestType(EMobilePhoneGetSecurityCodeInfo);
    RDEBUG("GetSecurityCodeInfo", 0);
    iPhone.GetSecurityCodeInfo(iWait->iStatus, secCodeType, codeInfoPkg);
    RDEBUG("WaitForRequestL", 0);
    res = iWait->WaitForRequestL();
    RDEBUG("WaitForRequestL res", res);
#ifdef __WINS__
    if (res == KErrNotSupported || res == KErrTimedOut)
        {
        res = KErrNone;
        codeInfo.iRemainingEntryAttempts=KMaxNumberOfPINAttempts;
        }
#endif
    User::LeaveIfError(res);

    RDEBUG("codeInfo.iRemainingEntryAttempts",
            codeInfo.iRemainingEntryAttempts);
    if (codeInfo.iRemainingEntryAttempts == KMaxNumberOfPINAttempts)
        codeInfo.iRemainingEntryAttempts = -1;

    RDEBUG("checking aOldPassword", 0);
    if (aOldPassword.Length() == 0)
        {
        RDEBUG("asking aOldPassword", 0);
        /* request PIN using QT */
        queryAccepted = KErrCancel;
        CSecQueryUi *iSecQueryUi;
        iSecQueryUi = CSecQueryUi::NewL();
        TBuf<0x100> title;
        title.Zero();
        title.Append(_L("PIN1-Old"));
        title.Append(_L("#"));
        title.AppendNum(codeInfo.iRemainingEntryAttempts);
        queryAccepted = iSecQueryUi->SecQueryDialog(title, oldPassword,
                SEC_C_PIN_CODE_MIN_LENGTH, SEC_C_PIN_CODE_MAX_LENGTH,
                ESecUiAlphaNotSupported | ESecUiCancelSupported
                        | ESecUiEmergencyNotSupported | secCodeType);
        RDEBUG("oldPassword", 0);
        RDebug::Print(oldPassword);
        RDEBUG("queryAccepted", queryAccepted);
        delete iSecQueryUi;
        if (queryAccepted != KErrNone)
            return KErrAbort;
        /* end request PIN using QT */

        // verify it now, so that the user doesn't need to see the error _after_ typing the new ones
        RDEBUG("VerifySecurityCode", 0);
        iPhone.VerifySecurityCode(iWait->iStatus, secCodeType, oldPassword,
                required_fourth);
        RDEBUG("WaitForRequestL", 0);
        res = iWait->WaitForRequestL();
        RDEBUG("WaitForRequestL res", res);
#ifdef __WINS__
        if (res == KErrNotSupported)
        res = KErrNone;
#endif
        if (res != KErrNone)
            {
            ShowResultNoteL(res, CAknNoteDialog::EErrorTone);
            return res; // not sure if it's wise to exit now.
            }

        newPassword = _L("");
        }
    else
        {
        oldPassword.Copy(aOldPassword);
        newPassword.Copy(aNewPassword);
        }

    RDEBUG("res", res);
    while (newPassword.Length() == 0)
        {
        // this is not needed because the dialog won't allow to close, unless codes match
        // codes do not match -> note -> ask new pin and verification codes again
        // if(newPassword.Length()>0)
        //  ShowResultNoteL(R_CODES_DONT_MATCH, CAknNoteDialog::EErrorTone);

        newPassword = _L("");

        // new pin code query
        if (aOldPassword.Length() == 0) // only if input parameters are empty
            {
            queryAccepted = KErrCancel;
            CSecQueryUi *iSecQueryUi;
            iSecQueryUi = CSecQueryUi::NewL();
            // this queries both, and verifies itself
            queryAccepted = iSecQueryUi->SecQueryDialog(_L(
                    "PIN1-New|PIN1-Ver"), newPassword,
                    SEC_C_PIN_CODE_MIN_LENGTH, SEC_C_PIN_CODE_MAX_LENGTH,
                    ESecUiAlphaNotSupported | ESecUiCancelSupported
                            | ESecUiEmergencyNotSupported | secCodeType);
            RDEBUG("newPassword", 1);
            RDebug::Print(newPassword);
            RDEBUG("queryAccepted", queryAccepted);
            delete iSecQueryUi;
            if (queryAccepted != KErrNone)
                return KErrAbort;
            RDEBUG("0", 0);
            }
        }

    // send code
    passwords.iOldPassword = oldPassword;
    passwords.iNewPassword = newPassword;
    RDEBUG("passwords", 0);
    RDebug::Print(passwords.iOldPassword);
    RDebug::Print(passwords.iNewPassword);
    RDEBUG("SetRequestType", 0);
    iWait->SetRequestType(EMobilePhoneChangeSecurityCode);
    RDEBUG("ChangeSecurityCode", 0);
    iPhone.ChangeSecurityCode(iWait->iStatus, secCodeType, passwords);
    RDEBUG("WaitForRequestL", 0);
    res = iWait->WaitForRequestL();
    RDEBUG("WaitForRequestL res", res);
#ifdef __WINS__
    if (res == KErrNotSupported)
    res = KErrNone;
#endif

    switch (res)
        {
        case KErrNone:
            {
            // code changed 
            ShowResultNoteL(R_PIN_CODE_CHANGED_NOTE,
                    CAknNoteDialog::EConfirmationTone);
            break;
            }
        case KErrGsm0707IncorrectPassword:
        case KErrAccessDenied:
            {
            // code was entered erroneously. This is strange, because it was verified before
            ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
            ChangePinParamsL(_L(""), _L(""), aFlags, aCaption, aShowError);
            break;
            }
        case KErrGsmSSPasswordAttemptsViolation:
        case KErrLocked:
            {
            // Pin1 blocked! 
            return KErrLocked;
            }
        case KErrGsm0707OperationNotAllowed:
            {
            // not allowed with this sim
            ShowResultNoteL(R_OPERATION_NOT_ALLOWED,
                    CAknNoteDialog::EErrorTone);
            return KErrGsm0707OperationNotAllowed;
            }
        case KErrAbort:
            {
            break;
            }
        default:
            {
            ShowErrorNoteL(res);
            ChangePinParamsL(_L(""), _L(""), aFlags, aCaption, aShowError);
            break;
            }
        }
    return res;
    }
/*********************************************/
// qtdone
EXPORT_C TInt CSecuritySettings::ChangeUPinParamsL(
        RMobilePhone::TMobilePassword aOldPassword,
        RMobilePhone::TMobilePassword aNewPassword, TInt aFlags,
        TDes& aCaption, TInt aShowError)
    {
    RDEBUG("aFlags", aFlags);
    // the password parameters are not used
    if (aOldPassword.Length() > 0)
        RDebug::Print(aOldPassword);
    if (aNewPassword.Length() > 0)
        RDebug::Print(aNewPassword);

    if (aCaption.Length() > 0)
        RDebug::Print(aCaption);

    TBool wcdmaSupported(
            FeatureManager::FeatureSupported( KFeatureIdProtocolWcdma));
    TBool upinSupported(FeatureManager::FeatureSupported( KFeatureIdUpin));
    if (!(wcdmaSupported || upinSupported))
        {
        RDEBUG("! upinSupported", upinSupported);
        return KErrAccessDenied;
        }

    RDEBUG("upinSupported", upinSupported);
    TInt simState;
    TInt err(KErrGeneral);
    err = RProperty::Get(KPSUidStartup, KPSSimStatus, simState);
    User::LeaveIfError(err);
    TBool simRemoved(simState == ESimNotPresent);

    if (simRemoved)
        {
        ShowResultNoteL(R_INSERT_SIM, CAknNoteDialog::EErrorTone);
        return KErrAccessDenied;
        }

    RMobilePhone::TMobilePhoneSecurityCode secCodeType;
    secCodeType = RMobilePhone::ESecurityUniversalPin;

    RMobilePhone::TMobilePassword oldPassword;
    RMobilePhone::TMobilePassword newPassword;
    RMobilePhone::TMobilePassword verifcationPassword;
    RMobilePhone::TMobilePhonePasswordChangeV1 passwords;
    RMobilePhone::TMobilePhoneSecurityCodeInfoV5 codeInfo;
    RMobilePhone::TMobilePhoneSecurityCodeInfoV5Pckg codeInfoPkg(codeInfo);
    RMobilePhone::TMobilePhoneLock lockType;
    RMobilePhone::TMobilePhoneLockInfoV1 lockInfo;
    TInt queryAccepted = KErrCancel;

    lockType = RMobilePhone::ELockUniversalPin;

    RMobilePhone::TMobilePhoneLockInfoV1Pckg lockInfoPkg(lockInfo);
    iWait->SetRequestType(EMobilePhoneGetLockInfo);
    RDEBUG("GetLockInfo", 0);
    iPhone.GetLockInfo(iWait->iStatus, lockType, lockInfoPkg);
    RDEBUG("WaitForRequestL", 0);
    TInt res = iWait->WaitForRequestL();
    RDEBUG("WaitForRequestL res", res);
#ifdef __WINS__
    if (res == KErrNotSupported)
        {
        res = KErrNone;
        lockInfo.iSetting = RMobilePhone::ELockSetEnabled;
        }
#endif
    User::LeaveIfError(res);

    RDEBUG("lockInfo.iSetting", lockInfo.iSetting);
    RDEBUG("RMobilePhone::ELockSetDisabled", RMobilePhone::ELockSetDisabled);
    if (lockInfo.iSetting == RMobilePhone::ELockSetDisabled)
        {
        ShowResultNoteL(R_UPIN_NOT_ALLOWED, CAknNoteDialog::EErrorTone);
        return KErrAccessDenied;
        }

    iWait->SetRequestType(EMobilePhoneGetSecurityCodeInfo);
    RDEBUG("GetSecurityCodeInfo", 0);
    iPhone.GetSecurityCodeInfo(iWait->iStatus, secCodeType, codeInfoPkg);
    RDEBUG("WaitForRequestL", 0);
    res = iWait->WaitForRequestL();
    RDEBUG("WaitForRequestL res", res);
#ifdef __WINS__
    if (res == KErrNotSupported || res == KErrTimedOut)
        {
        res = KErrNone;
        codeInfo.iRemainingEntryAttempts=KMaxNumberOfPINAttempts;
        }
#endif
    User::LeaveIfError(res);

    RDEBUG("codeInfo.iRemainingEntryAttempts",
            codeInfo.iRemainingEntryAttempts);
    if (codeInfo.iRemainingEntryAttempts == KMaxNumberOfPINAttempts)
        codeInfo.iRemainingEntryAttempts = -1;

    queryAccepted = KErrCancel;
    CSecQueryUi *iSecQueryUi;
    iSecQueryUi = CSecQueryUi::NewL();
    TBuf<0x100> title;
    title.Zero();
    title.Append(_L("UPIN-Old"));
    title.Append(_L("#"));
    title.AppendNum(codeInfo.iRemainingEntryAttempts);
    queryAccepted = iSecQueryUi->SecQueryDialog(title, oldPassword,
            SEC_C_PIN_CODE_MIN_LENGTH, SEC_C_PIN_CODE_MAX_LENGTH,
            ESecUiAlphaNotSupported | ESecUiCancelSupported
                    | ESecUiEmergencyNotSupported | secCodeType);
    RDEBUG("oldPassword", 0);
    RDebug::Print(oldPassword);
    RDEBUG("queryAccepted", queryAccepted);
    delete iSecQueryUi;
    if (queryAccepted != KErrNone)
        return KErrAbort;
    res = KErrNone; // indicate that everything is ok

        {
        queryAccepted = KErrCancel;
        CSecQueryUi * iSecQueryUi;
        iSecQueryUi = CSecQueryUi::NewL();
        // this queries both, and verifies itself
        queryAccepted = iSecQueryUi->SecQueryDialog(
                _L("UPIN1-New|UPIN1-Ver"), newPassword,
                SEC_C_PIN_CODE_MIN_LENGTH, SEC_C_PIN_CODE_MAX_LENGTH,
                ESecUiAlphaNotSupported | ESecUiCancelSupported
                        | ESecUiEmergencyNotSupported | secCodeType);
        RDEBUG("newPassword", 0);
        RDebug::Print(newPassword);
        RDEBUG("queryAccepted", queryAccepted);
        delete iSecQueryUi;
        if (queryAccepted != KErrNone)
            return KErrAbort;
        }
    // send code
    passwords.iOldPassword = oldPassword;
    passwords.iNewPassword = newPassword;
    iWait->SetRequestType(EMobilePhoneChangeSecurityCode);
    RDEBUG("ChangeSecurityCode", 0);
    iPhone.ChangeSecurityCode(iWait->iStatus, secCodeType, passwords);
    RDEBUG("WaitForRequestL", 0);
    res = iWait->WaitForRequestL();
    RDEBUG("WaitForRequestL res", res);
#ifdef __WINS__
    if (res == KErrNotSupported)
    res = KErrNone;
#endif
    switch (res)
        {
        case KErrNone:
            {
            // code changed 
            ShowResultNoteL(R_UPIN_CODE_CHANGED_NOTE,
                    CAknNoteDialog::EConfirmationTone);
            break;
            }
        case KErrGsm0707IncorrectPassword:
        case KErrAccessDenied:
            {
            // code was entered erroneously
            ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
            ChangeUPinParamsL(_L(""), _L(""), aFlags, aCaption, aShowError);
            break;
            }
        case KErrGsmSSPasswordAttemptsViolation:
        case KErrLocked:
            {
            return KErrLocked;
            }
        case KErrGsm0707OperationNotAllowed:
            {
            // not allowed with this sim
            ShowResultNoteL(R_OPERATION_NOT_ALLOWED,
                    CAknNoteDialog::EErrorTone);
            return KErrGsm0707OperationNotAllowed;
            }
        case KErrAbort:
            {
            break;
            }
        default:
            {
            ShowErrorNoteL(res);
            ChangeUPinParamsL(_L(""), _L(""), aFlags, aCaption, aShowError);
            break;
            }
        }
    return res;
    }
/***************************************/
// qtdone
EXPORT_C TInt CSecuritySettings::ChangePin2ParamsL(
        RMobilePhone::TMobilePassword aOldPassword,
        RMobilePhone::TMobilePassword aNewPassword, TInt aFlags,
        TDes& aCaption, TInt aShowError)
    {
    RDEBUG("aFlags", aFlags);
    // the password parameters are not used
    if (aOldPassword.Length() > 0)
        RDebug::Print(aOldPassword);
    if (aNewPassword.Length() > 0)
        RDebug::Print(aNewPassword);

    if (aCaption.Length() > 0)
        RDebug::Print(aCaption);

    TInt simState;
    TInt err(KErrGeneral);
    err = RProperty::Get(KPSUidStartup, KPSSimStatus, simState);
    User::LeaveIfError(err);
    TBool simRemoved(simState == ESimNotPresent);

    if (simRemoved)
        {
        ShowResultNoteL(R_INSERT_SIM, CAknNoteDialog::EErrorTone);
        return KErrAccessDenied;
        }

    RMmCustomAPI::TSecurityCodeType secCodeType;
    RMobilePhone::TMobilePhoneSecurityCode EtelsecCodeType;
    secCodeType = RMmCustomAPI::ESecurityCodePin2;
    RMobilePhone::TMobilePassword oldPassword;
    RMobilePhone::TMobilePassword newPassword;
    RMobilePhone::TMobilePassword verifcationPassword;
    RMobilePhone::TMobilePhonePasswordChangeV1 passwords;
    RMobilePhone::TMobilePhoneSecurityCodeInfoV5 codeInfo;
    RMobilePhone::TMobilePhoneSecurityCodeInfoV5Pckg codeInfoPkg(codeInfo);
    TInt queryAccepted = KErrCancel;

    // check if pin2 is blocked...
    TBool isBlocked = EFalse;

    TInt ret = iCustomPhone.IsBlocked(secCodeType, isBlocked);
    RDEBUG("isBlocked", isBlocked);
    if (isBlocked)
        return KErrAccessDenied;
    RDEBUG("ret", ret);
#ifdef __WINS__
    if (ret == KErrNotSupported)
    ret = KErrNone;
#endif

    if (ret != KErrNone)
        {
        switch (ret)
            {
            // PIN2 Blocked.
            case KErrGsm0707SIMPuk2Required:
                break;
            case KErrGsmSSPasswordAttemptsViolation:
            case KErrLocked:
                // Pin2 features blocked permanently!
                ShowResultNoteL(R_PIN2_REJECTED,
                        CAknNoteDialog::EConfirmationTone);
                break;
            case KErrGsm0707SimNotInserted:
                // not allowed with this sim
                ShowResultNoteL(R_OPERATION_NOT_ALLOWED,
                        CAknNoteDialog::EErrorTone);
                break;
            default:
                ShowErrorNoteL(ret);
                break;
            }
        return KErrAccessDenied;
        }

    // Security code must be changed to Etel API format
    // Custom API Pin1 and Pin2 have the same enum values as the Etel ones
    EtelsecCodeType = (RMobilePhone::TMobilePhoneSecurityCode) secCodeType;
    iWait->SetRequestType(EMobilePhoneGetSecurityCodeInfo);
    RDEBUG("GetSecurityCodeInfo", 0);
    iPhone.GetSecurityCodeInfo(iWait->iStatus, EtelsecCodeType, codeInfoPkg);
    RDEBUG("WaitForRequestL", 0);
    ret = iWait->WaitForRequestL();
    RDEBUG("WaitForRequestL ret", ret);
#ifdef __WINS__
    if ( ret == KErrNotSupported || ret == KErrTimedOut)
        {
        codeInfo.iRemainingEntryAttempts = 1;
        ret = KErrNone;
        }
#endif
    User::LeaveIfError(ret);

    RDEBUG("codeInfo.iRemainingEntryAttempts",
            codeInfo.iRemainingEntryAttempts);
    if (codeInfo.iRemainingEntryAttempts == KMaxNumberOfPINAttempts)
        codeInfo.iRemainingEntryAttempts = -1;

    /* request PIN using QT */
    queryAccepted = KErrCancel;
    CSecQueryUi *iSecQueryUi;
    iSecQueryUi = CSecQueryUi::NewL();
    TBuf<0x100> title;
    title.Zero();
    title.Append(_L("PIN2-Old"));
    title.Append(_L("#"));
    title.AppendNum(codeInfo.iRemainingEntryAttempts);
    queryAccepted = iSecQueryUi->SecQueryDialog(title, oldPassword,
            SEC_C_PIN2_CODE_MIN_LENGTH, SEC_C_PIN2_CODE_MAX_LENGTH,
            ESecUiAlphaNotSupported | ESecUiCancelSupported
                    | ESecUiEmergencyNotSupported | secCodeType);
    RDEBUG("oldPassword", 0);
    RDebug::Print(oldPassword);
    RDEBUG("queryAccepted", queryAccepted);
    delete iSecQueryUi;
    if (queryAccepted != KErrNone)
        return KErrAbort;
    /* end request PIN using QT */

    /* request PIN using QT */
        {
        queryAccepted = KErrCancel;
        CSecQueryUi * iSecQueryUi;
        iSecQueryUi = CSecQueryUi::NewL();
        // this queries both, and verifies itself
        queryAccepted = iSecQueryUi->SecQueryDialog(_L("PIN2-New|PIN2-Ver"),
                newPassword, SEC_C_PIN2_CODE_MIN_LENGTH,
                SEC_C_PIN2_CODE_MAX_LENGTH, ESecUiAlphaNotSupported
                        | ESecUiCancelSupported | ESecUiEmergencyNotSupported
                        | secCodeType);
        RDEBUG("newPassword", 0);
        RDebug::Print(newPassword);
        RDEBUG("queryAccepted", queryAccepted);
        delete iSecQueryUi;
        if (queryAccepted != KErrNone)
            return KErrAbort;
        }
    /* end request PIN using QT */

    passwords.iOldPassword = oldPassword;
    passwords.iNewPassword = newPassword;
    iWait->SetRequestType(EMobilePhoneChangeSecurityCode);
    RDEBUG("ChangeSecurityCode", 0);
    iPhone.ChangeSecurityCode(iWait->iStatus, EtelsecCodeType, passwords);
    RDEBUG("WaitForRequestL", 0);
    TInt res = iWait->WaitForRequestL();
    RDEBUG("WaitForRequestL res", res);
#ifdef __WINS__
    if (res == KErrNotSupported)
    res = KErrNone;
#endif
    switch (res)
        {
        case KErrNone:
            {
            // code changed 
            ShowResultNoteL(R_PIN2_CODE_CHANGED_NOTE,
                    CAknNoteDialog::EConfirmationTone);
            break;
            }
        case KErrGsm0707IncorrectPassword:
        case KErrAccessDenied:
            {
            ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
            ChangePin2ParamsL(_L(""), _L(""), aFlags, aCaption, aShowError);
            break;
            }
        case KErrGsmSSPasswordAttemptsViolation:
        case KErrLocked:
            {
            // Pin2 blocked!
            ShowResultNoteL(KErrLocked, CAknNoteDialog::EErrorTone);
            CSecurityHandler* handler = new (ELeave) CSecurityHandler(iPhone);
            CleanupStack::PushL(handler);
            handler->HandleEventL(RMobilePhone::EPuk2Required);
            CleanupStack::PopAndDestroy(handler); // handler    
            return KErrLocked;
            }
        case KErrGsm0707OperationNotAllowed:
            {
            // not allowed with this sim
            ShowResultNoteL(R_OPERATION_NOT_ALLOWED,
                    CAknNoteDialog::EErrorTone);
            return KErrGsm0707OperationNotAllowed;
            }
        case KErrAbort:
            {
            break;
            }
        default:
            {
            ShowErrorNoteL(res);
            ChangePin2ParamsL(_L(""), _L(""), aFlags, aCaption, aShowError);
            break;
            }
        }
    return res;
    }
/************************************************/
// qtdone
EXPORT_C TInt CSecuritySettings::ChangeSecCodeParamsL(
        RMobilePhone::TMobilePassword aOldPassword,
        RMobilePhone::TMobilePassword aNewPassword, TInt aFlags,
        TDes& aCaption, TInt aShowError)
    {
    RDEBUG("aFlags", aFlags);
    RDEBUG("aShowError", aShowError);
    /*****************************************************
     *    Series 60 Customer / ETel
     *    Series 60  ETel API
     *****************************************************/
    TInt res = KErrNone;
    TInt queryAccepted = KErrCancel;
    RMobilePhone::TMobilePassword newPassword;

    RMobilePhone::TMobilePhoneSecurityCode secCodeType;
    secCodeType = RMobilePhone::ESecurityCodePhonePassword;
    RMobilePhone::TMobilePassword oldPassword;
    RMobilePhone::TMobilePassword required_fourth;
    RMobilePhone::TMobilePhonePasswordChangeV1 passwords;

    if (aOldPassword.Length() == 0)
        {
        queryAccepted = KErrCancel;
        CSecQueryUi *iSecQueryUi;
        iSecQueryUi = CSecQueryUi::NewL();
        queryAccepted = iSecQueryUi->SecQueryDialog(_L("Lock-Old"),
                oldPassword, SEC_C_SECURITY_CODE_MIN_LENGTH,
                SEC_C_SECURITY_CODE_MAX_LENGTH, ESecUiAlphaSupported
                        | ESecUiCancelSupported | ESecUiEmergencyNotSupported
                        | secCodeType);
        RDEBUG("oldPassword", 0);
        RDebug::Print(oldPassword);
        RDEBUG("queryAccepted", queryAccepted);
        delete iSecQueryUi;
        if (queryAccepted != KErrNone)
            return KErrAbort;
        newPassword = _L("");
        }
    else
        {
        oldPassword.Copy(aOldPassword);
        newPassword.Copy(aNewPassword);
        }

    RDEBUG("EMobilePhoneVerifySecurityCode", EMobilePhoneVerifySecurityCode);
    iWait->SetRequestType(EMobilePhoneVerifySecurityCode);
    // check code before proceeding
    RDEBUG("VerifySecurityCode", 0);
    iPhone.VerifySecurityCode(iWait->iStatus, secCodeType, oldPassword,
            required_fourth);
    RDEBUG("WaitForRequestL", 0);
    res = iWait->WaitForRequestL();
    RDEBUG("WaitForRequestL res", res);
#ifdef __WINS__
    if (res == KErrNotSupported)
    res = KErrNone;
#endif

    if (res != KErrNone)
        {
        ShowResultNoteL(res, CAknNoteDialog::EErrorTone);
        return res;
        }

    while (newPassword.Length() == 0)
        {
        // codes do not match -> note -> ask new pin and verification codes again  
        // note that this never happens because the dialog doesn't dismiss until both codes match
        if (newPassword.Length() > 0)
            ShowResultNoteL(R_CODES_DONT_MATCH, CAknNoteDialog::EErrorTone);

            {
            queryAccepted = KErrCancel;
            CSecQueryUi *iSecQueryUi;
            iSecQueryUi = CSecQueryUi::NewL();
            // will ask both codes and compare itself
            // mix, max , alpha is handled using TARM params, in the dialog itself
            TInt lType = ESecUiAlphaSupported | ESecUiCancelSupported
                    | ESecUiEmergencyNotSupported | secCodeType;
            RDEBUG("lType", lType);
            queryAccepted = iSecQueryUi->SecQueryDialog(_L(
                    "Lock-New|Lock-Verif"), newPassword,
                    SEC_C_PIN_CODE_MIN_LENGTH, SEC_C_PIN_CODE_MAX_LENGTH,
                    lType);
            RDEBUG("newPassword", 0);
            RDebug::Print(newPassword);
            RDEBUG("queryAccepted", queryAccepted);
            delete iSecQueryUi;
            if (queryAccepted != KErrNone)
                return KErrAbort;
            }

        } // while

    // change code
    RDEBUG("res", res);
    if (res == KErrNone)
        {
        passwords.iOldPassword = oldPassword;
        passwords.iNewPassword = newPassword;
        iWait->SetRequestType(EMobilePhoneChangeSecurityCode);
        RDEBUG("ChangeSecurityCode", 0);
        iPhone.ChangeSecurityCode(iWait->iStatus, secCodeType, passwords);
        RDEBUG("WaitForRequestL", 0);
        res = iWait->WaitForRequestL();
        RDEBUG("WaitForRequestL res", res);
#ifdef __WINS__
        if (res == KErrNotSupported)
        res = KErrNone;
#endif

        if (res == KErrNone && 1 == 0) // TODO not possible to enable because it asks code again
            {
            RMobilePhone::TMobilePhoneLock lockType =
                    RMobilePhone::ELockPhoneDevice;
            RMobilePhone::TMobilePhoneLockSetting lockChangeSetting =
                    RMobilePhone::ELockSetEnabled;
            iWait->SetRequestType(EMobilePhoneSetLockSetting);
            RDEBUG("SetLockSetting", 0);
            iPhone.SetLockSetting(iWait->iStatus, lockType, lockChangeSetting);
            RDEBUG("WaitForRequestL", 0);
            res = iWait->WaitForRequestL();
            RDEBUG("WaitForRequestL res", res);
#ifdef __WINS__
            if (res == KErrNotSupported || res == KErrTimedOut)
            res = KErrNone;
#endif
            }
        }

    RDEBUG("res", res);
    switch (res)
        {
        case KErrNone:
            {
            // code changed 
            ShowResultNoteL(R_SECURITY_CODE_CHANGED_NOTE,
                    CAknNoteDialog::EConfirmationTone);
                {
                // Send the changed code to the SCP server, even with device lock enhancements.
                RDEBUG("scpClient.Connect", 0);
                RSCPClient scpClient;
                TSCPSecCode newCode;
                TSCPSecCode oldPassword;
                newCode.Copy(newPassword);
                if (scpClient.Connect() == KErrNone)
                    {
                    RDEBUG("scpClient.StoreCode", 0);
                     // this is the old method. Obsolete now
                     // scpClient.StoreCode( newCode );
                     RArray<TDevicelockPolicies> aFailedPolicies;
                     TDevicelockPolicies failedPolicy;
                     TInt retLockcode = KErrNone;
                     retLockcode = scpClient.StoreLockcode( newCode, oldPassword, aFailedPolicies );
                     RDEBUG( "retLockcode", retLockcode );
                     RDEBUG( "aFailedPolicies.Count()", aFailedPolicies.Count() );
                     for(TInt i=0; i<aFailedPolicies.Count(); i++)
	                     {
	                     failedPolicy = aFailedPolicies[i];
	                     RDEBUG( "failedPolicy", failedPolicy );
	                     }
                    scpClient.Close();
                    }
                }
            break;
            }
        case KErrGsmSSPasswordAttemptsViolation:
        case KErrLocked:
            {
            ShowResultNoteL(R_SEC_BLOCKED, CAknNoteDialog::EErrorTone);
            ChangeSecCodeParamsL(aOldPassword, aNewPassword, aFlags,
                    aCaption, aShowError);
            break;
            }
        case KErrGsm0707IncorrectPassword:
        case KErrAccessDenied:
            {
            // code was entered erroneously
            ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
            ChangeSecCodeParamsL(aOldPassword, aNewPassword, aFlags,
                    aCaption, aShowError);
            break;
            }
        case KErrAbort:
            {
            break;
            }
        default:
            {
            ShowErrorNoteL(res);
            ChangeSecCodeParamsL(aOldPassword, aNewPassword, aFlags,
                    aCaption, aShowError);
            break;
            }
        } // switch
    return res;
    }

/**************************************/
// qtdone
// the params are changed in the settings,. This only asks for password.
EXPORT_C TInt CSecuritySettings::ChangeAutoLockPeriodParamsL(TInt aPeriod,
        RMobilePhone::TMobilePassword aOldPassword, TInt aFlags,
        TDes& aCaption, TInt aShowError)
    {
    RDEBUG("aPeriod", aPeriod);
    RDEBUG("aFlags", aFlags);
    /*****************************************************
     *    Series 60 Customer / ETel
     *    Series 60  ETel API
     *****************************************************/

    RMobilePhone::TMobilePhoneLockSetting lockChange(
            RMobilePhone::ELockSetDisabled);
    RMobilePhone::TMobilePhoneLock lockType = RMobilePhone::ELockPhoneDevice;
    TInt oldPeriod = aPeriod;

    TInt maxPeriod = 0;
    if (FeatureManager::FeatureSupported(KFeatureIdSapTerminalControlFw ))
        {
        // Retrieve the current autolock period max. value from the SCP server, 
        // and check that the value the user
        // selected is ok from the Corporate Policy point of view.
        RSCPClient scpClient;
        TInt ret = scpClient.Connect();
        if (ret == KErrNone)
            {
            CleanupClosePushL(scpClient);
            TBuf<KSCPMaxIntLength> maxPeriodBuf;
            if (scpClient.GetParamValue(ESCPMaxAutolockPeriod, maxPeriodBuf)
                    == KErrNone)
                {
                TLex lex(maxPeriodBuf);
                if ((lex.Val(maxPeriod) == KErrNone) && (maxPeriod > 0))
                    {
                    RDEBUG("from SCP maxPeriod", maxPeriod);
                    }
                else
                    {
                    maxPeriod = 0;
                    RDEBUG("not from SCP maxPeriod", maxPeriod);
                    }
                }
            else
                {
                RDEBUG("Failed to retrieve max period", maxPeriod);
                }
            }
        else
            {
            RDEBUG("Failed to connect to SCP", 0);
            }
        CleanupStack::PopAndDestroy(); // scpClient 
        }
    RDEBUG("maxPeriod", maxPeriod);
    if (FeatureManager::FeatureSupported(KFeatureIdSapTerminalControlFw ))
        {
        TBool allow = ETrue;

        if ((aPeriod == 0) && (maxPeriod > 0))
            {
            	RDEBUG("The period is not allowed by TARM", aPeriod);
            	RDEBUG( "maxPeriod", maxPeriod );
            allow = EFalse;
            ShowResultNoteL(R_SECUI_TEXT_AUTOLOCK_MUST_BE_ACTIVE,
                    CAknNoteDialog::EErrorTone);
            }
        if (!allow)
            {
            return ChangeAutoLockPeriodParamsL(aPeriod, aOldPassword, aFlags,
                    aCaption, aShowError); // ask again
            }
        }

    if (aPeriod == 0)
        {
        // If remote lock is enabled, don't disable the domestic OS device lock
        // since that would render the RemoteLock useless.
        // Instead just re-set the DOS lock to enabled which as a side effect
        // requests the security code from the user.

        TBool remoteLockStatus( EFalse );
        CRemoteLockSettings* remoteLockSettings = CRemoteLockSettings::NewL();

        if ( remoteLockSettings->GetEnabled( remoteLockStatus ) )
            {
            if ( remoteLockStatus )
                {
                // Remote lock is enabled
                lockChange = RMobilePhone::ELockSetEnabled;
                RDEBUG( "RemoteLock is enabled lockChange", lockChange );
                }
            else
                {
                // Remote lock is disabled
                lockChange = RMobilePhone::ELockSetDisabled;
                RDEBUG( "RemoteLock is disabled lockChange", lockChange );
                }
            }
        else
            {
            // Failed to get remote lock status
            RDEBUG( "Failed to get remote lock status lockChange", lockChange );
            }

        delete remoteLockSettings;
        remoteLockSettings = NULL;

        }
    else
        {
        lockChange = RMobilePhone::ELockSetEnabled;
        RDEBUG("aPeriod != 0 lockChange", lockChange);
        }

    iWait->SetRequestType(EMobilePhoneSetLockSetting);
    RDEBUG("lockChange", lockChange);
    RProperty::Set(KPSUidSecurityUIs, KSecurityUIsQueryRequestCancel,
            ESecurityUIsQueryRequestOk);
    RDEBUG("SetLockSetting", 0);
    iPhone.SetLockSetting(iWait->iStatus, lockType, lockChange); // this eventually calls PassPhraseRequiredL
    RDEBUG("WaitForRequestL", 0);
    TInt status = KErrNone;
    status = iWait->WaitForRequestL();
    RDEBUG("WaitForRequestL status", status);
#ifdef __WINS__
    if (status == KErrNotSupported || status == KErrTimedOut)
    status = KErrNone;
#endif
    switch (status)
        {
        case KErrNone:
            break;
        case KErrGsmSSPasswordAttemptsViolation:
        case KErrLocked:
        		RDEBUG("KErrLocked", KErrLocked);
            ShowResultNoteL(KErrLocked, CAknNoteDialog::EErrorTone); // the old code didn't show messages
            return ChangeAutoLockPeriodParamsL(oldPeriod, aOldPassword,
                    aFlags, aCaption, aShowError); // ask again
        case KErrGsm0707IncorrectPassword:
        case KErrAccessDenied:
        		RDEBUG("KErrAccessDenied", KErrAccessDenied);
            // code was entered erroneously
            ShowResultNoteL(KErrAccessDenied, CAknNoteDialog::EErrorTone); // the old code didn't show messages
            return ChangeAutoLockPeriodParamsL(oldPeriod, aOldPassword,
                    aFlags, aCaption, aShowError); // ask again
        case KErrAbort:
            // User pressed "cancel" in the code query dialog.
            return oldPeriod;
        default:
        		RDEBUG("default", status);
            ShowResultNoteL(status, CAknNoteDialog::EErrorTone); // the old code didn't show messages
            return ChangeAutoLockPeriodParamsL(oldPeriod, aOldPassword,
                    aFlags, aCaption, aShowError); // ask again
        }
 		RDEBUG("aPeriod", aPeriod);
    return aPeriod;
    }
/*****************************/
// qtdone
EXPORT_C TInt CSecuritySettings::ChangePinRequestParamsL(TInt aEnable,
        RMobilePhone::TMobilePassword aOldPassword, TInt aFlags,
        TDes& aCaption, TInt aShowError)
    {
    RDEBUG("aEnable", aEnable);
    RDEBUG("aFlags", aFlags);
    TInt simState = 0;
    TInt lEnable = aEnable;
    TInt err(KErrGeneral);
    err = RProperty::Get(KPSUidStartup, KPSSimStatus, simState);
    User::LeaveIfError(err);
    TBool simRemoved(simState == ESimNotPresent);

    if (simRemoved)
        {
        ShowResultNoteL(R_INSERT_SIM, CAknNoteDialog::EErrorTone);
        return KErrAccessDenied;
        }

    RMobilePhone::TMobilePhoneLockInfoV1 lockInfo;
    RMobilePhone::TMobilePhoneLockInfoV1Pckg lockInfoPkg(lockInfo);
    RMobilePhone::TMobilePhoneLock lockType = RMobilePhone::ELockICC;

    RMobilePhone::TMobilePhoneLockSetting lockChangeSetting;

    //get lock info
    iWait->SetRequestType(EMobilePhoneGetLockInfo);
    iPhone.GetLockInfo(iWait->iStatus, lockType, lockInfoPkg);
    RDEBUG("WaitForRequestL", 0);
    TInt status = iWait->WaitForRequestL();
    RDEBUG("WaitForRequestL status", status);
#ifdef __WINS__
    if (status == KErrNotSupported || status == KErrTimedOut)
        {
        lockInfo.iSetting = RMobilePhone::ELockSetDisabled;
        status = KErrNone;
        }
#endif
    User::LeaveIfError(status);

    if (aOldPassword.Length() == 0) // only if input parameters are empty
        {
        // switch the value.
        if (lockInfo.iSetting == RMobilePhone::ELockSetDisabled)
            lEnable = 1; // on
        else
            lEnable = 0; // off
        }

		RDEBUG("lEnable", lEnable);
    if (lEnable == 0)
        {
        lockChangeSetting = RMobilePhone::ELockSetDisabled;
        }
    else
        {
        lockChangeSetting = RMobilePhone::ELockSetEnabled;
        }

		RDEBUG("lockChangeSetting", lockChangeSetting);
    // Raise a flag to indicate that the PIN
    // request coming from ETEL has originated from SecUi and not from Engine.
    TInt tRet = RProperty::Set(KPSUidSecurityUIs,
            KSecurityUIsSecUIOriginatedQuery, ESecurityUIsSecUIOriginated);
		RDEBUG("tRet", tRet);

    // Change the lock setting
    iWait->SetRequestType(EMobilePhoneSetLockSetting);
    RProperty::Set(KPSUidSecurityUIs, KSecurityUIsQueryRequestCancel,
            ESecurityUIsQueryRequestOk);
    RDEBUG("SetLockSetting", 0);
    iPhone.SetLockSetting(iWait->iStatus, lockType, lockChangeSetting); // this will trigger Pin1RequiredL
    RDEBUG("WaitForRequestL", 0);
    status = iWait->WaitForRequestL();
    RDEBUG("WaitForRequestL status", status);
#ifdef __WINS__
    if (status == KErrNotSupported || status == KErrTimedOut)
    status = KErrNone;
#endif

    // Lower the flag                             
    RProperty::Set(KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery,
            ESecurityUIsSecUIOriginatedUninitialized);

    switch (status)
        {
        case KErrNone:
            {
            break;
            }
        case KErrGsm0707OperationNotAllowed:
            {
            // not allowed with this sim
            ShowResultNoteL(R_OPERATION_NOT_ALLOWED,
                    CAknNoteDialog::EErrorTone);
            return KErrGsm0707OperationNotAllowed;
            }
        case KErrGsm0707IncorrectPassword:
        case KErrAccessDenied:
            {
            // code was entered erroneously
            return ChangePinRequestParamsL(aEnable, aOldPassword, aFlags,
                    aCaption, aShowError);
            }
        case KErrGsmSSPasswordAttemptsViolation:
        case KErrLocked:
            {
            return KErrLocked;
            }
        case KErrAbort:
            {
            return KErrAbort;
            }
        default:
            {
            return ChangePinRequestParamsL(aEnable, aOldPassword, aFlags,
                    aCaption, aShowError);
            }
        }
    return KErrNone;
    }

//
// ----------------------------------------------------------
// CSecuritySettings::AskSecCodeParamsL()
// For asking security code e.g in settings
// not used
// ----------------------------------------------------------
// qtdone
EXPORT_C TBool CSecuritySettings::AskSecCodeParamsL(
        RMobilePhone::TMobilePassword &aOldPassword, TInt aFlags,
        TDes& aCaption, TInt aShowError)
    {
    RDEBUG("aFlags", aFlags);
    RDEBUG("aShowError", aShowError);
    RDEBUG("This doesn't do anything", 0);
    RDEBUG("aFlags", aFlags);

    // the password parameters are not used
    if (aOldPassword.Length() > 0)
        RDebug::Print(aOldPassword);

    return EFalse;
    }

// End of file
