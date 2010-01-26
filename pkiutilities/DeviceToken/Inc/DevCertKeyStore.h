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
* Description:   The header file of DevCertKeyStore
*
*/



#ifndef __DEVKEYSTORE_H__
#define __DEVKEYSTORE_H__

#include <mctkeystoremanager.h>
#include <ct/rmpointerarray.h>
#include "DTClient.h"


// Maximum allowed size of digest to RSA sign (implied by max 2048 bit key length).
const TInt KMaxRSADigestSize = 2048 / 8;

// Maximum allowed size of digest to DSA sign (implied by 160 bit output of SHA1).
const TInt KMaxDSADigestSize = 160 / 8;

class CDevCertOpenedKey;
class MDevTokenKeyInfoArray;
class CDevTokenDHParams;


/**
 * Implements the MCTKeyStoreManager interface.  Inherits from CDTClient.
 *
 * The class implements the keystore manager token interface for the software
 * keystore.  It will be created and owned by the unified keystore.  Its main
 * function is to marshall user requests to the filetokens server and unmarshall
 * the results back again.
 
 *  @lib DevTokenClient.dll
 *  @since S60 v3.2
 */
class CDevCertKeyStore : public CDTClient, public MCTKeyStoreManager
    {
    public:// One creator function for each token interface
        
        static MCTTokenInterface* NewKeyStoreUserInterfaceL(MCTToken& aToken, RDevTokenClientSession& aClient);
        
        static MCTTokenInterface* NewKeyStoreManagerInterfaceL(MCTToken& aToken, RDevTokenClientSession& aClient);
        
    public:
        
        virtual ~CDevCertKeyStore();
        
        void AddRef();
        
    public:
        
        //  MKeyStore
        virtual void List(RMPointerArray<CCTKeyInfo>& aKeys, const TCTKeyAttributeFilter& aFilter, TRequestStatus& aStatus);
        
        virtual void CancelList();
        
        virtual void GetKeyInfo(TCTTokenObjectHandle aHandle, CCTKeyInfo*& aInfo, TRequestStatus& aStatus);
        
        virtual void CancelGetKeyInfo();
        
        /**
         * Open an RSA key for signing
         *
         * @param aHandle Handle to the key 
         * @param aSigner pointer to a RSA Signer
         * @param aStatus
         * @return 
         */
        virtual void Open(const TCTTokenObjectHandle& aHandle, 
                          MRSASigner*& aSigner,
                          TRequestStatus& aStatus);
        
        /**
         * Open an DSA key for signing
         *
         * @param aHandle Handle to the key 
         * @param aSigner pointer to a DSA Signer
         * @param aStatus
         * @return 
         */
        virtual void Open(const TCTTokenObjectHandle& aHandle, 
                          MDSASigner*& aSigner, 
                          TRequestStatus& aStatus);

        /**
         * Open a RSA key for private decryption
         *
         * @param aHandle Handle to the key 
         * @param aSigner pointer to a RSA Decryptor
         * @param aStatus
         * @return 
         */
        virtual void Open(const TCTTokenObjectHandle& aHandle, 
                          MCTDecryptor*& aDecryptor,
                          TRequestStatus& aStatus);
        
        /**
         * Open a DH key for key agreement
         *
         * @param aHandle Handle to the key 
         * @param aSigner pointer to a DH 
         * @param aStatus
         * @return 
         */
        virtual void Open(const TCTTokenObjectHandle& aHandle, 
                          MCTDH*& aDH, TRequestStatus& aStatus);
        
        /**
         * Cancels an ongoing open request
         *
         * @param 
         * @return 
         */
        virtual void CancelOpen();
        
        /**
         * Returns the public key in DER-encoded ASN-1
         *
         * @param aHandle handle to the public key
         * @param aPublicKey buffer for the public key
         * @param aStatus
         * @return 
         */
        virtual void ExportPublic(const TCTTokenObjectHandle& aHandle,
                                  HBufC8*& aPublicKey,
                                  TRequestStatus& aStatus);
        
        virtual void CancelExportPublic();

    public:
        //  MCTKeyStoreManager
        virtual void CreateKey(CCTKeyInfo*& aReturnedKey, TRequestStatus& aStatus);
        
        virtual void CancelCreateKey();
        
        virtual void ImportKey(const TDesC8& aKey, CCTKeyInfo*& aReturnedKey, TRequestStatus& aStatus);
        
        virtual void CancelImportKey();
        
        virtual void ImportEncryptedKey(const TDesC8& aKey, CCTKeyInfo*& aReturnedKey, TRequestStatus& aStatus);
        
        virtual void CancelImportEncryptedKey();
        
        virtual void ExportKey(TCTTokenObjectHandle aHandle, HBufC8*& aKey, TRequestStatus& aStatus); 
        
        virtual void CancelExportKey();
        
        virtual void ExportEncryptedKey(TCTTokenObjectHandle aHandle, const CPBEncryptParms& aParams, HBufC8*& aKey, TRequestStatus& aStatus);
        
        virtual void CancelExportEncryptedKey();
        
        virtual void DeleteKey(TCTTokenObjectHandle aHandle, TRequestStatus& aStatus);
        
        virtual void CancelDeleteKey(); 
        
        virtual void SetUsePolicy(TCTTokenObjectHandle aHandle, const TSecurityPolicy& aPolicy, TRequestStatus& aStatus);
        
        virtual void CancelSetUsePolicy();
        
        virtual void SetManagementPolicy(TCTTokenObjectHandle aHandle, const TSecurityPolicy& aPolicy, TRequestStatus& aStatus);
        
        virtual void CancelSetManagementPolicy();
        
        virtual void SetPassphraseTimeout(TInt aTimeout, TRequestStatus& aStatus) ;
        
        virtual void CancelSetPassphraseTimeout();  
        
        virtual void Relock(TRequestStatus& aStatus);
        
        virtual void CancelRelock();  
        
        void ReleaseObject(const TCTTokenObjectHandle& aObject);
        
    public: 
        
        //  From MCTTokenInterface
        virtual MCTToken& Token();
        
    public:
        
        void RepudiableDSASign(const TCTTokenObjectHandle& aObject, const TDesC8& aDigest, CDSASignature*& aSignature, TRequestStatus& aStatus);
        
        void RepudiableRSASign(const TCTTokenObjectHandle& aObject, const TDesC8& aDigest, CRSASignature*& aSignature, TRequestStatus& aStatus);
        
        void Decrypt(const TCTTokenObjectHandle& aObject, const TDesC8& aCiphertext, TDes8& aPlaintext, TRequestStatus& aStatus);
        
        void DHPublicKey(const TCTTokenObjectHandle& aHandle, const TInteger& aN, const TInteger& aG, CDHPublicKey*& aX, TRequestStatus& aStatus);
        
        void DHAgree(const TCTTokenObjectHandle& aHandle, const CDHPublicKey& iY, HBufC8*& aAgreedKey, TRequestStatus& aStatus);

    protected:
        
        //  From MCTTokenInterface
        virtual void DoRelease();
        
    protected:
        
        //  From CActive
        virtual void RunL();
        
        virtual TInt RunError(TInt aError);
        
        virtual void DoCancel();
        
    protected:
        
        CDevCertKeyStore(TInt aUID, MCTToken& aToken, RDevTokenClientSession& aClient);
        
        virtual void ConstructL();
        
    private:
        
        void DoListL(const TCTKeyAttributeFilter& aFilter, MDevTokenKeyInfoArray& aOut);
        
        void DoGetKeyInfoL(TCTTokenObjectHandle aHandle, CCTKeyInfo*& aInfo);
        
        TInt DoOpenKey(TDevTokenMessages aMessage, const TCTTokenObjectHandle& aHandle, CDevCertOpenedKey* aOpenedKey);
        
        void DoImportKey(TDevTokenMessages aMessage, const TDesC8& aKey, CCTKeyInfo*& aReturnedKey, TRequestStatus& aStatus);
        
        void DoExportPublicL(const TCTTokenObjectHandle& aHandle, HBufC8*& aPublicKey);
        
        TInt MarshalKeyInfo(CCTKeyInfo& aKey);
        
        void Process(const TCTTokenObjectHandle& aHandle,const TDesC8& aText,TDevTokenMessages aMessage,TRequestStatus& aStatus,TInt aBufSize);
        
        void DoDHPublicKeyL(const TCTTokenObjectHandle& aHandle, const TInteger& aN, const TInteger& aG, CDHPublicKey*& aX, TRequestStatus& aStatus);

    private:
        
        TInt AllocKeyBuffer(TInt aReqdSize);
        
        void FreeKeyBuffer();
        
        void UpdateKey();
        
        TUint GetKeySizeL(TInt aObjectId);
        
        void Cleanup();
    
    private:
        
        TInt iRefCount;
        
        //  Data holders for server to write into
        TPckgBuf<TInt> iIntOut;

        TInt iObjectId;     // Id of object passed to server

    private:
        
        // Pointers to client pointers, which we set to return created objects.
        // Only one of these is ever in use at any one time, determined by the
        // state.
        union 
            {
            TAny* iAny;           // generic pointer, used to zero the contents of the union
            CCTKeyInfo** iKeyInfo;      // key info object, used for create and import
            HBufC8** iBuffer;       // generic buffer, used for export key and DH agree
            TDes8* iDecryptedText;      // decypled plaintext, used for RSA decrypt
            CDSASignature** iDSASignature;  // DSA signature
            CRSASignature** iRSASignature;  // RSA signature
            CDHPublicKey** iDHPublicKey;  // DH public key
            } iClientPtr;
            
    private:
        
        HBufC8* iKey;           // Key data buffer for import/export
        
        HBufC8* iText;            // Buffer used to hold text for decrypt and sign
        
        CDevTokenDHParams* iDHParams;       // DH params for DH public key
        
    };

#endif  //  __DEVKEYSTORE_H__

//EOF
