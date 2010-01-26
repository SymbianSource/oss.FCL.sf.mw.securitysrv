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
* Description:   Implementation of DevCertKeyStreamUtils
*
*/


#include <asymmetrickeys.h>
#include "DevCertKeyStreamUtils.h"
#include "DevCertKeyEncryptor.h"

// ======== GLOBAL FUNCTIONS ========

// ---------------------------------------------------------------------------
// ExternalizeL()
// RSA Public Key
// ---------------------------------------------------------------------------
//
void ExternalizeL(const CRSAPublicKey& aKey, RWriteStream& aStream)
    {
    aStream << aKey.N() << aKey.E();
    }


// ---------------------------------------------------------------------------
// ExternalizeL()
// RSA Private Key
// ---------------------------------------------------------------------------
//
void ExternalizeL(const CRSAPrivateKey& aData, RWriteStream& aStream)
    {
    CDevCertKeyEncryptor* encryptor = CDevCertKeyEncryptor::NewLC();
    if( encryptor->IsPluginExistL() )
        {
        MKeyEncryptor* encrypt = encryptor->CreateImplementationL();  

        //aStream << aData.N();
        EncryptAndStoreL(aData.N(), aStream, encrypt );

        // Check the incoming RSA private key (standard or CRT) 
        TRSAPrivateKeyType keyType = aData.PrivateKeyType();
        aStream.WriteInt32L((TInt32)keyType);

        if (EStandard==keyType)
            {
            const CRSAPrivateKeyStandard& key = static_cast<const CRSAPrivateKeyStandard&>(aData);
            //aStream << key.D();
            EncryptAndStoreL(key.D(), aStream, encrypt );
            }
        else if (EStandardCRT==keyType)
            {
            const CRSAPrivateKeyCRT& key = static_cast<const CRSAPrivateKeyCRT&>(aData);
            //aStream << key.P() << key.Q() << key.DP() << key.DQ() << key.QInv();
            EncryptAndStoreL(key.P(), aStream, encrypt );
            EncryptAndStoreL(key.Q(), aStream, encrypt );
            EncryptAndStoreL(key.DP(), aStream, encrypt );
            EncryptAndStoreL(key.DQ(), aStream, encrypt );
            EncryptAndStoreL(key.QInv(), aStream, encrypt );
            }
        else
            { 
            User::Leave(KErrNotSupported);
            }   
        }
    else
        {
        aStream << aData.N();

        // Check the incoming RSA private key (standard or CRT) 
        TRSAPrivateKeyType keyType = aData.PrivateKeyType();
        aStream.WriteInt32L((TInt32)keyType);

        if (EStandard==keyType)
            {
            const CRSAPrivateKeyStandard& key = static_cast<const CRSAPrivateKeyStandard&>(aData);
            aStream << key.D();
            }
        else if (EStandardCRT==keyType)
            {
            const CRSAPrivateKeyCRT& key = static_cast<const CRSAPrivateKeyCRT&>(aData);
            aStream << key.P() << key.Q() << key.DP() << key.DQ() << key.QInv();
            }
        else
            { 
            User::Leave(KErrNotSupported);
            }
        } 
    CleanupStack::PopAndDestroy( encryptor );        
    }


// ---------------------------------------------------------------------------
// ExternalizeL()
// DSA Public Key
// ---------------------------------------------------------------------------
//
void ExternalizeL(const CDSAPublicKey& aKey, RWriteStream& aStream)
    {
    aStream << aKey.P() << aKey.Q() << aKey.G() << aKey.Y();
    }


// ---------------------------------------------------------------------------
// ExternalizeL()
// DSA Private Key
// ---------------------------------------------------------------------------
//
void ExternalizeL(const CDSAPrivateKey& aKey, RWriteStream& aStream)
    {
    CDevCertKeyEncryptor* encryptor = CDevCertKeyEncryptor::NewLC();
    if( encryptor->IsPluginExistL() )
        {
        MKeyEncryptor* encrypt = encryptor->CreateImplementationL();  
        //aStream << aKey.P() << aKey.Q() << aKey.G() << aKey.X();
        EncryptAndStoreL(aKey.P(), aStream, encrypt );
        EncryptAndStoreL(aKey.Q(), aStream, encrypt );
        EncryptAndStoreL(aKey.G(), aStream, encrypt );
        EncryptAndStoreL(aKey.X(), aStream, encrypt );
        }
    else
        {
        aStream << aKey.P() << aKey.Q() << aKey.G() << aKey.X();  
        }
    CleanupStack::PopAndDestroy(encryptor);        
    }


// ---------------------------------------------------------------------------
// CreateL()
// RSA Public key
// ---------------------------------------------------------------------------
//
void CreateL(RReadStream& aStream, CRSAPublicKey*& aOut)
    {
    RInteger N, keyPublicExp;
    CreateLC(aStream, N);
    CreateLC(aStream, keyPublicExp);

    aOut = CRSAPublicKey::NewL(N, keyPublicExp);

    CleanupStack::Pop(2, &N); // keyPublicExp, N
    }


// ---------------------------------------------------------------------------
// CreateL()
// RSA Private key
// ---------------------------------------------------------------------------
//
void CreateL(RReadStream& aStream, CRSAPrivateKey*& aOut)
    {
    CDevCertKeyEncryptor* encryptor = CDevCertKeyEncryptor::NewLC();
    if( encryptor->IsPluginExistL() )
        {
        MKeyEncryptor* encrypt = encryptor->CreateImplementationL();

        RInteger privateN;
        DecryptAndCreateLC(aStream, privateN, encrypt);

        TRSAPrivateKeyType keyType = EStandard;
        keyType = (TRSAPrivateKeyType)aStream.ReadInt32L();

        if (EStandard==keyType)
            {
            RInteger D;
            DecryptAndCreateLC(aStream, D, encrypt);

            aOut = CRSAPrivateKeyStandard::NewL(privateN, D);

            CleanupStack::Pop(&D);
            }
        else if (EStandardCRT==keyType)
            {
            RInteger p, q, dP, dQ, qInv;
            DecryptAndCreateLC(aStream, p, encrypt);
            DecryptAndCreateLC(aStream, q, encrypt);
            DecryptAndCreateLC(aStream, dP, encrypt);
            DecryptAndCreateLC(aStream, dQ, encrypt);
            DecryptAndCreateLC(aStream, qInv, encrypt);

            aOut = CRSAPrivateKeyCRT::NewL(privateN, p, q, dP, dQ, qInv);

            CleanupStack::Pop(5, &p);
            }
        else
            {
            User::Leave(KErrNotSupported);
            }

        CleanupStack::Pop(&privateN);
        }
    else
        {
        RInteger privateN;
        CreateLC(aStream, privateN);

        TRSAPrivateKeyType keyType = EStandard;
        keyType = (TRSAPrivateKeyType)aStream.ReadInt32L();

        if (EStandard==keyType)
            {
            RInteger D;
            CreateLC(aStream, D);

            aOut = CRSAPrivateKeyStandard::NewL(privateN, D);

            CleanupStack::Pop(&D);
            }
        else if (EStandardCRT==keyType)
            {
            RInteger p, q, dP, dQ, qInv;
            CreateLC(aStream, p);
            CreateLC(aStream, q);
            CreateLC(aStream, dP);
            CreateLC(aStream, dQ);
            CreateLC(aStream, qInv);

            aOut = CRSAPrivateKeyCRT::NewL(privateN, p, q, dP, dQ, qInv);

            CleanupStack::Pop(5, &p);
            }
        else
            {
            User::Leave(KErrNotSupported);
            }

        CleanupStack::Pop(&privateN); 
        } 
    CleanupStack::PopAndDestroy(encryptor);       
    }


// ---------------------------------------------------------------------------
// CreateL()
// DSA Public key
// ---------------------------------------------------------------------------
//
void CreateL(RReadStream& aStream, CDSAPublicKey*& aOut)
    {
    RInteger P, Q, G, Y;
    CreateLC(aStream, P);
    CreateLC(aStream, Q);
    CreateLC(aStream, G);
    CreateLC(aStream, Y);

    aOut = CDSAPublicKey::NewL(P, Q, G, Y);

    CleanupStack::Pop(4, &P);
    }


// ---------------------------------------------------------------------------
// CreateL()
// DSA Private key
// ---------------------------------------------------------------------------
//
void CreateL(RReadStream& aStream, CDSAPrivateKey*& aOut)
    {
    CDevCertKeyEncryptor* encryptor = CDevCertKeyEncryptor::NewLC();
    if( encryptor->IsPluginExistL() )
        {
        MKeyEncryptor* encrypt = encryptor->CreateImplementationL();
        RInteger P, Q, G, X;
        DecryptAndCreateLC(aStream, P, encrypt);
        DecryptAndCreateLC(aStream, Q, encrypt);
        DecryptAndCreateLC(aStream, G, encrypt);
        DecryptAndCreateLC(aStream, X, encrypt);

        aOut = CDSAPrivateKey::NewL(P, Q, G, X);

        CleanupStack::Pop(4, &P);
        }
    else
        {
        RInteger P, Q, G, X;
        CreateLC(aStream, P);
        CreateLC(aStream, Q);
        CreateLC(aStream, G);
        CreateLC(aStream, X);

        aOut = CDSAPrivateKey::NewL(P, Q, G, X);

        CleanupStack::Pop(4, &P); 
        }
    CleanupStack::PopAndDestroy(encryptor);        
    }

//EOF

