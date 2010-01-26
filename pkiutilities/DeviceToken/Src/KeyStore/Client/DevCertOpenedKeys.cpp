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
* Description:   Implementation of DevCertOpenedKey
*
*/



#include <e32base.h>
#include <ct.h>
#include <mctkeystore.h>
#include <mctkeystoreuids.h>
#include <hash.h>
#include "DevCertOpenedKeys.h"
#include "DevCertKeyStore.h"


// ======== MEMBER FUNCTIONS ========

//CDevCertOpenedKey

// -----------------------------------------------------------------------------
// CDevCertOpenedKey::~CDevCertOpenedKey()
// -----------------------------------------------------------------------------
//
CDevCertOpenedKey::~CDevCertOpenedKey()
    {
    if ( iClient )
        {
        iClient->ReleaseObject(iHandle);
    	  }	
    delete iLabel;
    }

// CDevCertRSARepudiableSigner

// -----------------------------------------------------------------------------
// CDevCertRSARepudiableSigner::New()
// -----------------------------------------------------------------------------
//
CDevCertRSARepudiableSigner* CDevCertRSARepudiableSigner::New(CDevCertKeyStore* aClient)
    {
    return new CDevCertRSARepudiableSigner(aClient);
    }


// -----------------------------------------------------------------------------
// CDevCertRSARepudiableSigner::CDevCertRSARepudiableSigner()
// -----------------------------------------------------------------------------
//
CDevCertRSARepudiableSigner::CDevCertRSARepudiableSigner(CDevCertKeyStore* aClient)
    : MCTSigner<CRSASignature*>(aClient->Token())
    {
    iClient = aClient;
    iHandle.iTokenHandle = aClient->Token().Handle();
    iHandle.iObjectId = 0;
    }


// -----------------------------------------------------------------------------
// CDevCertRSARepudiableSigner::~CDevCertRSARepudiableSigner()
// -----------------------------------------------------------------------------
//
CDevCertRSARepudiableSigner::~CDevCertRSARepudiableSigner()
    {
    delete iDigest;
    }


// -----------------------------------------------------------------------------
// CDevCertRSARepudiableSigner::Release()
// -----------------------------------------------------------------------------
//
void CDevCertRSARepudiableSigner::Release()
    {
    MCTTokenObject::Release();
    }


// -----------------------------------------------------------------------------
// CDevCertRSARepudiableSigner::Label()
// -----------------------------------------------------------------------------
//
const TDesC& CDevCertRSARepudiableSigner::Label() const
    {
    return iLabel ? static_cast<const TDesC&>(*iLabel) : static_cast<const TDesC&>(KNullDesC);
    }


// -----------------------------------------------------------------------------
// CDevCertRSARepudiableSigner::Token()
// -----------------------------------------------------------------------------
//
MCTToken& CDevCertRSARepudiableSigner::Token() const
    {
    return iClient->Token();
    }


// -----------------------------------------------------------------------------
// CDevCertRSARepudiableSigner::Type()
// -----------------------------------------------------------------------------
//
TUid CDevCertRSARepudiableSigner::Type() const
    {
    return KRSARepudiableSignerUID;
    }


// -----------------------------------------------------------------------------
// CDevCertRSARepudiableSigner::Handle()
// -----------------------------------------------------------------------------
//
TCTTokenObjectHandle CDevCertRSARepudiableSigner::Handle() const
    {
    return iHandle;
    }


// -----------------------------------------------------------------------------
// CDevCertRSARepudiableSigner::SignMessage()
// -----------------------------------------------------------------------------
//
void CDevCertRSARepudiableSigner::SignMessage(const TDesC8& aPlaintext, 
          CRSASignature*& aSignature, 
          TRequestStatus& aStatus)
    {
    // Hash the data on the client side
    TRAPD(err, iDigest = CSHA1::NewL());
    if (err != KErrNone)
        {
        TRequestStatus* status = &aStatus;
        User::RequestComplete(status, err);
        }
    else
        {
        iDigest->Update(aPlaintext);  
        Sign(iDigest->Final(), aSignature, aStatus);
        }
    }


// -----------------------------------------------------------------------------
// CDevCertRSARepudiableSigner::Sign()
// -----------------------------------------------------------------------------
//
void CDevCertRSARepudiableSigner::Sign(const TDesC8& aPlaintext, 
          CRSASignature*& aSignature, 
          TRequestStatus& aStatus)
    {
    iClient->RepudiableRSASign(Handle(),aPlaintext, aSignature, aStatus);
    }


// -----------------------------------------------------------------------------
// CDevCertRSARepudiableSigner::CancelSign()
// -----------------------------------------------------------------------------
//
void CDevCertRSARepudiableSigner::CancelSign()
    {
    // Synchronous, no need to cancel
    }


// CDevCertDSARepudiableSigner 

// -----------------------------------------------------------------------------
// CDevCertDSARepudiableSigner::New()
// -----------------------------------------------------------------------------
//
CDevCertDSARepudiableSigner* CDevCertDSARepudiableSigner::New(CDevCertKeyStore* aClient)
    {
    return new CDevCertDSARepudiableSigner(aClient);
    }


// -----------------------------------------------------------------------------
// CDevCertDSARepudiableSigner::CDevCertDSARepudiableSigner()
// -----------------------------------------------------------------------------
//
CDevCertDSARepudiableSigner::CDevCertDSARepudiableSigner(CDevCertKeyStore* aClient)
    : MCTSigner<CDSASignature*>(aClient->Token())
    {
    iClient = aClient;
    iHandle.iTokenHandle = aClient->Token().Handle();
    iHandle.iObjectId = 0;
    }


// -----------------------------------------------------------------------------
// CDevCertDSARepudiableSigner::~CDevCertDSARepudiableSigner()
// -----------------------------------------------------------------------------
//
CDevCertDSARepudiableSigner::~CDevCertDSARepudiableSigner()
    {
    delete iDigest;
    }


// -----------------------------------------------------------------------------
// CDevCertDSARepudiableSigner::Release()
// -----------------------------------------------------------------------------
//
void CDevCertDSARepudiableSigner::Release()
    {
    MCTTokenObject::Release();
    }


// -----------------------------------------------------------------------------
// CDevCertDSARepudiableSigner::Label()
// -----------------------------------------------------------------------------
//
const TDesC& CDevCertDSARepudiableSigner::Label() const
    {
    return iLabel ? static_cast<const TDesC&>(*iLabel) : static_cast<const TDesC&>(KNullDesC);
    }


// -----------------------------------------------------------------------------
// CDevCertDSARepudiableSigner::Token()
// -----------------------------------------------------------------------------
//
MCTToken& CDevCertDSARepudiableSigner::Token() const
    {
    return iClient->Token();
    }


// -----------------------------------------------------------------------------
// CDevCertDSARepudiableSigner::Type()
// -----------------------------------------------------------------------------
//
TUid CDevCertDSARepudiableSigner::Type() const
    {
    return KDSARepudiableSignerUID;
    }


// -----------------------------------------------------------------------------
// CDevCertDSARepudiableSigner::Handle()
// -----------------------------------------------------------------------------
//
TCTTokenObjectHandle CDevCertDSARepudiableSigner::Handle() const
    {
    return iHandle;
    }


// -----------------------------------------------------------------------------
// CDevCertDSARepudiableSigner::SignMessage()
// -----------------------------------------------------------------------------
//
void CDevCertDSARepudiableSigner::SignMessage(const TDesC8& aPlaintext, 
          CDSASignature*& aSignature, 
        TRequestStatus& aStatus)
    {
    // Hash the data on the client side
    TRAPD(err, iDigest = CSHA1::NewL());
    if (err != KErrNone)
        {
        TRequestStatus* status = &aStatus;
        User::RequestComplete(status, err);
        }
    else
        {
        iDigest->Update(aPlaintext);
        Sign(iDigest->Final(), aSignature, aStatus);
        }
    }


// -----------------------------------------------------------------------------
// CDevCertDSARepudiableSigner::Sign()
// -----------------------------------------------------------------------------
//
void CDevCertDSARepudiableSigner::Sign(const TDesC8& aPlaintext, 
          CDSASignature*& aSignature, 
        TRequestStatus& aStatus)
    {
    iClient->RepudiableDSASign(Handle(),aPlaintext, aSignature, aStatus);
    }


// -----------------------------------------------------------------------------
// CDevCertDSARepudiableSigner::CancelSign()
// -----------------------------------------------------------------------------
//
void CDevCertDSARepudiableSigner::CancelSign()
    {
    // Synchronous, no need to cancel
    }


// CDevCertRSADecryptor

// -----------------------------------------------------------------------------
// CDevCertRSADecryptor::New()
// -----------------------------------------------------------------------------
//
CDevCertRSADecryptor* CDevCertRSADecryptor::New(CDevCertKeyStore* aClient)
    {
    return new CDevCertRSADecryptor(aClient);
    }


// -----------------------------------------------------------------------------
// CDevCertRSADecryptor::CDevCertRSADecryptor()
// -----------------------------------------------------------------------------
//
CDevCertRSADecryptor::CDevCertRSADecryptor(CDevCertKeyStore* aClient)
    : MCTDecryptor(aClient->Token())
    {
    iClient = aClient;
    iHandle.iTokenHandle = aClient->Token().Handle();
    iHandle.iObjectId = 0;
    }


// -----------------------------------------------------------------------------
// CDevCertRSADecryptor::~CDevCertRSADecryptor()
// -----------------------------------------------------------------------------
//
CDevCertRSADecryptor::~CDevCertRSADecryptor()
    {
    }


// -----------------------------------------------------------------------------
// CDevCertRSADecryptor::Release()
// -----------------------------------------------------------------------------
//
void CDevCertRSADecryptor::Release()
    {
    MCTTokenObject::Release();
    }


// -----------------------------------------------------------------------------
// CDevCertRSADecryptor::Label()
// -----------------------------------------------------------------------------
//
const TDesC& CDevCertRSADecryptor::Label() const
    {
    return iLabel ? static_cast<const TDesC&>(*iLabel) : static_cast<const TDesC&>(KNullDesC);
    }


// -----------------------------------------------------------------------------
// CDevCertRSADecryptor::Token()
// -----------------------------------------------------------------------------
//
MCTToken& CDevCertRSADecryptor::Token() const
    {
    return iClient->Token();
    }


// -----------------------------------------------------------------------------
// CDevCertRSADecryptor::Type()
// -----------------------------------------------------------------------------
//
TUid CDevCertRSADecryptor::Type() const
    {
    return KPrivateDecryptorUID;
    }


// -----------------------------------------------------------------------------
// CDevCertRSADecryptor::Handle()
// -----------------------------------------------------------------------------
//
TCTTokenObjectHandle CDevCertRSADecryptor::Handle() const
    {
    return iHandle;
    }


// -----------------------------------------------------------------------------
// CDevCertRSADecryptor::Decrypt()
// -----------------------------------------------------------------------------
//
void CDevCertRSADecryptor::Decrypt(const TDesC8& aCiphertext,
      TDes8& aPlaintext, 
      TRequestStatus& aStatus)
    {
    iClient->Decrypt(Handle(),aCiphertext,aPlaintext, aStatus);
    }


// -----------------------------------------------------------------------------
// CDevCertRSADecryptor::CancelDecrypt()
// -----------------------------------------------------------------------------
//
void CDevCertRSADecryptor::CancelDecrypt()
    {
    // Synchronous, no need to cancel
    }


// CDevCertDHAgreement

// -----------------------------------------------------------------------------
// CDevCertDHAgreement::New()
// -----------------------------------------------------------------------------
//
CDevCertDHAgreement* CDevCertDHAgreement::New(CDevCertKeyStore* aClient)
    {
    return new CDevCertDHAgreement(aClient);
    }


// -----------------------------------------------------------------------------
// CDevCertDHAgreement::CDevCertDHAgreement()
// -----------------------------------------------------------------------------
//
CDevCertDHAgreement::CDevCertDHAgreement(CDevCertKeyStore* aClient)
  : MCTDH(aClient->Token())
    {
    iClient = aClient;
    iHandle.iTokenHandle = aClient->Token().Handle();
    iHandle.iObjectId = 0;
    }


// -----------------------------------------------------------------------------
// CDevCertDHAgreement::~CDevCertDHAgreement()
// -----------------------------------------------------------------------------
//
CDevCertDHAgreement::~CDevCertDHAgreement()
    {
    }


// -----------------------------------------------------------------------------
// CDevCertDHAgreement::Release()
// -----------------------------------------------------------------------------
//
void CDevCertDHAgreement::Release()
    {
    MCTTokenObject::Release();
    }


// -----------------------------------------------------------------------------
// CDevCertDHAgreement::Label()
// -----------------------------------------------------------------------------
//
const TDesC& CDevCertDHAgreement::Label() const
    {
    return iLabel ? static_cast<const TDesC&>(*iLabel) : static_cast<const TDesC&>(KNullDesC);
    }


// -----------------------------------------------------------------------------
// CDevCertDHAgreement::Token()
// -----------------------------------------------------------------------------
//
MCTToken& CDevCertDHAgreement::Token() const
    {
    return iClient->Token();
    }


// -----------------------------------------------------------------------------
// CDevCertDHAgreement::Type()
// -----------------------------------------------------------------------------
//
TUid CDevCertDHAgreement::Type() const
    {
    return KKeyAgreementUID;
    }


// -----------------------------------------------------------------------------
// CDevCertDHAgreement::Handle()
// -----------------------------------------------------------------------------
//
TCTTokenObjectHandle CDevCertDHAgreement::Handle() const
    {
    return iHandle;
    }


// -----------------------------------------------------------------------------
// CDevCertDHAgreement::PublicKey()
// Returns the public key ('Big X') for the supplied set of parameters
// -----------------------------------------------------------------------------
//
void CDevCertDHAgreement::PublicKey(const TInteger& aN, const TInteger& aG, 
               CDHPublicKey*& aX, TRequestStatus& aStatus)
    {
    iClient->DHPublicKey(Handle(), aN, aG, aX, aStatus);
    }


// -----------------------------------------------------------------------------
// CDevCertDHAgreement::Agree()
// Agrees a session key given the public key of the other party
// -----------------------------------------------------------------------------
//
void CDevCertDHAgreement::Agree(const CDHPublicKey& iY, HBufC8*& aAgreedKey,
             TRequestStatus& aStatus)
    {
    iClient->DHAgree(Handle(), iY, aAgreedKey, aStatus);
    }


// -----------------------------------------------------------------------------
// CDevCertDHAgreement::CancelAgreement()
// Cancels either a PublicKey or Agree operation
// -----------------------------------------------------------------------------
//
void CDevCertDHAgreement::CancelAgreement()
    {
    // Synchronous, no need to cancel
    }
    
//EOF

