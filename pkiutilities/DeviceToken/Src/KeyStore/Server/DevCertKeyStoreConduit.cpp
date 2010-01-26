/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Implementation of DevCertKeyStoreConduit
*
*/



#ifdef SYMBIAN_ENABLE_SPLIT_HEADERS
#include <mctkeystoreuids.h>
#endif
#include "DevCertKeyStoreConduit.h"
#include "DevCertKeyStoreServer.h"
#include "DevCertKeyStoreSession.h"
#include "DevCertOpenedKeysSrv.h"
#include "DevTokenDataTypes.h"
#include "DevTokenMarshaller.h"
#include "DevTokenUtil.h"
#include <asymmetric.h>
#include <mctkeystoremanager.h>


// ======== MEMBER FUNCTIONS ========

// CDevCertKeyStoreConduit

// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::NewL()
// ---------------------------------------------------------------------------
// 
CDevCertKeyStoreConduit* CDevCertKeyStoreConduit::NewL(CDevCertKeyStoreServer& aServer)
    {
    CDevCertKeyStoreConduit* self = new (ELeave) CDevCertKeyStoreConduit(aServer);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::~CDevCertKeyStoreConduit()
// ---------------------------------------------------------------------------
// 
CDevCertKeyStoreConduit::~CDevCertKeyStoreConduit()
    {
    Cancel();
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::CDevCertKeyStoreConduit()
// ---------------------------------------------------------------------------
// 
CDevCertKeyStoreConduit::CDevCertKeyStoreConduit(CDevCertKeyStoreServer& aServer)
  : CActive(EPriorityHigh), iServer(aServer), iCurrentRequest(iStatus)
    {
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::ConstructL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreConduit::ConstructL()
    {
    CActiveScheduler::Add(this);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::ServiceRequestL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreConduit::ServiceRequestL(const RMessage2& aMessage, CDevCertKeyStoreSession& aSession)
    {
    TDevTokenMessages request = static_cast<TDevTokenMessages>(aMessage.Function());

    if (iCurrentRequest.OutstandingRequest()!=EIdle)
        {
        //  There is currently a request outstanding, only allow this one if it's a cancel
        if ( (request!=ECancelCreateKey) &&
        (request!=ECancelImportKey) &&
        (request!=ECancelImportEncryptedKey) &&
        (request!=ECancelExportKey) &&
        (request!=ECancelRSASign) &&
        (request!=ECancelDSASign) &&
        (request!=ECancelDecrypt) )
            {
            User::Leave(KErrServerBusy);
            }
        }

    switch (request)
        {
        case EListKeys:
            ListL(aMessage);
            break;
        case EGetKeyInfo:
            GetKeyInfoL(aMessage);
            break;
        case ECreateKey:
            CreateKeyL(aMessage);
            break;
        case ECancelCreateKey:
            CancelCreateKey(aMessage);
            break;
        case EImportKey:
        case EImportEncryptedKey:
            ImportKeyL(aMessage);
            break;
        case ECancelImportKey:
        case ECancelImportEncryptedKey:
            CancelImportKey(aMessage);
            break;
        case EExportKey:
            ExportKeyL(aMessage);
            break;
        case ECancelExportKey:
            CancelExportKey(aMessage);
            break;
        case EExportPublic:
            ExportPublicL(aMessage);
            break;
        case EGetKeyLength:
            GetKeyLengthL(aMessage);
            break;
        case EDeleteKey:
            DeleteKeyL(aMessage);
            break;
        case ESetUsePolicy:
            SetUsePolicyL(aMessage);
            break;
        case ESetManagementPolicy:
            SetManagementPolicyL(aMessage);
            break;
        case EOpenKeyRepudiableRSASign:
            OpenKeyL(aMessage, aSession, KRSARepudiableSignerUID);
            break;
        case EOpenKeyRepudiableDSASign:
            OpenKeyL(aMessage, aSession, KDSARepudiableSignerUID);
            break;
        case EOpenKeyDecrypt:
            OpenKeyL(aMessage, aSession, KPrivateDecryptorUID);
            break;
        case EOpenKeyAgree:
            OpenKeyL(aMessage, aSession, KKeyAgreementUID);
            break;
        case ECloseObject:
            CloseObjectL(aMessage, aSession);
            break;
        case ERepudiableDSASign:
            RepudiableDSASignL(aMessage, aSession);
            break;
        case ECancelDSASign:
            CancelDSASign(aMessage);
            break;         
        case ERepudiableRSASign:
            RepudiableRSASignL(aMessage, aSession);
            break;
        case ECancelRSASign:
            CancelRSASign(aMessage);
            break;         
        case EDecryptText:
            DecryptL(aMessage, aSession);
            break;
        case ECancelDecrypt:
            CancelDecrypt(aMessage);
            break;         
        case EDHPublicKey:
            DHPublicKeyL(aMessage, aSession);
            break;
        case EDHAgree:
            DHAgreeL(aMessage, aSession);
            break;
        case ECancelDH:
            CancelDH(aMessage);
            break;         

        default:  //  Should not reach here
            __ASSERT_DEBUG(EFalse, PanicServer(EPanicInvalidRequest));
            User::Leave(KErrNotSupported);
        }
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::ListL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreConduit::ListL(const RMessage2& aMessage)
    {
    //  p[0] has the filter to use
    RCPointerArray<CDevTokenKeyInfo> keyInfos;
    CleanupClosePushL(keyInfos);

    TPckgBuf<TCTKeyAttributeFilter> filter;
    aMessage.ReadL(0, filter);

    iServer.ListL(filter(), keyInfos);

    TInt bufSize = User::LeaveIfError(aMessage.GetDesMaxLength(2)); 
    TInt reqdSize = DevTokenDataMarshaller::Size(keyInfos);
    if (bufSize >= reqdSize)
        {
        HBufC8* buffer = HBufC8::NewMaxLC(reqdSize);
        TPtr8 thePtr(buffer->Des());
        DevTokenDataMarshaller::Write(keyInfos, thePtr);
        aMessage.WriteL(2, thePtr);
        CleanupStack::PopAndDestroy(buffer);
        }
    else
        {
        TPckg<TInt> sizePckg(reqdSize);
        aMessage.WriteL(2, sizePckg);
        User::Leave(KErrOverflow);
        }

    CleanupStack::PopAndDestroy(&keyInfos); // keyInfos
    aMessage.Complete(KErrNone);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::GetKeyInfoL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreConduit::GetKeyInfoL(const RMessage2& aMessage)
    {
    //  Token object to delete is in p[0]
    TInt objectId = aMessage.Int0();
    CDevTokenKeyInfo* info = NULL;
    iServer.GetKeyInfoL(objectId, info);
    info->CleanupPushL();

    TInt bufferSize = User::LeaveIfError(aMessage.GetDesMaxLength(2));
    TInt requiredSize = DevTokenDataMarshaller::Size(*info);

    if (bufferSize >= requiredSize)
        {
        HBufC8* buffer = HBufC8::NewMaxLC(requiredSize);
        TPtr8 thePtr(buffer->Des());
        DevTokenDataMarshaller::Write(*info, thePtr);
        aMessage.WriteL(2, thePtr);
        CleanupStack::PopAndDestroy(buffer);
        }
    else
        {
        TPckg<TInt> theSize(requiredSize);
        aMessage.WriteL(2, theSize);
        User::Leave(KErrOverflow);
        }

    CleanupStack::PopAndDestroy(info);
    aMessage.Complete(KErrNone);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::GetClientUidL()
// Work out the UID of the client process.
// ---------------------------------------------------------------------------
//
TUid CDevCertKeyStoreConduit::GetClientUidL(const RMessage2& aMessage) const
    {
    RThread clientThread;
    User::LeaveIfError(aMessage.Client(clientThread));
    CleanupClosePushL(clientThread);  
    RProcess clientProcess;
    User::LeaveIfError(clientThread.Process(clientProcess));
    CleanupClosePushL(clientProcess);
    TUid uid = clientProcess.Type()[2];
    CleanupStack::PopAndDestroy(2); // clientProcess, clientThread
    return uid;
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::CreateKeyL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreConduit::CreateKeyL(const RMessage2& aMessage)
    {
    ASSERT(!iKeyInfo);
    //  p[0] has the length of the buffer. Check our buffer is big
    //  enough, to cope with requests for keys with very long labels.

    TInt bufLength = User::LeaveIfError(aMessage.GetDesLength(1));
    HBufC8* keyInfoBuf = HBufC8::NewMaxLC(bufLength);

    //  p[1] has the CDevTokenKeyInfo structure required to create a key
    //  Read it and convert from descriptor back to a CDevTokenKeyInfo
    TPtr8 thePtr(keyInfoBuf->Des());
    thePtr.FillZ();

    aMessage.ReadL(1, thePtr);
    DevTokenDataMarshaller::ReadL(*keyInfoBuf, iKeyInfo);
    CleanupStack::PopAndDestroy(keyInfoBuf);

    iCurrentRequest.Set(ECreateKey, aMessage);
    iStatus = KRequestPending;
    SetActive();
    iServer.CreateKey(*iKeyInfo, iStatus);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::CancelCreateKey()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreConduit::CancelCreateKey(const RMessage2& aMessage)
    {
    if (iCurrentRequest.OutstandingRequest() == ECreateKey)
        {
        Cancel();
        }
    aMessage.Complete(KErrNone);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::ImportKeyL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreConduit::ImportKeyL(const RMessage2& aMessage)
    {
    ASSERT(!iImportKey);
    ASSERT(!iKeyInfo);

    // p[0] has the descriptor containing the PKCS8 object (not be encrypted)
    TInt keyLen = User::LeaveIfError(aMessage.GetDesLength(0));

    HBufC8* importBuf = HBufC8::NewMaxLC(keyLen);
    TPtr8 theKeyData(importBuf->Des());
    theKeyData.FillZ();
    aMessage.ReadL(0, theKeyData);

    TInt bufLen = User::LeaveIfError(aMessage.GetDesLength(1));
    HBufC8* keyInfoBuf = HBufC8::NewMaxLC(bufLen);

    // p[1] has the CDevTokenKeyInfo structure required to create a key
    // Read it and convert from descriptor back to a CDevTokenKeyInfo
    TPtr8 thePtr(keyInfoBuf->Des());
    thePtr.FillZ();
    aMessage.ReadL(1, thePtr);

    DevTokenDataMarshaller::ReadL(*keyInfoBuf, iKeyInfo);
    CleanupStack::PopAndDestroy(keyInfoBuf);

    iImportKey = importBuf;
    CleanupStack::Pop(importBuf);

    iCurrentRequest.Set(static_cast<TDevTokenMessages>(aMessage.Function()), aMessage);
    iStatus = KRequestPending;
    SetActive();
    
    TBool isEncrypted = (aMessage.Function() == EImportEncryptedKey);
	iServer.ImportKey(*iImportKey, *iKeyInfo, isEncrypted, iStatus);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::CancelImportKey()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreConduit::CancelImportKey(const RMessage2& aMessage)
    {
    if (iCurrentRequest.OutstandingRequest() == EImportKey ||
		iCurrentRequest.OutstandingRequest() == EImportEncryptedKey)
        {
        Cancel();
        }
    aMessage.Complete(KErrNone); 
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::GetKeyLengthL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreConduit::GetKeyLengthL(const RMessage2& aMessage)
    {
    TInt length = iServer.GetKeyLengthL(aMessage.Int0());
    ASSERT(length > 0);
    aMessage.Complete(length);    
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::CancelExportKey()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreConduit::CancelExportKey(const RMessage2& aMessage)
    {
    if (iCurrentRequest.OutstandingRequest() == EExportKey ||
        iCurrentRequest.OutstandingRequest() == EImportEncryptedKey)
        {
        Cancel();
        }
    aMessage.Complete(KErrNone);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::ExportKeyL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreConduit::ExportKeyL(const RMessage2& aMessage)
    {
    ASSERT(!iExportBuf);

    TInt objectId = aMessage.Int0();
    TInt bufLen = User::LeaveIfError(aMessage.GetDesMaxLength(1));  // #1 IPC argument is the request Ptr

    HBufC8* exportBuf =  HBufC8::NewMaxLC(bufLen);
    TPtr8 temp(exportBuf->Des());
    temp.FillZ();

    // No more leaves
    iExportBuf = exportBuf;
    CleanupStack::Pop(exportBuf);

    iCurrentRequest.Set(static_cast<TDevTokenMessages>(aMessage.Function()), aMessage); 
    iStatus = KRequestPending;
    SetActive();    

    if (aMessage.Function() == EExportKey) 
        {
        iServer.ExportKey(objectId, temp, iStatus);
        }    
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::ExportPublicL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreConduit::ExportPublicL(const RMessage2& aMessage)
    {
    TInt objectId = aMessage.Int0();
    TInt bufLen = User::LeaveIfError(aMessage.GetDesMaxLength(1));

    HBufC8* exportBuf = HBufC8::NewMaxLC(bufLen);
    TPtr8 ptr(exportBuf->Des());
    ptr.FillZ();
    iServer.ExportPublicL(objectId, ptr);
    aMessage.WriteL(1, ptr);

    CleanupStack::PopAndDestroy(exportBuf);
    aMessage.Complete(KErrNone);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::DeleteKeyL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreConduit::DeleteKeyL(const RMessage2& aMessage)
    {
    TInt objectId = aMessage.Int0();
    iServer.DeleteKeyL(objectId);
    aMessage.Complete(KErrNone);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::SetUsePolicyL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreConduit::SetUsePolicyL(const RMessage2& aMessage)
    {
    TInt objectId = aMessage.Int0();
    TSecurityPolicyBuf policyBuf;

    aMessage.ReadL(1, policyBuf);

    iServer.SetUsePolicyL(objectId, policyBuf());
    aMessage.Complete(KErrNone);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::SetManagementPolicyL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreConduit::SetManagementPolicyL(const RMessage2& aMessage)
    {
    TInt objectId = aMessage.Int0();
    TSecurityPolicyBuf policyBuf;

    aMessage.ReadL(1, policyBuf);

    iServer.SetManagementPolicyL(objectId, policyBuf());
    aMessage.Complete(KErrNone);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::OpenKeyL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreConduit::OpenKeyL(const RMessage2& aMessage, 
                                       CDevCertKeyStoreSession& aSession, 
                                       TUid aType)
    { 
    CDevCertOpenedKeySrv* openedKey = iServer.OpenKeyL(aMessage.Int0(), aType);
    CleanupStack::PushL(openedKey);

    const TDesC& label = openedKey->Label();

    TInt writeBufLen = User::LeaveIfError(aMessage.GetDesMaxLength(3));
    TInt reqdLength = label.Length();
    if (writeBufLen < reqdLength)
        {
        // We're writing into a TDes16 so we can't use TPckg<TInt>
        TPtrC sizePtr(reinterpret_cast<TUint16*>(&reqdLength), 2);
        aMessage.WriteL(3, sizePtr);
        User::Leave(KErrOverflow);
        }

    aMessage.WriteL(3, label);

    TInt handle = aSession.AddOpenedKeyL(*openedKey);

    TPckg<TInt> handlePckg(handle);
    TRAPD(err, aMessage.WriteL(1, handlePckg));

    if (err != KErrNone)
        {
        aSession.RemoveOpenedKeyL(handle);
        User::Leave(err);
        }

    CleanupStack::Pop(openedKey); // now owned by session
    aMessage.Complete(KErrNone);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::RepudiableRSASignL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreConduit::RepudiableRSASignL(const RMessage2& aMessage, 
                                         CDevCertKeyStoreSession& aSession)
    {
    ASSERT( iOpenedKey == NULL );
    TPtr8 thePtr(0,0);
    iOpenedKey = ProcessL(aMessage, aSession, KRSARepudiableSignerUID, ERepudiableRSASign, thePtr);
    static_cast<CDevCertRSARepudiableSignerSrv*>(iOpenedKey)->Sign(thePtr, iRSASignature, iStatus);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::CancelRSASign()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreConduit::CancelRSASign(const RMessage2& aMessage)
    {
    if (iCurrentRequest.OutstandingRequest() == ERepudiableRSASign)
        {
        Cancel();
        }
    aMessage.Complete(KErrNone);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::RepudiableDSASignL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreConduit::RepudiableDSASignL(const RMessage2& aMessage, 
                                          CDevCertKeyStoreSession& aSession)
    {
    ASSERT( iOpenedKey == NULL );
    TPtr8 thePtr(0,0);
    iOpenedKey = ProcessL(aMessage, aSession, KDSARepudiableSignerUID, ERepudiableDSASign, thePtr);
    static_cast<CDevCertDSARepudiableSignerSrv*>(iOpenedKey)->Sign(thePtr, iDSASignature, iStatus);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::CancelDSASign()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreConduit::CancelDSASign(const RMessage2& aMessage)
    {
    if (iCurrentRequest.OutstandingRequest() == ERepudiableDSASign)
        {
        Cancel();
        }
    aMessage.Complete(KErrNone);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::DecryptL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreConduit::DecryptL(const RMessage2& aMessage, 
                                       CDevCertKeyStoreSession& aSession)
    {
    ASSERT( iOpenedKey == NULL );
    TPtr8 thePtr(0,0);
    iOpenedKey = ProcessL(aMessage, aSession, KPrivateDecryptorUID, EDecryptText, thePtr);
    static_cast<CDevCertRSADecryptorSrv*>(iOpenedKey)->Decrypt(thePtr, iPlaintext, iStatus);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::CancelDecrypt()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreConduit::CancelDecrypt(const RMessage2& aMessage)
    {
    if (iCurrentRequest.OutstandingRequest() == EDecryptText)
        {
        Cancel();
        }
    aMessage.Complete(KErrNone);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::ProcessL()
// ---------------------------------------------------------------------------
//
CDevCertOpenedKeySrv* CDevCertKeyStoreConduit::ProcessL(
                     const RMessage2& aMessage,
                     CDevCertKeyStoreSession& aSession,
                     const TUid& aCKeyInfoID,
                     const TDevTokenMessages& aState,
                     TPtr8& aTextPtr)
    {
    CDevCertOpenedKeySrv* object = aSession.OpenedKey(aMessage.Int0());
    if (!object)
        {
        User::Leave(KErrNotFound);
        }

    if (aCKeyInfoID != object->Type())
        {
        User::Leave(KErrAccessDenied);
        }

    TInt length = User::LeaveIfError(aMessage.GetDesLength(1));
    iText = HBufC8::NewL(length);
    aTextPtr.Set(iText->Des());
    aMessage.ReadL(1, aTextPtr);

    iCurrentRequest.Set(aState, aMessage);
    iStatus=KRequestPending;
    SetActive();

    return object;
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::DHPublicKeyL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreConduit::DHPublicKeyL(const RMessage2& aMessage, 
                                      CDevCertKeyStoreSession& aSession)
    {
    // 0: Object id
    // 1: DH paramters

    ASSERT(iDHParams == NULL );
    ASSERT(iOpenedKey == NULL );
    TRAPD(err, DoDHPublicKeyL(aMessage, aSession));
    if (err != KErrNone)
        {
        iOpenedKey = NULL;
        delete iDHParams;
        iDHParams = NULL;
        User::Leave(err);
        }
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::DoDHPublicKeyL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreConduit::DoDHPublicKeyL(const RMessage2& aMessage, 
                                      CDevCertKeyStoreSession& aSession)
    {
    iOpenedKey = aSession.OpenedKey(aMessage.Int0());
    if (!iOpenedKey)
        {
        User::Leave(KErrNotFound);
        }

    //Check that this is a DH object
    if (KKeyAgreementUID != iOpenedKey->Type())
        {
        iOpenedKey = NULL;
        User::Leave(KErrAccessDenied);
        }

    HBufC8* clientBuf = HBufC8::NewMaxLC(User::LeaveIfError(aMessage.GetDesLength(1)));
    TPtr8 clientPtr = clientBuf->Des();
    aMessage.ReadL(1, clientPtr);
    DevTokenDataMarshaller::ReadL(*clientBuf, iDHParams);
    CleanupStack::PopAndDestroy(clientBuf);

    static_cast<CDevCertDHAgreementSrv*>(iOpenedKey)->PublicKey(*iDHParams, iDHPublicKeyOut, iStatus);

    iCurrentRequest.Set(EDHPublicKey, aMessage);
    iStatus=KRequestPending;
    SetActive();
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::FinishDHPublicKeyL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreConduit::FinishDHPublicKeyL()
    {
    // Client buffer must be big enough for result because it had the DH
    // parameters in it
    TInt reqdSize = DevTokenDataMarshaller::Size(iDHPublicKeyOut);
    ASSERT(reqdSize <= iCurrentRequest.Message().GetDesMaxLength(1));

    HBufC8* clientBuf = HBufC8::NewMaxLC(reqdSize);
    TPtr8 clientPtr = clientBuf->Des();
    DevTokenDataMarshaller::WriteL(iDHPublicKeyOut, clientPtr);
    iCurrentRequest.Message().WriteL(1, clientPtr);
    CleanupStack::PopAndDestroy(clientBuf);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::DHAgreeL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreConduit::DHAgreeL(const RMessage2& aMessage, CDevCertKeyStoreSession& aSession)
    {
    // 0: Object id
    // 1: DH public key

    ASSERT( iOpenedKey == NULL );
    ASSERT( iDHPublicKey == NULL );
    TRAPD(err, DoDHAgreeL(aMessage, aSession));
    if (err != KErrNone)
        {
        iOpenedKey = NULL;
        delete iDHPublicKey;
        iDHPublicKey = NULL;
        User::Leave(err);
        }
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::DoDHAgreeL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreConduit::DoDHAgreeL(const RMessage2& aMessage, CDevCertKeyStoreSession& aSession)
    {
    CDevCertOpenedKeySrv* iOpenedKey = aSession.OpenedKey(aMessage.Int0());
    if (!iOpenedKey)
        {
        User::Leave(KErrNotFound);
        }

    //Check that this is a DH object
    if (KKeyAgreementUID != iOpenedKey->Type())
        {
        iOpenedKey = NULL;
        User::Leave(KErrAccessDenied);
        }

    HBufC8* clientBuf = HBufC8::NewMaxLC(User::LeaveIfError(aMessage.GetDesLength(1)));
    TPtr8 clientPtr = clientBuf->Des();
    aMessage.ReadL(1, clientPtr);
    DevTokenDataMarshaller::ReadL(*clientBuf, iDHPublicKey);
    CleanupStack::PopAndDestroy(clientBuf);

    static_cast<CDevCertDHAgreementSrv*>(iOpenedKey)->Agree(*iDHPublicKey, iDHAgreedKeyOut, iStatus);

    iCurrentRequest.Set(EDHAgree, aMessage);
    iStatus=KRequestPending;
    SetActive();
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::FinishDHAgreeL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreConduit::FinishDHAgreeL()
    {
    // Client buffer must be big enough for result because it had the DH
    // public key in it
    ASSERT(iDHAgreedKeyOut->Length() <= iCurrentRequest.Message().GetDesMaxLength(1));

    iCurrentRequest.Message().WriteL(1, *iDHAgreedKeyOut);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::CancelDH()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreConduit::CancelDH(const RMessage2& aMessage)
    {
    if (iCurrentRequest.OutstandingRequest() == EDHPublicKey ||
    iCurrentRequest.OutstandingRequest() == EDHAgree)
        {
        Cancel();
        }
    aMessage.Complete(KErrNone);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::CloseObjectL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreConduit::CloseObjectL(const RMessage2& aMessage, CDevCertKeyStoreSession& aSession)
    {
    aSession.RemoveOpenedKeyL(aMessage.Int0());
    aMessage.Complete(KErrNone);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::DoCancel()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreConduit::DoCancel()
    {
    switch (iCurrentRequest.OutstandingRequest())
        {
        case ECreateKey:
            iServer.CancelCreateKey();
            break;

        case ERepudiableRSASign:
        case ERepudiableDSASign:
        case EDecryptText:
        case EDHAgree:
        case EDHPublicKey:
            if (iOpenedKey)
                {       
                iOpenedKey->Cancel();
                }
            break;

        case EImportKey:
            iServer.CancelImportKey();
            break;

        case EExportKey:
            iServer.CancelExportKey();
            break;

        default:
            // Nothing to do
            break;
        }

    Cleanup();
    iCurrentRequest.Cancel();
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::RunL()
// The token interface has completed the request - munge any return parameters
// and write back to client.
// ---------------------------------------------------------------------------
// 
void CDevCertKeyStoreConduit::RunL()
    {
    // Handle errors from server
    User::LeaveIfError(iStatus.Int());

    switch (iCurrentRequest.OutstandingRequest())
        {
        case ECreateKey:
        case EImportKey:
        case EImportEncryptedKey:
            {
            //  Marshal TKeyUpdate to client - the client's buffer will be large
            //  enough as it passed us a CCTKeyInfo in the first place
            ASSERT(iKeyInfo);
            TDevTokenKeyUpdate update;
            update.iReference = iKeyInfo->HandleID();
            update.iId = iKeyInfo->ID();
            update.iSize = iKeyInfo->Size();
            update.iAlgorithm = iKeyInfo->Algorithm();
            TPckg<TDevTokenKeyUpdate> pckg(update);
            iCurrentRequest.Message().WriteL(1, pckg);
            break;
            }
        case EExportKey:
            {
            ASSERT(iExportBuf);
            TPtr8 clientPtr(iExportBuf->Des());
            iCurrentRequest.Message().WriteL(1, clientPtr);
            break;
            }
        case ERepudiableDSASign:
            {
            ASSERT(iDSASignature);

            TInt length = User::LeaveIfError(iCurrentRequest.Message().GetDesMaxLength(2));
            TInt reqdLength = DevTokenDataMarshaller::Size(*iDSASignature);
            if (length < reqdLength)
                {
                User::Leave(KErrOverflow);
                }

            HBufC8* clientBuffer = HBufC8::NewLC(reqdLength);
            TPtr8 ptr(clientBuffer->Des());
            DevTokenDataMarshaller::WriteL(*iDSASignature, ptr);
            iCurrentRequest.Message().WriteL(2, ptr);
            CleanupStack::PopAndDestroy(clientBuffer);
            break;
            }
        case ERepudiableRSASign:
            {
            ASSERT(iRSASignature);
            TInt length = User::LeaveIfError(iCurrentRequest.Message().GetDesMaxLength(2));
            TInt reqdLength = DevTokenDataMarshaller::Size(*iRSASignature);
            if (length < reqdLength)
                {
                User::Leave(KErrOverflow);
                }

            HBufC8* clientBuffer = HBufC8::NewLC(reqdLength);
            TPtr8 ptr(clientBuffer->Des());
            DevTokenDataMarshaller::WriteL(*iRSASignature, ptr);
            iCurrentRequest.Message().WriteL(2, ptr);
            CleanupStack::PopAndDestroy(clientBuffer);
            break;
            }
        case EDecryptText:
            {
            ASSERT(iPlaintext);
            TInt length = User::LeaveIfError(iCurrentRequest.Message().GetDesMaxLength(2));
            if (length < iPlaintext->Length())
                {
                User::Leave(KErrOverflow);
                }
            iCurrentRequest.Message().WriteL(2, *iPlaintext);
            break;
            }

        case EDHPublicKey:
            FinishDHPublicKeyL();
            break;
        case EDHAgree:
            FinishDHAgreeL();
            break;

        default:
            __ASSERT_DEBUG(EFalse, PanicServer(EPanicInvalidRequest));
            User::Leave(KErrNotSupported);
        }

    Cleanup();
    iCurrentRequest.Complete(KErrNone);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::Cleanup()
// Cleans up data members used in processing a client request.  Called whenenver a
// request is completed, either from RunL, RunError or indirectly from DoCancel.
// ---------------------------------------------------------------------------
// 
void CDevCertKeyStoreConduit::Cleanup()
    {
    delete iKeyInfo;     iKeyInfo = NULL;
    delete iImportKey;     iImportKey = NULL;
    delete iText;        iText = NULL;
    delete iDSASignature;  iDSASignature = NULL;
    delete iRSASignature;  iRSASignature = NULL;
    delete iExportBuf;     iExportBuf = NULL;
    delete iDHParams;    iDHParams = NULL;
    iDHPublicKeyOut.Close();
    delete iDHPublicKey;   iDHPublicKey = NULL;
    delete iDHAgreedKeyOut;  iDHAgreedKeyOut = NULL;
    delete iPlaintext;     iPlaintext = NULL;
    iOpenedKey = NULL;
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::RunError()
// ---------------------------------------------------------------------------
// 
TInt CDevCertKeyStoreConduit::RunError(TInt aError)
    {
    Cleanup();
    //  Handle error by completing client appropriately
    iCurrentRequest.Complete(aError);
    return (KErrNone);
    }


// CDevCertKeyStoreConduit::TAsyncRequest 

// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::TAsyncRequest::TAsyncRequest()
// ---------------------------------------------------------------------------
// 
CDevCertKeyStoreConduit::TAsyncRequest::TAsyncRequest(TRequestStatus& aStatus) :
  iOwnerStatus(aStatus)
    {
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::TAsyncRequest::~TAsyncRequest()
// ---------------------------------------------------------------------------
// 
CDevCertKeyStoreConduit::TAsyncRequest::~TAsyncRequest()
    {
    __ASSERT_DEBUG(EIdle==iRequest, PanicServer(EPanicRequestOutstanding));
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::TAsyncRequest::Set()
// ---------------------------------------------------------------------------
// 
void CDevCertKeyStoreConduit::TAsyncRequest::Set(TDevTokenMessages aRequest, const RMessage2& aMessage)
    {
    __ASSERT_DEBUG(EIdle==iRequest, PanicServer(EPanicRequestOutstanding));

    iOwnerStatus = KRequestPending;
    iRequest = aRequest;
    iMessage = aMessage;
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::TAsyncRequest::Complete()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreConduit::TAsyncRequest::Complete(TInt aResult)
    {
    iMessage.Complete(aResult);
    iRequest = EIdle;
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreConduit::TAsyncRequest::Cancel()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreConduit::TAsyncRequest::Cancel()
    {// Complete outstanding request with KErrCancel
    iMessage.Complete(KErrCancel);
    iRequest = EIdle;
    }
    
//EOF

