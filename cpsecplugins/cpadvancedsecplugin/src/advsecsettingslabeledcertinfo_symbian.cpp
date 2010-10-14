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

#include "advsecsettingslabeledcertinfo_symbian.h"


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CAdvSecSettingsLabeledCertInfo::CAdvSecSettingsLabeledCertInfo()
// ---------------------------------------------------------------------------
//
CAdvSecSettingsLabeledCertInfo::CAdvSecSettingsLabeledCertInfo(
    const CCTCertInfo &aCertInfo) : iCertInfo(aCertInfo)
{
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsLabeledCertInfo::~CAdvSecSettingsLabeledCertInfo()
// ---------------------------------------------------------------------------
//
CAdvSecSettingsLabeledCertInfo::~CAdvSecSettingsLabeledCertInfo()
{
    delete iLabel;
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsLabeledCertInfo::CertInfo()
// ---------------------------------------------------------------------------
//
const CCTCertInfo &CAdvSecSettingsLabeledCertInfo::CertInfo() const
{
    return iCertInfo;
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsLabeledCertInfo::Label()
// ---------------------------------------------------------------------------
//
const TDesC &CAdvSecSettingsLabeledCertInfo::Label() const
{
    if (iLabel) {
        return *iLabel;
    }
    return KNullDesC;
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsLabeledCertInfo::SetLabelL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsLabeledCertInfo::SetLabelL(const TDesC &aLabel)
{
    if (iLabel) {
        delete iLabel;
        iLabel = NULL;
    }
    iLabel = aLabel.AllocL();
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsLabeledCertInfo::AppendLabelL()
// ---------------------------------------------------------------------------
//
void CAdvSecSettingsLabeledCertInfo::AppendLabelL(const TDesC &aSeparator,
    const TDesC &aAdditionalText)
{
    if (iLabel && iLabel->Length()) {
        TInt length = iLabel->Length() + aSeparator.Length() + aAdditionalText.Length();
        iLabel = iLabel->ReAllocL(length);
        TPtr labelPtr = iLabel->Des();
        labelPtr.Append(aSeparator);
        labelPtr.Append(aAdditionalText);
    } else {
        SetLabelL(aAdditionalText);
    }
}

// ---------------------------------------------------------------------------
// CAdvSecSettingsLabeledCertInfo::Compare()
// ---------------------------------------------------------------------------
//
TInt CAdvSecSettingsLabeledCertInfo::Compare(const CAdvSecSettingsLabeledCertInfo& aLeft,
    const CAdvSecSettingsLabeledCertInfo& aRight)
{
    return (aLeft.Label().CompareF(aRight.Label()));
}

