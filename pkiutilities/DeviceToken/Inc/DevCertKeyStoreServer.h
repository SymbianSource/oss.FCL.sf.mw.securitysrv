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
* Description:   The header file of DevCertKeyStoreServer
*
*/



#ifndef __DEVCERTKEYSTORESERVER_H__
#define __DEVCERTKEYSTORESERVER_H__

#include <mctkeystoremanager.h>
#include <s32file.h>
#include <secdlg.h>

class CDecPKCS8Data;
class CDevCertRSARepudiableSigner;
class CDevCertDSARepudiableSigner;
class CDevCertRSADecryptor;
class CDevCertDHAgreement;
class CDevCertKeyCreator;
class CDevCertKeyData;
class CDevCertKeyDataManager;
class CDevCertOpenedKeySrv;
class CDevTokenKeyInfo;
class CDevCertKeyStoreSession;
class CDevCertKeyStoreConduit;

/** 
 * Server side implementation of keystore interfaces as an active object.
 * 
 *  @lib 
 *  @since S60 v3.2
 */
class CDevCertKeyStoreServer : public CActive
    {
    public:
        
        static CDevCertKeyStoreServer* NewL();
        
        virtual ~CDevCertKeyStoreServer();
        
        CDevCertKeyStoreSession* CreateSessionL();
        
        void RemoveSession(CDevCertKeyStoreSession& aSession);
        
        void ServiceRequestL(const RMessage2& aMessage, CDevCertKeyStoreSession& aSession);
    
    public:
        
        // For MKeyStore
        void ListL(const TCTKeyAttributeFilter& aFilter, RPointerArray<CDevTokenKeyInfo>& aKeys);
        
        void GetKeyInfoL(TInt aObjectId, CDevTokenKeyInfo*& aInfo);
        
        CDevCertOpenedKeySrv* OpenKeyL(TInt aHandle, TUid aOpenedKeyType);
        
        void ExportPublicL(TInt aObjectId, TDes8& aOut);
        
        TInt GetKeyLengthL(TInt aObjectId);
    
    public:
        
        // For MCTKeyStoreManager
        void CreateKey(CDevTokenKeyInfo& aReturnedKey, TRequestStatus& aStatus);
        
        void CancelCreateKey();
        
        void ImportKey(const TDesC8& aKey, CDevTokenKeyInfo& aReturnedKey, TBool aIsEncrypted,TRequestStatus& aStatus);
        
        void CancelImportKey();
        
        void ExportKey(TInt aObjectId, const TPtr8& aKey, TRequestStatus& aStatus); 
        
        void CancelExportKey();
        
        void DeleteKeyL(TInt aObjectId);
        
        void SetUsePolicyL(TInt aObjectId, const TSecurityPolicy& aPolicy);
        
        void SetManagementPolicyL(TInt aObjectId, const TSecurityPolicy& aPolicy);

    private:
        
        CDevCertKeyStoreServer();
        
        void ConstructL();
    
    private:
        
        //  From CActive
        void RunL();
        
        TInt RunError(TInt aError);
        
        void DoCancel();
    
    private:
        
        enum ECurrentAction
            {
            EIdle,
            EImportOpenPrivateStream,
            ECreateKeyCreate,
            ECreateKeyFinal,
            EImportKey, 
            EExportKey,
            EKeyCreated
            };

        /**
        * The operations that can create a new key, used by CheckKeyAttributes.
        */
        enum TNewKeyOperation
            {
            ENewKeyCreate,
            ENewKeyImportPlaintext
            };

    private:
    
        CDevCertKeyStoreServer(const CDevCertKeyStoreServer&);      //  No copying
    
        CDevCertKeyStoreServer& operator=(const CDevCertKeyStoreServer&); //  No copying
    
    private:

        TInt CheckKeyAttributes(CDevTokenKeyInfo& aKey, TNewKeyOperation aOp);
        
        TInt CheckKeyAlgorithmAndSize(CDevTokenKeyInfo& aKey);
        
        void DoCreateKeyL();
        
        void DoStoreKeyL();
        
        void DoImportKeyL();
        
        void DoExportKeyL(TInt aObjectId, const TPtr8& aKey, TRequestStatus& aStatus);
        
        void CompleteKeyExportL();
        
        void OpenPrivateStream();

    private:
    
        void PKCS8ToKeyL(CDecPKCS8Data* aPKCS8Data);
        
        TBool KeyMatchesFilterL(const CDevTokenKeyInfo& aInfo,
        
        const TCTKeyAttributeFilter& aFilter);
    
    private:
        
        CDevCertKeyDataManager* iKeyDataManager;
        
        CDevCertKeyStoreConduit* iConduit;
        
        RPointerArray<CDevCertKeyStoreSession> iSessions;
        
        const RMessage2* iMessage;    //The request currently being processed, not owned by us
        
        CDevCertKeyStoreSession* iSession;      // The session of the request being processed, not owned through here
        
        TRequestStatus* iCallerRequest;
        
        ECurrentAction iAction;
    
    private:
        
        CDevTokenKeyInfo* iKeyInfo;
        
        CDevCertKeyCreator* iKeyCreator;

        TPtr8 iExportBuf;

        TInt iObjectId;
    
        TPtrC8 iPKCS8Data;
        
        TBool iImportingEncryptedKey;
        
        TPINValue iPassword;
        
        const CDevCertKeyData* iKeyData;
    };

#endif  //  __CDevCertKeyStoreServer_H__

//EOF

