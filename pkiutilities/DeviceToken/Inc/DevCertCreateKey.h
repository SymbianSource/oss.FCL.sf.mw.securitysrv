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
* Description:   The header file of DevCertCreateKey
*
*/



#ifndef __DEVCERTCREATEKEY_H__
#define __DEVCERTCREATEKEY_H__

#include <bigint.h>
#include <e32std.h>
#include <mctkeystore.h>
#include <asymmetrickeys.h>
#include "DevTokenDataTypes.h"

class CSimpleDHKey;
class CDevCertKeyCreatorData;

_LIT(KCreateKeyName, "AsymmetricKeysCreator");

/**
 *  class CDevCertKeyCreator
 *
 * Server side
 * Class to make key creation "asynchronous" by spinning off a thread
 * to make the synchronous call to the appropriate key creation function
 * The thread entry point is a static member of this class, which holds the 
 * thread and thread parameter data too.  When the key has been created, the
 * thread terminates using Rendezvous to notify the main thread.  The created key
 * is returned through the CAsymmetricCipher member of CKeyCreatorData
 *
 *  @lib 
 *  @since S60 v3.2
 */
class CDevCertKeyCreator : public CActive
    {
    public:
        
        CDevCertKeyCreator();
        
        ~CDevCertKeyCreator();
        
    public: //  Spin a thread to create an appropriate key, if successful, left on CleanupStack
        
        void DoCreateKeyAsyncL(CDevTokenKeyInfo::EKeyAlgorithm aAlgorithm, TInt aSize, TRequestStatus& aStatus);
        
    public:
        
        //  JCS this needs improvement when new crypto api is reviewed
        CRSAKeyPair* GetCreatedRSAKey();
        
        CDSAKeyPair* GetCreatedDSAKey();
        
        void GetCreatedDHKey(RInteger& aDHKey);
        
    protected:
        
        void DoCancel();
        
        void RunL();
        
        TInt RunError(TInt anError);
        
    private:
        
        static TInt CreatorThreadEntryPoint(TAny*);
        
    private:
        
        enum TAction {EIdle, EReadyToCreateKey, ECreatedKey};
        
        TAction iAction;
    private:
        
        TRequestStatus* iClientStatus;
        
        RThread iCreatorThread;
        
    private:
        
        class CDevCertKeyCreatorData : public CBase
            {
            public:
        
                CDevCertKeyCreatorData(CDevTokenKeyInfo::EKeyAlgorithm aAlgorithm, TInt aSize);
        
                ~CDevCertKeyCreatorData();
                
            public: //  Don't bother hiding the data from myself
        
                TInt iSize;
        
                //  Algorithm identified by iKeyAlgorithm
                CDevTokenKeyInfo::EKeyAlgorithm iKeyAlgorithm;

            union CreatedKey
                {
                CRSAKeyPair* iRSAKey;
                CDSAKeyPair* iDSAKey;
                CSimpleDHKey* iDHKey;
                } iKey;
            };

    CDevCertKeyCreatorData* iCreateData;
    };

#endif  //  __DEVCERTCREATEKEY_H__

//EOF

