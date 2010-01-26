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
* Description:   Implementation of DevCertKeyStoreServer
*
*/


#include <securityerr.h>
#include <asnpkcs.h>
#include <asn1enc.h>
#ifndef SYMBIAN_ENABLE_SPLIT_HEADERS
#include <x509keys.h>
#else
#include <x509keys.h>
#include <x509keyencoder.h>
#endif
#include <keyidentifierutil.h>
#include <mctauthobject.h>
#include <utf.h>

#include "DevCertKeyStoreServer.h"
#include "DevCertKeyStoreSession.h"
#include "DevCertKeyDataManager.h"
#include "DevCertKeyStoreConduit.h"
#include "DevCertKeyStoreSession.h"
#include "DevCertOpenedKeysSrv.h"
#include "DevTokenCliServ.h"
#include "DevCertCreateKey.h"
#include "DevCertOpenedKeys.h"
#include "DevTokenDataTypes.h"
#include "DevCertKeyStreamUtils.h"
#include "DevTokenUtil.h"
#include "DevCertKeyEncryptor.h"
#include "DevTokenPWManager.h"

// We don't currently allow any keys larger than 2048 bits.  It may be necessary to
// increase this limit in the future. 
const TUint KTheMinKeySize = 512;
const TUint KTheMaxKeySize = 2048;

// Security policies
_LIT_SECURITY_POLICY_C1(KImportRemoveSecurityPolicy, ECapabilityWriteDeviceData);
_LIT_SECURITY_POLICY_C1(KCreateSecurityPolicy, ECapabilityReadDeviceData);
_LIT_SECURITY_POLICY_C1(KOpenSecurityPolicy, ECapabilityReadDeviceData);
_LIT_SECURITY_POLICY_C1(KListSecurityPolicy, ECapabilityReadUserData);


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CDevCertKeyStoreServer::NewL()
// ---------------------------------------------------------------------------
// 
CDevCertKeyStoreServer* CDevCertKeyStoreServer::NewL()
    {
    CDevCertKeyStoreServer* me = new (ELeave) CDevCertKeyStoreServer();
    CleanupStack::PushL(me);
    me->ConstructL();
    CleanupStack::Pop(me);
    return (me);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreServer::CDevCertKeyStoreServer()
// ---------------------------------------------------------------------------
//
CDevCertKeyStoreServer::CDevCertKeyStoreServer() :
	CActive(EPriorityStandard),
	iAction(EIdle),
	iExportBuf(NULL, 0)
    {
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreServer::ConstructL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreServer::ConstructL()
    {
    iKeyDataManager = CDevCertKeyDataManager::NewL();
    iConduit = CDevCertKeyStoreConduit::NewL(*this);
    CActiveScheduler::Add(this);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreServer::~CDevCertKeyStoreServer()
// ---------------------------------------------------------------------------
//
CDevCertKeyStoreServer::~CDevCertKeyStoreServer()
    {
    Cancel();

    delete iKeyDataManager;
    delete iConduit;
    delete iKeyCreator;
    iSessions.Close();
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreServer::CreateSessionL()
// ---------------------------------------------------------------------------
//
CDevCertKeyStoreSession* CDevCertKeyStoreServer::CreateSessionL()
    {
    CDevCertKeyStoreSession* session = CDevCertKeyStoreSession::NewL(*this );
    CleanupStack::PushL(session);
    User::LeaveIfError(iSessions.Append(session));
    CleanupStack::Pop(session);
    return session;
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreServer::RemoveSession()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreServer::RemoveSession(CDevCertKeyStoreSession& aSession)
    {
    if (iSession == &aSession) // just in case
        {
        iSession = NULL; 
        }

    for (TInt index = 0 ; index < iSessions.Count() ; ++index)
        {
        if (iSessions[index] == &aSession)
            {
            iSessions.Remove(index);
            return;
            }
        }
    User::Invariant();
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreServer::ServiceRequestL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreServer::ServiceRequestL(const RMessage2& aMessage, CDevCertKeyStoreSession& aSession)
    {
    iMessage = &aMessage;
    iSession = &aSession;
    iConduit->ServiceRequestL(aMessage, aSession);
    }


//	From MCTKeyStore

// ---------------------------------------------------------------------------
// CDevCertKeyStoreServer::ListL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreServer::ListL(const TCTKeyAttributeFilter& aFilter,
							  RPointerArray<CDevTokenKeyInfo>& aKeys)
    {
    ASSERT(iMessage);

    // Check the calling process has ReadUserData capability
    if (!KListSecurityPolicy.CheckPolicy(*iMessage))
        {
        User::Leave(KErrPermissionDenied);
        }

    TInt count = iKeyDataManager->Count();

    for (TInt i = 0; i < count; ++i)
        {
        const CDevCertKeyData* data = (*iKeyDataManager)[i];
        CDevTokenKeyInfo* info = iKeyDataManager->ReadKeyInfoLC(*data);
        if (KeyMatchesFilterL(*info, aFilter))
            {
            User::LeaveIfError(aKeys.Append(info));
            CleanupStack::Pop(info);
            }
        else
            {
            CleanupStack::PopAndDestroy(info);
            }
        }
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreServer::KeyMatchesFilterL()
// ---------------------------------------------------------------------------
//
TBool CDevCertKeyStoreServer::KeyMatchesFilterL(const CDevTokenKeyInfo& aInfo,
										   const TCTKeyAttributeFilter& aFilter)
    {
    ASSERT(iMessage);

    if (aFilter.iKeyId.Length() && aFilter.iKeyId != aInfo.ID())
        {
        return EFalse;
        }

    if (aFilter.iUsage != EPKCS15UsageAll)
        {
        if ((aInfo.Usage() & aFilter.iUsage) == 0)
        return EFalse;
        }

    if (aFilter.iKeyAlgorithm != CCTKeyInfo::EInvalidAlgorithm && 
    aFilter.iKeyAlgorithm != aInfo.Algorithm())
        {
        return EFalse;
        }

    switch (aFilter.iPolicyFilter)
        {
        case TCTKeyAttributeFilter::EAllKeys:
        // All keys pass
        break;

        case TCTKeyAttributeFilter::EUsableKeys:
        if (!aInfo.UsePolicy().CheckPolicy(*iMessage))
            {
            return EFalse;
            }
        break;

        case TCTKeyAttributeFilter::EManageableKeys:
        if (!aInfo.ManagementPolicy().CheckPolicy(*iMessage))
            {
            return EFalse;
            }
        break;

        case TCTKeyAttributeFilter::EUsableOrManageableKeys:
        if (!aInfo.UsePolicy().CheckPolicy(*iMessage) &&
        !aInfo.ManagementPolicy().CheckPolicy(*iMessage))
            {
            return EFalse;
            }
        break;

        default:
        User::Leave(KErrArgument);
        }

    return ETrue;
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreServer::GetKeyInfoL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreServer::GetKeyInfoL(TInt aObjectId, CDevTokenKeyInfo*& aInfo)
    {
    const CDevCertKeyData* keyData = iKeyDataManager->Lookup(aObjectId);
    if (!keyData)
        {
        User::Leave(KErrNotFound);
        }

    CDevTokenKeyInfo* result = iKeyDataManager->ReadKeyInfoLC(*keyData);
    if (!result->UsePolicy().CheckPolicy(*iMessage))
        {
        User::Leave(KErrPermissionDenied);
        }

    aInfo = result;
    CleanupStack::Pop(aInfo);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreServer::GetKeyLengthL()
// ---------------------------------------------------------------------------
//
TInt CDevCertKeyStoreServer::GetKeyLengthL(TInt aObjectId)
    {
    const CDevCertKeyData* keyData = iKeyDataManager->Lookup(aObjectId);
    if (!keyData)
        {
        User::Leave(KErrNotFound);
        }

    // this could be cached in memory (would break file format though)
    CDevTokenKeyInfo* keyInfo = iKeyDataManager->ReadKeyInfoLC(*keyData);
    TInt result = keyInfo->Size();
    CleanupStack::PopAndDestroy(keyInfo);

    return result;
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreServer::OpenKeyL()
// ---------------------------------------------------------------------------
//
CDevCertOpenedKeySrv* CDevCertKeyStoreServer::OpenKeyL(TInt aHandle, TUid aOpenedKeyType)
    {
    ASSERT(iMessage);

    if (!KOpenSecurityPolicy.CheckPolicy(*iMessage))
        {
        User::Leave(KErrPermissionDenied);
        }


    const CDevCertKeyData *keyData = iKeyDataManager->Lookup(aHandle);
    if (!keyData)
        {
        User::Leave(KErrNotFound);
        }

    return CDevCertOpenedKeySrv::NewL( *keyData, aOpenedKeyType, *iMessage,*iKeyDataManager );						
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreServer::ExportPublicL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreServer::ExportPublicL(TInt aObjectId,
									  TDes8& aOut)
    {
    const CDevCertKeyData* keyData = iKeyDataManager->Lookup(aObjectId);
    if (!keyData)
        {
        User::Leave(KErrNotFound);
        }

    CDevTokenKeyInfo* keyInfo = iKeyDataManager->ReadKeyInfoLC(*keyData);

    RStoreReadStream stream;
    iKeyDataManager->OpenPublicDataStreamLC(*keyData, stream);

    CDevTokenKeyInfo::EKeyAlgorithm keyAlgorithm = keyInfo->Algorithm();

    switch(keyAlgorithm)
        {
        case (CDevTokenKeyInfo::ERSA):
            {
            CRSAPublicKey* publicKey = NULL;

            CreateL(stream, publicKey);
            ASSERT(publicKey);
            CleanupStack::PushL(publicKey);

            TX509RSAKeyEncoder encoder(*publicKey, ESHA1);
            CASN1EncBase* encoded = encoder.EncodeKeyLC();

            if (encoded->LengthDER() > static_cast<TUint>(aOut.MaxLength()))
                {
                User::Leave(KErrOverflow);
                }

            //	Get the Public key DER encoding
            TUint pos=0;
            encoded->WriteDERL(aOut, pos);

            // WriteDERL does not set the length of the buffer, we do it ourselves			
            aOut.SetLength(encoded->LengthDER());			

            CleanupStack::PopAndDestroy(2, publicKey);
            }
        break;

        case (CDevTokenKeyInfo::EDSA):
            {
            CDSAPublicKey* publicKey = NULL;

            CreateL(stream, publicKey);
            ASSERT(publicKey);
            CleanupStack::PushL(publicKey);

            TX509DSAKeyEncoder encoder(*publicKey, ESHA1);
            CASN1EncBase* encoded = encoder.EncodeKeyLC();

            if (encoded->LengthDER() > static_cast<TUint>(aOut.MaxLength()))
                {
                User::Leave(KErrOverflow);
                }

            //	Get the Public key DER encoding
            TUint pos=0;
            encoded->WriteDERL(aOut, pos);

            // WriteDERL does not set the length of the buffer, we do it ourselves			
            aOut.SetLength(encoded->LengthDER());						

            CleanupStack::PopAndDestroy(2, publicKey);
            }
        break;

        case (CDevTokenKeyInfo::EDH):
        default:
        User::Leave(KErrKeyAlgorithm);
        break;
        }

    CleanupStack::PopAndDestroy(2, keyInfo); //stream, keyinfo
    }


//	From MCTKeyStoreManager

// ---------------------------------------------------------------------------
// CDevCertKeyStoreServer::CheckKeyAttributes()
// ---------------------------------------------------------------------------
//
TInt CDevCertKeyStoreServer::CheckKeyAttributes(CDevTokenKeyInfo& aKey, TNewKeyOperation aOp)
    {
    ASSERT(iMessage);

    // Sort out the access rights
    TInt access = aKey.AccessType(); 

    // Only allow sensitive and extractable to be sepcified
    if (access & ~(CDevTokenKeyInfo::ESensitive | CDevTokenKeyInfo::EExtractable))
        {
        return KErrKeyAccess;
        }	

    // If it's sensitive and either created internally 
    // or imported from an encrypted source then it's always been sensitive
    if ((access & CDevTokenKeyInfo::ESensitive) &&
        (aOp == ENewKeyCreate))
        {
        access |= CDevTokenKeyInfo::EAlwaysSensitive;		
        }

    // If it's not extractable and it's created internally
    // then it's never been extractable
    if ((!(access & CDevTokenKeyInfo::EExtractable)) && aOp == ENewKeyCreate)
        {
        access |= CDevTokenKeyInfo::ENeverExtractable;		
        }

    aKey.SetAccessType(access);

    // check management policy allows the calling process to manage the key
    if (!aKey.ManagementPolicy().CheckPolicy(*iMessage))
        {
        return KErrArgument;
        }

    // check end date is not in the past
    TTime timeNow;
    timeNow.UniversalTime();
    if (aKey.EndDate().Int64() != 0 && aKey.EndDate() <= timeNow)
        {
        return KErrKeyValidity;
        }

    // We don't support non-repudiation, however we currently allow keys
    // to be created with this usage

    return KErrNone;
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreServer::CheckKeyAlgorithmAndSize()
// ---------------------------------------------------------------------------
//
TInt CDevCertKeyStoreServer::CheckKeyAlgorithmAndSize(CDevTokenKeyInfo& aKey)
    {
    CDevTokenKeyInfo::EKeyAlgorithm keyAlgorithm = aKey.Algorithm();
    if ( ((keyAlgorithm!=CDevTokenKeyInfo::ERSA) && (keyAlgorithm!=CDevTokenKeyInfo::EDSA) && (keyAlgorithm!=CDevTokenKeyInfo::EDH) ))
        {
        return KErrKeyAlgorithm;
        }

    if (aKey.Size() < KTheMinKeySize || aKey.Size() > KTheMaxKeySize)
        {
        return KErrKeySize;
        }

    return KErrNone;
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreServer::CreateKey()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreServer::CreateKey(CDevTokenKeyInfo& aReturnedKey, TRequestStatus& aStatus)
    {
    ASSERT(iSession);
    ASSERT(iMessage);

    TInt err = KErrNone;

    // Check the calling process has WriteUserData capability
    if (!KCreateSecurityPolicy.CheckPolicy(*iMessage))
        {
        err = KErrPermissionDenied;
        }

    // Check the incoming information has been initialised correctly
    if (err == KErrNone)
        {
        err = CheckKeyAttributes(aReturnedKey, ENewKeyCreate);
        }

    if (err == KErrNone)
        {
        err = CheckKeyAlgorithmAndSize(aReturnedKey);
        }

    if (err != KErrNone)
        {
        TRequestStatus* status = &aStatus;
        User::RequestComplete(status, err);
        return;
        }

    // should make it local only if it's created in the keystore
    aReturnedKey.SetAccessType(aReturnedKey.AccessType() | CDevTokenKeyInfo::ELocal);

    if (iKeyDataManager->IsKeyAlreadyInStore(aReturnedKey.Label()))
        {
        TRequestStatus* status = &aStatus;
        User::RequestComplete(status, KErrAlreadyExists);
        return;
        }

    iCallerRequest = &aStatus;
    iKeyInfo = &aReturnedKey;

    iStatus = KRequestPending;
    iAction = ECreateKeyCreate;
    SetActive();
    TRequestStatus* status = &iStatus;
    User::RequestComplete(status, KErrNone);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreServer::CancelCreateKey()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreServer::CancelCreateKey()
    {
    if (iAction == ECreateKeyCreate ||
    iAction == ECreateKeyFinal)
        {
        Cancel();
        }
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreServer::DoCreateKeyL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreServer::DoCreateKeyL()
    {
    __ASSERT_DEBUG(iAction==ECreateKeyCreate, PanicServer(EPanicECreateKeyNotReady));
    __ASSERT_DEBUG(iKeyInfo, PanicServer(EPanicNoClientData));

    if (iKeyCreator)
        {
        delete iKeyCreator;
        iKeyCreator = NULL;
        }

    iKeyCreator = new (ELeave) CDevCertKeyCreator();
    iStatus = KRequestPending;
    iAction = EKeyCreated;
    iKeyCreator->DoCreateKeyAsyncL(iKeyInfo->Algorithm(), iKeyInfo->Size(), iStatus);
    SetActive(); 
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreServer::DoStoreKeyL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreServer::DoStoreKeyL()
    {
    __ASSERT_DEBUG(iAction==ECreateKeyFinal, PanicServer(EPanicECreateKeyNotReady));
    __ASSERT_DEBUG(iKeyInfo, PanicServer(EPanicNoClientData));
    __ASSERT_DEBUG(iKeyCreator, PanicServer(ENoCreatedKeyData));

    const CDevCertKeyData* keyData = iKeyDataManager->CreateKeyDataLC(iKeyInfo->Label());	   

    RStoreWriteStream privateStream;
    iKeyDataManager->OpenPrivateDataStreamLC(*keyData, privateStream);

    CDevTokenKeyInfo::EKeyAlgorithm keyAlgorithm = iKeyInfo->Algorithm();

    // 	Get key identifier and externalize private key 
    TKeyIdentifier theKeyId;
    switch (keyAlgorithm)
        {
        case (CDevTokenKeyInfo::ERSA):
            {
            CRSAKeyPair* newKey = iKeyCreator->GetCreatedRSAKey();
            KeyIdentifierUtil::RSAKeyIdentifierL(newKey->PublicKey(), theKeyId);			
            privateStream << newKey->PrivateKey();
            break;
            }

        case (CDevTokenKeyInfo::EDSA):
            {
            CDSAKeyPair* newKey = iKeyCreator->GetCreatedDSAKey();			
            KeyIdentifierUtil::DSAKeyIdentifierL(newKey->PublicKey(), theKeyId);
            privateStream << newKey->PrivateKey();
            break;
            }

        case (CDevTokenKeyInfo::EDH):
            {
            CDevCertKeyEncryptor* encryptor = CDevCertKeyEncryptor::NewLC();
            if( encryptor->IsPluginExistL() )
                {
                //SetEncryptor( encryptor->CreateImplementationL() );
                MKeyEncryptor* encrypt = encryptor->CreateImplementationL();

                RInteger newKey; 
                iKeyCreator->GetCreatedDHKey(newKey);			
                KeyIdentifierUtil::DHKeyIdentifierL(newKey, theKeyId);

                if (newKey.IsZero())
                User::Leave(KErrArgument);

                //privateStream << newKey;
                EncryptAndStoreL(newKey, privateStream, encrypt );

                privateStream.CommitL();

                }
            else
                {
                RInteger newKey; 
                iKeyCreator->GetCreatedDHKey(newKey);			
                KeyIdentifierUtil::DHKeyIdentifierL(newKey, theKeyId);

                if (newKey.IsZero())
                User::Leave(KErrArgument);

                privateStream << newKey;
                privateStream.CommitL();
                } 
            CleanupStack::PopAndDestroy( encryptor );      
            break;
            }

        default:
        __ASSERT_DEBUG(EFalse, PanicServer(EPanicInvalidKeyCreateReq));
        break;
        }

    privateStream.CommitL();
    CleanupStack::PopAndDestroy(); // privateStream

    //	Fill in the CCTKeyInfo data currently missing (TKeyIdentifier and handle)
    iKeyInfo->SetHandle(keyData->Handle());
    iKeyInfo->SetIdentifier(theKeyId);	

    // 	Externalize public key

    RStoreWriteStream publicStream;
    iKeyDataManager->OpenPublicDataStreamLC(*keyData, publicStream);

    switch (keyAlgorithm)
        {
        case (CDevTokenKeyInfo::ERSA):
        publicStream << iKeyCreator->GetCreatedRSAKey()->PublicKey();			
        break;

        case (CDevTokenKeyInfo::EDSA):
        publicStream << iKeyCreator->GetCreatedDSAKey()->PublicKey();
        break;

        case (CDevTokenKeyInfo::EDH):
        // Nothing to do for DH
        break;

        default:
        __ASSERT_DEBUG(EFalse, PanicServer(EPanicInvalidKeyCreateReq));
        break;
        }

    publicStream.CommitL();
    CleanupStack::PopAndDestroy(); // publicStream

    //	Finished with the key creator
    if (iKeyCreator)
        {
        delete iKeyCreator;
        iKeyCreator = NULL;
        }

    //	Externalize the CDevTokenKeyInfo data associated with the key,
    iKeyDataManager->WriteKeyInfoL(*keyData, *iKeyInfo);

    //	Now add the new key to the data manager (which adds it to the store)
    iKeyDataManager->AddL(keyData);
    CleanupStack::Pop(); // keydata
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreServer::ImportKey()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreServer::ImportKey(const TDesC8& aKey, CDevTokenKeyInfo& aReturnedKey, TBool aIsEncrypted, TRequestStatus& aStatus)
    {
    ASSERT(iMessage);

    TInt err = KErrNone;

    // Check the calling process has WriteUserData capability
    if (!KImportRemoveSecurityPolicy.CheckPolicy(*iMessage))
        {
        err = KErrPermissionDenied;
        }

    if (err == KErrNone)
        {
        err = CheckKeyAttributes(aReturnedKey, ENewKeyImportPlaintext);
        }

    if (err == KErrNone && iKeyDataManager->IsKeyAlreadyInStore(aReturnedKey.Label()))
        {
        err = KErrAlreadyExists;
        }

    if (err != KErrNone)
        {
        TRequestStatus* status = &aStatus;
        User::RequestComplete(status, err);
        return;
        }

    iPKCS8Data.Set(aKey);
    
    iImportingEncryptedKey = aIsEncrypted;
    
    iCallerRequest = &aStatus;
    iKeyInfo = &aReturnedKey;

    iAction = EImportKey;
    SetActive();
    
    if ( aIsEncrypted )
	    {
		TDevTokenPWManager::ImportPassword( iPassword, iStatus );
	    }
	else
		{
		TRequestStatus* status = &iStatus;
		User::RequestComplete(status, KErrNone);
		}
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreServer::CancelImportKey()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreServer::CancelImportKey()
    {
    if (iAction == EImportOpenPrivateStream ||
    iAction == EImportKey)
        {
        Cancel();
        }
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreServer::DoImportKeyL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreServer::DoImportKeyL()
    {
    // Generate a decode PKCS8 data object from the incoming descriptor of PKCS8
    // data Creation of this will parse the DER stream and generate the
    // appropriate key representation based on the algorithm

    ASSERT(iPKCS8Data.Ptr());

    CDecPKCS8Data* pkcs8Data = NULL;

    if ( iImportingEncryptedKey )
	    {
	    // Convert import passphrase to 8 bit representation
		TBuf8<32> password;
		
		CnvUtfConverter::ConvertFromUnicodeToUtf8(password, iPassword);
		pkcs8Data = TASN1DecPKCS8::DecodeEncryptedDERL(iPKCS8Data, password);
	    }
	else
		{
		pkcs8Data = TASN1DecPKCS8::DecodeDERL(iPKCS8Data);
		}
    
    CleanupStack::PushL(pkcs8Data);	
    PKCS8ToKeyL(pkcs8Data);
    CleanupStack::PopAndDestroy(pkcs8Data);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreServer::PKCS8ToKeyL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreServer::PKCS8ToKeyL(CDecPKCS8Data* aPKCS8Data)
    { 
    ASSERT(aPKCS8Data);

    MPKCS8DecodedKeyPairData* keyPairData = aPKCS8Data->KeyPairData();

    // Set algorithm and size from pkcs8 data, and sanity check them
    if (aPKCS8Data->Algorithm() != ERSA && aPKCS8Data->Algorithm() != EDSA)
        {
        User::Leave(KErrKeyAlgorithm);
        }
    iKeyInfo->SetAlgorithm((aPKCS8Data->Algorithm() == ERSA) ? CKeyInfoBase::ERSA : CKeyInfoBase::EDSA);
    iKeyInfo->SetSize(keyPairData->KeySize());
    User::LeaveIfError(CheckKeyAlgorithmAndSize(*iKeyInfo));

    // Retrieve and store any PKCS8 attributes (in DER encoded descriptor)
    // These will form part of CDevTokenKeyInfo & available for callers to decode	
    TPtrC8 theAttributes(aPKCS8Data->PKCS8Attributes());
    if (theAttributes != KNullDesC8)
        {
        iKeyInfo->SetPKCS8AttributeSet(theAttributes.AllocL());
        }

    const CDevCertKeyData* keyData = iKeyDataManager->CreateKeyDataLC(iKeyInfo->Label());
    RStoreWriteStream privateStream;
    iKeyDataManager->OpenPrivateDataStreamLC(*keyData, privateStream);

    // Generate the key identifier
    TKeyIdentifier theKeyId;
    keyPairData->GetKeyIdentifierL(theKeyId);

    // Fill in the CDevTokenKeyInfo data currently missing (TKeyIdentifier and handle)
    iKeyInfo->SetHandle(keyData->Handle());
    iKeyInfo->SetIdentifier(theKeyId);	

    CDevTokenKeyInfo::EKeyAlgorithm keyAlgorithm = iKeyInfo->Algorithm();

    // Externalize private key data
    switch (keyAlgorithm)
        {
        case (CDevTokenKeyInfo::ERSA):
        privateStream << static_cast<CPKCS8KeyPairRSA*>(keyPairData)->PrivateKey();
        break;

        case (CDevTokenKeyInfo::EDSA):
        privateStream << static_cast<CPKCS8KeyPairDSA*>(keyPairData)->PrivateKey();
        break;

        default:
        __ASSERT_DEBUG(EFalse, PanicServer(EPanicInvalidKeyCreateReq));
        break;
        }

    privateStream.CommitL();
    CleanupStack::PopAndDestroy(&privateStream);

    // Externalize public key data
    RStoreWriteStream publicStream;
    iKeyDataManager->OpenPublicDataStreamLC(*keyData, publicStream);

    switch (keyAlgorithm)
        {
        case (CDevTokenKeyInfo::ERSA):
        publicStream << static_cast<CPKCS8KeyPairRSA*>(keyPairData)->PublicKey();
        break;

        case (CDevTokenKeyInfo::EDSA):
        publicStream << static_cast<CPKCS8KeyPairDSA*>(keyPairData)->PublicKey();
        break;

        default:
        __ASSERT_DEBUG(EFalse, PanicServer(EPanicInvalidKeyCreateReq));
        break;
        }

    publicStream.CommitL();
    CleanupStack::PopAndDestroy(&publicStream);

    // Externalize the CDevTokenKeyInfo data associated with the key,
    iKeyDataManager->WriteKeyInfoL(*keyData, *iKeyInfo);

    // Now add the new key to the data manager (which adds it to the store)
    iKeyDataManager->AddL(keyData);
    CleanupStack::Pop(const_cast<CDevCertKeyData*>(keyData));
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreServer::ExportKey()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreServer::ExportKey(TInt aObjectId, const TPtr8& aKey, TRequestStatus& aStatus)
    {				
    TRAPD(err, DoExportKeyL(aObjectId, aKey, aStatus));
    if (err != KErrNone)
        {
        TRequestStatus* status = &aStatus;
        User::RequestComplete(status, err);
        }
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreServer::CancelExportKey()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreServer::CancelExportKey()
    {
    if ( iAction == EExportKey )
        {
        Cancel();
        }
    }	


// ---------------------------------------------------------------------------
// CDevCertKeyStoreServer::DoExportKeyL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreServer::DoExportKeyL(TInt aObjectId, const TPtr8& aKey, TRequestStatus& aStatus)
    {
    ASSERT(iMessage);
    ASSERT(!iKeyData);
    ASSERT(!iKeyInfo);

    const CDevCertKeyData* keyData = iKeyDataManager->Lookup(aObjectId);
    if (!keyData)
        {
        User::Leave(KErrNotFound);
        }

    CDevTokenKeyInfo* keyInfo = iKeyDataManager->ReadKeyInfoLC(*keyData);

    // Check access flags allow key to be exported
    if (!(keyInfo->AccessType() & CCTKeyInfo::EExtractable) ||
    ((keyInfo->AccessType() & CCTKeyInfo::ESensitive)))
        {
        User::Leave(KErrKeyAccess);
        }

    // Check this isn't a DH key
    if (keyInfo->Algorithm() != CDevTokenKeyInfo::ERSA &&
    keyInfo->Algorithm() != CDevTokenKeyInfo::EDSA)
        {
        User::Leave(KErrNotSupported);
        }

    // Check the caller is allowed by the management policy
    if (!keyInfo->ManagementPolicy().CheckPolicy(*iMessage))
        {
        User::Leave(KErrPermissionDenied);
        }

    iKeyData = keyData;
    iKeyInfo = keyInfo;
    CleanupStack::Pop(keyInfo);
    iExportBuf.Set(aKey);		
    iCallerRequest = &aStatus;

    iAction = EExportKey;

    SetActive();				
    TRequestStatus* status = &iStatus;
    User::RequestComplete(status, KErrNone);		
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreServer::CompleteKeyExportL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreServer::CompleteKeyExportL()
    {
    ASSERT(iKeyData);
    ASSERT(iExportBuf.Ptr());

    CDevTokenKeyInfo::EKeyAlgorithm keyAlgorithm = iKeyInfo->Algorithm();
    RStoreReadStream privStream;		
    iKeyDataManager->OpenPrivateDataStreamLC(*iKeyData, privStream);

    CASN1EncSequence* encoded = NULL;

    switch(keyAlgorithm)
        {
        case (CDevTokenKeyInfo::ERSA):
            {
            RStoreReadStream pubStream;
            iKeyDataManager->OpenPublicDataStreamLC(*iKeyData, pubStream);
            CRSAPublicKey* publicKey = NULL;
            CreateL(pubStream, publicKey);
            ASSERT(publicKey);
            CleanupStack::PushL(publicKey);

            CRSAPrivateKey* privateKey = NULL;
            CreateL(privStream, privateKey);
            ASSERT(privateKey);
            CleanupStack::PushL(privateKey);			
            
            encoded = TASN1EncPKCS8::EncodeL(*(static_cast<CRSAPrivateKeyCRT*>(privateKey)), *publicKey, iKeyInfo->PKCS8AttributeSet());					
            
            CleanupStack::PopAndDestroy(3, &pubStream);          // privateKey,  publicKey, pubStream
            }
        break;

        case (CDevTokenKeyInfo::EDSA):
            {
            CDSAPrivateKey* privateKey = NULL;

            CreateL(privStream, privateKey);
            ASSERT(privateKey);
            CleanupStack::PushL(privateKey);

            encoded = TASN1EncPKCS8::EncodeL(*privateKey, iKeyInfo->PKCS8AttributeSet());					
            					
            CleanupStack::PopAndDestroy(privateKey);
            }
        break;

        case (CDevTokenKeyInfo::EInvalidAlgorithm):
        default:
        User::Leave(KErrKeyAlgorithm);		
        break;
        }

    // common to all algorithms			
    ASSERT(encoded);
    CleanupStack::PushL(encoded);
    if (encoded->LengthDER() > static_cast<TUint>(iExportBuf.MaxLength()))
        {
        User::Leave(KErrOverflow);
        }
    TUint pos=0;
    encoded->WriteDERL(iExportBuf, pos);

    // WriteDERL does not set the length of the buffer, we do it ourselves			
    iExportBuf.SetLength(encoded->LengthDER());

    CleanupStack::PopAndDestroy(encoded); 
    CleanupStack::PopAndDestroy(&privStream); 
    RunError(KErrNone);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreServer::DeleteKeyL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreServer::DeleteKeyL(TInt aObjectId)
    {
    ASSERT(iMessage);

    const CDevCertKeyData* keyData = iKeyDataManager->Lookup(aObjectId);
    if (!keyData)
        {
        User::Leave(KErrNotFound);
        }

    CDevTokenKeyInfo* keyInfo = iKeyDataManager->ReadKeyInfoLC(*keyData);

    // Check the caller is allowed by the management policy
    if (!KImportRemoveSecurityPolicy.CheckPolicy(*iMessage))
        {
        User::Leave(KErrPermissionDenied);
        }

    CleanupStack::PopAndDestroy(keyInfo);

    // Check if any session has this key open
    for (TInt i = 0 ; i < iSessions.Count() ; ++i)
        {
        CDevCertKeyStoreSession& session = *iSessions[i];
        if (session.HasOpenKey(aObjectId))
            {
            User::Leave(KErrInUse);
            }	
        }

    iKeyDataManager->RemoveL(aObjectId);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreServer::SetUsePolicyL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreServer::SetUsePolicyL(TInt aObjectId, const TSecurityPolicy& aPolicy)
    {
    ASSERT(iMessage);

    const CDevCertKeyData* keyData = iKeyDataManager->Lookup(aObjectId);
    if (!keyData)
        {
        User::Leave(KErrNotFound);
        }

    CDevTokenKeyInfo* keyInfo = iKeyDataManager->ReadKeyInfoLC(*keyData);

    // Check the caller is allowed by the management policy
    if (!keyInfo->ManagementPolicy().CheckPolicy(*iMessage))
        {
        User::Leave(KErrPermissionDenied);
        }

    // should revert change if write fails
    keyInfo->SetUsePolicy(aPolicy); 
    iKeyDataManager->SafeWriteKeyInfoL(*keyData, *keyInfo);	

    CleanupStack::PopAndDestroy(keyInfo);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreServer::SetManagementPolicyL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreServer::SetManagementPolicyL(TInt aObjectId, const TSecurityPolicy& aPolicy)
    {
    ASSERT(iMessage);

    const CDevCertKeyData* keyData = iKeyDataManager->Lookup(aObjectId);
    if (!keyData)
        {
        User::Leave(KErrNotFound);
        }

    CDevTokenKeyInfo* keyInfo = iKeyDataManager->ReadKeyInfoLC(*keyData);

    // Check the caller is allowed by current management policy
    if (!keyInfo->ManagementPolicy().CheckPolicy(*iMessage))
        {
        User::Leave(KErrPermissionDenied);
        }

    // Check the caller is allowed by new management policy
    if (!aPolicy.CheckPolicy(*iMessage))
        {
        User::Leave(KErrArgument);
        }

    // should revert change if write fails
    keyInfo->SetManagementPolicy(aPolicy);
    iKeyDataManager->SafeWriteKeyInfoL(*keyData, *keyInfo);	

    CleanupStack::PopAndDestroy(keyInfo);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreServer::RunError()
// ---------------------------------------------------------------------------
//
TInt CDevCertKeyStoreServer::RunError(TInt aError)
    { 
    // Delete anything we might have created
    delete iKeyCreator; iKeyCreator = NULL;

    if ( iAction == EExportKey )
        {
        // we only own iKeyInfo for export operations
        delete iKeyInfo;
        iKeyInfo = NULL;
        }

    // Zero pointers to things we don't own
    iKeyInfo = NULL;
    iKeyData = NULL;
    iExportBuf.Set(NULL, 0, 0);
    iPKCS8Data.Set(NULL, 0);
    iSession = NULL;
    iMessage = NULL;

    if (iCallerRequest)
    User::RequestComplete(iCallerRequest, aError);

    iAction = EIdle;		//	Reset action
    return (KErrNone);		//	Handled
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreServer::DoCancel()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreServer::DoCancel()
    {
    switch (iAction)
        {
        case ECreateKeyFinal:
        ASSERT(iKeyCreator);
        iKeyCreator->Cancel();
        break;

        default:
        // Nothing to do
        break;
        }

    RunError(KErrCancel);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyStoreServer::RunL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyStoreServer::RunL()
    {
    User::LeaveIfError(iStatus.Int()); 

    switch (iAction)
        {
        case ECreateKeyCreate:
            DoCreateKeyL();	
            iAction = ECreateKeyFinal;
            break;
        case ECreateKeyFinal:
            DoStoreKeyL();			 
            //	Check iKeyInfo was initialised for the caller
            ASSERT(iKeyInfo->HandleID() != 0);						
            RunError(KErrNone);
            break;
        case EImportKey:
            {
            TRAPD(err, DoImportKeyL());
            if (err == KErrTooBig)
                {
                // Returned by ASN library if data is unexpected probably as a result of
                // bad import data
                err = KErrArgument;
                }
            User::LeaveIfError(err);
            RunError(KErrNone);
            break;
            }
            	
        case EExportKey:
            {
            CompleteKeyExportL();
            break;
            }
   				
        default:
            ASSERT(EFalse);
        }
    }

//EOF

