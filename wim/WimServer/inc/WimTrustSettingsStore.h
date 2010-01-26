/*
* Copyright (c) 2003 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Implements WIM TrustSettingsStore for WIM certificates
*
*/



#ifndef CWIMTRUSTSETTINGSSTORE_H
#define CWIMTRUSTSETTINGSSTORE_H

//  INCLUDES
#include <e32base.h>
#include <cctcertinfo.h>
#include <f32file.h>
#include <d32dbms.h>
#include "WimDefs.h"    // TTSSCertLocation
#include <data_caging_path_literals.hrh>

// CONSTANTS

// The location of wim trust setting storage
// The path is taken from data_caging_path_literals
_LIT( KWimTrustSettingsDBFile, "WimTrustSettingsStore.dat" );

// Object identifier (OID) for server authentication
_LIT( KIpKpServerAuth, "2.5.29.37.1" ); 
// Object identifier (OID) for JavaMidlet certificates
_LIT( KIpKpJavaMidlet, "1.3.6.1.4.1.42.2.110.2.2.2.1" ); 
// Object identifier (OID) for code signer certificates
_LIT( KWimCodeSigningOID, "1.3.6.1.5.5.7.3.3" );

// Default value for trusted flag
const TBool KDefaultTrustFlag = ETrue;
// Maximum length of SQL query clause
const TInt KMaxSQLLength = 100;
// Trusting applications
const TInt KTrustUidWapBearer     = 268479059;
const TInt KTrustUidAppController = 268452523;
const TInt KTrustUidInternet      = 268441661;
const TInt KTrustUidJavaMidlet    = 270506792;

// FORWARD DECLARATION

class CWimCertUtil;
class CWimCertInfo;

// CLASS DECLARATION

/**
*  This class implements services for trust settings.
*  Class includes functions for creating DB, fetching, updating and 
*  deleting trust settings.
*
*  @lib WimPlugin.lib
*  @since Series60 2.1
*/
class CWimTrustSettingsStore : public CActive
    {
    public:  // Constructor and destructor
        
        /**
        * Two-phased constructor.
        */
        static CWimTrustSettingsStore* NewL();
        
        /**
        * Destructor.
        */
        virtual ~CWimTrustSettingsStore();

    public: // New functions
        
        /**
        * Fetches trust settings for given certificate. 
        * Acts like an asynchronous for caller but not activates itself.
        * @param  aCert (IN) Certificate
        * @param  aTrusted  (OUT) Is certificate trusted
        * @param  aApplications  (OUT) Array of applications supported 
        *         by certificate
        * @param  aStatus (IN/OUT) Request Status of asynchronous call
        * @return void
        */
        void GetTrustSettings( const CWimCertInfo& aCert,
                               TBool& aTrusted,
                               RArray<TUid>& aApplications, 
                               TRequestStatus& aStatus );

        /**
        * Set applicability for given certificate. Calls DoSetApplicabilityL,
        * which handles actual operation. New certificate entry is set if
        * one not found from database (trust flag is set to EFalse).
        * Acts like an asynchronous for caller but not activates itself.
        * @param aCert (IN) Certificate
        * @param aApplications (IN) Array of applications supported by 
        *        certificate
        * @param aStatus Request (IN/OUT) Status of asynchronous call.
        * @return void
        */
        void SetApplicability( const CWimCertInfo& aCert,
                               const RArray<TUid>& aApplications,
                               TRequestStatus& aStatus );

        /**
        * Set trust flag for given certificate. Calls DoSetTrustL,
        * which handles actual operation. New certificate entry is set if
        * one not found from database.
        * Acts like an asynchronous for caller but not activates itself.
        * @param aCert (IN) Certificate
        * @param aTrusted (IN) Is certificate trusted
        * @param aStatus (IN/OUT) Request Status of asynchronous call.
        * @return void
        */
        void SetTrust( const CWimCertInfo& aCert,
                       TBool aTrusted,
                       TRequestStatus& aStatus );

        /**
        * Set default trust settings for given certificate. If certificate not
        * found from database new entry is inserted.
        * Acts like an asynchronous for caller but not activate itself.
        * @param aCert (IN) Certificate
        * @param aAddApps (IN) Are default applications inserted 
        *        (ETrue means yes)
        * @param aStatus (IN/OUT) Request Status of asynchronous call
        * @return void
        */
        void SetDefaultTrustSettings( const CWimCertInfo& aCert,
                                      TBool aAddApps,
                                      TRequestStatus& aStatus );

        /**
        * Remove trust settings of given certificate. If certificate
        * is not found, return with status.Int() = KErrNotFound
        * Acts like an asynchronous for caller but not activates itself.
        * @param aCert (IN) Certificate
        * @param aStatus (IN/OUT) Request Status of asynchronous call
        * @return void
        */
        void RemoveTrustSettings( const CWimCertInfo& aCert,
                                  TRequestStatus& aStatus );

        /**
        * Delete TrustSettingsStore instance and release all resources
        * @return void
        */
        void CloseD();

        /**
        * Cancel any issued asynchronous call
        * @return void
        */
        void CancelDoing();

    private:

        /**
        * Default constructor.
        */
        CWimTrustSettingsStore();

        /**
        * 2nd phase constructor
        */
        void ConstructL();

        /**
        * Fetches trust settings for given certificate
        * @param aCert (IN) Certificate
        * @param aTrusted (OUT) Is certificate trusted
        * @param aApplications (OUT) Array of applications supported 
        *        by certificate
        * @return void
        */
        void DoGetTrustSettingsL( const CWimCertInfo& aCert,
                                  TBool& aTrusted,
                                  RArray<TUid>& aApplications );

        /**
        * Set applicability for given certificate. If certificate is not 
        * allready in database insert new entry.
        * @param aCert (IN) Certificate
        * @param aTrusted (IN) Is certificate trusted
        * @param aApplications (IN) Array of applications supported 
        *        by certificate
        * @return void
        */
        void DoSetApplicabilityL( const CWimCertInfo& aCert,
                                  const RArray<TUid>& aApplications );

        /**
        * Set trust flag for given certificate. If certificate is not 
        * allready in database insert new entry.
        * @param aCert (IN) Certificate
        * @param aTrusted (IN) Is certificate trusted
        * @return void
        */
        void DoSetTrustL( const CWimCertInfo& aCert,
                          TBool aTrusted );

        /**
        * Set default trust settings for given certificate
        * @param aCert (IN) Certificate
        * @param aAddApps (IN) Are default applications inserted 
        *        (ETrue means yes)
        * @return void
        */
        void DoSetDefaultTrustSettingsL( const CWimCertInfo& aCert,
                                         TBool aAddApps );

        /**
        * Remove trust settings of given certificate. If certificate not 
        * found, leave with error code KErrNotFound
        * @param aCert (IN) Certificate
        * @return void
        */
        void DoRemoveTrustSettingsL( const CWimCertInfo& aCert );

        /**
        * Get database ID for given certificate. 
        * Returns certificate ID = KErrNotFound if matching ID not found.
        * @param aCert (IN) Certificate
        * @param aCertID (OUT) Certificate's database ID
        * @return void
        */
        void GetCertificateIDL( const CWimCertInfo& aCert,
                                TInt& aCertificateID );

        /**
        * Insert certificate to Certificates table
        * @param aCert (IN) Certificate
        * @param aTrusted (IN) Is certificate trusted or not
        * @return void
        */
        void InsertCertificateL( const CWimCertInfo& aCert, 
                                 TBool aTrusted );

        
        /**
        * Fetch certificates trusted flag
        * @param aCertificateID (IN) Certificate's database ID
        * @param aTrusted (OUT) Is certificate trusted or not
        * @return void
        */
        void GetTrustedL( TInt aCertificateID,  
                          TBool& aTrusted );

        /**
        * Set trusted flag for certificate
        * @param aCertificateID (IN) Certificate's database ID
        * @param aTrusted (IN) Is certificate trusted or not
        * @return void
        */
        void SetTrustedL( TInt aCertificateID,  
                          TBool aTrusted );
        /**
        * Remove certificate from Certificates table
        * @param aCertificateID (IN) Certificate's database ID
        * @return void
        */
        void RemoveCertificateL( TInt aCertificateID );

        /**
        * Remove all applications of given certificate
        * @param aCertificateID (IN) Certificate
        * @return void
        */
        void RemoveApplicationsL( TInt aCertificateID );

        /**
        * Get all applications of given certificate
        * @param aCertificateID (IN) Certificate's database ID
        * @param aApplications (OUT) Array of applications supported 
        *        by certificate
        * @return void
        */
        void GetApplicationsL( TInt aCertificateID, 
                               RArray<TUid>& aApplications );

        /**
        * Set applications for given certificate
        * @param aCertificateID (IN) Certificate database ID
        * @param aApplications (IN) Array of applications supported 
        *        by certificate
        * @return void
        */
        void InsertApplicationsL( TInt aCertificateID, 
                                  const RArray<TUid>& aApplications );
        
        /**
        * Decide default applications for given certificate.
        * Default applications are decided regarding the certificate's:
        *   - format
        *   - extented usage data
        *   - trusted usage data
        * @param aCert (IN) Certificate
        * @param aApplications (OUT) List of applications returned
        * @return void
        */
        void DefaultApplicationsL( const CWimCertInfo& aCert,
                                   RArray<TUid>& aApplications );

        /**
        * Decide default applications by OID's. 
        * @param aCert (IN) Certificate
        * @param aApplications (OUT) List of applications returned
        * @return void
        */
        void DefaultAppsByOIDsL( const CWimCertInfo& aCert,
                                 RArray<TUid>& aApplications );

        /**
        * Create database. Leaves if error occurs.
        * @return void
        */
        void CreateDBL() const;

        /**
        * Insert default appUid selection rules. Leaves if error occurs.
        * @return void
        */
        void InsertDefaultAppRulesL() const;

        /**
        * Extracts certificate extensions. Leaves if error occurs.
        * @return void
        */
        void ExtractExtensionL();

        /**
        * Makes a SHA-1 hash of certificate label
        * @param aString (IN) String to be hashed
        * @return 8 bit string of label hash
        */
        HBufC8* GetHashL( TDesC8& aString );

    private: // From base class CActive 

        /**
        * Handle asyncronous response
        * @return void
        */
        void RunL();

        /**
        * Handle asyncronous call cancel
        * @return void
        */
        void DoCancel();

        /**
        * Handles RunL function leaves
        * @param aError  Leaving code.
        * @return TInt
        */
        TInt RunError( TInt aError );

    private:    // Data

        enum TPhase
            {
            EIdle,
            ERetrieve,
            ECheckRestore,
            ECheckInitializeCertStore,
            ECheckRetrieve
            };

        // The state of this active object
        TPhase iPhase;

        // Used for saving caller status. Not owned
        TRequestStatus* iOriginalRequestStatus;

        //Database session for WimTrustSettingsStore

        RFs iDBSession;
         
        //Database for WimTrustSettingsStore
        RDbNamedDatabase iDatabase;

        //Flag to tell if compact operation for dbhas to be done in destructor
        TBool            iCompact;

        // For file session
        RFs iFs;

        // Pointer to CertUtil.Owned
        CWimCertUtil* iWimCertUtil;

        // Pointer to current wim specific certificate info
        const CWimCertInfo* iWimCert;

        // Pointer to current Symbian specific certificate info
        const CCTCertInfo* iCert;

        // Boolean indicating if default applications are added or not
        TBool iAddApps;

        // Pointer to whole certificate data buffer
        HBufC8* iEncodedCertBuf;

        // Pointer to certificate data buffer
        TPtr8* iEncodedCertPtr;

        // Temporal array for certificate infos. 
        // This array is used to fetch certificate infos from WimClient.
        // Owned.
        RPointerArray<CWimCertInfo> iCertInfos;
    };

#endif      // CWIMTRUSTSETTINGSSTORE_H
            
// End of File
