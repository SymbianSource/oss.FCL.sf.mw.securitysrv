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
 * Description:  Basic PIN query operation in secui dialog
 *
 */

#include "secuidialogoperbasicpinquery.h" // CBasicPinQueryOperation
#include "secuidialogoperationobserver.h" // MSecuiDialogOperationObserver
#include <hb/hbcore/hbtextresolversymbian.h> // HbTextResolverSymbian
#include "secuidialogstrace.h"       // TRACE macro
// TODO: fix this
#include "../../../securitydialogs/SecUi/Inc/SecQueryUi.h"  // CSecQueryUi
#include <secui.h>
#include <secuisecurityhandler.h>
#include <gsmerror.h>
#include <etelmm.h>
#include <rmmcustomapi.h>
#include <startupdomainpskeys.h>
#include <featmgr.h>

const TInt KPhoneIndex(0);
const TInt KTriesToConnectServer(2);
const TInt KTimeBeforeRetryingServerConnection(50000);

_LIT( KMmTsyModuleName, "PhoneTsy");

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CBasicPinQueryOperation::NewL()
// ---------------------------------------------------------------------------
//
CBasicPinQueryOperation* CBasicPinQueryOperation::NewL(MSecuiDialogOperationObserver& aObserver, const RMessage2& aMessage, TInt aReplySlot)
    {
    RDEBUG("aMessage.Handle()", aMessage.Handle());
    RDEBUG("aMessage.Function()", aMessage.Function());
    return new (ELeave) CBasicPinQueryOperation(aObserver, aMessage, aReplySlot);
    }

// ---------------------------------------------------------------------------
// CBasicPinQueryOperation::~CBasicPinQueryOperation()
// ---------------------------------------------------------------------------
//
CBasicPinQueryOperation::~CBasicPinQueryOperation()
    {
    RDEBUG("0", 0);
    Cancel();
    iPinInput = NULL; // not owned
    }

// ---------------------------------------------------------------------------
// CBasicPinQueryOperation::StartL()
// ---------------------------------------------------------------------------
//
void CBasicPinQueryOperation::StartL(const TDesC8& aBuffer)
    {
    RDEBUG("0", 0);
    iPinInput = reinterpret_cast<const TPINInput*> (aBuffer.Ptr());
    ASSERT(iPinInput != NULL);

    iStatus = KRequestPending;
    TRequestStatus* status = &iStatus;
    User::RequestComplete(status, KErrNone);
    SetActive();
    }

// ---------------------------------------------------------------------------
// CBasicPinQueryOperation::CancelOperation()
// ---------------------------------------------------------------------------
//
void CBasicPinQueryOperation::CancelOperation()
    {
    RDEBUG("0", 0);
    // nothing to do
    }

// ---------------------------------------------------------------------------
// CBasicPinQueryOperation::RunL()
// ---------------------------------------------------------------------------
//
void CBasicPinQueryOperation::RunL()
    {
    RDEBUG("iStatus.Int()", iStatus.Int());
    User::LeaveIfError(iStatus.Int());

    TBool isRetry = (iPinInput->iOperation & EPINValueIncorrect);
    if (isRetry)
        {
        // Show "Invalid PIN code" error note, as previous attempt was failed.
        // TODO: localized UI string needed
        _LIT(KInvalidPinCode, "Invalid PIN code");
        ShowWarningNoteL( KInvalidPinCode);
        }
    RDEBUG("iPinInput->iOperation", iPinInput->iOperation);

    iPinValue.Copy(_L("0"));
    RDEBUG("0", 0);

    TInt resultVerif = KErrNone;
    RDEBUG("0", 0);

        {
        RMobilePhone iPhone;

        TInt err(KErrGeneral);
        RDEBUG("ESecurityQueryActive", ESecurityQueryActive);
        err = RProperty::Set(KPSUidStartup, KStartupSecurityCodeQueryStatus, ESecurityQueryActive);
        RDEBUG("err", err);

        TInt thisTry(0);
        RTelServer iTelServer;
        RMmCustomAPI iCustomPhone;
        while ((err = iTelServer.Connect()) != KErrNone && (thisTry++) <= KTriesToConnectServer)
            {
            User::After( KTimeBeforeRetryingServerConnection);
            }
        err = iTelServer.LoadPhoneModule(KMmTsyModuleName);
        RTelServer::TPhoneInfo PhoneInfo;
        err = iTelServer.SetExtendedErrorGranularity(RTelServer::EErrorExtended);
        err = iTelServer.GetPhoneInfo(KPhoneIndex, PhoneInfo);
        err = iPhone.Open(iTelServer, PhoneInfo.iName);
        err = iCustomPhone.Open(iPhone);
        RDEBUG("err", err);
        CSecurityHandler* handler = new (ELeave) CSecurityHandler(iPhone);
        CleanupStack::PushL(handler);
        // TSecUi::InitializeLibL(); 
        RDEBUG("0", 0);
        RMobilePhone::TMobilePhoneSecurityEvent iEvent;
        TInt lEvent = iPinInput->iOperation;
        RDEBUG("lEvent", lEvent);
        if (lEvent >= 0x1000) // flag for iStartUp
            {
            lEvent -= 0x1000;
            }
        iEvent = static_cast<RMobilePhone::TMobilePhoneSecurityEvent> (lEvent);
        RDEBUG("iEvent", iEvent);
        RDEBUG("iStartUp", iStartUp);
        TRAPD(resultHandler, handler->HandleEventL(iEvent, iStartUp, resultVerif));
        RDEBUG("resultHandler", resultHandler);
        RDEBUG("resultVerif", resultVerif);

        // if something went wrong cancel the code request
        if (resultHandler)
            {
            if (resultVerif == KErrNone) // if the process failed, then the result shoud also indicate the failure (unless it does it already)
                resultVerif = resultHandler;
            RDEBUG("iEvent", iEvent);
            TBool wcdmaSupported(FeatureManager::FeatureSupported(KFeatureIdProtocolWcdma));
            TBool upinSupported(FeatureManager::FeatureSupported(KFeatureIdUpin));
            switch (iEvent)
                {
                case RMobilePhone::EUniversalPinRequired:
                    if (wcdmaSupported || upinSupported)
                        {
                        iPhone.AbortSecurityCode(RMobilePhone::ESecurityUniversalPin);
                        }
                    break;
                case RMobilePhone::EUniversalPukRequired:
                    if (wcdmaSupported || upinSupported)
                        {
                        iPhone.AbortSecurityCode(RMobilePhone::ESecurityUniversalPuk);
                        }
                    break;
                case RMobilePhone::EPin1Required:
                    iPhone.AbortSecurityCode(RMobilePhone::ESecurityCodePin1);
                    break;
                case RMobilePhone::EPuk1Required:
                    iPhone.AbortSecurityCode(RMobilePhone::ESecurityCodePuk1);
                    break;
                case RMobilePhone::EPin2Required:
                    iPhone.AbortSecurityCode(RMobilePhone::ESecurityCodePin2);
                    break;
                case RMobilePhone::EPuk2Required:
                    iPhone.AbortSecurityCode(RMobilePhone::ESecurityCodePuk2);
                    break;
                case RMobilePhone::EPhonePasswordRequired:
                    iPhone.AbortSecurityCode(RMobilePhone::ESecurityCodePhonePassword);
                    break;
                default:
                    RDEBUG("default iEvent", iEvent);
                    break;
                }
            }

        // uninitialize security ui
        RDEBUG("PopAndDestroy 0", 0);
        CleanupStack::PopAndDestroy(handler); // handler
        RDEBUG("UnInitializeLib 0", 0);
        TSecUi::UnInitializeLib();
        RDEBUG("ESecurityQueryNotActive", ESecurityQueryNotActive);
        err = RProperty::Set(KPSUidStartup, KStartupSecurityCodeQueryStatus, ESecurityQueryNotActive);
        RDEBUG("err", err);
        //close ETel connection
        if (iTelServer.Handle())
            {
            RDEBUG("iPhone.Close()", 0);
            iPhone.Close();
            iTelServer.UnloadPhoneModule(KMmTsyModuleName);
            iTelServer.Close();
            RDEBUG("0", 0);
            }
        }

    ReturnResultL(resultVerif);

    }

// ---------------------------------------------------------------------------
// CBasicPinQueryOperation::DoCancel()
// ---------------------------------------------------------------------------
//
void CBasicPinQueryOperation::DoCancel()
    {
    RDEBUG("0", 0);
    }

// ---------------------------------------------------------------------------
// CBasicPinQueryOperation::CBasicPinQueryOperation()
// ---------------------------------------------------------------------------
//
CBasicPinQueryOperation::CBasicPinQueryOperation(MSecuiDialogOperationObserver& aObserver, const RMessage2& aMessage, TInt aReplySlot) :
    CSecuiDialogOperation(aObserver, aMessage, aReplySlot)
    {
    RDEBUG("aMessage.Handle()", aMessage.Handle());
    RDEBUG("aMessage.Function()", aMessage.Function());
    iStartUp = EFalse;
    if (aMessage.Function() >= 0x1000)
        iStartUp = ETrue;
    }

// ---------------------------------------------------------------------------
// CBasicPinQueryOperation::ReturnResultL()
// ---------------------------------------------------------------------------
//
void CBasicPinQueryOperation::ReturnResultL(TInt aErrorCode)
    {
    RDEBUG("aErrorCode", aErrorCode);
    if (aErrorCode <= KErrNone) // TODO should skip WriteL is error?
        {
        TPINValueBuf output(iPinValue);
        iMessage.WriteL(iReplySlot, output);
        }
    RDEBUG("Complete iMessage.Handle()", iMessage.Handle());
    iMessage.Complete(aErrorCode);
    RDEBUG("informing observer 0", 0);
    iObserver.OperationComplete();
    RDEBUG("0x99", 0x99);
    }

