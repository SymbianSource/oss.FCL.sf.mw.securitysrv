/*
* Copyright (c) 2006-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   Definition of the CCTSecurityDialogsAO class
*
*/


#ifndef CTSECURITYDIALOGSAO_H
#define CTSECURITYDIALOGSAO_H

//  INCLUDES
#include "CTSecurityDialogDefs.h"
#include <secdlgimpldefs.h>
#include <badesca.h>
#include <mctkeystore.h>

// FORWARD DECLARATIONS
class CCTSecurityDialogNotifier;
class CUnifiedCertStore;
class CUnifiedKeyStore;
class CCertAttributeFilter;
class CCTCertInfo;
class CCTKeyInfo;
class CX509Certificate;
class CX500DistinguishedName;
class MCTWritableCertStore;
class CAknQueryDialog;
class CCTPinQueryDialog;


// CLASS DECLARATION

/**
*  Active object class of CT Security Dialogs
*  Calls from client side are handled in this class.
*
*  @lib CTSecDlgNotifier.dll
*  @since Series 60 2.8
*/
NONSHARABLE_CLASS( CCTSecurityDialogsAO ): public CActive
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CCTSecurityDialogsAO* NewL( CCTSecurityDialogNotifier* aNotifier, TBool& aDeleted );

        /**
        * Destructor.
        */
        virtual ~CCTSecurityDialogsAO();

    public: // New functions

        /**
        * Starts handling of the message
        * @param aBuffer  Data passed from client RNotifier::StartNotifierAndGetResponse() call
        * @param aReplySlot  Identifies which message argument to use for the reply
        * @param aMessage  Encapsulates the client request
        */
        void StartLD( const TDesC8& aBuffer, TInt aReplySlot, const RMessagePtr2& aMessage );


        // For server authentication failure
        /**
        * Creates the message to be shown in the certificate details query.
        *
        * @return message text
        */
        HBufC* CreateMessageL();

    protected:  // Functions from CActive

        /**
        * RunL is called after modeless dialog is dismissed.
        */
        void RunL();

        /**
        * Does nothing.
        */
        void DoCancel();

        /**
        * If RunL leaves, error is handled here.
        */
        TInt RunError(TInt aError);

    private:

        enum TOperationStatus
        {
            EOperationCompleted = 0xFFF0,
            EOperationCancel,
            EOperationSignTextShown,
            EOperationInitCertStore,
            EOperationRetrieveCertInfos,
            EOperationSelectCert,
            EGetCertInfo,
            EGetCertificate,
            EInitKeyStore,
            EGetKeyInfos,
            EGetKeyInfoByHandle,
            EShowCertDialog,
            EShowCSRDialog,
            EEnterNewPIN,
            EEnterUnblockPIN,
            EVerifyNewPin,
            EVerifyPINs,
            EServerCertCheckUserResp,
            ESaveServerCert,
            EAddTrustedSite,
            ERetrieveServerCerts,
            ECheckServerCerts,
            EProcessTrustedSite,
            EUnknownError
            };

        /**
        * C++ default constructor.
        */
        CCTSecurityDialogsAO( CCTSecurityDialogNotifier* aNotifier, TBool& aDeleted );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        void InitCertStoreL();

        void InitKeyStoreL();

        void GetKeyInfosL();

        void GetKeyInfoByHandleL();

        void DoHandlePinOperationL();

        void DoHandleUnblockPinOperationL();

        void DoHandleMultilinePinQueryL(const TInt& aDlgType);

        void DoHandleSignTextL(const TDesC8& aBuffer);

        void DoHandleCSRL(const TDesC8& aBuffer);

        void DoHandleCertDetailsL(const TDesC8& aBuffer);

        void DoHandleSaveCertL(const TDesC8& aBuffer);

        void DoHandleSaveServerCertL();

        void DoHandleServerAuthFailL(const TDesC8& aBuffer);

        void ShowCertDialogL();

        void ShowNoTrustDialogL();

        void ShowCSRDialogL();

        void ShowInformationNoteL( TInt aResourceID ) const;

        /**
        * Saves digital signature receipt
        */
        void SaveReceiptL(const TDesC8& aBuffer);

        void DoHandleSelectCertificateL();

        void DoHandleMessageL(
            const TInt& aDlgType,
            const TDesC& aDynamicText,
            const TDesC& aPinQueryHeading,
            const TInt aMinLength,
            const TInt aMaxLength);

        /////////////////////////////////
        // Certificate details functions

        HBufC* MessageQueryCertDetailsL(
            const TDesC8& aCert,
            const CCTCertInfo* aCertInfo,
            TCertificateFormat aCertFormat,
            const CCTKeyInfo* aKeyInfo);

        void AddSiteL( TDes& aMessage );

        void AddIssuerAndSubjectL(
            TDes& aMessage,
            const CX509Certificate& aCert);

        void AddKeyUsageL(
            TDes& aMessage,
            const CCTKeyInfo* aKeyInfo );

        void AddKeyUsageL(
            TDes& aMessage,
            const CX509Certificate& aCert);

        void AddKeyLocationL(
            TDes& aMessage,
            const CCTKeyInfo* aKeyInfo );

        void AddKeySizeL(
            TDes& aMessage,
            const CCTKeyInfo* aKeyInfo );

        void AddKeyAlgorithmL(
            TDes& aMessage,
            const CCTKeyInfo* aKeyInfo );

        void AddCertLocationL(
            TDes& aMessage,
            const CCTCertInfo* aCertInfo );

        void AddLocationInfoL(
            TDes& aMessage,
            TUid aUid,
            TBool aCertificate );

        void AddValidityPeriodL(
            TDes& aMessage, const CX509Certificate& aCert);

        void AddCertFormatL(
            TDes& aMessage, TCertificateFormat aCertFormat);

        void AddCertAlgorithmsL(
            TDes& aMessage, const CX509Certificate& aCert);

        void AddCertSerialNumberL(
            TDes& aMessage, const CX509Certificate& aCert);

        void AddCertFingerprintsL(
            TDes& aMessage, const CX509Certificate& aCert);

        void DevideToBlocks( const TDesC8& aInput, TDes& aOutput);

        void DetailsFieldDynamicL(
            TDes& aMessage,
            const TDesC& aValue,
            TInt aResourceOne,
            TInt aResourceTwo);

        void DetailsFieldResourceL(
            TDes& aMessage, TInt aResourceOne,
            TInt aResourceTwo);

        void DetailsResourceL(TDes& aMessage, TInt aResource);

        void DetailsDynamicL(
            TDes& aMessage,
            const TDesC& aValue,
            TInt aResource);

        void MapTlsProviderOperation( TUint aOperation );

        /**
        * Completes client's request
        */
        void HandleResponseAndCompleteL();

        /**
        * Checks that are two PINs the same
        */
        void VerifyPinsL();

    private:    // Data
        RMessagePtr2 iMessagePtr;
        TSecurityDialogNotification iOperation;
        TBool iRetry;
        TSignInput iSignInput;
        TPINParams iPIN;
        TPINParams iUnblockPIN;
        TInt iReplySlot;
        TPINValue iPINValue1;
        TPINValue iPINValue2;
        TPINValue iPINValueVerify;
        RArray<TCTTokenObjectHandle> iCertHandleList;
        TBool iRetValue;
        TCTTokenObjectHandle iTokenHandle;

        TServerAuthenticationFailureInput iServerAuthenticationFailureInput;

        CCTSecurityDialogNotifier* iNotifier;
        TInt iNextStep;
        TInt iMultiLineDlgType;

        // For retrieving signText dialog
        CUnifiedCertStore* iCertStore;
        CCertAttributeFilter* iFilter;
        RFs iFs;
        CCTCertInfo* iCertInfo;
        CDesCArrayFlat*  iCertArray;
        RMPointerArray<CCTCertInfo> iCertInfos;
        HBufC8* iCertBuf;
        MCTWritableCertStore *iTrustedSiteCertStore;
        HBufC* iCertLabel;
        HBufC8* iServerCert;
        RMPointerArray<HBufC8> iServerCerts;
        HBufC* iServerName;
        TBool iTrustedSite;
        TInt iHandleIndex;
        TPtr8 iCertPtr;

        CUnifiedKeyStore* iKeyStore;
        CCTKeyInfo* iKeyInfo;
        RMPointerArray<CCTKeyInfo> iKeyInfos;
        TCTKeyAttributeFilter iKeyFilter;
        HBufC* iText;

        TSaveCertInput iSaveCertInput;
        TValidationError iAuthFailReason;

        TBool& iDeleted;

        CCTPinQueryDialog* iPinQueryDialog;
        TBool iPinQueryDialogDeleted;
        };

#endif      // CTSECURITYDIALOGSAO_H

// End of File
