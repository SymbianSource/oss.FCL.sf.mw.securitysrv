/*
* Copyright (c) 2002-2006 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Resource mapper for CT Security Dialogs.
*
*/


#ifndef __CTSECURITYDIALOGDEFS_H
#define __CTSECURITYDIALOGDEFS_H

//  INCLUDES
#include <e32base.h>
#include <CTSecDlgs.rsg>
#include <avkon.rsg>
#include <secdlg.h>
#include <data_caging_path_literals.hrh>
#include <secdlgimpldefs.h>

const TUid KUidCTSecDlgAppServer = { 0x10207445 };

/**
 * Input data for the ESaveCert operation.
 */
class TSaveCertInput
  {
public:
  TSecurityDialogNotification iOperation;
  TCertificateFormat iCertFormat;
  TCertificateOwnerType iCertOwnerType;
  TInt iDataSize;
  };

/** */
typedef TPckgBuf<TSaveCertInput> TSaveCertInputBuf;


// Selection types
enum
    {
    EServerCertNotAccepted,
    EServerCertAcceptedTemporarily,
    EServerCertAcceptedPermanently
    };

// Dialog types for Security dialogs and notes.
enum TDialogType
    {
    EEnterPinNR,
    EEnterNewPinNR,
    EEnterPukNR,
    EVerifyPinNR,
    EEnterPw,
    EEnterKeyStorePw,
    EEnterNewKeyStorePw,
    EVerifyKeyStorePw,

    EImportKeyPw,
    EExportKeyPw,
    //ESignText,
                       // something wrong with the server certificate)
    EInfoSignTextRequested,
    EInfoSignTextCancelled,
    EInfoSignTextDone,
    EInfoPinCodeUnblocked,
    EInfoPwCreating,

    EErrorPinCodeIncorrect,
    EErrorPinCodeBlocked,
    EErrorPukCodeIncorrect,
    EErrorPukCodeBlocked,
    EErrorCodesDoNotMatch,
    EErrorWimNotAvailable,

    EPinCodeBlockedInfo,
    ENoMatchingPersonalCert,
    EErrorInternal,
    ESaveReceipt,

    EUserAuthentication,
    EUserAuthenticationText,
    ECreateCSR,
    ECertDetails,
    ESaveCert,
    EDeleteCert,
    };

// Layout types for Dialogs/Notes
enum TNoteType
    {
    EInfoNote,      // Meaning 'information', with green ! sign
    EErrorNote,     // Meaning 'error', with red ! sign
    EConfirmationNote,  // Meaning 'confirmed', with green V sign

    EInfoDialog,    // Information dlg,
    EWarningDialog,   //
    EErrorDialog,   //
    EConfirmationDialog,// Confirmation dlg, with blue
    EOkDialog,      //

    EEnterPinGCodeDialog, //
    EVerifyPinGCodeDialog,  //
    EEnterPinNRCodeDialog,  //
    EVerifyPinNRCodeDialog, //
    EEnterPinPinCodeDialog,
    EEnterPwDialog,
    EEnterPwPwDialog,

    ESecurityAlertDialog
    };

// KDialogTypeToResourceMapper item definition
struct TDialogTypeItem
    {
    TInt      iDialogType;      // Dialog type
    TInt    iResourceIdentifier;  // Main resource id
    TInt    iDynamicResourceIdentifier; // dynamic resource for main resource
    TNoteType iNoteType;        // Type of dialog/note that we are going to use.
    TInt    iSoftKeyResource;   // Softkey Resource to use for dlg (does not have effect on notes)                    // if 0 -> The dialog uses its defaults.
    };

// Dialog type to resource mapper array. Items are TDialogTypeItems.
const TDialogTypeItem KDialogTypeToResourceMapper[] =
    {
    { EEnterPinNR, R_QTN_WIM_PRMT_GEN_WIM_PIN, 0, EEnterPinNRCodeDialog, 0 },
    { EEnterNewPinNR, R_QTN_WIM_NEW_PIN, 0, EEnterPinPinCodeDialog, 0 },
    { EEnterPukNR, R_QTN_WIM_PIN_UNBLOCK_QUERY, 0, EEnterPinNRCodeDialog, 0 },
    { EVerifyPinNR, R_QTN_WIM_VERIFY_PIN, 0, EVerifyPinNRCodeDialog, 0 },
    { EEnterPw, R_QTN_CM_TITLE_P12_PASSWORD, 0, EEnterPwDialog, 0 },
    //{ EEnterKeyStorePw, R_QTN_CM_TITLE_KEYSTORE_PASSWORD, 0, EEnterPwDialog, 0 },
    { EEnterKeyStorePw, R_QTN_SN_ENTER_PHONE_KEYSTORE, 0, EEnterPwDialog, 0 },
    //{ EEnterNewKeyStorePw, R_QTN_CM_TITLE_NEW_KEYSTORE_PASSWORD, 0, EEnterPwPwDialog, 0 },
    { EEnterNewKeyStorePw, R_QTN_SN_NEW_PHONE_KEYSTORE, 0, EEnterPwPwDialog, 0 },
    //{ EVerifyKeyStorePw, R_QTN_CM_TITLE_VERIFY_PASSWORD, 0, EEnterPwPwDialog, 0 },
    { EVerifyKeyStorePw, R_QTN_WIM_VERIFY_PIN, 0, EEnterPwPwDialog, 0 },
    { EImportKeyPw, R_QTN_SN_PROMPT_IMPORT_KEY, 0, EEnterPwDialog, 0 },
    { EExportKeyPw, R_QTN_SN_PROMPT1_CREATE_CODE, 0, EEnterPwPwDialog, 0 },


//  { ESignText, 0, 0, EErrorNote, 0 }, // just a dummy

    { EInfoSignTextRequested, R_QTN_WIM_INFO_DIGIT_SIGN_REQ, 0, EInfoNote, 0 },
    { EInfoSignTextCancelled, R_QTN_WIM_SIGN_CANCELLED, 0, EInfoNote, 0 },
    { EInfoSignTextDone, R_QTN_WIM_INFO_SIGNATURE_MADE, 0, EConfirmationNote, 0 },
    { EInfoPinCodeUnblocked, R_QTN_WIM_INFO_PIN_UNBLOCKED, 0, EConfirmationNote, 0 },
    { EInfoPwCreating, R_QTN_CM_CREATING_KEYSTORE, 0, EInfoNote, 0 },

    { EErrorPinCodeIncorrect, R_QTN_WIM_ERR_WRONG_PIN, 0, EErrorNote, 0 },
    { EErrorPinCodeBlocked, R_QTN_WIM_ERR_PIN_BLOCKED, 0, EErrorNote, 0 },
    { EErrorPukCodeIncorrect, R_QTN_WIM_WRONG_UNBLOCK_CODE, 0, EErrorNote, 0 },
    { EErrorPukCodeBlocked, R_QTN_WIM_PIN_TOTALBLOCKED, 0, EErrorNote, 0 },
    { EErrorCodesDoNotMatch, R_QTN_WIM_ERR_CODES_DO_NOT_MATCH, 0, EErrorNote, 0 },

    { EPinCodeBlockedInfo, R_QTN_WIM_UNBLOCK_INFO, 0, EInfoNote, 0 },
    { ENoMatchingPersonalCert, R_QTN_WIM_NO_MATCHING_CERTIFICATE, 0, EInfoDialog, 0 },
    { EErrorInternal, R_QTN_WIM_ERR_IO_ERROR, 0, EErrorNote, 0 },
    { EErrorWimNotAvailable, R_QTN_WIM_NOT_AVAILABLE, 0, EErrorNote, 0 },
    { ESaveReceipt, R_QTN_WIM_RECEIPT_SAVED, 0, EConfirmationDialog, 0 },
    { EUserAuthentication, R_QTN_SN_AUTHENTICATION_NOTE, 0, EInfoNote, 0 }
    };

const TInt KCTSecDialSendBufLen = 128;

#endif

// End of File