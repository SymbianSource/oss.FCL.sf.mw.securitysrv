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
* Description: Certificate info class for TLS untrusted certificate dialog.
*
*/

#include "untrustedcertificateinfo_symbian.h"
#include <signed.h>                             // TAlgorithmId
#include <x509cert.h>                           // CX509Certificate
#include <QTextStream>


// ======== LOCAL FUNCTIONS ========

// ----------------------------------------------------------------------------
// mapAlgorithm()
// ----------------------------------------------------------------------------
//
UntrustedCertificateInfoBase::Algorithm mapAlgorithm(TAlgorithmId aAlgId)
{
    UntrustedCertificateInfoBase::Algorithm algorithm =
        UntrustedCertificateInfoBase::Unknown;
    switch(aAlgId) {
        case ERSA:
            algorithm = UntrustedCertificateInfoBase::RSA;
            break;
        case EDSA:
            algorithm = UntrustedCertificateInfoBase::DSA;
            break;
        case EDH:
            algorithm = UntrustedCertificateInfoBase::DH;
            break;
        case EMD2:
            algorithm = UntrustedCertificateInfoBase::MD2;
            break;
        case EMD5:
            algorithm = UntrustedCertificateInfoBase::MD5;
            break;
        case ESHA1:
            algorithm = UntrustedCertificateInfoBase::SHA1;
            break;
        case ESHA224:
            algorithm = UntrustedCertificateInfoBase::SHA224;
            break;
        case ESHA256:
            algorithm = UntrustedCertificateInfoBase::SHA256;
            break;
        case ESHA384:
            algorithm = UntrustedCertificateInfoBase::SHA384;
            break;
        case ESHA512:
            algorithm = UntrustedCertificateInfoBase::SHA512;
            break;
        default:
            break;
    }
    return algorithm;
}

// ----------------------------------------------------------------------------
// convertDateTime()
// ----------------------------------------------------------------------------
//
QDateTime convertDateTime(const TTime& aTime)
{
    const TDateTime &symbianDateTime = aTime.DateTime();

    QDateTime dateTime;
    QDate date(symbianDateTime.Year(), symbianDateTime.Month()+1, symbianDateTime.Day()+1);
    QTime time(symbianDateTime.Hour(), symbianDateTime.Minute(), symbianDateTime.Second());
    dateTime.setDate(date);
    dateTime.setTime(time);

    return dateTime;
}


// ======== MEMBER FUNCTIONS ========

// ----------------------------------------------------------------------------
// UntrustedCertificateInfoSymbian::UntrustedCertificateInfoSymbian()
// ----------------------------------------------------------------------------
//
UntrustedCertificateInfoSymbian::UntrustedCertificateInfoSymbian(
    const CX509Certificate& aCert) : UntrustedCertificateInfoBase(), iCert(0)
{
    QT_TRAP_THROWING(ConstructL(aCert));
}

// ----------------------------------------------------------------------------
// UntrustedCertificateInfoSymbian::~UntrustedCertificateInfoSymbian()
// ----------------------------------------------------------------------------
//
UntrustedCertificateInfoSymbian::~UntrustedCertificateInfoSymbian()
{
    delete iCert;
}

// ----------------------------------------------------------------------------
// UntrustedCertificateInfoSymbian::commonNameMatches()
// ----------------------------------------------------------------------------
//
bool UntrustedCertificateInfoSymbian::commonNameMatches(const QString &siteName) const
{
    bool matches = false;
    QT_TRAP_THROWING(matches = CommonNameMatchesL(siteName));
    return matches;
}

// ----------------------------------------------------------------------------
// UntrustedCertificateInfoSymbian::isPermanentAcceptAllowed()
// ----------------------------------------------------------------------------
//
bool UntrustedCertificateInfoSymbian::isPermanentAcceptAllowed() const
{
    return isDateValid();
}

// ----------------------------------------------------------------------------
// UntrustedCertificateInfoSymbian::certificateDetails()
// ----------------------------------------------------------------------------
//
QString UntrustedCertificateInfoSymbian::certificateDetails() const
{
    QString details;
    QTextStream stream(&details);
    // TODO: localised UI strings needed
    stream << tr("Issuer:\n%1\n").arg(issuerName());
    stream << endl;
    stream << tr("Subject:\n%1\n").arg(subjectName());
    stream << endl;
    stream << tr("Valid from:\n%1\n").arg(validFrom().toString());
    stream << endl;
    stream << tr("Valid until:\n%1\n").arg(validTo().toString());
    stream << endl;
    stream << tr("Certificate format:\n%1\n").arg(format());
    stream << endl;
    stream << tr("Algorithm:\n%1\n").arg(combinedAlgorithmName());
    stream << endl;
    stream << tr("Serial number:\n%1\n").arg(formattedSerialNumber());
    stream << endl;
    stream << tr("Fingerprint (SHA1):\n%1\n").arg(formattedFingerprint());
    // TODO: MD5 fingerprint missing
    return details;
}

// ----------------------------------------------------------------------------
// UntrustedCertificateInfoSymbian::ConstructL()
// ----------------------------------------------------------------------------
//
void UntrustedCertificateInfoSymbian::ConstructL(const CX509Certificate& aCert)
{
    ASSERT( iCert == 0 );
    iCert = CX509Certificate::NewL( aCert );

    HBufC16* subjectBuf = iCert->SubjectL();
    mSubjectName = QString::fromUtf16(subjectBuf->Ptr(), subjectBuf->Length());
    delete subjectBuf;

    HBufC16* issuerBuf = iCert->IssuerL();
    mIssuerName = QString::fromUtf16(issuerBuf->Ptr(), issuerBuf->Length());
    delete issuerBuf;

    TPtrC8 fingerprint = iCert->Fingerprint();
    mFingerprint = QByteArray::fromRawData(
        reinterpret_cast<const char*>(fingerprint.Ptr()), fingerprint.Length());

    TPtrC8 serialNumber = iCert->SerialNumber();
    mSerialNumber = QByteArray::fromRawData(
        reinterpret_cast<const char*>(serialNumber.Ptr()), serialNumber.Length());

    const CValidityPeriod& validityPeriod = iCert->ValidityPeriod();
    mValidFrom = convertDateTime(validityPeriod.Start());
    mValidTo = convertDateTime(validityPeriod.Finish());

    mFormat = X509Certificate;

    const CSigningAlgorithmIdentifier& alg = iCert->SigningAlgorithm();
    mDigestAlgorithm = mapAlgorithm(alg.DigestAlgorithm().Algorithm());
    mAsymmetricAlgorithm = mapAlgorithm(alg.AsymmetricAlgorithm().Algorithm());
}

// ----------------------------------------------------------------------------
// UntrustedCertificateInfoSymbian::CommonNameMatchesL()
// ----------------------------------------------------------------------------
//
bool UntrustedCertificateInfoSymbian::CommonNameMatchesL(const QString &siteName) const
{
    bool matches = false;
    const CX500DistinguishedName& distinguishedName = iCert->SubjectName();
    HBufC* commonNameSymbian = distinguishedName.ExtractFieldL( KX520CommonName );
    if (commonNameSymbian) {
        CleanupStack::PushL(commonNameSymbian);
        QString commonName = QString::fromRawData(
            reinterpret_cast<const QChar*>(commonNameSymbian->Ptr()),
            commonNameSymbian->Length());
        matches = ( commonName == siteName );       // TODO: accept '*' chars in commonName?
        CleanupStack::PopAndDestroy(commonNameSymbian);
    }
    return matches;
}

