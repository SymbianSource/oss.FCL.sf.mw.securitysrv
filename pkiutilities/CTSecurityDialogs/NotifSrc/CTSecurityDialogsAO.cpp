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
* Description:
*
*/


// INCLUDE FILES
#include "CTSecurityDialogsAO.h"
#include "CTSecurityDialogNotifier.h"
#include "CTPinQueryDialog.h"
#include "CTSignTextDialog.h"
#include "CTSelectCertificateDialog.h"
#include "CTQueryDialog.h"
#include "CTPinPinQueryDialog.h"
#include "CTNoTrustQuery.h"
#include "CTInvalidCertNote.h"
#include <PKIDlg.h>
#include <badesca.h>
#include <StringLoader.h>
#include <aknnotewrappers.h>
#include <unifiedcertstore.h>
#include <unifiedkeystore.h>
#include <mctkeystore.h>
#include <cctcertinfo.h>
#include <DocumentHandler.h>
#include <apmstd.h>
#include <DigSigningNote.h>
#include <certmanui.rsg>
#include <X509CertNameParser.h>
#include <x509cert.h>
#include <x500dn.h>
#include <hash.h>
#include <x509certext.h>
#include <TrustedSitesStore.h>
#include <mctwritablecertstore.h>
#include <coreapplicationuisdomainpskeys.h>			// KCoreAppUIsAutolockStatus

// LOCAL CONSTANTS AND MACROS
const TInt KCertArrayGranularity = 3;
const TInt KMaxLengthTextDetailsBody = 5000;
// CertLabel, Issuer, Owner max visible length
const TInt KMaxLengthTextCertLabelVisible = 200;
// "dd/mm/yyyy0"
const TInt KMaxLengthTextDateString = 20;
// HH0
const TInt KMaxLengthTextSerialNumberFormatting = 3;
// Maximum length of a certificate
const TInt KMaxCertificateLength = 5000;

// Maximum length of key store password
const TInt KMaxKeystorePwLength = 6;

const TInt KMinImportKeyPwLen = 1;

const TInt KMaxCNLength = 64;

_LIT(KCertManUIDetailsViewHexFormat, "%02X");

const TInt KFileCertStoreUid( 0x101F501A );
const TInt KWIMCertStoreUid ( 0x101F79D9 );
const TUid KTrustedServerTokenUid = { 0x101FB66F };
const TUid KDeviceCertStoreTokenUid = { 0x101FB668 };

_LIT( KBlockSeparator, " " );
_LIT( KEnter, "\n" );
_LIT( KEnterEnter, "\n\n" );
_LIT( KKeyStoreEnterPwLabel, "Key store passphrase" );
_LIT( KKeyStoreCreatePwLabel, "New key store passphrase" );
_LIT( KKeyStoreImportKeyLabel, "Passphrase of the imported key file" );
_LIT( KKeyStoreExportKeyLabel, "Passphrase of the exported key file" );
_LIT( KPKCS12TokenLabel, "PKCS12");


// ============================ LOCAL FUNCTIONS ===============================

TInt AlgorithmNameResourceId( TAlgorithmId aAlgorithmId )
    {
    TInt resourceId = 0;
    switch( aAlgorithmId )
        {
        case ERSA:
            resourceId = R_TEXT_RESOURCE_DETAILS_VIEW_ALGORITHM_RSA;
            break;
        case EDSA:
            resourceId = R_TEXT_RESOURCE_DETAILS_VIEW_ALGORITHM_DSA;
            break;
        case EDH:
            resourceId = R_TEXT_RESOURCE_DETAILS_VIEW_ALGORITHM_DH;
            break;
        case EMD2:
            resourceId = R_TEXT_RESOURCE_DETAILS_VIEW_ALGORITHM_MD2;
            break;
        case EMD5:
            resourceId = R_TEXT_RESOURCE_DETAILS_VIEW_ALGORITHM_MD5;
            break;
        case ESHA1:
            resourceId = R_TEXT_RESOURCE_DETAILS_VIEW_ALGORITHM_SHA1;
            break;
        case ESHA224:
        case ESHA256:
        case ESHA384:
        case ESHA512:
            resourceId = R_TEXT_RESOURCE_DETAILS_VIEW_ALGORITHM_SHA2;
            break;
        default:
            resourceId = R_TEXT_RESOURCE_DETAILS_VIEW_UNKNOWN;
            break;
        }
    return resourceId;
    }


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::CCTSecurityDialogsAO
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CCTSecurityDialogsAO::CCTSecurityDialogsAO( CCTSecurityDialogNotifier* aNotifier,
        TBool& aDeleted ) :
    CActive( EPriorityStandard ), iNotifier( aNotifier ),
    iNextStep( EOperationCompleted ), iTrustedSiteCertStore( 0 ),
    iHandleIndex( -1 ), iCertPtr( 0, 0 ), iDeleted( aDeleted )
    {
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::ConstructL()
    {
    iDeleted = EFalse;

    iQueryDialog = NULL;
    iQueryDialogDeleted = ETrue;
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CCTSecurityDialogsAO* CCTSecurityDialogsAO::NewL( CCTSecurityDialogNotifier* aNotifier,
        TBool& aDeleted )
    {
    CCTSecurityDialogsAO* self = new( ELeave ) CCTSecurityDialogsAO( aNotifier, aDeleted );

    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();

    return self;
    }


// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::~CCTSecurityDialogsAO
// Destructor
// -----------------------------------------------------------------------------
//
CCTSecurityDialogsAO::~CCTSecurityDialogsAO()
    {
    Cancel();
    delete iText;
    delete iFilter;
    iCertHandleList.Close();
    if ( iCertArray )
        {
        //iCertArray->Reset(); // Should we reset this?
        //delete iCertArray;
        }
    iCertInfos.Close();
    if ( iCertInfo )
        {
        iCertInfo->Release(); // Should we release this?
        }
    iKeyInfos.Close();
    if ( iKeyInfo )
        {
        iKeyInfo->Release(); // Should we release this?
        }

    iServerCerts.ResetAndDestroy();
    delete iCertBuf;
    delete iServerName;
    delete iCertLabel;
    delete iKeyStore;
    delete iCertStore;
    iFs.Close();

    iDeleted = ETrue;
    }


// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::StartLD()
// Handles all the operations calls appropriate handler functionq
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::StartLD(
    const TDesC8& aBuffer,
    TInt aReplySlot,
    const RMessagePtr2& aMessage )
    {
    iOperation = *reinterpret_cast<const TInt *>( aBuffer.Ptr() ) & KSecurityDialogOperationMask;
    iRetry = *reinterpret_cast<const TInt *>( aBuffer.Ptr() ) & EPINValueIncorrect;
    iReplySlot = aReplySlot;
    iMessagePtr = aMessage;

    WIMSECURITYDIALOGS_WRITE_FORMAT( "CCTSecurityDialogsAO::StartLD iOperation=%d", iOperation );

    MapTlsProviderOperation( iOperation );

    switch ( iOperation )
        {
        case EEnablePIN:  // flow thru
        case EDisablePIN: // flow thru
        case EEnterPIN:
        case EChangePIN:
            {
            TPINInput PINInput = reinterpret_cast<const TPINInput&>( *aBuffer.Ptr() );
            iPIN = PINInput.iPIN;
            DoHandlePinOperationL();
            iStatus = KRequestPending;
            SetActive();
            break;
            }
        case EUnblockPIN:
            {
            TUnblockPINInput unblockPINInput = reinterpret_cast<const TUnblockPINInput&>( *aBuffer.Ptr() );
            iPIN = unblockPINInput.iPIN;
            iUnblockPIN = unblockPINInput.iUnblockPIN;
            DoHandleUnblockPinOperationL();
            iStatus = KRequestPending;
            SetActive();
            break;
            }

        case EPINBlocked:
            {
            TPINInput PINInput = reinterpret_cast<const TPINInput&>( *aBuffer.Ptr() );
            iPIN = PINInput.iPIN;
            DoHandleMessageL( EErrorPinCodeBlocked, iPIN.iPINLabel, iPIN.iTokenLabel, 0, 0 );
            iMessagePtr.Complete( KErrNone );
            break;
            }
        case ETotalBlocked:
            {
            TPINInput PINInput = reinterpret_cast<const TPINInput&>( *aBuffer.Ptr() );
            iPIN = PINInput.iPIN;
            DoHandleMessageL( EErrorPukCodeBlocked, iPIN.iPINLabel, iPIN.iTokenLabel, 0, 0 );
            iMessagePtr.Complete( KErrNone );
            break;
            }
        case ESignText:
        case EUserAuthenticationText:
        case EUserAuthentication:
            {
            iSignInput = reinterpret_cast<const TSignInput&>( *aBuffer.Ptr() );
            DoHandleSignTextL( aBuffer );
            break;
            }
        case ECreateCSR:
            {
            DoHandleCSRL( aBuffer.Mid( sizeof(TInt) ) );
            break;
            }
        case EDeleteCert:
        case ECertDetails:
            {
            DoHandleCertDetailsL( aBuffer.Mid( sizeof(TInt) ) );
            break;
            }
        case ESaveCert:
            {
            DoHandleSaveCertL( aBuffer );
            break;
            }
        case EUnblockPINInClear:
            {
            User::Leave( KErrNotSupported );
            break;
            }
        case EPinCodeBlockedInfo:
            {
            TPtrC8 pinLabelPtr = aBuffer.Mid( sizeof(TInt) + 1 );
            TPINLabel pinLabel;
            pinLabel.Copy( pinLabelPtr );
            HBufC* text = iNotifier->LoadResourceStringLC( iOperation, pinLabel );

            CCTSignTextDialog::RunDlgLD( R_WIM_UNBLOCK_INFO_DIALOG, *text, iStatus, iRetValue );
            CleanupStack::PopAndDestroy( text );
            iStatus = KRequestPending;
            SetActive();
            break;
            }
        case EServerAuthenticationFailure:
            {
            DoHandleServerAuthFailL( aBuffer );
            break;
            }
        case MPKIDialog::ESignatureRequested:
            {
            DoHandleMessageL( EInfoSignTextRequested, KNullDesC, KNullDesC, 1, KMaxTInt );
            iRetValue = ETrue;
            HandleResponseAndCompleteL();
            break;
            }
        case MPKIDialog::ESignatureDone:
            {
            DoHandleMessageL( EInfoSignTextDone, KNullDesC, KNullDesC, 1, KMaxTInt );
            iRetValue = ETrue;
            HandleResponseAndCompleteL();
            break;
            }
        case MPKIDialog::ESigningCancelled:
            {
            DoHandleMessageL( EInfoSignTextCancelled, KNullDesC, KNullDesC, 1, KMaxTInt );
            iRetValue = ETrue;
            HandleResponseAndCompleteL();
            break;
            }
        case MPKIDialog::ENoMatchCert:
            {
            DoHandleMessageL( ENoMatchingPersonalCert, KNullDesC, KNullDesC, 1, KMaxTInt );
            break;
            }
        case MPKIDialog::ENoSecurityModule:
            {
            DoHandleMessageL( EErrorWimNotAvailable, KNullDesC, KNullDesC, 1, KMaxTInt );
            iRetValue = ETrue;
            HandleResponseAndCompleteL();
            break;
            }
        case MPKIDialog::EInternalError:
            {
            DoHandleMessageL( EErrorInternal, KNullDesC, KNullDesC, 1, KMaxTInt );
            iRetValue = ETrue;
            HandleResponseAndCompleteL();
            break;
            }
        case ESaveReceipt:
            {
            SaveReceiptL( aBuffer.Mid( sizeof(TInt) ) );
            break;
            }
        default:
            User::Panic( _L("CTestSecDlgNotifier"), 0 );
            break;
        }

    // Note that CCTSecurityDialogsAO::StartLD() must complete the given message and
    // delete itself when ready. However, there may be several steps before it can be
    // deleted. CCTSecurityDialogsAO::HandleResponseAndCompleteL() completes the given
    // message and deletes CCTSecurityDialogsAO object. Hence, every operation handling
    // function called above must call HandleResponseAndCompleteL() when it is ready.
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::DoHandleServerAuthFailL(const TDesC8& aBuffer)
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::DoHandleServerAuthFailL( const TDesC8& aBuffer )
    {
    CServerAuthenticationFailureInput* srvAuthFail =
        CServerAuthenticationFailureInput::NewLC( aBuffer );
    TPtrC8 cert;
    TPtrC8 serverName;

    srvAuthFail->GetEncodedCert( cert );
    srvAuthFail->GetServerName( serverName );
    iAuthFailReason = srvAuthFail->FailureReason();
    CleanupStack::PopAndDestroy( srvAuthFail );

    iServerName = HBufC::NewL( serverName.Length() );
    iServerName->Des().Copy( serverName );

    HBufC* cn = NULL;
    iCertLabel = NULL;
    // Site will be checked later. For now it is not trusted
    iTrustedSite = EFalse;


    CX509Certificate* serverCert = CX509Certificate::NewLC( cert );
    // Retrieve subject name
    const CX500DistinguishedName& dName = serverCert->SubjectName();
    // Retrieve CN
    cn = dName.ExtractFieldL( KX520CommonName );
    if ( cn != NULL )
        {
        // commonName exist in the certificate.
        // Check the length of CN. RFC 3280 states
        // that max length of CN is 64.
        if ( cn->Length() <= KMaxCNLength )
            {
            iCertLabel = HBufC::NewL( cn->Length() );
            iCertLabel->Des().Append( cn->Des() );
            }
        }

    if ( iCertLabel == NULL )
        {
        // No or invalid commonName. Use domain name as label
        iCertLabel = HBufC::NewL( iServerName->Length() );
        iCertLabel->Des().Append( iServerName->Des() );
        }

    CleanupStack::PopAndDestroy( serverCert );

    iCertBuf = HBufC8::NewL( cert.Length() );
    *iCertBuf = cert;

    InitCertStoreL();
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::DoHandleCertDetailsL(const TDesC8& aBuffer)
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::DoHandleCertDetailsL( const TDesC8& aBuffer )
    {
    TCTTokenObjectHandle tokenHandle =
        reinterpret_cast<const TCTTokenObjectHandle&>( *aBuffer.Ptr() );

    iTokenHandle.iTokenHandle.iTokenTypeUid = tokenHandle.iTokenHandle.iTokenTypeUid;
    iTokenHandle.iTokenHandle.iTokenId = tokenHandle.iTokenHandle.iTokenId;
    iTokenHandle.iObjectId = tokenHandle.iObjectId;

    InitCertStoreL();
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::DoHandleSaveCertL(const TDesC8& aBuffer)
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::DoHandleSaveCertL( const TDesC8& aBuffer )
    {
    TSaveCertInput saveCertInput =
        reinterpret_cast<const TSaveCertInput&>( *aBuffer.Ptr() );

    TInt start = sizeof( TSaveCertInput );
    delete iCertBuf;
    iCertBuf = NULL;
    iCertBuf = HBufC8::NewL( saveCertInput.iDataSize );
    const TUint8* certPtr = reinterpret_cast<const TUint8*>( aBuffer.Mid(start).Ptr() );
    iCertBuf->Des().Append( certPtr, saveCertInput.iDataSize );
    ShowCertDialogL();
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::DoHandleCSRL(const TDesC8& aBuffer)
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::DoHandleCSRL( const TDesC8& aBuffer )
    {
    WIMSECURITYDIALOGS_ENTERFN("CCTSecurityDialogsAO::DoHandleCSRL");
    TInt length = reinterpret_cast<const TInt&>( *aBuffer.Ptr() );
    TInt start = sizeof( TInt ) + 1;
    iText = HBufC::NewL( length );
    iText->Des().Copy( aBuffer.Mid( start, length ) );

    start = start + length;

    const TUint8* bufferPtr = aBuffer.Ptr();
    bufferPtr += start;

    TCTTokenObjectHandle tokenHandle;
    TCTTokenObjectHandle* tokenHandlePtr = &tokenHandle;
    TUint8* copyPtr = reinterpret_cast< TUint8* >( tokenHandlePtr );
    for( TInt i = 0; i < 12; i++ )
        {
        copyPtr[ i ] = bufferPtr[ i ];
        }

    iTokenHandle.iTokenHandle.iTokenTypeUid = tokenHandle.iTokenHandle.iTokenTypeUid;
    iTokenHandle.iTokenHandle.iTokenId = tokenHandle.iTokenHandle.iTokenId;
    iTokenHandle.iObjectId = tokenHandle.iObjectId;

    InitKeyStoreL();
    iNextStep = EGetKeyInfoByHandle;
    WIMSECURITYDIALOGS_LEAVEFN("CCTSecurityDialogsAO::DoHandleCSRL");
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::DoHandleSignTextL()
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::DoHandleSignTextL( const TDesC8& aBuffer )
    {
    TInt arraySize = sizeof( TCTTokenObjectHandle ) * iSignInput.iCertHandleArrayTotal;
    TInt start = sizeof( TSignInput );
    TInt end = start + arraySize;
    for (; start < end; start+=sizeof( TCTTokenObjectHandle ))
        {
        TCTTokenObjectHandle handle =
            reinterpret_cast<const TCTTokenObjectHandle&>(*aBuffer.Mid(start).Ptr());
        User::LeaveIfError( iCertHandleList.Append( handle ) );
        }
    start = end;
    HBufC* textToSign = NULL;
    if ( iOperation != EUserAuthentication )
        {
        textToSign = HBufC::NewLC( iSignInput.iVariableDataSize / 2 ); // 8 bit -> 16 bit descriptor
        //textToSign->Des().Copy(aBuffer.Mid(start));
        const TUint16* textToSignPtr = reinterpret_cast<const TUint16*>( aBuffer.Mid(start).Ptr() );
        textToSign->Des().Append( textToSignPtr, (iSignInput.iVariableDataSize / 2) );
        }

    if ( iCertHandleList.Count() > 0 )
        {
        iNextStep = EOperationSignTextShown;
        }
    else
        {
        iNextStep = EOperationCompleted;
        iCertHandleList.Close();
        }
    switch( iOperation )
        {
        case EUserAuthenticationText:
            {
            CCTSignTextDialog::RunDlgLD( R_WIM_USERAUTHTEXT_DIALOG,
                *textToSign, iStatus, iRetValue );
            break;
            }
        case EUserAuthentication:
            {
            break;
            }
        case ESignText:
            {
            CCTSignTextDialog::RunDlgLD( R_WIM_SIGNTEXT_DIALOG,*textToSign, iStatus, iRetValue );
            break;
            }
        default:
            {
            __ASSERT_DEBUG( EFalse,
                _L( "CCTSecurityDialogsAO::DoHandleSignTextL: Invalid operation" ) );
            break;
            }
        }
    iStatus = KRequestPending;
    SetActive();
    if ( iOperation == EUserAuthentication )
        {
        TRequestStatus* status = &iStatus;
        User::RequestComplete( status, KErrNone );
        }
    else
        {
        CleanupStack::PopAndDestroy( textToSign );
        }
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::DoHandleUnblockPinOperationL()
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::DoHandleUnblockPinOperationL()
    {
    if ( iRetry ) // Is this new try?
      {
        // Previous attempt was failed
        DoHandleMessageL( EErrorPukCodeIncorrect, KNullDesC, KNullDesC, 0, 0 );
      }
    // Ask the PUK code
    // The label is iPIN instead of iUnblockPIN, since we need to show to
    // the user which PIN to unblock
    DoHandleMessageL( EEnterPukNR, iPIN.iPINLabel,
        iPIN.iTokenLabel, iUnblockPIN.iMinLength, iUnblockPIN.iMaxLength );
    iNextStep = EEnterNewPIN;
    iMultiLineDlgType = EEnterNewPinNR;
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::DoHandlePinOperationL()
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::DoHandlePinOperationL()
    {
    if ( iRetry ) // Is this new try?
      {
        // Previous attempt was failed
        DoHandleMessageL( EErrorPinCodeIncorrect, KNullDesC, KNullDesC, 0, 0 );
      }
    // Ask the PIN code or PUK code
    if ( iPIN.iPINLabel == KKeyStoreEnterPwLabel )
        {
        HBufC* header = StringLoader::LoadLC(
            R_QTN_CM_HEADING_PHONE_KEYSTORE, CEikonEnv::Static() );
        iPIN.iMinLength = KMaxKeystorePwLength;
        DoHandleMessageL( EEnterKeyStorePw, KNullDesC, *header,
            iPIN.iMinLength, iPIN.iMaxLength );
        iMultiLineDlgType = EEnterNewKeyStorePw;
        CleanupStack::PopAndDestroy( header );
        }
    else if ( iPIN.iPINLabel == KKeyStoreImportKeyLabel )
        {
        iPIN.iMinLength = KMaxKeystorePwLength;
        DoHandleMessageL( EImportKeyPw, KNullDesC, KNullDesC,
            KMinImportKeyPwLen, iPIN.iMaxLength );
        iMultiLineDlgType = EEnterNewKeyStorePw;
        }
    else if ( iPIN.iPINLabel == KKeyStoreExportKeyLabel )
        {
        DoHandleMultilinePinQueryL( EExportKeyPw );
        iNextStep = EVerifyPINs;
        }
    else if ( iPIN.iPINLabel == KKeyStoreCreatePwLabel )
        {
        iPIN.iMinLength = KMaxKeystorePwLength;
        DoHandleMessageL( EInfoPwCreating, KNullDesC, KNullDesC, 0, 0 );
        DoHandleMultilinePinQueryL( EEnterNewKeyStorePw );
        iNextStep = EVerifyPINs;
        }
    else if ( iPIN.iTokenLabel == KPKCS12TokenLabel )
        {
        DoHandleMessageL( EEnterPw, iPIN.iPINLabel, KNullDesC,
          iPIN.iMinLength, iPIN.iMaxLength );
        }
    else
        {
        DoHandleMessageL( EEnterPinNR, iPIN.iPINLabel, iPIN.iTokenLabel,
          iPIN.iMinLength, iPIN.iMaxLength );
        iMultiLineDlgType = EEnterNewPinNR;
        }
    if ( EChangePIN == iOperation )
        {
        iNextStep = EEnterNewPIN;
        }
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::DoHandleMultilinePinQueryL()
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::DoHandleMultilinePinQueryL( const TInt& aDlgType )
    {
    iMultiLineDlgType = aDlgType;
    HBufC* dlgText1 = NULL;
    HBufC* dlgText2 = NULL;
    CCTPinPinQueryDialog* dlg = NULL;
    TDialogType dlgType = ( TDialogType )aDlgType;
    if ( EEnterNewKeyStorePw == dlgType )
        {
        dlgText1 = iNotifier->LoadResourceStringLC( dlgType, KNullDesC );
        dlgText2 = iNotifier->LoadResourceStringLC( EVerifyKeyStorePw, KNullDesC );
        dlg = CCTPinPinQueryDialog::NewL( *dlgText1, *dlgText2, iPINValue2,
            iPINValueVerify, iPIN.iMinLength, iPIN.iMaxLength, iRetValue );
        dlg->RunDlgLD( iStatus, R_WIM_PWPW_QUERY_DIALOG );
        CleanupStack::PopAndDestroy( 2, dlgText1 ); // dlgText1, dlgText2
        }
    else if ( EExportKeyPw == dlgType )
        {
        dlgText1 = iNotifier->LoadResourceStringLC( dlgType, KNullDesC );
        dlgText2 = iNotifier->LoadResourceStringLC( EVerifyKeyStorePw, KNullDesC );
        dlg = CCTPinPinQueryDialog::NewL( *dlgText1, *dlgText2, iPINValue2,
            iPINValueVerify, iPIN.iMinLength, iPIN.iMaxLength, iRetValue );
        dlg->RunDlgLD( iStatus, R_WIM_PWPW_QUERY_DIALOG );
        CleanupStack::PopAndDestroy( 2, dlgText1 ); // dlgText1, dlgText2
        }
    else // Enter new PIN
        {
        dlgText1 = iNotifier->LoadResourceStringLC( dlgType, iPIN.iPINLabel );
        dlgText2 = iNotifier->LoadResourceStringLC( EVerifyPinNR, iPIN.iPINLabel );
        dlg = CCTPinPinQueryDialog::NewL( *dlgText1, *dlgText2, iPINValue2,
            iPINValueVerify, iPIN.iMinLength, iPIN.iMaxLength, iRetValue );
        dlg->RunDlgLD( iStatus, R_WIM_PINPIN_QUERY_DIALOG );
        CleanupStack::PopAndDestroy( 2, dlgText1 ); // dlgText1, dlgText2
        }
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::DoHandleMessageL()
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::DoHandleMessageL(
    const TInt& aDlgType,
    const TDesC& aDynamicText,
    const TDesC& aPinQueryHeading,
    const TInt aMinLength,
    const TInt aMaxLength )
    {
  TDialogType dlgType = ( TDialogType )aDlgType;

  HBufC* dlgText = iNotifier->LoadResourceStringLC( dlgType, aDynamicText, aPinQueryHeading );

  TDialogTypeItem item = iNotifier->GetDialogTypeItem( dlgType );

  CAknResourceNoteDialog* dlg = NULL;

  TInt resource = 0;

  switch ( item.iNoteType )
        {
        case EInfoNote:
            {
            dlg = new ( ELeave ) CAknInformationNote( ETrue );
            break;
            }

        case EErrorNote:
            {
            dlg = new ( ELeave ) CAknErrorNote( ETrue );
            break;
            }
        case EConfirmationNote:
            {
            dlg = new ( ELeave ) CAknConfirmationNote( ETrue );
            break;
            }
        case EInfoDialog:
            {
            CCTQueryDialog::RunDlgLD( iStatus,
                                        iRetValue,
                                        *dlgText, item.iSoftKeyResource,
                                        ECTInfoDialog );
            iStatus = KRequestPending;
            SetActive();
            break;
            }
        case EEnterPwPwDialog:
            {
            CCTPinPinQueryDialog* dialog =
                CCTPinPinQueryDialog::NewL( *dlgText, *dlgText,
                iPINValue2, iPINValueVerify, aMinLength, aMaxLength, iRetValue );
            dialog->RunDlgLD( iStatus, R_WIM_PWPW_QUERY_DIALOG );
            break;
            }
        case EEnterPinPinCodeDialog:
            {
            CCTPinPinQueryDialog* dialog =
                CCTPinPinQueryDialog::NewL( *dlgText, *dlgText,
                iPINValue2, iPINValueVerify, aMinLength, aMaxLength, iRetValue );
            dialog->RunDlgLD( iStatus, R_WIM_PINPIN_QUERY_DIALOG );
            break;
            }
        case EEnterPwDialog:
        case EEnterPinNRCodeDialog:
        case EVerifyPinNRCodeDialog:
            {
            if ( item.iNoteType == EEnterPwDialog )
                {
                resource = R_WIM_PASSWORD_QUERY_DIALOG;
                }
            else
                {
                resource = R_WIM_PIN_QUERY_DIALOG;
                }

            TPINValue* pinValue = NULL;
            switch ( aDlgType )
                {
                case EEnterPinNR: // fall thru
                case EEnterPukNR:
                case EEnterKeyStorePw:
                case EImportKeyPw:
                    {
                    pinValue = &iPINValue1;
                    break;
                    }
                case EEnterNewPinNR:
                case EEnterNewKeyStorePw:
                case EExportKeyPw:
                    {
                    pinValue = &iPINValue2;
                    break;
                    }
                case EVerifyPinNR:
                    {
                    pinValue = &iPINValueVerify;
                    break;
                    }
                default:
                    {
                    User::Panic(_L("CSecDlgNotifier"), 0);
                    }
                }
               iPinQueryDialogDeleted = EFalse;
                CCTPinQueryDialog::RunDlgLD( iStatus,
                                        *dlgText,
                                        *pinValue,
                                        aMinLength,
                                        aMaxLength,
                                        iRetValue,
                                        resource,
                                        iPinQueryDialog,
                                        iPinQueryDialogDeleted );
                break;
            }
    default:
            {
            break;
            }

      }

  if ( dlg )
      {
      dlg->ExecuteLD( *dlgText );
      dlg = NULL;
      }

    CleanupStack::PopAndDestroy( dlgText ); // dlgText
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::RunError(TInt aError)
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CCTSecurityDialogsAO::RunError( TInt aError )
    {
    iMessagePtr.Complete( aError );
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::InitCertStoreL()
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::InitCertStoreL()
    {
    switch(iOperation)
        {
        case ESignText:
        case EUserAuthenticationText:
        case EUserAuthentication:
            {
            iNextStep = EOperationRetrieveCertInfos;
            break;
            }
        case EDeleteCert:
        case ECertDetails:
            {
            iNextStep = EGetCertInfo;
            break;
            }
        case EServerAuthenticationFailure:
            {
            iNextStep = EProcessTrustedSite;;
            break;
            }
        default:
            {
            User::Panic( _L("CTestSecDlgNotifier"), 0 );
            }
        }

    if ( iCertStore == NULL )
        {
        TInt err = KErrNone;
        err = iFs.Connect();

        if( err != KErrNone && err != KErrAlreadyExists )
            {
            User::Leave( KErrGeneral );
            }

        iCertStore = CUnifiedCertStore::NewL( iFs, ETrue );
        iCertArray = new (ELeave) CDesCArrayFlat( KCertArrayGranularity );
        iCertStore->Initialize( iStatus );
        iStatus = KRequestPending;
        SetActive();
        }
    else
        {
        iStatus = KRequestPending;
        SetActive();
        TRequestStatus* status = &iStatus;
        User::RequestComplete( status, KErrNone );
        }
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::InitKeyStoreL()
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::InitKeyStoreL()
    {
    if ( NULL == iKeyStore )
        {
        TInt err = KErrNone;
        err  = iFs.Connect();
        if( err != KErrNone && err != KErrAlreadyExists )
            {
            User::Leave( KErrGeneral );
            }

        iKeyStore = CUnifiedKeyStore::NewL( iFs );
        iKeyStore->Initialize( iStatus );
        iStatus = KRequestPending;
        SetActive();
        }
    else
        {
        iStatus = KRequestPending;
        SetActive();
        TRequestStatus* status = &iStatus;
        User::RequestComplete( status, KErrNone );
        }
    }
// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::GetKeyInfosL()
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::GetKeyInfosL()
    {
    if ( NULL == iKeyStore )
        {
        iStatus = KRequestPending;
        SetActive();
        TRequestStatus* status = &iStatus;
        User::RequestComplete( status, KErrNone );
        }
    else
        {
        iStatus = KRequestPending;
        iKeyFilter.iKeyId = iCertInfo->SubjectKeyId();
        iKeyStore->List( iKeyInfos, iKeyFilter, iStatus );
        SetActive();
        }
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::GetKeyInfoByHandleL()
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::GetKeyInfoByHandleL()
    {
    if ( NULL == iKeyStore )
        {
        User::Panic( _L("CCTSecurityDialogsAO::GetKeyInfoByHandleL()"), 0 );
        }
    else
        {
        iStatus = KRequestPending;
        iKeyStore->GetKeyInfo( iTokenHandle, iKeyInfo, iStatus );
        SetActive();
        }
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::RunL()
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::RunL()
    {
    WIMSECURITYDIALOGS_WRITE_FORMAT( "CCTSecurityDialogsAO::RunL, iStatus %d", iStatus.Int() );
    WIMSECURITYDIALOGS_WRITE_FORMAT( "  iNextStep %d", iNextStep );

    if ( iStatus != KErrNone )
        {
        User::Leave( iStatus.Int() );
        } // if

    switch ( iNextStep )
      {
      case EOperationCompleted:
        {
        WIMSECURITYDIALOGS_WRITE( "EOperationCompleted" );
        HandleResponseAndCompleteL();
        break;
        }
      case ECheckServerCerts:
        {
        WIMSECURITYDIALOGS_WRITE( "ECheckServerCerts" );

        TBool foundCert( EFalse );

        if ( iServerCerts.Count() )
            {
            // Found certificates in the trusted site certificate storage.
            // Compare them with the one received from TlsProvider
            iCertPtr.Set( iCertBuf->Des() );
            CX509Certificate* serverCert = CX509Certificate::NewLC( iCertPtr );

            for (TInt i=0; i<iServerCerts.Count(); i++ )
                {
                TPtr8 certPtr( iServerCerts[i]->Des() );

                CX509Certificate* cert = CX509Certificate::NewLC( certPtr );

                // Compare fingerprints
                if ( cert->Fingerprint() == serverCert->Fingerprint() )
                    {
                    // Fingerprints match
                    foundCert = ETrue;
                    }
                CleanupStack::PopAndDestroy( cert );
                }
             CleanupStack::PopAndDestroy( serverCert );
             if ( foundCert )
                {
                // Found matching certificate. Complete the operation
                iRetValue = EServerCertAcceptedPermanently;
                iNextStep = EOperationCompleted;
                HandleResponseAndCompleteL();
                }
            }

            if ( !foundCert )
                {
                // Couldn't find matching certificate. Prompt user
                ShowNoTrustDialogL();
                }

        break;
        }
      case EProcessTrustedSite:
        {
        WIMSECURITYDIALOGS_WRITE( "EProcessTrustedSite" );
        TInt count = iCertStore->WritableCertStoreCount();
        for ( TInt i = 0; i < count; i++ )
            {
            MCTWritableCertStore *certstore = &iCertStore->WritableCertStore( i );
            MCTToken& token = certstore->Token();
            TUid tokenuid = token.Handle().iTokenTypeUid;
            if ( tokenuid == KTrustedServerTokenUid )
                {
                iTrustedSiteCertStore = certstore;
                }
            }

        CTrustSitesStore* trustedSitesStore = CTrustSitesStore::NewL();
        CleanupStack::PushL( trustedSitesStore );

        // Find out whether or not site associated with certificate is trusted
        iTrustedSite = trustedSitesStore->IsTrustedSiteL( *iCertBuf, *iServerName );

        if ( iTrustedSite )
            {
            TBool allowOutOfDate = trustedSitesStore->IsOutOfDateAllowedL(*iCertBuf, *iServerName);

            if (!allowOutOfDate)
                {
                CX509Certificate* cert = CX509Certificate::NewLC( iCertBuf->Des() );

                const CValidityPeriod& validityPeriod = cert->ValidityPeriod();
                const TTime& startValue = validityPeriod.Start();
                const TTime& finishValue = validityPeriod.Finish();
                TTime current;
                current.UniversalTime();

                // First check certificate validity period
                if ( ( startValue > current ) || ( finishValue < current ) )
                    {
                    iTrustedSite = EFalse;
                    }

                CleanupStack::PopAndDestroy( cert );
                }
            }
        CleanupStack::PopAndDestroy( trustedSitesStore );

        if ( iTrustedSite )
            {
            // Site is trusted. Next step is to check that server
            // certificate is in the trusted site certificate storage
            iNextStep = ERetrieveServerCerts;
            iStatus = KRequestPending;
            SetActive();
            TRequestStatus* status = &iStatus;
            User::RequestComplete( status, KErrNone );
            }
         else
            {
            // Site is not trusted. Prompt user
            ShowNoTrustDialogL();
            }

        break;
        }

      case ERetrieveServerCerts:
        {
        WIMSECURITYDIALOGS_WRITE( "ERetrieveServerCerts" );
        if ( iHandleIndex == -1 )
            {
                iFilter = CCertAttributeFilter::NewL();
                iFilter->SetOwnerType( EPeerCertificate );
                if ( iTrustedSiteCertStore )
                    {
                    // Thet the list of all certificates from Trusted site certificate
                    // storage
                    iTrustedSiteCertStore->List( iCertInfos, *iFilter, iStatus );
                    iNextStep = ERetrieveServerCerts;
                    iStatus = KRequestPending;
                    SetActive();
                    ++iHandleIndex;
                    }
                else
                    {
                    // Trusted Site certificate storage doesn't exist
                    // or something went wrong.
                    User::Leave( KErrGeneral );
                    }
            }
        else if ( iHandleIndex < iCertInfos.Count() )
            {
            iStatus = KRequestPending;
            iServerCert = HBufC8::NewL( KMaxCertificateLength );
            iCertPtr.Set( iServerCert->Des() );

            iTrustedSiteCertStore->Retrieve( *(iCertInfos[iHandleIndex]), iCertPtr, iStatus );

            iServerCerts.Append( iServerCert );

            iNextStep = ERetrieveServerCerts;
            SetActive();
            ++iHandleIndex;
            }
        else
            {
            iNextStep = ECheckServerCerts;
            iStatus = KRequestPending;
            SetActive();
            TRequestStatus* status = &iStatus;
            User::RequestComplete( status, KErrNone );
            }
        break;
        }
      case EAddTrustedSite:
          {
          if ( iStatus.Int() == KErrNone )
              {
              //Added server certificate succesfully
              CTrustSitesStore* trustedSitesStore = CTrustSitesStore::NewL();
              CleanupStack::PushL( trustedSitesStore );

              TInt status = trustedSitesStore->AddL( *iCertBuf, *iServerName );

              CleanupStack::PopAndDestroy( trustedSitesStore );
              if ( status  == KErrNone )
                  {
                  iRetValue = EServerCertAcceptedPermanently;
                  }
              else
                  {
                  iRetValue = EServerCertNotAccepted;
                  }

              iNextStep = EOperationCompleted;
              HandleResponseAndCompleteL();
              }
          else
              {
              //Adding server certificate failed
              iNextStep = EOperationCompleted;
              iRetValue = EServerCertNotAccepted;
              HandleResponseAndCompleteL();
              }
              break;
          }

      case ESaveServerCert:
          {
          WIMSECURITYDIALOGS_WRITE( "ESaveServerCert" );
          DoHandleSaveServerCertL();
          break;
          }
      case EServerCertCheckUserResp:
          {
          WIMSECURITYDIALOGS_WRITE( "EServerCertCheckUserResp" );
          if ( iRetValue == EServerCertAcceptedPermanently )
              {
              // User accepted to select certificate permanently.
              // First add server certificate
              iNextStep = ESaveServerCert;
              iStatus = KRequestPending;
              SetActive();
              TRequestStatus* status = &iStatus;
              User::RequestComplete( status, KErrNone );
              }
          else
              {
              // User declined or temporarily accepted server certificate
              HandleResponseAndCompleteL();
              }
          break;
          }
      case EOperationSignTextShown:
          {
          if ( iRetValue || iOperation == EUserAuthentication )
              {
              iNextStep = EOperationInitCertStore;
              }
          else
              {
              iNextStep = EOperationCompleted;
              }
          iStatus = KRequestPending;
          SetActive();
          TRequestStatus* status = &iStatus;
          User::RequestComplete( status, KErrNone );
          break;
          }
      case EOperationInitCertStore:
          {
          TInt err = KErrNone;
          err = iFs.Connect();
          if( err != KErrNone && err != KErrAlreadyExists )
             {
             User::Leave( KErrGeneral );
             }

          iCertStore = CUnifiedCertStore::NewL( iFs, EFalse );
          iCertArray = new (ELeave) CDesCArrayFlat( KCertArrayGranularity );
          iCertStore->Initialize( iStatus );
          iStatus = KRequestPending;
          SetActive();
          iNextStep = EOperationRetrieveCertInfos;
          break;
          }
      case EOperationRetrieveCertInfos:
          {
          if ( iCertInfo )
              {
              iCertArray->AppendL( iCertInfo->Label() );
              iCertInfo = NULL;
              }
          if ( iHandleIndex == -1 )
              {
              iFilter = CCertAttributeFilter::NewL();
              iCertStore->List( iCertInfos, *iFilter, iStatus );
              iNextStep = EOperationRetrieveCertInfos;
              iStatus = KRequestPending;
              SetActive();
              ++iHandleIndex;
              }
          else if ( iHandleIndex < iCertHandleList.Count() )
              {
              iStatus = KRequestPending;
              iCertStore->GetCert( iCertInfo, iCertHandleList[iHandleIndex], iStatus );
              ++iHandleIndex;
              iNextStep = EOperationRetrieveCertInfos;
              SetActive();
              }
          else
              {
              iHandleIndex = -1;
              iNextStep = EOperationSelectCert;
              iStatus = KRequestPending;
              SetActive();
              TRequestStatus* status = &iStatus;
              User::RequestComplete( status, KErrNone );
              }
          break;
          }
      case EOperationSelectCert:
          {
          DoHandleSelectCertificateL();
          break;
          }
      case EGetCertInfo:
          {
          iStatus = KRequestPending;
          iCertInfo = NULL;
          iCertStore->GetCert( iCertInfo, iTokenHandle, iStatus );
          iNextStep = EGetCertificate;
          SetActive();
          break;
          }
      case EGetCertificate:
          {
          iStatus = KRequestPending;
          if ( iCertInfo )
              {
              iCertBuf = HBufC8::NewL( KMaxCertificateLength );
              iCertPtr.Set( iCertBuf->Des() );
              iCertStore->Retrieve( *iCertInfo, iCertPtr, iStatus );
              SetActive();
              }
          else
              {
              SetActive();
              TRequestStatus* status = &iStatus;
              User::RequestComplete( status, KErrNone );
              }
          iNextStep = EInitKeyStore;
          break;
          }
      case EInitKeyStore:
          {
          InitKeyStoreL();
          iNextStep = EGetKeyInfos;
          break;
          }
      case EGetKeyInfos:
          {
          GetKeyInfosL();
          iNextStep = EShowCertDialog;
          break;
          }
      case EGetKeyInfoByHandle:
          {
          GetKeyInfoByHandleL();
          iNextStep = EShowCSRDialog;
          break;
          }
      case EShowCSRDialog:
          {
          ShowCSRDialogL();
          iNextStep = EOperationCompleted;
          break;
          }
      case EShowCertDialog:
          {
          ShowCertDialogL();
          iNextStep = EOperationCompleted;
          break;
          }
      /////////////
      // PIN steps
      /////////////
      case EEnterNewPIN:
          {
          if ( iRetValue)
              {
              DoHandleMultilinePinQueryL( iMultiLineDlgType );
              iNextStep = EVerifyPINs;
              iStatus = KRequestPending;
              SetActive();
              }
          else
              {
              iNextStep = EOperationCompleted;
              iPINValue1 = KNullDesC;
              iPINValue2 = KNullDesC;
              HandleResponseAndCompleteL();
              }
          break;
          }

      case EVerifyNewPin:
          {
          if ( iRetValue )
              {
              DoHandleMessageL( EVerifyPinNR, iPIN.iPINLabel, iPIN.iTokenLabel,
                    iPIN.iMinLength, iPIN.iMaxLength );
              iNextStep = EVerifyPINs;
              iStatus = KRequestPending;
              SetActive();
              }
            //Dialog was cancelled by user
          else
              {
              iNextStep = EOperationCompleted;
              iPINValue1 = KNullDesC;
              iPINValue2 = KNullDesC;
              HandleResponseAndCompleteL();
              }
          break;
          }
      case EVerifyPINs:
          {
          VerifyPinsL();
          break;
          }
      default:
          {
          User::Panic( _L("CTestSecDlgNotifier"), 0 );
          }
      }

    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::VerifyPinsL()
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::VerifyPinsL()
    {
    if ( iRetValue )
        {
        if ( iPINValue2 == iPINValueVerify )
            {
            HandleResponseAndCompleteL();
            }
        else
            {
            DoHandleMessageL( EErrorCodesDoNotMatch, KNullDesC, KNullDesC, 0, 0 );
            iNextStep = EVerifyPINs;
            iPINValue2.Zero();
            iPINValueVerify.Zero();
            DoHandleMultilinePinQueryL( iMultiLineDlgType );
            iStatus = KRequestPending;
            SetActive();
            }
        }
    else
        {
        iPINValue1 = KNullDesC;
        iPINValue2 = KNullDesC;
        iNextStep = EOperationCompleted;
        HandleResponseAndCompleteL();
        }
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::DoHandleSelectCertificateL()
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::DoHandleSelectCertificateL()
    {
    TBool foundDevCert = EFalse;

    // Check certificate list to find out if there is certificate from
    // Device Certificate Store.
    for ( TInt ii = 0; ii < iCertHandleList.Count(); ii++)
        {
        TCTTokenObjectHandle handle = iCertHandleList[ii];
        if ( handle.iTokenHandle.iTokenTypeUid == KDeviceCertStoreTokenUid )
            {
            // Found a certificate from Device Certificate Store.
            foundDevCert = ETrue;
            iTokenHandle = handle;
            iRetValue = ETrue;
            iNextStep = EOperationCompleted;
            iStatus = KRequestPending;
            SetActive();
            TRequestStatus* status = &iStatus;
            User::RequestComplete( status, KErrNone );
            break;
            }

        }

    if ( !foundDevCert )
        {
        // No certificate from Device Certificate Store. Prompt user
        // for certificate selection
        CCTSelectCertificateDialog::RunDlgLD(
            iCertArray, iCertHandleList, iTokenHandle,
            iStatus, iRetValue ); // Takes ownerhip of array

        iNextStep = EOperationCompleted;
        iStatus = KRequestPending;
        SetActive();
        }
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::ShowNoTrustDialogL()
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::ShowNoTrustDialogL()
    {
    // Cannot display untrusted certificate dialog if device is locked. Untrusted
    // certificate dialog would be displayed on top of possible lock code query.
    TInt value = 0;
    TInt err = RProperty::Get( KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus, value );
    if( !err && value > EAutolockOff )
        {
        iRetValue = EServerCertNotAccepted;
        iStatus = KRequestPending;
        TRequestStatus* status( &iStatus );
        User::RequestComplete( status, KErrNone );
        }
    else
        {
        CX509Certificate* cert = CX509Certificate::NewLC( iCertBuf->Des() );
        TInt dialogResourceId = R_WIM_NO_TRUST_QUERY_UNTRUSTED;
        TInt promptResourceId = R_QTN_ICS_SSL_CONF_Q_ACCEPT_UNTRUSTED;
        TBool showPermAccept = ETrue;

        if( iAuthFailReason == ESignatureInvalid || iAuthFailReason == ECertificateRevoked )
            {
            // Invalid or revoked certificate
            CCTInvalidCertificateNote* note =
                new( ELeave ) CCTInvalidCertificateNote( *this, iStatus );
            note->ExecuteLD( R_WIM_INVALID_CERTIFICATE_INFORMATION_NOTE );
            iRetValue = EServerCertNotAccepted;

            // CCTNoTrustQuery below self-completes this active object. It is necessary to
            // complete it here too, in order to run the next step EServerCertCheckUserResp.
            iStatus = KRequestPending;
            TRequestStatus* status( &iStatus );
            User::RequestComplete( status, KErrNone );
            }
        else
            {
            if( iAuthFailReason == EValidatedOK || iAuthFailReason == EDateOutOfRange )
                {
                // Trusted certificate, but problems with CN or date

                // Retrieve subject name
                const CX500DistinguishedName& dName = cert->SubjectName();

                // Retrieve common name
                HBufC* cn = dName.ExtractFieldL( KX520CommonName );

                // Check common name first and then date
                if( !cn )
                    {
                    // Couldn't retrieve CN from certificate
                    dialogResourceId = R_WIM_NO_TRUST_QUERY_UNTRUSTED;
                    promptResourceId = R_QTN_ICS_SSL_CONF_Q_ACCEPT_UNTRUSTED;
                    }
                else if( iServerName->Des() != cn->Des() )
                    {
                    // Domain name doesn't match with CN
                    dialogResourceId = R_WIM_NO_TRUST_QUERY_SITE;
                    promptResourceId = R_QTN_ICS_SSL_CONF_Q_ACCEPT_SITE;
                    }
                else if( iAuthFailReason == EDateOutOfRange )
                    {
                    // Certificate is out of date
                    dialogResourceId = R_WIM_NO_TRUST_QUERY_OOD;
                    promptResourceId = R_QTN_ICS_SSL_CONF_Q_ACCEPT_OOD;
                    showPermAccept = EFalse;
                    }
                else
                    {
                    // Otherwise show general untrusted note
                    dialogResourceId = R_WIM_NO_TRUST_QUERY_UNTRUSTED;
                    promptResourceId = R_QTN_ICS_SSL_CONF_Q_ACCEPT_UNTRUSTED;
                    }
                }
            else
                {
                // Untrusted certificate
                dialogResourceId = R_WIM_NO_TRUST_QUERY_UNTRUSTED;
                promptResourceId = R_QTN_ICS_SSL_CONF_Q_ACCEPT_UNTRUSTED;
                }

            // No "Accept Permanently" option if certificate is out of date, or
            // if domain name is not defined, or if trusted site store failed to
            // open (and it's not possible to save the server certificate).
            const CValidityPeriod& validityPeriod = cert->ValidityPeriod();
            const TTime& startValue = validityPeriod.Start();
            const TTime& finishValue = validityPeriod.Finish();
            TTime current;
            current.UniversalTime();

            if( (( startValue > current ) || ( finishValue < current )) ||
                    ( iServerName->Des().Length() == 0 ) ||
                    ( !iTrustedSiteCertStore ) )
                {
                showPermAccept = EFalse;
                }

            // CCTNoTrustQuery completes iStatus asynchronously when ready. Note that
            // it happens after ExecuteLD() and also this function have returned.
            iStatus = KRequestPending;

            iQueryDialog = new( ELeave ) CCTNoTrustQuery( *this, iRetValue, iStatus,
                    iServerName, showPermAccept, iQueryDialogDeleted );
            HBufC* prompt = StringLoader::LoadLC( promptResourceId, *iServerName );

            iQueryDialog->ExecuteLD( dialogResourceId, *prompt );

            CleanupStack::PopAndDestroy( prompt );
            }

        CleanupStack::PopAndDestroy( cert );
        }

    iNextStep = EServerCertCheckUserResp;
    SetActive();
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::ShowCSRDialogL()
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::ShowCSRDialogL()
    {
    HBufC* message = HBufC::NewLC( KMaxLengthTextDetailsBody );
    TPtr messagePtr = message->Des();

    DetailsResourceL( messagePtr, R_QTN_SN_CERT_SIGN_EXPLANATION );

    messagePtr.Append( KEnter );

    DetailsFieldDynamicL( messagePtr, *iText,
        R_TEXT_RESOURCE_DETAILS_VIEW_SUBJECT,
        R_TEXT_RESOURCE_VIEW_NO_SUBJECT_DETAILS );

    AddKeyUsageL( messagePtr, iKeyInfo );
    AddKeyAlgorithmL( messagePtr, iKeyInfo );
    AddKeySizeL( messagePtr, iKeyInfo );
    AddKeyLocationL( messagePtr, iKeyInfo );

    CCTSignTextDialog::RunDlgLD( R_WIM_CSR_DIALOG, *message, iStatus, iRetValue );
    CleanupStack::PopAndDestroy( message );

    iStatus = KRequestPending;
    SetActive();
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::ShowCertDialogL()
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::ShowCertDialogL()
    {
    CCTKeyInfo* keyInfo = NULL;
    if ( 0 < iKeyInfos.Count() )
        {
        keyInfo = iKeyInfos[0]; // There should be only one keyInfo.
        }
    else if ( iKeyInfo )
        {
        keyInfo = iKeyInfo;
        }
    else
        {
        }

    HBufC* message = NULL;

    if (iOperation == ESaveCert)
        {
        message = MessageQueryCertDetailsL( *iCertBuf, NULL, iSaveCertInput.iCertFormat, NULL );
        }
    else
        {
        message = MessageQueryCertDetailsL( *iCertBuf, iCertInfo, iCertInfo->CertificateFormat(), keyInfo );
        }

    CleanupStack::PushL( message );
    TInt resource = 0;
    switch( iOperation )
        {
        case ECertDetails:
            {
            resource = R_WIM_CERTDETAILS_DIALOG;
            break;
            }
        case ESaveCert:
            {
            resource = R_WIM_SAVE_CERT_DIALOG;
            break;
            }
        case EDeleteCert:
            {
            resource = R_WIM_DELETE_CERT_DIALOG;
            break;
            }
        default:
            {
            User::Panic(_L("CTestSecDlgNotifier"), 0);
            }
        }
    CCTSignTextDialog::RunDlgLD( resource, *message, iStatus, iRetValue );
    CleanupStack::PopAndDestroy( message );

    iStatus = KRequestPending;
    SetActive();
    }

// ---------------------------------------------------------
// CCertManUICertificateHelper::MessageQueryViewDetailsL(TInt aIndex)
// Creates the whole of certificate details view
// Appends strings from both resources and CertManAPI to one
// message body text and displays it.
// ---------------------------------------------------------
//
HBufC* CCTSecurityDialogsAO::MessageQueryCertDetailsL(
    const TDesC8& aCert,
    const CCTCertInfo* aCertInfo,
    TCertificateFormat aCertFormat,
    const CCTKeyInfo* aKeyInfo)
    {
    // Create message buffer
    HBufC* message = HBufC::NewLC( KMaxLengthTextDetailsBody );
    TPtr messagePtr = message->Des();
    // Label
    if ( aCertInfo )
        {
        DetailsFieldDynamicL( messagePtr, aCertInfo->Label(),
            R_TEXT_RESOURCE_DETAILS_VIEW_LABEL,
            R_TEXT_RESOURCE_VIEW_NO_LABEL_DETAILS );
        }

    switch ( aCertFormat )
        {
        case EX509Certificate:
            {
            CX509Certificate* cert = CX509Certificate::NewLC( aCert );
            // Let's append all the fields to the message
            AddSiteL( messagePtr );
            AddIssuerAndSubjectL( messagePtr, *cert );
            if( aKeyInfo )
                {
                AddKeyUsageL( messagePtr, aKeyInfo );
                }
            else
                {
                AddKeyUsageL( messagePtr, *cert );
                }

            AddValidityPeriodL( messagePtr, *cert );
            AddCertLocationL( messagePtr, aCertInfo );
            AddCertFormatL( messagePtr, aCertFormat );
            AddKeyLocationL( messagePtr, aKeyInfo );
            AddCertAlgorithmsL( messagePtr, *cert );
            AddCertSerialNumberL( messagePtr, *cert );
            AddCertFingerprintsL( messagePtr, *cert );
            CleanupStack::PopAndDestroy( cert );
            break;
            }
        case EX509CertificateUrl:
            {
            AddKeyUsageL( messagePtr, aKeyInfo );

            if( aCertInfo )
                {
                DetailsFieldDynamicL( messagePtr, aCertInfo->Label(),
                        R_TEXT_RESOURCE_DETAILS_VIEW_LABEL,
                        R_TEXT_RESOURCE_VIEW_NO_LABEL_DETAILS );
                }
            HBufC* url = HBufC::NewLC( aCert.Length() );
            url->Des().Copy( aCert );
            DetailsFieldDynamicL( messagePtr, *url,
                    R_TEXT_RESOURCE_DETAILS_VIEW_CERTIFICATE_LOCATION,
                    R_TEXT_RESOURCE_DETAILS_VIEW_NOT_DEFINED );
            CleanupStack::PopAndDestroy( url );
            AddCertFormatL( messagePtr, aCertFormat );
            AddKeyLocationL( messagePtr, aKeyInfo );
            break;
            }
        default:
            {
            User::Leave( KErrNotSupported );
            }
        }

    CleanupStack::Pop( message );
    return message;
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::AddIssuerAndSubjectL(...)
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::AddSiteL( TDes& aMessage )
    {
    if ( iServerName )
        {
        DetailsFieldDynamicL( aMessage, iServerName->Des(),
            R_QTN_CM_SITE,
            R_TEXT_RESOURCE_DETAILS_VIEW_NOT_DEFINED );
        }
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::AddIssuerAndSubjectL(...)
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::AddIssuerAndSubjectL( TDes& aMessage, const CX509Certificate& aCert )
    {
    HBufC* issuer = NULL;
    HBufC* owner = NULL;
    X509CertNameParser::SubjectFullNameL( aCert, owner );
    CleanupStack::PushL( owner );
    X509CertNameParser::IssuerFullNameL( aCert, issuer );
    CleanupStack::PushL( issuer );

    DetailsFieldDynamicL( aMessage, issuer->Des(),
        R_TEXT_RESOURCE_DETAILS_VIEW_ISSUER,
        R_TEXT_RESOURCE_DETAILS_VIEW_NOT_DEFINED );
    CleanupStack::PopAndDestroy( issuer );   //issuer

    // certificate subject
    DetailsFieldDynamicL( aMessage, owner->Des(),
        R_TEXT_RESOURCE_DETAILS_VIEW_SUBJECT,
        R_TEXT_RESOURCE_VIEW_NO_SUBJECT_DETAILS );
    CleanupStack::PopAndDestroy( owner );   //owner
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::AddKeySizeL(...)
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::AddKeySizeL( TDes& aMessage, const CCTKeyInfo* aKeyInfo )
    {
    TUint keySize = aKeyInfo->Size();

    TBuf<KMaxLengthTextDateString> sizeBuf;
    if ( 0 < keySize )
        {
        sizeBuf.AppendNum( keySize, EDecimal );
        }

    DetailsFieldDynamicL( aMessage, sizeBuf,
        R_QTN_SN_KEY_LENGTH_LABEL,
        R_TEXT_RESOURCE_DETAILS_VIEW_NOT_DEFINED );

    aMessage.Append( KEnterEnter );
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::AddKeyUsageL(...)
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::AddKeyUsageL( TDes& aMessage, const CCTKeyInfo* aKeyInfo )
    {
    if ( aKeyInfo != NULL )
        {
        TKeyUsagePKCS15 keyUsage = aKeyInfo->Usage();

        TInt usage = 0;
        switch( keyUsage )
            {
            case EPKCS15UsageSignSignRecover:
            case EPKCS15UsageSign:
            case EPKCS15UsageSignDecrypt:
                usage = R_TEXT_RESOURCE_DETAILS_VIEW_CLIENT_AUTHENTICATION;
                break;
            case EPKCS15UsageNonRepudiation:
                usage = R_TEXT_RESOURCE_DETAILS_VIEW_DIGITAL_SIGNING;
                break;
            default:
                usage = R_TEXT_RESOURCE_DETAILS_VIEW_NOT_DEFINED;
                break;
            }
        DetailsFieldResourceL( aMessage,
            R_TEXT_RESOURCE_DETAILS_VIEW_KEY_USAGE, usage);
        }
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::AddKeyUsageL(...)
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::AddKeyUsageL( TDes& aMessage, const CX509Certificate& aCert )
    {
  TKeyUsageX509 x509Usage = EX509UsageNone;
  TKeyUsagePKCS15 pkcs15KeyUsage = EPKCS15UsageNone;
  const CX509CertExtension* ext = aCert.Extension(KKeyUsage);
  if ( ext )
      {
      CX509KeyUsageExt* keyUsageExt =
      CX509KeyUsageExt::NewLC(ext->Data());
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
          usage = R_TEXT_RESOURCE_DETAILS_VIEW_CLIENT_AUTHENTICATION;
          break;
          }
      case EPKCS15UsageNonRepudiation:
          {
          usage = R_TEXT_RESOURCE_DETAILS_VIEW_DIGITAL_SIGNING;
          break;
          }
      default:
          {
          usage = R_TEXT_RESOURCE_DETAILS_VIEW_NOT_DEFINED;
          break;
          }

      }
   DetailsFieldResourceL( aMessage,
          R_TEXT_RESOURCE_DETAILS_VIEW_KEY_USAGE, usage );
   }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::AddKeyAlgorithmL(...)
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::AddKeyAlgorithmL( TDes& aMessage, const CCTKeyInfo* aKeyInfo )
    {
    TInt algRes = 0;
    switch( aKeyInfo->Algorithm())
        {
        case CKeyInfoBase::ERSA:
            {
            algRes = R_TEXT_RESOURCE_DETAILS_VIEW_ALGORITHM_RSA;
            break;
            }
        case CKeyInfoBase::EDSA:
            {
            algRes = R_TEXT_RESOURCE_DETAILS_VIEW_ALGORITHM_DSA;
            break;
            }
        case CKeyInfoBase::EDH:
            {
            algRes = R_TEXT_RESOURCE_DETAILS_VIEW_ALGORITHM_DH;
            break;
            }
        case CKeyInfoBase::EInvalidAlgorithm:
            {
            algRes = R_TEXT_RESOURCE_DETAILS_VIEW_UNKNOWN;
            break;
            }
        default:
            {
            algRes = R_TEXT_RESOURCE_DETAILS_VIEW_UNKNOWN;
            }
        }
    DetailsFieldResourceL( aMessage,
            R_TEXT_RESOURCE_DETAILS_VIEW_ALGORITHM, algRes);
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::AddCertLocationL(...)
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::AddCertLocationL( TDes& aMessage, const CCTCertInfo* aCertInfo )
    {
    if ( aCertInfo )
        {
        AddLocationInfoL( aMessage, aCertInfo->Token().TokenType().Type(), ETrue );
        }
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::AddKeyLocationL(...)
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::AddKeyLocationL( TDes& aMessage, const CCTKeyInfo* aKeyInfo )
    {
    if ( aKeyInfo )
        {
        AddLocationInfoL( aMessage, aKeyInfo->Token().TokenType().Type(), EFalse );
        }
    }

// ---------------------------------------------------------
// CCTSecurityDialogsAO::SetLocationInfo( (HBufC& aMessage,
//                                               TBool aCertificate,
//                                               TUid* aLocUid)
// Adds certificate/private key's location info to certificate details
// ---------------------------------------------------------
//
void CCTSecurityDialogsAO::AddLocationInfoL(
    TDes& aMessage, TUid aUid, TBool aCertificate )
    {
    TInt location = 0;
    TInt locationRes =0;

    switch ( aUid.iUid )
        {
        case KFileCertStoreUid:
        case KTokenTypeFileKeystore:
            {
            location = R_TEXT_RESOURCE_DETAILS_VIEW_LOCATION_PHONE_MEMORY;
            break;
            }
        case KWIMCertStoreUid:
            {
            location = R_TEXT_RESOURCE_DETAILS_VIEW_LOCATION_SMART_CARD;
            break;
            }
        default:
            {
            if ( aCertificate )
                {
                location = R_TEXT_RESOURCE_DETAILS_VIEW_NOT_DEFINED;
                }
            else
                {
                location = R_TEXT_RESOURCE_DETAILS_VIEW_NO_PRIVATE_KEY;
                }
            break;
            }
        }

    if ( aCertificate )
        {
        locationRes = R_TEXT_RESOURCE_DETAILS_VIEW_CERTIFICATE_LOCATION;
        }
    else
        {
        locationRes = R_TEXT_RESOURCE_DETAILS_VIEW_PRIVATE_KEY_LOCATION;
        }

    DetailsFieldResourceL( aMessage, locationRes, location );
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::AddValidityPeriodL(...)
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::AddValidityPeriodL(
    TDes& aMessage, const CX509Certificate& aCert )
    {
    TLocale locale;
    TTimeIntervalSeconds offSet = locale.UniversalTimeOffset();
    DetailsResourceL( aMessage, R_TEXT_RESOURCE_DETAILS_VIEW_VALID_FROM );
    const CValidityPeriod& validityPeriod = aCert.ValidityPeriod();
    TTime startValue = validityPeriod.Start();
    startValue += offSet;
    TBuf<KMaxLengthTextDateString> startString;
    // read format string from AVKON resource
    HBufC* dateFormatString = CEikonEnv::Static()->AllocReadResourceLC(
        R_QTN_DATE_USUAL_WITH_ZERO );
    // format the date to user readable format. The format is locale dependent
    startValue.FormatL( startString, *dateFormatString );
    AknTextUtils::DisplayTextLanguageSpecificNumberConversion( startString );
    CleanupStack::PopAndDestroy(); // dateFormatString
    aMessage.Append( startString );
    aMessage.Append( KEnterEnter );

    DetailsResourceL( aMessage, R_TEXT_RESOURCE_DETAILS_VIEW_VALID_UNTIL );
    TTime finishValue = validityPeriod.Finish();
    finishValue += offSet;
    TBuf<KMaxLengthTextDateString> finishString;
    // read format string from AVKON resource
    dateFormatString = CEikonEnv::Static()->AllocReadResourceLC(
        R_QTN_DATE_USUAL_WITH_ZERO );
    // format the date to user readable format. The format is locale dependent
    finishValue.FormatL( finishString, *dateFormatString );
    AknTextUtils::DisplayTextLanguageSpecificNumberConversion(finishString);
    CleanupStack::PopAndDestroy(); // dateFormatString
    aMessage.Append( finishString );
    aMessage.Append( KEnterEnter );
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::AddCertFormatL(...)
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::AddCertFormatL( TDes& aMessage, TCertificateFormat aCertFormat)
    {
    TInt fieldType = 0;
    switch ( aCertFormat )
        {
        case EX509CertificateUrl:
        case EX509Certificate:
            {
            fieldType = R_TEXT_RESOURCE_DETAILS_VIEW_CERT_FORMAT_X509;
            break;
            }
        default:
            {
            fieldType = R_TEXT_RESOURCE_DETAILS_VIEW_NOT_DEFINED;
            break;
            }
        }
    DetailsFieldResourceL( aMessage,
        R_TEXT_RESOURCE_DETAILS_VIEW_CERT_FORMAT, fieldType );
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::AddCertAlgorithmsL(...)
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::AddCertAlgorithmsL( TDes& aMessage, const CX509Certificate& aCert )
    {
    TInt fieldType = 0;
    TInt fieldType2 = 0;

    // digest algorithm
    TAlgorithmId algorithmId = aCert.SigningAlgorithm().DigestAlgorithm().Algorithm();
    fieldType = AlgorithmNameResourceId( algorithmId );

    // public-key algorithm
    algorithmId = aCert.SigningAlgorithm().AsymmetricAlgorithm().Algorithm();
    fieldType2 = AlgorithmNameResourceId( algorithmId );

    // If other algorithm is unknown
    if( fieldType == R_TEXT_RESOURCE_DETAILS_VIEW_UNKNOWN ||
        fieldType2 == R_TEXT_RESOURCE_DETAILS_VIEW_UNKNOWN )
        {
        DetailsFieldResourceL( aMessage,
        R_TEXT_RESOURCE_DETAILS_VIEW_ALGORITHM, R_TEXT_RESOURCE_DETAILS_VIEW_UNKNOWN );
        }
    else // Both are known.
        {
        DetailsResourceL( aMessage, R_TEXT_RESOURCE_DETAILS_VIEW_ALGORITHM);
        HBufC* stringHolder = StringLoader::LoadLC( fieldType );
        aMessage.Append( stringHolder->Des() );
        CleanupStack::PopAndDestroy();
        stringHolder = StringLoader::LoadLC( fieldType2 );
        aMessage.Append( stringHolder->Des() );
        CleanupStack::PopAndDestroy();  // stringHolder
        aMessage.Append( KEnterEnter );
        }
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::AddCertSerialNumberL(...)
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::AddCertSerialNumberL( TDes& aMessage, const CX509Certificate& aCert )
    {
    // certificate serial number
    DetailsResourceL( aMessage, R_TEXT_RESOURCE_DETAILS_VIEW_SERIAL_NUMBER );
    TPtrC8 serialNumber = aCert.SerialNumber();
    TBuf<KMaxLengthTextSerialNumberFormatting> buf2;

    for ( TInt i = 0; i < serialNumber.Length(); i++ )
       {
       buf2.Format( KCertManUIDetailsViewHexFormat, serialNumber[i] );
       aMessage.Append( buf2 );
       }

    aMessage.Append( KEnterEnter );
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::AddCertFingerprintsL(...)
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::AddCertFingerprintsL( TDes& aMessage, const CX509Certificate& aCert )
    {
     // certificate fingerprint SHA-1
    DetailsResourceL( aMessage, R_TEXT_RESOURCE_DETAILS_VIEW_FINGERPRINT );

    TPtrC8 sha1_fingerprint = aCert.Fingerprint();
    DevideToBlocks( sha1_fingerprint, aMessage );

    aMessage.Append( KEnterEnter );

    // certificate fingerprint MD5
    DetailsResourceL( aMessage, R_TEXT_RESOURCE_DETAILS_VIEW_FINGERPRINT_MD5 );

    CMD5* md5 = CMD5::NewL();
    CleanupStack::PushL( md5 );
    TBuf8<20> fingerprint = md5->Hash( aCert.Encoding() );
    CleanupStack::PopAndDestroy( md5 );

    DevideToBlocks( fingerprint, aMessage );
    }

// ---------------------------------------------------------
// CCTSecurityDialogsAO::DevideToBlocks
// ---------------------------------------------------------
//
void CCTSecurityDialogsAO::DevideToBlocks( const TDesC8& aInput, TDes& aOutput )
    {
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

// ---------------------------------------------------------
// CCTSecurityDialogsAO::DetailsFieldResourceL(
// HBufC& aMessage, TInt aResourceOne, TInt aResourceTwo);
// Appends a field that has two strings from resources
// to the details view message body
// ---------------------------------------------------------
//
void CCTSecurityDialogsAO::DetailsFieldResourceL(
    TDes& aMessage, TInt aResourceOne,
    TInt aResourceTwo )
    {
    DetailsResourceL( aMessage, aResourceOne );
    DetailsResourceL( aMessage, aResourceTwo );
    aMessage.Append( KEnter );
    }

// ---------------------------------------------------------
// CCTSecurityDialogsAO::DetailsFieldDynamicL(HBufC& aMessage, TPtrC aValue,
//                                        TInt aResourceOne, TInt aResourceTwo);
// Appends a field that has string from resources and string from CertManAPI
// to the details view message body
// ---------------------------------------------------------
//
void CCTSecurityDialogsAO::DetailsFieldDynamicL(
    TDes& aMessage,
    const TDesC& aValue,
    TInt aResourceOne,
    TInt aResourceTwo)
    {
    DetailsResourceL( aMessage, aResourceOne );
    DetailsDynamicL( aMessage, aValue, aResourceTwo );
    aMessage.Append( KEnter );
    }

// ---------------------------------------------------------
// CCTSecurityDialogsAO::DetailsResourceL(
// HBufC& aMessage, TInt aResourceOne);
// Reads line from resources
// ---------------------------------------------------------
//
void CCTSecurityDialogsAO::DetailsResourceL(
    TDes& aMessage, TInt aResource)
    {
    HBufC* stringHolder = StringLoader::LoadLC( aResource );
    aMessage.Append( stringHolder->Des() );
    CleanupStack::PopAndDestroy();  // stringHolder
    aMessage.Append( KEnter );
    }

// ---------------------------------------------------------
// CCTSecurityDialogsAO::DetailsDynamicL(
// HBufC& aMessage, TPtrC aValue, TInt aResourceOne)
// Reads dynamic text, if the string is empty
// put a not defined text from the resource in its place
// ---------------------------------------------------------
//
void CCTSecurityDialogsAO::DetailsDynamicL(
    TDes& aMessage, const TDesC& aValue, TInt aResource)
    {
    HBufC* buf = aValue.AllocLC();
    TPtr trimmedValue( buf->Des() );
    trimmedValue.TrimLeft();

    if( trimmedValue.Length() > KMaxLengthTextCertLabelVisible )
        {
        trimmedValue.SetLength( KMaxLengthTextCertLabelVisible - 1 );
        trimmedValue.Append( KTextUtilClipEndChar );
        }

    if( trimmedValue.Length() == 0 )
        {
        DetailsResourceL( aMessage, aResource );
        }
    else
        {
        aMessage.Append( trimmedValue );
        aMessage.Append( KEnter );
        }

    CleanupStack::PopAndDestroy( buf );
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::HandleResponseAndCompleteL()
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::HandleResponseAndCompleteL()
    {
    WIMSECURITYDIALOGS_WRITE_FORMAT( "HandleResponseAndCompleteL: %d", iRetValue );

    MapTlsProviderOperation(iOperation);

    switch (iOperation)
        {
        case EEnablePIN: // flow thru
        case EDisablePIN: // flow thru
        case EEnterPIN:
            {
            TPINValueBuf pinValueBufPtr;
            if ( iPIN.iPINLabel == KKeyStoreCreatePwLabel ||
                iPIN.iPINLabel == KKeyStoreExportKeyLabel )
                {
                pinValueBufPtr = iPINValue2;
                }
            else
                {
                pinValueBufPtr = iPINValue1;
                }
            iMessagePtr.WriteL( iReplySlot, pinValueBufPtr );
            break;
            }
        case EChangePIN:
        case EUnblockPIN:
            {
            TTwoPINOutput twoPINOutput;
            twoPINOutput.iPINValueToCheck = iPINValue1;
            twoPINOutput.iNewPINValue = iPINValue2;
            TTwoPINOutputBuf twoPINOutputBuf( twoPINOutput );
            iMessagePtr.WriteL( iReplySlot, twoPINOutputBuf );
            break;
            }
        case ESignText: // flow thru
        case EUserAuthenticationText: // flow thru
        case EUserAuthentication:
            {
            TCTTokenObjectHandleBuf tokenObjectHandleBuf( iTokenHandle );
            iMessagePtr.WriteL( iReplySlot, tokenObjectHandleBuf );
            break;
            }
        case EServerAuthenticationFailure:
            {
            TServerAuthenticationFailureDialogResult result = EStop;

            if ( iRetValue != EServerCertAcceptedPermanently )
                {
                if ( iRetValue == EServerCertAcceptedTemporarily )
                    {
                    result = EContinue;
                    }
                else
                    {
                    result = EStop;
                    }
                }
            else
                {
                // User permanently accepted server certificate.
                result = EContinue;
                }

            TServerAuthenticationFailureOutputBuf output( result );
            iMessagePtr.WriteL( iReplySlot, output );
            iMessagePtr.Complete( KErrNone );

            break;
            }
        case EPINBlocked:
        case EUnblockPINInClear:
            {
            User::Leave( KErrNotSupported );
            break;
            }
        case ECreateCSR:
        case ECertDetails:
        case ESaveCert:
        case EDeleteCert:
        case ESaveReceipt:
        case EPinCodeBlockedInfo:
        case MDigSigningNote::ESignatureRequested:
        case MDigSigningNote::ESignatureDone:
        case MDigSigningNote::ESigningCancelled:
        case MDigSigningNote::ENoMatchCert:
        case MDigSigningNote::ENoSecurityModule:
        case MDigSigningNote::EInternalError:
            {
            break; // Complete is enough
            }
        default:
            User::Panic( _L("CTestSecDlgNotifier"), 0 );
        }

    if( iOperation != EServerAuthenticationFailure )
        {
        iMessagePtr.Complete( iRetValue ? KErrNone : KErrCancel );
        }

    delete this;
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::SaveReceiptL()
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::SaveReceiptL( const TDesC8& aBuffer )
    {
    TInt size = reinterpret_cast<const TInt&>( *aBuffer.Ptr() );
    TInt start = sizeof(TInt) + 1;
    HBufC* signedText = HBufC::NewLC( size );
    signedText->Des().Copy( aBuffer.Mid(start) );
    CEikProcess* process = CEikonEnv::Static()->Process();
    CDocumentHandler* docHandler =  CDocumentHandler::NewLC( process );
    _LIT8( KPlainText, "text/plain" );
    TDataType dataType( KPlainText );
    // convert 16->8bits
    HBufC8* content = HBufC8::NewLC( signedText->Length() );
    content->Des().Copy( *signedText );

    docHandler->SaveL( *content,
    dataType,
    KEntryAttNormal);

    CleanupStack::PopAndDestroy( 3, signedText );   // content, docHandler, signedText
    HandleResponseAndCompleteL();
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::DoCancel()
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::DoCancel()
    {
    WIMSECURITYDIALOGS_WRITE( "CCTSecurityDialogsAO::DoCancel" );

    // Note that iQueryDialog may point to already deleted memory.
    // Dialogs need to set and reset iQueryDialogDeleted flag to
    // allow deleting the dialog from CCTSecurityDialogsAO.
    if( !iQueryDialogDeleted )
        {
        delete iQueryDialog;
        iQueryDialogDeleted = ETrue;
        }
    iQueryDialog = NULL;

    if ( !iPinQueryDialogDeleted )
        {
        delete iPinQueryDialog;
        iPinQueryDialogDeleted = ETrue;
        }
    iPinQueryDialog = NULL;

    // Complete message if it has not been completed earlier.
    if( !iMessagePtr.IsNull() )
        {
        iMessagePtr.Complete( KErrCancel );
        }
    }

// For server authentication failure
// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::CreateMessageLC()
// -----------------------------------------------------------------------------

HBufC* CCTSecurityDialogsAO::CreateMessageL()
    {
    //TPtrC8 certPtrC;
    //iSrvAuthFail->GetEncodedCert(certPtrC);
    return MessageQueryCertDetailsL(
        *iCertBuf, NULL, EX509Certificate, NULL );
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::DoHandleSaveServerCertL()
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::DoHandleSaveServerCertL()
    {
    if ( iTrustedSiteCertStore )
        {
        iTrustedSiteCertStore->Add( *iCertLabel, EX509Certificate,
                                EPeerCertificate, NULL, NULL, *iCertBuf, iStatus );

        // Next step is to update trust site db
        iNextStep = EAddTrustedSite;
        iStatus = KRequestPending;
        SetActive();
        }
    else
        {
        User::Leave( KErrGeneral );
        }
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogsAO::MapTlsProviderOperation()
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogsAO::MapTlsProviderOperation( TUint aOperation )
    {
    switch (aOperation)
        {
        case ESecureConnection:
            {
            iOperation = EUserAuthentication;
            break;
            }
        default:
            break;
        }
    }

