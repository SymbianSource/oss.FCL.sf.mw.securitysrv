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
* Description:   Project definition file for DevTokenClient
*
*/



#ifndef __DEVTOKENCLISERV_H__
#define __DEVTOKENCLISERV_H__

#include <e32base.h>
#include "DevTokenTypesEnum.h"

const TUid KUidDevTokenServer = {0x101FB66C};


// Classes provide static lookup for tokens supported by the device tokens server
// According to either description (string) or UID

/** 
 * Stores the _LIT description of each token with its associated ETokenEnum. 
 *
 * @lib DevTokenClient.dll
 * @since S60 v3.2
 */
class RSupportedTokensArray : public TFixedArray<const TDesC*, ETotalTokensSupported>
    {
    public:
    RSupportedTokensArray();
    };


/** 
 * Stores the UID of each token type against the associated ETokenEnum. 
 *
 * @lib DevTokenClient.dll
 * @since S60 v3.2
 */
class RTokenTypeUIDLookup : public TFixedArray<TInt, ETotalTokensSupported>
    {
    public:
    RTokenTypeUIDLookup();
    };

// Client server protocol major version
//   1 => 8.0 - 8.1
//   2 => 9.0 onwards
const TInt KDevTokenProtolVersion = 2;  

// Request message enumeration between client and server.
enum TDevTokenMessages
    {
    EIdle           = 0,
    ESupportsToken        = 1,
    // For MCertStore
    EListCerts          = 10,
    EGetCert          = 11,
    EApplications       = 12,
    EIsApplicable       = 13,
    ETrusted          = 14,
    ERetrieve         = 15,

    // For MCTWritableCertStore
    EAddCert          = 16,
    ERemoveCert         = 17,
    ESetApplicability       = 18,
    ESetTrust           = 19,
    // For MKeyStore
    EListKeys           = 50,
    EGetKeyInfo           = 51,
    // For MCTKeyStoreManager
    ECreateKey            = 52,
    ECancelCreateKey        = 53,
    EImportKey            = 54,
    ECancelImportKey        = 55,
    EImportEncryptedKey       = 56,
    ECancelImportEncryptedKey   = 57,
    EExportKey            = 58,
    ECancelExportKey        = 59,
    EExportEncryptedKey       = 60,
    ECancelExportEncryptedKey   = 61,
    EDeleteKey            = 62,

    EOpenKeyRepudiableRSASign   = 64,
    EOpenKeyRepudiableDSASign   = 65,
    EOpenKeyDecrypt             = 66,
    EOpenKeyAgree               = 67,
    ECloseObject          = 68,
    ERepudiableDSASign        = 69,
    ECancelDSASign          = 70,
    ERepudiableRSASign        = 71,
    ECancelRSASign          = 72,
    EExportPublic         = 73,
    EDecryptText          = 74,
    ECancelDecrypt          = 75,
    EDHPublicKey          = 76,
    EDHAgree            = 77,
    ECancelDH           = 78,


    EGetKeyLength       = 90,
    ESetUsePolicy       = 91,
    ESetManagementPolicy    = 92,

    // For server OOM testing
    EStartOOMTest       = 100,
    EIncHeapFailPoint     = 101,
    EResetHeapFail        = 102,
    EAllocCount         = 103,

    //For Trusted Site Store
    EAddTrustSite = 125,
    EIsTrustedSite = 126,
    EGetTrustedSites = 127,
    EAddForgivenSite = 128,
    ERemoveForgivenSite = 129,
    EIsOutOfDateAllowed = 130,
    };

#endif  //  __DEVTOKENCLISERV_H__

//EOF

