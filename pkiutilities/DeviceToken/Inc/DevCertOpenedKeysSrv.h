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
* Description:   The header file of DevCertOpenedKeysSrv
*
*/



#ifndef __DEVCERTOPENEDKEYSSRV_H__
#define __DEVCERTOPENEDKEYSSRV_H__

#include "DevCertKeyDataManager.h"
#include "DevTokenDataTypes.h"

class CRSAPrivateKey;
class CRSASignature;
class CDSAPrivateKey;
class CDSASignature;
class CDevTokenDHParams;
class CDHPublicKey;
class CDevCertKeyStoreServer;

/**
 * DevToken Server side
 * Abstract base class for performing crypto operations on keys.
 * 
 *  @lib
 *  @since S60 v3.2
 */
class CDevCertOpenedKeySrv : public CActive
    {
    public:
    
        /**
         * Factory function for creating CDevCertOpenedKeySrv-derived classes.
         * 
         * @param aKeyData representing the key
         * @param aType Type of opened key
         * @param aMessage IPC message
         * @param aKeyDataMan Key data manager
         * @return new object of CDevCertOpenedKeySrv
         */
        static CDevCertOpenedKeySrv* NewL(const CDevCertKeyData& aKeyData, TUid aType, const RMessage2& aMessage,
                                          CDevCertKeyDataManager& aKeyDataMan );
        
        virtual ~CDevCertOpenedKeySrv();

         /**
         * Returns the object's human-readable label.
         * 
         * @param 
         * @return the object's human-readable label
         */
        const TDesC& Label() const;
        
        /**
         * Returns the key's handle so we can identifiy it.
         * 
         * @param 
         * @return the hadnle of key
         */
        TInt Handle() const;

        /**
         * The type of opened key.
         * 
         * @param 
         * @return the uid of opened key
         */
        virtual TUid Type() const = 0;

    protected:
        
        CDevCertOpenedKeySrv(const CDevCertKeyData& aKeyData, CDevCertKeyDataManager& aKeyDataMan );
        
        void DoOperation(TRequestStatus& aStatus);
        
        // Methods supplied by derived classes
        
    protected:
    
        /**
         * The key algorithm this operation applies to
         * 
         * @param
         * @return
         */
        virtual CDevTokenKeyInfo::EKeyAlgorithm Algorithm() const = 0;

        /**
         * The key usage required to perform this operation
         * 
         * @param
         * @return
         */
        virtual TKeyUsagePKCS15 RequiredUsage() const = 0;
        
        /**
         * Load the private key data
         * 
         * @param aStream used to read data
         * @return
         */
        virtual void ReadPrivateKeyL(RReadStream& aStream) = 0;
        
        /**
         * Perform the operation, called from RunL
         * 
         * @param 
         * @return
         */
        virtual void PerformOperationL() = 0;
        
        /**
         * Clean up, called after normal end error completion
         * 
         * @param 
         * @return
         */
        virtual void Cleanup() = 0;

    private:
        enum TState
            {
            EIdle,
            EOperation
            };
            
        // from CActive
        
        virtual void RunL();
        
        virtual TInt RunError(TInt aError);
        
        virtual void DoCancel();

    private:
        
        void ConstructL(const RMessage2& aMessage);
        
        void CheckKeyL(const CDevTokenKeyInfo& aKeyInfo, const RMessage2& aMessage);
        
        void Complete(TInt aError);

    private:
        
        const CDevCertKeyData&  iKeyData;
        
        CDevCertKeyDataManager& iKeyDataMan;
        
        HBufC*         iLabel;
        
        TState         iState;
        
        TRequestStatus*    iClientStatus;

    protected:
        
        TBool        iKeyRead;
    };


/**
 * DevToken Server side
 * class for performing crypto operations on RSA keys.
 *
 *  @lib
 *  @since S60 v3.2
 */
class CDevCertRSARepudiableSignerSrv : public CDevCertOpenedKeySrv
    {
    public:
        
        CDevCertRSARepudiableSignerSrv(const CDevCertKeyData& aKeyData, CDevCertKeyDataManager& aKeyDataMan );
        
        ~CDevCertRSARepudiableSignerSrv();
        
        void Sign(const TDesC8& aPlaintext, CRSASignature*& aSignature, TRequestStatus& aStatus);

    private:
        
        virtual TUid Type() const;
        
        virtual CDevTokenKeyInfo::EKeyAlgorithm Algorithm() const;
        
        virtual TKeyUsagePKCS15 RequiredUsage() const;
        
        virtual void ReadPrivateKeyL(RReadStream& aStream);
        
        virtual void PerformOperationL();
        
        virtual void Cleanup();

    private:
        
        CRSAPrivateKey* iPrivateKey;
        
        TPtrC8      iPlaintext;
        
        CRSASignature** iSignaturePtr;  
    };


/**
 * DevToken Server side
 * class for performing crypto operations on DSA keys.
 *
 *  @lib
 *  @since S60 v3.2
 */
class CDevCertDSARepudiableSignerSrv : public CDevCertOpenedKeySrv
    {
    public:
        
        CDevCertDSARepudiableSignerSrv(const CDevCertKeyData& aKeyData, CDevCertKeyDataManager& aKeyDataMan );
        
        ~CDevCertDSARepudiableSignerSrv();
        
        void Sign(const TDesC8& aPlaintext, CDSASignature*& aSignature, TRequestStatus& aStatus);

    private: 
        
        virtual TUid Type() const;
        
        virtual CDevTokenKeyInfo::EKeyAlgorithm Algorithm() const;
        
        virtual TKeyUsagePKCS15 RequiredUsage() const;
        
        virtual void ReadPrivateKeyL(RReadStream& aStream);
        
        virtual void PerformOperationL();
        
        virtual void Cleanup();

    private:
        
        CDSAPrivateKey* iPrivateKey;
        
        TPtrC8      iPlaintext;
        
        CDSASignature** iSignaturePtr;  
    };


/**
 * DevToken Server side
 * class for performing decrypt operations on RSA keys.
 *
 *  @lib
 *  @since S60 v3.2
 */
class CDevCertRSADecryptorSrv : public CDevCertOpenedKeySrv
    {
    public:
        
        CDevCertRSADecryptorSrv(const CDevCertKeyData& aKeyData, CDevCertKeyDataManager& aKeyDataMan );
        
        ~CDevCertRSADecryptorSrv();
        
        void Decrypt(const TDesC8& aCiphertext, HBufC8*&, TRequestStatus& aStatus);

    private:
        
        virtual TUid Type() const;
        
        virtual CDevTokenKeyInfo::EKeyAlgorithm Algorithm() const;
        
        virtual TKeyUsagePKCS15 RequiredUsage() const;
        
        virtual void ReadPrivateKeyL(RReadStream& aStream);
        
        virtual void PerformOperationL();
        
        virtual void Cleanup();

    private:
        
        CRSAPrivateKey* iPrivateKey;
        
        TPtrC8      iCiphertext;
        
        HBufC8**    iPlaintextPtr;
    };


/**
 * DevToken Server side
 * class for performing crypto operations on DH keys.
 *
 *  @lib
 *  @since S60 v3.2
 */
class CDevCertDHAgreementSrv : public CDevCertOpenedKeySrv
    {
    public:
    
        CDevCertDHAgreementSrv(const CDevCertKeyData& aKeyData, CDevCertKeyDataManager& aKeyDataMan );
        
        ~CDevCertDHAgreementSrv();
        
        void PublicKey(CDevTokenDHParams& aParameters, RInteger& aPublicKey, TRequestStatus& aStatus);
        
        void Agree(CDHPublicKey& aY, HBufC8*& aAgreedKey, TRequestStatus& aStatus);

    private:
        
        virtual TUid Type() const;
        
        virtual CDevTokenKeyInfo::EKeyAlgorithm Algorithm() const;
        
        virtual TKeyUsagePKCS15 RequiredUsage() const;
        
        virtual void ReadPrivateKeyL(RReadStream& aStream);
        
        virtual void PerformOperationL();
        
        virtual void Cleanup();

    private:
        
        void DoPublicKeyL();
        
        void DoAgreeL();

    enum TDHState
        {
        EIdle,
        EPublicKey,
        EAgree
        };

    private:
        
        RInteger    iKey;
        
        TDHState    iDHState;
        
        // For public key operation
        CDevTokenDHParams*    iPKParams;
        
        RInteger*     iPKPublicKeyPtr;
        
        // For agree key operation
        CDHPublicKey* iAKPublicKey;
        
        HBufC8**    iAKAgreedKeyPtr;
    };

#endif  //__DEVCERTOPENEDKEYSSRV_H__

//EOF

