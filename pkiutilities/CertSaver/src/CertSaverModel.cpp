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
* Description:   Class that handles user input and launching of dialogs
*
*/


#include <StringLoader.h>           // For loading resource strings
#include <unifiedcertstore.h>       // For saving the certificates
#include <mctwritablecertstore.h>   // For saving the certificates
#include <sysutil.h>                // For FFSSpaceBelowCriticalLevelL(..)
#include <X509CertNameParser.h>     // For default label
#include <x509cert.h>               // For CX509Certificate
#include <wtlscert.h>               // For WTLSCertificate
#include <hash.h>                   // MD5 fingerprint
#include <certificateapps.h>
#include <mctkeystore.h>
#include <securityerr.h>
#include <asnpkcs.h>
#include <unifiedkeystore.h>
#include <x509certext.h>
#include <mctkeystore.h>
#include <TrustedSitesStore.h>
#include <eikenv.h>                 // CEikonEnv
#include <AknUtils.h>               // AknTextUtils

#include <CertSaver.rsg>
#include "CertSaverModel.h"
#include "CertSaverDocument.h"
#include "CertSaverAppUi.h"
#include "CertSaverSyncWrapper.h"
#include "certsaver.hrh"
#include "securityuisvariant.hrh"

#include <hbdevicemessageboxsymbian.h>
#include <hbdevicenotificationdialogsymbian.h>
#include <hbsymbianvariant.h>

#include "SecQueryUi.h"                   // needed for label dialog

// CONSTANTS

const TInt32 KWTLSTrusterUID( 268479059 );
const TInt32 KInternetTrusterUID( 268441661 );
const TInt32 KApplicationControllerTrusterUID( 268452523 );
const TInt32 KJavaInstallTrusterUID( 270506792 );
const TInt32 KOCSPTrusterUID( 268478646 );
const TInt32 KVPNUID( 270498195 );

const TInt KTrusterArrayInitSize( 2 );

const TInt KMaxLengthTextMeassageBody( 5000 );
const TInt KMaxLengthTextDateString( 11 );    // "dd/mm/yyyy0"
const TInt KMaxLengthTextCheckBoxData( 255 );    // "dd/mm/yyyy0"

//Issuer and Owner max visible length
const TInt KMaxLengthTextCertIdentifierVisible( 1000 );
const TInt KAttempts( 3 );

const TInt KFingerprintLength( 50 );
const TInt KTokenLength( 32 );

_LIT( KCertSaverListBoxItemPrefix, "1\t" );
const TInt KItemBufLen = 2 + KMaxName;

_LIT( KPrivateKeyLabel, "PrivateKey " );
const TInt KPrivaKeyLabelLength( 50 );
_LIT( KDateString,"%D%M%Y%1%2%3" );
_LIT( KTimeString,"%-B%:0%J%:1%T%:2%S%:3%+B" );
_LIT( KSpace, " " );
_LIT( KDoubleEnter, "\n\n" );
_LIT( KEnter, "\n" );

const TUid KTrustedServerTokenUid = { 0x101FB66F };
const TUid KFileTokensUid = { 0x101F501A };

_LIT_SECURITY_POLICY_V1( KSymbianKeyStoreMgmtPolicy,
                         VID_DEFAULT, ECapabilityWriteUserData );
_LIT_SECURITY_POLICY_C1( KSymbianKeyStoreUsePolicy, ECapabilityReadUserData );


// ================= MEMBER FUNCTIONS =======================

// Destructor
CCertSaverModel::~CCertSaverModel()
    {
    delete iWrapper;
    iEntries.Close();
    delete iUnifiedCertStore;
    iTrusterUids.Close();
    }

// ----------------------------------------------------------
// CCertSaverModel::CCertSaverModel(
//    const CCertSaverDocument* aDocument, CCertSaverAppUi* aAppUi)
// ----------------------------------------------------------
//
CCertSaverModel::CCertSaverModel(
    CCertSaverAppUi* aAppUi,
    const CCertParser& aParser ):
    iAppUi( aAppUi ), iFs( iAppUi->CoeEnv()->FsSession() ), iParser( aParser ),
    iSavedCACertsCount( 0 ), iSavedKeysCount( 0 ),
    iSavedUserCertsCount( 0 ), iKeyAlreadyExists( EFalse )
    {
    }

// ----------------------------------------------------------
// CCertSaverModel::SaveCertificateL()
// ----------------------------------------------------------
//
void CCertSaverModel::SaveCertificateL(
            const CX509Certificate& aCertificate,
            const TCertificateOwnerType& aOwnerType,
            const TCertificateFormat& aCertFormat )
    {

    iCertOwnerType = aOwnerType;
    iCertFormat = aCertFormat;
    iNewCert = &aCertificate;

    if ( aOwnerType == EPeerCertificate )
        {
        TInt ret = QueryTrustedSiteL();
        if ( ret != KErrNone )
            {
             return;
            }
        }

    SaveCertL();
    }

// ----------------------------------------------------------
// CCertSaverModel::DoSavePrivateKeyL()
//
// ----------------------------------------------------------
//
void CCertSaverModel::DoSavePrivateKeyL( const TDesC8& aKey )
    {

    CheckFSSpaceL( aKey );

    TKeyIdentifier keyIdentifier;
    CDecPKCS8Data* pkcs8Data = TASN1DecPKCS8::DecodeDERL( aKey );
    CleanupStack::PushL( pkcs8Data );
    MPKCS8DecodedKeyPairData* keyPairData = pkcs8Data->KeyPairData();
    keyPairData->GetKeyIdentifierL( keyIdentifier );

    TTime startDate;
    TTime endDate;
    GetKeyValidityPeriodL( startDate, endDate, keyIdentifier );

    TInt err = KErrNone;
    CCTKeyInfo* keyInfo = NULL;
    TBuf<KPrivaKeyLabelLength> keyLabel( KPrivateKeyLabel );
    TKeyUsagePKCS15 keyUsage = KeyUsageL( keyIdentifier, pkcs8Data->Algorithm() );
    CleanupStack::PopAndDestroy( pkcs8Data );
    if (KeyAlreadyExistsL( startDate, endDate, keyIdentifier, keyUsage) )
        {
        // used to leave with error none
        return;
        }

    TInt accessType( 0 );

    accessType |= CCTKeyInfo::EExtractable;

    for ( TInt i = 0; i < KAttempts; i++ )
        {
        CreateKeyLabelL( keyLabel );
        // Should not use hardcoded index
        err = iWrapper->AddKey( 0, aKey, keyUsage, keyLabel,
            accessType, startDate, endDate, keyInfo );
        if ( err != KErrAlreadyExists )
            {
            i = KAttempts;
            }
        keyLabel.Zero();
        }

    TCTTokenObjectHandle handle;
    if ( keyInfo )
        {
        handle = keyInfo->Handle();
        keyInfo->Release();
        keyInfo = NULL;
        }

    switch ( err )
        {
        case KErrNone:
            {
            ++iSavedKeysCount;
            break;
            }
        case KErrKeySize:
        case KErrArgument:
            {
            ShowInformationNoteL( R_CERTSAVER_KEY_TYPE_NOT_SUPPORTED );
            User::Leave( KErrCancel );
            break;
            }
        case KErrKeyValidity:
            {
            ShowInformationNoteL( R_CERTSAVER_QTN_CM_PKCS12_EXPIRED );
            User::Leave( KErrCancel );
            }
        case KErrKeyUsage:
            {
            ShowInformationNoteL( R_CERTSAVER_PRIVATE_KEY_CORRUPTED );
            User::Leave( KErrCancel );
            break;
            }
        case KErrCancel:
        case KErrPermissionDenied:
            {
            ShowInformationNoteL( R_CERTSAVER_PKCS12_DISCARDED );
            User::Leave( KErrCancel );
            break;
            }
        case KErrCorrupt:
        case KErrEof:
            {
            ShowInformationNoteL( R_CERTSAVER_KEYSTORE_CORRUPTED );
            User::Leave( KErrCancel );
            break;
            }
        case KErrAlreadyExists:
            {
            User::Leave( KErrCancel );
            break;
            }
        default:
            {
            User::Leave( err );
            break;
            }
        }

    User::LeaveIfError(
        iWrapper->SetManagementPolicy( handle, KSymbianKeyStoreMgmtPolicy) );
    User::LeaveIfError(
        iWrapper->SetUsePolicy( handle, KSymbianKeyStoreUsePolicy ) );
    }

// ----------------------------------------------------------
// TBool CCertSaverModel::KeyAlreadyExistsL()
//
// ----------------------------------------------------------
//
TBool CCertSaverModel::KeyAlreadyExistsL(
    TTime& aStartDate,
    TTime& aEndDate,
    const TKeyIdentifier& aKeyIdentifier,
    TKeyUsagePKCS15& aKeyUsage )
    {

    TBool ret = EFalse;
    TCTKeyAttributeFilter keyFilter;
    keyFilter.iKeyAlgorithm = CKeyInfoBase::EInvalidAlgorithm;
    keyFilter.iKeyId = aKeyIdentifier;
    keyFilter.iPolicyFilter = TCTKeyAttributeFilter::EManageableKeys;
    keyFilter.iUsage = aKeyUsage;
    RMPointerArray<CCTKeyInfo> keyArray;
    TInt err = iWrapper->ListKeys( keyArray, keyFilter );
    CleanupClosePushL( keyArray );
    switch ( err )
        {
        case KErrNone:
            {
            break;
            }
        case KErrCorrupt:
        case KErrEof:
            {
            ShowInformationNoteL( R_CERTSAVER_KEYSTORE_CORRUPTED );
            User::Leave( KErrCancel );
            }
        default:
            {
            User::Leave( err );
            }
        }
    for ( TInt i = 0; ret == EFalse && i < keyArray.Count(); i++ )
        {
        if ( keyArray[i]->StartDate() == aStartDate &&
             keyArray[i]->EndDate() == aEndDate )
            {
            ret = ETrue;
            iKeyAlreadyExists = ETrue;
            }
        }
    CleanupStack::PopAndDestroy( &keyArray );
    return ret;
    }

// ----------------------------------------------------------
// CCertSaverModel::GetKeyValidityPeriodL()
//
// ----------------------------------------------------------
//
void CCertSaverModel::GetKeyValidityPeriodL(
    TTime& aStartDate,
    TTime& aEndDate,
    const TKeyIdentifier& aKeyIdentifier )
    {


    for ( TInt i = 0; i < iParser.UserCertificates().Count(); i++ )
        {
        const CX509Certificate* cert = iParser.UserCertificates().At( i );
        if ( cert->KeyIdentifierL() == aKeyIdentifier )
            {
            // Associated certificate found
            // In the first round aStartDate and aEndDate is initialised.
            if ( i == 0 || aStartDate > cert->ValidityPeriod().Start() )
                {
                aStartDate = cert->ValidityPeriod().Start();
                }
            if ( i == 0 || aEndDate < cert->ValidityPeriod().Finish() )
                {
                aEndDate = cert->ValidityPeriod().Finish();
                }
            }
        }
    }

// ----------------------------------------------------------
// CCertSaverModel::CreateKeyLabelL()
//
// ----------------------------------------------------------
//
void CCertSaverModel::CreateKeyLabelL( TDes& aLabel )
    {

    TTime time;
    time.UniversalTime();
    TBuf<KPrivaKeyLabelLength> dateBuf;
    time.FormatL( dateBuf, KDateString );
    aLabel.Append( dateBuf );
    dateBuf.Zero();
    aLabel.Append( KSpace );

    time.FormatL( dateBuf, KTimeString );
    aLabel.Append( dateBuf );
    }

// ----------------------------------------------------------
// CCertSaverModel::SavePrivateKeyL()
//
// ----------------------------------------------------------
//
void CCertSaverModel::SavePrivateKeyL()
    {

    if ( iParser.Keys().Count() <= 0 )
        {
        return;
        }
    if ( NULL == iWrapper )
        {
        iWrapper = CCertSaverSyncWrapper::NewL();
        }
    TInt err = iWrapper->InitializeKeyStoreL( iFs );

    TInt keyStoreCount = iWrapper->UnifiedKeyStore().KeyStoreManagerCount();
    if ( keyStoreCount <= 0 )
        {
        User::Leave( KErrCancel );
        }
    else
        {
        for ( TInt i = 0; i < keyStoreCount; ++i )
            {
            TUid uid =
            iWrapper->UnifiedKeyStore().KeyStoreManager(i).Token().TokenType().Type();
            if ( uid == TUid::Uid( KTokenTypeFileKeystore ) )
                // if this is not found, we use the first one,
                // which is already initialised
                {
                iSelectedKeyStore = i;
                }
            }
        }

    for ( TInt i = 0; i < iParser.Keys().Count(); i++ )
        {
        const HBufC8* key = iParser.Keys().At( i );
        TRAP(err, DoSavePrivateKeyL( *key ));
        User::LeaveIfError( err );
        }
    }

// ----------------------------------------------------------
// CCertSaverModel::KeyUsageL(
//    const TKeyIdentifier& aKeyIdentifier, TAlgorithmId aAlgorithm)
// ----------------------------------------------------------
//
TKeyUsagePKCS15 CCertSaverModel::KeyUsageL(
    const TKeyIdentifier& aKeyIdentifier,
    TAlgorithmId aAlgorithm )
    {


    TKeyUsagePKCS15 pkcs15KeyUsage = EPKCS15UsageNone;
    TKeyUsageX509 x509Usage = EX509UsageNone;

    for ( TInt i = 0; i < iParser.UserCertificates().Count(); i++ )
        {
        const CX509Certificate* cert = iParser.UserCertificates().At( i );
        if ( cert->KeyIdentifierL() == aKeyIdentifier )
            {
            const CX509CertExtension* ext = cert->Extension( KKeyUsage );
            if (ext)
                {
                CX509KeyUsageExt* keyUsageExt =
                    CX509KeyUsageExt::NewLC( ext->Data() );
                if ( keyUsageExt->IsSet( EX509DigitalSignature ) )
                  {
                  x509Usage |= EX509UsageDigitalSignature;
                  }
                if ( keyUsageExt->IsSet( EX509NonRepudiation ) )
                  {
                  x509Usage |= EX509UsageNonRepudiation;
                  }
                if ( keyUsageExt->IsSet( EX509KeyEncipherment ) )
                  {
                  x509Usage |= EX509UsageKeyEncipherment;
                  }
                if ( keyUsageExt->IsSet( EX509DataEncipherment ) )
                  {
                  x509Usage |= EX509UsageDataEncipherment;
                  }
                if ( keyUsageExt->IsSet( EX509KeyAgreement ) )
                  {
                  x509Usage |= EX509UsageKeyAgreement;
                  }
                if ( keyUsageExt->IsSet( EX509KeyCertSign ) )
                  {
                  x509Usage |= EX509UsageKeyCertSign;
                  }
                if ( keyUsageExt->IsSet( EX509CRLSign ) )
                  {
                  x509Usage |= EX509UsageCRLSign;
                  }
                if ( keyUsageExt->IsSet( EX509EncipherOnly ) )
                  {
                  x509Usage |= EX509UsageEncipherOnly;
                  }
                if ( keyUsageExt->IsSet( EX509DecipherOnly ) )
                  {
                  x509Usage |= EX509UsageDecipherOnly;
                  }
                CleanupStack::PopAndDestroy( keyUsageExt );
                }
            }
        }

    pkcs15KeyUsage = KeyUsageX509ToPKCS15Private( x509Usage );
    // If any certificate in the file did not include key usage,
    // let's use default values.
    if ( EPKCS15UsageNone == pkcs15KeyUsage )
        {
        switch ( aAlgorithm )
            {
            case ERSA:
                {
                pkcs15KeyUsage |= EPKCS15UsageSignSignRecover;
                pkcs15KeyUsage |= EPKCS15UsageDecryptUnwrap;
                break;
                }
            case EDSA:
                {
                pkcs15KeyUsage |= EPKCS15UsageSignSignRecover;
                break;
                }
            default:
                {
                break;
                }
            }
        }
    return pkcs15KeyUsage;
    }

// ----------------------------------------------------------
// CCertSaverModel::CheckFSSpaceL(
//    const TDesC8& aDataToSave)
// ----------------------------------------------------------
//
void CCertSaverModel::CheckFSSpaceL( const TDesC8& aDataToSave )
    {

    if (SysUtil::FFSSpaceBelowCriticalLevelL( &iFs, aDataToSave.Size() ))
        {
        ShowInformationNoteL(R_CERTSAVER_MEMORY);
        User::Leave( KErrExitApp );
        }
    }
// ----------------------------------------------------------
// CCertSaverModel::SaveCertL()
// Saves certificate
// ----------------------------------------------------------
//
void CCertSaverModel::SaveCertL()
  {

    if ( !CertificateOkL() )
        {
        User::Leave( KErrCancel );
        }
 
    HBufC* message = HBufC::NewLC( KMaxLengthTextMeassageBody );
    TPtr msgPtr = message->Des();
    ConstructMessageL( msgPtr );
    CHbDeviceMessageBoxSymbian::TButtonId selection = 
        CHbDeviceMessageBoxSymbian::QuestionL(msgPtr, KNullDesC, KNullDesC);
    TBool doSave= (selection == CHbDeviceMessageBoxSymbian::EAcceptButton);
    CleanupStack::PopAndDestroy(message);
/*
    if ( doSave && iCertOwnerType == ECACertificate )
        {
        // warn user about security risk
        HBufC* stringHolder = StringLoader::LoadLC( R_CERTSAVER_WARNING_NOTE );
        CHbDeviceMessageBoxSymbian::TButtonId selection = 
            CHbDeviceMessageBoxSymbian::QuestionL(stringHolder->Des(),KNullDesC, KNullDesC);
        CleanupStack::PopAndDestroy(stringHolder);
	    doSave=(selection == CHbDeviceMessageBoxSymbian::EAcceptButton);
        }
*/
    if ( doSave )
        {
        //Check that there still is enough space to store the certificate.
        CheckFSSpaceL( iNewCert->Encoding() );
        DoSaveCertL();
        }
    else
        {
        ShowInformationNoteL(R_CERTSAVER_CERT_DISCARDED);
        User::Leave( KErrCancel );
        }
  }


// ----------------------------------------------------------
// CCertSaverModel::InitCertStoreL()
//
// ----------------------------------------------------------
//
void CCertSaverModel::InitCertStoreL()
    {

    if ( !iUnifiedCertStore )
        {
        TRAPD( status, iUnifiedCertStore = CUnifiedCertStore::NewL( iFs, ETrue ) );
        if ( status != KErrNone )
            {
            ShowInformationNoteL( R_CERTSAVER_ERROR_CACERTS_DB_CORRUPTED );
            User::Leave( KErrExitApp );
            }
        // initialize unified cert store
        status = iWrapper->InitializeCertStore( *iUnifiedCertStore );
        if ( status )
            {
            HandleSaveErrorL( status );
            User::Leave( KErrExitApp );
            }
        }
    }

// ----------------------------------------------------------
// CCertSaverModel::DoSaveCertL()
// Tries to save the certificate to phone memory.
// ----------------------------------------------------------
//
void CCertSaverModel::DoSaveCertL()
    {

    TInt status = KErrNone;
    CCertAttributeFilter* filter = NULL;
    TCertificateFormat certFormat = EX509Certificate;
    HBufC* secondaryName = NULL;
    TCertLabel labelBuf;
    CCertificate* certificate = NULL;
    HBufC* label = NULL;

    if ( NULL == iWrapper )
        {
        iWrapper = CCertSaverSyncWrapper::NewL();
        }
    // Init unified certstore
    InitCertStoreL();

    // Check that certificate doesn't already exist.
    // Fingerprint of certificate is used to confirm this.
    filter = CCertAttributeFilter::NewL();
    filter->SetFormat( iCertFormat );
    filter->SetOwnerType( iCertOwnerType );
    // Delete old array first
    iEntries.Close();
    status = iWrapper->ListCerts( *iUnifiedCertStore, iEntries, *filter );
    delete filter;
    filter = NULL;
    if ( status )
        {
        if ( status != KErrCancel )
            {
            HandleSaveErrorL( status );
            }
        User::Leave( KErrExitApp );
        }

    TBool found = EFalse;

    for ( TInt i = 0; i < iEntries.Count() && !found; ++i )
        {
        iWrapper->Retrieve( *iUnifiedCertStore, *iEntries[i], certificate );
        // Compare fingerprint of listed certificates to
        // fingerprint of new certificate
        // If the certificate is already in CACerts.dat,
        // then don't save it
        if ( iNewCert->Fingerprint() == certificate->Fingerprint() &&
            (( *iEntries[i]).Handle().iTokenHandle.iTokenTypeUid == KFileTokensUid ) )
            {
            found = ETrue;
            }
        delete certificate;
        certificate = NULL;
        }

    // Cancel if certificate already exists.
    if ( found )
        {
        HandleSaveErrorL( KErrAlreadyExists );
        User::Leave( KErrCancel );
        }

    // Create default label from the certificate
    X509CertNameParser::PrimaryAndSecondaryNameL(
        (*(CX509Certificate*)iNewCert), label, secondaryName );
    CleanupStack::PushL( label );
    delete secondaryName;
    secondaryName = NULL;
    labelBuf = label->Des().Left( CERTSAVER_MAX_LABEL_LEN );
    CleanupStack::PopAndDestroy( label );
    label = NULL;
    // Asks unique label from user.
    status = QueryLabelL( labelBuf, *iUnifiedCertStore );
    if ( status )
        {
        if ( status != KErrCancel )
            {
            HandleSaveErrorL( status );
            }
        User::Leave( KErrCancel );
        }

    if ( ECACertificate == iCertOwnerType )
        {
        // Query trusted UIDs
        status = QueryTrusterUidsL( iTrusterUids );
        if ( status )
            {
            HandleSaveErrorL( status );
            User::Leave( KErrCancel );
            }
        }

    // Get interface to writable store.
    TInt certstoreIndex( -1 );
    TInt count = iUnifiedCertStore->WritableCertStoreCount();
    if ( count > 0 )
        {
        for ( TInt i = 0; i < count; i++ )
            {
            MCTWritableCertStore& writableCertStore =
                    iUnifiedCertStore->WritableCertStore( i );

            MCTToken& token = writableCertStore.Token();
            TUid tokenuid = token.Handle().iTokenTypeUid;
            if ( ( tokenuid == KTrustedServerTokenUid ) && 
                 ( iCertOwnerType == EPeerCertificate ) ||
                 ( tokenuid == KFileTokensUid ) && ( iCertOwnerType == ECACertificate ) ||
                 ( tokenuid == KFileTokensUid ) && ( iCertOwnerType == EUserCertificate ) )
                {
                certstoreIndex = i;
                break;
                }
            }

        if ( certstoreIndex < 0 )
            {
            // Couldn't find certificate storage
            ShowInformationNoteL( R_CERTSAVER_ERROR_CACERTS_DB_CORRUPTED );
            User::Leave( KErrExitApp );
            }

        status = iWrapper->AddCert( iUnifiedCertStore->WritableCertStore( certstoreIndex ),
                        labelBuf, iCertFormat, iCertOwnerType, 0, 0, iNewCert->Encoding() );

        if ( ( status == KErrNone ) && ( iCertOwnerType == EPeerCertificate) )
            {
            // Adding certificate succeded. Update trust site storage
            CTrustSitesStore* trustedSitesStore = CTrustSitesStore::NewL();
            CleanupStack::PushL( trustedSitesStore );
            TPtrC8 certBuf = iParser.CertificateBuf();
            TInt err = trustedSitesStore->AddL( certBuf, labelBuf );
            CleanupStack::PopAndDestroy( trustedSitesStore );
            }

        // If error happened, show error note and give up. Otherwise, continue
        HandleSaveErrorL( status );
        if ( status )
            {
            User::Leave( KErrCancel );
            }
        else
            {
            if ( ECACertificate == iCertOwnerType )
                {
                ++iSavedCACertsCount;
                }
            else
                {
                ++iSavedUserCertsCount;
                }
            }
        }
    else
        {
        // If there is none WritableCertStore,
        // then at least cacerts.dat is corrupted.
        ShowInformationNoteL( R_CERTSAVER_ERROR_CACERTS_DB_CORRUPTED );
        User::Leave( KErrExitApp );
        }

    if ( ECACertificate == iCertOwnerType )
        {
        // get just saved certificate
        // first, create a filter
        filter = CCertAttributeFilter::NewL();
        filter->SetLabel( labelBuf );
        filter->SetFormat( certFormat );
        filter->SetOwnerType( ECACertificate );

        // Delete array
        iEntries.Close();
        // then list certificates
        status = iWrapper->ListCerts( *iUnifiedCertStore, iEntries, *filter );
        delete filter;
        filter = NULL;

        // If error happened, show error note and give up. Otherwise, continue
        if ( status )
            {
            HandleSaveErrorL( status );
            User::Leave( KErrExitApp );
            }

        // takes ownership of trusterUids
        status = iWrapper->SetApplicability(
            iUnifiedCertStore->WritableCertStore( certstoreIndex ), *(iEntries[0]), iTrusterUids );

        // If error happened, show error note and give up. Otherwise, continue
        if ( status )
            {
            HandleSaveErrorL( status );
            User::Leave( KErrExitApp );
            }

        // Downloaded certificate is trusted by default
        status = iWrapper->SetTrust(
            iUnifiedCertStore->WritableCertStore(certstoreIndex), *(iEntries[0]), ETrue );
        if ( status )
            {
            HandleSaveErrorL( status );
            }


        } // if ( ECACertificate == iCertOwnerType )
    }
// ----------------------------------------------------------
// CCertSaverModel::QueryLabel()
// Queries label from user and confirms that it doesn't
// already exist.
// ----------------------------------------------------------
//
TInt CCertSaverModel::QueryLabelL( TCertLabel& aLabel, CUnifiedCertStore& aStore )
    {

    CCertAttributeFilter* filter = NULL;
    TInt status = KErrNone;
    RMPointerArray<CCTCertInfo> entries;
    TBool loop = ETrue;

    while ( loop )
        {
    HBufC* labelprompt =  CEikonEnv::Static()->AllocReadResourceLC( R_CERTSAVER_ENTER_LABEL );
    CSecQueryUi* SecQueryUi = CSecQueryUi::NewL();                                       
    TInt queryAccepted = SecQueryUi->SecQueryDialog(labelprompt->Des(), aLabel,
                                                1,KMaxCertLabelLength,          
                                                ESecUiAlphaSupported |          
                                                ESecUiCancelSupported |         
                                                ESecUiEmergencyNotSupported);   
    CleanupStack::PopAndDestroy( labelprompt );
    delete SecQueryUi;  
    SecQueryUi=NULL;

      if ( queryAccepted!=KErrNone )
            {
            // cancel
            ShowInformationNoteL(R_CERTSAVER_CERT_DISCARDED);
            return KErrCancel;
            }
        // Create filter to confirm that label doesn't already exist.
        filter = CCertAttributeFilter::NewL();
        filter->SetLabel( aLabel );

        // then list certificates
        status = iWrapper->ListCerts( aStore, entries, *filter );
        CleanupClosePushL( entries );
        delete filter;

        // If error happened, show error note and give up. Otherwise, continue
        if ( status )
            {
            HandleSaveErrorL( status );
            User::Leave( KErrExitApp );
            }

        if ( entries.Count() )
            {
            // Label already exists. Label is queried again.
            HandleSaveErrorL( KErrBadName );
            }
        else
            {
            // Label didn't exist.
            loop = EFalse;
            }
        CleanupStack::PopAndDestroy( &entries ); // entries
        } // while

    return status;
    }

// ----------------------------------------------------------
// CCertSaverModel::HandleSaveErrorL() const
// Shows a note according to aStatus. aStatus is the status
// of the save operation.
// ----------------------------------------------------------
//
void CCertSaverModel::HandleSaveErrorL( TInt aStatus ) const
    {

    switch ( aStatus )
        {
        case KErrNone:
            {
            ShowInformationNoteL(R_CERTSAVER_ERROR_SAVEOK);
            break;
            }
        case KErrNotSupported:
            {
            ShowInformationNoteL(R_CERTSAVER_ERROR_UNSUPPORTED_CERT);
            break;
            }
        case KErrBadName:
            {
            ShowInformationNoteL(R_CERTSAVER_ERROR_LABEL_ALREADY_EXISTS);
            break;
            }
        case KErrAlreadyExists:
            {
            ShowInformationNoteL(R_CERTSAVER_ERROR_ALREADY_EXISTS);
            break;
            }
        case KErrArgument:
            {
            ShowInformationNoteL(R_CERTSAVER_ERROR_CACERTS_DB_CORRUPTED);
            break;
            }
        default:
            {
            //No error note defined for unknown error.
            User::Leave( aStatus );
            break;
            }
        }   //switch
    }

// ----------------------------------------------------------
// CCertSaverModel::AddToMessageWithStringL() const
// String loaded from resources with StringLoader.
// ----------------------------------------------------------
//
void CCertSaverModel::AddToMessageWithStringL(
    TDes& aMessage, TInt aStringResID, const TDesC& aString ) const
    {

    HBufC* promptPtr = NULL;
    promptPtr = StringLoader::LoadL( aStringResID, aString );
    CleanupStack::PushL( promptPtr );
    TPtrC prompt( promptPtr->Des() );

    aMessage.Append( prompt );

    CleanupStack::PopAndDestroy( promptPtr ); // promptPtr
    }

// ----------------------------------------------------------
// CCertSaverModel::AddToMessageWithStringL() const
// String loaded from resources with StringLoader.
// ----------------------------------------------------------
//
void CCertSaverModel::AddToMessageWithIntL(
    TDes& aMessage,
    TInt aStringResID,
    TInt aInt ) const
    {

    HBufC* promptPtr = NULL;
    promptPtr = StringLoader::LoadL( aStringResID, aInt );
    CleanupStack::PushL( promptPtr );
    TPtrC prompt( promptPtr->Des() );

    aMessage.Append( prompt );
    aMessage.Append( KEnter );

    CleanupStack::PopAndDestroy( promptPtr ); // promptPtr
    }

// ----------------------------------------------------------
// CCertSaverModel::AddToMessageL() const
// String loaded from resources with StringLoader.
// ----------------------------------------------------------
//
void CCertSaverModel::AddToMessageL( TDes& aMessage, TInt aStringResID ) const
    {

    HBufC* promptPtr = NULL;
    promptPtr = StringLoader::LoadL( aStringResID );
    CleanupStack::PushL( promptPtr );
    TPtrC prompt( promptPtr->Des() );

    aMessage.Append( prompt );
    aMessage.Append( KEnter );

    CleanupStack::PopAndDestroy( promptPtr ); // promptPtr
    }

// ----------------------------------------------------------
// CCertSaverModel::ConstructMessageL() const
// Creates the certificate details message shown to the user.
// ----------------------------------------------------------
//
void CCertSaverModel::ConstructMessageL( TDes& aMessage ) const
    {

    HBufC16* issuerName = NULL;
    HBufC16* subjectName = NULL;


    X509CertNameParser::SubjectFullNameL( *((CX509Certificate*)iNewCert),
        subjectName );
    CleanupStack::PushL( subjectName );

    X509CertNameParser::IssuerFullNameL( *((CX509Certificate*)iNewCert),
        issuerName );
    CleanupStack::PushL( issuerName );

    AddToMessageWithStringL( aMessage, R_CERTSAVER_TEXT_LABEL,
        CutCertificateField(*subjectName) );
    AddNewlinesToMessage( aMessage );

    AddToMessageWithStringL( aMessage, R_CERTSAVER_TEXT_ISSUER,
        CutCertificateField( *issuerName ) );
    CleanupStack::PopAndDestroy( 2, subjectName );
    AddNewlinesToMessage( aMessage );

    if ( iCertOwnerType==EUserCertificate )
        {
        AddKeyUsageL( aMessage, *((CX509Certificate*)iNewCert) );
        }

    AddValidityPeriodL( aMessage, *((CX509Certificate*)iNewCert) );

    // SHA-1 fingerprint
    TBuf<KFingerprintLength> divided_fingerprint;
    DivideToBlocks( iNewCert->Fingerprint(), divided_fingerprint );
    AddToMessageWithStringL( aMessage, R_CERTSAVER_TEXT_FINGERPRINT,
        divided_fingerprint );

    AddNewlinesToMessage( aMessage );
    divided_fingerprint.Zero();

    // MD5 fingerprint
    CMD5* md5 = CMD5::NewL();
    CleanupStack::PushL( md5 );
    TPtrC8 MD5fingerprint = md5->Hash( iNewCert->Encoding() );

    // Divide fingerprint to blocks
    DivideToBlocks( MD5fingerprint, divided_fingerprint );
    CleanupStack::PopAndDestroy( md5 );
    AddToMessageWithStringL( aMessage, R_CERTSAVER_TEXT_FINGERPRINT_MD5,
        divided_fingerprint );
    AddNewlinesToMessage( aMessage );
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::AddKeyUsageL(...)
// -----------------------------------------------------------------------------
//
void CCertSaverModel::AddKeyUsageL( TDes& aMessage, const CX509Certificate& aCert ) const
    {

    TKeyUsageX509 x509Usage = EX509UsageNone;
    TKeyUsagePKCS15 pkcs15KeyUsage = EPKCS15UsageNone;
    const CX509CertExtension* ext = aCert.Extension( KKeyUsage );
    if (ext)
        {
        CX509KeyUsageExt* keyUsageExt =
            CX509KeyUsageExt::NewLC( ext->Data() );
        if ( keyUsageExt->IsSet( EX509DigitalSignature ) )
            {
            x509Usage |= EX509UsageDigitalSignature;
            }
        if ( keyUsageExt->IsSet( EX509NonRepudiation ) )
            {
            x509Usage |= EX509UsageNonRepudiation;
            }
        if ( keyUsageExt->IsSet( EX509KeyEncipherment ) )
            {
            x509Usage |= EX509UsageKeyEncipherment;
            }
        if ( keyUsageExt->IsSet( EX509DataEncipherment ) )
            {
            x509Usage |= EX509UsageDataEncipherment;
            }
        if ( keyUsageExt->IsSet( EX509KeyAgreement ) )
            {
            x509Usage |= EX509UsageKeyAgreement;
            }
        if ( keyUsageExt->IsSet( EX509KeyCertSign ) )
            {
            x509Usage |= EX509UsageKeyCertSign;
            }
        if ( keyUsageExt->IsSet( EX509CRLSign ) )
            {
            x509Usage |= EX509UsageCRLSign;
            }
        if ( keyUsageExt->IsSet( EX509EncipherOnly ) )
            {
            x509Usage |= EX509UsageEncipherOnly;
            }
        if ( keyUsageExt->IsSet( EX509DecipherOnly ) )
            {
            x509Usage |= EX509UsageDecipherOnly;
            }
        CleanupStack::PopAndDestroy( keyUsageExt );
        }

    pkcs15KeyUsage = KeyUsageX509ToPKCS15Private( x509Usage );

    TInt usage = 0;
    switch( pkcs15KeyUsage )
        {
        case EPKCS15UsageSignSignRecover:
        case EPKCS15UsageSign:
        case EPKCS15UsageSignDecrypt:
            {
            usage = R_QTN_CM_CLIENT_AUTHENTICATION;
            break;
            }
        case EPKCS15UsageNonRepudiation:
            {
            usage = R_QTN_CM_DIGITAL_SIGNING;
            break;
            }
        default:
            {
            usage = R_QTN_CM_NOT_DEFINED;
            break;
            }
        }
    AddToMessageL( aMessage, R_QTN_CM_KEY_USAGE );
    HBufC* usageString = iAppUi->CoeEnv()->AllocReadResourceLC( usage );
    aMessage.Append( *usageString );
    CleanupStack::PopAndDestroy( usageString );
    AddNewlinesToMessage( aMessage );
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::AddValidityPeriodL(...)
// -----------------------------------------------------------------------------
//
void CCertSaverModel::AddValidityPeriodL(
    TDes& aMessage, const CX509Certificate& aCert ) const
    {

    // Hometime's offset to UTC
    TLocale locale;
    TTimeIntervalSeconds offSet = locale.UniversalTimeOffset();
    AddToMessageL( aMessage, R_CERTSAVER_QTN_CM_VALID_FROM );

    const CValidityPeriod& validityPeriod = aCert.ValidityPeriod();
    TTime startValue = validityPeriod.Start();
    startValue += offSet;
    TBuf<KMaxLengthTextDateString> startString;
    // read format string from AVKON resource
    HBufC* dateFormatString = iAppUi->CoeEnv()->AllocReadResourceLC(
        R_QTN_DATE_USUAL_WITH_ZERO );
    // format the date to user readable format. The format is locale dependent
    startValue.FormatL( startString, *dateFormatString );
    AknTextUtils::DisplayTextLanguageSpecificNumberConversion( startString );
    CleanupStack::PopAndDestroy( dateFormatString ); // dateFormatString
    aMessage.Append( startString );
    AddNewlinesToMessage( aMessage );

    AddToMessageL( aMessage, R_CERTSAVER_QTN_CM_VALID_UNTIL );
    TTime finishValue = validityPeriod.Finish();
    finishValue += offSet;
    TBuf<KMaxLengthTextDateString> finishString;
    // read format string from AVKON resource
    dateFormatString = iAppUi->CoeEnv()->AllocReadResourceLC(
        R_QTN_DATE_USUAL_WITH_ZERO );
    // format the date to user readable format. The format is locale dependent
    finishValue.FormatL( finishString, *dateFormatString );
    AknTextUtils::DisplayTextLanguageSpecificNumberConversion( finishString );
    CleanupStack::PopAndDestroy(dateFormatString); // dateFormatString
    aMessage.Append( finishString );
    AddNewlinesToMessage( aMessage );
    }

// ----------------------------------------------------------
// CCertSaverModel::AddNewlinesToMessage() const
// Adds two new lines to message.
// ----------------------------------------------------------
//
void CCertSaverModel::AddNewlinesToMessage( TDes& aMessage ) const
    {

    aMessage.Append( KDoubleEnter );
    }

// ----------------------------------------------------------
// CCertSaverModel::CertificateSupported() const
// Checks that cert is of supported type.
// ----------------------------------------------------------
//
TBool CCertSaverModel::CertificateSupported() const
    {

    if ( iCertFormat == EX509Certificate &&
       ( iCertOwnerType == ECACertificate ||
         iCertOwnerType == EPeerCertificate ||
         iCertOwnerType == EUserCertificate))
        {
        return ETrue;
        }
    return EFalse;
    }


// ----------------------------------------------------------
// CCertSaverModel::CertificateOkL() const
// Checks that cert is ok. Shows an error note if not.
// If certificate is not valid yet/anymore an error note is shown
// but ETrue is returned so that user can still save the
// certificate.
// ----------------------------------------------------------
//
TBool CCertSaverModel::CertificateOkL() const
    {

    if ( !CertificateSupported() ) 
        {
        ShowInformationNoteL(R_CERTSAVER_ERROR_UNSUPPORTED_CERT);
        return EFalse;
        }
    if ( CertNotValidAnymore() )
        {
        ShowInformationNoteL(R_CERTSAVER_ERROR_CERT_NOT_VALID);
        return ETrue;
        }
    else if ( CertNotValidYet() )
        {
        ShowInformationNoteL(R_CERTSAVER_ERROR_CERT_NOT_VALID_YET);
        }
    return ETrue;
    }

// ----------------------------------------------------------
// CCertSaverModel::CertNotValidAnymore() const
// Checks if cert isn't valid anymore.
// ----------------------------------------------------------
//
TBool CCertSaverModel::CertNotValidAnymore() const
    {

    TTime homeTime;
    homeTime.HomeTime();
    if ( iNewCert->ValidityPeriod().Finish() < homeTime )
        {
        return ETrue;
        }
    return EFalse;
    }

// ----------------------------------------------------------
// CCertSaverModel::CertNotValidYet() const
// Checks if cert isn't valid yet.
// ----------------------------------------------------------
//
TBool CCertSaverModel::CertNotValidYet() const
    {

    TTime homeTime;
    homeTime.HomeTime();
    if ( iNewCert->ValidityPeriod().Start() > homeTime )
        {
        return ETrue;
        }
    return EFalse;
    }

// ----------------------------------------------------------
// CCertSaverModel::ShowInformationNoteL() const
// Creates and shows a confirmation note.
// ----------------------------------------------------------
//
void CCertSaverModel::ShowInformationNoteL( TInt aResourceID ) const
    {

    HBufC* buffer = iAppUi->CoeEnv()->AllocReadResourceLC( aResourceID );
    CHbDeviceMessageBoxSymbian* iMessageBox = CHbDeviceMessageBoxSymbian::NewL(CHbDeviceMessageBoxSymbian::EInformation);
    CleanupStack::PushL(iMessageBox);                                                                                    
    iMessageBox->SetTextL(buffer->Des());                                                                                
    iMessageBox->SetTimeout(6000);                                                                                      
    iMessageBox->ExecL();                                                                                                
    CleanupStack::PopAndDestroy(iMessageBox);                                                                            
    CleanupStack::PopAndDestroy( buffer );      
    }

// ----------------------------------------------------------
// CCertSaverModel::TrimCertificateFields() const
// Trims given descriptor so that everything after and
// including the fourth semicolon (;) is cropped.
// Returns the trimmed certificate field.
// ----------------------------------------------------------
//
TPtrC CCertSaverModel::TrimCertificateFields( TPtrC aField ) const
    {

    TPtrC cutField = CutCertificateField( aField );
    // Find one semicolon at a time and crop the
    // helpField from the left to search for the next semicolon
    TInt position = cutField.Locate( ';' ); // 1st semicolon
    TInt fieldLength = cutField.Length();

    // Need to check that position is not bigger than the length of cutField
    if ( position != KErrNotFound && position < fieldLength )
        {
        // Locate function counts from zero, Mid function
        // counts also from zero, add one to exclude the found semicolon
        TInt totalPosition = position;
        TPtrC field = cutField.Mid(totalPosition + 1);
        position = field.Locate(';'); // 2nd semicolon
        fieldLength = field.Length();

        if ( position != KErrNotFound && position < fieldLength )
            {
            totalPosition += position + 1;
            TPtrC field = cutField.Mid( totalPosition + 1 );
            position = field.Locate( ';' ); // 3rd semicolon
            fieldLength = field.Length();

            if ( position != KErrNotFound && position < fieldLength )
                {
                totalPosition += position + 1;
                TPtrC field = cutField.Mid( totalPosition + 1 );
                position = field.Locate( ';' ); // 4th semicolon
                fieldLength = field.Length();

                if ( position != KErrNotFound && position < fieldLength )
                  {
                  totalPosition += position + 1;
                  // Extract the leftmost part of the data field up to n:th character
                  TPtrC field = cutField.Mid( 0, totalPosition );
                  return field;
                  } // if
                } // if
            } // if
        } // if
    return cutField;
    }

// ----------------------------------------------------------
// CCertSaverModel::CutCertificateField() const
// If given descriptor is larger than defined maximum length
// this function cuts it.
// ----------------------------------------------------------
//
TPtrC CCertSaverModel::CutCertificateField( TPtrC aField ) const
    {

    TInt fieldLength = aField.Length();
    if ( fieldLength >= KMaxLengthTextCertIdentifierVisible )
        {
        TPtrC cutCertLabel = aField.Mid( 0, KMaxLengthTextCertIdentifierVisible );
        return cutCertLabel;
        }
    return aField;
    }

// ----------------------------------------------------------
// CCertSaverModel::QueryTrusterUidsL()
// Does needed tasks to exit.
// ----------------------------------------------------------
//
TInt CCertSaverModel::QueryTrusterUidsL( RArray<TUid>& aUids )
    {

    TInt ret = KErrCancel;
    CDesCArray* itemsArray = new (ELeave) CDesCArrayFlat( KTrusterArrayInitSize );
    CleanupStack::PushL( itemsArray );

    CCertificateAppInfoManager* appInfoManager =
        CCertificateAppInfoManager::NewL( iFs, EFalse );
    CleanupStack::PushL( appInfoManager );
    // copy applications to own array
    const RArray<TCertificateAppInfo>& apps = appInfoManager->Applications();
    RArray<TCertificateAppInfo> appsInItemArray( KTrusterArrayInitSize );
    CleanupClosePushL( appsInItemArray );
    UpdateTrustListboxItemL( apps, appsInItemArray, *itemsArray );

    HBufC* title = CEikonEnv::Static()->AllocReadResourceLC( R_CERTSAVER_SELECT_TRUSTED_APPS);

    RBuf rBuf;     // buffer for items with big enough space
    rBuf.CreateL(KMaxLengthTextMeassageBody);
    CleanupClosePushL(rBuf);
    for(TInt i = 0; i<itemsArray->Count(); i++) 
        rBuf.Append( (*itemsArray)[i] ); 

    CSecQueryUi* SecQueryUi = CSecQueryUi::NewL();     
    TInt saved = SecQueryUi->SecQueryDialog(*title, rBuf,1,KTokenLength,              
                                             ESecUiAlphaSupported | ESecUiCancelSupported |
                                             ESecUiBasicTypeMultiCheck |
                                             ESecUiEmergencyNotSupported);   
    delete SecQueryUi;
    SecQueryUi=NULL;

    if (saved==KErrNone)
        {
        TLex16 lex(rBuf);
        TChar ch;                                        
        TBuf16<KTokenLength> token;          
        TInt  val;
        while((ch = lex.Get()) != 0 ){                   
           while ((ch = lex.Peek()) != '|' && ch!=0) lex.Inc();
           token.Copy(lex.MarkedToken());                
           TLex lexc(token);
           if(lexc.Val(val)!=KErrNone) val=0;
           if(val<=appsInItemArray.Count()-1)
               aUids.Append( appsInItemArray[ val ].Id() );
           lex.Inc();                                    
           lex.Mark();                                   
         }  
        ret = KErrNone;
        }
    else
        {
        ShowInformationNoteL(R_CERTSAVER_CERT_DISCARDED);
        ret = KErrCancel;
        }

    rBuf.Close();
    CleanupStack::PopAndDestroy(&rBuf);
    CleanupStack::PopAndDestroy(title);
    CleanupStack::PopAndDestroy(&appsInItemArray);
    CleanupStack::PopAndDestroy(appInfoManager);
    CleanupStack::PopAndDestroy(itemsArray);
    return ret;
    }

// ----------------------------------------------------------
// CCertSaverModel::QueryTrustedSiteL()
// Shows query for trusted site certificate
// ----------------------------------------------------------
//
TInt CCertSaverModel::QueryTrustedSiteL()
    {

    TInt ret = KErrCancel;
    HBufC* label = NULL;
    HBufC* secondaryName = NULL;
    TCertLabel labelBuf;

    X509CertNameParser::PrimaryAndSecondaryNameL(
        (*(CX509Certificate*)iNewCert), label, secondaryName );
    CleanupStack::PushL( label );
    delete secondaryName;
    secondaryName = NULL;
    labelBuf = label->Des().Left( CERTSAVER_MAX_LABEL_LEN );
    HBufC* prompt = StringLoader::LoadLC( R_CERTSAVER_TRUSTEDSITE_WARNING, labelBuf );
    CHbDeviceMessageBoxSymbian::TButtonId selection =
        CHbDeviceMessageBoxSymbian::QuestionL(prompt->Des(), KNullDesC, KNullDesC);
    if ( selection == CHbDeviceMessageBoxSymbian::EAcceptButton)
        {
        ret = KErrNone;
        }
    else
        {
        ShowInformationNoteL(R_CERTSAVER_CERT_DISCARDED);
        ret = KErrCancel;
        }
    CleanupStack::PopAndDestroy( prompt );
    CleanupStack::PopAndDestroy( label );
    return ret;
    }

// ----------------------------------------------------------
// CCertSaverModel::UpdateTrustListboxItemL()
// Updates trust setting listbox items
// ----------------------------------------------------------
//
void CCertSaverModel::UpdateTrustListboxItemL(
    const RArray<TCertificateAppInfo>& aApps,
    RArray<TCertificateAppInfo>& aAppsInItemArray,
    CDesCArray& aItemsArray ) const
  {

    for ( TInt i = 0; i < aApps.Count(); i++ )
        {
        TCertificateAppInfo appInfo = aApps[ i ];
        TBuf<KItemBufLen> item;
        item.Append( KCertSaverListBoxItemPrefix );
        TBuf<KMaxName> resource;

        switch( appInfo.Id().iUid )
            {
            case KWTLSTrusterUID:
                {
                // X.509 certiticate isn't needed with WTLS
                break;
                }
            case KInternetTrusterUID:
                {
               iAppUi->CoeEnv()->ReadResource(
                resource, R_CERTSAVER_ITEM_MAIL_IMAGE_CONN );
                break;
                }
            case KApplicationControllerTrusterUID:
                {
                // SWI certs are ROM based in platsec world.
                break;
                }
            case KJavaInstallTrusterUID:
                {
                // MIDP certstore is read-only..
                break;
                }
            case KOCSPTrusterUID:
                {
                iAppUi->CoeEnv()->ReadResource( resource, R_CERTSAVER_ITEM_OCSP );
                break;
                }
            case KVPNUID:
                {
                iAppUi->CoeEnv()->ReadResource( resource, R_CERTSAVER_ITEM_VPN );
                break;
                }
            default:
                {
                resource = appInfo.Name();
                break;
                }
            }
        if ( 0 < resource.Length())
            {
            item.Append( resource );
            aItemsArray.AppendL( item );
            aAppsInItemArray.Append( appInfo );
            resource.Zero();
            }
        }
  }

// ---------------------------------------------------------
// CCertManUIViewAuthority::DevideToBlocks
// ---------------------------------------------------------
//
void CCertSaverModel::DivideToBlocks( const TDesC8& aInput, TDes& aOutput ) const
    {

    _LIT( KBlockSeparator, " " );
    const TInt KBlockLength = 2;
    TInt blockIndex = 0;
    for ( TInt j = 0 ; j < aInput.Length() ; j++ )
        {
        if ( blockIndex == KBlockLength )
            {
            aOutput.Append( KBlockSeparator );
            blockIndex = 0;
            }
        aOutput.AppendNumFixedWidthUC( (TUint)(aInput[ j ]), EHex, 2 );
        ++blockIndex;
        }
    }

// ----------------------------------------------------
// CCertSaverModel::SavePKCS12L()
// Saves content of the PKCS#12 file
// ----------------------------------------------------
//
void CCertSaverModel::SavePKCS12L()
    {

    HBufC* message = HBufC::NewLC( KMaxLengthTextMeassageBody );
    TPtr msgPtr = message->Des();
    ConstructPKCS12QueryMsgL(msgPtr, iParser.Keys().Count(),
                            iParser.UserCertificates().Count(),
                            iParser.CACertificates().Count() );

    //TODO: Should be in loc file but hardcoded now
    _LIT(KCheckBoxCaption,"Protect with Password|1");
    TBuf<KMaxLengthTextCheckBoxData> CheckeBoxData(KCheckBoxCaption);

    CSecQueryUi* SecQueryUi = CSecQueryUi::NewL();    
    TInt save = SecQueryUi->SecQueryDialog(*message, CheckeBoxData,1,1,
                                            ESecUiAlphaSupported | ESecUiCancelSupported |
                                            ESecUiBasicTypeCheck | ESecUiEmergencyNotSupported);
    CleanupStack::Pop(message);
    message = NULL;
    delete SecQueryUi;
    SecQueryUi=NULL;

    if ( save!=KErrNone )
        {
        ShowInformationNoteL( R_CERTSAVER_PKCS12_DISCARDED );
        User::Leave( KErrExitApp );
        }
    TInt status = KErrNone;
    // save private keys
   if(CheckeBoxData.Compare(_L("1"))==0)
   {
       TRAP( status, SavePrivateKeyL() );
   }
    // save user certificates if private key was saved.
    if ( ( iSavedKeysCount > 0 || iKeyAlreadyExists ) && iParser.UserCertificates().Count() > 0 )
        {
        for ( TInt i = 0; i < iParser.UserCertificates().Count(); i++ )
            {
            const CX509Certificate* cert = iParser.UserCertificates().At(i);
            iCertOwnerType = EUserCertificate;
            iCertFormat = EX509Certificate;
            iNewCert = cert;
            TRAP( status, SaveCertL() );
            if ( KErrExitApp == status )
                {
                User::Leave( KErrExitApp );
                }
            }
        }
    // save CA certificates
    if ( iParser.CACertificates().Count() > 0 )
        {
        for ( TInt i = 0; i < iParser.CACertificates().Count(); i++ )
            {
            const CX509Certificate* cert = iParser.CACertificates().At( i );
            iCertOwnerType = ECACertificate;
            iCertFormat = EX509Certificate;
            iNewCert = cert;
            TRAP( status, SaveCertL() );
            if ( KErrExitApp == status )
                {
                User::Leave( KErrExitApp );
                }
            }
        }
/*
    if ( iSavedCACertsCount != 0 || iSavedKeysCount != 0
        || iSavedUserCertsCount != 0 )
        {
// show how many have been saved
        HBufC* p = StringLoader::LoadLC( R_CERTSAVER_HEADER_SAVED );             
        message = HBufC::NewLC( KMaxLengthTextMeassageBody );                    
        message->Des().Append(p->Des());                                         
        TPtr msgPtr2 = message->Des();                                           
        ConstructPKCS12QueryMsgL(                                                
            msgPtr2, iSavedKeysCount, iSavedUserCertsCount, iSavedCACertsCount );
        CHbDeviceMessageBoxSymbian::InformationL(message->Des());                
        CleanupStack::PopAndDestroy( message );                                  
        CleanupStack::PopAndDestroy( p );                                        
        }
    else
        {
        // the contents could not be saved is dropped
        ShowInformationNoteL( R_QTN_CM_PKCS12_SAVING_FAILED );
        CleanupStack::PopAndDestroy( message );
        }
*/
    }

// ----------------------------------------------------------
// CCertSaverModel::ConstructPKCS12QueryMsgL() const
// Creates the certificate details message shown to the user.
// ----------------------------------------------------------
//
void CCertSaverModel::ConstructPKCS12QueryMsgL(
    TDes& aMessage,
    TInt aPrivateKeys,
    TInt aUserCerts,
    TInt aCACerts ) const
    {

    if ( aPrivateKeys > 0 )
        {
        if ( aPrivateKeys == 1 )
            {
            AddToMessageL( aMessage, R_CERTSAVER_ONE_PRIVATE_KEY );
            }
        else
            {
            AddToMessageWithIntL( aMessage, R_CERTSAVER_PRIVATE_KEYS, aPrivateKeys );
            }
        }
    if ( aUserCerts > 0 )
        {
        if ( aUserCerts == 1 )
            {
            AddToMessageL(
                aMessage, R_CERTSAVER_ONE_PERSONAL_CERTIFICATE );
            }
        else
            {
            AddToMessageWithIntL(
                aMessage, R_CERTSAVER_PERSONAL_CERTIFICATES, aUserCerts );
            }
        }
    if ( aCACerts > 0 )
        {

        if ( aCACerts == 1 )
            {
            AddToMessageL(
                aMessage, R_CERTSAVER_ONE_AUTHORITY_CERTIFICATE );
            }
        else
            {
            AddToMessageWithIntL(
                aMessage, R_CERTSAVER_AUTHORITY_CERTIFICATES, aCACerts );
            }
        }
    }

// End of File
