/*
* Copyright (c) 2003-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   Model of the CertSaver. Saves the keys and certificates
*              : to the databases and shows appropriate dialogs for user
*
*/


#ifndef CERTSAVERMODEL_H
#define CERTSAVERMODEL_H

// INCLUDES
#include <f32file.h>
#include <e32std.h>
#include <ct/rmpointerarray.h>
#include <securitydefs.h>
#include <mpkcs12.h>
#include <cctcertinfo.h>
#include <signed.h>

// FORWARD DECLARATIONS
class CCertSaverDocument;
class CCertSaverAppUi;
class RFs;
class CCertSaverSyncWrapper;
class CUnifiedCertStore;
class CCTCertInfo;
class CX509Certificate;
class TCertificateAppInfo;
class CCertParser;

// CONSTANTS


// CLASS DECLARATION


/**
* CCertSaverModel dialog class.
* Saves certificates and shows
* needed queries and error notes for user.
*/
class CCertSaverModel : public CBase
    {

    public: // Constructors and destructor

        CCertSaverModel( CCertSaverAppUi* aAppUi, const CCertParser& aParser );

        /**
        * Destructor.
        */
        ~CCertSaverModel();

    public: // New functions

        void SaveCertificateL(
                const CX509Certificate& aCertificate,
                const TCertificateOwnerType& aOwnerType,
                const TCertificateFormat& aCertFormat );

         /**
        * Saves content of the PKCS#12 PDU.
        * @param void
        */
        void SavePKCS12L();


        /**
        * Shows a error note with given text.
        * @param aResourceID Id of the resource text to be shown.
        */
        void ShowErrorNoteL( TInt aResourceID ) const;

    private:  // New functions

        /**
        * Initialises unified certstore.
        */
        void InitCertStoreL();

         /**
        * Checks that there is enough space in filesystem
        * @param aDataToSave Data to be saved.
        */
        void CheckFSSpaceL( const TDesC8& aDataToSave );

        void SaveCertL();

        void SavePrivateKeyL();

        void DoSavePrivateKeyL( const TDesC8& aKey );

        void CreateKeyLabelL( TDes& aLabel );

        void GetKeyValidityPeriodL(
            TTime& aStartDate,
            TTime& aEndDate,
            const TKeyIdentifier& aKeyIdentifier,
            const CArrayPtr<CX509Certificate>& aCertArray );
        
        void GetKeyValidityPeriodL(
            TTime& aStartDate,
            TTime& aEndDate,
            const TKeyIdentifier& aKeyIdentifier );

        TKeyUsagePKCS15 KeyUsageL(
            const TKeyIdentifier& aKeyIdentifier,
            TAlgorithmId aAlgorithm );

        TBool KeyAlreadyExistsL(
            TTime& aStartDate,
            TTime& aEndDate,
            const TKeyIdentifier& aKeyIdentifier,
            TKeyUsagePKCS15& aKeyUsage );

        /**
        * Saves certificate to database in phone memory.
        */
        void DoSaveCertL();

        /**
        * Shows appropriate error note if something odd happeneds
        * certificate is saved. Shows also note if everything
        * goes fine.
        * @param aStatus Status of the save operation.
        */
        void HandleSaveErrorL( TInt aStatus ) const;

        /**
        * Adds string to aMessage.
        * @param aMessage Buffer where message is saved.
        * @param aStringResID id of the string to be added. Read from resources.
        */
        void AddToMessageL( TDes& aMessage, TInt aStringResID ) const;

        /**
        * Constructs the message shown to user in details-dialog.
        * @param aMessage Buffer where message is saved.
        */
        void ConstructMessageL( TDes& aMessage ) const;

        /**
        * Constructs the message shown to user in pkcs#12
        * file contents and saving completed queries.
        * @param aMessage Buffer where message is saved.
        */
        void ConstructPKCS12QueryMsgL(
            TDes& aMessage,
            TInt aPrivateKeys,
            TInt aUserCerts,
            TInt aCACerts ) const;

        /**
        * Adds string to aMessage.
        * @param aMessage Buffer where message is saved.
        * @param aStringResID id of the string to be added. Read from resources.
        * @param aString Replaces the first (and only) %U parameter in resources.
        */
        void AddToMessageWithStringL(
            TDes& aMessage,
            TInt aStringResID,
            const TDesC& aString ) const;

        /**
        * Adds string to aMessage.
        * @param aMessage Buffer where message is saved.
        * @param aStringResID id of the string to be added. Read from resources.
        * @param aString Replaces the first (and only) %U parameter in resources.
        */
        void AddToMessageWithIntL(
            TDes& aMessage,
            TInt aStringResID,
            TInt aInt ) const;

        /**
        * Adds validity period of the certificate to aMessage.
        * @param aMessage Buffer where message is appended.
        * @param aCert Certificate
        */
        void AddValidityPeriodL( TDes& aMessage, const CX509Certificate& ) const;

        /**
        * Adds key usage of the certificate to aMessage.
        * @param aMessage Buffer where message is appended.
        * @param aCert Certificate
        */
        void AddKeyUsageL( TDes& aMessage, const CX509Certificate& aCert ) const;

        /**
        * Adds new lines to message.
        * @param aMessage Buffer where message is saved.
        */
        void AddNewlinesToMessage( TDes& aMessage ) const;

        /**
        * Shows a confirmation note with given text.
        * @param aResourceID Id of the resource text to be shown.
        */
        void ShowConfirmationNoteL( TInt aResourceID ) const;

        /**
        * Shows a information note with given text.
        * @param aResourceID Id of the resource text to be shown.
        */
        void ShowInformationNoteL( TInt aResourceID ) const;

        /**
        * Checks that certificate is ok. That is, it's not
        * corrupted and it's of supported type. If certificate
        * is not ok an error note is displayed. If certificate
        * is not valid yet/anymore an error note is displayed
        * but certificate is considered to be ok ie. can be saved anyway.
        * @return TBool ETrue if ok else EFalse.
        */
        TBool CertificateOkL() const;

        /**
        * Checks that certificate is of supported type.
        * @return TBool ETrue if supported else EFalse.
        */
        TBool CertificateSupported() const;

        /**
        * Checks if certificate isn't valid anymore.
        * @return TBool ETrue if not valid else EFalse.
        */
        TBool CertNotValidAnymore() const;

        /**
        * Checks if certificate isn't valid yet.
        * @return TBool ETrue if not valid else EFalse.
        */
        TBool CertNotValidYet() const;

        /**
        * Trims given descriptor so that everything after and
        * including the fourth semicolon (;) is cropped. .
        * @param aField data to be trimmed.
        * @return TPtrC trimmed content
        */
        TPtrC TrimCertificateFields( TPtrC aField ) const;

        /**
        * If given descriptor contains more than 80
        * characters, this function cut it to 80.
        * @param aField data to be possibly cut.
        * @return TPtrC trimmed content
        */
        TPtrC CutCertificateField( TPtrC aField ) const;

        /**
        * Queries label from user and confirms that label doesn't already
        * exist.
        * @param aLabel IN Default label of the Certificate
        *               OUT Unique label that might been changed by user
        * @param aStore Unified Certificate Store
        * @return KErrNone if everything is ok.
        *         KErrCancel if user pressed cancel
        *         Otherwise status of the list operation.
        *
        */
        TInt QueryLabelL( TCertLabel& aLabel, CUnifiedCertStore& aStore );

        /**
        * Queries trust settings from user
        * @param OUT aUids contains user selected uids of
        * the trusted applications
        * @return KErrNone if everything is ok.
        *         KErrCancel if user pressed cancel
        *         Otherwise status of the list operation.
        *
        */
        TInt QueryTrusterUidsL( RArray<TUid>& aUids );

        /**
        * Queries trust site certificate from user
        * @param
        * @return KErrNone if everything is ok.
        *         KErrCancel if user pressed cancel
        *         Otherwise status of the list operation.
        *
        */
        TInt QueryTrustedSiteL();

        /**
        * Updates localised strings to listbox item array
        * @param aApps All trusted applications
        * @param aAppsInItemArray Trusted applications in right order.
        * @param aItemsArray Localised strings in same order.
        * @return void
        *
        */
        void UpdateTrustListboxItemL(
            const RArray<TCertificateAppInfo>& aApps,
            RArray<TCertificateAppInfo>& aAppsInItemArray,
            CDesCArray& aItemsArray ) const;

        /**
        * Divides string to four characters blocks
        * and delimits them with space.
        * @param aInput String to devide
        * @param aOutput Descriptor where result is appended
        * @return void
        */
        void DivideToBlocks( const TDesC8& aInput, TDes& aOutput ) const;

        TBool DoMessageQueryL(
            TInt aDialogResId,
            TInt aHeadingResId,
            TDesC& aMessage );

    private: //data
        TCertificateOwnerType     iCertOwnerType;
        TCertificateFormat        iCertFormat;
        const CCertSaverDocument* iDocument;
        CCertSaverAppUi*          iAppUi;
        CCertSaverSyncWrapper*    iWrapper;    // synchronous wrapper

        // variables for SaveCertL
        CUnifiedCertStore*          iUnifiedCertStore;
        RMPointerArray<CCTCertInfo> iEntries;
        const CCertificate*         iNewCert;
        RArray<TUid>                iTrusterUids;
        RFs& iFs;

        const CCertParser& iParser;

        TInt  iSavedCACertsCount;
        TInt  iSavedKeysCount;
        TInt  iSavedUserCertsCount;
        TBool iKeyAlreadyExists;
        
        TUid  iSelectedKeyStoreToken;
        TUid  iSelectedCertStoreToken;
        TInt  iSelectedKeyStoreIndex;
    };

#endif

// End of File
