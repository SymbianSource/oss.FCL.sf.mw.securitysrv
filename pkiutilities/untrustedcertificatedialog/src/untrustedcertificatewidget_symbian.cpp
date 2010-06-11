/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Device dialog plugin that shows untrusted certificate
*               dialog for TLS server authentication failure errors.
*
*/

#include "untrustedcertificatewidget.h"
#include "untrustedcertificatedefinitions.h"
#include "untrustedcertificateinfobase.h"
#include "untrustedcertificateinfo_symbian.h"
#include <x509cert.h>                   // CX509Certificate


// ======== LOCAL FUNCTIONS ========

// ----------------------------------------------------------------------------
// DoProcessEncodedCertificateL()
// ----------------------------------------------------------------------------
//
UntrustedCertificateInfoBase* DoProcessEncodedCertificateL( const QByteArray &encodedCert )
{
    TPtrC8 ptr8( reinterpret_cast<const TText8*>(encodedCert.constData()), encodedCert.length());
    CX509Certificate* cert = CX509Certificate::NewLC( ptr8 );

    UntrustedCertificateInfoSymbian *info = 0;
    info = new UntrustedCertificateInfoSymbian(*cert);
    CleanupStack::PopAndDestroy( cert );

    return info;
}


// ======== MEMBER FUNCTIONS ========

// ----------------------------------------------------------------------------
// UntrustedCertificateWidget::isInvalidCertificate()
// ----------------------------------------------------------------------------
//
bool UntrustedCertificateWidget::isCertificateValid()
{
    return (mValidationError != ESignatureInvalid && mValidationError != ECertificateRevoked);
}

// ----------------------------------------------------------------------------
// UntrustedCertificateWidget::processEncodedCertificate()
// ----------------------------------------------------------------------------
//
void UntrustedCertificateWidget::processEncodedCertificate(const QByteArray &encodedCert)
{
    UntrustedCertificateInfoBase *info = 0;
    QT_TRAP_THROWING(info = DoProcessEncodedCertificateL(encodedCert));
    if (mCertificateInfo) {
        delete mCertificateInfo;
        mCertificateInfo = 0;
    }
    mCertificateInfo = info;
}

// ----------------------------------------------------------------------------
// UntrustedCertificateWidget::descriptionText()
// ----------------------------------------------------------------------------
//
QString UntrustedCertificateWidget::descriptionText()
{
    QString text;
    if (!isCertificateValid()) {
        //: Information note text shown if the server certificate has an invalid signature
        // TODO: localised UI string
        text = tr("'%1' has sent an invalid certificate. Connection cannot be created.");
    } else if (mValidationError == EValidatedOK || mValidationError == EDateOutOfRange) {
        bool isDateValid = mCertificateInfo->isDateValid();
        bool isSiteValid = mCertificateInfo->commonNameMatches(mServerName);
        if (!isDateValid && !isSiteValid) {
            // TODO: hbTrId("txt_untrustedcert_dialog _accept_site_ood")
            text = tr("'%1' has sent a certificate with different site name and which is out of date. Accept anyway?");
        } else if (!isSiteValid) {
            // TODO: hbTrId("txt_untrustedcert_dialog _accept_site"
            text = tr("'%1' has sent a certificate with different site name. Accept anyway?");
        } else {
            // TODO: hbTrId("txt_untrustedcert_dialog_accept_ood")
            text = tr("'%1' has sent a certificate which is out of date. Accept anyway?");
        }
    } else {
        bool isDateValid = mCertificateInfo->isDateValid();
        bool isSiteValid = mCertificateInfo->commonNameMatches(mServerName);
        if (!isDateValid && !isSiteValid) {
            // TODO: hbTrId("txt_untrustedcert_dialog _accept_untrusted_site_ood"
            text = tr("'%1' has sent an untrusted certificate with different site name and which is out of date. Accept anyway?");
        } else if (!isSiteValid) {
            // TODO: hbTrId("txt_untrustedcert_dialog _accept_untrusted_site"
            text = tr("'%1' has sent an untrusted certificate with different site name. Accept anyway?");
        } else if (!isDateValid) {
            // TODO: hbTrId("txt_untrustedcert_dialog _accept_untrusted_ood")
            text = tr("'%1' has sent an untrusted certificate which is out of date. Accept anyway?");
        } else {
            // TODO: hbTrId("txt_untrustedcert_dialog _accept_untrusted"
            text = tr("'%1' has sent an untrusted certificate. Accept anyway?");
        }
    }
    return text;
}

