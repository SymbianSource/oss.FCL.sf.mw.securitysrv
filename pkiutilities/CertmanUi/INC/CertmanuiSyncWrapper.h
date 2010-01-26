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
* Description:   Wrapper class for Symbian's Security Framework's calls
*
*/


#ifndef     CERTMANUISYNCWRAPPER_H
#define     CERTMANUISYNCWRAPPER_H


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
NONSHARABLE_CLASS( CCertManUISyncWrapper ): public CActive
    {
    public:

        /**
        * Two-phased constructor.
        */
        static CCertManUISyncWrapper* NewLC();

        /**
        * Two-phased constructor.
        */
        static CCertManUISyncWrapper* NewL();

        /**
        * Destructor.
        */
        ~CCertManUISyncWrapper();

    public: // New functions

        /**
        * TInt ListL
        * @since S60 2.8
        * @param aStore reference to store where operation is done.
        * @param aArray Keys are stored here.
        * @param aFilter Information on what kind keys are looked.
        * @return TInt Status Symbian's API's call.
        */

        TInt ListL( CUnifiedKeyStore*& aStore,
            RMPointerArray<CCTKeyInfo>* aArray,
            const TCTKeyAttributeFilter& aFilter );

        /**
        * TInt ListL
        * @since S60 3.2
        * @param aStore reference to store where operation is done.
        * @param aArray Keys are stored here.
        * @param aFilter Information on what kind keys are looked.
        * @param aTokenUid Token UID of key store
        * @return TInt Status Symbian's API's call.
        */

        TInt ListL( CUnifiedKeyStore*& aStore,
            RMPointerArray<CCTKeyInfo>* aArray,
            const TCTKeyAttributeFilter& aFilter,
            const TUid aTokenUid );

        /**
        * TInt ListL
        * @since S60 2.0
        * @param aStore reference to store where operation is done.
        * @param aArray certificates are stored here.
        * @param aFilter Information on what kind certificates are looked.
        * @return TInt Status Symbian's API's call.
        */
        TInt ListL( CUnifiedCertStore*& aStore,
            RMPointerArray<CCTCertInfo>* aArray,
            const CCertAttributeFilter& aFilter );

        /**
        * TInt ListL
        * @since S60 3.2
        * @param aStore reference to store where operation is done.
        * @param aArray certificates are stored here.
        * @param aFilter Information on what kind certificates are looked.
        * @param aTokenUid Token UID of certificate store
        * @return TInt Status Symbian's API's call.
        */
        TInt ListL( CUnifiedCertStore*& aStore,
            RMPointerArray<CCTCertInfo>* aArray,
            const CCertAttributeFilter& aFilter,
            const TUid aTokenUid );


        /**
        * TInt GetCertificateL
        * @since S60 2.0
        * @param aStore reference to store where operation is done.
        * @param aCertInfo Certificate that's detailed information is fetched.
        * @param aCert Fetched certificate.
        * @return TInt Status Symbian's API's call.
        */
        TInt GetCertificateL( CUnifiedCertStore*& aStore,
            const CCTCertInfo& aCertInfo, CCertificate*& aCert );

        /**
        * TInt GetCertificateL
        * @since S60 3.2
        * @param aStore reference to store where operation is done.
        * @param aCertInfo Certificate that's detailed information is fetched.
        * @param aCert Fetched certificate.
        * @return TInt Status Symbian's API's call.
        */
        TInt GetCertificateL( CUnifiedCertStore*& aStore,
            const CCTCertInfo& aCertInfo, CCertificate*& aCert, TUid aTokenUid );


        /**
        * TInt GetUrlCertificateL
        * @since S60 3.0
        * @param aStore reference to store where operation is done.
        * @param aCertInfo Certificate that's detailed information is fetched.
        * @param aUrl Fetched URL.
        * @return TInt Status Symbian's API's call.
        */
        TInt GetUrlCertificateL(
            CUnifiedCertStore*& aStore,
            const CCTCertInfo& aCertInfo,
            TDes8& aUrl)            ;

        /**
        * TInt GetApplicationsL
        * @since S60 2.0
        * @param aStore reference to store where operation is done.
        * @param aCertInfo Certificate that applications are fetched.
        * @param Certificates applications are stoted here.
        * @return TInt Status Symbian's API's call.
        */
        TInt GetApplicationsL( CUnifiedCertStore*& aStore,
            const CCTCertInfo& aCertInfo, RArray<TUid>& aApps );

        /**
        * TInt ValidateX509RootCertificateL
        * @since S60 2.0
        * @param aValidationResult Result of validation.
        * @param aValidationTime Time of validation.
        * @param aChain Chain to be validated.
        * @return TInt Status Symbian's API's call.
        */
        TInt ValidateX509RootCertificateL(
            CPKIXValidationResult*& aValidationResult,
            const TTime& aValidationTime, CPKIXCertChain* aChain );

        /**
        * TInt InitStoreL
        * @since S60 2.0
        * @param aStore reference to store to be initialized.
        * @return TInt Status Symbian's API's call.
        */
        TInt InitStoreL( CUnifiedCertStore*& aStore );

        /**
        * void InitStoreL
        * @since S60 2.8
        * @param aStore reference to store to be initialized.
        * @return TInt Status Symbian's API's call.
        */
        TInt InitStoreL( CUnifiedKeyStore*& aStore );

        /**
        * TInt DeleteCertL
        * @since S60 2.0
        * @param aStore reference to store where operation is done.
        * @param aCertInfo Certificate to be deleted.
        * @return TInt Status Symbian's API's call.
        */
        TInt DeleteCertL( CUnifiedCertStore*& aStore,
            const CCTCertInfo& aCertInfo );

        /**
        * TInt DeleteCertL
        * @since S60 3.2
        * @param aStore reference to store where operation is done.
        * @param aCertInfo Certificate to be deleted.
        * @return TInt Status Symbian's API's call.
        */
        TInt DeleteCertL( CUnifiedCertStore*& aStore,
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
        * @since S60 2.0
        * @param aStore reference to store where operation is done.
        * @param aCertInfo Certificate thats applicability is set.
        * @param aApplications New applicability settings.
        * @return TInt Status Symbian's API's call.
        */
        TInt SetApplicabilityL( CUnifiedCertStore*& aStore,
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
        * @since S60 3.2
        * @param aStore reference to store where operation is done
        * @param CCTKeyInfo Key info
        * @param aSourceStore UID of source key store
        * @param aTargetStore UID of source key store
        * @return none
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

        CCertManUISyncWrapper();
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

        // Token handle
        MCTToken*               iToken;

    };

#endif // CERTMANUISYNCWRAPPER_H

// End of File
