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
#include <SecUi.rsg>
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
#include "secuisecuritysettings.h"
#include "SecUiAutoLockSettingPage.h"
#include "secui.hrh"
#include "secuisecurityhandler.h"
#include "secuicodequerydialog.h"
#include "SecUiWait.h"

#ifdef RD_REMOTELOCK
#include <aknnotewrappers.h>
#include <StringLoader.h>
#include <RemoteLockSettings.h>
#include "SecUiRemoteLockSettingPage.h"
#endif // RD_REMOTELOCK
#include <featmgr.h>
    /*****************************************************
    *    Series 60 Customer / TSY
    *    Needs customer TSY implementation
    *****************************************************/
//  LOCAL CONSTANTS AND MACROS  

const TInt KTriesToConnectServer( 2 );
const TInt KTimeBeforeRetryingServerConnection( 50000 );
const TInt PhoneIndex( 0 );

const TInt KMaxNumberOfPINAttempts(3);
const TInt KLastRemainingInputAttempt(1);

// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CSecuritySettings::NewL()
// ----------------------------------------------------------
//
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
//
EXPORT_C CSecuritySettings::CSecuritySettings()
    {
    }
//
// ----------------------------------------------------------
// CSecuritySettings::ConstructL()
// Symbian OS constructor.
// ----------------------------------------------------------
//
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

    TInt err( KErrGeneral );
    TInt thisTry( 0 );
    iWait = CWait::NewL();
    RTelServer::TPhoneInfo PhoneInfo;
    /* All server connections are tried to be made KTriesToConnectServer times because occasional
    fails on connections are possible, at least on some servers */
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecuritySettings::ConstructL()"));
    #endif
    
    FeatureManager::InitializeLibL();
    // connect to ETel server
    while ( ( err = iServer.Connect() ) != KErrNone && ( thisTry++ ) <= KTriesToConnectServer )
        {
        User::After( KTimeBeforeRetryingServerConnection );
        }
    User::LeaveIfError( err );

    // load TSY
    err = iServer.LoadPhoneModule( KMmTsyModuleName );
    if ( err != KErrAlreadyExists )
        {
        // May return also KErrAlreadyExists if something else
        // has already loaded the TSY module. And that is
        // not an error.
        User::LeaveIfError( err );
        }

    // open phones
    User::LeaveIfError(iServer.SetExtendedErrorGranularity(RTelServer::EErrorExtended));
    User::LeaveIfError(iServer.GetPhoneInfo(PhoneIndex, PhoneInfo));
    User::LeaveIfError(iPhone.Open(iServer,PhoneInfo.iName));
    User::LeaveIfError(iCustomPhone.Open(iPhone));

    iSecurityHandler = new( ELeave ) CSecurityHandler( iPhone );
    }
//
// ----------------------------------------------------------
// CSecuritySettings::~CSecuritySettings()
// Destructor
// ----------------------------------------------------------
//
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
    if(iWait->IsActive())
    {
        #if defined(_DEBUG)
	    RDebug::Print(_L("(SECUI)CManualSecuritySettings::~CSecuritySettings() CANCEL REQ"));
	    #endif
        iPhone.CancelAsyncRequest(iWait->GetRequestType());
        
        switch(iWait->GetRequestType())
            {   //inform query that it has beeen canceled
                case EMobilePhoneSetLockSetting:
                case EMobilePhoneSetFdnSetting:
                    RProperty::Set(KPSUidSecurityUIs, KSecurityUIsQueryRequestCancel, ESecurityUIsQueryRequestCanceled);
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
//
EXPORT_C void CSecuritySettings::ChangePinL()
    {
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
    
    TInt simState;
    TInt err( KErrGeneral );
    err = RProperty::Get(KPSUidStartup, KPSSimStatus, simState);
    User::LeaveIfError( err );
    TBool simRemoved(simState == ESimNotPresent);

    if ( simRemoved )
        {
        ShowResultNoteL(R_INSERT_SIM, CAknNoteDialog::EErrorTone);
        return;
        }

    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecuritySettings::ChangePinL()"));
    #endif    
    RMobilePhone::TMobilePhoneSecurityCode secCodeType;
    secCodeType = RMobilePhone::ESecurityCodePin1;

    RMobilePhone::TMobilePassword oldPassword;
    RMobilePhone::TMobilePassword newPassword;
    RMobilePhone::TMobilePassword verifcationPassword;
    RMobilePhone::TMobilePhonePasswordChangeV1 passwords;
    RMobilePhone::TMobilePhoneSecurityCodeInfoV5 codeInfo;
    RMobilePhone::TMobilePhoneSecurityCodeInfoV5Pckg codeInfoPkg(codeInfo);

    CCodeQueryDialog* verdlg = new (ELeave) CCodeQueryDialog (verifcationPassword,SEC_C_PIN_CODE_MIN_LENGTH,SEC_C_PIN_CODE_MAX_LENGTH,ESecUiNone);
    CleanupStack::PushL(verdlg);

    CCodeQueryDialog* newdlg = new (ELeave) CCodeQueryDialog(newPassword,SEC_C_PIN_CODE_MIN_LENGTH,SEC_C_PIN_CODE_MAX_LENGTH,ESecUiNone);
    CleanupStack::PushL(newdlg);

    CCodeQueryDialog* dlg = new (ELeave) CCodeQueryDialog (oldPassword,SEC_C_PIN_CODE_MIN_LENGTH,SEC_C_PIN_CODE_MAX_LENGTH,ESecUiNone);       
    CleanupStack::PushL(dlg);

    RMobilePhone::TMobilePhoneLock lockType;
    RMobilePhone::TMobilePhoneLockInfoV1 lockInfo;
    
    lockType = RMobilePhone::ELockICC;

    RMobilePhone::TMobilePhoneLockInfoV1Pckg lockInfoPkg(lockInfo);
    iWait->SetRequestType(EMobilePhoneGetLockInfo);
    iPhone.GetLockInfo(iWait->iStatus, lockType, lockInfoPkg);
    TInt res = iWait->WaitForRequestL();
    User::LeaveIfError(res);

    if (lockInfo.iSetting == RMobilePhone::ELockSetDisabled)
        {    
        CleanupStack::PopAndDestroy(3,verdlg);
        ShowResultNoteL(R_PIN_NOT_ALLOWED, CAknNoteDialog::EErrorTone);
        return;
        }
    
    CleanupStack::Pop(); // dlg
    iWait->SetRequestType(EMobilePhoneGetSecurityCodeInfo);
    iPhone.GetSecurityCodeInfo(iWait->iStatus, secCodeType, codeInfoPkg);
    res = iWait->WaitForRequestL();
    User::LeaveIfError(res);
    // ask pin
    if( codeInfo.iRemainingEntryAttempts >= KMaxNumberOfPINAttempts )
            res = dlg->ExecuteLD(R_PIN_QUERY);
    else if(codeInfo.iRemainingEntryAttempts > KLastRemainingInputAttempt)
       {
         HBufC* queryPrompt = StringLoader::LoadLC(R_SECUI_REMAINING_PIN_ATTEMPTS, codeInfo.iRemainingEntryAttempts);
         res = dlg->ExecuteLD(R_PIN_QUERY, *queryPrompt);
         CleanupStack::PopAndDestroy(queryPrompt);
       }
    else
       {
         HBufC* queryPrompt = StringLoader::LoadLC(R_SECUI_FINAL_PIN_ATTEMPT);
         res = dlg->ExecuteLD(R_PIN_QUERY, *queryPrompt);
         CleanupStack::PopAndDestroy(queryPrompt);   
       }  
    
      if( !res )
        {
        CleanupStack::PopAndDestroy(2,verdlg);
        return;
        }      
    CleanupStack::Pop(); // newdlg
    // new pin code query
     if (!(newdlg->ExecuteLD(R_NEW_PIN_CODE_QUERY)))
        {
        CleanupStack::PopAndDestroy(verdlg);
        return;
        }

    CleanupStack::Pop(); // verdlg
    // verification code query
    if (!(verdlg->ExecuteLD(R_VERIFY_NEW_PIN_CODE_QUERY)))
            return;
        
    while (newPassword.CompareF(verifcationPassword) != 0) 
        {
        // codes do not match -> note -> ask new pin and verification codes again  
        ShowResultNoteL(R_CODES_DONT_MATCH, CAknNoteDialog::EErrorTone);
    
        newPassword = _L("");
        verifcationPassword = _L("");

        // new pin code query
        CCodeQueryDialog* newdlg = new (ELeave) CCodeQueryDialog (newPassword,SEC_C_PIN_CODE_MIN_LENGTH,SEC_C_PIN_CODE_MAX_LENGTH,ESecUiNone);
        if (!(newdlg->ExecuteLD(R_NEW_PIN_CODE_QUERY)))
              return;
        
        // verification code query
        CCodeQueryDialog* verdlg = new (ELeave) CCodeQueryDialog (verifcationPassword,SEC_C_PIN_CODE_MIN_LENGTH,SEC_C_PIN_CODE_MAX_LENGTH,ESecUiNone);
        if (!(verdlg->ExecuteLD(R_VERIFY_NEW_PIN_CODE_QUERY)))
            return;
        }            
        
    // send code
    passwords.iOldPassword = oldPassword;
    passwords.iNewPassword = newPassword;
    iWait->SetRequestType(EMobilePhoneChangeSecurityCode);
    iPhone.ChangeSecurityCode(iWait->iStatus, secCodeType, passwords);
    res = iWait->WaitForRequestL();
    #if defined(_DEBUG)
    RDebug::Print( _L("(SECUI)CSecuritySettings::ChangePinL(): RETURN CODE: %d"), res);
    #endif
    switch(res)
        {
        case KErrNone:
            {
            // code changed 
            ShowResultNoteL(R_PIN_CODE_CHANGED_NOTE, CAknNoteDialog::EConfirmationTone);
            break;
            }        
        case KErrGsm0707IncorrectPassword:
        case KErrAccessDenied:
            {    
            // code was entered erroneously
            ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
            ChangePinL();
            break;
            }    
        case KErrGsmSSPasswordAttemptsViolation:
        case KErrLocked:
            {
            // Pin1 blocked! 
            return;
            }
        case KErrGsm0707OperationNotAllowed:
            {
            // not allowed with this sim
            ShowResultNoteL(R_OPERATION_NOT_ALLOWED, CAknNoteDialog::EErrorTone);
            return;
            }
        case KErrAbort:
            {
            break;
            }
        default:
            {
            ShowErrorNoteL(res);
            ChangePinL();
            break;
            }
        }

    }

//
// ----------------------------------------------------------
// CSecuritySettings::ChangeUPinL()
// Changes Universal PIN
// ----------------------------------------------------------
//
EXPORT_C void CSecuritySettings::ChangeUPinL()
    {
    TBool wcdmaSupported(FeatureManager::FeatureSupported( KFeatureIdProtocolWcdma ));
    TBool upinSupported(FeatureManager::FeatureSupported( KFeatureIdUpin ));
    if(wcdmaSupported || upinSupported)
      {
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecuritySettings::ChangeUPinL()"));
        #endif
        TInt simState;
        TInt err( KErrGeneral );
        err = RProperty::Get(KPSUidStartup, KPSSimStatus, simState);
        User::LeaveIfError( err );
        TBool simRemoved(simState == ESimNotPresent);
    
        if ( simRemoved )
            {
            ShowResultNoteL(R_INSERT_SIM, CAknNoteDialog::EErrorTone);
            return;
            }
    
        RMobilePhone::TMobilePhoneSecurityCode secCodeType;
        secCodeType = RMobilePhone::ESecurityUniversalPin;
    
        RMobilePhone::TMobilePassword oldPassword;
        RMobilePhone::TMobilePassword newPassword;
        RMobilePhone::TMobilePassword verifcationPassword;
        RMobilePhone::TMobilePhonePasswordChangeV1 passwords;
        RMobilePhone::TMobilePhoneSecurityCodeInfoV5 codeInfo;
        RMobilePhone::TMobilePhoneSecurityCodeInfoV5Pckg codeInfoPkg(codeInfo);
    
        CCodeQueryDialog* verdlg = new (ELeave) CCodeQueryDialog (verifcationPassword,SEC_C_PIN_CODE_MIN_LENGTH,SEC_C_PIN_CODE_MAX_LENGTH,ESecUiNone);
        CleanupStack::PushL(verdlg);
    
        CCodeQueryDialog* newdlg = new (ELeave) CCodeQueryDialog(newPassword,SEC_C_PIN_CODE_MIN_LENGTH,SEC_C_PIN_CODE_MAX_LENGTH,ESecUiNone);
        CleanupStack::PushL(newdlg);
    
        CCodeQueryDialog* dlg = new (ELeave) CCodeQueryDialog (oldPassword,SEC_C_PIN_CODE_MIN_LENGTH,SEC_C_PIN_CODE_MAX_LENGTH,ESecUiNone);       
        CleanupStack::PushL(dlg);
    
        RMobilePhone::TMobilePhoneLock lockType;
        RMobilePhone::TMobilePhoneLockInfoV1 lockInfo;
        
        lockType = RMobilePhone::ELockUniversalPin;
        
        RMobilePhone::TMobilePhoneLockInfoV1Pckg lockInfoPkg(lockInfo);
        iWait->SetRequestType(EMobilePhoneGetLockInfo);
        iPhone.GetLockInfo(iWait->iStatus, lockType, lockInfoPkg);
        TInt res = iWait->WaitForRequestL();
        User::LeaveIfError(res);
    
        if (lockInfo.iSetting == RMobilePhone::ELockSetDisabled)
            {    
            CleanupStack::PopAndDestroy(3,verdlg);
            ShowResultNoteL(R_UPIN_NOT_ALLOWED, CAknNoteDialog::EErrorTone);
            return;
            }
        
        CleanupStack::Pop(); // dlg
        // ask pin
        iWait->SetRequestType(EMobilePhoneGetSecurityCodeInfo);
        iPhone.GetSecurityCodeInfo(iWait->iStatus, secCodeType, codeInfoPkg);
        res = iWait->WaitForRequestL();
        User::LeaveIfError(res);
            
        if( codeInfo.iRemainingEntryAttempts >= KMaxNumberOfPINAttempts )
            res = dlg->ExecuteLD(R_UPIN_QUERY);
        else if(codeInfo.iRemainingEntryAttempts > KLastRemainingInputAttempt)
            {
              HBufC* queryPrompt = StringLoader::LoadLC(R_SECUI_REMAINING_UPIN_ATTEMPTS, codeInfo.iRemainingEntryAttempts);
              res = dlg->ExecuteLD(R_UPIN_QUERY, *queryPrompt);
              CleanupStack::PopAndDestroy(queryPrompt);
            }
        else
            {
              HBufC* queryPrompt = StringLoader::LoadLC(R_SECUI_FINAL_UPIN_ATTEMPT);
              res = dlg->ExecuteLD(R_UPIN_QUERY, *queryPrompt);
              CleanupStack::PopAndDestroy(queryPrompt);   
            }        
        
        
        
         if( !res )
            {
            CleanupStack::PopAndDestroy(2,verdlg);
            return;
            }      
        CleanupStack::Pop(); // newdlg
        // new pin code query
         if (!(newdlg->ExecuteLD(R_NEW_UPIN_CODE_QUERY)))
            {
            CleanupStack::PopAndDestroy(verdlg);
            return;
            }
    
        CleanupStack::Pop(); // verdlg
        // verification code query
        if (!(verdlg->ExecuteLD(R_VERIFY_NEW_UPIN_CODE_QUERY)))
                return;
            
        while (newPassword.CompareF(verifcationPassword) != 0) 
            {
            // codes do not match -> note -> ask new pin and verification codes again  
            ShowResultNoteL(R_CODES_DONT_MATCH, CAknNoteDialog::EErrorTone);
        
            newPassword = _L("");
            verifcationPassword = _L("");
    
            // new pin code query
            CCodeQueryDialog* newdlg = new (ELeave) CCodeQueryDialog (newPassword,SEC_C_PIN_CODE_MIN_LENGTH,SEC_C_PIN_CODE_MAX_LENGTH,ESecUiNone);
            if (!(newdlg->ExecuteLD(R_NEW_UPIN_CODE_QUERY)))
                  return;
            
            // verification code query
            CCodeQueryDialog* verdlg = new (ELeave) CCodeQueryDialog (verifcationPassword,SEC_C_PIN_CODE_MIN_LENGTH,SEC_C_PIN_CODE_MAX_LENGTH,ESecUiNone);
            if (!(verdlg->ExecuteLD(R_VERIFY_NEW_UPIN_CODE_QUERY)))
                return;
            }            
            
        // send code
        passwords.iOldPassword = oldPassword;
        passwords.iNewPassword = newPassword;
        iWait->SetRequestType(EMobilePhoneChangeSecurityCode);
        iPhone.ChangeSecurityCode(iWait->iStatus, secCodeType, passwords);
        res = iWait->WaitForRequestL();
        #if defined(_DEBUG)
        RDebug::Print( _L("(SECUI)CSecuritySettings::ChangePinL(): RETURN CODE: %d"), res);
        #endif
        switch(res)
            {
            case KErrNone:
                {
                // code changed 
                ShowResultNoteL(R_UPIN_CODE_CHANGED_NOTE, CAknNoteDialog::EConfirmationTone);
                break;
                }        
            case KErrGsm0707IncorrectPassword:
            case KErrAccessDenied:
                {    
                // code was entered erroneously
                ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
                ChangeUPinL();
                break;
                }    
            case KErrGsmSSPasswordAttemptsViolation:
            case KErrLocked:
                {
                return;
                }
            case KErrGsm0707OperationNotAllowed:
                {
                // not allowed with this sim
                ShowResultNoteL(R_OPERATION_NOT_ALLOWED, CAknNoteDialog::EErrorTone);
                return;
                }
            case KErrAbort:
                {
                break;
                }
            default:
                {
                ShowErrorNoteL(res);
                ChangeUPinL();
                break;
                }
            }
      }

    }

//
// ----------------------------------------------------------
// CSecuritySettings::ChangePin2L()
// Changes PIN2
// ----------------------------------------------------------
//
EXPORT_C void CSecuritySettings::ChangePin2L()
    {
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecuritySettings::ChangePin2L()"));
    #endif
    TInt simState;
    TInt err( KErrGeneral );
    err = RProperty::Get(KPSUidStartup, KPSSimStatus, simState);
    User::LeaveIfError( err );
    TBool simRemoved(simState == ESimNotPresent);

    if ( simRemoved )
        {
        ShowResultNoteL(R_INSERT_SIM, CAknNoteDialog::EErrorTone);
        return;
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
    
    // check if pin2 is blocked...
    TBool isBlocked = EFalse;

    TInt ret = iCustomPhone.IsBlocked(secCodeType,isBlocked);
    
    if(isBlocked)
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
                ShowResultNoteL(R_PIN2_REJECTED, CAknNoteDialog::EConfirmationTone);
                break;
            case KErrGsm0707SimNotInserted:
                // not allowed with this sim
                ShowResultNoteL(R_OPERATION_NOT_ALLOWED, CAknNoteDialog::EErrorTone);
                break;
            default:
                ShowErrorNoteL(ret);
                break;
            }
        return;
        }
    
    CCodeQueryDialog* verdlg = new (ELeave) CCodeQueryDialog (verifcationPassword,SEC_C_PIN2_CODE_MIN_LENGTH,SEC_C_PIN2_CODE_MAX_LENGTH,ESecUiNone);
    CleanupStack::PushL(verdlg);

    CCodeQueryDialog* newdlg = new (ELeave) CCodeQueryDialog(newPassword,SEC_C_PIN2_CODE_MIN_LENGTH,SEC_C_PIN2_CODE_MAX_LENGTH,ESecUiNone);    
    CleanupStack::PushL(newdlg);

    CCodeQueryDialog* dlg = new (ELeave) CCodeQueryDialog (oldPassword,SEC_C_PIN2_CODE_MIN_LENGTH,SEC_C_PIN2_CODE_MAX_LENGTH,ESecUiNone);      
    CleanupStack::PushL(dlg);


    // Security code must be changed to Etel API format
    // Custom API Pin1 and Pin2 have the same enum values as the Etel ones
    EtelsecCodeType = (RMobilePhone::TMobilePhoneSecurityCode)secCodeType;
    #ifndef __WINS__    
        iWait->SetRequestType(EMobilePhoneGetSecurityCodeInfo);
        iPhone.GetSecurityCodeInfo(iWait->iStatus, EtelsecCodeType, codeInfoPkg);
        ret = iWait->WaitForRequestL();
        User::LeaveIfError(ret);
    #else
        codeInfo.iRemainingEntryAttempts = 1;
    #endif //__WINS__

    if(codeInfo.iRemainingEntryAttempts == KMaxNumberOfPINAttempts)
            ret = dlg->ExecuteLD(R_PIN2_QUERY);
    else if(codeInfo.iRemainingEntryAttempts > KLastRemainingInputAttempt)
       {
         HBufC* queryPrompt = StringLoader::LoadLC(R_SECUI_REMAINING_PIN2_ATTEMPTS, codeInfo.iRemainingEntryAttempts );
         ret = dlg->ExecuteLD(R_PIN2_QUERY, *queryPrompt);
         CleanupStack::PopAndDestroy(queryPrompt);
       }
    else
       {
         HBufC* queryPrompt = StringLoader::LoadLC(R_SECUI_FINAL_PIN2_ATTEMPT);
         ret = dlg->ExecuteLD(R_PIN2_QUERY, *queryPrompt);
         CleanupStack::PopAndDestroy(queryPrompt);   
       }

    CleanupStack::Pop(); // dlg
    if(!ret)
        {
        CleanupStack::PopAndDestroy(2,verdlg);
        return;
        }

    // new pin code query
    CleanupStack::Pop(); // newdlg
    if(!(newdlg->ExecuteLD(R_NEW_PIN2_CODE_QUERY)))
        {
        CleanupStack::PopAndDestroy(verdlg);
        return;
        }

     // verification code query
    CleanupStack::Pop(); // verdlg
    if(!(verdlg->ExecuteLD(R_VERIFY_NEW_PIN2_CODE_QUERY)))
        {
        return;
        }

    while (newPassword.CompareF(verifcationPassword) != 0)     
        {
        // codes do not match -> note -> ask new pin and verification codes again  
        ShowResultNoteL(R_CODES_DONT_MATCH, CAknNoteDialog::EErrorTone);

        newPassword = _L("");
        verifcationPassword = _L("");
        
        // new pin code query
        CCodeQueryDialog* dlg = new (ELeave) CCodeQueryDialog (newPassword,SEC_C_PIN2_CODE_MIN_LENGTH,SEC_C_PIN2_CODE_MAX_LENGTH,ESecUiNone);
        if(!(dlg->ExecuteLD(R_NEW_PIN2_CODE_QUERY)))
            return;
              
        // verification code query
        CCodeQueryDialog* dlg2 = new (ELeave) CCodeQueryDialog (verifcationPassword,SEC_C_PIN2_CODE_MIN_LENGTH,SEC_C_PIN2_CODE_MAX_LENGTH,ESecUiNone);
          if(!(dlg2->ExecuteLD(R_VERIFY_NEW_PIN2_CODE_QUERY)))
            return;
        }        
    

    passwords.iOldPassword = oldPassword;
    passwords.iNewPassword = newPassword;
    iWait->SetRequestType(EMobilePhoneChangeSecurityCode);
    iPhone.ChangeSecurityCode(iWait->iStatus,EtelsecCodeType,passwords);
    TInt res = iWait->WaitForRequestL();
        #if defined(_DEBUG)
    RDebug::Print( _L("(SECUI)CSecuritySettings::ChangePin2L(): RETURN CODE: %d"), res);
    #endif
    switch(res)
        {
        case KErrNone:
            {
            // code changed 
            ShowResultNoteL(R_PIN2_CODE_CHANGED_NOTE, CAknNoteDialog::EConfirmationTone);
            break;
            }        
        case KErrGsm0707IncorrectPassword:
        case KErrAccessDenied:
            {    
            ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
            ChangePin2L();
            break;
            }    
        case KErrGsmSSPasswordAttemptsViolation:
        case KErrLocked:
            {
            // Pin2 blocked!
            ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
            CSecurityHandler* handler = new(ELeave) CSecurityHandler(iPhone);
            CleanupStack::PushL(handler); 
            handler->HandleEventL(RMobilePhone::EPuk2Required);
            CleanupStack::PopAndDestroy(handler); // handler    
            return;
            }
        case KErrGsm0707OperationNotAllowed:
            {
            // not allowed with this sim
            ShowResultNoteL(R_OPERATION_NOT_ALLOWED, CAknNoteDialog::EErrorTone);
            return;
            }
        case KErrAbort:
            {
            break;
            }
        default:
            {
            ShowErrorNoteL(res);
            ChangePin2L();
            break;
            }
        }
     }
//
// ----------------------------------------------------------
// CSecuritySettings::ChangeSecCodeL()
// Changes security code 
// ----------------------------------------------------------
//
EXPORT_C void CSecuritySettings::ChangeSecCodeL()
    {  
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecuritySettings::ChangeSecCodeL()"));
    #endif
    TInt res;
    RMobilePhone::TMobilePassword newPassword;
if(FeatureManager::FeatureSupported(KFeatureIdSapTerminalControlFw ) &&
		(FeatureManager::FeatureSupported(KFeatureIdSapDeviceLockEnhancements  )))   
{ 
    
    // Connect to the SCP server, and request the code change
    RSCPClient scpClient;
    User::LeaveIfError( scpClient.Connect() );
    CleanupClosePushL( scpClient );
    res = scpClient.ChangeCodeRequest();
    CleanupStack::PopAndDestroy(); // scpClient
    
}
else
{
         
    RMobilePhone::TMobilePhoneSecurityCode secCodeType;
    secCodeType = RMobilePhone::ESecurityCodePhonePassword;
    RMobilePhone::TMobilePassword oldPassword;
    RMobilePhone::TMobilePassword verifcationPassword;
    RMobilePhone::TMobilePassword required_fourth;
    RMobilePhone::TMobilePhonePasswordChangeV1 passwords;

    CCodeQueryDialog* verdlg = new (ELeave) CCodeQueryDialog (verifcationPassword,SEC_C_SECURITY_CODE_MIN_LENGTH,SEC_C_SECURITY_CODE_CHANGE_MAX_LENGTH,ESecUiNone);
    CleanupStack::PushL(verdlg);

    CCodeQueryDialog* newdlg = new (ELeave) CCodeQueryDialog(newPassword,SEC_C_SECURITY_CODE_MIN_LENGTH,SEC_C_SECURITY_CODE_CHANGE_MAX_LENGTH,ESecUiNone);
    CleanupStack::PushL(newdlg);

    CCodeQueryDialog* dlg = new (ELeave) CCodeQueryDialog (oldPassword,SEC_C_SECURITY_CODE_MIN_LENGTH,SEC_C_SECURITY_CODE_MAX_LENGTH,ESecUiNone);
    CleanupStack::PushL(dlg);

    // ask security code
    CleanupStack::Pop(); // dlg
    if (!(dlg->ExecuteLD(R_SECURITY_QUERY)))
        {
        CleanupStack::PopAndDestroy(2,verdlg);
        return;
        }
    // new security code query
    CleanupStack::Pop(); // newdlg
    if(!(newdlg->ExecuteLD(R_NEW_SECURITY_CODE_QUERY)))
        {    
        CleanupStack::PopAndDestroy(verdlg);
        return;
        }
     
    // verification code query
    CleanupStack::Pop(); // verdlg
      if(!(verdlg->ExecuteLD(R_VERIFY_NEW_SECURITY_CODE_QUERY)))
        {
        return;
        }

    while (newPassword.CompareF(verifcationPassword) != 0)         
        {            
        // codes do not match -> note -> ask new pin and verification codes again  
        ShowResultNoteL(R_CODES_DONT_MATCH, CAknNoteDialog::EErrorTone);
        
        newPassword = _L("");
        verifcationPassword = _L("");

        // new pin code query
        CCodeQueryDialog* dlg = new (ELeave) CCodeQueryDialog (newPassword,SEC_C_SECURITY_CODE_MIN_LENGTH,SEC_C_SECURITY_CODE_CHANGE_MAX_LENGTH,ESecUiNone);
        if(!(dlg->ExecuteLD(R_NEW_SECURITY_CODE_QUERY)))
            return;
          
        // verification code query
        CCodeQueryDialog* dlg2 = new (ELeave) CCodeQueryDialog (verifcationPassword,SEC_C_SECURITY_CODE_MIN_LENGTH,SEC_C_SECURITY_CODE_CHANGE_MAX_LENGTH,ESecUiNone);
          if(!(dlg2->ExecuteLD(R_VERIFY_NEW_SECURITY_CODE_QUERY)))
            return;    
        }            
    iWait->SetRequestType(EMobilePhoneVerifySecurityCode);    
    // check code
    iPhone.VerifySecurityCode(iWait->iStatus,secCodeType, oldPassword, required_fourth);
    res = iWait->WaitForRequestL();
    #if defined(_DEBUG)
    RDebug::Print( _L("(SECUI)CSecuritySettings::ChangeSecCode(): CODE VERIFY RESP: %d"), res);
    #endif
    // change code 
    if (res == KErrNone)
        {
        passwords.iOldPassword = oldPassword;
        passwords.iNewPassword = newPassword;
        iWait->SetRequestType(EMobilePhoneChangeSecurityCode);
        iPhone.ChangeSecurityCode(iWait->iStatus,secCodeType,passwords);
        res = iWait->WaitForRequestL();
        }
        
}
        
    #if defined(_DEBUG)
    RDebug::Print( _L("(SECUI)CSecuritySettings::ChangeSecCode(): RETURN CODE: %d"), res);
    #endif
    switch(res)
        {
        case KErrNone:
            {
            // code changed 
            ShowResultNoteL(R_SECURITY_CODE_CHANGED_NOTE, CAknNoteDialog::EConfirmationTone);
            if(FeatureManager::FeatureSupported(KFeatureIdSapTerminalControlFw ) &&
								!(FeatureManager::FeatureSupported(KFeatureIdSapDeviceLockEnhancements  )))
						{
            // Send the changed code to the SCP server. Not used with device lock enhancements.
            
            RSCPClient scpClient;
            TSCPSecCode newCode;
            newCode.Copy( newPassword );
            if ( scpClient.Connect() == KErrNone )
                {
                scpClient.StoreCode( newCode );
                scpClient.Close();
                }                                               
          }
                        
            break;
            }
        case KErrGsmSSPasswordAttemptsViolation:
        case KErrLocked:
            {
            ShowResultNoteL(R_SEC_BLOCKED, CAknNoteDialog::EErrorTone);
            ChangeSecCodeL();
            break;
            }
        case KErrGsm0707IncorrectPassword:
        case KErrAccessDenied:
            {    
            // code was entered erroneously
            ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
            ChangeSecCodeL();
            break;
            }
        case KErrAbort:
            {
            break;
            }
        default:
            {
            ShowErrorNoteL(res);
            ChangeSecCodeL();
            break;
            }
        }
    }
//
// ----------------------------------------------------------
// CSecuritySettings::ChangeAutoLockPeriodL()
// Changes autolock period
// ----------------------------------------------------------
//
EXPORT_C TInt CSecuritySettings::ChangeAutoLockPeriodL(TInt aPeriod)
    {            
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecuritySettings::ChangeAutoLockPeriodLXXXX()"));
    #endif
    RMobilePhone::TMobilePhoneLockSetting lockChange(RMobilePhone::ELockSetDisabled);
    RMobilePhone::TMobilePhoneLock lockType = RMobilePhone::ELockPhoneDevice;
    TInt currentItem = 0;
    TInt oldPeriod = aPeriod;

    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecuritySettings::ChangeAutoLockPeriodL() ReadDesC16ArrayResourceL"));
    #endif


    CCoeEnv* coeEnv = CCoeEnv::Static();        
    CDesCArrayFlat* items =  coeEnv->ReadDesC16ArrayResourceL(R_AUTOLOCK_LBX);
    CleanupStack::PushL(items);
        
    if (aPeriod == 0)
        {
        currentItem = 0;  // autolock off
        }
    else
        {
        currentItem = 1;  // user defined
        }
    

    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecuritySettings::ChangeAutoLockPeriodL() New autolocksettingpage"));
    #endif
    
    CAutoLockSettingPage* dlg = new (ELeave)CAutoLockSettingPage(R_AUTOLOCK_SETTING_PAGE, currentItem, items, aPeriod);
    CleanupStack::PushL(dlg);
    dlg->ConstructL();
    TInt maxPeriod;
    if(FeatureManager::FeatureSupported(KFeatureIdSapTerminalControlFw ))
		{
    // Retrieve the current autolock period max. value from the SCP server, 
    // and check that the value the user
    // selected is ok from the Corporate Policy point of view.
	RSCPClient scpClient;
    TInt ret = scpClient.Connect();
    if ( ret == KErrNone )
        {       
        CleanupClosePushL( scpClient );
        TBuf<KSCPMaxIntLength> maxPeriodBuf;
        if ( scpClient.GetParamValue( ESCPMaxAutolockPeriod, maxPeriodBuf ) == KErrNone )
            {
            TLex lex( maxPeriodBuf );          
            if ( ( lex.Val( maxPeriod ) == KErrNone ) && ( maxPeriod > 0 ) )
                {               
                 dlg->SetPeriodMaximumValue(maxPeriod);
                }
            else
                {
                   maxPeriod = 0;
                   dlg->SetPeriodMaximumValue(maxPeriod);     
                }
                
            }
        else
            {
            #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CSecuritySettings::ChangeAutoLockPeriodL():\
                ERROR: Failed to retrieve max period"));
            #endif            
            }            
        }
    else
        {
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecuritySettings::ChangeAutoLockPeriodL():\
            ERROR: Failed to connect to SCP."));
        #endif            
        }
    CleanupStack::PopAndDestroy(); // scpClient 
}
    CleanupStack::Pop(); //dlg
    if (!dlg->ExecuteLD(CAknSettingPage::EUpdateWhenChanged))
        {
        CleanupStack::PopAndDestroy(items);           
        return oldPeriod;
        }
    
    CleanupStack::PopAndDestroy();    // items
    
    if ( FeatureManager::FeatureSupported( KFeatureIdSapTerminalControlFw ) )
        {
		// define a flag indicating whether disable autolock is allowed.
        TBool allowDisableAL = ETrue;
        
        if ( ( aPeriod == 0 ) && ( maxPeriod > 0 ) )
            {
            #if defined( _DEBUG )
                RDebug::Print(
                    _L("(SECUI)CSecuritySettings::ChangeAutoLockPeriodL() \
                    The period: %d is not allowed by TARM; max: %d" ),
                    aPeriod, maxPeriod );
            #endif                
            allowDisableAL = EFalse;
            HBufC* prompt = NULL;
            prompt = StringLoader::LoadLC(
                    R_SECUI_TEXT_AUTOLOCK_MUST_BE_ACTIVE );
            CAknNoteDialog* noteDlg = new ( ELeave ) CAknNoteDialog(
                    REINTERPRET_CAST( CEikDialog**,&noteDlg ) );
            noteDlg->PrepareLC( R_CODE_ERROR );
            noteDlg->SetTextL( *prompt );
            noteDlg->SetTimeout( CAknNoteDialog::ELongTimeout );
            noteDlg->SetTone( CAknNoteDialog::EErrorTone );
            noteDlg->RunLD();
            CleanupStack::PopAndDestroy( prompt );
            }
        
        if ( !allowDisableAL )
            {
            return ChangeAutoLockPeriodL( oldPeriod );
            }
        }

    if (aPeriod == 0)
        {
        
        #ifdef RD_REMOTELOCK

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
                #ifdef _DEBUG
                RDebug::Print( _L( "(SecUi)CSecuritySettings::ChangeAutoLockPeriodL() - RemoteLock is enabled: lockChange = RMobilePhone::ELockSetEnabled" ) );
                #endif // _DEBUG

                lockChange = RMobilePhone::ELockSetEnabled;
                }
            else
                {
                // Remote lock is disabled
                #ifdef _DEBUG
                RDebug::Print( _L( "(SecUi)CSecuritySettings::ChangeAutoLockPeriodL() - RemoteLock is disabled: lockChange = RMobilePhone::ELockSetDisabled" ) );
                #endif // _DEBUG

                lockChange = RMobilePhone::ELockSetDisabled;
                }
            }
        else
            {
            // Failed to get remote lock status
            #ifdef _DEBUG
            RDebug::Print( _L( "(SecUi)CSecuritySettings::ChangeAutoLockPeriodL() - Failed to get RemoteLock status" ) );
            #endif // _DEBUG
            }

        delete remoteLockSettings;
        remoteLockSettings = NULL;

        #else // not defined RD_REMOTELOCK

        lockChange = RMobilePhone::ELockSetDisabled;

        #endif // RD_REMOTELOCK
        }
    else
        {
        lockChange = RMobilePhone::ELockSetEnabled;
        }
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecuritySettings::ChangeAutoLockPeriodL() SetLockSetting"));
    #endif
        iWait->SetRequestType(EMobilePhoneSetLockSetting);
        RProperty::Set(KPSUidSecurityUIs, KSecurityUIsQueryRequestCancel, ESecurityUIsQueryRequestOk);
        iPhone.SetLockSetting(iWait->iStatus,lockType,lockChange);
        TInt status = iWait->WaitForRequestL();
        #if defined(_DEBUG)
        RDebug::Print( _L("(SECUI)CSecuritySettings::ChangeAutoLockPeriodL(): RETURN CODE: %d"), status);
        #endif
        switch(status)
        {
        case KErrNone:
            #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CSecuritySettings::ChangeAutoLockPeriodL() KErrNone"));
            #endif
            break;
        case KErrGsmSSPasswordAttemptsViolation:
        case KErrLocked:
            #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CSecuritySettings::ChangeAutoLockPeriodL() PasswordAttemptsViolation"));
            #endif
            return ChangeAutoLockPeriodL(oldPeriod);
        case KErrGsm0707IncorrectPassword:
        case KErrAccessDenied:
            #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CSecuritySettings::ChangeAutoLockPeriodL() IncorrectPassword"));
            #endif
            // code was entered erroneously
            return ChangeAutoLockPeriodL(oldPeriod);
        case KErrAbort:
            // User pressed "cancel" in the code query dialog.
            return oldPeriod;
        default:
            #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CSecuritySettings::ChangeAutoLockPeriodL() default"));
            #endif            
            return ChangeAutoLockPeriodL(oldPeriod);
        }
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecuritySettings::ChangeAutoLockPeriodL() END"));
    #endif
    return aPeriod; 
    }



//
// ----------------------------------------------------------
// CSecuritySettings::ChangeRemoteLockStatusL()
// Changes remote lock status (on/off)
// ----------------------------------------------------------
//
EXPORT_C TInt CSecuritySettings::ChangeRemoteLockStatusL( TBool& aRemoteLockStatus, TDes& aRemoteLockCode, TInt aAutoLockPeriod )
    {
   #ifdef RD_REMOTELOCK
    TInt retValue( KErrNone );

    #ifdef _DEBUG
    RDebug::Print(_L("(SecUi)CSecuritySettings::ChangeRemoteLockStatusL() - Enter, aRemoteLockStatus == %d, aAutoLockPeriod == %d" ), aRemoteLockStatus, aAutoLockPeriod );
    #endif // _DEBUG

    CCoeEnv* coeEnv       = CCoeEnv::Static();        
    CDesCArrayFlat* items = coeEnv->ReadDesC16ArrayResourceL( R_REMOTELOCK_LBX );
    CleanupStack::PushL( items );

    // Store the current remote lock setting 
    TInt previousItem( 0 );
    TInt currentItem(  0 );

    if ( aRemoteLockStatus )
        {
        previousItem = KRemoteLockSettingItemOn;
        currentItem  = KRemoteLockSettingItemOn;
        }
    else
        {
        previousItem = KRemoteLockSettingItemOff;
        currentItem  = KRemoteLockSettingItemOff;
        }

    // Create Remote Lock setting page for user to enable or disable remote locking 
    CRemoteLockSettingPage* remoteLockSettingPage = new( ELeave ) CRemoteLockSettingPage( R_REMOTELOCK_SETTING_PAGE, currentItem, items );

    #ifdef _DEBUG
    RDebug::Print( _L( "(SecUi)CSecuritySettings::ChangeRemoteLockStatusL() - Executing CRemoteLockSettingPage dialog" ) );
    #endif // _DEBUG

    // Execute the remote lock enable/disable dialog
    TBool ret = remoteLockSettingPage->ExecuteLD( CAknSettingPage::EUpdateWhenChanged );

    // Setting page list box items (texts) no longer needed
    CleanupStack::PopAndDestroy( items );

    if ( ret )
        {
        if ( currentItem == KRemoteLockSettingItemOn )
            {
            #ifdef _DEBUG
            RDebug::Print( _L( "(SecUi)CSecuritySettings::ChangeRemoteLockStatusL() - Remote lock status set to ON" ) );
            #endif // _DEBUG

            aRemoteLockStatus = ETrue;

            // If user wishes to enable remote lock
            // a new remote lock code is required.
            // RemoteLockCodeQueryL also 
            retValue = RemoteLockCodeQueryL( aRemoteLockCode );
            }
        else if ( currentItem == KRemoteLockSettingItemOff )
            {
            #ifdef _DEBUG
            RDebug::Print( _L( "(SecUi)CSecuritySettings::ChangeRemoteLockStatusL() - Remote lock status set to OFF" ) );
            #endif // _DEBUG

            aRemoteLockStatus = EFalse;
            retValue          = KErrNone;

            // Check whether the status was already off
            // If not don't make the user enter the security code
            // (which occurs when setting the DOS lock setting) for no reason.
            if ( currentItem != previousItem )
                {
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
            }
        else
            {
            // This should never happen. But if it does don't change anything
            retValue = KErrUnknown;
            }
        }
    else
        {
        // Something went wrong with the RemoteLockSettingPage dialog 
        retValue = KErrGeneral;

        #ifdef _DEBUG
        RDebug::Print( _L( "(SecUi)CSecuritySettings::ChangeRemoteLockStatusL() - CRemoteLockSettingPage::ExecuteLD() failed" ) );
        #endif // _DEBUG
        }

    #ifdef _DEBUG
    RDebug::Print(_L("(SecUi)CSecuritySettings::ChangeRemoteLockStatusL() - Exit" ) );
    #endif

    return retValue;
    #else //! RD_REMOTELOCK
    return KErrNotSupported;
    #endif //RD_REMOTELOCK
    }
//
// ----------------------------------------------------------
// CSecuritySettings::RemoteLockCodeQueryL()
// Pops up remote lock code query. Requires user to enter a new remote lock 
// code twice and if they match enables the domestic OS device lock (which as 
// a side effect pops up security code query).
// ----------------------------------------------------------
//
TInt CSecuritySettings::RemoteLockCodeQueryL( TDes& aRemoteLockCode )
    {
    #ifdef RD_REMOTELOCK
    TInt retValue( KErrNone );

    #ifdef _DEBUG
    RDebug::Print(_L("(SecUi)CSecuritySettings::ChangeRemoteLockCodeL() - Enter" ) );
    #endif // _DEBUG

    // Clear the remote lock code buffer
    aRemoteLockCode.Zero();

    // ----- Remote lock code query -------------------------------------------

    // Execute Remote Lock code query
    #ifdef _DEBUG
    RDebug::Print( _L( "(SecUi)CSecuritySettings::ChangeRemoteLockCodeL() - Executing remote lock code query" ) );
    #endif // _DEBUG

    // Load the query prompt from resources
    CCodeQueryDialog* codeQuery = new( ELeave ) CCodeQueryDialog( aRemoteLockCode, SEC_REMOTELOCK_CODE_MIN_LENGTH,SEC_REMOTELOCK_CODE_MAX_LENGTH, ESecUiNone, ETrue );
    TInt buttonId = codeQuery->ExecuteLD( R_REMOTELOCK_CODE_QUERY );
    if ( buttonId == EEikBidOk )
        {
            // Ok was pressed and the remote lock code seems fine
            retValue = KErrNone;
        }
    else
        {
        // User pressed Cancel
        // Set the code length to zero leaving no trash for possible retry
        aRemoteLockCode.Zero();
        retValue = KErrAbort;
        }

    if ( retValue != KErrNone )
        {
        #ifdef _DEBUG
        RDebug::Print(_L("(SecUi)CSecuritySettings::ChangeRemoteLockCodeL() - Exit, Remote lock code error" ) );
        #endif // _DEBUG

        // Can't continue beyond this 
        return retValue;
        }

    // ----- Remote lock code confirm query -----------------------------------

    // Confirm the code by asking it again
    #ifdef _DEBUG
    RDebug::Print( _L( "(SecUi)CSecuritySettings::ChangeRemoteLockCodeL() - Executing remote lock code verify query" ) );
    #endif // _DEBUG

    // Buffer for the confirmation code
    TBuf<KRLockMaxLockCodeLength> confirmCode;

    // Load the confirmation query prompt from resources
    CCodeQueryDialog* codeConfirmQuery = new( ELeave ) CCodeQueryDialog( confirmCode, SEC_REMOTELOCK_CODE_MIN_LENGTH, SEC_REMOTELOCK_CODE_MAX_LENGTH, ESecUiNone, ETrue );
    buttonId = codeConfirmQuery->ExecuteLD( R_VERIFY_REMOTELOCK_CODE_QUERY);


   if ( buttonId == EEikBidOk )
        {
        // Compare codes. Compare returns zero if descriptors have
        // the same length and the same content
        while ( (aRemoteLockCode.CompareF( confirmCode ) != 0) && (buttonId == EEikBidOk))
            {
                //Codes didn't match; zero the bufffers and show the dialog again
                aRemoteLockCode.Zero();
                confirmCode.Zero();
                // Codes don't match. Notify user
                ShowResultNoteL( R_REMOTELOCK_CODES_DONT_MATCH, CAknNoteDialog::EErrorTone );
                codeQuery = new( ELeave ) CCodeQueryDialog( aRemoteLockCode, SEC_REMOTELOCK_CODE_MIN_LENGTH,SEC_REMOTELOCK_CODE_MAX_LENGTH, ESecUiNone, ETrue );
                buttonId = codeQuery->ExecuteLD( R_REMOTELOCK_CODE_QUERY );
                //Unless user pressed Cancel show the verification query
                if(buttonId == EEikBidOk)
                    {
                        codeConfirmQuery = new( ELeave ) CCodeQueryDialog( confirmCode, SEC_REMOTELOCK_CODE_MIN_LENGTH, SEC_REMOTELOCK_CODE_MAX_LENGTH, ESecUiNone, ETrue );
                        buttonId = codeConfirmQuery->ExecuteLD( R_VERIFY_REMOTELOCK_CODE_QUERY);
                    }
                
            } 
        //User pressed cancel        
        if(buttonId != EEikBidOk)
            {
              // Set the code length to zero leaving no trash for possible retry
              // Report error and let the user try again 
              aRemoteLockCode.Zero();
              confirmCode.Zero();
              retValue = KErrAbort; 
            }
        else
            {
            // Codes match
            confirmCode.Zero();

            // ----- Check against security code ------------------------------
            
            // Check that the new remote lock code doesn't match the security 
            // code of the device.

            RMobilePhone::TMobilePhoneSecurityCode secCodeType;
            secCodeType = RMobilePhone::ESecurityCodePhonePassword;
            RMobilePhone::TMobilePassword securityCode;
            RMobilePhone::TMobilePassword unblockCode;  // Required here only as a dummy parameter 

            if ( aRemoteLockCode.Length() <= RMobilePhone::KMaxMobilePasswordSize )
                {
                securityCode = aRemoteLockCode;
                iWait->SetRequestType( EMobilePhoneVerifySecurityCode );
                iPhone.VerifySecurityCode( iWait->iStatus, secCodeType, securityCode, unblockCode );
                TInt res = iWait->WaitForRequestL();
                // The remote lock code matches the security code 
                // and that is not allowed
                while ( (res == KErrNone) && (buttonId == EEikBidOk))
                    {
                    #ifdef _DEBUG
                    RDebug::Print(_L("(SecUi)CSecuritySettings::ChangeRemoteLockCodeL() - Unacceptable remote lock code" ) );
                    #endif // _DEBUG
                    aRemoteLockCode.Zero();
                    confirmCode.Zero();
                    
					ShowResultNoteL( R_REMOTELOCK_INVALID_CODE, CAknNoteDialog::EErrorTone );
					
					codeQuery = new( ELeave ) CCodeQueryDialog( aRemoteLockCode, SEC_REMOTELOCK_CODE_MIN_LENGTH,SEC_REMOTELOCK_CODE_MAX_LENGTH, ESecUiNone, ETrue );
                    buttonId = codeQuery->ExecuteLD( R_REMOTELOCK_CODE_QUERY );
                    //Unless user pressed Cancel show the verification query
                    if(buttonId == EEikBidOk)
                        {
                            codeConfirmQuery = new( ELeave ) CCodeQueryDialog( confirmCode, SEC_REMOTELOCK_CODE_MIN_LENGTH, SEC_REMOTELOCK_CODE_MAX_LENGTH, ESecUiNone, ETrue );
                            buttonId = codeConfirmQuery->ExecuteLD( R_VERIFY_REMOTELOCK_CODE_QUERY);
                            
                            // Compare codes. Compare returns zero if descriptors have
                            // the same length and the same content
                            while ( (aRemoteLockCode.CompareF( confirmCode ) != 0) && (buttonId == EEikBidOk))
                                {
                                    //Codes didn't match; zero the bufffers and show the dialog again
                                    aRemoteLockCode.Zero();
                                    confirmCode.Zero();
                                    // Codes don't match. Notify user
                                    ShowResultNoteL( R_REMOTELOCK_CODES_DONT_MATCH, CAknNoteDialog::EErrorTone );
                                    codeQuery = new( ELeave ) CCodeQueryDialog( aRemoteLockCode, SEC_REMOTELOCK_CODE_MIN_LENGTH,SEC_REMOTELOCK_CODE_MAX_LENGTH, ESecUiNone, ETrue );
                                    buttonId = codeQuery->ExecuteLD( R_REMOTELOCK_CODE_QUERY );
                                    //Unless user pressed Cancel show the verification query
                                    if(buttonId == EEikBidOk)
                                        {
                                            codeConfirmQuery = new( ELeave ) CCodeQueryDialog( confirmCode, SEC_REMOTELOCK_CODE_MIN_LENGTH, SEC_REMOTELOCK_CODE_MAX_LENGTH, ESecUiNone, ETrue );
                                            buttonId = codeConfirmQuery->ExecuteLD( R_VERIFY_REMOTELOCK_CODE_QUERY);
                                        }
                
                                } 
                            //User pressed cancel        
                            if(buttonId != EEikBidOk)
                                {
                                  // Set the code length to zero leaving no trash for possible retry
                                  // Report error and let the user try again 
                                  aRemoteLockCode.Zero();
                                  confirmCode.Zero();
                                  retValue = KErrAbort; 
                                }
                            else //Check against security code
                                {
                                    securityCode = aRemoteLockCode;
                                    iWait->SetRequestType( EMobilePhoneVerifySecurityCode );
                                    iPhone.VerifySecurityCode( iWait->iStatus, secCodeType, securityCode, unblockCode );
                                    res = iWait->WaitForRequestL();
                                }
                        }
					
                    }
               //User pressed cancel        
               if(buttonId != EEikBidOk)
                    {
                      // Set the code length to zero leaving no trash for possible retry
                      // Report error and let the user try again 
                      aRemoteLockCode.Zero();
                      confirmCode.Zero();
                      retValue = KErrAbort; 
                    }
                }

            // ----- Enable DOS device lock (Security code query) -------------

            if ( retValue == KErrNone )
                {
                // Enable lock setting in domestic OS. It is safe to enable the 
                // lock setting since RemoteLock API requires remote locking to
                // be enabled when changing or setting the remote lock message.
                retValue = RemoteLockSetLockSettingL( ETrue );
                }
            }
        }
    else //User pressed Cancel
        {
        // Set the code length to zero leaving no trash for possible retry
        aRemoteLockCode.Zero();
        confirmCode.Zero();
        retValue = KErrAbort;
        }

    #ifdef _DEBUG
    RDebug::Print(_L("(SecUi)CSecuritySettings::ChangeRemoteLockCodeL() - Exit" ) );
    #endif // _DEBUG

    return retValue;
    #else //! RD_REMOTELOCK
    return KErrNotSupported;
    #endif //RD_REMOTELOCK
    }
//
// ----------------------------------------------------------
// CSecuritySettings::RemoteLockSetLockSettingL()
// Changes lock setting in domestic OS. Changing the domestic OS lock setting
// requires user to enter the security code.
// ----------------------------------------------------------
//
TInt CSecuritySettings::RemoteLockSetLockSettingL( TBool aLockSetting )
    {
    #ifdef RD_REMOTELOCK
    TInt retValue( KErrNone );

    #ifdef _DEBUG
    RDebug::Print(_L("(SecUi)CSecuritySettings::RemoteLockSetLockSettingL( %d ) - Enter" ), aLockSetting );
    #endif // _DEBUG

    RMobilePhone::TMobilePhoneLockSetting lockSetting = RMobilePhone::ELockSetEnabled;
    RMobilePhone::TMobilePhoneLock        lockType    = RMobilePhone::ELockPhoneDevice;

    if ( aLockSetting )
        {
        lockSetting = RMobilePhone::ELockSetEnabled;
        }
    else
        {
        lockSetting = RMobilePhone::ELockSetDisabled;
        }

    iWait->SetRequestType( EMobilePhoneSetLockSetting );
    RProperty::Set(KPSUidSecurityUIs, KSecurityUIsQueryRequestCancel, ESecurityUIsQueryRequestOk);
    iPhone.SetLockSetting( iWait->iStatus, lockType, lockSetting );

    // Wait for code verify to complete
    retValue = iWait->WaitForRequestL();

    switch( retValue )
        {
        case KErrNone:
            #ifdef _DEBUG
            RDebug::Print( _L( "(SecUi)CSecuritySettings::RemoteLockSetLockSettingL() - EMobilePhoneSetLockSetting request returned KErrNone" ) );
            #endif // _DEBUG
            break;

        case KErrGsmSSPasswordAttemptsViolation:
        case KErrLocked:
            #ifdef _DEBUG
            RDebug::Print( _L( "(SecUi)CSecuritySettings::RemoteLockSetLockSettingL() - EMobilePhoneSetLockSetting request returned KErrLocked" ) );
            #endif // _DEBUG
            //Error note is shown in CSecurityHandler::PassPhraseRequired()
            break;

        case KErrGsm0707IncorrectPassword:
        case KErrAccessDenied:
            #ifdef _DEBUG
            RDebug::Print( _L( "(SecUi)CSecuritySettings::RemoteLockSetLockSettingL() - EMobilePhoneSetLockSetting request returned KErrAccessDenied" ) );
            #endif // _DEBUG
            // Security code was entered erroneously
            //Error note is shown in CSecurityHandler::PassPhraseRequired()
            break;

        case KErrAbort:
            #ifdef _DEBUG
            RDebug::Print( _L( "(SecUi)CSecuritySettings::RemoteLockSetLockSettingL() - EMobilePhoneSetLockSetting request returned KErrAbort" ) );
            #endif // _DEBUG
            break;

        default:
            #ifdef _DEBUG
            RDebug::Print( _L( "(SecUi)CSecuritySettings::RemoteLockSetLockSettingL() - EMobilePhoneSetLockSetting request returned: %d"), retValue );
            #endif // _DEBUG
            break;
        }

    #ifdef _DEBUG
    RDebug::Print(_L("(SecUi)CSecuritySettings::RemoteLockSetLockSettingL() - Exit" ) );
    #endif // _DEBUG

    return retValue;
    #else //! RD_REMOTELOCK
    return KErrNotSupported;
    #endif //RD_REMOTELOCK
    }



//
// ----------------------------------------------------------
// CSecuritySettings::ChangeSimSecurityL()
// Changes SIM security
// ----------------------------------------------------------
//
EXPORT_C TBool CSecuritySettings::ChangeSimSecurityL()
    {    
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecuritySettings::ChangeSimSecurityL()"));
    #endif

    RMobilePhone::TMobilePhoneLockInfoV1 lockInfo;
    RMobilePhone::TMobilePhoneLockInfoV1Pckg lockInfoPkg(lockInfo);
    RMobilePhone::TMobilePhoneLock lockType = RMobilePhone::ELockPhoneToICC;
    RMobilePhone::TMobilePhoneLockSetting lockChangeSetting;
    CCoeEnv* coeEnv = CCoeEnv::Static();
    CDesCArrayFlat* items = coeEnv->ReadDesC16ArrayResourceL(R_SECURITY_LBX);
    CleanupStack::PushL(items);
                        
    //get lock info
    iWait->SetRequestType(EMobilePhoneGetLockInfo);
    iPhone.GetLockInfo(iWait->iStatus, lockType, lockInfoPkg);
    TInt status = iWait->WaitForRequestL();
    User::LeaveIfError(status);
    TInt currentItem = 0;

    if (lockInfo.iSetting == RMobilePhone::ELockSetDisabled)
        {
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecuritySettings::ChangeSimSecurityL()lockInfo: ELockSetDisabled"));
        #endif
        currentItem = 1;  // off
        }
                        
    TInt oldItem = currentItem;

    CAknRadioButtonSettingPage* dlg = new (ELeave)CAknRadioButtonSettingPage(R_SECURITY_SETTING_PAGE, currentItem, items);
    CleanupStack::PushL(dlg);
    

    CleanupStack::Pop(); // dlg
    if ( !(dlg->ExecuteLD(CAknSettingPage::EUpdateWhenChanged)) || oldItem==currentItem )
        {
        CleanupStack::PopAndDestroy();    // items
        return EFalse;
        }    



    if (currentItem == 1)
        {
        lockChangeSetting = RMobilePhone::ELockSetDisabled;
        }
    else
        {
        lockChangeSetting = RMobilePhone::ELockSetEnabled;
        }

    CleanupStack::PopAndDestroy();    // items 
    
    iWait->SetRequestType(EMobilePhoneSetLockSetting);
    RProperty::Set(KPSUidSecurityUIs, KSecurityUIsQueryRequestCancel, ESecurityUIsQueryRequestOk);
    iPhone.SetLockSetting(iWait->iStatus,lockType,lockChangeSetting);
    status = iWait->WaitForRequestL();

    #if defined(_DEBUG)
    RDebug::Print( _L("(SECUI)CSecuritySettings::ChangeSimSecurityL(): RETURN CODE: %d"), status);
    #endif        
    switch(status)
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
//
EXPORT_C TBool CSecuritySettings::ChangePinRequestL()
    {    
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecuritySettings::ChangePinRequestL()"));
    #endif      
    TInt simState;
    TInt err( KErrGeneral );
    err = RProperty::Get(KPSUidStartup, KPSSimStatus, simState);
    User::LeaveIfError( err );
    TBool simRemoved(simState == ESimNotPresent);

    if ( simRemoved )
        {
        ShowResultNoteL(R_INSERT_SIM, CAknNoteDialog::EErrorTone);
        return EFalse;;
        }

    RMobilePhone::TMobilePhoneLockInfoV1 lockInfo;
    RMobilePhone::TMobilePhoneLockInfoV1Pckg lockInfoPkg(lockInfo);
    RMobilePhone::TMobilePhoneLock lockType = RMobilePhone::ELockICC;

    RMobilePhone::TMobilePhoneLockSetting lockChangeSetting;
    
    CCoeEnv* coeEnv = CCoeEnv::Static();
    CDesCArrayFlat* items = coeEnv->ReadDesC16ArrayResourceL(R_PIN_LBX);
    CleanupStack::PushL(items);
                        
    //get lock info
    iWait->SetRequestType(EMobilePhoneGetLockInfo);
    iPhone.GetLockInfo(iWait->iStatus, lockType, lockInfoPkg);
    TInt status = iWait->WaitForRequestL();
    User::LeaveIfError(status);                    
    TInt currentItem = 0;

    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecuritySettings::ChangePinRequestL() GetLockInfo"));
    #endif

    if (lockInfo.iSetting == RMobilePhone::ELockSetDisabled)
        {
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecuritySettings::ChangePinRequestL() lockInfo: ELockSetDisabled"));
        #endif
        currentItem = 1;  // off
        }
                        
    TInt oldItem = currentItem;

    CAknRadioButtonSettingPage* dlg = new (ELeave)CAknRadioButtonSettingPage(R_PIN_SETTING_PAGE, currentItem, items);
    CleanupStack::PushL(dlg);
    

    CleanupStack::Pop(); // dlg
    if ( !(dlg->ExecuteLD(CAknSettingPage::EUpdateWhenChanged)) || oldItem==currentItem )
        {
        CleanupStack::PopAndDestroy();    // items
        return EFalse;
        }    


    if (currentItem == 1)
        {
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecuritySettings::ChangePinRequestL() currentItem: ELockSetDisabled"));
        #endif
        lockChangeSetting = RMobilePhone::ELockSetDisabled;
        }
    else
        {
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecuritySettings::ChangePinRequestL() currentItem: ELockSetEnabled"));
        #endif
        lockChangeSetting = RMobilePhone::ELockSetEnabled;
        }

    CleanupStack::PopAndDestroy();    // items 

    // Raise a flag to indicate that the PIN
    // request coming from ETEL has originated from SecUi and not from Engine.
    TInt tRet = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery, ESecurityUIsSecUIOriginated);
    if ( tRet != KErrNone )
        {
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecuritySettings::ChangePinRequestL():\
            FAILED to set the SECUI query Flag: %d"), tRet);
        #endif
        }
    // Change the lock setting
    iWait->SetRequestType(EMobilePhoneSetLockSetting);
    RProperty::Set(KPSUidSecurityUIs, KSecurityUIsQueryRequestCancel, ESecurityUIsQueryRequestOk);
    iPhone.SetLockSetting(iWait->iStatus,lockType,lockChangeSetting);
    status = iWait->WaitForRequestL();
    #if defined(_DEBUG)
    RDebug::Print( _L("(SECUI)CSecuritySettings::ChangePinRequestL(): RETURN CODE: %d"), status);
    #endif

    // Lower the flag                             
    RProperty::Set(KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery, ESecurityUIsETelAPIOriginated);

    switch(status)
        {
        case KErrNone:
            {
            break;
            }
        case KErrGsm0707OperationNotAllowed:
            {
            // not allowed with this sim
            ShowResultNoteL(R_OPERATION_NOT_ALLOWED, CAknNoteDialog::EErrorTone);
            return EFalse;
            }
        case KErrGsm0707IncorrectPassword:
        case KErrAccessDenied:
            {    
            // code was entered erroneously
            return ChangePinRequestL();
            }    
        case KErrGsmSSPasswordAttemptsViolation:
        case KErrLocked:
            {
            return ETrue;
            }
        case KErrAbort:
            {
            return EFalse;
            }
        default:
            {
            return ChangePinRequestL();
            }
        }
    
    return ETrue;
    }

//
// ----------------------------------------------------------
// CSecuritySettings::ChangeUPinRequestL()
// Changes UPIN request on/off
// ----------------------------------------------------------
//
EXPORT_C TBool CSecuritySettings::ChangeUPinRequestL()
    {
    TBool wcdmaSupported(FeatureManager::FeatureSupported( KFeatureIdProtocolWcdma ));
    TBool upinSupported(FeatureManager::FeatureSupported( KFeatureIdUpin ));
    if(wcdmaSupported || upinSupported)
      {
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecuritySettings::ChangeUPinRequestL()"));
        #endif
        
        TInt simState;
        TInt err( KErrGeneral );
        err = RProperty::Get(KPSUidStartup, KPSSimStatus, simState);
        User::LeaveIfError( err );
        TBool simRemoved(simState == ESimNotPresent);
    
        if ( simRemoved )
            {
            ShowResultNoteL(R_INSERT_SIM, CAknNoteDialog::EErrorTone);
            return EFalse;
            }
    
        RMobilePhone::TMobilePhoneLockInfoV1 lockInfo;
        RMobilePhone::TMobilePhoneLockInfoV1Pckg lockInfoPkg(lockInfo);
        RMobilePhone::TMobilePhoneLock lockType = RMobilePhone::ELockUniversalPin;
    
        RMobilePhone::TMobilePhoneLockSetting lockChangeSetting = RMobilePhone::ELockSetDisabled;
        
        CCoeEnv* coeEnv = CCoeEnv::Static();
        CDesCArrayFlat* items = coeEnv->ReadDesC16ArrayResourceL(R_UPIN_LBX);
        CleanupStack::PushL(items);
                            
        //get lock info
        iWait->SetRequestType(EMobilePhoneGetLockInfo);
        iPhone.GetLockInfo(iWait->iStatus, lockType, lockInfoPkg);
        TInt status = iWait->WaitForRequestL();
        User::LeaveIfError(status);                    
        TInt currentItem = 0;
    
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecuritySettings::ChangeUPinRequestL() GetLockInfo"));
        #endif
    
        if (lockInfo.iSetting == RMobilePhone::ELockSetDisabled)
            {
            #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CSecuritySettings::ChangeUPinRequestL() lockInfo: ELockSetDisabled"));
            #endif
            currentItem = 1;  // off
            }
                            
        TInt oldItem = currentItem;
    
        CAknRadioButtonSettingPage* dlg = new (ELeave)CAknRadioButtonSettingPage(R_UPIN_SETTING_PAGE, currentItem, items);
        CleanupStack::PushL(dlg);
        
    
        CleanupStack::Pop(); // dlg
        if ( !(dlg->ExecuteLD(CAknSettingPage::EUpdateWhenChanged)) || oldItem==currentItem )
            {
            CleanupStack::PopAndDestroy();    // items
            return EFalse;
            }    
    
    
       if (currentItem == 1)
            {
            #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CSecuritySettings::ChangePinRequestL() currentItem: ELockSetDisabled"));
            #endif
            lockChangeSetting = RMobilePhone::ELockSetDisabled;
            }
        else
            {
            #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CSecuritySettings::ChangePinRequestL() currentItem: ELockSetEnabled"));
            #endif
            lockChangeSetting = RMobilePhone::ELockSetEnabled;
            }
    
        CleanupStack::PopAndDestroy();    // items 
    
        // Raise a flag to indicate that the UPIN
        // request coming from ETEL has originated from SecUi and not from Engine.
        RProperty::Set(KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery, ESecurityUIsSecUIOriginated);                              
        // Change the lock setting
        iWait->SetRequestType(EMobilePhoneSetLockSetting);
        RProperty::Set(KPSUidSecurityUIs, KSecurityUIsQueryRequestCancel, ESecurityUIsQueryRequestOk);
        iPhone.SetLockSetting(iWait->iStatus,lockType,lockChangeSetting);
        status = iWait->WaitForRequestL();
        #if defined(_DEBUG)
        RDebug::Print( _L("(SECUI)CSecuritySettings::ChangeUPinRequestL(): RETURN CODE: %d"), status);
        #endif
    
        // Lower the flag                           
        RProperty::Set(KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery, ESecurityUIsETelAPIOriginated);
        
        switch(status)
            {
            case KErrNone:
                {
                break;
                }
            case KErrGsm0707OperationNotAllowed:
                {
                // not allowed with this sim
                ShowResultNoteL(R_OPERATION_NOT_ALLOWED, CAknNoteDialog::EErrorTone);
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
//
EXPORT_C TBool CSecuritySettings::SwitchPinCodesL()
    { 
    TBool wcdmaSupported(FeatureManager::FeatureSupported( KFeatureIdProtocolWcdma ));
    TBool upinSupported(FeatureManager::FeatureSupported( KFeatureIdUpin ));
    if(wcdmaSupported || upinSupported)
      {
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecuritySettings::SwitchPinCodesL()"));
        #endif 
    
        // If we are in simless offline mode the PIN codes can't obviously be switched
        TInt simState;
        TInt err( KErrGeneral );
        err = RProperty::Get(KPSUidStartup, KPSSimStatus, simState);    
        User::LeaveIfError( err );
        TBool simRemoved(simState == ESimNotPresent);
    
        if ( simRemoved )
            {
            ShowResultNoteL(R_INSERT_SIM, CAknNoteDialog::EErrorTone);
            return EFalse;
            }
    
       
        RMobilePhone::TMobilePhoneLock lockType = RMobilePhone::ELockUniversalPin;
        RMobilePhone::TMobilePhoneLockSetting lockChangeSetting = RMobilePhone::ELockReplaced;                       
        RMobilePhone::TMobilePhoneSecurityCode activeCode;
     
        iCustomPhone.GetActivePin(activeCode);
    
        RMobilePhone::TMobilePhoneLockInfoV1 lockInfo;    
        RMobilePhone::TMobilePhoneLockInfoV1Pckg lockInfoPkg(lockInfo);
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecuritySettings::SwitchPinCodesL() GetLockInfo"));
        #endif    
        iWait->SetRequestType(EMobilePhoneGetLockInfo);
    
        if (activeCode == RMobilePhone::ESecurityUniversalPin)
            {
             lockType = RMobilePhone::ELockUniversalPin;
             iPhone.GetLockInfo(iWait->iStatus, lockType, lockInfoPkg);
             TInt res = iWait->WaitForRequestL();
             User::LeaveIfError(res);
            #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CSecuritySettings::SwitchPinCodesL() Lock Info got: UPIN"));
            #endif 
            }
        else
            {
             lockType = RMobilePhone::ELockICC;
             iPhone.GetLockInfo(iWait->iStatus, lockType, lockInfoPkg);
             TInt res = iWait->WaitForRequestL();
             User::LeaveIfError(res);
            #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CSecuritySettings::SwitchPinCodesL() Lock Info got: PIN"));
            #endif 
            }
    
        // code request must be ON to change active code.
        if (lockInfo.iSetting == RMobilePhone::ELockSetDisabled)
           {
            #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CSecuritySettings::SwitchPinCodesL() CODE REQ NOT ON."));
            #endif
            if (activeCode == RMobilePhone::ESecurityUniversalPin)
                {
                ShowResultNoteL(R_UPIN_NOT_ALLOWED, CAknNoteDialog::EErrorTone);
                }
            else
                {
                ShowResultNoteL(R_PIN_NOT_ALLOWED, CAknNoteDialog::EErrorTone);
                }
            #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CSecuritySettings::SwitchPinCodesL() CODE REQ NOT ON NOTE END."));
            #endif 
            return EFalse;
            }
    
        
        
        CCoeEnv* coeEnv = CCoeEnv::Static();
        CDesCArrayFlat* items = coeEnv->ReadDesC16ArrayResourceL(R_CODE_LBX);
        CleanupStack::PushL(items);
    
        iCustomPhone.GetActivePin(activeCode);
        TInt currentItem = 0;
    
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecuritySettings::SwitchPinCodesL() GetLockInfo"));
        #endif
    
        if (activeCode == RMobilePhone::ESecurityUniversalPin)
            {
            #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CSecuritySettings::SwitchPinCodesL() active code: UPIN"));
            #endif
            currentItem = 1;  // UPIN
            }
                            
        TInt oldItem = currentItem;
    
        CAknRadioButtonSettingPage* dlg = new (ELeave)CAknRadioButtonSettingPage(R_CODE_IN_USE_SETTING_PAGE, currentItem, items);
        CleanupStack::PushL(dlg);
        
    
        CleanupStack::Pop(); // dlg
        if ( !(dlg->ExecuteLD(CAknSettingPage::EUpdateWhenChanged)) || oldItem==currentItem )
            {
            CleanupStack::PopAndDestroy();    // items
            return EFalse;
            }    
    
    
       if (currentItem == 1)
            {
            #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CSecuritySettings::SwitchPinCodesL() currentItem: UPIN"));
            #endif
            lockType = RMobilePhone::ELockUniversalPin;
            }
        else
            {
            #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CSecuritySettings::SwitchPinCodesL() currentItem: PIN1"));
            #endif
            lockType = RMobilePhone::ELockICC;
            }
    
        CleanupStack::PopAndDestroy();    // items 
    
        // Raise a flag to indicate that the code
        // request coming from ETEL has originated from SecUi and not from Engine.
        RProperty::Set(KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery, ESecurityUIsSecUIOriginated);                           
        // Change the lock setting
        iWait->SetRequestType(EMobilePhoneSetLockSetting);
        RProperty::Set(KPSUidSecurityUIs, KSecurityUIsQueryRequestCancel, ESecurityUIsQueryRequestOk);
        iPhone.SetLockSetting(iWait->iStatus,lockType,lockChangeSetting);
        TInt status = iWait->WaitForRequestL();
        #if defined(_DEBUG)
        RDebug::Print( _L("(SECUI)CSecuritySettings::SwitchPinCodesL(): RETURN CODE: %d"), status);
        #endif
        // Lower the flag                            
        RProperty::Set(KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery, ESecurityUIsETelAPIOriginated);
        
        switch(status)
            {
            case KErrNone:
                {
                break;
                }
            case KErrGsm0707OperationNotAllowed:
                {
                // not allowed with this sim
                ShowResultNoteL(R_OPERATION_NOT_ALLOWED, CAknNoteDialog::EErrorTone);
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
//
EXPORT_C TBool CSecuritySettings::IsLockEnabledL(RMobilePhone::TMobilePhoneLock aLockType)
    {
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecuritySettings::IsLockEnabledL()"));
    #endif
    #ifdef __WINS__

    return EFalse;

    #else  //WINS

    RMobilePhone::TMobilePhoneLockInfoV1 lockInfo;
    
    //get lock info
    RMobilePhone::TMobilePhoneLockInfoV1Pckg lockInfoPkg(lockInfo);
    iWait->SetRequestType(EMobilePhoneGetLockInfo);
    iPhone.GetLockInfo(iWait->iStatus, aLockType, lockInfoPkg);
    TInt res = iWait->WaitForRequestL();

    if (res != KErrNone)
        return ETrue;

     //lock is enabled return true
    if (lockInfo.iSetting == RMobilePhone::ELockSetEnabled)
        {
        return ETrue;                        
        }

    // lock is disabled return false
    return EFalse;
        
    #endif 

    }
//
// ----------------------------------------------------------
// CSecuritySettings::AskSecCodeL()
// For asking security code e.g in settings
// ----------------------------------------------------------
//
EXPORT_C TBool CSecuritySettings::AskSecCodeL()
    {
    return iSecurityHandler->AskSecCodeL();
    }
//
// ----------------------------------------------------------
// CSecuritySettings::AskPin2L()
// Asks PIN2
// ----------------------------------------------------------
//    
EXPORT_C TBool CSecuritySettings::AskPin2L()
    {    
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecuritySettings::AskPin2L()"));
    #endif
    TInt ret = 0;
    // check if pin2 is blocked...
    RMmCustomAPI::TSecurityCodeType secCodeType = RMmCustomAPI::ESecurityCodePin2;
    RMobilePhone::TMobilePhoneSecurityCode etelsecCodeType(RMobilePhone::ESecurityCodePin2);
    RMobilePhone::TMobilePhoneSecurityCodeInfoV5 codeInfo;
    RMobilePhone::TMobilePhoneSecurityCodeInfoV5Pckg codeInfoPkg(codeInfo);
    TBool isBlocked = EFalse;
    //Check whether PIN2 is blocked
    ret = iCustomPhone.IsBlocked(secCodeType,isBlocked);
    
	#if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecuritySettings::AskPin2L() IsBlocked return value: %d"), ret);
    #endif
    if(isBlocked)
        return EFalse;
    
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
                ShowResultNoteL(R_PIN2_REJECTED, CAknNoteDialog::EConfirmationTone);
                break;
            case KErrGsm0707SimNotInserted:
                // not allowed with this sim
                ShowResultNoteL(R_OPERATION_NOT_ALLOWED, CAknNoteDialog::EErrorTone);
                break;
            default:
                ShowErrorNoteL(ret);
                break;
            }
        return EFalse;
        }
    iWait->SetRequestType(EMobilePhoneGetSecurityCodeInfo);
    iPhone.GetSecurityCodeInfo(iWait->iStatus, etelsecCodeType, codeInfoPkg);
    ret = iWait->WaitForRequestL();
        
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecurityHandler::Pin2RequiredL(): get PIN2 info result: %d"), ret);
    TInt attempts(codeInfo.iRemainingEntryAttempts);
    RDebug::Print(_L("(SECUI)CSecurityHandler::Pin2RequiredL(): attempts remaining: %d"), attempts);
    #endif
    User::LeaveIfError(ret);
    
    // ask pin2 code  
    RMobilePhone::TMobilePassword password;
    CCodeQueryDialog* dlg = new (ELeave) CCodeQueryDialog (password,SEC_C_PIN_CODE_MIN_LENGTH,SEC_C_PIN_CODE_MAX_LENGTH,ESecUiNone);  
    if(codeInfo.iRemainingEntryAttempts == KMaxNumberOfPINAttempts)
            ret = dlg->ExecuteLD(R_PIN2_QUERY);
    else if(codeInfo.iRemainingEntryAttempts > KLastRemainingInputAttempt)
            {
                HBufC* queryPrompt = StringLoader::LoadLC(R_SECUI_REMAINING_PIN2_ATTEMPTS, codeInfo.iRemainingEntryAttempts );
                ret = dlg->ExecuteLD(R_PIN2_QUERY, *queryPrompt);
                CleanupStack::PopAndDestroy(queryPrompt);
            }
    else
            {
                HBufC* queryPrompt = StringLoader::LoadLC(R_SECUI_FINAL_PIN2_ATTEMPT);
                ret = dlg->ExecuteLD(R_PIN2_QUERY, *queryPrompt);
                CleanupStack::PopAndDestroy(queryPrompt);   
            } 
       
    if (!ret)
        {
		#if defined(_DEBUG)
		RDebug::Print(_L("(SECUI)CSecuritySettings::AskPin2L(): Cancel pressed"));
		#endif
        return EFalse;
        }
    
    // verify code
    RMobilePhone::TMobilePassword required_fourth;
    iWait->SetRequestType(EMobilePhoneVerifySecurityCode);
    iPhone.VerifySecurityCode(iWait->iStatus,etelsecCodeType, password, required_fourth);
    TInt res = iWait->WaitForRequestL();

	#if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecuritySettings::AskPin2L() VerifySecurityCode return value: %d"), res);
    #endif

    switch(res)
        {        
        case KErrNone:
            break;
        case KErrGsm0707IncorrectPassword:
        case KErrAccessDenied:
            // code was entered erroneously
            ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
            return    AskPin2L();    
        case KErrGsm0707OperationNotAllowed:
            // not allowed with this sim
            ShowResultNoteL(R_OPERATION_NOT_ALLOWED, CAknNoteDialog::EErrorTone);
            return EFalse;
        case KErrGsmSSPasswordAttemptsViolation:
        case KErrLocked:
            // code was blocked
            ShowResultNoteL(R_CODE_ERROR, CAknNoteDialog::EErrorTone);
            return EFalse;        
        default:
            ShowErrorNoteL(res);
            return    AskPin2L();
        }

    return ETrue;
    }
//
// ----------------------------------------------------------
// CSecuritySettings::SetFdnModeL()
// Activates or deactivates Fixed Dialling Numbers (FDN) mode.  
// ----------------------------------------------------------
//
EXPORT_C void CSecuritySettings::SetFdnModeL()
    {    
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecuritySettings::SetFdnModeL()"));
    #endif
    RMmCustomAPI::TSecurityCodeType secCodeType = RMmCustomAPI::ESecurityCodePin2;
    
    TBool isBlocked = EFalse;
    TInt ret = iCustomPhone.IsBlocked(secCodeType,isBlocked);
    
    if(isBlocked)
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
                ShowResultNoteL(R_PIN2_REJECTED, CAknNoteDialog::EConfirmationTone);
                break;
            case KErrGsm0707SimNotInserted:
                // not allowed with this sim
                ShowResultNoteL(R_OPERATION_NOT_ALLOWED, CAknNoteDialog::EErrorTone);    
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
        
      // Change the FDN setting
    iWait->SetRequestType(EMobilePhoneSetFdnSetting);
    RProperty::Set(KPSUidSecurityUIs, KSecurityUIsQueryRequestCancel, ESecurityUIsQueryRequestOk);
    iPhone.SetFdnSetting(iWait->iStatus, fdnSet);
    status = iWait->WaitForRequestL();

    #if defined(_DEBUG)
    RDebug::Print( _L("(SECUI)CSecuritySettings::SetFdnModeL(): RETURN CODE: %d"), status);
    #endif
    switch(status)
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
            ShowResultNoteL(R_OPERATION_NOT_ALLOWED, CAknNoteDialog::EErrorTone);
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
//
EXPORT_C TInt CSecuritySettings::GetFdnMode (RMobilePhone::TMobilePhoneFdnStatus& aFdnMode)
    {
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecuritySettings::GetFdnMode()"));
    #endif
    return iPhone.GetFdnStatus(aFdnMode);
    }

//
// ----------------------------------------------------------
// CSecuritySettings::ShowErrorNoteL()
// Shows error note
// ----------------------------------------------------------
//
void CSecuritySettings::ShowErrorNoteL(TInt aError)
    {
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecuritySettings::ShowErrorNoteL()"));
    #endif
    // Let's create TextResolver instance for error resolving...
    CTextResolver* textresolver = CTextResolver::NewLC(); 
    // Resolve the error
    TPtrC errorstring;
    errorstring.Set( textresolver->ResolveErrorString( aError ) );
    CAknNoteDialog* noteDlg = new (ELeave) CAknNoteDialog(REINTERPRET_CAST(CEikDialog**,&noteDlg));
    noteDlg->PrepareLC(R_CODE_ERROR);
    noteDlg->SetTextL((TDesC&)errorstring);
    noteDlg->SetTimeout(CAknNoteDialog::ELongTimeout);
    noteDlg->SetTone(CAknNoteDialog::EErrorTone);
    noteDlg->RunLD();
    CleanupStack::PopAndDestroy(); // resolver    
    }

//
// ----------------------------------------------------------
// CSecuritySettings::ShowResultNoteL()
// Shows result note
// ----------------------------------------------------------
//
void CSecuritySettings::ShowResultNoteL(TInt aResourceID, CAknNoteDialog::TTone aTone)
    {  
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CSecuritySettings::ShowResultNoteL()"));
    RDebug::Print(_L("(SECUI)CSecuritySettings::ShowResultNoteL() Resource ID: %d"), aResourceID);
    #endif
    CAknNoteDialog* noteDlg = new (ELeave) CAknNoteDialog(REINTERPRET_CAST(CEikDialog**,&noteDlg));
    noteDlg->SetTimeout(CAknNoteDialog::ELongTimeout);
    noteDlg->SetTone(aTone);
    noteDlg->ExecuteLD(aResourceID);
    }

//
// ----------------------------------------------------------
// CSecuritySettings::IsUpinSupportedL()
// Return is UPIN supported
// ----------------------------------------------------------
//
EXPORT_C TBool CSecuritySettings::IsUpinSupportedL()
{
    TBool wcdmaSupported(FeatureManager::FeatureSupported( KFeatureIdProtocolWcdma ));
    TBool upinSupported(FeatureManager::FeatureSupported( KFeatureIdUpin ));
    if(wcdmaSupported || upinSupported)
      {
    	#if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecuritySettings::IsUpinSupported() BEGIN"));
        #endif
        #ifdef __WINS__
    
        return EFalse;
    
        #else  //WINS
    
        RMobilePhone::TMobilePhoneLockInfoV1 lockInfo;
        
        //get lock info
        RMobilePhone::TMobilePhoneLockInfoV1Pckg lockInfoPkg(lockInfo);
        iWait->SetRequestType(EMobilePhoneGetLockInfo);
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecuritySettings::IsUpinSupported() GetLockInfo"));
        #endif
        iPhone.GetLockInfo(iWait->iStatus, RMobilePhone::ELockUniversalPin, lockInfoPkg);
        TInt res = iWait->WaitForRequestL();
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecuritySettings::IsUpinSupported() GetLockInfo DONE"));
        #endif
        if ((res == KErrNotSupported) || (res == KErrGsmInvalidParameter))
        {
            #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CSecuritySettings::IsUpinSupported(): NOT SUPPORTED"));
            #endif
            return EFalse;
        }
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecuritySettings::IsUpinSupported(): SUPPORTED: %d"), res);
        #endif
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecuritySettings::IsUpinSupported() END"));
        #endif
        return ETrue;
            
        #endif //WINS
      }
    else
        return EFalse;
}
//
// ----------------------------------------------------------
// CSecuritySettings::IsCodeBlocked()
// Return is a code blocked
// ----------------------------------------------------------
//
EXPORT_C TBool CSecuritySettings::IsUpinBlocked()
{
TBool wcdmaSupported(FeatureManager::FeatureSupported( KFeatureIdProtocolWcdma ));
    TBool upinSupported(FeatureManager::FeatureSupported( KFeatureIdUpin ));
    if(wcdmaSupported || upinSupported)
      {
    #ifdef __WINS__
        return EFalse;
    #else//__WINS__
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecuritySettings::IsUpinBlocked() BEGIN"));
        #endif
    	RMmCustomAPI::TSecurityCodeType secCodeType;
        secCodeType = RMmCustomAPI::ESecurityUniversalPin;
        TBool isBlocked = EFalse;
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecuritySettings::IsUpinBlocked() IsBlocked"));
        #endif
        TInt ret = iCustomPhone.IsBlocked(secCodeType,isBlocked);
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecuritySettings::IsUpinBlocked() DONE.RETURN: %d"), ret);
        #endif
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecuritySettings::IsUpinBlocked():isblocked: %d"), isBlocked);
        #endif
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecuritySettings::IsUpinBlocked() END"));
        #endif
        return isBlocked;
    #endif //__WINS__
      }
    else
        return EFalse;
}
//
// ----------------------------------------------------------
// CSecuritySettings::GetActivePinCode()
// Return the code active in current application (PIN/UPIN)
// ----------------------------------------------------------
//
EXPORT_C TBool CSecuritySettings::IsUpinActive()
{
    TBool wcdmaSupported(FeatureManager::FeatureSupported( KFeatureIdProtocolWcdma ));
    TBool upinSupported(FeatureManager::FeatureSupported( KFeatureIdUpin ));
    if(wcdmaSupported || upinSupported)
      {
        #ifdef __WINS__
        return EFalse;
        #else//__WINS__
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecuritySettings::IsUpinActive() BEGIN"));
        #endif
    	RMobilePhone::TMobilePhoneSecurityCode activePin;
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecuritySettings::IsUpinActive() GetActivePin"));
        #endif
        iCustomPhone.GetActivePin(activePin);
        #if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CSecuritySettings::IsUpinActive() GetActivePin DONE"));
        #endif
    	if(activePin == RMobilePhone::ESecurityUniversalPin)
        {
            #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CSecuritySettings::IsUpinActive(): UPIN ACTIVE"));
            #endif
    		return ETrue;
        }
         #if defined(_DEBUG)
         RDebug::Print(_L("(SECUI)CSecuritySettings::IsUpinActive(): UPIN NOT ACTIVE"));
         #endif
         #if defined(_DEBUG)
         RDebug::Print(_L("(SECUI)CSecuritySettings::IsUpinActive(): END"));
         #endif
    	return EFalse;
        #endif //__WINS__
      }
    else
        return EFalse;
}

// End of file
