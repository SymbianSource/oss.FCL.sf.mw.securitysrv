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
* Description:   Wrapper class for Symbian's Security Framework's calls
*
*/


#ifndef     CPCERTMANUISYNCWRAPPER_H
#define     CPCERTMANUISYNCWRAPPER_H


// INCLUDES
#include <e32base.h>

// FORWARD DECLERATIONS
class CCertificate;
class CPKIXValidationResult;
class CPKIXCertChain;
class CUnifiedCertStore;

// CLASS DECLARATION

/**
*  Wrapper class for Symbian's Security Framework's calls.
*  Shows error notes if an error happenes.
*
*  @lib certmanui.dll
*  @since S60 2.0
*/
class CpCertManUISyncWrapper : public CActive
    {
    public:

        /**
        * Two-phased constructor.
        */
        static CpCertManUISyncWrapper* NewLC();

        /**
        * Two-phased constructor.
        */
        static CpCertManUISyncWrapper* NewL();

        /**
        * Destructor.
        */
        ~CpCertManUISyncWrapper();

    public: // New functions

        /**
        * TInt ListL
        * @param aStore reference to store where operation is done.
        * @param aArray Keys are stored here.
        * @param aFilter Information on what kind keys are looked.
        */

        void ListL( CUnifiedKeyStore*& aStore,
            RMPointerArray<CCTKeyInfo>* aArray,
            const TCTKeyAttributeFilter& aFilter );

        /**
        * TInt ListL
        * @param aStore reference to store where operation is done.
        * @param aArray Keys are stored here.
        * @param aFilter Information on what kind keys are looked.
        * @param aTokenUid Token UID of key store
        */

        void ListL( CUnifiedKeyStore*& aStore,
            RMPointerArray<CCTKeyInfo>* aArray,
            const TCTKeyAttributeFilter& aFilter,
            const TUid aTokenUid );

        /**
        * TInt ListL
        * @param aStore reference to store where operation is done.
        * @param aArray certificates are stored here.
        * @param aFilter Information on what kind certificates are looked.
        */
        void ListL( CUnifiedCertStore*& aStore,
            RMPointerArray<CCTCertInfo>* aArray,
            const CCertAttributeFilter& aFilter );

        /**
        * TInt ListL
        * @param aStore reference to store where operation is done.
        * @param aArray certificates are stored here.
        * @param aFilter Information on what kind certificates are looked.
        * @param aTokenUid Token UID of certificate store
        */
        void ListL( CUnifiedCertStore*& aStore,
            RMPointerArray<CCTCertInfo>* aArray,
            const CCertAttributeFilter& aFilter,
            const TUid aTokenUid );


        /**
        * TInt GetCertificateL
        * @param aStore reference to store where operation is done.
        * @param aCertInfo Certificate that's detailed information is fetched.
        * @param aCert Fetched certificate.
        */
        void GetCertificateL( CUnifiedCertStore*& aStore,
            const CCTCertInfo& aCertInfo, CCertificate*& aCert );

        /**
        * TInt GetCertificateL
        * @param aStore reference to store where operation is done.
        * @param aCertInfo Certificate that's detailed information is fetched.
        * @param aCert Fetched certificate.
        */
        void GetCertificateL( CUnifiedCertStore*& aStore,
            const CCTCertInfo& aCertInfo, CCertificate*& aCert, TUid aTokenUid );


        /**
        * TInt GetUrlCertificateL
        * @param aStore reference to store where operation is done.
        * @param aCertInfo Certificate that's detailed information is fetched.
        * @param aUrl Fetched URL.
        */
        void GetUrlCertificateL(
            CUnifiedCertStore*& aStore,
            const CCTCertInfo& aCertInfo,
            TDes8& aUrl)            ;

        /**
        * TInt GetApplicationsL
        * @param aStore reference to store where operation is done.
        * @param aCertInfo Certificate that applications are fetched.
        * @param Certificates applications are stoted here.
        */
        void GetApplicationsL( CUnifiedCertStore*& aStore,
            const CCTCertInfo& aCertInfo, RArray<TUid>& aApps );

        /**
        * TInt ValidateX509RootCertificateL
        * @param aValidationResult Result of validation.
        * @param aValidationTime Time of validation.
        * @param aChain Chain to be validated.
        */
        void ValidateX509RootCertificateL(
            CPKIXValidationResult*& aValidationResult,
            const TTime& aValidationTime, CPKIXCertChain*& aChain );

        /**
        * TInt InitStoreL
        * @param aStore reference to store to be initialized.
        */
        void InitStoreL( CUnifiedCertStore*& aStore );

        /**
        * void InitStoreL
        * @param aStore reference to store to be initialized.
        */
        void InitStoreL( CUnifiedKeyStore*& aStore );

        /**
        * TInt DeleteCertL
        * @param aStore reference to store where operation is done.
        * @param aCertInfo Certificate to be deleted.
        */
        void DeleteCertL( CUnifiedCertStore*& aStore,
            const CCTCertInfo& aCertInfo );

        /**
        * TInt DeleteCertL
        * @param aStore reference to store where operation is done.
        * @param aCertInfo Certificate to be deleted.
        */
        void DeleteCertL( CUnifiedCertStore*& aStore,
            const CCTCertInfo& aCertInfo,
            TUid aTokenUid );

        /**
        * TInt IsApplicableL
        * @since S60 2.0
        * @param aStore reference to store where operation is done.
        * @param aCertInfo Certificate thats applicability is queried.
        * @param aApplication Application that's status is queried.
        * @param aIsApplicable ETrue if applcable.
        * @return TInt Status Symbian's API's call.
        */
        TInt IsApplicableL( CUnifiedCertStore*& aStore,
            const CCTCertInfo& aCertInfo, TUid aApplication,
        TBool& aIsApplicable );

        /**
        * TInt IsTrustedL
        * @since S60 2.0
        * @param aStore reference to store where operation is done.
        * @param aCertInfo Certificate that's trust status is queried.
        * @param aTrusted ETrue if trusted.
        * @return TInt Status Symbian's API's call.
        */
        TInt IsTrustedL( CUnifiedCertStore*& aStore,
            const CCTCertInfo& aCertInfo, TBool& aTrusted );

        /**
        * TInt SetApplicabilityL
        * @param aStore reference to store where operation is done.
        * @param aCertInfo Certificate thats applicability is set.
        * @param aApplications New applicability settings.
        */
        void SetApplicabilityL( CUnifiedCertStore*& aStore,
            const CCTCertInfo& aCertInfo,
            RArray<TUid>& aApplications );

      /**
        * void MoveCertL
        * @since S60 3.2
        * @param aStore reference to store where operation is done
        * @param aCertInfo Certificate info
        * @param aSourceStore UID of source certificate store
        * @param aTargetStore UID of source certificate store
        * @return TInt Number of moved certificates
        */
        TInt MoveCertL( CUnifiedCertStore*& aStore,
            const CCTCertInfo& aCertInfo,
            const TUid aSourceTokenId,
            const TUid aTargetTokenId );

      /**
        * void MoveKeyL
        * @param aStore reference to store where operation is done
        * @param CCTKeyInfo Key info
        * @param aSourceStore UID of source key store
        * @param aTargetStore UID of source key store
        */
        void MoveKeyL( CUnifiedKeyStore*& aStore,
            const TCTKeyAttributeFilter& aFilter,
            const TUid aSourceTokenId,
            const TUid aTargetTokenId );

        /**
        * TInt GetInterface
        * @since S60 3.0
        * @param aToken reference to token
        * @param aTokenInterface Pointer to returned interface
        * @return TInt Status Symbian's API's call.
        */
        TInt GetInterface( TUid aRequiredInterface,
            MCTToken& aToken,
            MCTTokenInterface*& aReturnedInterface );

    protected:

        void DoCancel();
        void RunL();

    private:

        CpCertManUISyncWrapper();
        void ConstructL();

        void HandleErrorL();

    private:    //Data

        // Internal operation states.
        enum TOperation
            {
            EOperationNone,
            EOperationInit,
            EOperationList,
            EGetCertificate,
            EAddCertificate,
            ERetriveURLCertificate,
            EOperationDelete,
            EOperationGetApps,
            EOperationIsApplicable,
            EOperationIsTrusted,
            EOperationSetApplicability,
            EOperationSetToTrusted,
            EOperationValidateX509Root,
            EShowErrorNote,
            EOperationInitKeyStore,
            EOperationKeyList,
            EOperationGetInterface,
            EOperationExportKey,
            EOperationImportKey,
            EOperationDeleteKey
            };

        // For wrapping asynchronous calls.
        CActiveSchedulerWait    iWait;

        // Internal state of operation.
        TOperation              iOperation;

        // Just for cancelling a operation. Does not own.
        CUnifiedCertStore*      iStore;

        // Just for cancelling a operation. Does not own.
        CUnifiedKeyStore*       iKeyStore;

        // Just for cancelling a operation. Does not own.
        CPKIXCertChain*         iChain;

        // For retrieving a certificate.
        TPtr8                   iCertPtr;

    };

#endif // CPCERTMANUISYNCWRAPPER_H

// End of File
