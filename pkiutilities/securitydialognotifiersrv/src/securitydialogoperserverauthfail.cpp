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
* Description:  Server authentication failure operation in security dialog
*
*/

#include "securitydialogoperserverauthfail.h" // CServerAuthFailOperation
#include "securitydialogoperationobserver.h" // MSecurityDialogOperationObserver
#include "untrustedcertquery.h"         // CUntrustedCertQuery
#include "../../DeviceToken/Inc/TrustedSitesStore.h" // CTrustSitesStore
#include <mctwritablecertstore.h>       // MCTWritableCertStore
#include <unifiedcertstore.h>           // CUnifiedCertStore
#include <cctcertinfo.h>                // CCTCertInfo
#include <x509cert.h>                   // CX509Certificate
#include "securitydialogstrace.h"       // TRACE macro

const TUid KTrustedSiteCertificatesTokenTypeUid = { 0x101FB66F };
const TInt KMaxCommonNameLength = 64;   // from RFC3280


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CServerAuthFailOperation::NewL()
// ---------------------------------------------------------------------------
//
CServerAuthFailOperation* CServerAuthFailOperation::NewL(
        MSecurityDialogOperationObserver& aObserver, const RMessage2& aMessage,
        TInt aReplySlot )
    {
    TRACE( "CServerAuthFailOperation::NewL, aMessage 0x%08x", aMessage.Handle() );
    return new( ELeave ) CServerAuthFailOperation( aObserver, aMessage, aReplySlot );
    }

// ---------------------------------------------------------------------------
// CServerAuthFailOperation::~CServerAuthFailOperation()
// ---------------------------------------------------------------------------
//
CServerAuthFailOperation::~CServerAuthFailOperation()
    {
    TRACE( "CServerAuthFailOperation::~CServerAuthFailOperation, begin" );
    Cancel();

    delete iUntrustedCertQuery;
    iUntrustedCertQuery = NULL;

    delete iInput;
    iInput = NULL;
    delete iCertLabel;
    iCertLabel = NULL;
    delete iServerName;
    iServerName = NULL;

    delete iCertStore;
    iCertStore = NULL;
    iTrustedSiteCertStore = NULL;   // not owned

    delete iCertAttributeFilter;
    iCertAttributeFilter = NULL;
    delete iRetrievedCertBuffer;
    iRetrievedCertBuffer = NULL;

    iCertInfos.Close();
    iFs.Close();
    TRACE( "CServerAuthFailOperation::~CServerAuthFailOperation, end" );
    }

// ---------------------------------------------------------------------------
// CServerAuthFailOperation::StartL()
// ---------------------------------------------------------------------------
//
void CServerAuthFailOperation::StartL( const TDesC8& aBuffer )
    {
    TRACE( "CServerAuthFailOperation::StartL, begin" );
    ASSERT( iInput == NULL );
    iInput = CServerAuthenticationFailureInput::NewL( aBuffer );

    iInput->GetEncodedCert( iEncodedServerCert );

    TPtrC8 serverName;
    iInput->GetServerName( serverName );
    ASSERT( iServerName == NULL );
    iServerName = HBufC::NewL( serverName.Length() );
    iServerName->Des().Copy( serverName );
    TRACE( "CServerAuthFailOperation::StartL, iServerName=%S", iServerName );

    iAuthFailReason = iInput->FailureReason();
    TRACE( "CServerAuthFailOperation::StartL, iAuthFailReason=%d", iAuthFailReason );

    InitializeUnifiedCertStoreL();
    // This is async function, processing continues in RunL().
    // Basically trusted site certificates are fetched and the
    // server certificate is compared to them. If the server
    // certificate is already in trusted site cert store, then
    // connection is accepted silently. If it is not, then
    // untrusted certificate dialog is displayed.
    }

// ---------------------------------------------------------------------------
// CServerAuthFailOperation::CancelOperation()
// ---------------------------------------------------------------------------
//
void CServerAuthFailOperation::CancelOperation()
    {
    TRACE( "CServerAuthFailOperation::CancelOperation, begin" );
    Cancel();
    if( iUntrustedCertQuery )
        {
        TRACE( "CServerAuthFailOperation::CancelOperation, cancelling untrusted query" );
        iUntrustedCertQuery->Cancel();
        }
    if( !iMessage.IsNull() )
        {
        TRACE( "CServerAuthFailOperation::CancelOperation, completing message 0x%08x",
                iMessage.Handle() );
        iMessage.Complete( KErrCancel );
        }
    TRACE( "CServerAuthFailOperation::CancelOperation, end" );
    }

// ---------------------------------------------------------------------------
// CServerAuthFailOperation::RunL()
// ---------------------------------------------------------------------------
//
void CServerAuthFailOperation::RunL()
    {
    TRACE( "CServerAuthFailOperation::RunL, iStatus.Int()=%d, iMode=%d",
            iStatus.Int(), iMode );
    User::LeaveIfError( iStatus.Int() );
    switch( iMode )
        {
        case EInitialiseCertStore:
            ProcessServerAuthorizationFailureL();
            break;
        case EListTrustedSiteCerts:
            RetrieveFirstTrustedSiteCertL();
            break;
        case ERetrieveTrustedSiteCert:
            if( IsRetrievedCertSameAsServerCertL() )
                {
                ReturnResultL( EContinue );
                }
            else
                {
                RetrieveNextTrustedSiteCertL();
                }
            break;
        case ESavingServerCert:
            SaveServerNameToTrustedSitesStoreL();
            ReturnResultL( EContinue );
            break;
        default:
            User::Leave( KErrGeneral );
            break;
        }
    TRACE( "CServerAuthFailOperation::RunL(), end" );
    }

// ---------------------------------------------------------------------------
// CServerAuthFailOperation::DoCancel()
// ---------------------------------------------------------------------------
//
void CServerAuthFailOperation::DoCancel()
    {
    TRACE( "CServerAuthFailOperation::DoCancel, iMode=%d", iMode );
    switch( iMode )
        {
        case EInitialiseCertStore:
            if( iCertStore )
                {
                iCertStore->CancelInitialize();
                }
            break;
        case EListTrustedSiteCerts:
            if( iTrustedSiteCertStore )
                {
                iTrustedSiteCertStore->CancelList();
                }
            break;
        case ERetrieveTrustedSiteCert:
            if( iTrustedSiteCertStore )
                {
                iTrustedSiteCertStore->CancelRetrieve();
                }
            break;
        case ESavingServerCert:
            if( iTrustedSiteCertStore )
                {
                iTrustedSiteCertStore->CancelAdd();
                }
            break;
        default:
            break;
        }
    TRACE( "CServerAuthFailOperation::DoCancel, end" );
    }

// ---------------------------------------------------------------------------
// CServerAuthFailOperation::CServerAuthFailOperation()
// ---------------------------------------------------------------------------
//
CServerAuthFailOperation::CServerAuthFailOperation(
        MSecurityDialogOperationObserver& aObserver, const RMessage2& aMessage,
        TInt aReplySlot ) : CSecurityDialogOperation( aObserver, aMessage, aReplySlot ),
        iRetrievedCertBufPtr(0, 0)
    {
    TRACE( "CServerAuthFailOperation::CServerAuthFailOperation" );
    }

// ---------------------------------------------------------------------------
// CServerAuthFailOperation::InitializeUnifiedCertStoreL()
// ---------------------------------------------------------------------------
//
void CServerAuthFailOperation::InitializeUnifiedCertStoreL()
    {
    TRACE( "CServerAuthFailOperation::InitializeUnifiedCertStoreL" );
    User::LeaveIfError( iFs.Connect() );
    iCertStore = CUnifiedCertStore::NewL( iFs, ETrue );

    iCertStore->Initialize( iStatus );
    iMode = EInitialiseCertStore;
    SetActive();
    }

// ---------------------------------------------------------------------------
// CServerAuthFailOperation::ProcessServerAuthorizationFailureL()
// ---------------------------------------------------------------------------
//
void CServerAuthFailOperation::ProcessServerAuthorizationFailureL()
    {
    TRACE( "CServerAuthFailOperation::ProcessServerAuthorizationFailureL" );
    OpenTrustedSiteCertificateStoreL();
    if( IsAlreadyTrustedSiteL() )
        {
        StartFetchingTrustedSiteCertsL();
        }
    else
        {
        ShowUntrustedCertificateDialogL();
        }
    }

// ---------------------------------------------------------------------------
// CServerAuthFailOperation::OpenTrustedSiteCertificateStoreL()
// ---------------------------------------------------------------------------
//
void CServerAuthFailOperation::OpenTrustedSiteCertificateStoreL()
    {
    TRACE( "CServerAuthFailOperation::OpenTrustedSiteCertificateStoreL, begin" );
    TInt count = iCertStore->WritableCertStoreCount();
    for( TInt index = 0; ( index < count ) && !iTrustedSiteCertStore; index++ )
        {
        MCTWritableCertStore* certstore = &( iCertStore->WritableCertStore( index ) );
        TUid tokenTypeUid = certstore->Token().Handle().iTokenTypeUid;
        if( tokenTypeUid == KTrustedSiteCertificatesTokenTypeUid )
            {
            iTrustedSiteCertStore = certstore;
            }
        }
    TRACE( "CServerAuthFailOperation::OpenTrustedSiteCertificateStoreL, store 0x%08x",
            iTrustedSiteCertStore );
    }

// ---------------------------------------------------------------------------
// CServerAuthFailOperation::IsAlreadyTrustedSiteL()
// ---------------------------------------------------------------------------
//
TBool CServerAuthFailOperation::IsAlreadyTrustedSiteL()
    {
    TRACE( "CServerAuthFailOperation::IsAlreadyTrustedSiteL, begin" );
    TBool isTrustedSite = EFalse;
    if( iTrustedSiteCertStore )
        {
        CTrustSitesStore* trustedSitesStore = CTrustSitesStore::NewL();
        CleanupStack::PushL( trustedSitesStore );

        isTrustedSite = trustedSitesStore->IsTrustedSiteL( iEncodedServerCert, *iServerName );
        if( isTrustedSite )
            {
            TBool isExpiredAccepted = trustedSitesStore->IsOutOfDateAllowedL(
                    iEncodedServerCert, *iServerName );
            TRACE( "CServerAuthFailOperation::IsAlreadyTrustedSiteL, isExpiredAccepted=%d",
                    isExpiredAccepted );
            if( !isExpiredAccepted && iAuthFailReason == EDateOutOfRange )
                {
                TRACE( "CServerAuthFailOperation::IsAlreadyTrustedSiteL, not accepted" );
                isTrustedSite = EFalse;
                }
            }

        CleanupStack::PopAndDestroy( trustedSitesStore );
        }
    TRACE( "CServerAuthFailOperation::IsAlreadyTrustedSiteL, isTrustedSite=%d", isTrustedSite );
    return isTrustedSite;
    }

// ---------------------------------------------------------------------------
// CServerAuthFailOperation::StartFetchingTrustedSiteCertsL()
// ---------------------------------------------------------------------------
//
void CServerAuthFailOperation::StartFetchingTrustedSiteCertsL()
    {
    TRACE( "CServerAuthFailOperation::StartFetchingTrustedSiteCertsL" );
    ASSERT( iCertAttributeFilter == NULL );
    iCertAttributeFilter = CCertAttributeFilter::NewL();
    iCertAttributeFilter->SetOwnerType( EPeerCertificate );
    ASSERT( iTrustedSiteCertStore != NULL );
    iTrustedSiteCertStore->List( iCertInfos, *iCertAttributeFilter, iStatus );
    iMode = EListTrustedSiteCerts;
    SetActive();
    }

// ---------------------------------------------------------------------------
// CServerAuthFailOperation::ShowUntrustedCertificateDialogL()
// ---------------------------------------------------------------------------
//
void CServerAuthFailOperation::ShowUntrustedCertificateDialogL()
    {
    TRACE( "CServerAuthFailOperation::ShowUntrustedCertificateDialogL, begin" );

    // If trusted site certstore open has failed, then it is not possible to save
    // the host name for permanent use. Hence, choice for permanent accept is not
    // displayed if trusted site certstore open has failed. Other restrictions for
    // permanent accept are defined in device dialog (UntrustedCertificateWidget).
    TBool isTrustedSiteCertStoreOpened = ( iTrustedSiteCertStore != NULL );
    ASSERT( iUntrustedCertQuery == NULL );
    iUntrustedCertQuery = CUntrustedCertQuery::NewL( iAuthFailReason, iEncodedServerCert,
            *iServerName, isTrustedSiteCertStoreOpened );

    CUntrustedCertQuery::TResponse response = CUntrustedCertQuery::EQueryRejected;
    iUntrustedCertQuery->ShowQueryAndWaitForResponseL( response );
    switch( response )
        {
        case CUntrustedCertQuery::EQueryAccepted:
            ReturnResultL( EContinue );
            break;
        case CUntrustedCertQuery::EQueryAcceptedPermanently:
            SaveServerCertToTrustedSiteCertStoreL();
            break;
        case CUntrustedCertQuery::EQueryRejected:
        default:
            ReturnResultL( EStop );
            break;
        }

    TRACE( "CServerAuthFailOperation::ShowUntrustedCertificateDialogL, end" );
    }

// ---------------------------------------------------------------------------
// CServerAuthFailOperation::SaveServerCertToTrustedSiteCertStoreL()
// ---------------------------------------------------------------------------
//
void CServerAuthFailOperation::SaveServerCertToTrustedSiteCertStoreL()
    {
    TRACE( "CServerAuthFailOperation::SaveServerCertToTrustedSiteCertStoreL" );

    ASSERT( iCertLabel == NULL );
    CX509Certificate* serverCert = CX509Certificate::NewLC( iEncodedServerCert );
    const CX500DistinguishedName& dName = serverCert->SubjectName();
    HBufC* commonName = dName.ExtractFieldL( KX520CommonName );
    if( commonName )
        {
        CleanupStack::PushL( commonName );
        TInt commonNameLen = commonName->Length();
        if( commonNameLen > 0 && commonNameLen <= KMaxCommonNameLength )
            {
            iCertLabel = commonName->AllocL();
            }
        CleanupStack::PopAndDestroy( commonName );
        }
    CleanupStack::PopAndDestroy( serverCert );

    if( !iCertLabel )
        {
        iCertLabel = iServerName->AllocL();
        }

    iTrustedSiteCertStore->Add( *iCertLabel, EX509Certificate, EPeerCertificate,
            NULL, NULL, iEncodedServerCert, iStatus );
    iMode = ESavingServerCert;
    SetActive();
    }

// ---------------------------------------------------------------------------
// CServerAuthFailOperation::SaveServerNameToTrustedSitesStoreL()
// ---------------------------------------------------------------------------
//
void CServerAuthFailOperation::SaveServerNameToTrustedSitesStoreL()
    {
    TRACE( "CServerAuthFailOperation::SaveServerNameToTrustedSitesStoreL, begin" );
    CTrustSitesStore* trustedSitesStore = CTrustSitesStore::NewL();
    CleanupStack::PushL( trustedSitesStore );

    // CTrustSitesStore::AddL() may leave or it may return an error code.
    // It leaves if parameters are incorrect, and it returns error code if
    // saving fails. Both kinds of errors are handled here in the same way.
    TInt err = trustedSitesStore->AddL( iEncodedServerCert, *iServerName );
    TRACE( "CServerAuthFailOperation::SaveServerNameToTrustedSitesStoreL, err=%d", err );
    User::LeaveIfError( err );

    CleanupStack::PopAndDestroy( trustedSitesStore );
    }

// ---------------------------------------------------------------------------
// CServerAuthFailOperation::ReturnResultL()
// ---------------------------------------------------------------------------
//
void CServerAuthFailOperation::ReturnResultL( TServerAuthenticationFailureDialogResult aResult )
    {
    TRACE( "CServerAuthFailOperation::ReturnResultL, aResult=%d", aResult );
    TServerAuthenticationFailureOutputBuf output( aResult );
    iMessage.WriteL( iReplySlot, output );
    TRACE( "CServerAuthFailOperation::ReturnResultL, completing msg 0x%08x", iMessage.Handle() );
    iMessage.Complete( KErrNone );
    TRACE( "CServerAuthFailOperation::ReturnResultL, informing observer" );
    iObserver.OperationComplete();
    TRACE( "CServerAuthFailOperation::ReturnResultL, end" );
    }

// ---------------------------------------------------------------------------
// CServerAuthFailOperation::RetrieveFirstTrustedSiteCertL()
// ---------------------------------------------------------------------------
//
void CServerAuthFailOperation::RetrieveFirstTrustedSiteCertL()
    {
    TRACE( "CServerAuthFailOperation::RetrieveFirstTrustedSiteCertL" );
    ASSERT( iRetrievedCertBuffer == NULL );
    iRetrieveCertIndex = 0;

    RetrieveNextTrustedSiteCertL();
    }

// ---------------------------------------------------------------------------
// CServerAuthFailOperation::RetrieveNextTrustedSiteCertL()
// ---------------------------------------------------------------------------
//
void CServerAuthFailOperation::RetrieveNextTrustedSiteCertL()
    {
    TRACE( "CServerAuthFailOperation::RetrieveNextTrustedSiteCertL" );
    if( iRetrieveCertIndex < iCertInfos.Count() )
        {
        CCTCertInfo& cert = *( iCertInfos[ iRetrieveCertIndex ] );

        if( iRetrievedCertBuffer )
            {
            delete iRetrievedCertBuffer;
            iRetrievedCertBuffer = NULL;
            }
        iRetrievedCertBuffer = HBufC8::NewL( cert.Size() );
        iRetrievedCertBufPtr.Set( iRetrievedCertBuffer->Des() );

        iTrustedSiteCertStore->Retrieve( cert, iRetrievedCertBufPtr, iStatus );
        iMode = ERetrieveTrustedSiteCert;
        SetActive();
        }
    else
        {
        ShowUntrustedCertificateDialogL();
        }
    }

// ---------------------------------------------------------------------------
// CServerAuthFailOperation::IsRetrievedCertSameAsServerCertL()
// ---------------------------------------------------------------------------
//
TBool CServerAuthFailOperation::IsRetrievedCertSameAsServerCertL()
    {
    TRACE( "CServerAuthFailOperation::IsRetrievedCertSameAsServerCertL, begin" );
    TBool isSame = EFalse;
    CX509Certificate* cert = CX509Certificate::NewLC( *iRetrievedCertBuffer );

    if( !iServerCertFingerprint )
        {
        CX509Certificate* serverCert = CX509Certificate::NewLC( iEncodedServerCert );
        iServerCertFingerprint = serverCert->Fingerprint().AllocL();
        CleanupStack::PopAndDestroy( serverCert );
        }

    if( cert->Fingerprint() == *iServerCertFingerprint )
        {
        isSame = ETrue;
        }

    CleanupStack::PopAndDestroy( cert );
    TRACE( "CServerAuthFailOperation::IsRetrievedCertSameAsServerCertL, isSame=%d", isSame );
    return isSame;
    }

