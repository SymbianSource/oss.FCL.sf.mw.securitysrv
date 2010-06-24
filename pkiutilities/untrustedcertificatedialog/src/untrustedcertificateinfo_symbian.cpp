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
#include <X509CertNameParser.h>                 // X509CertNameParser
#include <hash.h>                               // CMD5


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
void convertDateTime(const TTime& aFromTime, QDateTime& aToDateTime)
{
    const TDateTime &symbianDateTime = aFromTime.DateTime();

    QDate date(symbianDateTime.Year(), symbianDateTime.Month()+1, symbianDateTime.Day()+1);
    QTime time(symbianDateTime.Hour(), symbianDateTime.Minute(), symbianDateTime.Second());
    aToDateTime.setDate(date);
    aToDateTime.setTime(time);
}


// ======== MEMBER FUNCTIONS ========

// ----------------------------------------------------------------------------
// UntrustedCertificateInfoSymbian::UntrustedCertificateInfoSymbian()
// ----------------------------------------------------------------------------
//
UntrustedCertificateInfoSymbian::UntrustedCertificateInfoSymbian(
    const QByteArray &aEncodedCert) : UntrustedCertificateInfoBase(),
    mCert(0), mMd5Fingerprint()
{
    QT_TRAP_THROWING(ConstructL(aEncodedCert));
}

// ----------------------------------------------------------------------------
// UntrustedCertificateInfoSymbian::~UntrustedCertificateInfoSymbian()
// ----------------------------------------------------------------------------
//
UntrustedCertificateInfoSymbian::~UntrustedCertificateInfoSymbian()
{
    delete mCert;
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
// UntrustedCertificateInfoSymbian::certificateDetails()
// ----------------------------------------------------------------------------
//
QString UntrustedCertificateInfoSymbian::certificateDetails(const QString &siteName) const
{
    // TODO: localized UI string needed
    QString details = tr("Service:\n%1\n\nIssuer:\n%2\n\nSubject:\n%3\n\n"
        "Valid from:\n%4\n\nValid until:\n%5\n\nCertificate format:\n%6\n\n"
        "Algorithm:\n%7\n\nSerial number:\n%8\n\n"
        "Fingerprint (SHA1):\n%9\n\nFingerprint (MD5):\n%10")
        .arg(siteName)                                  // %1
        .arg(issuerName())                              // %2
        .arg(subjectName())                             // %3
        .arg(validFrom().toString())                    // %4
        .arg(validTo().toString())                      // %5
        .arg(format())                                  // %6
        .arg(combinedAlgorithmName())                   // %7
        .arg(formattedSerialNumber(serialNumber()))     // %8
        .arg(formattedFingerprint(fingerprint()))       // %9
        .arg(formattedFingerprint(mMd5Fingerprint));    // %10
    return details;
}

// ----------------------------------------------------------------------------
// UntrustedCertificateInfoSymbian::ConstructL()
// ----------------------------------------------------------------------------
//
void UntrustedCertificateInfoSymbian::ConstructL(const QByteArray &aEncodedCert)
{
    TPtrC8 encodedCert( reinterpret_cast<const TText8*>( aEncodedCert.constData() ),
        aEncodedCert.length() );

    ASSERT( mCert == 0 );
    mCert = CX509Certificate::NewL( encodedCert );

    HBufC16* subjectBuf = NULL;
    X509CertNameParser::SubjectFullNameL( *mCert, subjectBuf );
    CleanupStack::PushL( subjectBuf );
    QT_TRYCATCH_LEAVING( mSubjectName =
        QString::fromUtf16(subjectBuf->Ptr(), subjectBuf->Length()) );
    CleanupStack::PopAndDestroy( subjectBuf );

    HBufC16* issuerBuf = NULL;
    X509CertNameParser::IssuerFullNameL( *mCert, issuerBuf );
    CleanupStack::PushL( issuerBuf );
    QT_TRYCATCH_LEAVING( mIssuerName =
        QString::fromUtf16(issuerBuf->Ptr(), issuerBuf->Length()));
    CleanupStack::PopAndDestroy( issuerBuf );

    TPtrC8 fingerprint = mCert->Fingerprint();
    QT_TRYCATCH_LEAVING( mFingerprint = QByteArray::fromRawData(
        reinterpret_cast<const char*>(fingerprint.Ptr()), fingerprint.Length()) );

    mMd5Fingerprint = Md5FingerprintL( mCert->Encoding() );

    TPtrC8 serialNumber = mCert->SerialNumber();
    QT_TRYCATCH_LEAVING( mSerialNumber = QByteArray::fromRawData(
        reinterpret_cast<const char*>(serialNumber.Ptr()), serialNumber.Length()) );

    const CValidityPeriod& validityPeriod = mCert->ValidityPeriod();
    convertDateTime(validityPeriod.Start(), mValidFrom);
    convertDateTime(validityPeriod.Finish(), mValidTo);

    mFormat = X509Certificate;

    const CSigningAlgorithmIdentifier& alg = mCert->SigningAlgorithm();
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
    const CX500DistinguishedName& distinguishedName = mCert->SubjectName();
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

// ----------------------------------------------------------------------------
// UntrustedCertificateInfoSymbian::Md5FingerprintL()
// ----------------------------------------------------------------------------
//
QByteArray UntrustedCertificateInfoSymbian::Md5FingerprintL( const TDesC8& aEncodedCert ) const
{
    CMD5* md5 = CMD5::NewL();
    CleanupStack::PushL( md5 );

    const TPtrC8 fingerprintSymbian = md5->Hash( aEncodedCert );
    QByteArray fingerprint( reinterpret_cast<const char*>( fingerprintSymbian.Ptr() ),
        fingerprintSymbian.Length() );

    CleanupStack::PopAndDestroy( md5 );
    return fingerprint;
}

