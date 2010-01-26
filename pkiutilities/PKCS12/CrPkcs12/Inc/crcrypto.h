/*
* Copyright (c) 2000, 2002, 2004 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   This file contains the header of CCrCrypto class.
*
*/


#ifndef CR_CRYPTO_H
#define CR_CRYPTO_H

//  INCLUDES
#include <e32base.h>    // CBase.

#include "cralginfo.h"  // CCrAlgInfo
#include <crdata.h>

//  CONSTANTS
const TInt NO_PROCESS = -1;
const TInt KLenFileBuffer = 1000;

//  DATA TYPES
// Padding rules. 
enum TPaddingRule
    {
    ECrNone,
    ECrPKCS1,
    ECrSSLv3
    };

// Algorithms.
enum TCrAsymAlgorithm
    {
    ECrRSA,
    ECrDSA,
    ECrLAST_ASYMM_CRYPTO
    };                   


// CLASS DECLARATION

/**
*  class CCrCrypto
*  CCrCrypto crypto API, which is used in CrPKCS12.
*
*  @lib crpkcs12.lib
*  @since Series 60 3.0
*/
NONSHARABLE_CLASS( CCrCrypto ): public CBase
    {
    // Constructors and destructors.
    private:
        CCrCrypto();

        void ConstructL();
        
    public:
        ~CCrCrypto();
        static CCrCrypto* NewLC(); 
        static CCrCrypto* NewL();
    
    public: // Functions.

        /**
        * Initialize encryption or decryption with Triple DES algorithm
        * using three different keys.
        * @param aKey1 1. key.
        * @param aKey2 2. key.
        * @param aKey3 3. key.
        * @param aIV Initialization vector.
        * @param aEncrypt Encrypt if true.
        * @param aMode Algorithm mode.
        * @param aPadRule Padding rule.
        * @return KCrCrypto | KCrUnknownMode
        *         KCrCrypto | KCrUnknownLibrary
        *         KCrOK
        */
        TCrStatus InitCrypt3DESL(
            const TDesC8& aKey1,                
            const TDesC8& aKey2,                
            const TDesC8& aKey3,                
            const TDesC8& aIV,                  
            TBool         aEncrypt = ETrue,     
            TCrSymmMode   aMode = ECrCBC,       
            TPaddingRule  aPadRule = ECrPKCS1); 

        /**
        * Initialize encryption or decryption with RC2 algorithm.
        * @param aKey Key.
        * @param aIV Initialization vector.
        * @param aEncrypt Encrypt if true.
        * @param aEffectiveKeyLen Effective key length in bits
        * @param aMode Algorithm mode.
        * @param aPadRule Padding rule.
        * @return KCrCrypto | KCrUnknownMode
        *         KCrOK
        */
        TCrStatus InitCryptRC2L(
            const TDesC8& aKey,                 
            const TDesC8& aIV,                  
            TBool         aEncrypt = ETrue,     
                                                
            TInt          aEffectiveKeyLen = 0, 
                                                
            TCrSymmMode   aMode = ECrCBC,       
            TPaddingRule  aPadRule = ECrPKCS1); 

        /**
        * Initialize HMAC message digest algorithm.
        * @param aKey Key.
        * @param aDigestAlg Message digest algorithm that HMAC uses.
        * @return KCrCrypto | KCrUnknownMode
        *         KCrOK
        */
        TCrStatus InitDigestHMACL(
            const TDesC8& aKey,             
            TCrAlgorithm aDigestAlg);       
                                            
              
        /**
        * Initialize message digest with MD2 algorithm.
        * @param aAlgorithm      Digest algorithm
        * @return KCrOK or 
        *         KCrCrypto | KCrNotSupportedAlg
        */
        TCrStatus InitDigestL(TCrAlgorithm aAlgorithm);

        

        /**
        * Process given source data with initialized crypto operations.
        * If symmetric crypto is initialized appends to aTrg encrypted
        * or decrypted data without last portion. If only digest algorithm
        * is initialized, aTrg is not used.
        * @param aSrc Source buffer.
        * @param aTrg  Target buffer.
        * @return KCrOK
        *         KCrNotSupportedAlg
        *         KCrUndefinedLibrary
        *         KCrUnknownLibrary
        *         KCrUnknownMode
        */
        TCrStatus ProcessL(const TDesC8& aSrc, TDes8& aTrg);

        
        // Finalize symmetric algorithms. Function appends to aTrg
        // encrypted or decrypted last portion.
        TCrStatus FinalCryptL(TDes8& aTrg);
        
        // Finalize message digest algorithms. 
        // aTrg contains message digest of the data.
        TCrStatus FinalDigest(TDes8& aTrg);        

        // Derive key(s) or IV vector from password, salt and iteration count.
        TCrStatus DeriveKeyPKCS12L(
            const TDesC8& aPassword, 
            const TDesC8& aSalt,
            const TInt    aIterationCount,
            TCrAlgorithm  aHashFunc,
            const TUint8  aID,
            const TInt    aNumberOfBytes,
            TDes8&        aTrg);

    private: // Functions
        // Removes the last portion of the data when encrypting or decrypting.     
        TInt RemoveLastBlock(
            TDesC8&           aSrc,
            const TInt        aBlockSize,
            CCrAlgInfo*       algInfo);

        TInt MesDigestInputSize(TCrAlgorithm aDigestAlg);
        TInt MesDigestOutputSize(TCrAlgorithm aDigestAlg);
        void Reset();
    
    private: // Data.
        // Infos about initialized algorithms are collected in this array.
        CArrayPtrFlat<CCrAlgInfo>* iAlgorithmInfos;
    };

#endif // CR_CRYPTO_H

// End of File