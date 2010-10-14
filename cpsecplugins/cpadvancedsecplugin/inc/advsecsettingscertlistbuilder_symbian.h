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
* Description:  Helper class to construct certificate list
*
*/

#ifndef ADVSECSETTINGSCERTLISTBUILDER_SYMBIAN_H
#define ADVSECSETTINGSCERTLISTBUILDER_SYMBIAN_H

#include <e32base.h>                    // CActive
#include "advsecsettingscertificate.h"  // CertificateType
#include <ct/rmpointerarray.h>          // RMPointerArray
#include <unifiedcertstore.h>           // CUnifiedCertStore
#include <QList>

class RFs;
class CCTCertInfo;
class CCertAttributeFilter;
class CAdvSecSettingsLabeledCertInfo;


/**
 * Certificate list builder helper class.
 */
class CAdvSecSettingsCertListBuilder : public CActive
{
public:     // constructor and destructor
    static CAdvSecSettingsCertListBuilder *NewL(RFs &aFs, CUnifiedCertStore &aCertStore);
    ~CAdvSecSettingsCertListBuilder();

public:     // new functions
    void GetCertificateList(QList<AdvSecSettingsCertificate*> &certList,
        RMPointerArray<CCTCertInfo> &aCertInfoArray, TRequestStatus &aStatus);

protected:  // from CActive
    void DoCancel();
    void RunL();
    TInt RunError(TInt aError);

private:    // new functions
    CAdvSecSettingsCertListBuilder(RFs &aFs, CUnifiedCertStore &aCertStore);
    void ConstructL();
    void ResetCertAttributeFilter();
    void ProcessFirstCertificateL();
    void ProcessNextCertificateL();
    void StartProcessingCertificateL();
    void CompleteProcessingCertificateL();
    void ReturnCertificateListL();
    AdvSecSettingsCertificate::CertificateType CertType(const CCTCertInfo &aCertInfo);

private:    // data
    RFs &iFs;
    CUnifiedCertStore &iCertStore;
    TRequestStatus *iClientStatus;  // not owned
    QList<AdvSecSettingsCertificate*> *iCertList;  // not owned
    RMPointerArray<CCTCertInfo> *iClientInfoArray;  // not owned
    RMPointerArray<CCTCertInfo> iCertInfoArray;
    CCertAttributeFilter *iCertAttributeFilter;
    TInt iCertInfoIndex;
    HBufC8 *iCertificate;
    TPtr8 iCertPtr;
    RPointerArray<CAdvSecSettingsLabeledCertInfo> iLabeledCertInfos;

    enum TCertListBuilderState {
        EIdle,
        EListingCerts,
        ERetrievingCert,
        EProcessingCert
    } iState;
};

#endif // ADVSECSETTINGSCERTLISTBUILDER_SYMBIAN_H
