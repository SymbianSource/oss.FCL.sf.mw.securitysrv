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
* Description:   Implementation of DevCertOpenedKeySrv
*
*/


#ifdef SYMBIAN_ENABLE_SPLIT_HEADERS
#include <mctkeystoreuids.h>
#endif
#include <asymmetric.h>
#include <asymmetrickeys.h>
#include <bigint.h>
#include <ct.h>
#include <securityerr.h>
#include <e32base.h>

#include "DevCertOpenedKeysSrv.h"
#include "DevCertKeyStoreServer.h"
#include "DevCertKeyStreamUtils.h"
#include "DevTokenDataTypes.h"


// ======== MEMBER FUNCTIONS ========

// CDevCertOpenedKeySrv

// ---------------------------------------------------------------------------
// CDevCertOpenedKeySrv::NewL()
// ---------------------------------------------------------------------------
//
CDevCertOpenedKeySrv* CDevCertOpenedKeySrv::NewL(const CDevCertKeyData& aKeyData, 
                                                 TUid aType, 
                                                 const RMessage2& aMessage,
                                                 CDevCertKeyDataManager& aKeyDataMan )
    {
    CDevCertOpenedKeySrv* self = NULL;

    if (aType == KRSARepudiableSignerUID)
        {
        self = new (ELeave) CDevCertRSARepudiableSignerSrv(aKeyData, aKeyDataMan );
        }
    else if (aType == KDSARepudiableSignerUID)
        {
        self = new (ELeave) CDevCertDSARepudiableSignerSrv(aKeyData, aKeyDataMan );
        }
    else if (aType == KPrivateDecryptorUID)
        {
        self = new (ELeave) CDevCertRSADecryptorSrv(aKeyData, aKeyDataMan );
        }
    else if (aType == KKeyAgreementUID)
        {
        self = new (ELeave) CDevCertDHAgreementSrv(aKeyData, aKeyDataMan );
        }
    else
        {
        User::Invariant();
        }

    CleanupStack::PushL(self);
    self->ConstructL(aMessage);
    CleanupStack::Pop(self);
    return self;
    }


// ---------------------------------------------------------------------------
// CDevCertOpenedKeySrv::CDevCertOpenedKeySrv()
// ---------------------------------------------------------------------------
//
CDevCertOpenedKeySrv::CDevCertOpenedKeySrv(const CDevCertKeyData& aKeyData, CDevCertKeyDataManager& aKeyDataMan ) :
  CActive(EPriorityStandard),
  iKeyData(aKeyData),
  iKeyDataMan(aKeyDataMan)
    {
    }


// ---------------------------------------------------------------------------
// CDevCertOpenedKeySrv::ConstructL()
// ---------------------------------------------------------------------------
//
void CDevCertOpenedKeySrv::ConstructL(const RMessage2& aMessage)
    {
    CDevTokenKeyInfo* keyInfo = iKeyDataMan.ReadKeyInfoLC(iKeyData);  
    CheckKeyL(*keyInfo, aMessage);
    iLabel = keyInfo->Label().AllocL();
    CleanupStack::PopAndDestroy(keyInfo);
    CActiveScheduler::Add(this);
    }


// ---------------------------------------------------------------------------
// CDevCertOpenedKeySrv::~CDevCertOpenedKeySrv()
// ---------------------------------------------------------------------------
//
CDevCertOpenedKeySrv::~CDevCertOpenedKeySrv()
    {
    Cancel();
    delete iLabel;
    }


// ---------------------------------------------------------------------------
// CDevCertOpenedKeySrv::Label()
// ---------------------------------------------------------------------------
//
const TDesC& CDevCertOpenedKeySrv::Label() const
    {
    return *iLabel;
    }


// ---------------------------------------------------------------------------
// CDevCertOpenedKeySrv::Handle()
// ---------------------------------------------------------------------------
//
TInt CDevCertOpenedKeySrv::Handle() const
    {
    return iKeyData.Handle();
    }


// ---------------------------------------------------------------------------
// CDevCertOpenedKeySrv::CheckKeyL()
// ---------------------------------------------------------------------------
//
void CDevCertOpenedKeySrv::CheckKeyL(const CDevTokenKeyInfo& aKeyInfo, const RMessage2& aMessage)
    {
    // Check the client is allowed to use the key
    if (!aKeyInfo.UsePolicy().CheckPolicy(aMessage))
        {
        User::Leave(KErrPermissionDenied);
        }

    // Check that the operation represented by this object is supported for this
    // type of key
    if (aKeyInfo.Algorithm() != Algorithm())
        {
        User::Leave(KErrKeyAlgorithm);
        }

    // Check the key usage allows the operation
    if ((aKeyInfo.Usage() & RequiredUsage()) == 0)
        {
        User::Leave(KErrKeyUsage);
        }

    // Check current time is after start date (if set) and before end date (if
    // set)
    TTime timeNow;
    timeNow.UniversalTime();
    if (aKeyInfo.StartDate().Int64() != 0 && timeNow < aKeyInfo.StartDate())
        {
        User::Leave(KErrKeyValidity);
        }
    if (aKeyInfo.EndDate().Int64() != 0 && timeNow >= aKeyInfo.EndDate())
        {
        User::Leave(KErrKeyValidity);
        }
    }


// ---------------------------------------------------------------------------
// CDevCertOpenedKeySrv::DoOperation()
// ---------------------------------------------------------------------------
//
void CDevCertOpenedKeySrv::DoOperation( TRequestStatus& aStatus )
    {
    ASSERT(iState == EIdle);
    iStatus = KRequestPending;
    iClientStatus = &aStatus;
    TRequestStatus* status = &iStatus;
    iState = EOperation;
    SetActive();
    User::RequestComplete( status, KErrNone );
    }   


// ---------------------------------------------------------------------------
// CDevCertOpenedKeySrv::RunL()
// ---------------------------------------------------------------------------
//
void CDevCertOpenedKeySrv::RunL()
    {
    User::LeaveIfError(iStatus.Int());

    switch (iState)
        {
        case EOperation:
        if (!iKeyRead)
            {
            RStoreReadStream stream;
            iKeyDataMan.OpenPrivateDataStreamLC(iKeyData, stream);
            ReadPrivateKeyL(stream);
            CleanupStack::PopAndDestroy(&stream);
            iKeyRead = ETrue;
            }
        PerformOperationL();
        break;

        default:
        ASSERT(EFalse);
        }

    Complete(KErrNone);
    }


// ---------------------------------------------------------------------------
// CDevCertOpenedKeySrv::RunError()
// ---------------------------------------------------------------------------
//
TInt CDevCertOpenedKeySrv::RunError(TInt aError)
    {
    Complete(aError);
    return KErrNone;
    }


// ---------------------------------------------------------------------------
// CDevCertOpenedKeySrv::DoCancel()
// ---------------------------------------------------------------------------
//
void CDevCertOpenedKeySrv::DoCancel()
    {
    Complete(KErrCancel);
    }


// ---------------------------------------------------------------------------
// CDevCertOpenedKeySrv::Complete()
// ---------------------------------------------------------------------------
//
void CDevCertOpenedKeySrv::Complete(TInt aError)
    {
    Cleanup();
    if (iClientStatus)
        {
        User::RequestComplete(iClientStatus, aError);
        }
    iState = EIdle;
    }



// CDevCertRSARepudiableSignerSrv 

// ---------------------------------------------------------------------------
// CDevCertRSARepudiableSignerSrv::CDevCertRSARepudiableSignerSrv()
// ---------------------------------------------------------------------------
//
CDevCertRSARepudiableSignerSrv::CDevCertRSARepudiableSignerSrv(const CDevCertKeyData& aKeyData, CDevCertKeyDataManager& aKeyDataMan ) :
  CDevCertOpenedKeySrv(aKeyData, aKeyDataMan )
    {
    }


// ---------------------------------------------------------------------------
// CDevCertRSARepudiableSignerSrv::~CDevCertRSARepudiableSignerSrv()
// ---------------------------------------------------------------------------
//
CDevCertRSARepudiableSignerSrv::~CDevCertRSARepudiableSignerSrv()
    {
    delete iPrivateKey;
    }


// ---------------------------------------------------------------------------
// CDevCertRSARepudiableSignerSrv::Type()
// ---------------------------------------------------------------------------
//
TUid CDevCertRSARepudiableSignerSrv::Type() const
    {
    return KRSARepudiableSignerUID;
    }


// ---------------------------------------------------------------------------
// CDevCertRSARepudiableSignerSrv::Algorithm()
// ---------------------------------------------------------------------------
//
CDevTokenKeyInfo::EKeyAlgorithm CDevCertRSARepudiableSignerSrv::Algorithm() const
    {
    return CDevTokenKeyInfo::ERSA;
    }


// ---------------------------------------------------------------------------
// CDevCertRSARepudiableSignerSrv::RequiredUsage()
// ---------------------------------------------------------------------------
//
TKeyUsagePKCS15 CDevCertRSARepudiableSignerSrv::RequiredUsage() const
    {
    return EPKCS15UsageSignSignRecover;
    }


// ---------------------------------------------------------------------------
// CDevCertRSARepudiableSignerSrv::Sign()
// ---------------------------------------------------------------------------
//
void CDevCertRSARepudiableSignerSrv::Sign(const TDesC8& aPlaintext,
                CRSASignature*& aSignature,
                TRequestStatus& aStatus)
    {
    ASSERT( iPlaintext.Ptr() == NULL );
    ASSERT( iSignaturePtr == NULL );
    iPlaintext.Set(aPlaintext);
    iSignaturePtr = &aSignature;
    DoOperation( aStatus );
    }


// ---------------------------------------------------------------------------
// CDevCertRSARepudiableSignerSrv::ReadPrivateKeyL()
// ---------------------------------------------------------------------------
//
void CDevCertRSARepudiableSignerSrv::ReadPrivateKeyL(RReadStream& aStream)
    {
    ASSERT( iPrivateKey == NULL );
    CreateL(aStream, iPrivateKey);
    }


// ---------------------------------------------------------------------------
// CDevCertRSARepudiableSignerSrv::PerformOperationL()
// ---------------------------------------------------------------------------
//
void CDevCertRSARepudiableSignerSrv::PerformOperationL()
    {
    ASSERT(iPrivateKey);

    CRSAPKCS1v15Signer* signer = CRSAPKCS1v15Signer::NewLC(*iPrivateKey);
    const CRSASignature* signature = signer->SignL(iPlaintext);
    CleanupStack::PopAndDestroy(signer);
    *iSignaturePtr = const_cast<CRSASignature*>(signature);
    }


// ---------------------------------------------------------------------------
// CDevCertRSARepudiableSignerSrv::Cleanup()
// ---------------------------------------------------------------------------
//
void CDevCertRSARepudiableSignerSrv::Cleanup()
    {
    iPlaintext.Set(NULL, 0);
    iSignaturePtr = NULL;
    }


// CDevCertDSARepudiableSignerSrv 

// ---------------------------------------------------------------------------
// CDevCertDSARepudiableSignerSrv::CDevCertDSARepudiableSignerSrv()
// ---------------------------------------------------------------------------
//
CDevCertDSARepudiableSignerSrv::CDevCertDSARepudiableSignerSrv(const CDevCertKeyData& aKeyData, CDevCertKeyDataManager& aKeyDataMan ) :
  CDevCertOpenedKeySrv(aKeyData, aKeyDataMan )
    {
    }


// ---------------------------------------------------------------------------
// CDevCertDSARepudiableSignerSrv::~CDevCertDSARepudiableSignerSrv()
// ---------------------------------------------------------------------------
//
CDevCertDSARepudiableSignerSrv::~CDevCertDSARepudiableSignerSrv()
    {
    delete iPrivateKey;
    }


// ---------------------------------------------------------------------------
// CDevCertDSARepudiableSignerSrv::Type()
// ---------------------------------------------------------------------------
//
TUid CDevCertDSARepudiableSignerSrv::Type() const
    {
    return KDSARepudiableSignerUID;
    }


// ---------------------------------------------------------------------------
// CDevCertDSARepudiableSignerSrv::Algorithm()
// ---------------------------------------------------------------------------
//
CDevTokenKeyInfo::EKeyAlgorithm CDevCertDSARepudiableSignerSrv::Algorithm() const
    {
    return CDevTokenKeyInfo::EDSA;
    }


// ---------------------------------------------------------------------------
// CDevCertDSARepudiableSignerSrv::RequiredUsage()
// ---------------------------------------------------------------------------
//
TKeyUsagePKCS15 CDevCertDSARepudiableSignerSrv::RequiredUsage() const
    {
    return EPKCS15UsageSignSignRecover;
    }


// ---------------------------------------------------------------------------
// CDevCertDSARepudiableSignerSrv::Sign()
// ---------------------------------------------------------------------------
//
void CDevCertDSARepudiableSignerSrv::Sign(const TDesC8& aPlaintext,
                CDSASignature*& aSignature,
                TRequestStatus& aStatus)
    {
    ASSERT( iPlaintext.Ptr() == NULL );
    ASSERT( iSignaturePtr == NULL );
    iPlaintext.Set(aPlaintext);
    iSignaturePtr = &aSignature;
    DoOperation( aStatus );
    }


// ---------------------------------------------------------------------------
// CDevCertDSARepudiableSignerSrv::ReadPrivateKeyL()
// ---------------------------------------------------------------------------
//
void CDevCertDSARepudiableSignerSrv::ReadPrivateKeyL(RReadStream& aStream)
    {
    ASSERT( iPrivateKey == NULL );
    CreateL(aStream, iPrivateKey);
    }


// ---------------------------------------------------------------------------
// CDevCertDSARepudiableSignerSrv::PerformOperationL()
// ---------------------------------------------------------------------------
//
void CDevCertDSARepudiableSignerSrv::PerformOperationL()
    {
    ASSERT( iPrivateKey );

    CDSASigner* signer = CDSASigner::NewLC(*iPrivateKey);
    const CDSASignature* signature = signer->SignL(iPlaintext);
    CleanupStack::PopAndDestroy(signer);
    *iSignaturePtr = const_cast<CDSASignature*>(signature);
    }


// ---------------------------------------------------------------------------
// CDevCertDSARepudiableSignerSrv::Cleanup()
// ---------------------------------------------------------------------------
//
void CDevCertDSARepudiableSignerSrv::Cleanup()
    {
    iPlaintext.Set(NULL, 0);
    iSignaturePtr = NULL;
    }


// CDevCertRSADecryptorSrv 

// ---------------------------------------------------------------------------
// CDevCertRSADecryptorSrv::CDevCertRSADecryptorSrv()
// ---------------------------------------------------------------------------
//
CDevCertRSADecryptorSrv::CDevCertRSADecryptorSrv(const CDevCertKeyData& aKeyData, CDevCertKeyDataManager& aKeyDataMan ) :
  CDevCertOpenedKeySrv(aKeyData, aKeyDataMan )
    {
    }


// ---------------------------------------------------------------------------
// CDevCertRSADecryptorSrv::~CDevCertRSADecryptorSrv()
// ---------------------------------------------------------------------------
//
CDevCertRSADecryptorSrv::~CDevCertRSADecryptorSrv()
    {
    delete iPrivateKey;
    }


// ---------------------------------------------------------------------------
// CDevCertRSADecryptorSrv::Type()
// ---------------------------------------------------------------------------
//
TUid CDevCertRSADecryptorSrv::Type() const
    {
    return KPrivateDecryptorUID;
    }


// ---------------------------------------------------------------------------
// CDevCertRSADecryptorSrv::Algorithm()
// ---------------------------------------------------------------------------
//
CDevTokenKeyInfo::EKeyAlgorithm CDevCertRSADecryptorSrv::Algorithm() const
    {
    return CDevTokenKeyInfo::ERSA;
    }


// ---------------------------------------------------------------------------
// CDevCertRSADecryptorSrv::RequiredUsage()
// ---------------------------------------------------------------------------
//
TKeyUsagePKCS15 CDevCertRSADecryptorSrv::RequiredUsage() const
    {
    return EPKCS15UsageDecryptUnwrap;
    }


// ---------------------------------------------------------------------------
// CDevCertRSADecryptorSrv::Decrypt()
// ---------------------------------------------------------------------------
//
void CDevCertRSADecryptorSrv::Decrypt(const TDesC8& aCiphertext,
                HBufC8*& aPlaintext,
                TRequestStatus& aStatus)
    {
    ASSERT( iCiphertext.Ptr() == NULL );
    ASSERT( iPlaintextPtr == NULL );
    iCiphertext.Set(aCiphertext);
    iPlaintextPtr = &aPlaintext;
    DoOperation( aStatus );
    }


// ---------------------------------------------------------------------------
// CDevCertRSADecryptorSrv::ReadPrivateKeyL()
// ---------------------------------------------------------------------------
//
void CDevCertRSADecryptorSrv::ReadPrivateKeyL(RReadStream& aStream)
    {
    ASSERT( iPrivateKey == NULL );
    CreateL(aStream, iPrivateKey);
    }


// ---------------------------------------------------------------------------
// CDevCertRSADecryptorSrv::PerformOperationL()
// ---------------------------------------------------------------------------
//
void CDevCertRSADecryptorSrv::PerformOperationL()
    {
    ASSERT(iPrivateKey);

    CRSAPKCS1v15Decryptor* decryptor = CRSAPKCS1v15Decryptor::NewLC(*iPrivateKey);
    HBufC8* plaintext = HBufC8::NewMaxLC(decryptor->MaxOutputLength());
    TPtr8 ptr = plaintext->Des();
    decryptor->DecryptL(iCiphertext, ptr);

    *iPlaintextPtr = plaintext;
    CleanupStack::Pop(plaintext); // now owned by client
    CleanupStack::PopAndDestroy(decryptor);
    }


// ---------------------------------------------------------------------------
// CDevCertRSADecryptorSrv::Cleanup()
// ---------------------------------------------------------------------------
//
void CDevCertRSADecryptorSrv::Cleanup()
    {
    iCiphertext.Set(NULL, 0);
    iPlaintextPtr = NULL;
    }


// CDevCertDHAgreementSrv 

// ---------------------------------------------------------------------------
// CDevCertDHAgreementSrv::CDevCertDHAgreementSrv()
// ---------------------------------------------------------------------------
//
CDevCertDHAgreementSrv::CDevCertDHAgreementSrv(const CDevCertKeyData& aKeyData, CDevCertKeyDataManager& aKeyDataMan ) :
  CDevCertOpenedKeySrv(aKeyData, aKeyDataMan )
    {
    }


// ---------------------------------------------------------------------------
// CDevCertDHAgreementSrv::~CDevCertDHAgreementSrv()
// ---------------------------------------------------------------------------
//
CDevCertDHAgreementSrv::~CDevCertDHAgreementSrv()
    {
    iKey.Close();
    }


// ---------------------------------------------------------------------------
// CDevCertDHAgreementSrv::Type()
// ---------------------------------------------------------------------------
//
TUid CDevCertDHAgreementSrv::Type() const
    {
    return KKeyAgreementUID;
    }


// ---------------------------------------------------------------------------
// CDevCertDHAgreementSrv::Algorithm()
// ---------------------------------------------------------------------------
//
CDevTokenKeyInfo::EKeyAlgorithm CDevCertDHAgreementSrv::Algorithm() const
    {
    return CDevTokenKeyInfo::EDH;
    }


// ---------------------------------------------------------------------------
// CDevCertDHAgreementSrv::RequiredUsage()
// ---------------------------------------------------------------------------
//
TKeyUsagePKCS15 CDevCertDHAgreementSrv::RequiredUsage() const
    {
    return EPKCS15UsageDerive;
    }


// ---------------------------------------------------------------------------
// CDevCertDHAgreementSrv::PublicKey()
// ---------------------------------------------------------------------------
//
void CDevCertDHAgreementSrv::PublicKey(CDevTokenDHParams& aParameters, RInteger& aPublicKey, TRequestStatus& aStatus)
    {
    ASSERT( iPKParams == NULL );
    ASSERT( iPKPublicKeyPtr == NULL );
    iPKParams = &aParameters;
    iPKPublicKeyPtr = &aPublicKey;
    iDHState = EPublicKey;
    DoOperation( aStatus );
    }


// ---------------------------------------------------------------------------
// CDevCertDHAgreementSrv::Agree()
// ---------------------------------------------------------------------------
//
void CDevCertDHAgreementSrv::Agree(CDHPublicKey& aY, HBufC8*& aAgreedKey, TRequestStatus& aStatus)
    {
    ASSERT( iAKPublicKey == NULL );
    ASSERT( iAKAgreedKeyPtr == NULL );
    iAKPublicKey = &aY;
    iAKAgreedKeyPtr = &aAgreedKey;
    iDHState = EAgree;
    DoOperation( aStatus );
    }


// ---------------------------------------------------------------------------
// CDevCertDHAgreementSrv::ReadPrivateKeyL()
// ---------------------------------------------------------------------------
//
void CDevCertDHAgreementSrv::ReadPrivateKeyL(RReadStream& aStream)
    {
    CreateLC(aStream, iKey);
    CleanupStack::Pop(&iKey);
    }


// ---------------------------------------------------------------------------
// CDevCertDHAgreementSrv::PerformOperationL()
// ---------------------------------------------------------------------------
//
void CDevCertDHAgreementSrv::PerformOperationL()
    {
    switch (iDHState)
        {
        case EPublicKey:
        DoPublicKeyL();
        break;
        case EAgree:
        DoAgreeL();
        break;
        default:
        ASSERT(FALSE);
        }
    }


// ---------------------------------------------------------------------------
// CDevCertDHAgreementSrv::DoPublicKeyL()
// ---------------------------------------------------------------------------
//
void CDevCertDHAgreementSrv::DoPublicKeyL()
    {
    ASSERT(iPKParams);
    ASSERT(iPKPublicKeyPtr);

    RInteger n = iPKParams->TakeN();
    CleanupStack::PushL(n);
    RInteger g = iPKParams->TakeG();
    CleanupStack::PushL(g);
    RInteger x = RInteger::NewL(iKey);
    CleanupStack::PushL(x);
    CDHKeyPair* keyPair = CDHKeyPair::NewL(n, g, x);
    CleanupStack::Pop(3); // x, g, n
    CleanupStack::PushL(keyPair);

    const CDHPublicKey& pubKey = keyPair->PublicKey();
    *iPKPublicKeyPtr = RInteger::NewL(pubKey.X());
    CleanupStack::PopAndDestroy(keyPair);
    }


// ---------------------------------------------------------------------------
// CDevCertDHAgreementSrv::DoAgreeL()
// ---------------------------------------------------------------------------
//
void CDevCertDHAgreementSrv::DoAgreeL()
    {
    ASSERT(iAKPublicKey);
    ASSERT(iAKAgreedKeyPtr);

    RInteger n = RInteger::NewL(iAKPublicKey->N());
    CleanupStack::PushL(n);
    RInteger g = RInteger::NewL(iAKPublicKey->G());
    CleanupStack::PushL(g);
    RInteger x = RInteger::NewL(iKey);
    CleanupStack::PushL(x);
    CDHPrivateKey* privKey = CDHPrivateKey::NewL(n, g, x);
    CleanupStack::Pop(3); // x, g, n
    CleanupStack::PushL(privKey);
    CDH* dh = CDH::NewLC(*privKey);
    *iAKAgreedKeyPtr = const_cast<HBufC8*>(dh->AgreeL(*iAKPublicKey));
    CleanupStack::PopAndDestroy(2, privKey);
    }


// ---------------------------------------------------------------------------
// CDevCertDHAgreementSrv::Cleanup()
// ---------------------------------------------------------------------------
//
void CDevCertDHAgreementSrv::Cleanup()
    {
    iPKParams = NULL;
    iPKPublicKeyPtr = NULL;
    iAKPublicKey = NULL;
    iAKAgreedKeyPtr = NULL;
    iDHState = EIdle;
    }

//EOF


