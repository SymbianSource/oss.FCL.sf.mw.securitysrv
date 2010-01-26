/*
* Copyright (c) 2000, 2004 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   This file contains the implementation of CCrCrypto class. 
*
*/



//  INCLUDE FILES
#include "crcrypto.h"
#include <e32def.h>  // REINTERPRET_CAST
#include "crdata.h"
#include <bigint.h>  // Big integer.
#include <hash.h>
#include <symmetric.h>


// -----------------------------------------------------------------------------
// CCrCrypto
// Constructor.
// -----------------------------------------------------------------------------
CCrCrypto::CCrCrypto()
    : iAlgorithmInfos(0)
    {
    }

// -----------------------------------------------------------------------------
// ~CCrCrypto
// Destructor.
// -----------------------------------------------------------------------------
CCrCrypto::~CCrCrypto()
    {
    Reset();

    if( iAlgorithmInfos )
        {
        iAlgorithmInfos->Reset();
        delete iAlgorithmInfos;
        iAlgorithmInfos = NULL;
        }
    }

// -----------------------------------------------------------------------------
// CCrCrypto::ConstructL
// This function initializes this object's members.
// -----------------------------------------------------------------------------
void CCrCrypto::ConstructL()
    {
    iAlgorithmInfos = new (ELeave) CArrayPtrFlat<CCrAlgInfo>(1);
    }

// -----------------------------------------------------------------------------
// CCrCrypto::NewLC
// -----------------------------------------------------------------------------
CCrCrypto* CCrCrypto::NewLC()
    {
    CCrCrypto* self = new (ELeave) CCrCrypto();
    CleanupStack::PushL(self);

    self->ConstructL();

    return self; 
    }

// -----------------------------------------------------------------------------
// CCrCrypto::NewL
// -----------------------------------------------------------------------------
CCrCrypto* CCrCrypto::NewL()
    {
    CCrCrypto* self = NewLC();
    CleanupStack::Pop();

    return self; 
    }

// -----------------------------------------------------------------------------
// CCrCrypto::Reset
// Reset all algorithms initialized into this object and free
// memory associated to them. Note that GetDigest etc. functions are
// meaningles after this until new ones are initialized and finalized.
// -----------------------------------------------------------------------------
void CCrCrypto::Reset()
    {
    TInt        i = 0;
    TInt        size = 0;
    CCrAlgInfo* algInfo = 0;

    size = iAlgorithmInfos->Count();

    for (i = 0; i < size; i++)
        {    
        algInfo = (*iAlgorithmInfos)[i];

		if (algInfo)
			{
			switch (algInfo->iType)
				{
				case ECrDES2:
				case ECrDES3:
					{
                    if (algInfo->iAlgorithmObject)
                        {
						delete algInfo->iAlgorithmObject;
                        algInfo->iAlgorithmObject = NULL;
                        }
                    if (algInfo->iPadding)
                        {
                        delete algInfo->iPadding;
                        algInfo->iPadding = NULL;
                        }

                    break;
                    }
				default:
					{
                    if (algInfo->iAlgorithmObject)
                        {
                        delete algInfo->iAlgorithmObject;
                        algInfo->iAlgorithmObject = NULL;
                        }

                    if (algInfo->iPadding)
                        {
                        delete algInfo->iPadding;
                        algInfo->iPadding = NULL;
                        }

                    break;
					}
                }
            }
        }

    iAlgorithmInfos->ResetAndDestroy();
    }

// -----------------------------------------------------------------------------
// CCrCrypto::InitCrypt3DESL
// Initialize encryption or decryption with 3DES algorithm.
// -----------------------------------------------------------------------------
TCrStatus CCrCrypto::InitCrypt3DESL(
    const TDesC8& aKey1,
    const TDesC8& aKey2,
    const TDesC8& aKey3,
    const TDesC8& aIV,              
    TBool         aEncrypt,   // ETrue  
    TCrSymmMode   aMode,      // ECrCBC
    TPaddingRule  /*aPadRule*/)   // ECrPKCS1
    {
    // Number of items pushed to CleanupStack
    TUint pushedToCStack = 0;

    // Combined keys.
    HBufC8* keys = HBufC8::NewLC(
        aKey1.Length() + aKey2.Length() + aKey3.Length());
    ++pushedToCStack;

    TPtr8 ptrKeys = keys->Des();

    // Combine given keys to be able to give them to algorithm info object.
    ptrKeys = aKey1;
    ptrKeys.Append(aKey2);
    ptrKeys.Append(aKey3);

    // Create new algorithm info object.
    CCrAlgInfo* algInfo = CCrAlgInfo::NewLC(
        ECrDES3,
        *keys,
        aIV,
        aEncrypt,
        ETrue,
        aMode);
    ++pushedToCStack;
    
    switch (aMode)
        {
        case ECrCBC:
            {
			    if (aEncrypt)
				    {
				    C3DESEncryptor* tripleDes = C3DESEncryptor::NewL(*algInfo->iKey);
				    CleanupStack::PushL(tripleDes);

				    CModeCBCEncryptor* cbcEncryptor = CModeCBCEncryptor::NewL(tripleDes, aIV);
				    CleanupStack::Pop(tripleDes); // CModeCBCEncryptor takes care of tripleDes now
				    CleanupStack::PushL(cbcEncryptor);

				    CPaddingPKCS7* padding = CPaddingPKCS7::NewL(tripleDes->BlockSize());							
				    CleanupStack::PushL(padding);

				    CBufferedEncryptor* encryptor = CBufferedEncryptor::NewL(cbcEncryptor, padding);
				    CleanupStack::Pop(2); // CBufferedEncryptor takes care of freeing	

				    algInfo->iAlgorithmObject = encryptor;
				    }
			    else
				    {			
				    C3DESDecryptor* tripleDes = C3DESDecryptor::NewL(*algInfo->iKey);
				    CleanupStack::PushL(tripleDes);

				    CModeCBCDecryptor* cbcDecryptor = CModeCBCDecryptor::NewL(tripleDes, aIV);
				    CleanupStack::Pop(tripleDes); // CModeCBCEncryptor takes care of tripleDes now
				    CleanupStack::PushL(cbcDecryptor);

				    CPaddingPKCS7* padding = CPaddingPKCS7::NewL(tripleDes->BlockSize());
				    CleanupStack::PushL(padding);

				    CBufferedDecryptor* decryptor = CBufferedDecryptor::NewL(cbcDecryptor, padding);
				    CleanupStack::Pop(2); // CBufferedDecryptor takes care of freeing	

				    algInfo->iAlgorithmObject = decryptor;							
				    }	                    
            break;
            }
        case ECrCFB:
            {
		    return KCrCrypto | KCrUnknownMode;
            }
        case ECrECB:
            {
            return KCrCrypto | KCrUnknownMode;
            }
        case ECrOFB:
            {
		    return KCrCrypto | KCrUnknownMode;
            }
        default:
            {
            return KCrCrypto | KCrUnknownMode;
            }
        }

    CleanupStack::PushL(algInfo->iAlgorithmObject);
    ++pushedToCStack;

    // Append new algorithm info object into member set.
    iAlgorithmInfos->AppendL(algInfo);
 
    CleanupStack::Pop(pushedToCStack);

    delete keys;
    keys = NULL;

    return KCrOK;
    }

// -----------------------------------------------------------------------------
// CCrCrypto::InitCryptRC2L
// Initialize encryption or decryption with RC2 algorithm.
// -----------------------------------------------------------------------------
TCrStatus CCrCrypto::InitCryptRC2L(
    const TDesC8& aKey,             
    const TDesC8& aIV,
    TBool         aEncrypt,          // ETrue
    TInt          aEffectiveKeyLen,  // If 0 given, key len is used.
    TCrSymmMode   aMode,             // ECrCBC
    TPaddingRule  /*aPadRule */)          // ECrPKCS1
    {
    // Number of items pushed to CleanupStack
    TUint pushedToCStack = 0;

    // Create new algorithm info object.
    CCrAlgInfo* algInfo = CCrAlgInfo::NewLC(
        ECrRC2,
        aKey,
        aIV,
        aEncrypt,
        ETrue,
        aMode);
    ++pushedToCStack;

    // If given effective key length is zero, use key length.
    if (aEffectiveKeyLen == 0)
        {
        aEffectiveKeyLen = algInfo->iKey->Length() * 8;
        }
    
    // Create cipher object.
    switch (aMode)
        {
        case ECrCBC:
            {
			if (aEncrypt)
				{
				CRC2Encryptor* rc2 = CRC2Encryptor::NewL(*algInfo->iKey, aEffectiveKeyLen);
				CleanupStack::PushL(rc2);

				CModeCBCEncryptor* cbcEncryptor = CModeCBCEncryptor::NewL(rc2, aIV);
				CleanupStack::Pop(rc2); // CModeCBCEncryptor takes care of rc2 now
				CleanupStack::PushL(cbcEncryptor);

				CPaddingPKCS7* padding = CPaddingPKCS7::NewL(rc2->BlockSize());
				CleanupStack::PushL(padding);

				CBufferedEncryptor* encryptor = CBufferedEncryptor::NewL(cbcEncryptor, padding);
				CleanupStack::Pop(2); // CBufferedEncryptor takes care of freeing

				algInfo->iAlgorithmObject = encryptor;							
				}
			else
				{			
				CRC2Decryptor* rc2 = CRC2Decryptor::NewL(*algInfo->iKey, aEffectiveKeyLen);
				CleanupStack::PushL(rc2);

				CModeCBCDecryptor* cbcDecryptor = CModeCBCDecryptor::NewL(rc2, aIV);
				CleanupStack::Pop(rc2); // CModeCBCEncryptor takes care of rc2 now
				CleanupStack::PushL(cbcDecryptor);

				CPaddingPKCS7* padding = CPaddingPKCS7::NewL(rc2->BlockSize());
				CleanupStack::PushL(padding);

				CBufferedDecryptor* decryptor = CBufferedDecryptor::NewL(cbcDecryptor, padding);
				CleanupStack::Pop(2); // CBufferedDecryptor takes care of freeing

				algInfo->iAlgorithmObject = decryptor;							
				}	                    
            algInfo->iMode = ECrCBC;
            break;
            }
        case ECrCFB:
            {
            return KCrCrypto | KCrUnknownMode;
            }
        case ECrECB:
            {          
            return KCrCrypto | KCrUnknownMode;
            }
        case ECrOFB:
            {
			return KCrCrypto | KCrUnknownMode;
            }
        default:
            {
            return KCrCrypto | KCrUnknownMode;
            }
        }

    CleanupStack::PushL(algInfo->iAlgorithmObject);
    ++pushedToCStack;


    // Append new algorithm info object into member set.
    iAlgorithmInfos->AppendL(algInfo);

    CleanupStack::Pop(pushedToCStack);
 
    return KCrOK;
    }

// -----------------------------------------------------------------------------
// CCrCrypto::InitDigestL
// Initialize message digest with MD2 algorithm.
// -----------------------------------------------------------------------------
TCrStatus CCrCrypto::InitDigestL(TCrAlgorithm aAlgorithm)    
    {
    // Number of items pushed to CleanupStack
    TUint pushedToCStack = 0;

    // Create new algorithm info object.
    CCrAlgInfo *algInfo = CCrAlgInfo::NewLC(aAlgorithm);
    pushedToCStack++;

    switch(aAlgorithm)
        {
        case ECrSHA1:
            {
            // Create digest object.
            algInfo->iAlgorithmObject  = CSHA1::NewL();
            break;
            }
        case ECrMD5:
            {
            // Create digest object.
            algInfo->iAlgorithmObject  = CMD5::NewL();
            break;
            }
        case ECrMD2:
            {
            // Create digest object.
            algInfo->iAlgorithmObject = CMD2::NewL();
            break;
            }
        case ECrSHA:
            {  
            // Create digest object.
            algInfo->iAlgorithmObject  = CSHA::NewL();
            break;
            }
        default:
            {
            return KCrCrypto | KCrNotSupportedAlg;
            }
        }

    CleanupStack::PushL(algInfo->iAlgorithmObject);
    pushedToCStack++;

    // Append new algorithm info object into member set.
    iAlgorithmInfos->AppendL(algInfo);

    CleanupStack::Pop(pushedToCStack);

    return KCrOK;
    }

// -----------------------------------------------------------------------------
// CCrCrypto::InitDigestHMACL
// Initialize message digest with HMAC algorithm.
// -----------------------------------------------------------------------------
TCrStatus CCrCrypto::InitDigestHMACL(
    const TDesC8& aKey, 
    TCrAlgorithm aDigestAlg)     
    {
    // Number of items pushed to CleanupStack
    TUint pushedToCStack = 0;

    TCrAlgorithm hmacDigest;

    switch (aDigestAlg)
        {
        case ECrSHA1:
            {
            hmacDigest = ECrHMAC_SHA1;

            break;
            }
        case ECrMD5:
            {
            hmacDigest = ECrHMAC_MD5;

            break;
            }
        default:
            {
            return KCrCrypto | KCrNotSupportedAlg;
            }
        }

    // Create new algorithm info object.
    CCrAlgInfo *algInfo = CCrAlgInfo::NewLC(hmacDigest);
    pushedToCStack++;

    CMessageDigest* digest = 0;

    switch(aDigestAlg)
        {
        case ECrSHA1:
            {
            // Create digest object.
            digest = CSHA1::NewL();
            break;
            }
        case ECrMD5:
            {
            // Create digest object.
            digest = CMD5::NewL();
            break;
            }
        case ECrMD2:
            {
            // Create digest object.
            digest = CMD2::NewL();
            break;
            }
        case ECrSHA:
            {   
            // Create digest object.
            digest = CSHA::NewL();
            break;  
            }   
        default:
            {
            return KCrCrypto | KCrNotSupportedAlg;
            }
        }
    CleanupStack::PushL(digest);
    ++pushedToCStack;
    // Create digest object.
    algInfo->iAlgorithmObject  = CHMAC::NewL(aKey, digest);
    CleanupStack::PushL(algInfo->iAlgorithmObject);
    ++pushedToCStack;
    
    // Append new algorithm info object into member set.
    iAlgorithmInfos->AppendL(algInfo);

    CleanupStack::Pop(pushedToCStack);

    return KCrOK;
    }


// -----------------------------------------------------------------------------
// CCrCrypto::ProcessL
// Process given source data with initialized crypto operations.
// If symmetric crypto is initialized sets to aTrg encrypted
// or decrypted data without last portion. If aProcessFinalBlock
// is ETrue, appends also last portion. If only digest algorithm
// is initialized, aTrg is not used.
// -----------------------------------------------------------------------------
TCrStatus CCrCrypto::ProcessL(const TDesC8& aSrc, TDes8& aTrg) 
    {
    TInt              i, size;
    TUint             pushedToCStack = 0;
    CCrAlgInfo* algInfo = 0;

    size = iAlgorithmInfos->Count();

    for (i = 0; i < size; i++)
        {
        algInfo = (*iAlgorithmInfos)[i];

        // Message digest algorithm
        if (algInfo->iType < ECrLAST_DIGEST)
            {
            // Casting to right type
            CMessageDigest* digest = STATIC_CAST(
                CMessageDigest*, algInfo->iAlgorithmObject);

            // Store the digest to buf
            //algInfo->iDigest = HBufC8::NewL(digest->HashSize());
			TPtr8 ptr = algInfo->iDigest->Des();
			ptr.Copy(digest->Hash(aSrc));
            }   
        // Symmetric crypto algorithm
        else if (algInfo->iType > ECrLAST_DIGEST && algInfo->iType < ECrLAST_SYMM_CRYPTO)
            {
            // Casting to right type
            CSymmetricCipher* cipherSymm =
                STATIC_CAST(CSymmetricCipher*, algInfo->iAlgorithmObject);


			// This is not incremental
			cipherSymm->ProcessFinalL(aSrc, aTrg);			
			
            CleanupStack::Pop(pushedToCStack);
            }
        else 
            {
            return KCrCrypto | KCrNotSupportedAlg;
            }
        }
    return KCrOK;
    }

// -----------------------------------------------------------------------------
// CCrCrypto::FinalCryptL
// Finalize symmetric algorithms objects.
// -----------------------------------------------------------------------------
TCrStatus CCrCrypto::FinalCryptL(TDes8& aTrg)
    {
    TInt        i, size;
    CCrAlgInfo *algInfo = 0;
    TCrStatus   status = KCrOK;
    size = iAlgorithmInfos->Count();

    for (i = 0; i < size; i++)
        {
        algInfo = (*iAlgorithmInfos)[i];

        if (algInfo->iType > ECrLAST_DIGEST && 
            algInfo->iType < ECrLAST_SYMM_CRYPTO) 
            { 
            TUint8 pushedToCStack = 0;

            CSymmetricCipher* cipherSymm = 
                STATIC_CAST(CSymmetricCipher*, algInfo->iAlgorithmObject);
            
            HBufC8 *lastBlock =
                HBufC8::NewLC(algInfo->iLastPortion->Size());
            pushedToCStack++;

            TPtr8 ptrLastBlock = lastBlock->Des();

		    if (cipherSymm->MaxFinalOutputLength(ptrLastBlock.Size()) > 0)
		    {
			    cipherSymm->ProcessFinalL(ptrLastBlock, aTrg);                    
		    }

            CleanupStack::Pop(pushedToCStack);

            delete lastBlock;
            lastBlock = 0;

            delete cipherSymm;
            cipherSymm = 0;
            algInfo->iAlgorithmObject = 0;
        
            }                 
        }
    return status;
    }


TCrStatus CCrCrypto::FinalDigest(TDes8& aTrg)
    {
    TInt              i, size;
    CCrAlgInfo *algInfo = 0;
    TCrStatus         status = KCrOK;

    size = iAlgorithmInfos->Count();

    for (i = 0; i < size; i++)
        {
        algInfo = (*iAlgorithmInfos)[i];

        if ((algInfo->iType < ECrLAST_DIGEST) && (status == KCrOK))
            {
    
            aTrg.Copy(*algInfo->iDigest);

            CMessageDigest* digest = STATIC_CAST(
                CMessageDigest*, algInfo->iAlgorithmObject);

            delete digest;
            digest = 0;
            algInfo->iAlgorithmObject = 0;
            }
        }
    return status;
    }



// -----------------------------------------------------------------------------
// CCrCrypto::RemoveLastBlock
// Checks that aOriginSrc length is multiple of the block size, If not, 
// removes data from the end so that it is multiple of the block size.
// Parameters:     aSrc            Original data to remove the last block 
//                 aBlockSize      Size of the block
//                 alginfo         Pointer to object where last portion is 
//                                 stored.
// Return Values:  true if size of aOriginSrc is bigger than blockSize
//                 otherwise false. If false the whole aOriginSrc is stored 
//                 to alginfo.
// -----------------------------------------------------------------------------
TInt CCrCrypto::RemoveLastBlock(
    TDesC8&           aSrc,
    const TInt        aBlockSize,
    CCrAlgInfo*       algInfo)    
    { 
    TUint number_of_blocks = aSrc.Size() / aBlockSize;
    TUint size_of_last_block = aSrc.Size() % aBlockSize;     

    TUint size_of_checked; 
    
    // If aOriginSrc's size is smaller or equal than aBlocksize
    if (number_of_blocks == 0 || (number_of_blocks == 1 && size_of_last_block == 0))
        {
        *algInfo->iLastPortion = aSrc;
        
        return false;
        }
    else if (size_of_last_block == 0 && number_of_blocks > 0)
        {
        // 3des fix begins:
        // Don't do anything if size of data already is multiple of
        // blocksize. Otherwise padding will be ruined. Return true anyway.
        if(!algInfo->iEncrypt)
            {
            size_of_checked = aSrc.Size() - aBlockSize;
        
            *algInfo->iLastPortion = aSrc.Right(aBlockSize);

            aSrc = aSrc.Left(size_of_checked);
            }
        
        // 3des fix ends.

        return true;
        }
    else
        {
        size_of_checked = aSrc.Size() - size_of_last_block;
        
        *algInfo->iLastPortion = aSrc.Right(size_of_last_block);

        aSrc = aSrc.Left(size_of_checked);

        return true;
        }     
    }

// -----------------------------------------------------------------------------
// CCrCrypto::DeriveKeyPKCS12L
// Derives key(s) or IV vector from password, salt and iterarion count.
// Return Values:  KCrOK
//                 KCrNotSupportedAlg
//                 KCrUndefinedLibrary
//                 KCrUnknownLibrary
//                 KCrUnknownMode
//                 KCrErrorGeneral
// -----------------------------------------------------------------------------
TCrStatus CCrCrypto::DeriveKeyPKCS12L(
    const TDesC8&   aPassword, 
    const TDesC8&   aSalt,
    const TInt      aIterationCount,
    TCrAlgorithm    aHashFunc,
    const TUint8    aID,
    const TInt      aNumberOfBytes,
    TDes8&          aTrg)               // Output data, possible keys and IV
    {
    TInt remainder = 0;
    TInt rounds = 0;
    TInt pushedToCStack = 0;
    TInt inputSize = MesDigestInputSize(aHashFunc);
    TInt outputSize = MesDigestOutputSize(aHashFunc);

    // Step 1: Construct D by concatenating copies of ID.
    // Construct a string D
    HBufC8* D_buf = HBufC8::NewLC(inputSize);
    ++pushedToCStack;

    TPtr8 D_ptr = D_buf->Des();
	TInt i(0);
    for (i = 0; i < inputSize; ++i)
        {
        D_ptr.Append(aID);
        }

    // Step 2, 3, 4:
    TInt s_length = 0;
    TInt p_length = 0;

    s_length = inputSize * ((aSalt.Size() + inputSize - 1) / inputSize);
    p_length = inputSize * ((aPassword.Size() + inputSize - 1) / inputSize);
    
    HBufC8* I_buf = HBufC8::NewLC(s_length + p_length);
    ++pushedToCStack;
    TPtr8 I_ptr = I_buf->Des();

    if (aSalt.Size() != 0)
        {
        rounds = s_length / aSalt.Size();
        for (i = 0; i < rounds; ++i)
            {
            I_ptr.Append(aSalt);
            }
        remainder = s_length % aSalt.Size();
        if (remainder != 0)
            {
            I_ptr.Append(aSalt.Ptr(), remainder);
            }
        }
             
    
    if (aPassword.Size() != 0)
        {
        rounds = p_length / aPassword.Size();
        for (i = 0; i < rounds; ++i)
            {
            I_ptr.Append(aPassword);
            }
        remainder = p_length % aPassword.Size();
        if (remainder != 0)
            {
            I_ptr.Append(aPassword.Ptr(), remainder);
            }
        }

    // Step 5: Set c.
    TInt c = 0;
    c = (aNumberOfBytes + outputSize - 1) / outputSize; 
    
    // Step 6: Loop
    TCrStatus status = KCrCrypto | KCrErrorGeneral;
    
    CCrCrypto* hash = 0;

    HBufC8* B_buf = HBufC8::NewLC(inputSize);
    ++pushedToCStack;
    TPtr8 B_ptr = B_buf->Des();

    HBufC8* A_buf = HBufC8::NewLC(D_ptr.Size() + I_ptr.Size());    
    ++pushedToCStack;
    TPtr8 A_ptr = A_buf->Des();
    
    TInt j = 0;
    RInteger B_int;
    RInteger Ij_int;
    TInt N = aNumberOfBytes;

    rounds = inputSize / outputSize;
    remainder = inputSize % outputSize;

    for (i = 0; i < c; ++i)
        {
        A_ptr.Zero();
        A_ptr.Append(D_ptr);
        A_ptr.Append(I_ptr);
        
        hash = CCrCrypto::NewLC();

        for (TInt ii = 0; ii < aIterationCount; ++ii)
            {
            status = hash->InitDigestL(aHashFunc);
            status = hash->ProcessL(A_ptr, A_ptr); 
            A_ptr.Zero();
            status = hash->FinalDigest(A_ptr);
            hash->Reset();
            }
        
        CleanupStack::PopAndDestroy(); // hash

        if (outputSize < N)
            {
            aTrg.Append(A_ptr);
            }
        else
            {
            aTrg.Append(A_ptr.Ptr(), N);
            }

        if (outputSize >= N)
            {
            status = KCrOK;
            break;
            }

        N -= outputSize;

        rounds = inputSize / A_ptr.Size();
        remainder = inputSize % A_ptr.Size();
        for (j = 0; j < rounds; ++j)
            {
            B_ptr.Append(A_ptr);
            }
        if (remainder != 0)
            {
            B_ptr.Append(A_ptr.Ptr(), remainder);
            }
        
        B_int = RInteger::NewL(B_ptr);  
		CleanupStack::PushL(B_int);
        
        B_int += 1;

        for (j = 0; j < I_ptr.Size(); j += inputSize)
            {
            Ij_int = RInteger::NewL(I_ptr.Mid(j, inputSize));
			CleanupStack::PushL(Ij_int);
            Ij_int += B_int;
            
            I_ptr.Replace(j, inputSize, Ij_int.BufferLC()->Right(inputSize));

            CleanupStack::PopAndDestroy(2); // Ij_int, BufferLC            
            }

        CleanupStack::PopAndDestroy(); // B_int
        }

    CleanupStack::PopAndDestroy(pushedToCStack); 

    return status;          
}

// -----------------------------------------------------------------------------
// CCrCrypto::MesDigestInputSize
// Returns input size of the message digest algorithm. 
// Return Values:      Input size of the message digest algorithm in bytes.
//                     If unknown algorithm returns -1.      
// -----------------------------------------------------------------------------
TInt CCrCrypto::MesDigestInputSize(TCrAlgorithm aDigestAlg)
    {
    switch(aDigestAlg)
        {
        case ECrSHA1:
        case ECrMD5:
        case ECrMD2:
            {
            return KDigestInputSize;
            }
        default:
            {
            return -1;
            }
        }
    }

// -----------------------------------------------------------------------------
// CCrCrypto::MesDigestOutputSize
// Returns output size of the message digest algorithm. 
// Parameters:         aDigestAlg           message digest algortihm
// Return Values:      Output size of the message digest algorithm in bytes.
//                     If unknown algorithm returns -1.      
// -----------------------------------------------------------------------------
TInt CCrCrypto::MesDigestOutputSize(TCrAlgorithm aDigestAlg)
    {
    switch(aDigestAlg)
        {
        case ECrSHA1:
            {
            return KCrLongDigestLength;
            }
        case ECrMD5:
        case ECrMD2:
            {
            return KCrMediumDigestLength;
            }
        default:
            {
            return -1;
            }
        }
    }

// End Of Line


