/*
* Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Implementation of DevCertKeyStore
*
*/



#include <asymmetrickeys.h>
#include <e32debug.h>
#include <mctkeystoreuids.h>            // KInterfaceKeyStore
#include "DevCertKeyStore.h"
#include "DevToken.h"
#include "DevCertOpenedKeys.h"
#include "DevTokenUtils.h"
#include "DevTokenImplementationUID.hrh"
#include "DevTokenMarshaller.h"
#include "DevTokenCliServ.h"
#include "DevTokenKeyInfoArray.h" //TDevTokenKeyInfoArray
#include "DevtokenLog.h"


// When retrieving data from the server we sometimes make a guess at the size of
// the data that will be returned so we can allocate a buffer for it.  If it's
// not big enough, the server tells us the required size and we retry.  These
// values are the initial guesses at buffer size:

const TInt KInitialBufSizeList = 1024;
const TInt KInitialBufSizeOpen = 64;

// ======== MEMBER FUNCTIONS ========

// -----------------------------------------------------------------------------
// CDevCertKeyStore::NewKeyStoreUserInterfaceL()
// -----------------------------------------------------------------------------
//
MCTTokenInterface* CDevCertKeyStore::NewKeyStoreUserInterfaceL(
                            MCTToken& aToken, 
                            RDevTokenClientSession& aClient)
    {
    //  Destroyed by MCTTokenInterface::DoRelease() 
    CDevCertKeyStore* me = new (ELeave) CDevCertKeyStore(KInterfaceKeyStore, 
                                             aToken, 
                                             aClient);                   
    CleanupStack::PushL(me);
    me->ConstructL();
    CleanupStack::Pop(me);
    return (me);
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::NewKeyStoreManagerInterfaceL()
// -----------------------------------------------------------------------------
//
MCTTokenInterface* CDevCertKeyStore::NewKeyStoreManagerInterfaceL(MCTToken& aToken, RDevTokenClientSession& aClient)
    {
    //  Destroyed by MCTTokenInterface::DoRelease()
    TRACE_PRINT("CDevCertKeyStore::NewKeyStoreManagerInterfaceL -->1")
    
    CDevCertKeyStore* me = new (ELeave) CDevCertKeyStore(KInterfaceKeyStoreManager, 
                                                         aToken, 
                                                         aClient);
    CleanupStack::PushL(me);
    me->ConstructL();
    TRACE_PRINT("CDevCertKeyStore::NewKeyStoreManagerInterfaceL -->2")
    CleanupStack::Pop(me);
    return (me);
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::CDevCertKeyStore()
// -----------------------------------------------------------------------------
//
CDevCertKeyStore::CDevCertKeyStore(TInt aUID, MCTToken& aToken, RDevTokenClientSession& aClient)
    : CDTClient(aUID, aToken, aClient),
    // Reference count starts at one as we're always created and 
    // returned by a GetInterface() call on the token
    iRefCount(1)
    {
    TRACE_PRINT("CDevCertKeyStore::CDevCertKeyStore: keystore client interface created");
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::ConstructL()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::ConstructL()
    {
    TRACE_PRINT("CDevCertKeyStore::ConstructL -->1")
    CActiveScheduler::Add(this);
    TRACE_PRINT("CDevCertKeyStore::ConstructL -->2")
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::~CDevCertKeyStore()
// -----------------------------------------------------------------------------
//
CDevCertKeyStore::~CDevCertKeyStore()
    {
    Cleanup();
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::Token()
// -----------------------------------------------------------------------------
//
MCTToken& CDevCertKeyStore::Token()
    {
    return iToken;
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::AddRef()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::AddRef()
    {
    ++iRefCount;
    iToken.AddRef();
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::DoRelease()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::DoRelease()
    {
    --iRefCount;
    ASSERT( iRefCount >= 0 );
    if ( iRefCount == 0 )
        {
        MCTTokenInterface::DoRelease();
        }
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::ReleaseObject()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::ReleaseObject(const TCTTokenObjectHandle& aHandle)
    {
    iClientSession.SendRequest(ECloseObject, TIpcArgs(aHandle.iObjectId));
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::FreeKeyBuffer()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::FreeKeyBuffer()
    {
    delete iKey; 
    iKey = NULL;
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::AllocKeyBuffer()
// -----------------------------------------------------------------------------
//
TInt CDevCertKeyStore::AllocKeyBuffer(TInt aReqdSize)
    {
    ASSERT(aReqdSize > 0);
    
    TInt result = KErrNoMemory;
    FreeKeyBuffer();
    iKey = HBufC8::NewMax(aReqdSize);
    if ( iKey )
        {
        TPtr8 thePtr(iKey->Des());
        thePtr.FillZ();
        result = KErrNone;
        }
    return result;
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::RunL()
// Called when the server completes an asynchronous request.  Unmarshalls the
// data from the server and passes it back to the caller.
// -----------------------------------------------------------------------------
// 
void CDevCertKeyStore::RunL()
    {
    User::LeaveIfError(iStatus.Int());
    
    switch ( iCurrentRequest.OutstandingRequest() )
        {
        case ECreateKey:
        case EImportKey:
        case EImportEncryptedKey:
        // Unmarshal the created key data and create CCTKeyInfo 
        // to pass back to caller
            UpdateKey();
            break;

        case EExportKey:
        case EDHAgree:
            {
            *iClientPtr.iBuffer = HBufC8::NewL(iRequestPtr.Length());
            TPtr8 ptr((*iClientPtr.iBuffer)->Des());
            ptr.FillZ();
            ptr.Copy(iRequestPtr);
            break;
            }
        case ERepudiableDSASign:
            DevTokenDataMarshaller::ReadL(iRequestPtr, *iClientPtr.iDSASignature);
            break;

        case ERepudiableRSASign:
            DevTokenDataMarshaller::ReadL(iRequestPtr, *iClientPtr.iRSASignature);
            break;

        case EDecryptText:
           iClientPtr.iDecryptedText->Copy(iRequestPtr);   
           break;

        case EDHPublicKey:
           {
           ASSERT(iDHParams);
           RInteger n = iDHParams->TakeN();
           CleanupStack::PushL(n);
           RInteger g = iDHParams->TakeG();
           CleanupStack::PushL(g);
           RInteger X;
           DevTokenDataMarshaller::ReadL(iRequestPtr, X);
           CleanupStack::PushL(X);
           *iClientPtr.iDHPublicKey = CDHPublicKey::NewL(n, g, X);
           CleanupStack::Pop(3); // X, g, n
           break;
           }

        default:
            __ASSERT_DEBUG(EFalse, DevTokenPanic(EInvalidRequest));
        }
    Cleanup();
    iCurrentRequest.Complete(KErrNone); 
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::Cleanup()
// Clean up internal state
// -----------------------------------------------------------------------------
// 
void CDevCertKeyStore::Cleanup()
    {
    FreeRequestBuffer();
    FreeKeyBuffer();

    // Zero client pointers
    iClientPtr.iAny = NULL;
    
    delete iDHParams;
    iDHParams = NULL;
    
    delete iText;
    iText=NULL;
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::RunError()
// -----------------------------------------------------------------------------
// 
TInt CDevCertKeyStore::RunError(TInt aError)
    {
    Cleanup();
    return CDTClient::RunError(aError);
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::DoCancel()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::DoCancel()
    {
    TDevTokenMessages message = EIdle;

    switch (iCurrentRequest.OutstandingRequest())
        {
        case ECreateKey:
            message = ECancelCreateKey;
            break;

        case ERepudiableRSASign:
            message = ECancelRSASign;
            break;
            
        case ERepudiableDSASign:
            message = ECancelDSASign;
            break;
            
        case EDecryptText:
            message = ECancelDecrypt;
            break;
      
        case EDHPublicKey:
        case EDHAgree:
            message = ECancelDH;
            break;      

        case EImportKey:
            message = ECancelImportKey;
            break;
        
        case EImportEncryptedKey:
			message = ECancelImportEncryptedKey;
			break;
        
        case EExportKey:
            message = ECancelExportKey;
            break;

        default:
            // Nothing to do
            break;
        }

    if ( message != EIdle )
        {
        iClientSession.SendRequest(message, TIpcArgs());
        }

    Cleanup();  
    CDTClient::DoCancel();  
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::List()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::List(RMPointerArray<CCTKeyInfo>& aKeys, const TCTKeyAttributeFilter& aFilter, TRequestStatus& aStatus)
    {
    TDevTokenKeyInfoArray array(aKeys);
    TRAPD(err, DoListL(aFilter, array));
    FreeRequestBuffer();
    TRequestStatus* status = &aStatus;
    User::RequestComplete(status, err); 
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::DoList()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::DoListL(const TCTKeyAttributeFilter& aFilter, MDevTokenKeyInfoArray& aOut)
    {
    //ASSERT(aOut.Count() == 0);
    TPckg<TCTKeyAttributeFilter> filterPckg(aFilter);
    SendSyncRequestAndHandleOverflowL(EListKeys, KInitialBufSizeList, TIpcArgs(&filterPckg, 0, &iRequestPtr));
    
    CleanupClosePushL(aOut);
    DevTokenDataMarshaller::ReadL(iRequestPtr, iToken, aOut);

    CleanupStack::Pop(&aOut);
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::CancelList()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::CancelList()
    {
    // synchronous, nothing to do
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::GetKeyInfo()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::GetKeyInfo(TCTTokenObjectHandle aHandle, CCTKeyInfo*& aInfo, 
                   TRequestStatus& aStatus)
    {
    TRAPD(err, DoGetKeyInfoL(aHandle, aInfo));
    FreeRequestBuffer();
    TRequestStatus* status = &aStatus;
    User::RequestComplete(status, err);
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::DoGetKeyInfoL()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::DoGetKeyInfoL(TCTTokenObjectHandle aHandle, 
                                     CCTKeyInfo*& aInfo)
    {
    SendSyncRequestAndHandleOverflowL(EGetKeyInfo, KInitialBufSizeList, TIpcArgs(aHandle.iObjectId, 0, &iRequestPtr));
    DevTokenDataMarshaller::ReadL(iRequestPtr, iToken, aInfo);
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::CancelGetKeyInfo()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::CancelGetKeyInfo()
    {
    // synchronous, nothing to do
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::Open()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::Open(const TCTTokenObjectHandle& aHandle, 
               MRSASigner*& aSigner, 
               TRequestStatus& aStatus)
    {
    CDevCertRSARepudiableSigner* signer = CDevCertRSARepudiableSigner::New(this);
    
    TInt err = DoOpenKey(EOpenKeyRepudiableRSASign, aHandle, signer);
    if ( err == KErrNone )
        {
        aSigner = signer;
        }
    
    TRequestStatus* status = &aStatus;
    User::RequestComplete(status, err);
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::Open()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::Open(const TCTTokenObjectHandle& aHandle, 
               MCTSigner<CDSASignature*>*& aSigner, 
               TRequestStatus& aStatus)
    { 
    CDevCertDSARepudiableSigner* signer = CDevCertDSARepudiableSigner::New(this);
    TInt err = DoOpenKey(EOpenKeyRepudiableDSASign, aHandle, signer);
    if ( err == KErrNone )
        {
        aSigner = signer;
        }
    TRequestStatus* status = &aStatus;
    User::RequestComplete(status, err);
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::Open()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::Open(const TCTTokenObjectHandle& aHandle, 
               MCTDecryptor*& aDecryptor,
               TRequestStatus& aStatus)
    {
    CDevCertRSADecryptor* decryptor = CDevCertRSADecryptor::New(this);
    TInt err = DoOpenKey(EOpenKeyDecrypt, aHandle, decryptor);
    if ( err == KErrNone )
        {
        aDecryptor = decryptor;
        }
    TRequestStatus* status = &aStatus;
    User::RequestComplete(status, err);
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::Open()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::Open(const TCTTokenObjectHandle& aHandle, 
               MCTDH*& aDH, TRequestStatus& aStatus)
    {
    CDevCertDHAgreement* dh = CDevCertDHAgreement::New(this);
    TInt err = DoOpenKey(EOpenKeyAgree, aHandle, dh);
    if ( err == KErrNone )
        {
        aDH = dh;
        }
    TRequestStatus* status = &aStatus;
    User::RequestComplete(status, err);
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::DoOpenKey()
// -----------------------------------------------------------------------------
//
TInt CDevCertKeyStore::DoOpenKey(TDevTokenMessages aMessage,
                  const TCTTokenObjectHandle& aHandle,
                  CDevCertOpenedKey* aOpenedKey)
    {
    if ( !aOpenedKey )
        {
        return KErrNoMemory;
        }
    
    // This is similar to SendSyncRequestAndHandleOverflowL, 
    //but we're returning data in iLabel
    TInt handle;
    TPckg<TInt> handlePckg(handle);

    HBufC* label = HBufC::NewMax(KInitialBufSizeOpen);
    if ( !label )
        {
        aOpenedKey->Release();
        return KErrNoMemory;
        }
    
    TPtr labelPtr(label->Des());
    
    TIpcArgs args(aHandle.iObjectId, &handlePckg, 0, &labelPtr);
    
    TInt err = iClientSession.SendRequest(aMessage, args);
    if ( err == KErrOverflow )
        {
        TInt sizeReqd = 0;
        TPckg<TInt> theSize(sizeReqd);
        TPtrC8 sizePtr(reinterpret_cast<const TUint8*>(label->Ptr()), 4);
        theSize.Copy(sizePtr);  
        delete label;
        label = HBufC::NewMax(sizeReqd);
        if ( label )
            {
            labelPtr.Set(label->Des());   
            err = iClientSession.SendRequest(aMessage, args);
            }
        else
            {
            err = KErrNoMemory;
            }
        }

    if ( err != KErrNone )
        {
        aOpenedKey->Release();
        delete label;
        return err;
        }
    
    aOpenedKey->SetObjectID(handle);
    aOpenedKey->SetLabel(label);
    return KErrNone;
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::CancelOpen()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::CancelOpen()
    {
    // synchronous, nothing to do
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::GetKeySizeL()
// -----------------------------------------------------------------------------
//
TUint CDevCertKeyStore::GetKeySizeL(TInt aObjectId)
    {
    // Get the key size
    TInt result = iClientSession.SendRequest(EGetKeyLength, TIpcArgs(aObjectId, &iRequestPtr));
    User::LeaveIfError(result);
    return result;
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::GetKeySizeL()
// Returns the public key in DER-encoded ASN-1 
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::ExportPublic(const TCTTokenObjectHandle& aHandle,
                   HBufC8*& aPublicKey,
                   TRequestStatus& aStatus)
    {
    TRAPD(err, DoExportPublicL(aHandle, aPublicKey));
    FreeRequestBuffer();
    TRequestStatus* status = &aStatus;
    User::RequestComplete(status, err);
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::DoExportPublicL()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::DoExportPublicL(const TCTTokenObjectHandle& aHandle, 
                                       HBufC8*& aPublicKey)
    {
    // The size of the exported package is always less than keysize in bytes * 4 
    // DSA generates packages twice as big as RSA
    TUint bufSize = GetKeySizeL(aHandle.iObjectId) / 2;
    SendSyncRequestAndHandleOverflowL(EExportPublic, bufSize, TIpcArgs(aHandle.iObjectId, &iRequestPtr));
    aPublicKey = HBufC8::NewL(iRequestPtr.MaxLength());
    TPtr8 ptr = aPublicKey->Des();
    ptr.Copy(iRequestPtr);
    } 


// -----------------------------------------------------------------------------
// CDevCertKeyStore::CancelExportPublic()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::CancelExportPublic()
    {
    // synchronous, nothing to do
    }


//  MCTKeyStoreManager
// -----------------------------------------------------------------------------
// CDevCertKeyStore::CreateKey()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::CreateKey(CCTKeyInfo*& aReturnedKey, TRequestStatus& aStatus)
    {
    //  [in, out] CCTKeyInfo, caller fills with data required to create the key, 
    //  If request succeeds, iId and iHandle members are filled in by server
    TInt r = MarshalKeyInfo(*aReturnedKey);
    if ( KErrNone!=r )
        {
        TRequestStatus* stat = &aStatus;
        User::RequestComplete(stat, r);
        return;
        }
    //  Store CCTKeyInfo to write into later (server will put extra data into it)
    iClientPtr.iKeyInfo = &aReturnedKey;
    SetActive();
    iCurrentRequest(ECreateKey, &aStatus);
    iClientSession.SendAsyncRequest(ECreateKey, TIpcArgs(0, &iRequestPtr), &iStatus);   
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::CancelCreateKey()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::CancelCreateKey()
    {
    if ( iCurrentRequest.OutstandingRequest() == ECreateKey )
        {
        Cancel();
        }
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::ImportKey()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::ImportKey(const TDesC8& aKey, CCTKeyInfo*& aReturnedKey, TRequestStatus& aStatus)
    {
    DoImportKey(EImportKey, aKey, aReturnedKey, aStatus);
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::CancelImportKey()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::CancelImportKey()
    {
    if ( iCurrentRequest.OutstandingRequest() == EImportKey )
        {
        Cancel();
        }
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::ImportEncryptedKey()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::ImportEncryptedKey(const TDesC8& aKey, 
                                          CCTKeyInfo*& aReturnedKey, 
                                          TRequestStatus& aStatus)
    {
    DoImportKey(EImportEncryptedKey, aKey, aReturnedKey, aStatus);
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::CancelImportEncryptedKey()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::CancelImportEncryptedKey()
    {
    if (iCurrentRequest.OutstandingRequest() == EImportEncryptedKey)
		{
		Cancel();
		}
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::DoImportKey()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::DoImportKey(TDevTokenMessages aMessage, 
                                   const TDesC8& aKey, 
                                   CCTKeyInfo*& aReturnedKey, 
                                   TRequestStatus& aStatus)
    { 
    // [in] CCTKeyInfo, caller fills with data required to create the key,
    // [out] TKeyUpdate, key iId and iHandle members from the server
    TInt r = AllocKeyBuffer(aKey.Size());
    if ( KErrNone!=r )
        {
        TRequestStatus* stat = &aStatus;
        User::RequestComplete(stat, r);
        return;
        }

    TPtr8 keyPtr(iKey->Des());
    keyPtr.Copy(aKey);
    
    r = MarshalKeyInfo(*aReturnedKey);
    if ( KErrNone!=r )
        {
        FreeKeyBuffer();
        TRequestStatus* stat = &aStatus;
        User::RequestComplete(stat, r);
        return;
        }

    iClientPtr.iKeyInfo = &aReturnedKey;
    SetActive();
    iCurrentRequest(aMessage, &aStatus);
    iClientSession.SendAsyncRequest(aMessage, TIpcArgs(iKey, &iRequestPtr), &iStatus);
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::MarshalKeyInfo()
// -----------------------------------------------------------------------------
//
TInt CDevCertKeyStore::MarshalKeyInfo(CCTKeyInfo& aKey)
    {
    TInt r = AllocRequestBuffer(DevTokenDataMarshaller::Size(aKey));
    if ( KErrNone==r )  //  OOM or some catastrophe, no point continuing
        {
        DevTokenDataMarshaller::Write(aKey, iRequestPtr);
        }
    
    return (r);
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::ExportKey()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::ExportKey(TCTTokenObjectHandle aHandle, 
                                 HBufC8*& aKey, 
                                 TRequestStatus& aStatus)
    {
    TUint keySize=0;
    TRAPD(r, keySize = GetKeySizeL(aHandle.iObjectId));
    if ( KErrNone!=r )
        {
        TRequestStatus* stat = &aStatus;
        User::RequestComplete(stat, r);
        return; 
        }

    ASSERT(keySize);  

    iClientPtr.iBuffer = &aKey;
    iObjectId = aHandle.iObjectId;

    r = AllocRequestBuffer((keySize/8)*6);   
    // Heuristic, pkcs8 cleartext format is about 6 times the keysize in bytes...

    if ( KErrNone!=r )  //  OOM or some catastrophe, no point continuing
        {
        TRequestStatus* stat = &aStatus;
        User::RequestComplete(stat, r); 
        }
    else
        {     
        SetActive();
        iCurrentRequest(EExportKey, &aStatus);
        iClientSession.SendAsyncRequest(iCurrentRequest.OutstandingRequest(), TIpcArgs(iObjectId,  &iRequestPtr), &iStatus);    
        }
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::CancelExportKey()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::CancelExportKey()
    {
    if ( iCurrentRequest.OutstandingRequest() == EExportKey )
        {
        Cancel();
        }
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::ExportEncryptedKey()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::ExportEncryptedKey(TCTTokenObjectHandle /*aHandle*/, 
                                          const CPBEncryptParms& /*aParams*/,  
                                          HBufC8*& /*aKey*/, 
                                          TRequestStatus& aStatus)
    {
    TRequestStatus* status = &aStatus;
    User::RequestComplete( status, KErrNotSupported );
    }


// -----------------------------------------------------------------------------
// CDevCertKeyStore::CancelExportEncryptedKey()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::CancelExportEncryptedKey()
    {
    }


// -----------------------------------------------------------------------------
//  CDevCertKeyStore::RepudiableDSASign()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::RepudiableDSASign(const TCTTokenObjectHandle& aHandle,
                      const TDesC8& aDigest,
                      CDSASignature*& aSignature,
                      TRequestStatus& aStatus)
    { 
    if ( aDigest.Length() > KMaxDSADigestSize )
        {
        TRequestStatus* stat = &aStatus;
        User::RequestComplete(stat, KErrOverflow);  
        }
    else
        {// Calls the same DSA digest function server side for DSA
        iClientPtr.iDSASignature = &aSignature; 
        Process(aHandle,aDigest,ERepudiableDSASign,aStatus,KMaxDSASignatureSize);
        }
    }


// -----------------------------------------------------------------------------
//  CDevCertKeyStore::RepudiableRSASign()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::RepudiableRSASign(const TCTTokenObjectHandle& aHandle,
                      const TDesC8& aDigest,
                      CRSASignature*& aSignature,
                      TRequestStatus& aStatus)
    {
    if ( aDigest.Length() > KMaxRSADigestSize )
        {
        TRequestStatus* stat = &aStatus;
        User::RequestComplete(stat, KErrOverflow);  
        }
    else
        {
        iClientPtr.iRSASignature = &aSignature; 
        Process(aHandle,aDigest,ERepudiableRSASign,aStatus,KMaxRSASignatureSize);
        }
    }


// -----------------------------------------------------------------------------
//  CDevCertKeyStore::Process()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::Process(const TCTTokenObjectHandle& aHandle,
                const TDesC8& aText,
                TDevTokenMessages aMessage,
                TRequestStatus& aStatus, TInt aBuffSize)
    {
    TInt err = AllocRequestBuffer(aBuffSize); //Assuming the buf is large enough
    if (KErrNone!=err)  //  OOM or some catastrophe, no point continuing
        {
        TRequestStatus* stat = &aStatus;
        User::RequestComplete(stat, err);
        return;
        } 
    iObjectId = aHandle.iObjectId;  
    iText = aText.Alloc();
    if ( iText == NULL )
        {
        FreeRequestBuffer();
        TRequestStatus* stat = &aStatus;
        User::RequestComplete(stat, KErrNoMemory);
        return;
        }
    SetActive();
    iCurrentRequest(aMessage, &aStatus);
    iClientSession.SendAsyncRequest(aMessage, TIpcArgs(iObjectId, iText, &iRequestPtr), &iStatus);
    }


// -----------------------------------------------------------------------------
//  CDevCertKeyStore::Decrypt()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::Decrypt(const TCTTokenObjectHandle& aHandle,
                const TDesC8& aCiphertext,
                TDes8& aDecryptedText,
                TRequestStatus& aStatus)
    {
    iClientPtr.iDecryptedText = &aDecryptedText;
    Process(aHandle,aCiphertext,EDecryptText,aStatus, KMaxRSAPlaintextSize);
    }


// -----------------------------------------------------------------------------
//  CDevCertKeyStore::DHPublicKey()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::DHPublicKey(const TCTTokenObjectHandle& aHandle, const TInteger& aN, const TInteger& aG,
                  CDHPublicKey*& aX, TRequestStatus& aStatus)
    {
    TRAPD(err, DoDHPublicKeyL(aHandle, aN, aG, aX, aStatus));
    if ( err != KErrNone )
        {
        delete iDHParams;
        iDHParams = NULL;
        FreeRequestBuffer();
        TRequestStatus* status = &aStatus;
        User::RequestComplete(status, err);
        }
    }


// -----------------------------------------------------------------------------
//  CDevCertKeyStore::DoDHPublicKeyL()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::DoDHPublicKeyL(const TCTTokenObjectHandle& aHandle, 
                                      const TInteger& aN, const TInteger& aG,
                     CDHPublicKey*& aX, TRequestStatus& aStatus)
    {
    iDHParams = CDevTokenDHParams::NewL(aN, aG);
    
    TInt sizeReqd = DevTokenDataMarshaller::Size(*iDHParams);
    User::LeaveIfError(AllocRequestBuffer(sizeReqd));

    DevTokenDataMarshaller::WriteL(*iDHParams, iRequestPtr);
    
    iObjectId = aHandle.iObjectId;
    iClientPtr.iDHPublicKey = &aX;
    iCurrentRequest(EDHPublicKey, &aStatus);
    iClientSession.SendAsyncRequest(EDHPublicKey, TIpcArgs(iObjectId, &iRequestPtr), &iStatus); 
    SetActive();
    }


// -----------------------------------------------------------------------------
//  CDevCertKeyStore::DHAgree()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::DHAgree(const TCTTokenObjectHandle& aHandle, 
                               const CDHPublicKey& iY,
                 HBufC8*& aAgreedKey, TRequestStatus& aStatus)
    { 
    TInt sizeReqd = DevTokenDataMarshaller::Size(iY);
    TInt err = AllocRequestBuffer(sizeReqd);
    if ( err != KErrNone )
        {
        TRequestStatus* status = &aStatus;
        User::RequestComplete(status, err);
        return;
        }

    TRAP(err, DevTokenDataMarshaller::WriteL(iY, iRequestPtr));
    if ( err != KErrNone )
        {
        FreeRequestBuffer();
        TRequestStatus* status = &aStatus;
        User::RequestComplete(status, err);
        return;   
        }
  
    iObjectId = aHandle.iObjectId;
    iClientPtr.iBuffer = &aAgreedKey;
    iCurrentRequest(EDHAgree, &aStatus);
    iClientSession.SendAsyncRequest(EDHAgree, TIpcArgs(iObjectId, &iRequestPtr), &iStatus); 
    SetActive();
    }


// -----------------------------------------------------------------------------
//  CDevCertKeyStore::DeleteKey()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::DeleteKey(TCTTokenObjectHandle aHandle, TRequestStatus& aStatus)
    {
    TInt err = iClientSession.SendRequest(EDeleteKey, TIpcArgs(aHandle.iObjectId));
    TRequestStatus* status = &aStatus;
    User::RequestComplete(status, err);
    }


// -----------------------------------------------------------------------------
//  CDevCertKeyStore::CancelDeleteKey()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::CancelDeleteKey()
    {
    // synchronous, nothing to do
    }


// -----------------------------------------------------------------------------
//  CDevCertKeyStore::SetUsePolicy()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::SetUsePolicy(TCTTokenObjectHandle aHandle, 
                   const TSecurityPolicy& aPolicy,
                   TRequestStatus& aStatus)
    {
    TPckgC<TSecurityPolicy> policyPckg(aPolicy);
    TInt err = iClientSession.SendRequest(ESetUsePolicy, TIpcArgs(aHandle.iObjectId, &policyPckg));
    TRequestStatus* status = &aStatus;
    User::RequestComplete(status, err);
    }


// -----------------------------------------------------------------------------
//  CDevCertKeyStore::CancelSetUsePolicy()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::CancelSetUsePolicy()
    {
    // synchronous, nothing to do
    }


// -----------------------------------------------------------------------------
//  CDevCertKeyStore::SetManagementPolicy()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::SetManagementPolicy(TCTTokenObjectHandle aHandle, 
                      const TSecurityPolicy& aPolicy,
                      TRequestStatus& aStatus)
    {
    TPckgC<TSecurityPolicy> policyPckg(aPolicy);
    TInt err = iClientSession.SendRequest(ESetManagementPolicy, TIpcArgs(aHandle.iObjectId, &policyPckg));
    TRequestStatus* status = &aStatus;
    User::RequestComplete(status, err);
    }


// -----------------------------------------------------------------------------
//  CDevCertKeyStore::CancelSetManagementPolicy()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::CancelSetManagementPolicy()
    {
    // synchronous, nothing to do
    }


// -----------------------------------------------------------------------------
//  CDevCertKeyStore::UpdateKey()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::UpdateKey()
    {
    __ASSERT_DEBUG((iRequestPtr.Ptr()), DevTokenPanic(ENoDataMarshalled));
    CCTKeyInfo& keyInfo = **iClientPtr.iKeyInfo;

    // Set handle and key id returned from server
    TDevTokenKeyUpdate update;
    TPckg<TDevTokenKeyUpdate> pckg(update);
    pckg.Copy(iRequestPtr);
    keyInfo.SetHandle(update.iReference);
    keyInfo.SetIdentifier(update.iId);
    // Set size and algorithm - only strictly necessary for import
    keyInfo.SetSize(update.iSize);
    keyInfo.SetAlgorithm(update.iAlgorithm);
    }


// -----------------------------------------------------------------------------
//  CDevCertKeyStore::SetPassphraseTimeout()
// -----------------------------------------------------------------------------
//
void CDevCertKeyStore::SetPassphraseTimeout(TInt /*aTimeout*/, TRequestStatus& aStatus) 
    { 
    TRequestStatus* status = &aStatus;
    User::RequestComplete(status, KErrNone);
    }


// -----------------------------------------------------------------------------
//  CDevCertKeyStore::CancelSetPassphraseTimeout()
// -----------------------------------------------------------------------------
//    
void CDevCertKeyStore::CancelSetPassphraseTimeout()
    { 
    }


// -----------------------------------------------------------------------------
//  CDevCertKeyStore::Relock()
// -----------------------------------------------------------------------------
// 
void CDevCertKeyStore::Relock(TRequestStatus& aStatus )
    {
    TRequestStatus* status = &aStatus;
    User::RequestComplete(status, KErrNone);
    }


// -----------------------------------------------------------------------------
//  CDevCertKeyStore::CancelRelock()
// -----------------------------------------------------------------------------
//     
void CDevCertKeyStore::CancelRelock()
    { 
    }
//EOF

