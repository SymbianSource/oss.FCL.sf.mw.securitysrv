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

#include "UntrustedCertificateInfoBase.h"

const char *KHexNumberFormatSimple = "%X";
const char *KHexNumberFormatTwoDigitsWithLeadingZeroes = "%02X";
const int KCharsPerBlock = 2;
const char KBlockSeparator = ' ';


// ======== MEMBER FUNCTIONS ========

// ----------------------------------------------------------------------------
// UntrustedCertificateInfoBase::UntrustedCertificateInfoBase()
// ----------------------------------------------------------------------------
//
UntrustedCertificateInfoBase::UntrustedCertificateInfoBase() : QObject(),
    mSubjectName(), mIssuerName(), mFingerprint(), mSerialNumber(), mValidFrom(), mValidTo(),
    mFormat(UnknownCertificate), mDigestAlgorithm(Unknown), mAsymmetricAlgorithm(Unknown)
{
}

// ----------------------------------------------------------------------------
// UntrustedCertificateInfoBase::~UntrustedCertificateInfoBase()
// ----------------------------------------------------------------------------
//
UntrustedCertificateInfoBase::~UntrustedCertificateInfoBase()
{
}

// ----------------------------------------------------------------------------
// UntrustedCertificateInfoBase::subjectName()
// ----------------------------------------------------------------------------
//
const QString UntrustedCertificateInfoBase::subjectName() const
{
    return mSubjectName;
}

// ----------------------------------------------------------------------------
// UntrustedCertificateInfoBase::issuerName()
// ----------------------------------------------------------------------------
//
const QString UntrustedCertificateInfoBase::issuerName() const
{
    return mIssuerName;
}

// ----------------------------------------------------------------------------
// UntrustedCertificateInfoBase::fingerprint()
// ----------------------------------------------------------------------------
//
const QByteArray UntrustedCertificateInfoBase::fingerprint() const
{
    return mFingerprint;
}

// ----------------------------------------------------------------------------
// UntrustedCertificateInfoBase::formattedFingerprint()
// ----------------------------------------------------------------------------
//
const QString UntrustedCertificateInfoBase::formattedFingerprint() const
{
    QString formatted;
    QByteArray fp = fingerprint();

    QString number;
    int blockIndex = 0;
    int count = fp.count();
    for (int index = 0; index < count; ++index) {
        if (blockIndex == KCharsPerBlock) {
            formatted.append(KBlockSeparator);
            blockIndex = 0;
        }
        number.sprintf(KHexNumberFormatTwoDigitsWithLeadingZeroes,
            static_cast<unsigned char>(fp.at(index)));
        formatted.append(number);
        ++blockIndex;
    }

    return formatted;
}

// ----------------------------------------------------------------------------
// UntrustedCertificateInfoBase::serialNumber()
// ----------------------------------------------------------------------------
//
const QByteArray UntrustedCertificateInfoBase::serialNumber() const
{
    return mSerialNumber;
}

// ----------------------------------------------------------------------------
// UntrustedCertificateInfoBase::formattedSerialNumber()
// ----------------------------------------------------------------------------
//
const QString UntrustedCertificateInfoBase::formattedSerialNumber() const
{
    QString formatted;
    QByteArray sn = serialNumber();

    QString number;
    int count = sn.count();
    for (int index = 0; index < count; ++index) {
        number.sprintf(KHexNumberFormatSimple,
            static_cast<unsigned char>(sn.at(index)));
        formatted.append(number);
    }

    return formatted;
}

// ----------------------------------------------------------------------------
// UntrustedCertificateInfoBase::validFrom()
// ----------------------------------------------------------------------------
//
const QDateTime UntrustedCertificateInfoBase::validFrom() const
{
    return mValidFrom;
}

// ----------------------------------------------------------------------------
// UntrustedCertificateInfoBase::validTo()
// ----------------------------------------------------------------------------
//
const QDateTime UntrustedCertificateInfoBase::validTo() const
{
    return mValidTo;
}

// ----------------------------------------------------------------------------
// UntrustedCertificateInfoBase::format()
// ----------------------------------------------------------------------------
//
const QString UntrustedCertificateInfoBase::format() const
{
    QString format;
    switch (mFormat) {
        case X509Certificate:
            //: Type name for X509 certificates displayed in certificate details.
            // TODO: localised UI string
            format = tr("X509");
            break;
        default:
            //: Type name for unknown certificates displayed in certificate details.
            // TODO: localised UI string
            format = tr("Unknown");
            break;
    }
    return format;
}

// ----------------------------------------------------------------------------
// UntrustedCertificateInfoBase::digestAlgorithm()
// ----------------------------------------------------------------------------
//
const QString UntrustedCertificateInfoBase::digestAlgorithm() const
{
    return algorithmName(mDigestAlgorithm);
}

// ----------------------------------------------------------------------------
// UntrustedCertificateInfoBase::AsymmetricAlgorithm()
// ----------------------------------------------------------------------------
//
const QString UntrustedCertificateInfoBase::asymmetricAlgorithm() const
{
    return algorithmName(mAsymmetricAlgorithm);
}

// ----------------------------------------------------------------------------
// UntrustedCertificateInfoBase::AsymmetricAlgorithm()
// ----------------------------------------------------------------------------
//
const QString UntrustedCertificateInfoBase::combinedAlgorithmName() const
{
    if (mDigestAlgorithm != Unknown && mAsymmetricAlgorithm != Unknown) {
        //: Format to combine digest and asymmetric algorithm names.
        //: %1 is digest (signing) and %2 is asymmetric (public-key) algorithm.
        // TODO: localised UI string needed
        return tr("%1%2").arg(digestAlgorithm()).arg(asymmetricAlgorithm());
    }
    //: Algorithm name for unknown algorithm.
    // TODO: localised UI string needed
    return tr("Unknown");
}

// ----------------------------------------------------------------------------
// UntrustedCertificateInfoBase::isDateValid()
// ----------------------------------------------------------------------------
//
bool UntrustedCertificateInfoBase::isDateValid() const
{
    QDateTime current = QDateTime::currentDateTime();
    return (mValidFrom <= current && mValidTo >= current);
}

// ----------------------------------------------------------------------------
// UntrustedCertificateInfoBase::algorithmName()
// ----------------------------------------------------------------------------
//
const QString UntrustedCertificateInfoBase::algorithmName(Algorithm algorithm) const
{
    QString name;
    switch (algorithm) {
        case RSA:
            //: Certificate details algorithm name
            // TODO: localized UI string
            name = tr("RSA");
            break;
        case DSA:
            //: Certificate details algorithm name
            // TODO: localized UI string
            name = tr("DSA");
            break;
        case DH:
            //: Certificate details algorithm name
            // TODO: localized UI string
            name = tr("DH");
            break;
        case MD2:
            //: Certificate details algorithm name
            // TODO: localized UI string
            name = tr("MD2");
            break;
        case MD5:
            //: Certificate details algorithm name
            // TODO: localized UI string
            name = tr("MD5");
            break;
        case SHA1:
            //: Certificate details algorithm name
            // TODO: localized UI string
            name = tr("SHA1");
            break;
        case SHA224:
        case SHA256:
        case SHA384:
        case SHA512:
            //: Certificate details algorithm name
            // TODO: localized UI string
            name = tr("SHA2");
            break;
        case Unknown:
        default:
            //: Certificate details algorithm name
            // TODO: localized UI string
            name = tr("Unknown");
            break;
    }
    return name;
}

