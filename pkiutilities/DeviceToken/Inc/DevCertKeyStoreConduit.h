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
* Description:   The header file of DevCertKeyStoreConduit
*
*/



#ifndef __CDEVCERTKEYSTORECONDUIT_H__
#define __CDEVCERTKEYSTORECONDUIT_H__

#include <bigint.h>
#include "DevTokenServer.h"

class CDSASignature;
class CRSASignature;
class CDevCertKeyStoreServer;
class CDevCertKeyStoreSession;
class CDevCertRSARepudiableSigner;
class CDevCertDSARepudiableSigner;
class CDevCertRSADecryptor;
class CDevTokenDHParams;
class CDevCertOpenedKeySrv;
class CDevTokenKeyInfo;
class CDHPublicKey;

/**
 * The key store server object (CDevtokenKeyStoreServer) owns one instance of this
 * object, which it calls for every client request.  This object unmarshalls the
 * client data and fulfills the request by calling back to the server object.
 * Any return information is then marshalled back to the client.
 *
 *  @lib
 *  @since S60 v3.2
 */
class CDevCertKeyStoreConduit : public CActive
    {
    public:
        
        static CDevCertKeyStoreConduit* NewL(CDevCertKeyStoreServer& aServer);
        
        virtual ~CDevCertKeyStoreConduit();
    
    public:
        
        void ServiceRequestL(const RMessage2& aMessage, CDevCertKeyStoreSession& aSession);
    
    protected: 
        
        //  From CActive
        virtual void DoCancel();
        
        virtual void RunL();
        
        virtual TInt RunError(TInt aError);
    
    private:
        
        CDevCertKeyStoreConduit(CDevCertKeyStoreServer& aServer); 
        
        void ConstructL();
    
    private:
        
        CDevCertKeyStoreConduit(const CDevCertKeyStoreConduit&);        //  No copying
        
        CDevCertKeyStoreConduit& operator=(const CDevCertKeyStoreConduit&); //  No copying
    
    private:
        
        // For MKeyStore
        void ListL(const RMessage2& aMessage);
        
        void CancelList(const RMessage2& aMessage);
        
        void GetKeyInfoL(const RMessage2& aMessage);
        
        void CancelGetKeyInfo(const RMessage2& aMessage);
    
    private:
        
        // For MCTKeyStoreManager
        void CreateKeyL(const RMessage2& aMessage);
        
        void CancelCreateKey(const RMessage2& aMessage);
        
        void ImportKeyL(const RMessage2& aMessage);
        
        void CancelImportKey(const RMessage2& aMessage);
        
        void ExportKeyL(const RMessage2& aMessage); 
        
        void CancelExportKey(const RMessage2& aMessage);
        
        void ExportPublicL(const RMessage2& aMessage); 
        
        void CancelExportPublic(const RMessage2& aMessage);
        
        void DeleteKeyL(const RMessage2& aMessage);
        
        void SetUsePolicyL(const RMessage2& aMessage);
        
        void SetManagementPolicyL(const RMessage2& aMessage); 
        
        void OpenKeyL(const RMessage2& aMessage, CDevCertKeyStoreSession& aSession, TUid aType);
        
        void CloseObjectL(const RMessage2& aMessage, CDevCertKeyStoreSession& aSession);
    
    private:
        
        void GetKeyLengthL(const RMessage2& aMessage); 
        
        TUid GetClientUidL(const RMessage2& aMessage) const;
        
        void OpenKeyGetInfoL(const RMessage2& aMessage);
        
        void RepudiableDSASignL(const RMessage2& aMessage, CDevCertKeyStoreSession& aSession);
        
        void CancelDSASign(const RMessage2& aMessage);
        
        void RepudiableRSASignL(const RMessage2& aMessage, CDevCertKeyStoreSession& aSession);
        
        void CancelRSASign(const RMessage2& aMessage);
        
        void DecryptL(const RMessage2& aMessage, CDevCertKeyStoreSession& aSession);
        
        void CancelDecrypt(const RMessage2& aMessage);
        
        void DHPublicKeyL(const RMessage2& aMessage, CDevCertKeyStoreSession& aSession);
        
        void DoDHPublicKeyL(const RMessage2& aMessage, CDevCertKeyStoreSession& aSession);
        
        void FinishDHPublicKeyL();
        
        void DHAgreeL(const RMessage2& aMessage, CDevCertKeyStoreSession& aSession);
        
        void DoDHAgreeL(const RMessage2& aMessage, CDevCertKeyStoreSession& aSession);
        
        void FinishDHAgreeL();
        
        void CancelDH(const RMessage2& aMessage);
        
        CDevCertOpenedKeySrv* ProcessL(const RMessage2& aMessage, CDevCertKeyStoreSession& aSession,
        
        const TUid& aCKeyInfoID, const TDevTokenMessages& aState, TPtr8& aPtr);
        
        void FinishOpenL(const RMessage2& aMessage, CDevCertKeyStoreSession& aSession, CDevCertOpenedKeySrv& aOpenedKey);
        
        void Cleanup();
    
    private:
    
        /** 
         * Encapsulate data about the current request.
         */
        class TAsyncRequest
            {
            public:
                
                void Set(TDevTokenMessages aRequest, const RMessage2& aMessage);
                
                void Complete(TInt aResult);
                
                void Cancel();
            
            public:
                
                inline TDevTokenMessages OutstandingRequest() { return iRequest; }
                
                inline const RMessage2& Message() { return iMessage ;}
        
            public:
                
                TAsyncRequest(TRequestStatus& aStatus);
            
                ~TAsyncRequest();
            
            private:
                
                TDevTokenMessages iRequest;
                
                RMessage2 iMessage;
                
                TRequestStatus& iOwnerStatus;
            };
             
    private:
    
        CDevCertKeyStoreServer& iServer;    // The server object used to execute client requests

        TAsyncRequest iCurrentRequest;      // The client request we are currently servicing

        HBufC8* iImportKey;           // Buffer for key imported key data 

        CDevTokenKeyInfo* iKeyInfo;       // Key info, used by create, import and export

        HBufC8* iText;              // Stores input data for sign and decrypt operations

        HBufC8* iExportBuf;           // Holds the exported key data
        
        CDevCertOpenedKeySrv* iOpenedKey;     // The key object for crypto operations
        
        CDSASignature* iDSASignature;     // Signature to return for DSA sign operation
        
        CRSASignature* iRSASignature;     // Signature to return for RSA sign operation
        
        CDevTokenDHParams* iDHParams;     // DH parameters for DU public key operation
        
        RInteger iDHPublicKeyOut;       // DH public key to return to client
        
        CDHPublicKey* iDHPublicKey;       // DH public key and params object for agree operation 
        
        HBufC8* iDHAgreedKeyOut;        // Agreed key to return to client
        
        HBufC8* iPlaintext;           // Buffer to hold decrypted data
    };

#endif  //  __CDEVCERTKEYSTORECONDUIT_H__
