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
* Description:   The header file of DevTokenDataTypes
*
*/



#ifndef __DEVTOKENDATATYPES_H__
#define __DEVTOKENDATATYPES_H__

#include <mctkeystore.h>
#include <cctcertinfo.h>
#include <bigint.h>

/**
 * Server-side key info.
 * 
 * This is the server-side counterpart to the CCTKeyInfo class, containing all
 * the key attributes.
 
 * @lib DevTokenShared.dll
 * @since S60 v3.2
 */
class CDevTokenKeyInfo : public CKeyInfoBase
    {
    public:
        // Creates a new CKeyInfo from a stream.
        IMPORT_C static CDevTokenKeyInfo* NewL(RReadStream& aStream);
        
        // Make destructor public again.
        inline ~CDevTokenKeyInfo();
        
        // Push object onto the cleanup stack (pointer won't convert to CBase* due
        // to protected inheritance.
        inline void CleanupPushL();

    public:
        
        // Get the handle of the key.
        inline TInt Handle() const;
        
        // Set the key access type.
        inline void SetAccessType(TInt aAccessType);
        
        // Set the security policy for key use operations.
        IMPORT_C void SetUsePolicy(const TSecurityPolicy& aPolicy);
        
        // Set the security policy for key management operations.
        IMPORT_C void SetManagementPolicy(const TSecurityPolicy& aPolicy);
        
        // Sets the set of DER encoded PKCS8 attributes.
        IMPORT_C void SetPKCS8AttributeSet(HBufC8* aPKCS8AttributeSet);

    private:
    
        inline CDevTokenKeyInfo();    
    };


/**
 * An internal abstraction for Diffie-Hellman parmeters.
 *
 * @lib DevTokenShared.dll
 * @since S60 v3.2
 */
class CDevTokenDHParams : public CBase
    {
    public:
    
        IMPORT_C static CDevTokenDHParams* NewL(const TInteger& aN, const TInteger& aG);
    
        IMPORT_C ~CDevTokenDHParams();
    
        inline const TInteger& N() const;
    
        inline const TInteger& G() const;
    
        //Return N and release ownership
    
        IMPORT_C RInteger TakeN();
    
        //Return G and release ownership
    
        IMPORT_C RInteger TakeG();
        
    private:
    
        IMPORT_C CDevTokenDHParams(RInteger aN, RInteger aG);
    
        RInteger iN;
    
        RInteger iG;
    };


/** A common interface for arrays of key infos.  
 *
 * @lib DevTokenShared.dll
 * @since S60 v3.2
 */
class MDevTokenKeyInfoArray
    {
    public:
    
        virtual TInt Count() = 0;
    
        virtual CCTKeyInfo* operator[](TInt aIndex) = 0;
    
        virtual TInt Append(CCTKeyInfo* aInfo) = 0;
    
        virtual void Close() = 0;
    };


/**
 * Information sent back to client when a key has been created or imported.
 * 
 * @lib DevTokenShared.dll
 * @since S60 v3.2
 */
struct TDevTokenKeyUpdate
    {
    
    TInt iReference;
    
    TKeyIdentifier iId;
    
    TUint iSize;
    
    CKeyInfoBase::EKeyAlgorithm iAlgorithm;
    };


/**
 * Server-side cert info.
 * 
 * This is the server-side counterpart to the CCTCertInfo class, containing all
 * the certificate attributes.
 * We don't support the issuer hash, as it is not
 * stored.
 *
 * @lib DevTokenShared.dll
 * @since S60 v3.2
 */

class CDevTokenCertInfo : public CBase, public MCertInfo
    {
    public:
        
        // Copy a CCertInfo.
        IMPORT_C static CDevTokenCertInfo* NewL(const CDevTokenCertInfo& aOther);
        
        // Copy a CCertInfo. 
        IMPORT_C static CDevTokenCertInfo* NewLC(const CDevTokenCertInfo& aOther);
        
        // Makes a new CCTCertInfo from its constituent parts.
        IMPORT_C static CDevTokenCertInfo* NewL(const TDesC& aLabel,
                                                TCertificateFormat aFormat,
                                                TCertificateOwnerType aCertificateOwnerType,
                                                TInt aSize,
                                                const TKeyIdentifier* aSubjectKeyId,
                                                const TKeyIdentifier* aIssuerKeyId,
                                                TInt aCertificateId);
        
        IMPORT_C static CDevTokenCertInfo* NewL(const TDesC& aLabel,
                                                TCertificateFormat aFormat,
                                                TCertificateOwnerType aCertificateOwnerType,
                                                TInt aSize,
                                                const TKeyIdentifier* aSubjectKeyId,
                                                const TKeyIdentifier* aIssuerKeyId,
                                                TInt aCertificateId,
                                                const TBool aDeletable);
        
        // Makes a new CCTCertInfo from its constituent parts. 
        IMPORT_C static CDevTokenCertInfo* NewLC(const TDesC& aLabel,
                                                 TCertificateFormat aFormat,
                                                 TCertificateOwnerType aCertificateOwnerType,
                                                 TInt aSize,
                                                 const TKeyIdentifier* aSubjectKeyId,
                                                 const TKeyIdentifier* aIssuerKeyId,
                                                 TInt aCertificateId);
        
        IMPORT_C static CDevTokenCertInfo* NewLC(const TDesC& aLabel,
                                                 TCertificateFormat aFormat,
                                                 TCertificateOwnerType aCertificateOwnerType,
                                                 TInt aSize,
                                                 const TKeyIdentifier* aSubjectKeyId,
                                                 const TKeyIdentifier* aIssuerKeyId,
                                                 TInt aCertificateId,
                                                 const TBool aDeletable);
        
        // Reads a CDevTokenCertInfo from a stream. 
        IMPORT_C static CDevTokenCertInfo* NewL(RReadStream& aStream);
        
        // Reads a CCertInfo from a stream. 
        IMPORT_C static CDevTokenCertInfo* NewLC(RReadStream& aStream);
        
        // Accessors
        inline const TDesC& Label() const;
        
        inline TInt Reference() const;
        
        inline const TKeyIdentifier& SubjectKeyId() const;
        
        inline const TKeyIdentifier& IssuerKeyId() const;
        
        inline TCertificateFormat CertificateFormat() const;
        
        inline TCertificateOwnerType CertificateOwnerType() const;
        
        inline TInt Size() const;
        
        inline TInt CertificateId() const;
        
        inline TBool IsDeletable() const;

    private:
        
        // Default constructor. 
        CDevTokenCertInfo();    
        
        // Copy constructor. 
        CDevTokenCertInfo(const CDevTokenCertInfo& aOther);
        
        CDevTokenCertInfo(const TDesC& aLabel,
                          TCertificateFormat aFormat,
                          TCertificateOwnerType aCertificateOwnerType, 
                          TInt aSize,
                          const TKeyIdentifier* aSubjectKeyId,
                          const TKeyIdentifier* aIssuerKeyId,
                          TInt aCertificateId,
                          const TBool aDeletable = ETrue);  
    };


struct TDevTokenAddCertDataStruct
    {
    TCertLabel iLabel;
    TCertificateFormat iFormat;
    TCertificateOwnerType iCertificateOwnerType;
    TKeyIdentifier iSubjectKeyId;
    TKeyIdentifier iIssuerKeyId;
    TBool iDeletable;
    };

#include "DevTokenDataTypes.inl"

#endif //__DEVTOKENDATATYPES_H__
