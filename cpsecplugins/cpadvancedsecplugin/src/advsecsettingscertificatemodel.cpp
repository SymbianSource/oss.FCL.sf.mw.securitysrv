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
* Description:  Advanced security settings certificate model
*
*/

#include "advsecsettingscertificatemodel.h"
#include <QStandardItemModel>

#if defined(Q_OS_SYMBIAN)
#include "advsecsettingscertificatemodel_symbian_p.h"
#else
#include "advsecsettingscertificatemodel_stub_p.h"
#endif


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModel::AdvSecSettingsCertificateModel()
// ---------------------------------------------------------------------------
//
AdvSecSettingsCertificateModel::AdvSecSettingsCertificateModel(QObject *parent) :
    QObject(parent), d_ptr(0)
{
    d_ptr = new AdvSecSettingsCertificateModelPrivate(this);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModel::~AdvSecSettingsCertificateModel()
// ---------------------------------------------------------------------------
//
AdvSecSettingsCertificateModel::~AdvSecSettingsCertificateModel()
{
    delete d_ptr;
    d_ptr = 0;
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModel::initialize()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModel::initialize()
{
    if (d_ptr) {
        d_ptr->initialize();
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModel::getCertificates()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModel::getCertificates(
    QList<AdvSecSettingsCertificate*> &certList)
{
    if (d_ptr) {
        d_ptr->getCertificates(certList);
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModel::getCertificateDetails()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModel::getCertificateDetails(
    const AdvSecSettingsCertificate &cert, QMap<int,QString> &details)
{
    if (d_ptr) {
        d_ptr->getCertificateDetails(cert, details);
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModel::isDeletable()
// ---------------------------------------------------------------------------
//
bool AdvSecSettingsCertificateModel::isDeletable(
    const AdvSecSettingsCertificate &cert) const
{
    if (d_ptr) {
        return d_ptr->isDeletable(cert);
    }
    return false;
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModel::deleteCertificate()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModel::deleteCertificate(
    const AdvSecSettingsCertificate &cert)
{
    if (d_ptr) {
        d_ptr->deleteCertificate(cert);
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModel::getTrustSettings()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModel::getTrustSettings(
    const AdvSecSettingsCertificate &cert, QMap<int,bool> &usageIdAndTrust)
{
    if (d_ptr) {
        d_ptr->getTrustSettings(cert, usageIdAndTrust);
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModel::setTrustSettings()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModel::setTrustSettings(
    const AdvSecSettingsCertificate &cert,
    const QMap<int,bool> &usageIdAndTrust)
{
    if (d_ptr) {
        d_ptr->setTrustSettings(cert, usageIdAndTrust);
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModel::getCertificateUsageNames()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModel::getCertificateUsageNames(
    QMap<int,QString> &usageIdAndName)
{
    if (d_ptr) {
        d_ptr->getCertificateUsageNames(usageIdAndName);
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModel::moveToPersonalCertificates()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModel::moveToPersonalCertificates(
    const AdvSecSettingsCertificate &cert)
{
    if (d_ptr) {
        d_ptr->moveToPersonalCertificates(cert);
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModel::moveToDeviceCertificates()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModel::moveToDeviceCertificates(
    const AdvSecSettingsCertificate &cert)
{
    if (d_ptr) {
        d_ptr->moveToDeviceCertificates(cert);
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModel::handleInitializeCompleted()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModel::handleInitializeCompleted()
{
    emit initializeCompleted();
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModel::handleGetCertificatesCompleted()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModel::handleGetCertificatesCompleted()
{
    emit getCertificatesCompleted();
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModel::handleGetCertificateDetailsCompleted()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModel::handleGetCertificateDetailsCompleted()
{
    emit getCertificateDetailsCompleted();
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModel::handleDeleteCertificateCompleted()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModel::handleDeleteCertificateCompleted()
{
    emit deleteCertificateCompleted();
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModel::handleGetTrustSettingsCompleted()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModel::handleGetTrustSettingsCompleted()
{
    emit getTrustSettingsCompleted();
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModel::handleSetTrustSettingsCompleted()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModel::handleSetTrustSettingsCompleted()
{
    emit setTrustSettingsCompleted();
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModel::handleMoveToPersonalCertificateCompleted()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModel::handleMoveToPersonalCertificateCompleted()
{
    emit moveToPersonalCertificatesCompleted();
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModel::handleMoveToDeviceCertificatesCompleted()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModel::handleMoveToDeviceCertificatesCompleted()
{
    emit moveToDeviceCertificatesCompleted();
}

// ---------------------------------------------------------------------------
// AdvSecSettingsCertificateModel::handleError()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsCertificateModel::handleError(int error)
{
    emit errorOccurred(error);
}

