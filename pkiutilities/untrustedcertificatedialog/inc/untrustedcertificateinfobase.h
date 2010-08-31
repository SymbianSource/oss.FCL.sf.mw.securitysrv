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

#ifndef UNTRUSTEDCERTIFICATEINFOBASE_H
#define UNTRUSTEDCERTIFICATEINFOBASE_H

#include <QObject>
#include <QDateTime>

/**
 * Certificate info class for secure connections (TLS) untrusted certificate dialog.
 * This is abstract class that is used in UI code to get displayable certificate
 * details. Concrete (possibly platform specific) classes implement the functionality.
 */
class UntrustedCertificateInfoBase : public QObject
{
    Q_OBJECT

public:     // definitions
    enum CertificateFormat {
        UnknownCertificate,
        X509Certificate,
        WTLSCertificate,
        X968Certificate
    };
    enum Algorithm {
        Unknown,
        RSA,
        DSA,
        DH,
        MD2,
        MD5,
        SHA1,
        SHA224,
        SHA256,
        SHA384,
        SHA512
    };

protected:  // constructor, available for derived classes only
    UntrustedCertificateInfoBase();

public:     // destructor
    virtual ~UntrustedCertificateInfoBase();

public:     // new functions
    virtual const QString subjectName() const;
    virtual const QString issuerName() const;
    virtual const QByteArray fingerprint() const;
    virtual const QByteArray serialNumber() const;
    virtual const QDateTime validFrom() const;
    virtual const QDateTime validTo() const;
    virtual const QString format() const;
    virtual const QString digestAlgorithm() const;
    virtual const QString asymmetricAlgorithm() const;
    virtual const QString combinedAlgorithmName() const;
    virtual const QString formattedFingerprint(const QByteArray &fingerprint) const;
    virtual const QString formattedSerialNumber(const QByteArray &serialNumber) const;
    virtual bool isDateValid() const;

public:     // new abstract functions
    virtual bool commonNameMatches(const QString &siteName) const = 0;
    virtual QString certificateDetails(const QString &siteName) const = 0;

private:    // new functions
    const QString algorithmName(Algorithm algorithm) const;

protected:  // data
    QString mSubjectName;
    QString mIssuerName;
    QByteArray mFingerprint;
    QByteArray mSerialNumber;
    QDateTime mValidFrom;
    QDateTime mValidTo;
    CertificateFormat mFormat;
    Algorithm mDigestAlgorithm;
    Algorithm mAsymmetricAlgorithm;
};

#endif // UNTRUSTEDCERTIFICATEINFOBASE_H
