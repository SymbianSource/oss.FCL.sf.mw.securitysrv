/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Implementation of RSecQueryUiCli class.
*
*/

#include "SecQueryUi.h"                              // CSecQueryUi
// #include <SecQueryUidefs.h>                          // SIF UI device dialog parameters
#include <hb/hbcore/hbdevicedialogsymbian.h>    // CHbDeviceDialogSymbian
#include <hb/hbcore/hbsymbianvariant.h>         // CHbSymbianVariantMap
#include <apgicnfl.h>                           // CApaMaskedBitmap
#include <securityuisprivatepskeys.h>

#include <cphcltemergencycall.h>
#include <SCPClient.h>
#include "SecUi.h"

const TUid KSWInstHelpUid =
    {
    0x101F8512
    }; // TODO


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CSecQueryUi::NewLC()
// ---------------------------------------------------------------------------
//
EXPORT_C CSecQueryUi* CSecQueryUi::NewLC()
    {
    CSecQueryUi* self = new (ELeave) CSecQueryUi();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::NewL()
// ---------------------------------------------------------------------------
//
EXPORT_C CSecQueryUi* CSecQueryUi::NewL()
    {
    CSecQueryUi* self = CSecQueryUi::NewLC();
    CleanupStack::Pop(self);
    return self;
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::~CSecQueryUi()
// ---------------------------------------------------------------------------
//
CSecQueryUi::~CSecQueryUi()
    {
    Cancel();
    delete iWait;
    delete iDeviceDialog;
    delete iVariantMap;
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::InstallConfirmationQueryL()
// ---------------------------------------------------------------------------
//
EXPORT_C TBool CSecQueryUi::InstallConfirmationQueryL(TInt aType,
        RMobilePhone::TMobilePassword& password)
/*
 const TDesC& aAppName,
 const TDesC& aIconFile, const TDesC& aAppVersion, TInt aAppSize,
 const TDesC& aAppDetails ) */
    {
    // this is never used. Kept as a reference
    RDEBUG("This should never be called. Obsolete aType", aType);
    return KErrAbort;
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::SecQueryDialog()
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CSecQueryUi::SecQueryDialog(const TDesC& aCaption,
        TDes& aDataText, TInt aMinLength, TInt aMaxLength, TInt aMode)
    {
    RDEBUG("aCaption", 0);
    RDEBUGSTR(aCaption);
    RDEBUG("aMode", aMode);
    TInt secUiOriginatedQuery(ESecurityUIsSecUIOriginatedUninitialized);
    TInt err = RProperty::Get(KPSUidSecurityUIs,
            KSecurityUIsSecUIOriginatedQuery, secUiOriginatedQuery);
    RDEBUG("secUiOriginatedQuery", secUiOriginatedQuery);
    if (secUiOriginatedQuery == ESecurityUIsSecUIOriginatedUninitialized )	// &&  )
        {
        // set only if not set
        err = RProperty::Set(KPSUidSecurityUIs,
                KSecurityUIsSecUIOriginatedQuery,
                ESecurityUIsETelAPIOriginated);
        RDEBUG("setting secUiOriginatedQuery", ESecurityUIsETelAPIOriginated);
      	}
		else if ( secUiOriginatedQuery != ESecurityUIsSecUIOriginated )
				{
        RDEBUG("!!!! warning: secUiOriginatedQuery", secUiOriginatedQuery);
        // The query is already shown. This is valid for ESecurityUIsSecUIOriginated, and maybe for ESecurityUIsETelAPIOriginated
        // For ESecurityUIsSystemLockOriginated it means that the "lock" dialog is already present.
        // Try to dismiss the dialog. Do same as CSecurityHandler::CancelSecCodeQuery
        TInt aDismissDialog = -1;
        err = RProperty::Get(KPSUidSecurityUIs, KSecurityUIsDismissDialog, aDismissDialog);
        // it might happen that the dialog is already dismissing. Well, it won't harm to try again.
        RDEBUG("aDismissDialog", aDismissDialog);
        RDEBUG("err", err);
        RDEBUG("set KSecurityUIsDismissDialog", ESecurityUIsDismissDialogOn);
        err = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsDismissDialog, ESecurityUIsDismissDialogOn);
        RDEBUG("err", err);
        }
		RDEBUG("calling ClearParamsAndSetNoteTypeL aMode", aMode);
    ClearParamsAndSetNoteTypeL(aMode);
    AddParamL(_L("KSecQueryUiApplicationName"), aCaption);

    _LIT(KTitle, "title");
    // _LIT(KTitleValue1, "Enter PIN");
    AddParamL(KTitle, aCaption);
    AddParamL(_L("MinLength"), aMinLength);
    AddParamL(_L("MaxLength"), aMaxLength);

    switch (aMode & ESecUiBasicTypeMask) {
    	case ESecUiBasicTypeCheck:
                             _LIT(KChecboxDialog, "ChecboxDialog");
                             _LIT(KChecbox, "ChecboxDialog");
                             AddParamL(KChecboxDialog,KChecbox);
                             break;
    	case ESecUiBasicTypeMultiCheck:
                            _LIT(KMultiChecboxDialog, "MultiChecboxDialog");
                            _LIT(KMultiChecbox, "MultiChecboxDialog");
                            AddParamL(KMultiChecboxDialog,KMultiChecbox);
                            break;
    	default:
                            _LIT(KCodeTop, "codeTop");
                            _LIT(KCodeTopValue, "codeTop");
                            AddParamL(KCodeTop, KCodeTopValue);
    }

    if (aCaption.Find(_L("|")) > 0)
        {
        RDEBUG("codeBottom aMode", aMode);
        _LIT(KCodeBottom, "codeBottom");
        _LIT(KCodeBottomValue, "codeBottom");
        AddParamL(KCodeBottom, KCodeBottomValue);
        }
    if (aDataText.Length() > 0)
        {
        RDEBUG("aDataText", 1);
        _LIT(KDefaultCode, "DefaultCode");
        AddParamL(KDefaultCode, aDataText);
        }

    RDEBUG("0", 0);
    DisplayDeviceDialogL();
    TSecUi::UnInitializeLib();	// the counterpart is at DisplayDeviceDialogL
    TInt error = WaitUntilDeviceDialogClosed();
    RDEBUG("error", error);
    User::LeaveIfError(error);
    RDEBUG("iPassword", 0);
		RDEBUGSTR(iPassword);
    aDataText.Copy(iPassword);

    err = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery,
            ESecurityUIsSecUIOriginatedUninitialized);
    RDEBUG("clearing secUiOriginatedQuery",
            ESecurityUIsSecUIOriginatedUninitialized);

    RDEBUG("iReturnValue", iReturnValue);
    return iReturnValue;
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::DisplayInformationNoteL()
// ---------------------------------------------------------------------------
//
EXPORT_C void CSecQueryUi::DisplayInformationNoteL(const TDesC& aText)
    {
    // this is never used. Kept as a reference
    RDEBUG("This should never be called. Obsolete", 0);
    /*
     ClearParamsAndSetNoteTypeL( SecQueryUiInformationNote );
     AddParamL( KNotifDeviceDialogKeyText, aText );
     AddParamL( KNotifDeviceDialogKeyTimeOut, 0 );
     iDeviceDialog->Show( KNotifDeviceDialogLiteral, *iVariantMap, this );
     User::LeaveIfError( WaitUntilDeviceDialogClosed() );
     */
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::DisplayWarningNoteL()
// ---------------------------------------------------------------------------
//
EXPORT_C void CSecQueryUi::DisplayWarningNoteL(const TDesC& aText)
    {
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::DisplayErrorNoteL()
// ---------------------------------------------------------------------------
//
EXPORT_C void CSecQueryUi::DisplayErrorNoteL(const TDesC& aText)
    {
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::DisplayPermanentNoteL()
// ---------------------------------------------------------------------------
//
EXPORT_C void CSecQueryUi::DisplayPermanentNoteL(const TDesC& aText)
    {
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::ClosePermanentNote()
// ---------------------------------------------------------------------------
//
EXPORT_C void CSecQueryUi::ClosePermanentNote()
    {
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::DisplayProgressNoteL()
// ---------------------------------------------------------------------------
//
EXPORT_C void CSecQueryUi::DisplayProgressNoteL(const TDesC& aText,
        TInt aFinalValue)
    {
    /*
     ClearParamsAndSetNoteTypeL( ESecQueryUiProgressNoteType );
     AddParamL( KSecQueryUiProgressNoteText, aText );
     AddParamL( KSecQueryUiProgressNoteFinalValue, aFinalValue );
     DisplayDeviceDialogL();
     */}

// ---------------------------------------------------------------------------
// CSecQueryUi::UpdateProgressNoteValueL()
// ---------------------------------------------------------------------------
//
EXPORT_C void CSecQueryUi::UpdateProgressNoteValueL(TInt aNewValue)
    {
    /*
     ClearParamsAndSetNoteTypeL( ESecQueryUiProgressNoteType );
     AddParamL( KSecQueryUiProgressNoteValue, aNewValue );
     DisplayDeviceDialogL();
     */}

// ---------------------------------------------------------------------------
// CSecQueryUi::CloseProgressNoteL()
// ---------------------------------------------------------------------------
//
EXPORT_C void CSecQueryUi::CloseProgressNoteL()
    {
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::DisplayWaitNoteL()
// ---------------------------------------------------------------------------
//
EXPORT_C void CSecQueryUi::DisplayWaitNoteL(const TDesC& aText,
        TRequestStatus& aStatus)
    {
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::CloseWaitNote()
// ---------------------------------------------------------------------------
//
EXPORT_C void CSecQueryUi::CloseWaitNote()
    {
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::LaunchHelpL()
// ---------------------------------------------------------------------------
//
EXPORT_C void CSecQueryUi::LaunchHelpL(const TDesC& aContext,
        const TUid& aUid)
    {
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::LaunchHelpL()
// ---------------------------------------------------------------------------
//
EXPORT_C void CSecQueryUi::LaunchHelpL(const TDesC& aContext)
    {
    LaunchHelpL(aContext, KSWInstHelpUid);
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::DoCancel()
// ---------------------------------------------------------------------------
//
void CSecQueryUi::DoCancel()
    {
    RDEBUG("0", 0);
    if (iWait && iWait->IsStarted() && iWait->CanStopNow())
        {
        iCompletionCode = KErrCancel;
        iWait->AsyncStop();
        }
    RDEBUG("0", 0);
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::RunL()
// ---------------------------------------------------------------------------
//
void CSecQueryUi::RunL()
    {
    RDEBUG("0", 0);
    if (iWait)
        {
        iWait->AsyncStop();
        }
    RDEBUG("0", 0);
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::DataReceived()
// ---------------------------------------------------------------------------
//
void CSecQueryUi::DataReceived(CHbSymbianVariantMap& aData)
    {
    RDEBUG("0", 0);
    const CHbSymbianVariant* acceptedVariant = aData.Get(_L("accepted")); // KSecQueryUiQueryAccepted
    RDEBUG("0", 0);
    if (acceptedVariant)
        {
        RDEBUG("0", 0);
        TInt* acceptedValue = acceptedVariant->Value<TInt> ();
        RDEBUG("acceptedValue", acceptedValue);
        RDEBUG("*acceptedValue", *acceptedValue);
        if (acceptedValue)
            {
            iReturnValue = *acceptedValue;
            }
        }
    const CHbSymbianVariant* acceptedVariantTop = aData.Get(_L("codeTop")); // KSecQueryUiQueryAccepted
    RDEBUG("0", 0);
    if (acceptedVariantTop)
        {
        TPtrC acceptedValueTop = *acceptedVariantTop->Value<TDesC> ();
        RDEBUG("acceptedValueTop", 0);
        RDEBUGSTR(acceptedValueTop);
        iPassword.Copy(acceptedValueTop);

        if (iReturnValue == KErrCompletion) // the user didn't OK. It was send automatically because validating new lock code through TARM
            {
            _LIT(KInvalidNewLockCode, "invalidNewLockCode");
            _LIT(KInvalidNewLockCode0, "invalidNewLockCode$-1");
            AddParamL(KInvalidNewLockCode, KInvalidNewLockCode0); // for starter
            RSCPClient scpClient;
            TSCPSecCode newCode;
            newCode.Copy(acceptedValueTop);
            RDEBUG("scpClient.Connect", 0);
            if (scpClient.Connect() == KErrNone)
                {
                 RArray<TDevicelockPolicies> aFailedPolicies;
                 TDevicelockPolicies failedPolicy;
                 TInt retLockcode = KErrNone;
                 TInt nPoliciesFailed = 0;
                 RDEBUG( "scpClient.VerifyNewLockcodeAgainstPolicies", 0 );
                 retLockcode = scpClient.VerifyNewLockcodeAgainstPolicies( newCode, aFailedPolicies );
                 RDEBUG( "retLockcode", retLockcode );
                 nPoliciesFailed = aFailedPolicies.Count();
                 RDEBUG( "nPoliciesFailed", nPoliciesFailed );
                 for(TInt i=0; i<nPoliciesFailed; i++)
	                 {
	                 failedPolicy = aFailedPolicies[i];
	                 RDEBUG( "failedPolicy", failedPolicy );
	                 TBuf<0x100> KInvalidNewLockCodeX;   KInvalidNewLockCodeX.Zero();    KInvalidNewLockCodeX.Append(_L("invalidNewLockCode"));  KInvalidNewLockCodeX.Append(_L("$"));
	                 KInvalidNewLockCodeX.AppendNum(failedPolicy);
	                 AddParamL( KInvalidNewLockCode, KInvalidNewLockCodeX );	// it overwrites the previous one
	                 }
                // TODO this should be able to modify MinLenght, MaxLenght
                scpClient.Close();
                }
            RDEBUG("iDeviceDialog->Update", 0);
            iDeviceDialog->Update(*iVariantMap);
            } // KErrCompletion

        if (acceptedValueTop.Length() <= 4) // TODO store aMinLenght and check it here, instead of "4"
            {
            RDEBUG("CPhCltEmergencyCall", 0);
            CPhCltEmergencyCall* emergencyCall = CPhCltEmergencyCall::NewL(
                    NULL);
            RDEBUG("PushL", 0);
            CleanupStack::PushL(emergencyCall);
            TPhCltEmergencyNumber emNumber;

            // this relies on the fact that emergency has 3 digits, and password needs at least 4
            TBool isEmergency(EFalse);
            RDEBUG("calling IsEmergencyPhoneNumber", 0);
            TInt error = emergencyCall->IsEmergencyPhoneNumber(
                    acceptedValueTop, isEmergency);
            RDEBUG("error", error);
            RDEBUG("emNumber", 0);

            RDEBUG("isEmergency", isEmergency);
#ifdef __WINS__
            RDEBUG( "__WINS__ checking", 0 );
            if(!acceptedValueTop.CompareF(_L("112")) || !acceptedValueTop.CompareF(_L("911")) || !acceptedValueTop.CompareF(_L("555")) )
                {
                isEmergency = ETrue;
                error = KErrNone;
                RDEBUG( "__WINS__ isEmergency", isEmergency );
                }
#endif

            if (!error) // oddly enough, missing capabilities also gives KErrNone
                {
                if (iReturnValue == KErrAbort) // the user didn't OK. It was send automatically because short code
                    {
                    _LIT(KEmergency, "emergency");
                    _LIT(KEmergencyValueYes, "emergencyYes");
                    _LIT(KEmergencyValueNo, "emergencyNo");
                    if (isEmergency)
                        {
                        RDEBUG("KEmergencyValueYes", 1);
                        AddParamL(KEmergency, KEmergencyValueYes);
                        }
                    else
                        {
                        RDEBUG("KEmergencyValueNo", 0);
                        AddParamL(KEmergency, KEmergencyValueNo);
                        }
                    iDeviceDialog->Update(*iVariantMap);
                    }
                else if (iReturnValue == KErrNone)
                    { // user pressed Call and number is valid
                    if (isEmergency)
                        {
                        RDEBUG("DialEmergencyCallL", isEmergency);
                        emergencyCall->DialEmergencyCallL(emNumber);
                        iReturnValue = KErrAbort; // this means emergency call
                        }
                    }
                } // if !error
            RDEBUG("0", 0);
            CleanupStack::PopAndDestroy(emergencyCall);
            } // lenght<3
        } // acceptedVariantTop
    RDEBUG("iReturnValue", iReturnValue);
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::DeviceDialogClosed()
// ---------------------------------------------------------------------------
//
void CSecQueryUi::DeviceDialogClosed(TInt aCompletionCode)
    {
    RDEBUG("aCompletionCode", aCompletionCode);
    iCompletionCode = aCompletionCode;
    iIsDisplayingDialog = EFalse;

    TRequestStatus* status(&iStatus);
    RDEBUG("0", 0);
    User::RequestComplete(status, KErrNone);
    RDEBUG("0", 0);
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::CSecQueryUi()
// ---------------------------------------------------------------------------
//
CSecQueryUi::CSecQueryUi() :
    CActive(CActive::EPriorityStandard)
    {
    RDEBUG("0", 0);
    CActiveScheduler::Add(this);
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::ConstructL()
// ---------------------------------------------------------------------------
//
void CSecQueryUi::ConstructL()
    {
    RDEBUG("0", 0);
    iWait = new (ELeave) CActiveSchedulerWait;
    // iDeviceDialog is allocated later, first call of DisplayDeviceDialogL()
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::ClearParamsL()
// ---------------------------------------------------------------------------
//
void CSecQueryUi::ClearParamsL()
    {
    RDEBUG("0", 0);
    if (iVariantMap)
        {
        delete iVariantMap;
        iVariantMap = NULL;
        }
    iVariantMap = CHbSymbianVariantMap::NewL();
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::ClearParamsAndSetNoteTypeL()
// ---------------------------------------------------------------------------
//
void CSecQueryUi::ClearParamsAndSetNoteTypeL(TInt aType)
    {
    RDEBUG("aType", aType);
    ClearParamsL();
    AddParamL(_L("type"), aType);
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::AddParamL()
// ---------------------------------------------------------------------------
//
void CSecQueryUi::AddParamL(const TDesC& aKey, TInt aValue)
    {
    RDEBUG("aValue", aValue);
    CHbSymbianVariant* variant = NULL;
    variant = CHbSymbianVariant::NewL(&aValue, CHbSymbianVariant::EInt);
    iVariantMap->Add(aKey, variant);
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::AddParamL()
// ---------------------------------------------------------------------------
//
void CSecQueryUi::AddParamL(const TDesC& aKey, const TDesC& aValue)
    {
    RDEBUG("0", 0);
    CHbSymbianVariant* variant = NULL;
    variant = CHbSymbianVariant::NewL(&aValue, CHbSymbianVariant::EDes);
    iVariantMap->Add(aKey, variant);
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::DisplayDeviceDialogL()
// ---------------------------------------------------------------------------
//
void CSecQueryUi::DisplayDeviceDialogL()
    {
    RDEBUG("0", 0);
    TInt err = KErrNone;
    RDEBUG("iIsDisplayingDialog", iIsDisplayingDialog);
    if (iDeviceDialog && iIsDisplayingDialog)
        {
    		RDEBUG("iDeviceDialog", 1);
        iDeviceDialog->Update(*iVariantMap);
        }
    else
        {
    		RDEBUG("!iDeviceDialog", 0);
        if (!iDeviceDialog)
            {
            RDEBUG("new iDeviceDialog", 0);
            iDeviceDialog = CHbDeviceDialogSymbian::NewL();
            }
        _LIT(KSecQueryUiDeviceDialog, "com.nokia.secuinotificationdialog/1.0");
        RDEBUG("Show", 0);
        err = iDeviceDialog->Show(KSecQueryUiDeviceDialog, *iVariantMap, this);
        RDEBUG("err", err);
        TSecUi::InitializeLibL();
        RDEBUG("iIsDisplayingDialog", iIsDisplayingDialog);
        iIsDisplayingDialog = ETrue;
        RDEBUG("iIsDisplayingDialog", iIsDisplayingDialog);
        }
    RDEBUG("0", 0);
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::WaitUntilDeviceDialogClosed()
// ---------------------------------------------------------------------------
//
TInt CSecQueryUi::WaitUntilDeviceDialogClosed()
    {
    RDEBUG("0", 0);
    iCompletionCode = KErrInUse;
    iReturnValue = KErrUnknown;
    if (!IsActive() && iWait && !iWait->IsStarted())
        {
        RDEBUG("KRequestPending", KRequestPending);
        iStatus = KRequestPending;
        SetActive();
        RDEBUG("Start", 0);
        iWait->Start();
        RDEBUG("Started", 1);
        }
    RDEBUG("iCompletionCode", iCompletionCode);
    return iCompletionCode;
    }

