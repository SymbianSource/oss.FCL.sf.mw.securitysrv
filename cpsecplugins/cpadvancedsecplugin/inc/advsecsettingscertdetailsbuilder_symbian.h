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
* Description:  Helper class to construct certificate details
*
*/

#ifndef ADVSECSETTINGSCERTDETAILSBUILDER_SYMBIAN_H
#define ADVSECSETTINGSCERTDETAILSBUILDER_SYMBIAN_H

#include <e32base.h>                    // CActive
#include <ct/rmpointerarray.h>          // RMPointerArray
#include <unifiedcertstore.h>           // CUnifiedCertStore
#include <QMap>

class RFs;
class CUnifiedCertStore;
class CUnifiedKeyStore;
class CCTCertInfo;
class CCTKeyInfo;
class CCertAttributeFilter;
class TCTKeyAttributeFilter;


/**
 * Certificate details helper class.
 */
class CAdvSecSettingsCertDetailsBuilder : public CActive
{
public:     // constructor and destructor
    static CAdvSecSettingsCertDetailsBuilder *NewL(RFs &aFs, CUnifiedCertStore &aCertStore);
    ~CAdvSecSettingsCertDetailsBuilder();

public:     // new functions
    void GetDetailsL(const CCTCertInfo &aCert, QMap<int,QString> &aDetails,
        TRequestStatus &aStatus);

protected:  // from CActive
    void DoCancel();
    void RunL();
    TInt RunError(TInt aError);

private:    // new functions
    CAdvSecSettingsCertDetailsBuilder(RFs &aFs, CUnifiedCertStore &aCertStore);
    void ConstructL();
    void InitializeKeyStoreL();
    void RetrieveCertificateL();
    void ListKeysL();
    void ReturnCertificateDetailsL();
    void AppendLabelL();
    void AppendIssuerL();
    void AppendSubjectL();
    void AppendValidityPeriodL();
    void AppendLocationL();
    void AppendFormatL();
    void AppendKeyUsageL();
    void AppendAlgorithmL();
    void AppendSerialNumberL();
    void AppendFingerprintsL();
    void AppendPublicKeyL();
    void AppendTrustedSitesL();
    QString CertificateFormatStringL(const TCertificateFormat aFormat) const;
    QString AlgorithmNameStringL(TAlgorithmId aAlgorithmId) const;

private:    // data
    RFs &iFs;
    CUnifiedCertStore &iCertStore;
    const CCTCertInfo *iCertInfo;   // not owned
    TRequestStatus *iClientStatus;  // not owned
    QMap<int,QString> *iDetails;    // not owned

    CCertificate *iCertificate;
    HBufC8 *iCertificateUrl;
    TPtr8 iCertUrlPtr;

    CUnifiedKeyStore *iKeyStore;
    TCTKeyAttributeFilter *iKeyFilter;
    RMPointerArray<CCTKeyInfo> iKeys;

    enum TDetailsBuilderState {
        ENotInitialized,
        EInitializingKeyStore,
        ERetrievingCertificate,
        EListingKeys,
        EIdle,
        EFailed
    } iState;
};

#endif // ADVSECSETTINGSCERTDETAILSBUILDER_SYMBIAN_H
