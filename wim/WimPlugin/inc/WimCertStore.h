/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  An implementation of certificate store interface
*
*/


#ifndef CWIMCERTSTORE_H
#define CWIMCERTSTORE_H

//  INCLUDES

#include "WimCertStoreMappings.h"
#include <mctkeystore.h>
#include <mctwritablecertstore.h>
#include <unifiedkeystore.h>

// FORWARD DECLARATION

class CWimCertConverter;
class CWimCertInfo;
class CWimTrustSettingsAPI;
class CWimToken;


// CLASS DECLARATION

/**
*  Represents read only and writable certificate store.
*
*  @lib   WimPlugin
*  @since Series60 2.1
*/

class CWimCertStore : public CActive, public MCTWritableCertStore   
    {
    public: 

        /**
        * Two-phased constructor
        * @param  aToken (IN) A refernece to curren token
        * @return An instance of this class
        */
        static CWimCertStore* NewL( CWimToken& aToken );

        /**
        * Destructor.
        */
        virtual ~CWimCertStore();

    public: // A function from base class MCTTokenInterface
        
        /**
        * Returns a reference to current token.
        * @return A reference to current token.
        */
        MCTToken& Token();

    public: // Functions from base class MCertStore

        /**
        * Lists certificates from the store. 
        * The caller of this function owns all its parameters.
        * @param  aCerts (OUT) An array into which the returned certificates 
        *         are placed.
        * @param  aFilter (IN) A filter to select which certificates should 
        *         be included.
        * @param  aStatus (IN/OUT) A Request status that will be completed when
        *         the operation completes.
        *         KErrNone, if no errors detected. Note: if certificates not
        *         found, it is also KErrNone
        *         KErrCancel, if call is canceled
        *         KErrHardwareNotAvailable, if Wim card suddenly removed
        *         Any other system wide error code (e.g. KErrNoMemory)
        * @return void
        */
        void List( RMPointerArray<CCTCertInfo>& aCerts, 
                   const CCertAttributeFilter& aFilter,
                   TRequestStatus& aStatus );
    
        /**
        * Cancels ongoing list operation.
        * @return void
        */
        void CancelList();

        /**
        * Gets a certificate info according to given handle. 
        * The caller of this function owns all its parameters.
        * @param  aCertInfo (OUT) The returned certificate info.
        * @param  aHandle   (IN) The handle of the certificate info to return.
        * @param  aStatus   (IN/OUT) A request status that is completed when 
        *         the operation has finished.
        *         KErrNone, if no errors detected. 
        *         KErrNotFound, if certificate with given handle not found
        *         KErrHardwareNotAvailable, if Wim card suddenly removed
        *         Any other system wide error code (e.g. KErrNoMemory)
        * @return void
        */
        void GetCert( CCTCertInfo*& aCertInfo, 
                      const TCTTokenObjectHandle& aHandle, 
                      TRequestStatus& aStatus );

        /**
        * Cancels an ongoing GetCert operation. No influence.
        * @return void
        */
        void CancelGetCert();

        /**
        * Queries the applications that given certificate supports.
        * The caller of this function owns all its parameters.
        * @param  aCertInfo (IN) The certificate to return applications for.
        * @param  aApplications (OUT) An array where applications are added in.
        * @param  aStatus       A request status that is completed when the 
        *         operation has finished.
        *         KErrNone, if no errors detected
        *         KErrNotFound, if aCertInfo not found
        *         KErrHardwareNotAvailable, if Wim card suddenly removed
        *         Any other system wide error code (e.g. KErrNoMemory)
        * @return void
        */
        void Applications( const CCTCertInfo& aCertInfo, 
                           RArray<TUid>& aApplications,
                           TRequestStatus& aStatus );

        /**
        * Cancels an ongoing Applications operation. No influence.
        * @return void
        */
        void CancelApplications();

        /**
        * Indicates if a particular certificate is applicable to a particular 
        * application. 
        * The caller of this function owns all its parameters.
        * @param  aCertInfo (IN) The certificate in question.
        * @param  aApplication (IN) The application.
        * @param  aIsApplicable (OUT) Set to ETrue or EFalse by the function 
        *         to return the result.
        * @param  aStatus (IN/OUT) A request status that is completed when the 
        *         operation has finished.
        *         KErrNone, if no errors detected.
        *         KErrNotFound, if aCertInfo not found
        *         KErrHardwareNotAvailable, if Wim card suddenly removed
        *         Any other system wide error code (e.g. KErrNoMemory)
        * @return void
        */
        void IsApplicable( const CCTCertInfo& aCertInfo, 
                           TUid aApplication, 
                           TBool& aIsApplicable, 
                           TRequestStatus& aStatus );

        /**
        * Cancels an IsApplicable operation. No influence.
        * @return void
        */
        void CancelIsApplicable();

        /**
        * Returns true value if given certificate is trusted. 
        * The caller of this function owns all its parameters.
        * @param  aCertInfo (IN) The certificate we're interested in.
        * @param  aTrusted (OUT) Used to return the trust status.
        * @param  aStatus (IN/OUT) A request status that is completed when the 
        *         operation has finished.
        *         KErrNone, if no errors detected.
        *         KErrNotFound, if aCertInfo not found
        *         KErrHardwareNotAvailable, if Wim card suddenly removed
        *         Any other system wide error code (e.g. KErrNoMemory)
        * @return void
        */
        void Trusted( const CCTCertInfo& aCertInfo, 
                      TBool& aTrusted, 
                      TRequestStatus& aStatus );

        /**
        * Cancels an ongoing Trusted operation. No influence.
        * @return void
        */
        void CancelTrusted();

        /**
        * Retrieves the actual certificate.
        * The caller of this function owns all its parameters.
        * @param  aCertInfo (IN) The certificate to retrieve.
        * @param  aEncodedCert (OUT) A buffer to put the certificate in. 
        *         It must be big enough. The size is stored in aCertInfo.
        * @param  aStatus (IN/OUT) A request status that is completed when the 
        *         operation has finished.
        *         KErrNone, if no errors detected.
        *         KErrNotFound, if aCertInfo not found
        *         KErrHardwareNotAvailable, if Wim card suddenly removed
        *         Any other system wide error code (e.g. KErrNoMemory)
        * @return void
        */
        void Retrieve( const CCTCertInfo& aCertInfo, 
                       TDes8& aEncodedCert, 
                       TRequestStatus& aStatus );

        /**
        * Cancels an ongoing Retrieve operation.
        * @return void
        */
        void CancelRetrieve();

    public: // From base class MCTWritableCertStore

        /** 
        * CURRENTLY NOT SUPPORTED. Adds a certificate to the cert store.
        * The caller of this function owns all its parameters.
        * @param  aLabel (IN) The label of the certificate to add
        * @param  aFormat (IN) the format of the certificate
        * @param  aCertificateOwnerType (IN) The owner type
        * @param  aSubjectKeyId (IN) The Subject key ID
        * @param  aIssuerKeyId (IN) The issuer key ID
        * @param  aCert (IN) The certificate to add
        * @param  aStatus (IN/OUT) This is completed with the return result 
        *         when the add has completed 
        *         KErrNone, if no errors detected.
        *         KErrArgument, 
        *           - if label length is zero, or
        *           - certificate owner type is unknown, or
        *           - certificate is X509 or WTLS and computed subject key id 
        *             is not as given subject key id
        *           - X509 and Url, and subject key is not given
        *         KErrNotSupported, if certificate format not supported.
        *         (Supported formats are: X509, WTLS and URL)
        *         KErrHardwareNotAvailable, if Wim card suddenly removed
        *         Any other system wide error code (e.g. KErrNoMemory)
        * return  void
        */
        void Add( const TDesC& aLabel, 
                  TCertificateFormat aFormat,
                  TCertificateOwnerType aCertificateOwnerType, 
                  const TKeyIdentifier* aSubjectKeyId,
                  const TKeyIdentifier* aIssuerKeyId,
                  const TDesC8& aCert, 
                  TRequestStatus& aStatus );

        /**
        * Cancels an ongoing add operation.
        * @return void
        */
        void CancelAdd();

        /** 
        * CURRENTLY NOT SUPPORTED. Removes a certificate from Wim cert store.
        * The caller of this function owns all its parameters.
        * @param  aCertInfo (IN) The certificate to remove.
        * @param  aStatus (IN/OUT) This is completed with the return result 
        *         when the remove has completed.
        *         KErrNone, if no errors detected.
        *         KErrNotFound, if aCertInfo not found
        *         KErrNotSupported, if certificate format not supported.
        *         (Supported formats are: X509, WTLS and URL)
        *         KErrHardwareNotAvailable, if Wim card suddenly removed
        *         Any other system wide error code (e.g. KErrNoMemory)
        */
        void Remove( const CCTCertInfo& aCertInfo, 
                     TRequestStatus& aStatus );

        /** 
        * Cancels an ongoing remove operation.
        * @return  void
        */
        void CancelRemove();

        /** 
        * Replaces the current applicability settings with the settings
        * in the supplied array. 
        * The caller of this function owns all its parameters except aApplications 
        * which ownership is transferred to this class
        * @param  aCertInfo (IN) The certificate whose applicability should 
        *         be updated.
        * @param  aApplications (IN) The new applications. 
        * @param  aStatus (IN/OUT) This is completed with the return result 
        *         when the operation has completed.
        *         KErrNone, if no errors detected.
        *         KErrNotFound, if aCertInfo not found
        *         KErrArgument, if some of the given applications is not 
        *         found from system file (certclients.dat)
        *         KErrHardwareNotAvailable, if Wim card suddenly removed
        *         Any other system wide error code (e.g. KErrNoMemory)
        */
        void SetApplicability( const CCTCertInfo& aCertInfo,
#ifdef __SECURITY_PLATSEC_ARCH__        
                               const RArray<TUid>& aApplications, 
#else
                               RArray<TUid>* aApplications,
#endif                               
                               TRequestStatus &aStatus );

        /** 
        * Cancels an ongoing operation. No influence.
        * @return void
        */
        void CancelSetApplicability();
    
        /** 
        * Changes the trust settings. CA certificate is 
        * trusted if the user is willing to use it for authenticating servers. 
        * It has no meaning with other types of certificates. 
        * The caller of this function owns all its parameters.
        * @param  aCertInfo (IN) The certificate to update.
        * @param  aTrusted (IN) Whether or not it is trusted
        * @param  aStatus (IN/OUT) This is completed with the return result 
        *         when the operation has completed.
        *         KErrNone, if no errors detected.
        *         KErrNotFound, if aCertInfo not found
        *         KErrArgument, if given aTrusted parameter is not true or false.
        *         KErrHardwareNotAvailable, if Wim card suddenly removed
        *         Any other system wide error code (e.g. KErrNoMemory)
        * @return void
        */
        void SetTrust( const CCTCertInfo& aCertInfo, 
                       TBool aTrusted, 
                       TRequestStatus& aStatus );

        /** 
        * Cancels an ongoing operation. No influence.
        * @return void
        */
        void CancelSetTrust();

    private: // New functions

        /** 
        * Makes some initializations.
        * @return void
        */
        void Initialize();

        /** 
        * Sets trusted and appliaction info on certificate mapping.
        * @param  aTrusted   (IN) True or false value
        * @param  aCertificateApps  (IN) An array of application uids for 
        *         certificate
        * @return void
        */
        void SetTrustSettingsOnMappingL( TBool aTrusted,
                                         RArray<TUid>* aCertificateApps );

    protected:  // From base class MCTTokenInterface
        
        virtual void DoRelease();

    protected:  // From base class CActive

        void RunL();
        
        TInt RunError(TInt aError);
        
        void DoCancel();

    private: // New functions

        /** 
        * Creates iCerts array and loads certificate infos into it.
        * @return void
        */
        void LoadMappingsL();

        // Computes and checks subject key in add operation
        void ComputeAndCheckSubjectKeyIdL();

        /** 
        * Checks if given application is known by system
        * @param  aCertInfo   (IN) The certificate that the update concern
        * @param  aApplications  (IN) An array of application uids for the
        *         certificate
        * @param  aStatus (IN/OUT) This is completed with the return result 
        *         when the operation has completed.
        * @return void
        */

        void CheckApplicabilityL(
#ifdef __SECURITY_PLATSEC_ARCH__        
                                  const RArray<TUid>& aTrusters );
#else
                                  RArray<TUid>* aTrusters );
#endif         

        /** 
        * Starts applicability updates and completes the message
        * @return void
        */

        void DoSetApplicability();

        /** 
        * Contains actual functions to update applicability
        * @return void
        */

        void DoSetApplicabilityL();

        /** 
        * Sets trust information for a certificate
        * @param  aCertInfo   (IN) The certificate that the update concern
        * @param  aTrusted  (IN) Trusted info for the certificate, values
        *         are true (trusted) or false (not trusted)
        * @param  aStatus (IN/OUT) This is completed with the return result 
        *         when the operation has completed.
        * @return void
        */
        void DoSetTrustL();

        // General function to free unified key storage objects
        void FreeUnifiedKeyStore();
        
        // RunL phase where LoadMappingsL is called
        void DoInitializeLoadMappingsL();

        // RunL phase where trust settings are loaded
        void DoInitializeLoadTrustSettingsStartL();

        // RunL phase where trust settings call are waited
        void DoInitializeLoadTrustSettingsWaitL();

        // RunL phase where key infos are fetched
        void DoGetKeyInfos();

        // RunL phase where list operation is actually done
        void DoListGoL();

        // RunL phase where list operation is actually started
        void DoInitializeGetCertListL();

        // RunL phase where list operation is started
        void DoList();

        // RunL phase where private key is checked
        void DoCheckCorrespondingPrivateKey();

        // RunL phase where delete from wim is checked
        void DoCheckDeleteFromWim();

        // Returns the status of token
        TBool TokenRemoved();

        /** 
        * Checks if token is removed or is this object active
        * @param  aStatus (IN/OUT) This is completed with the return result 
        *         when the operation has completed.
        * @return TBool
        */
        TBool EnteringAllowed( TRequestStatus& aStatus );

        // RunL phase for getting a certificate info
        void DoGetCert();

        // RunL phase for getting a certificate applications
        void DoApplications();

        // RunL phase for checking application applicability
        void DoIsApplicable();

        // RunL phase for checking certificate's trust info
        void DoTrusted();

        // RunL phase for retrieving certificate binary data
        void DoRetrieve();

        // RunL phase for adding a certificate
        void DoAdd();

        // RunL phase for certificate removal
        void DoRemove();

        // RunL phase for certificate trusted info
        void DoSetTrust();

        /** 
        * Cleans trusters array if leave occurs
        * @param  aTrusters (IN) An array of applications (trusters)
        * @return void
        */
        static void CleanTrustersArray( TAny* aTrusters );

    private: // Constructors
        
        /** 
        * The default constructor
        * @param  aToken   (IN) A reference of current token
        * @return None
        */
        CWimCertStore( CWimToken& aToken );

        // Second phase constructor
        void ConstructL();

    private: // Data

        // Reference to current token of this cert store interface. 

        CWimToken& iToken;

        // State flag for RunL

        enum TPhase
            {
            EInitializeGetCertList = 0,
            EInitializeLoadMappings,
            EInitializeLoadTrustSettingsStart,
            EInitializeLoadTrustSettingsWait,
            EList,
            EListGo,
            EGetKeyInfos,
            EGetCert,
            EApplications,
            EIsApplicable,
            ETrusted,
            ERetrieve,
            ERetrieveFromWim,
            ERetrieveWait,
            EGetCorrespondingPrivateKey,
            ECheckCorrespondingPrivateKey,
            EAdd,
            EAddToWim,
            ECheckAddToWim,
            ERemove,
            EWaitRemoveTrustSettings,
            EDeleteFromWim,
            ECheckDeleteFromWim,
            ESetApplicability,
            EWaitSetApplicability,
            ESetTrust,
            EWaitSetTrust,
            ECompleteMessage,
            EIdle
            };

        // Flag for internal state machine
        TPhase iPhase;

        // Save place for original state
        TPhase iPhaseOriginal;

        // Used for saving caller status 
        // This class don't own the pointed object
        TRequestStatus* iOriginalRequestStatus;

        // The list of certificates (mapping objects) contained in the 
        // trust store.
        // This class owns the pointed objects and is responsible
        // to release them in the end of this class life cycle.
        CWimCertStoreMappings* iCerts;

        // Temporal array for certificate infos. 
        // This array is used to fetch certificate infos from WimClient.
        // At first this class owns the pointed objects. Ownership is 
        // transferred to iCerts. See iCerts above.
        RPointerArray<CWimCertInfo> iCertInfos;

        // Temporal array for certificate info pointers
        // This array is used to append certificate infos in List operation
        // This class don't own the pointed objects
        RPointerArray<CCTCertInfo>* iCertsList;

        // Array for adding certificate applications to trust store
        // This class don't own the pointed objects
        RArray<TUid>* iCertificateApps;

        // Index for going through all certificates
        TInt iCertIndex;

        // Used for fetching and updating trust settings
        TBool iTrusted;
    
        // Used in List operation in order to use in RunL
        // This class don't own the pointed object.
        const CCertAttributeFilter* iFilter;

        // Used with unified key store to filter keys
        TCTKeyAttributeFilter iKeyFilter;   

        // Used with unified key store to save keys for a while
        RMPointerArray<CCTKeyInfo> iKeyInfos;

        // A pointer to unified key store
        // This class owns also the pointed object
        CUnifiedKeyStore* iUnifiedKeyStore;

        // Used in Retrieve operation in order to use in RunL
        // Points to whole certificate data string
        // This class don't own the pointed data
        TDes8* iEncodedCert;

        // The pointed object acts as a converter between
        // CWimCertStore interface and WimClient
        // This class owns the pointed object
        CWimCertConverter* iCWimCertConverter;

        // The pointed object acts as a database store for trust settings
        // This class owns the pointed object
        CWimTrustSettingsAPI* iCWimTrustSettingsStore;

        // A pointer to label of certificate to be added
        // This class don't own the pointed object
        const TDesC* iLabel;

        // A pointer to format of certificate to be added
        TCertificateFormat iFormat;

        // Owner type  of certificate to be added
        TCertificateOwnerType iCertificateOwnerType;

        // A pointer to subject key id of certificate to be added
        // This class don't own the pointed object
        const TKeyIdentifier* iSubjectKeyId;

        // A pointer to issuer key id of certificate to be added
        // This class don't own the pointed object
        const TKeyIdentifier* iIssuerKeyId;

        // A pointer to binary data of certificate to be added
        // This class don't own the pointed object
        const TDesC8* iCert;

        // A hash of subject key id of certificate to be added
        TKeyIdentifier iComputedSubjectKeyId;   

        // For the call of unified keystore.
        RFs iFs;

        // A pointer to user's certificate data area. Used during 
        // set active object. Not owned.
        CCTCertInfo** iCertInfo;

        // A pointer to user given certificate handle. Not owned.
        const TCTTokenObjectHandle* iHandle;

        // A pointer to user's certificate data area. Used during 
        // set active object. Not owned.
        const CCTCertInfo* iCertInfoReadOnly;

        // A pointer array application Uids
        RArray<TUid>* iApplications;

        // A place holder to application Uid
        TUid iApplication;

        // A pointer to boolean variable in user area. Not owned.
        TBool* iIsApplicable;

        // A pointer to boolean variable in user area. Not owned.
        TBool* iTrustedCert;

        // Place holder for certificate trusted info update
        TBool iTrustedValue;

        // Save old trust value for back up reason
        TBool iOldTrusted;

        // Pointer array for old applications. Ownership moved to mapping
        // if setting new applications fails.
        RArray<TUid>* iOldTrusters;

};

#endif      // CWIMCERTSTORE_H   
            
// End of File
