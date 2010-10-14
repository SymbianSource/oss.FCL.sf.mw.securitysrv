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
* Description:  Certificate class in advances security settings
*
*/

#include "advsecsettingscertificate.h"


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CpAdvSecSettingsCertificate::CpAdvSecSettingsCertificate()
// ---------------------------------------------------------------------------
//
AdvSecSettingsCertificate::AdvSecSettingsCertificate(QObject *parent)
    : QObject(parent), mCertType(NotDefined), mLabel(), mModelIndex(0)
{
}

// ---------------------------------------------------------------------------
// CpAdvSecSettingsCertificate::~CpAdvSecSettingsCertificate()
// ---------------------------------------------------------------------------
//
AdvSecSettingsCertificate::~AdvSecSettingsCertificate()
{
}

// ---------------------------------------------------------------------------
// CpAdvSecSettingsCertificate::label()
// ---------------------------------------------------------------------------
//
const QString &AdvSecSettingsCertificate::label() const
{
    return mLabel;
}

// ---------------------------------------------------------------------------
// CpAdvSecSettingsCertificate::setLabel()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificate::setLabel(const QString &label)
{
    mLabel = label;
}

// ---------------------------------------------------------------------------
// CpAdvSecSettingsCertificate::certType()
// ---------------------------------------------------------------------------
//
AdvSecSettingsCertificate::CertificateType
    AdvSecSettingsCertificate::certType() const
{
    return mCertType;
}

// ---------------------------------------------------------------------------
// CpAdvSecSettingsCertificate::setCertType()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificate::setCertType(CertificateType type)
{
    mCertType = type;
}

// ---------------------------------------------------------------------------
// CpAdvSecSettingsCertificate::modelIndex()
// ---------------------------------------------------------------------------
//
int AdvSecSettingsCertificate::modelIndex() const
{
    return mModelIndex;
}

// ---------------------------------------------------------------------------
// CpAdvSecSettingsCertificate::setModelIndex()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificate::setModelIndex(int index)
{
    mModelIndex = index;
}

