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
* Description:  Basic PIN query operation in security dialog
*
*/

#include "securitydialogoperbasicpinquery.h" // CBasicPinQueryOperation
#include "securitydialogoperationobserver.h" // MSecurityDialogOperationObserver
#include <hb/hbcore/hbtextresolversymbian.h> // HbTextResolverSymbian
#include "securitydialogstrace.h"       // TRACE macro

// TODO: fix this
#include "../../../securitydialogs/SecUi/Inc/SecQueryUi.h"  // CSecQueryUi

// Descriptors for different password queries
_LIT( KBasicPinQueryKeyStorePassphrase, "Key store passphrase" );
_LIT( KBasicPinQueryNewKeyStorePassphrase, "New key store passphrase" );
_LIT( KBasicPinQueryImportedKeyFilePassphrase, "Passphrase of the imported key file" );
_LIT( KBasicPinQueryExportedKeyFilePassphrase, "Passphrase of the exported key file" );
_LIT( KBasicPinQueryPKCS12, "PKCS12");

const TInt KMinimunAcceptedKeystorePassphraseLength = 6;


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CBasicPinQueryOperation::NewL()
// ---------------------------------------------------------------------------
//
CBasicPinQueryOperation* CBasicPinQueryOperation::NewL(
        MSecurityDialogOperationObserver& aObserver, const RMessage2& aMessage,
        TInt aReplySlot )
    {
    TRACE( "CBasicPinQueryOperation::NewL, aMessage 0x%08x", aMessage.Handle() );
    return new( ELeave ) CBasicPinQueryOperation( aObserver, aMessage, aReplySlot );
    }

// ---------------------------------------------------------------------------
// CBasicPinQueryOperation::~CBasicPinQueryOperation()
// ---------------------------------------------------------------------------
//
CBasicPinQueryOperation::~CBasicPinQueryOperation()
    {
    TRACE( "CBasicPinQueryOperation::~CBasicPinQueryOperation" );
    Cancel();
    delete iQueryUi;
    iQueryUi = NULL;
    iPinInput = NULL;   // not owned
    }

// ---------------------------------------------------------------------------
// CBasicPinQueryOperation::StartL()
// ---------------------------------------------------------------------------
//
void CBasicPinQueryOperation::StartL( const TDesC8& aBuffer )
    {
    TRACE( "CBasicPinQueryOperation::StartL" );
    iPinInput = reinterpret_cast< const TPINInput* >( aBuffer.Ptr() );
    ASSERT( iPinInput != NULL );

    iStatus = KRequestPending;
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, KErrNone );
    SetActive();
    }

// ---------------------------------------------------------------------------
// CBasicPinQueryOperation::CancelOperation()
// ---------------------------------------------------------------------------
//
void CBasicPinQueryOperation::CancelOperation()
    {
    TRACE( "CBasicPinQueryOperation::CancelOperation" );
    // nothing to do
    }

// ---------------------------------------------------------------------------
// CBasicPinQueryOperation::RunL()
// ---------------------------------------------------------------------------
//
void CBasicPinQueryOperation::RunL()
    {
    TRACE( "CBasicPinQueryOperation::RunL, iStatus.Int()=%d", iStatus.Int() );
    User::LeaveIfError( iStatus.Int() );

    TBool isRetry = ( iPinInput->iOperation & EPINValueIncorrect );
    if( isRetry )
        {
        // Show "Invalid PIN code" error note, as previous attempt was failed.
        // TODO: localized UI string needed
        _LIT( KInvalidPinCode, "Invalid PIN code" );
        ShowWarningNoteL( KInvalidPinCode );
        }

    HBufC* caption = NULL;
    TInt minLength = 0;
    TInt maxLength = 0;
    TInt mode = ESecUiAlphaSupported | ESecUiSecretSupported | ESecUiCancelSupported |
            ESecUiEmergencyNotSupported;
    if( iPinInput->iPIN.iPINLabel == KBasicPinQueryKeyStorePassphrase )
        {
        // "Enter code for phone keystore:"
        minLength = KMinimunAcceptedKeystorePassphraseLength;
        // TODO: localized UI string needed
        _LIT( KText, "Keystore password:" );
        caption = KText().AllocLC();
        }
    else if( iPinInput->iPIN.iPINLabel == KBasicPinQueryNewKeyStorePassphrase )
        {
        // "Keystore password must be created for using private keys."
        minLength = KMinimunAcceptedKeystorePassphraseLength;
        // SecUi creates two input fields when caption contains two labels
        // separated with a vertical bar. SecUi verifies that user types the
        // same passphrase in both fields.
        // TODO: localized UI strings needed
        _LIT( KText, "Create keystore password:|Verify:" );
        caption = KText().AllocLC();
        }
    else if( iPinInput->iPIN.iPINLabel == KBasicPinQueryImportedKeyFilePassphrase )
        {
        // "Enter code for imported key:"
        // TODO: localized UI strings needed
        _LIT( KText, "Enter code for imported key:|Verify:" );
        caption = KText().AllocLC();
        }
    else if( iPinInput->iPIN.iPINLabel == KBasicPinQueryExportedKeyFilePassphrase )
        {
        // "Enter new code for exported key:"
        // TODO: localized UI strings needed
        _LIT( KText, "Enter new code for exported key:|Verify:" );
        caption = KText().AllocLC();
        }
    else if( iPinInput->iPIN.iPINLabel == KBasicPinQueryPKCS12 )
        {
        // "Password for %U:", PKCS#12 password query prompt
        // TODO: localized UI string needed
        _LIT( KText, "Password for PKCS#12 file:" );
        caption = KText().AllocLC();
        }
    else
        {
        // "Enter code for %0U in %1U"
        // Data query for PIN request in keystore where %0U is the PIN's name %1U is the keystore's name.
        // TODO: EEnterPinNR
        caption = iPinInput->iPIN.iPINLabel.AllocLC();
        }

    ASSERT( iQueryUi == NULL );
    iQueryUi = CSecQueryUi::NewL();
    TInt resultCode = iQueryUi->SecQueryDialog( *caption, iPinValue, minLength, maxLength, mode );
    ReturnResultL( resultCode );

    if( caption )
        {
        CleanupStack::PopAndDestroy( caption );
        caption = NULL;
        }
    }

// ---------------------------------------------------------------------------
// CBasicPinQueryOperation::DoCancel()
// ---------------------------------------------------------------------------
//
void CBasicPinQueryOperation::DoCancel()
    {
    TRACE( "CBasicPinQueryOperation::DoCancel" );
    if( iQueryUi )
        {
        delete iQueryUi;
        iQueryUi = NULL;
        }
    }

// ---------------------------------------------------------------------------
// CBasicPinQueryOperation::CBasicPinQueryOperation()
// ---------------------------------------------------------------------------
//
CBasicPinQueryOperation::CBasicPinQueryOperation(
        MSecurityDialogOperationObserver& aObserver, const RMessage2& aMessage,
        TInt aReplySlot ) : CSecurityDialogOperation( aObserver, aMessage, aReplySlot )
    {
    }

// ---------------------------------------------------------------------------
// CBasicPinQueryOperation::ReturnResultL()
// ---------------------------------------------------------------------------
//
void CBasicPinQueryOperation::ReturnResultL( TInt aErrorCode )
    {
    TRACE( "CBasicPinQueryOperation::ReturnResultL, begin, aErrorCode=%d", aErrorCode );
    if( aErrorCode == KErrNone )
        {
        TPINValueBuf output( iPinValue );
        iMessage.WriteL( iReplySlot, output );
        }
    TRACE( "CBasicPinQueryOperation::ReturnResultL, completing msg 0x%08x", iMessage.Handle() );
    iMessage.Complete( aErrorCode );
    TRACE( "CBasicPinQueryOperation::ReturnResultL, informing observer" );
    iObserver.OperationComplete();
    TRACE( "CBasicPinQueryOperation::ReturnResultL, end" );
    }

