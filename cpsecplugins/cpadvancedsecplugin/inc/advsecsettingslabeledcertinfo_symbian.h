/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of the License "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  Helper class to contain a labeled CCTCertInfo object
*
*/

#ifndef CPADVSECSETTINGSLABELEDCERTINFO_SYMBIAN_H
#define CPADVSECSETTINGSLABELEDCERTINFO_SYMBIAN_H

#include <e32base.h>                // CBase

class CCTCertInfo;


/**
 * Displayed certificate labels are constructed using CCTCertInfo
 * label and subject name. CAdvSecSettingsLabeledCertInfo class
 * is used as contained class for labeled CCTCertInfo objects.
 */
class CAdvSecSettingsLabeledCertInfo : public CBase
{
public:     // constructor and destructor
    CAdvSecSettingsLabeledCertInfo(const CCTCertInfo &aCertInfo);
    ~CAdvSecSettingsLabeledCertInfo();

public:     // new functions
    const CCTCertInfo &CertInfo() const;
    const TDesC &Label() const;
    void SetLabelL(const TDesC &aLabel);
    void AppendLabelL(const TDesC &aSeparator, const TDesC &aAdditionalText);
    static TInt Compare(const CAdvSecSettingsLabeledCertInfo& aLeft,
        const CAdvSecSettingsLabeledCertInfo& aRight);

private:    // data
    const CCTCertInfo& iCertInfo;
    HBufC* iLabel;
};

#endif // CPADVSECSETTINGSLABELEDCERTINFO_SYMBIAN_H

