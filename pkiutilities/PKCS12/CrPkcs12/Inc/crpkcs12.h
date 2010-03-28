/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   This file contains the header of a CCrPKCS12 Class.
*
*/



#ifndef CRPKCS12
#define CRPKCS12


//  INCLUDES
#include "CX509CertificateSet.h"        // X509CertificateSet
#include "crcrypto.h"                   // Crypto library
#include <crber.h>		                // CCrBer
#include <e32base.h>
#include <secdlg.h>                    


//  EXTERNAL DATA STRUCTURES
//  FUNCTION PROTOTYPES
//  FORWARD DECLARATIONS
//  CLASS DEFINITIONS

// DATA TYPES

// Object Identifiers
// They are used to identify different objects
enum TCrPkcs12Algorithm
    {
    // Pkcs12 algorithms.
    ECrpbeWithSHAAnd128BitRC4,
    ECrpbeWithSHAAnd40BitRC4,
    ECrpbeWithSHAAnd3_KeyTripleDES_CBC,
    ECrpbeWithSHAAnd2_KeyTripleDES_CBC,
    ECrKpbeWithSHAAnd128BitRC2_CBC,
    ECrKpbeWithSHAAnd40BitRC2_CBC
    };

// CONSTANTS
const TInt K40BitRC2KeySize = 5;
const TInt K128BitRC2KeySize = 16;
const TInt KDesKeySize = 8;

// Digest algorithm identifiers
_LIT(KCrSha1,"1.3.14.3.2.26"); // Digest algorithm Sha1.
_LIT(KCrMD5, "1.2.840.113549.2.5"); // Digest algorithm Sha1.

// Used pkcs12 encryption algorithms
_LIT(KpbeWithSHAAnd128BitRC4,"1.2.840.113549.1.12.1.1"); // These are algorith
_LIT(KpbeWithSHAAnd40BitRC4,"1.2.840.113549.1.12.1.2"); // identifiers for deriving
_LIT(KpbeWithSHAAnd3_KeyTripleDES_CBC,"1.2.840.113549.1.12.1.3"); // keys and IVs.
_LIT(KpbeWithSHAAnd2_KeyTripleDES_CBC,"1.2.840.113549.1.12.1.4"); // As implied by their
_LIT(KpbeWithSHAAnd128BitRC2_CBC,"1.2.840.113549.1.12.1.5"); // names, all of the object
_LIT(KpbeWithSHAAnd40BitRC2_CBC,"1.2.840.113549.1.12.1.6"); // identifiers here use the hash
                                                         // function SHA-1.

// Pkcs12 SafeBags-types
_LIT(KkeyBag,"1.2.840.113549.1.12.10.1.1"); // SafeBag types. Each SafeBag holds one piece
_LIT(Kpkcs8ShroudedKeyBag,"1.2.840.113549.1.12.10.1.2"); // of information -a key,
_LIT(KcertBag,"1.2.840.113549.1.12.10.1.3"); // a certificate, etc.- which is identified
_LIT(KcrlBag,"1.2.840.113549.1.12.10.1.4"); // by an object identifier.
_LIT(KsecretBag,"1.2.840.113549.1.12.10.1.5");
_LIT(KsafeContentsBag,"1.2.840.113549.1.12.10.1.6");

// Pkcs7 datatypes used in this library
_LIT(Kpkcs7Data,"1.2.840.113549.1.7.1");  // Pkcs7 Data-type
_LIT(Kpkcs7EncryptedData,"1.2.840.113549.1.7.6"); // Pkcs7 encryptedData-type

// Pkcs9 object identifiers
_LIT(Kpkcs9LocalKeyId,"1.2.840.113549.1.9.21");     // LocalKeyId. This is used to identify
                                                    // what certificate belongs to what
                                                    // PrivateKey

_LIT(Kx509certificate,"1.2.840.113549.1.9.22.1");   // X509Certificate

// pkcs1 object identifier for rsaEncryption
_LIT(Kpkcs1rsaEncryption,"1.2.840.113549.1.1.1");   // For PrivateKey

_LIT(KdsaSignatureKey,"1.2.840.10040.4.1");   // DSA signature key


const TInt KVersion3 = 0x3; // Version number for PFX
const TInt KVersion1 = 0x0; // PrivateKey version
const TInt KFirstObject = 0; // First object
const TInt KEdVer0 = 0x0;   // PKCS #7 Encrypted-data content type
                             // version number

const TInt KId1 = 1;        // If ID = 1, then pseudo-random bits being produced are 
                            // used as key material for performing encryption / decryption

const TInt KId2 = 2;        // If ID = 2, then pseudo-random bits produced are to be used as
                            // an IV (Initial Value) for encryption / decryption

const TInt KId3 = 3;        // If ID = 3, then pseudo-random bits being produced are to be
                            // used as an integrity key for MAC-ing.

                     
/**
*  Class CCrPKCS12
*  Class CCrPKCS12 opens pkcs12-file with help of CCrBer-library.
*
*  @lib crpkcs12.lib
*  @since Series 60 3.0
*/
NONSHARABLE_CLASS( CCrPKCS12): public CBase
    {
    // Constructors and destructors.
    private:
        
		CCrPKCS12();
        void ConstructL();
		
    public:
		static CCrPKCS12* NewLC();
		static CCrPKCS12* NewL();
		~CCrPKCS12();

    public: // Functions

        // This is the main function of this library. It validates PKCS #12 file,
        // checks integrity of file, checks password, opens and decrypts ContentInfo,
        // decrypts certificates and private keys. If this function returns
        // ValidFile, other exported functions can be called.
        TCrStatus OpenL(
            CCrData&       aPkcs12File,
            const TDesC16& aPassword );

        // Returns set of CX509Certificate objects defined in certman.
        // These are CACertificates.
        const CX509CertificateSet& CACertificates() const;

        // Returns set of TDesC8 objects defined in certman.
        // These are CACertificates.
        const CArrayPtrFlat<TDesC8>& CACertificateBuffer() const;

        // Returns set of CX509Certificate objects defined in certman.
        // These are UserCertificates.
        const CX509CertificateSet& UserCertificates() const;

        // Returns set of TDesC8 objects defined in certman.
        // These are UserCertificates.
        const CArrayPtrFlat<TDesC8>& UserCertificateBuffer() const;
        
        const CArrayPtrFlat<HBufC8>& PrivateKeys() const;
		
        // Returns the number of iterations.
		TUint Iter();

        // Returns mac.
		HBufC8* Mac();

        // Returns salt.
		HBufC8* Salt();

        // Returns number of SafeBags in PKCS #12 file.
        TUint SafeBagsCount();

    private:

        // Private helper class for asynchronous calls.
        NONSHARABLE_CLASS( CPKCS12SyncWrapper ): public CActive
            {
            public:
                CPKCS12SyncWrapper();
                ~CPKCS12SyncWrapper();
                TInt EnterPasswordL(const TPINParams& aPINParams, TBool aRetry, TPINValue& aPINValue);
            protected:  // Functions from base classes        
                /**
                * From CActive
                */
                void DoCancel();
	            void RunL();
            private:
                TInt SetActiveAndWait();
            private:
                MSecurityDialog* iSecDlg;
                CActiveSchedulerWait iWait;
            };
        
    private:
        // Jumps next object at same level, used in Open-function.
        TUint JumpNextObjectAtSameLevel();
        
        // Returns what's the algorithm (via OID).
        // Returns 0, if it isn't algorithm at all.
        TUint GetAlgorithmL( HBufC* aBuf );

        // This function adds given certificate to given set.
        void PutCertsIntoSetL(CX509CertificateSet* aSet,
                              CArrayPtrFlat<TDesC8>* aBufSet,
                              HBufC8* aX509certificate );

        // This function generates double byte pasword from given eight 
        // byte password and calls VerifyMac.
        TBool VerifyMacFromEightBytePassword(const TDesC8& aPassWord);

        // Verifies mac (integrity of file and if password is correct).
        // Used in Open-function.
        TBool VerifyMacL(const TDesC8& aPassWord);
        TBool VerifyMacL(const TDesC16& aPassWord);

        // Opens ContentInfo, used in Open-function.
        TBool ReadContentInfo( CCrData& aContentInfo );

        // Decodes SafeBags. Used in Open function.
        TInt DecodeSafeBagsL();

        // Decrypts ContentInfo, used in OpenContentInfo-function.
        TBool UnpackContentInfo( CCrData& aContentData );

        // Decrypts pkcs7Data, used in DecryptContentInfo-function.
        TBool UnpackPkcs7DataL( CCrData& aPkcs7Data );

        // Unpacks pkcs7EncryptedData, used in DecryptContentInfo-
        // function.
        TBool UnpackPkcs7EncryptedDataL( CCrData& aPkcs7EncryptedData );

        // Decrypts pkcs7EncryptesData, used in UnpackPkcs7EncryptedData 
        // function.
        TBool DecryptPkcs7EncryptedDataL(HBufC8* aPkcs7EncryptedData,
                                         HBufC8* aSalt,
                                         TInt aIter,
                                         TInt aAlgorithm );


        // Extracts contents of keybag, used in DecodeSafeBags function.
		void ExtractKeybagL( CCrData& aSafeBag );

        // Decrypts pkcs8-shroudedkeybag, used in DecodeSafeBags function.
        void DecryptShroudedKeybagL( CCrData& aSafeBag );

        // Decodes certificate bag, used in DecodeSafeBags function.
        void DecodeCertBagL( CCrData& aSafeBags );

        // Decrypts private key, used in DecryptShroudedKeybag.
        TBool DecryptPrivateKeyL( HBufC8* aEncryptedPrivateKey,
                                 HBufC8* aSalt,
                                 TInt aIter,
                                 TInt aAlgorithm );
        
        // Gets SafeBags LocalKeyId. This is used to identidy what certificate
        // belongs to private key. Used in DecryptPrivateKey function.
        HBufC8* GetLocalKeyId( CCrData& aBagData );
        
        void DecryptDataL( const TDesC8& aEncryptedData,
                           const TDesC8& aSalt,
                           TInt aIter,
                           TInt aAlgorithm,
                           TDes8& aDecryptedData );
                                            

	private: // Data.

        // Iterations
		TUint iIter;

        // CCrBerSet, set of CCrBer-objects, from crber.lib.
		CCrBerSet* iberSet;

        // CCrBer-object, from crber.lib.
		CCrBer* iberObject;

        // UserCertificates, contains set of CX509Certificate Objects,
        // defined in certman. These certificates are user certificates.
        // Usually there are just one user certificate in PKCS #12 file.
        CX509CertificateSet* iUserCertificates;
        

        // CACertificates, contains set of CX509Certificate Objects,
        // defined in certman. These certificates are CA certificates.
        CX509CertificateSet* iCACertificates;

        CArrayPtrFlat<TDesC8>* iUserCertificateBuffer;
        CArrayPtrFlat<TDesC8>* iCACertificateBuffer;
                
        // Decrypted PKCS#8 encoded private keys.        
        CArrayPtrFlat<HBufC8>* iPKCS8PrivateKeyArray;
        
        // This is private keys' LocalKeyIds. With these we can identify
        // a corresponding certificate.        
        RPointerArray<HBufC8> iPrivateKeyIdArray;
        
        // For storing mac.
		HBufC8* iMac;

        // For storing salt.
		HBufC8* iSalt;
        
        // index number of berObject.
        TInt iObjectNum;

        // Password given to Open function. Must be available to other
        // functions also, so it is declared here.
        HBufC8* iPassWord;
                        
        // Number of SafeBags.
        TInt iSafeBagsCount;

        // Used HMAC-algorithm.
        TInt iHMACalgorithm;

        // PKCS #7 data.
        HBufC8* iContentInfo;

        // SafeBags.
        HBufC8* iBags;
                                		        
        HBufC8* iDecryptionKey;

        TFileName iFileName;
	};

#endif
