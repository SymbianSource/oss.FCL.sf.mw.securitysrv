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
* Description:  Provides api for changing security settings via PhoneApp
*     
*
*/


#include <rmmcustomapi.h>
#include "SecQueryUi.h"
#include "secuimanualsecuritysettings.h"
#include "secuisecuritysettings.h"
#include <exterror.h>
#include "SecUi.h"
#include "SecUiWait.h"
#include <mmtsy_names.h>
    /*****************************************************
    *    Series 60 Customer / TSY
    *    Needs customer TSY implementation
    *****************************************************/
//  LOCAL CONSTANTS AND MACROS  



_LIT(Operator,"1");
_LIT(Gid1,"2");
_LIT(Gid2,"3");
_LIT(Imsi,"4");
_LIT(Operator_Gid1,"5");
_LIT(Operator_Gid2,"6");
_LIT(GlobalUnlock, "7");

const TInt KTriesToConnectServer( 2 );
const TInt KTimeBeforeRetryingServerConnection( 50000 );
const TInt PhoneIndex( 0 );

// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CManualSecuritySettings::NewL()
// Constructs a new entry with given values.
// ----------------------------------------------------------
//
EXPORT_C CManualSecuritySettings* CManualSecuritySettings::NewL()
    {
    CManualSecuritySettings* self = new (ELeave) CManualSecuritySettings();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }
//
// ----------------------------------------------------------
// CManualSecuritySettings::CManualSecuritySettings()
// constructor
// ----------------------------------------------------------
//
CManualSecuritySettings::CManualSecuritySettings()
    {
    }

//
// ----------------------------------------------------------
// CManualSecuritySettings::ConstructL()
// Symbian OS constructor.
// ----------------------------------------------------------
//
void CManualSecuritySettings::ConstructL()
    {
    
    iWait = CWait::NewL();
    
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
    RTelServer::TPhoneInfo PhoneInfo;
    /* All server connections are tried to be made KTriesToConnectServer times because occasional
    fails on connections are possible, at least on some servers */

    // connect ETel server
    while ( ( err = iServer.Connect() ) != KErrNone && ( thisTry++ ) <= KTriesToConnectServer )
        {
        User::After( KTimeBeforeRetryingServerConnection );
        }
    User::LeaveIfError( err );

    // load TSY
    err = iServer.LoadPhoneModule( KMmTsyModuleName );
    if ( err != KErrAlreadyExists )
        {
        // May also return KErrAlreadyExists if something else
        // has already loaded the TSY module. And that is
        // not an error.
        RDEBUG("err", err);
        User::LeaveIfError( err );
        }

    // open phones
    User::LeaveIfError(iServer.SetExtendedErrorGranularity(RTelServer::EErrorExtended));
    User::LeaveIfError(iServer.GetPhoneInfo(PhoneIndex, PhoneInfo));
    User::LeaveIfError(iPhone.Open(iServer,PhoneInfo.iName));
    User::LeaveIfError(iCustomPhone.Open(iPhone));
    }
//
// ----------------------------------------------------------
// CManualSecuritySettings::~CManualSecuritySettings()
// Destructor
// ----------------------------------------------------------
//
EXPORT_C CManualSecuritySettings::~CManualSecuritySettings()
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
    // Cancel active requests
    if(iWait->IsActive())
    {
      RDEBUG("CancelAsyncRequest", 0);
        iPhone.CancelAsyncRequest(iWait->GetRequestType());
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
    delete iNote;
    delete iWait;
	RDEBUG("0", 0);
    }

//
// ----------------------------------------------------------
// CManualSecuritySettings::ChangePinL()
// Changes pin
// ----------------------------------------------------------
//
EXPORT_C TBool CManualSecuritySettings::ChangePinL(TPin aPin,const TDesC& aOld,const TDesC& aNew,const TDesC& aVerifyNew )
    {    
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
    RDEBUG("aPin", aPin);
    if (aPin == EPin1)
        {
				RDEBUG("EPin1", EPin1);
        RMobilePhone::TMobilePhoneLock lockType;
        RMobilePhone::TMobilePhoneLockInfoV1 lockInfo;
        RMobilePhone::TMobilePhoneLockInfoV1Pckg lockInfoPkg(lockInfo);


        lockType = RMobilePhone::ELockICC;
        // check if pin code is enabled...
        // The following is required, since the new multimode Etel is asynchronous
        // rather than synchronous.
    
				RDEBUG("SetRequestType", EMobilePhoneGetLockInfo);
        iWait->SetRequestType(EMobilePhoneGetLockInfo);
        iPhone.GetLockInfo(iWait->iStatus, lockType, lockInfoPkg);
        RDEBUG("WaitForRequestL", 0);
        TInt res = iWait->WaitForRequestL();
        RDEBUG("WaitForRequestL res", res);
         
        if (res != KErrNone)
            return EFalse;
        // if pin is disabled -> pin code change is not allowed
        if (lockInfo.iSetting == RMobilePhone::ELockSetDisabled)
            {    
            ShowResultNoteL(CAknNoteDialog::EErrorTone,R_PIN_NOT_ALLOWED);
            return EFalse;
            }
        }
    
		RDEBUG("CompareF", 0);
    if (aNew.CompareF(aVerifyNew) != 0)     
        {
        // codes do not match note 
        ShowResultNoteL(CAknNoteDialog::EErrorTone,R_CODES_DONT_MATCH);
        return EFalse;
        }
    

    RMobilePhone::TMobilePhoneSecurityCode secCodeType;
    if (aPin == EPin1)
        {
        secCodeType = RMobilePhone::ESecurityCodePin1;
        }
    else
        {
        secCodeType = RMobilePhone::ESecurityCodePin2;
        }

    RMobilePhone::TMobilePhonePasswordChangeV1 aChange;
    aChange.iOldPassword = aOld;
    aChange.iNewPassword = aNew;

		RDEBUG("EMobilePhoneChangeSecurityCode", EMobilePhoneChangeSecurityCode);
    iWait->SetRequestType(EMobilePhoneChangeSecurityCode);
    iPhone.ChangeSecurityCode(iWait->iStatus,secCodeType,aChange);
    RDEBUG("WaitForRequestL", 0);
    TInt res = iWait->WaitForRequestL();
    RDEBUG("WaitForRequestL res", res);
    
    switch(res)
        {        
        case KErrNone:
            if (aPin == EPin1)
                {
                ShowResultNoteL(CAknNoteDialog::EConfirmationTone,R_PIN_CODE_CHANGED_NOTE);
                }
            else
                {
                ShowResultNoteL(CAknNoteDialog::EConfirmationTone,R_PIN2_CODE_CHANGED_NOTE);
                }
            return ETrue;
        case KErrGsmSSPasswordAttemptsViolation:
        case KErrLocked:
            if (aPin == EPin1)
                {
                ShowResultNoteL(CAknNoteDialog::EErrorTone,R_PIN_BLOCKED_NOTE);
                }
            else
                {
                iCustomPhone.CheckSecurityCode(iWait->iStatus,RMmCustomAPI::ESecurityCodePuk2);
               	RDEBUG("WaitForRequestL", 0);
                TInt res = iWait->WaitForRequestL();
                RDEBUG("WaitForRequestL res", res);
                }
            break;
        case KErrGsm0707OperationNotAllowed:
            ShowResultNoteL(CAknNoteDialog::EErrorTone,R_OPERATION_NOT_ALLOWED);
            break;
        default:
            ShowResultNoteL(CAknNoteDialog::EErrorTone,R_CODE_ERROR);
            break;
        }
    
    return EFalse;
    }
//
// ----------------------------------------------------------
// CManualSecuritySettings::CancelChangePin()
// Deletes pin operation note if necessary
// ----------------------------------------------------------
//
EXPORT_C void CManualSecuritySettings::CancelChangePin()
    {
    RDEBUG("0", 0);
    delete iNote;
    iNote = NULL;
    }
//
// ----------------------------------------------------------
// CManualSecuritySettings::UnblockPinL()
// Unblocks pin 
// ----------------------------------------------------------
//    
EXPORT_C TBool CManualSecuritySettings::UnblockPinL(TPin aPin,const TDesC& aPuk,const TDesC& aNew,const TDesC& aVerifyNew )
    {
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
    RDEBUG("0", 0);  
    if (aNew.CompareF(aVerifyNew) != 0)     
        {
        // codes do not match note 
        ShowResultNoteL(CAknNoteDialog::EErrorTone,R_CODES_DONT_MATCH);
        return EFalse;
        }
    
    RMobilePhone::TMobilePhoneSecurityCode blockCodeType;
    
    if (aPin == EPin1)
        {
        blockCodeType = RMobilePhone::ESecurityCodePuk1;
        }
    else
        {
        blockCodeType = RMobilePhone::ESecurityCodePuk2;
        }
		RDEBUG("blockCodeType", blockCodeType);
    iWait->SetRequestType(EMobilePhoneVerifySecurityCode);
    iPhone.VerifySecurityCode(iWait->iStatus, blockCodeType, aNew, aPuk);
    RDEBUG("WaitForRequestL", 0);
    TInt res = iWait->WaitForRequestL();
    RDEBUG("WaitForRequestL res", res);

    switch(res)
        {        
        case KErrNone:
            if (aPin == EPin1)
                {
                ShowResultNoteL(CAknNoteDialog::EConfirmationTone,R_PIN_CODE_CHANGED_NOTE);
                }
            else
                {
                ShowResultNoteL(CAknNoteDialog::EConfirmationTone,R_PIN2_CODE_CHANGED_NOTE);
                }
            return ETrue;
        case KErrGsmSSPasswordAttemptsViolation:
        case KErrLocked:
            if (aPin == EPin2)
                {
                ShowResultNoteL(CAknNoteDialog::EErrorTone,R_PIN2_REJECTED);
                }
            break;
        case KErrGsm0707OperationNotAllowed:
            ShowResultNoteL(CAknNoteDialog::EErrorTone,R_OPERATION_NOT_ALLOWED);
            break;
        default:
            ShowResultNoteL(CAknNoteDialog::EErrorTone,R_CODE_ERROR);
            break;
        }

    return EFalse;
    }
//
// ----------------------------------------------------------
// CManualSecuritySettings::CancelUnblockPin()
// Deletes unblock pin operation note if necessary
// ----------------------------------------------------------
//    
EXPORT_C void CManualSecuritySettings::CancelUnblockPin()
    {
    RDEBUG("0", 0);      
    delete iNote;
    iNote = NULL;        
    }
//
// ----------------------------------------------------------
// CManualSecuritySettings::LockSimL()
// 
// ----------------------------------------------------------
//        
EXPORT_C TBool CManualSecuritySettings::LockSimL(const TDesC& aLockCode,const TDesC& aType )
    {
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
    RDEBUG("0", 0);
    RMmCustomAPI::TLockNumber aLockType;
    TInt ret = KErrGsm0707IncorrectPassword;
    TInt length = aLockCode.Length();
    aLockType = RMmCustomAPI::EOperator;
    aLockType = aLockType;

    RDEBUG("length", length);
		// from now on, it accepts restricted lengths, although some locks are 20, others are 13
		// On 07/10/2010, this API is non-existent. Temporarilly remove the offendig code until a solution is found.
    if(aLockCode.Length() <= KSimLockMaxPasswordSize)
    {
    if (aType.CompareF(Operator) == 0)
        {
        aLockType = RMmCustomAPI::EOperator;
        // ret = iCustomPhone.ActivateSimLock( aLockCode,aLockType );
        }

    if (aType.CompareF(Gid1) == 0)
        {
        aLockType = RMmCustomAPI::EGid1;
        // ret = iCustomPhone.ActivateSimLock( aLockCode,aLockType );
        }

    if (aType.CompareF(Gid2) == 0)
        {
        aLockType = RMmCustomAPI::EGid2;
        // ret = iCustomPhone.ActivateSimLock( aLockCode,aLockType );
        }

    if (aType.CompareF(Imsi) == 0)
        {
        aLockType = RMmCustomAPI::EImsi;
        // ret = iCustomPhone.ActivateSimLock( aLockCode,aLockType );
        }

    if (aType.CompareF(Operator_Gid1) == 0)
        {    
        aLockType = RMmCustomAPI::EOperator_Gid1;
        // ret = iCustomPhone.ActivateSimLock( aLockCode,aLockType );
        }

    if (aType.CompareF(Operator_Gid2) == 0)
        {
        aLockType = RMmCustomAPI::EOperator_Gid2;
        // ret = iCustomPhone.ActivateSimLock( aLockCode,aLockType );
        }
    }

    RDEBUG("ret", ret);
    switch (ret)
        {
        case KErrNone:
            ShowResultNoteL(CAknNoteDialog::EConfirmationTone,R_SIM_ON);
            return ETrue;
        case KErrAlreadyExists:
            ShowResultNoteL(CAknNoteDialog::EConfirmationTone,R_SIM_ALLREADY_ON);
            break;
        case KErrGeneral:
        case KErrLocked:
        case KErrAccessDenied:
        case KErrNotSupported:
        case KErrGsm0707OperationNotAllowed:
            ShowResultNoteL(CAknNoteDialog::EErrorTone,R_SIM_NOT_ALLOWED);
            break;
        case KErrArgument:
        case KErrGsm0707IncorrectPassword:
            ShowResultNoteL(CAknNoteDialog::EErrorTone,R_CODE_ERROR);
            break;
        default:
            break;
        }

    return EFalse;
    }

//
// ----------------------------------------------------------
// CManualSecuritySettings::CancelLockSim()
// 
// ----------------------------------------------------------
//        
EXPORT_C void CManualSecuritySettings::CancelLockSim()
    {
    RDEBUG("0", 0);
    delete iNote;
    iNote = NULL;    
    }    
//
// ----------------------------------------------------------
// CManualSecuritySettings::UnlockSimL()
// 
// ----------------------------------------------------------
//            
EXPORT_C TBool CManualSecuritySettings::UnlockSimL(const TDesC& aUnlockCode,const TDesC& aType )
    {
    /*****************************************************
    *    Series 60 Customer / ETel
    *    Series 60  ETel API
    *****************************************************/
    RDEBUG("0", 0);
    RMmCustomAPI::TLockNumber aLockType;
    aLockType = RMmCustomAPI::EOperator;
    aLockType = aLockType;
    TInt ret = KErrGsm0707IncorrectPassword;
    TInt length = aUnlockCode.Length();

    RDEBUG("length", length);
		// from now on, it accepts restricted lengths, although some locks are 20, others are 13
		// On 07/10/2010, this API is non-existent. Temporarilly remove the offendig code until a solution is found.
    if(aUnlockCode.Length() <= KSimLockMaxPasswordSize)
    {
    if (aType.CompareF(Operator) == 0)
        {
        aLockType = RMmCustomAPI::EOperator;
        // ret = iCustomPhone.DeActivateSimLock( aUnlockCode,aLockType );
        }

    if (aType.CompareF(Gid1) == 0)
        {
        aLockType = RMmCustomAPI::EGid1;
        // ret = iCustomPhone.DeActivateSimLock( aUnlockCode,aLockType );
        }

    if (aType.CompareF(Gid2) == 0)
        {
        aLockType = RMmCustomAPI::EGid2;
        // ret = iCustomPhone.DeActivateSimLock( aUnlockCode,aLockType );
        }

    if (aType.CompareF(Imsi) == 0)
        {
        aLockType = RMmCustomAPI::EImsi;
        // ret = iCustomPhone.DeActivateSimLock( aUnlockCode,aLockType );
        }

    if (aType.CompareF(Operator_Gid1) == 0)
        {    
        aLockType = RMmCustomAPI::EOperator_Gid1;
        // ret = iCustomPhone.DeActivateSimLock( aUnlockCode,aLockType );
        }

    if (aType.CompareF(Operator_Gid2) == 0)
        {
        aLockType = RMmCustomAPI::EOperator_Gid2;
        // ret = iCustomPhone.DeActivateSimLock( aUnlockCode,aLockType );
        }
    if (aType.CompareF(GlobalUnlock) == 0)
        {
        aLockType = RMmCustomAPI::EGlobalUnlock;
        // ret = iCustomPhone.DeActivateSimLock( aUnlockCode,aLockType );
        }    
        
    }
    RDEBUG("ret", ret);
    switch (ret)
        {
        case KErrNone:
            ShowResultNoteL(CAknNoteDialog::EConfirmationTone,R_SIM_OFF);
            return ETrue;
        case KErrAlreadyExists:
            ShowResultNoteL(CAknNoteDialog::EConfirmationTone,R_SIM_ALLREADY_OFF);
            break;
        case KErrGeneral:
        case KErrLocked:
        case KErrAccessDenied:
        case KErrGsm0707OperationNotAllowed:
            ShowResultNoteL(CAknNoteDialog::EErrorTone,R_SIM_NOT_ALLOWED);
            break;
        case KErrArgument:
        case KErrGsm0707IncorrectPassword:
            ShowResultNoteL(CAknNoteDialog::EErrorTone,R_CODE_ERROR);
            break;
        default:
            break;
        }

    return EFalse;
    }
//
// ----------------------------------------------------------
// CManualSecuritySettings::CancelUnlockSim()
// 
// ----------------------------------------------------------
//                
EXPORT_C void CManualSecuritySettings::CancelUnlockSim()
    {
    RDEBUG("0", 0);
    delete iNote;
    iNote = NULL;
    }

//
// ----------------------------------------------------------
// CManualSecuritySettings::ShowResultNoteL()
// Shows settingchange results
// ----------------------------------------------------------
//
void CManualSecuritySettings::ShowResultNoteL(CAknNoteDialog::TTone aTone, TInt aResourceID)
    {
    RDEBUG("aResourceID", aResourceID);
    CSecuritySettings::ShowResultNoteL(aResourceID, aTone);
    }

// End of file
