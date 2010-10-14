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
* Description:  Platform specific private certificate handling model
*
*/


#include "advsecsettingscertificatemodel_stub_p.h"
#include "advsecsettingscertificate.h"
#include <QStandardItemModel>


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModelPrivate::AdvSecSettingsCertificateModelPrivate()
// ---------------------------------------------------------------------------
//
AdvSecSettingsCertificateModelPrivate::AdvSecSettingsCertificateModelPrivate(
    AdvSecSettingsCertificateModel *q) : QObject(0), q_ptr(q)
{
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModelPrivate::~AdvSecSettingsCertificateModelPrivate()
// ---------------------------------------------------------------------------
//
AdvSecSettingsCertificateModelPrivate::~AdvSecSettingsCertificateModelPrivate()
{
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModelPrivate::initialize()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModelPrivate::initialize()
{
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModelPrivate::getCertificates()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModelPrivate::getCertificates(
    QList<AdvSecSettingsCertificate*> &/*certList*/)
{
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModelPrivate::getCertificateDetails()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModelPrivate::getCertificateDetails(
    const AdvSecSettingsCertificate &/*cert*/,
    QMap<int,QString>& /*details*/)
{
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModelPrivate::isDeletable()
// ---------------------------------------------------------------------------
//
bool AdvSecSettingsCertificateModelPrivate::isDeletable(
    const AdvSecSettingsCertificate &/*cert*/) const
{
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModelPrivate::deleteCertificate()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModelPrivate::deleteCertificate(
    const AdvSecSettingsCertificate &/*cert*/)
{
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModelPrivate::getTrustSettings()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModelPrivate::getTrustSettings(
    const AdvSecSettingsCertificate &/*cert*/,
    QMap<int,bool> &/*usageIdAndTrust*/)
{
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModelPrivate::setTrustSettings()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModelPrivate::setTrustSettings(
    const AdvSecSettingsCertificate &/*cert*/,
    const QMap<int,bool> &/*usageIdAndTrust*/)
{
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModelPrivate::getCertificateUsageNames()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModelPrivate::getCertificateUsageNames(
    QMap<int,QString> &/*usageIdAndName*/)
{
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModelPrivate::moveToPersonalCertificates()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModelPrivate::moveToPersonalCertificates(
    const AdvSecSettingsCertificate &/*cert*/)
{
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModelPrivate::moveToDeviceCertificates()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModelPrivate::moveToDeviceCertificates(
    const AdvSecSettingsCertificate &/*cert*/)
{
}

