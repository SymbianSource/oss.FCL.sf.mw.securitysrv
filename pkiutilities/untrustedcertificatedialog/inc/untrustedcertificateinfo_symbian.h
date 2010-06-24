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

#ifndef UNTRUSTEDCERTIFICATEINFOSYMBIAN_H
#define UNTRUSTEDCERTIFICATEINFOSYMBIAN_H

#include "untrustedcertificateinfobase.h"

class CX509Certificate;


/**
 * Certificate info class for secure connections (TLS) untrusted certificate dialog.
 * Symbian-specific concrete implementeation for the certificate info class.
 */
class UntrustedCertificateInfoSymbian : public UntrustedCertificateInfoBase
{
public:     // constructor and destructor
    UntrustedCertificateInfoSymbian(const QByteArray &aEncodedCert);
    ~UntrustedCertificateInfoSymbian();

public:     // from UntrustedCertificateInfoBase
    bool commonNameMatches(const QString &siteName) const;
    QString certificateDetails(const QString &siteName) const;

private:    // new functions
    void ConstructL(const QByteArray &aEncodedCert);
    bool CommonNameMatchesL(const QString &siteName) const;
    QByteArray Md5FingerprintL( const TDesC8& aEncodedCert ) const;

private:    // data
    CX509Certificate* mCert;
    QByteArray mMd5Fingerprint;
};

#endif // UNTRUSTEDCERTIFICATEINFOSYMBIAN_H
