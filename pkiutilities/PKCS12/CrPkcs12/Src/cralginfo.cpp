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
* Description:   This module contains the implementation of CCrAlgInfo class. 
*
*/


//  INCLUDE FILES
#include "cralginfo.h"

#include "random.h"  // Random data.


// -----------------------------------------------------------------------------
// CCrAlgInfo
// Constructor.
// This function constructs CCrAlgInfo object.
// -----------------------------------------------------------------------------
CCrAlgInfo::CCrAlgInfo()
    {
    iType = ECrLAST_SYMM_CRYPTO;
    iAlgorithmObject = NULL;
    iLastPortion = NULL;
    iDigest = NULL;
    iKey = NULL;
    iIV = NULL;
    iEncrypt = ETrue;
    iSingleBlock = ETrue;
    iMode = ECrCBC;
    iPadding = NULL;
    }

// -----------------------------------------------------------------------------
// ~CCrAlgInfo
// Destructor.
// This function destructs CCrAlgInfo object.
// -----------------------------------------------------------------------------
CCrAlgInfo::~CCrAlgInfo()
    {    
    delete iDigest;
    delete iLastPortion;
	delete iKey;    
    delete iIV;

    }

// -----------------------------------------------------------------------------
// CCrAlgInfo::ConstructL
// This function initializes this object's members.
// Parameters: aType                Type of the algorithm
//             aLen                 Length of the key or digest
//             aIVLen               Length of the initialization vector
//                                  of this object.
//             aKey                 Key of this object.
//             aIV                  Initialization vector of this object.
//             aEncrypt = ETrue     Encrypt if true, otherwise decrypt.
//             aSingleBlock = ETrue Single block mode if true.
//             aMode = ECrCBC       Algorithm mode.
// -----------------------------------------------------------------------------
void CCrAlgInfo::ConstructL(
    const TCrAlgorithm aType,
    const TInt         aLen,
    const TInt         aIVLen       /* = 0 */,
    const TDesC8*      aKey         /* = NULL */,
    const TDesC8*      aIV          /* = NULL */,
    const TBool        aEncrypt     /* = ETrue */,
    const TBool        aSingleBlock /* = ETrue */,
    const TCrSymmMode  aMode        /* = ECrCBC */)
    {
    iType = aType;

    if (iType < ECrLAST_DIGEST)
        {
        iDigest = HBufC8::NewL(aLen);
        }
    else
        {
        iEncrypt = aEncrypt;
        iSingleBlock = aSingleBlock;
        iMode = aMode;

        switch (iType)
            {
            case ECrDES3:
            case ECrDES:
            case ECrDES2:
            case ECrDESX:
            case ECrRC2:
            case ECrRC5:
                {
                iLastPortion = HBufC8::NewL(KCrPaddingLength);
                iKey = HBufC8::NewL(aLen);
                iIV = HBufC8::NewL(aIVLen);

                break;
                }
            case ECrRC4:
                {
                iLastPortion = HBufC8::NewL(KCrPaddingLength);
                iKey = HBufC8::NewL(aLen);

                break;
                }
            default:
                {
                break;
                }
            }

        // If given key isn't empty,
        if (aKey && aKey->Length())
            {
            // use it,
            *iKey = *aKey;

            // Check if IV is needed.
            if (iIV != NULL)
                {
                // Use given iv, if any.
                if (aIV && aIV->Length())
                    {
                    *iIV = *aIV;
                    }
                else
                    {
                    // Otherwise null iv.
                    TPtr8 ptrIV = iIV->Des();

                    ptrIV.AppendFill(0, aIVLen);
                    }
                }
            }
        else
            {
/*
            // otherwise create key.
            TPtr8 ptrKey = iKey->Des();
            
            // aLen must have a reasonable value.
            if (aLen < 1 || aLen > ptrKey.MaxLength())
                {
                return; 
                }

            ptrKey.SetLength(aLen);

            GetRandom(ptrKey);
						
			switch (iType)
				{
                case ECrDES3:
                case ECrDES:
                case ECrDES2:
                case ECrDESX:
                    {

                    // Take care that weak keys are not used.
                    // Set parity bits. Check that the key is not weak.
                    // Recreate key until it is not weak.

                    // Key is weak, when weakKey is 1. Then it has to 
					//	be recreated.
                    // Key is acceptable, when weakKey is 0.

                    if (aLen == 24) // 3des
                        {
                        TInt weakKey = 1;
                        while (weakKey != 0)
                            {
                            // Put parity bits to DESkeys.
                            SetParityBits(ptrKey);

                            TPtrC8 firstKeyPtr = ptrKey.Left(8);
                            NC_BYTE* tempKey = CONST_CAST(NC_BYTE*, firstKeyPtr.Ptr());                 
                            weakKey = des_weak_key(tempKey);
                        
                            if (weakKey == 0)
                                {
                                TPtrC8 secondKeyPtr = ptrKey.Mid(8,8);
                                tempKey = CONST_CAST(NC_BYTE*, secondKeyPtr.Ptr());
                                weakKey = des_weak_key(tempKey);
                            
                                if (weakKey == 0)
                                    {
                                    TPtrC8 thirdKeyPtr  = ptrKey.Right(8);
                                    tempKey = CONST_CAST(NC_BYTE*, thirdKeyPtr.Ptr());
                                    weakKey = des_weak_key(tempKey);
                                    }
                                }

                            if (weakKey == 1)
                                {
                                GetRandom(ptrKey);
                                }
                            }
                        }
                                       
                    if (aLen == 8)
                        {
                        // Put parity bits to DESkeys.
                        SetParityBits(ptrKey);

                        NC_BYTE* tempKey = CONST_CAST(NC_BYTE*, iKey->Ptr());
                        TInt weakKey = des_weak_key(tempKey);
                        while (weakKey != 0)
                            {
                            GetRandom(ptrKey);
                            SetParityBits(ptrKey);
                            weakKey = des_weak_key(tempKey );
                            }
                        }

                    break;
                    }
                default:
                    break;
                }

            // Check if IV is needed.
            if (iIV != NULL)
                {
                // create iv.
                TPtr8 ptrIV = iIV->Des();

                ptrIV.SetLength(aIVLen);

                GetRandom(ptrIV);
                }*/
            }
        }
		
    }

// -----------------------------------------------------------------------------
// CCrAlgInfo::NewLC
// This function implements the two-phase construction of this class.
// The function uses standard constructor to reserve memory for
// CCrAlgInfo object, stores a pointer to the object into clean up
// stack, and returns the pointer to the object.
// -----------------------------------------------------------------------------
CCrAlgInfo* CCrAlgInfo::NewLC(
    const TCrAlgorithm aType,
    const TDesC8&      aKey,
    const TDesC8&      aIV,
    const TBool        aEncrypt     /* = ETrue */,
    const TBool        aSingleBlock /* = ETrue */,
    const TCrSymmMode  aMode        /* = ECrCBC */)
    {
    TInt keyLen = aKey.Length();
    TInt ivLen = aIV.Length();

    CCrAlgInfo* self = new (ELeave) CCrAlgInfo();
    CleanupStack::PushL(self);

    // If given key is empty, assume that caller wants us to create key.
    if (keyLen == 0)
        {
        switch (aType)
            {
            case ECrDES3:
                {
                keyLen = 3 * KCrDESKeyLength;

                break;
                }
            case ECrDES:
                {
                keyLen = KCrDESKeyLength;

                break;
                }
            case ECrDES2:
                {
                keyLen = 2 * KCrDESKeyLength;

                break;
                }
            case ECrDESX:
                {
                keyLen = 3 * KCrDESKeyLength;

                break;
                }
            case ECrRC2:
            case ECrRC5:
                {
                keyLen = KCrRCLongKeyLength;

                break;
                }
            case ECrRC4:
                {
                keyLen = KCrRCLongKeyLength;

                break;
                }
            default:
                {
                break;
                }
            }
        }

    // If given iv is empty, assume that caller wants us to create iv.
    if (ivLen == 0)
        {
        switch (aType)
            {
            case ECrDES3:
                {
                ivLen = KCrDESIVLength;

                break;
                }
            case ECrDES:
                {
                ivLen = KCrDESIVLength;

                break;
                }
            case ECrDES2:
                {
                ivLen = KCrDESIVLength;

                break;
                }
            case ECrDESX:
                {
                ivLen = KCrDESIVLength;

                break;
                }
            case ECrRC2:
            case ECrRC5:
                {
                ivLen = KCrRCIVLength;

                break;
                }
            case ECrRC4:
                {
                break;
                }
            default:
                {
                break;
                }
            }
        }

    self->ConstructL(
        aType,
        keyLen,
        ivLen,
        &aKey,
        &aIV,
        aEncrypt,
        aSingleBlock,
        aMode);

    return self; 
    }

// -----------------------------------------------------------------------------
// CCrAlgInfo::NewLC
// This function implements the two-phase construction of this class.
// The function uses standard constructor to reserve memory for
// CCrAlgInfo object, stores a pointer to the object into clean up
// stack, and returns the pointer to the object.
// -----------------------------------------------------------------------------
CCrAlgInfo* CCrAlgInfo::NewLC(const TCrAlgorithm aType)
    {
    TInt len = 0;

    CCrAlgInfo* self = new (ELeave) CCrAlgInfo();
    CleanupStack::PushL(self);

    switch (aType)
        {
        case ECrSHA1:
        case ECrHMAC_SHA1:
        case ECrHMAC_RIPEMD:
        case ECrRIPEMD:
        case ECrSHA:
            {
            len = KCrLongDigestLength;
            break;
            }
        case ECrMD5:
        case ECrMD2:
        case ECrHMAC_MD5:
            {
            len = KCrMediumDigestLength;
            break;
            }
        default:
            {
            break;
            }
        }

    self->ConstructL(aType, len);

    return self; 
    }

// --------------------------------------------------------------------------------
// CCrAlgInfo::NewL
// This function implements the two-phase construction of this class.
// The function reserves memory for CCrAlgInfo object and returns
// pointer to that object. This function uses NewLC to create the object
// and store it to cleanup stack. Finally the object is popped from clean
// up stack.
// --------------------------------------------------------------------------------
CCrAlgInfo* CCrAlgInfo::NewL(
    const TCrAlgorithm aType,
    const TDesC8&      aKey,
    const TDesC8&      aIV,
    const TBool        aEncrypt     /* = ETrue */,
    const TBool        aSingleBlock /* = ETrue */,
    const TCrSymmMode  aMode        /* = ECrCBC */)
    {
    CCrAlgInfo* self = NewLC(
        aType,
        aKey,
        aIV,
        aEncrypt,
        aSingleBlock,
        aMode);

    CleanupStack::Pop();

    return self; 
    }

// --------------------------------------------------------------------------------
// CCrAlgInfo::NewL
// This function implements the two-phase construction of this class.
// The function reserves memory for CCrAlgInfo object and returns
// pointer to that object. This function uses NewLC to create the object
// and store it to cleanup stack. Finally the object is popped from clean
// up stack.
// --------------------------------------------------------------------------------
CCrAlgInfo* CCrAlgInfo::NewL(const TCrAlgorithm aType)
    {
    CCrAlgInfo* self = NewLC(aType);

    CleanupStack::Pop();

    return self; 
    }

// --------------------------------------------------------------------------------
// CCrAlgInfo::SetParityBits
// Sets parity bits to des keys.
// --------------------------------------------------------------------------------
TCrStatus CCrAlgInfo::SetParityBits(TPtr8 aPtrKey)
    {
    TInt len = aPtrKey.Length();
    if (len < 1 ) // len should be positive
        {
        return KCrNotInitialized; 
        }

    for (TInt i = 0; i < len; i++)
        {
        TUint temp = aPtrKey[i];
        TInt parity = 0; 
        for (TInt j = 0; j < 7; j++)
            {
            temp >>= 1;
            if (temp & 0x01) 
                { 
                parity++; 
                } 
            } 

        if ((parity % 2) == 0)
            { 
            // even parity, change least significant bit to one.
              aPtrKey[i] |= 0x01; // or
            }
        else
            {
            // Change least significant bit to zero.
            aPtrKey[i] &= 0xfe; // and 
            }
        } 
    return KCrOK;
    }

// End Of File
