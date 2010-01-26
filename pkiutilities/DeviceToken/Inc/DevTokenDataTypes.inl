/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   The inl file of DevTokenDataTypes
*
*/


// Data types used by the device tokens server

inline CDevTokenKeyInfo::CDevTokenKeyInfo() :
    CKeyInfoBase()
    {
    }

inline CDevTokenKeyInfo::~CDevTokenKeyInfo()
    {
    }

inline void CDevTokenKeyInfo::CleanupPushL()
    {
    CleanupStack::PushL(this);
    }

inline TInt CDevTokenKeyInfo::Handle() const
    {
    return iHandle;
    }

inline void CDevTokenKeyInfo::SetAccessType(TInt aAccessType)
    {
    iAccessType = aAccessType;
    }

inline const TInteger& CDevTokenDHParams::N() const
    {
    return iN;
    }

inline const TInteger& CDevTokenDHParams::G() const
    {
    return iG;
    }

inline const TDesC& CDevTokenCertInfo::Label() const
    {
    return iLabel;
    }

inline TInt CDevTokenCertInfo::Reference() const
    {
    return iCertificateId;
    }

inline const TKeyIdentifier& CDevTokenCertInfo::SubjectKeyId() const
    {
    return iSubjectKeyId;
    }

inline const TKeyIdentifier& CDevTokenCertInfo::IssuerKeyId() const
    {
    return iIssuerKeyId;
    }

inline TCertificateFormat CDevTokenCertInfo::CertificateFormat() const
    {
    return iFormat;
    }

inline TCertificateOwnerType CDevTokenCertInfo::CertificateOwnerType() const
    {
    return iCertificateOwnerType;
    }

inline TInt CDevTokenCertInfo::Size() const
    {
    return iSize;
    }

inline TInt CDevTokenCertInfo::CertificateId() const
    {
    return iCertificateId;
    }

inline TBool CDevTokenCertInfo::IsDeletable() const
    {
    return iDeletable;
    }


//EOF

