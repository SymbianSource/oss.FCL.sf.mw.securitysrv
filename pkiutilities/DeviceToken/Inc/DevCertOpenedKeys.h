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
* Description:   The header file of DevCertOpenedKeys
*
*/



#ifndef __DEVCERTOPENEDKEYS_H__
#define __DEVCERTOPENEDKEYS_H__

class CDevCertKeyStore;
class CMessageDigest;


/**
 * DevToken Client side
 * Abstract base class for performing crypto operations on keys.
 * 
 *  @lib DevTokenClient.dll
 *  @since S60 v3.2
 */
class CDevCertOpenedKey : protected CBase
    {
    public:
        void SetObjectID(TInt aObjectId) {iHandle.iObjectId = aObjectId;}
        void SetLabel(HBufC* aLabel) {iLabel = aLabel;}
        // Virtual Release method visible though CDevCertOpenedKey pointers - the
        // implementation just calls the Release method from the mixin class in each
        // case.
        virtual void Release() = 0;

    protected:
        virtual ~CDevCertOpenedKey();

    protected:
        TCTTokenObjectHandle iHandle;
        CDevCertKeyStore* iClient;
        HBufC* iLabel;
    };


/**
 * DevToken Client side
 * class for performing crypto operations on RSA keys.
 * 
 *  @lib DevTokenClient.dll
 *  @since S60 v3.2
 */
class CDevCertRSARepudiableSigner : public CDevCertOpenedKey, public MRSASigner
    {
    public:
        static CDevCertRSARepudiableSigner* New(CDevCertKeyStore* aClient);
    
        // MCTTokenObject 
    public:
        /**
         * The object's human-readable label 
         *
         * @param 
         * @return The object's human-readable label
         */
        virtual const TDesC& Label() const;
        
        /**
         * A reference to the associated token 
         *
         * @param 
         * @return A reference to the associated token
         */
        virtual MCTToken& Token() const;
        
        /**
         * A UID representing the type of the token object. The
         * meanings of possible UIDs should be documented in the
         * documentation for the interface that returns them.  
         *
         * @param 
         * @return A UID representing the type of the token object
         */
        virtual TUid Type() const;
        
        /**
         * A handle for the object. The primary purpose of the
         * handle is to allow token objects to be 'passed' between
         * processes. See TCTTokenObjectHandle for more details.
         *
         * @param 
         * @return A handle for the object
         */
        virtual TCTTokenObjectHandle Handle() const;
        
        // MSigner 
    public:
        /**
         * Sign some data. 
         * The data is hashed before the signature is created using the SHA-1
         * algorithm.
         *
         * @param aPlaintext The string to be signed.
         * @param aSignature The returned signature. This remains owned
                             by the signer, and is valid until the signer 
                             is released or until the next Sign.
           @param aStatus Async status notification
         * @return 
         */
        virtual void SignMessage(const TDesC8& aPlaintext, CRSASignature*& aSignature, 
        TRequestStatus& aStatus);

        /** 
         * Perform a raw signing operation.
         *
         * @param aDigest The data to be signed - this should be some form of hash of
         *                the actual message to be signed.  If the data is too long,
                          this method will return KErrOverflow through aStatus.  
         * @param aSignature The returned signature. This remains owned by the signer,
                             and is valid until the signer is released or until the next 
                             Sign.
         * @param aStatus Async status notification
         * @return
         */
        virtual void Sign(const TDesC8& aData, CRSASignature*& aSignature, 
        TRequestStatus& aStatus);

        /** 
         * Cancel an ongoing sign.
         *
         * @param 
         * @return
         */
        virtual void CancelSign();

        virtual void Release();

    private:
        CDevCertRSARepudiableSigner(CDevCertKeyStore* aClient);
        virtual ~CDevCertRSARepudiableSigner();

    private:
        CMessageDigest* iDigest;
        HBufC8* iSignature; 
    };


/**
 * DevToken Client side
 * class for performing crypto operations on DSA keys.
 * 
 *  @lib DevTokenClient.dll
 *  @since S60 v3.2
 */
class CDevCertDSARepudiableSigner : public CDevCertOpenedKey, public MDSASigner
    {
    public:
        
        static CDevCertDSARepudiableSigner* New(CDevCertKeyStore* aClient);
        
        virtual const TDesC& Label() const;
        
        virtual MCTToken& Token() const;
        
        virtual TUid Type() const;
        
        virtual TCTTokenObjectHandle Handle() const;
        
        virtual void SignMessage(const TDesC8& aPlaintext, CDSASignature*& aSignature, 
                                 TRequestStatus& aStatus);
        
        virtual void Sign(const TDesC8& aData, CDSASignature*& aSignature, 
                         TRequestStatus& aStatus);
        
        virtual void CancelSign();
        
        virtual void Release();

    private:
        
        CDevCertDSARepudiableSigner(CDevCertKeyStore* aClient);
        
        virtual ~CDevCertDSARepudiableSigner();

    private:
        
        CMessageDigest* iDigest;
    };


/**
 * DevToken Client side
 * class for performing decrypt operations on RSA keys.
 * 
 *  @lib DevTokenClient.dll
 *  @since S60 v3.2
 */
class CDevCertRSADecryptor : public CDevCertOpenedKey, public MCTDecryptor
    {
    public:
        
        static CDevCertRSADecryptor* New(CDevCertKeyStore* aClient);
        
        virtual const TDesC& Label() const;
        
        virtual MCTToken& Token() const;
        
        virtual TUid Type() const;
        
        virtual TCTTokenObjectHandle Handle() const;
        
        /** 
         * Perform a decrypt operation.
         *
         * @param aCiphertext The data to be decrypted 
         * @param aPlaintext The returned decrypted text
         * @param aStatus Async status notification
         * @return
         */
        virtual void Decrypt(const TDesC8& aCiphertext, TDes8& aPlaintext,
                             TRequestStatus& aStatus);
        
        virtual void CancelDecrypt();
        
        virtual void Release();

    private:
    
        CDevCertRSADecryptor(CDevCertKeyStore* aClient);
        
        virtual ~CDevCertRSADecryptor();
    };


/**
 * DevToken Client side
 * class for performing crypto operations on DH keys.
 * 
 *  @lib DevTokenClient.dll
 *  @since S60 v3.2
 */
class CDevCertDHAgreement : public CDevCertOpenedKey, public MCTDH
    {
    public:
        
        static CDevCertDHAgreement* New(CDevCertKeyStore* aClient);
        
        virtual const TDesC& Label() const;
        
        virtual MCTToken& Token() const;
        
        virtual TUid Type() const;
        
        virtual TCTTokenObjectHandle Handle() const;
        
        /** 
         * Get DH public key.
         *
         * @param aN Integer N 
         * @param aG Integer G
         * @param aX DH public key
         * @param aStatus Async status notification
         * @return
         */
        
        virtual void PublicKey(const TInteger& aN, const TInteger& aG, 
                               CDHPublicKey*& aX,
                               TRequestStatus& aStatus);
        /** 
         * Agrees a session key given the public key of the other party.
         *
         * @param aY DH PublicKey 
         * @param aAgreedKey Agreed key
         * @param aStatus Async status notification
         * @return
         */
         virtual void Agree(const CDHPublicKey& iY,
                            HBufC8*& aAgreedKey,
                            TRequestStatus& aStatus);
                            
         virtual void CancelAgreement();
    
         virtual void Release();

    private:
    
         CDevCertDHAgreement(CDevCertKeyStore* aClient);
    
         virtual ~CDevCertDHAgreement();
    };
#endif
