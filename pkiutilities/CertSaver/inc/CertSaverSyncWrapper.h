/*
* Copyright (c) 2003-2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Header file of the CCertSaverSyncWrapper class
*
*/


#ifndef CERTSAVERSYNCWRAPPER_H
#define CERTSAVERSYNCWRAPPER_H

//  INCLUDES
#include <e32base.h>
#include <securitydefs.h>   // TCertificateFormat, TCertificateOwnerType
#include <ct/rmpointerarray.h> // RMPointerArray
#include <mpkcs12.h>
#include <mctkeystore.h>

typedef TBuf8<20> TKeyIdBuf;

// FORWARD DECLARATIONS
class CUnifiedCertStore;
class MCTWritableCertStore;
class CCTCertInfo;
class CCertAttributeFilter;
class CCertificate;
class CUnifiedKeyStore;

// CLASS DECLARATION

/**
*  Synchronous wrapper for asynchronous functions of Crypto Token fw.
*
*  @lib CertSaver.app
*  @since 2.0
*/
class CCertSaverSyncWrapper : public CActive
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CCertSaverSyncWrapper* NewL();

        /**
        * Destructor.
        */
        virtual ~CCertSaverSyncWrapper();

    public: // New functions

        /**
        * Initialise unified cert store
        * @since 2.0
        * @param aStore unified cert store
        * @return status code from CUnifiedCertStore
        */
        TInt InitializeCertStore( CUnifiedCertStore& aStore );

        /**
        * Initialise unified cert store
        * @since 2.0
        * @param aStore unified cert store
        * @return status code from CUnifiedCertStore
        */
        TInt InitializeKeyStoreL( RFs& aFs );

        /**
        * Add certificate to certman.
        * @since 2.0
        * @param aStore writable certstore interface
        * @param aLabel label of the certificate
        * @param aCertFormat format of the certificate
        * @param aCertType type of the certificate
        * @param aSubjectKeyId subject key ID
        * @param aIssuerKeyId issuer key ID
        * @param aCert certificate to be added
        * @return status code from MCTWritableCertStore
        */
        TInt AddCert( MCTWritableCertStore& aStore,
                      const TDesC& aLabel,
                      TCertificateFormat aCertFormat,
                      TCertificateOwnerType aCertType,
                      TKeyIdentifier* aSubjectKeyId,
                      TKeyIdentifier* aIssuerKeyId,
                      TPtrC8 aCert );

        /**
        * List certificates matching to filter.
        * @since 2.0
        * @param aStore unified cert store
        * @param aEntries found certificates
        * @param aFilter filter defining which certs are listed
        * @return status code from CUnifiedCertStore
        */
        TInt ListCerts( CUnifiedCertStore& aStore,
                        RMPointerArray<CCTCertInfo>& aEntries,
                        CCertAttributeFilter& aFilter );

        /**
        * Set applicability for certificate.
        * @since 2.0
        * @param aStore writable certstore interface
        * @param aCertInfo certificate
        * @param aApplications UIDs of applicable applications
        * @return status code from MCTWritableCertStore
        */
        TInt SetApplicability( MCTWritableCertStore& aStore,
            const CCTCertInfo& aCertInfo, RArray<TUid>& aApplications );

        /**
        * Set trust for certificate.
        * @since 2.0
        * @param aStore writable certstore interface
        * @param aCertInfo certificate
        * @param aTrust trusted or not
        * @return status code from MCTWritableCertStore
        */
        TInt SetTrust( MCTWritableCertStore& aStore,
            const CCTCertInfo& aCertInfo, TBool aTrust );

        /**
        * Retrieve certificate
        * @since 2.0
        * @param aStore writable certstore interface
        * @param aCertInfo certificate
        * @param aCert IN NULL
        *              OUT Constructed object.
        *              Ownership moves to caller.
        * @return status code from MCTWritableCertStore
        */
        TInt Retrieve( CUnifiedCertStore& aStore,
            const CCTCertInfo& aCertInfo, CCertificate*& aCert );

        /**
        * Add key to the sw keystore
        * @since 2.8
        * @param aKeyStoreIndex Index of the keystore
        * @param aKeyData PKCS#8 encoded keypair
        * @param aUsage PKCS#15 usage flags of the key
        * @param aLabel Label of the key
        * @param aAccessType Access type of the key
        * @param aStartDate Start date of the validation period
        * @param aEndDate End date of the validation period
        * @param aKeyInfoOut Key info of the added key
        * @return status code from UnifiedKeyStore
        */
        TInt AddKey( TInt aKeyStoreIndex,
                     const TDesC8& aKeyData,
                     TKeyUsagePKCS15 aUsage,
                     const TDesC& aLabel,
                     TInt aAccessType,
                     TTime aStartDate,
                     TTime aEndDate,
                     CCTKeyInfo*& aKeyInfoOut );

        /**
        * List keys matching to filter.
        * @since 3.0
        * @param aEntries Found Keys
        * @param aFilter Filter defining which keys are listed
        * @return status code from UnifiedKeystore
        */
        TInt ListKeys( RMPointerArray<CCTKeyInfo>& aEntries,
                       TCTKeyAttributeFilter& aFilter );

        /**
        * Sets the security policy for key management.
        *
        * Specifies which processes are allowed to perform management operations on
        * the key.
        *
        * @param aHandle  The handle of the key
        * @param aPolicy  The new security policy.
        *
        * @leave KErrPermissionDenied If the caller does not have WriteUserData capability,
        *               or is not the owner of the key.
        * @leave KErrNotFound     If the key the handle referes to does not exist.
        */
        TInt SetManagementPolicy( TCTTokenObjectHandle aHandle,
                  const TSecurityPolicy& aPolicy );

        /**
        * Sets the security policy for key use.
        *
        * Specifies which processes are allowed to use the key for cryptographic
        * operations.
        *
        * @param aHandle  The handle of the key
        * @param aPolicy  The new security policy.
        *
        * @leave KErrPermissionDenied If the caller does not have WriteUserData capability,
        *               or is not the owner of the key.
        * @leave KErrNotFound     If the key the handle referes to does not exist.
        */
        TInt SetUsePolicy( TCTTokenObjectHandle aHandle,
               const TSecurityPolicy& aPolicy );

        /**
        * Get reference to UnifiedKeyStore.
        * @since 3.0
        * @return Reference to UnifiedKeyStore;
        */
        CUnifiedKeyStore& UnifiedKeyStore();

    public: // Functions from base classes


    protected:  // New functions


    protected:  // Functions from base classes

        /**
        * From CActive
        */
        void DoCancel();
        void RunL();

    private:

        /**
        * C++ default constructor.
        */
        CCertSaverSyncWrapper();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        /**
        * Wait until asynchronous call is completed and return status
        */
        TInt SetActiveAndWait();

    public:     // Data

    protected:  // Data

    private:    // Data
        CActiveSchedulerWait  iWait;
        CUnifiedKeyStore*     iUnifiedKeyStore;

    };

#endif      // CERTSAVERSYNCWRAPPER_H

// End of File
