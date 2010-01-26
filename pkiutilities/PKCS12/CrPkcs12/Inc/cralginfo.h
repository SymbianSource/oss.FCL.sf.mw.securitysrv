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
* Description:   This file contains the header of CCrAlgInfo class.
*
*/



#ifndef CR_ALGINFO_H
#define CR_ALGINFO_H

//  INCLUDES
#include <e32base.h>    // CBase.

#include "crdefs.h"     // CrDefines.

//  CONSTANTS
const TUint8 KCrMediumDigestLength = 16;
const TUint8 KCrLongDigestLength = 20;

const TUint8 KDigestInputSize = 64;

const TUint8 KCrDESKeyLength= 8;
const TUint8 KCrDESIVLength = 8;

const TUint8 KCrRCLongKeyLength = 16;
const TUint8 KCrRCMediumKeyLength = 8;
const TUint8 KCrRCShortKeyLength = 5;
const TUint8 KCrRCIVLength = 8;

const TUint8 KCrPaddingLength = 8;

//  MACROS
//  DATA TYPES

// Algorithms.
enum TCrAlgorithm
    {
    // Message digest algorithms.
    ECrMD2,
    ECrMD5,
    ECrRIPEMD,
    ECrSHA,
    ECrSHA1,
    ECrHMAC,          // item is used only with Symbian library
    ECrHMAC_MD5,      // item is used only with NoCry library
    ECrHMAC_RIPEMD,   // item is used only with NoCry library
    ECrHMAC_SHA1,     // item is used only with NoCry library
    ECrLAST_DIGEST,         

    // Symmetric crypto algorithms.
    ECrDES,
    ECrDES2,
    ECrDES3,
    ECrDESX,
    ECrRC2,
    ECrRC4,
    ECrRC5,
    ECrLAST_SYMM_CRYPTO,    
    };                   

// Mode of the symmetric crypto algorithm.
enum TCrSymmMode
    {
    ECrCBC,
    ECrCFB,
    ECrECB,
    ECrOFB,
    };

// CLASS DECLARATION

/**
*  class CCrAlgInfo
*  CCrAlgInfo contains all information related to symmetric
*  crypto and digest algorithms.
*
*  @lib crpkcs12.lib
*  @since Series 60 3.0
*/
NONSHARABLE_CLASS( CCrAlgInfo ): public CBase
    {
    /////////////////////////////////////////////
    // Constructors and destructors.
    private:
        CCrAlgInfo();

        void ConstructL(
            const TCrAlgorithm aType,
            const TInt         aLen,
            const TInt         aIVLen = 0,
            const TDesC8*      aKey = NULL,
            const TDesC8*      aIV = NULL,
            const TBool        aEncrypt = ETrue,
            const TBool        aSingleBlock = ETrue,
            const TCrSymmMode  aMode = ECrCBC);
        
    public:
        ~CCrAlgInfo();

        /**
        * Constructors
        * @param aType Type of the algorithm of this object
        * @param aKey Key of this object. 
        * @param aIV Initialization vector of this object. 
        * @param aEncrypt Encrypt if true, otherwise decrypt. 
        * @param aSingleBlock Single block mode if true. 
        * @param aMode Algorithm mode.
        * @return Pointer to the created object.
        */
        static CCrAlgInfo* NewLC(
            const TCrAlgorithm aType,
            const TDesC8&      aKey,
            const TDesC8&      aIV,
            const TBool        aEncrypt = ETrue,
            const TBool        aSingleBlock = ETrue,
            const TCrSymmMode  aMode = ECrCBC);

        static CCrAlgInfo* NewLC(
            const TCrAlgorithm aType);

        static CCrAlgInfo* NewL(
            const TCrAlgorithm aType,
            const TDesC8&      aKey,
            const TDesC8&      aIV,
            const TBool        aEncrypt = ETrue,
            const TBool        aSingleBlock = ETrue,
            const TCrSymmMode  aMode = ECrCBC);

        static CCrAlgInfo* NewL(
            const TCrAlgorithm aType);

    private: 
        // Sets parity bits.
        TCrStatus SetParityBits(TPtr8 aPtrKey);
    
    public: // Data
        // Type of the algorithm.
        TCrAlgorithm iType;

        // Pointer to initialized algorithm object.
        TAny* iAlgorithmObject;

        // This buffer contains last portion of the
        // data to encrypt/decrypt which is encrypted/
        // decrypted in FinalCryptSymm function. 
        // Relevant only if symmetric crypto algorithm 
        // in question.
        HBufC8* iLastPortion;
    
        // This buffer contains digest if calculating
        // message digest. Relevant only if digest 
        // algorithm in question.
        HBufC8* iDigest;

        // This buffer contains key if en/decryption
        // in process. Relevant only if crypt 
        // algorithm in question.
        HBufC8* iKey;

        // This buffer contains initialization
        // vector if en/decryption in process.
        // Relevant only if crypt algorithm in question.
        HBufC8* iIV;

        // Is this object for encrypting or decrypting?
        // Relevant only if symmetric crypto algorithm
        // in question.
        TBool iEncrypt;

        // True if only one block.
        TBool iSingleBlock;
    
        // Block status, first/middle/last. Usage depends on algorithm used.
        TInt iBlockStatus;
    
        // Mode of a symmetric crypto algorithm. Relevant
        // only if symmetric crypto algorithm in question.
        TCrSymmMode iMode;

        // Padding. Relevant only if Symbian symmetric
        // crypto algorithm in question.
        TAny* iPadding;

    };

#endif // CR_ALGINFO_H

// End of File