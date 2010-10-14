/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0""
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  Helper class to move certificates
*
*/

#ifndef ADVSECSETTINGSCERTMOVER_SYMBIAN_H
#define ADVSECSETTINGSCERTMOVER_SYMBIAN_H

#include <e32base.h>                    // CActive
#include <ct/rmpointerarray.h>          // RMPointerArray
#include <ct/tcttokenobjecthandle.h>    // TCTTokenObjectHandle

class RFs;
class CUnifiedCertStore;
class CUnifiedKeyStore;
class CCTCertInfo;
class CCTKeyInfo;
class CCertAttributeFilter;
class TCTKeyAttributeFilter;
class MCTWritableCertStore;
class MCTKeyStoreManager;


/**
 * Certificate mover helper class.
 */
class CAdvSecSettingsCertMover : public CActive
{
public:     // constructor and destructor
    static CAdvSecSettingsCertMover *NewL(RFs &aFs);
    ~CAdvSecSettingsCertMover();

public:     // new functions
    void Move(const CCTCertInfo &aCert, TInt aSourceStoreTokenId,
        TInt aTargetStoreTokenId, TRequestStatus &aStatus);

protected:  // from CActive
    void DoCancel();
    void RunL();
    TInt RunError(TInt aError);

private:    // new functions
    CAdvSecSettingsCertMover(RFs &aFs);
    void ConstructL();
    void StartMoveOperationL();
    TInt CorrespondingKeyStoreTokenId(TInt aCertStoreTokenId);
    void StartMovingKeysL();
    void FindSourceAndTargetKeyStoresL();
    void FindSourceAndTargetCertStoreL();
    void ExportFirstKeyL();
    void ExportOneKeyL();
    void ExportNextKeyL();
    void SaveExportedKeyL();
    void DeleteOriginalKeyL();
    void StartMovingCertificatesL();
    void RetrieveFirstCertL();
    void RetrieveOneCertL();
    void RetrieveNextCertL();
    void SaveRetrievedCertL();
    void DeleteOriginalCertL();


private:    // data
    RFs &iFs;
    const CCTCertInfo *iCertInfo;   // not owned
    TInt iSourceCertStoreTokenId;
    TInt iTargetCertStoreTokenId;
    TRequestStatus *iClientStatus;  // not owned
    HBufC8 *iDataBuffer;

    CUnifiedCertStore *iCertStore;
    MCTWritableCertStore *iSourceCertStore; // not owned
    MCTWritableCertStore *iTargetCertStore; // not owned
    CCertAttributeFilter *iCertFilter;
    RMPointerArray<CCTCertInfo> iCerts;
    TInt iCertIndex;
    TPtr8 iDataPtr;

    CUnifiedKeyStore *iKeyStore;
    MCTKeyStoreManager *iSourceKeyStore;    // not owned
    MCTKeyStoreManager *iTargetKeyStore;    // not owned
    TCTKeyAttributeFilter *iKeyFilter;
    RMPointerArray<CCTKeyInfo> iKeys;
    TInt iKeyIndex;
    TCTTokenObjectHandle iSourceKeyHandle;
    CCTKeyInfo *iSavedKeyInfo;

    enum TMoverState {
        ENotInitialized,
        EInitializingCertStore,
        EInitializingKeyStore,
        EIdle,
        EMovingKeyListingKeys,
        EMovingKeyExportingKeys,
        EMovingKeyImportingKeys,
        EMovingKeyDeletingOriginal,
        EMovingCertListingCerts,
        EMovingCertRetrievingCerts,
        EMovingCertAddingCerts,
        EMovingCertDeletingOriginal,
        EFailed
    } iState;
};

#endif // ADVSECSETTINGSCERTMOVER_SYMBIAN_H
