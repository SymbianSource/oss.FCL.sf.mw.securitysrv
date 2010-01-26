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
* Description:   Implements a synchronous wrapper for easier use of Symbian's
*                Security Frameworks's API's.
*
*/


// INCLUDE FILES
#include <unifiedcertstore.h>
#include <unifiedkeystore.h>
#include <mctwritablecertstore.h>
#include <x509cert.h>
#include <pkixcertchain.h>
#include "CertmanuiSyncWrapper.h"
#include "CertmanuiCommon.h"
#include "CertManUILogger.h"

// CONSTANTS
_LIT_SECURITY_POLICY_C1( KKeyStoreUsePolicy, ECapabilityReadUserData );


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCertManUISyncWrapper::CCertManUISyncWrapper()
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CCertManUISyncWrapper::CCertManUISyncWrapper() : CActive( EPriorityStandard ), iCertPtr(0,0)
    {
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CCertManUISyncWrapper::ConstructL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CCertManUISyncWrapper::ConstructL()
    {
    }


// -----------------------------------------------------------------------------
// CCertManUISyncWrapper::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CCertManUISyncWrapper* CCertManUISyncWrapper::NewLC()
    {
    CCertManUISyncWrapper* wrap = new ( ELeave ) CCertManUISyncWrapper();
    CleanupStack::PushL( wrap );
    wrap->ConstructL();
    return wrap;
    }

// -----------------------------------------------------------------------------
// CCertManUISyncWrapper::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CCertManUISyncWrapper* CCertManUISyncWrapper::NewL()
    {
    CCertManUISyncWrapper* wrap = CCertManUISyncWrapper::NewLC();
    CleanupStack::Pop();
    return wrap;
    }

// Destructor
CCertManUISyncWrapper::~CCertManUISyncWrapper()
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUISyncWrapper::~CCertManUISyncWrapper()" );
    CERTMANUILOGGER_LEAVEFN( "CCertManUISyncWrapper::~CCertManUISyncWrapper()" );
    }


// -----------------------------------------------------------------------------
// CCertManUISyncWrapper::DoCancel
// Cancels the ongoing operation if possible.
// -----------------------------------------------------------------------------
//
void CCertManUISyncWrapper::DoCancel()
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUISyncWrapper::DoCancel" );

    switch ( iOperation )
        {
        case EOperationInit:
            {
            LOG_WRITE( "CCertManUISyncWrapper::DoCancel EOperationInit" );
            iStore->CancelInitialize();
            break;
            }
        case EOperationList:
            {
            LOG_WRITE( "CCertManUISyncWrapper::DoCancel EOperationList" );
            iStore->CancelList();
            break;
            }
        case EGetCertificate:
            {
            LOG_WRITE( "CCertManUISyncWrapper::DoCancel EGetCertificate" );
            iStore->CancelRetrieve();
            break;
            }
        case EAddCertificate:
            {
            LOG_WRITE( "CCertManUISyncWrapper::DoCancel EAddCertificate" );
            iStore->Cancel();
            break;
            }
        case ERetriveURLCertificate:
            {
            LOG_WRITE(" CCertManUISyncWrapper::DoCancel ERetriveURLCertificate ");
            iStore->CancelRetrieve();
            break;
            }
        case EOperationDelete:
            {
            LOG_WRITE( "CCertManUISyncWrapper::DoCancel EOperationDelete" );
            iStore->CancelRemove();
            break;
            }
        case EOperationGetApps:
            {
            LOG_WRITE( "CCertManUISyncWrapper::DoCancel EOperationGetApps" );
            iStore->CancelApplications();
            break;
            }
        case EOperationValidateX509Root:
            {
            LOG_WRITE( "CCertManUISyncWrapper::DoCancel EOperationValidateX509Root" );
            iChain->CancelValidate();
            break;
            }
        case EShowErrorNote:
            {
            LOG_WRITE( "CCertManUISyncWrapper::DoCancel EShowErrorNote" );
            break;
            }
        case EOperationInitKeyStore:
            {
            LOG_WRITE( "CCertManUISyncWrapper::DoCancel EOperationInitKeyStore" );
            iKeyStore->CancelInitialize();
            break;
            }
        case EOperationKeyList:
            {
            LOG_WRITE( "CCertManUISyncWrapper::DoCancel EOperationKeyList" );
            iKeyStore->CancelList();
            break;
            }
        case EOperationGetInterface:
            {
            LOG_WRITE( "CCertManUISyncWrapper::DoCancel EOperationGetInterface" );
            iToken->CancelGetInterface();
            break;
            }
        case EOperationExportKey:
            {
            LOG_WRITE( "CCertManUISyncWrapper::DoCancel EOperationExportKey" );
            iKeyStore->CancelExportKey();
            break;
            }
        case EOperationImportKey:
            {
            LOG_WRITE( "CCertManUISyncWrapper::DoCancel EOperationImportKey" );
            iKeyStore->CancelImportKey();
            break;
            }
        case EOperationDeleteKey:
            {
            LOG_WRITE( "CCertManUISyncWrapper::DoCancel EOperationDeleteKey" );
            iKeyStore->CancelDeleteKey();
            break;
            }
        default:
            {
            break;
            }
        }

    CERTMANUILOGGER_LEAVEFN( "CCertManUISyncWrapper::DoCancel" );
    }

// -----------------------------------------------------------------------------
// CCertManUISyncWrapper::InitStoreL
// -----------------------------------------------------------------------------
//
TInt CCertManUISyncWrapper::InitStoreL( CUnifiedCertStore*& aStore )
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUISyncWrapper::InitStoreL Certstore" );

    aStore->Initialize( iStatus );
    iOperation = EOperationInit;
    iStore = aStore;
    SetActive();
    iWait.Start();
    HandleErrorL();
    iOperation = EOperationNone;

    CERTMANUILOGGER_LEAVEFN( "CCertManUISyncWrapper::InitStoreL Certstore" );
    return iStatus.Int();
    }

// -----------------------------------------------------------------------------
// CCertManUISyncWrapper::InitStoreL
// -----------------------------------------------------------------------------
//
TInt CCertManUISyncWrapper::InitStoreL( CUnifiedKeyStore*& aStore  )
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUISyncWrapper::InitStoreL Keystore" );

    aStore->Initialize( iStatus );
    iOperation = EOperationInitKeyStore;
    iKeyStore = aStore;
    SetActive();
    iWait.Start();
    HandleErrorL();
    iOperation = EOperationNone;

    CERTMANUILOGGER_LEAVEFN( "CCertManUISyncWrapper::InitStoreL Keystore" );
    return iStatus.Int();
    }

// -----------------------------------------------------------------------------
// CCertManUISyncWrapper::ListL
// -----------------------------------------------------------------------------
//
TInt CCertManUISyncWrapper::ListL(
    CUnifiedCertStore*& aStore,
    RMPointerArray<CCTCertInfo>* aArray,
    const CCertAttributeFilter& aFilter )
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUISyncWrapper::ListL 1" );

    if ( IsActive() )
        {
        // Wrapper is active. Don't go further
        return KErrGeneral;
        }

    aStore->List( *aArray, aFilter, iStatus );
    iOperation = EOperationList;
    iStore = aStore;
    SetActive();
    iWait.Start();
    HandleErrorL();
    iOperation = EOperationNone;

    CERTMANUILOGGER_LEAVEFN( "CCertManUISyncWrapper::ListL 1" );
    return iStatus.Int();
    }


// -----------------------------------------------------------------------------
// CCertManUISyncWrapper::ListL
// -----------------------------------------------------------------------------
//
TInt CCertManUISyncWrapper::ListL(
    CUnifiedCertStore*& aStore,
    RMPointerArray<CCTCertInfo>* aArray,
    const CCertAttributeFilter& aFilter,
    const TUid aTokenUid )
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUISyncWrapper::ListL 2" );

    TInt status( KErrNone );

    if ( IsActive() )
        {
        // Wrapper is active. Don't go further
        return KErrGeneral;
        }

    TInt count = aStore->CertStoreCount();
    for ( TInt ii = 0; ii < count; ii++ )
        {
        MCTCertStore& certstore = aStore->CertStore( ii );
        MCTToken& token = certstore.Token();
        TUid tokenuid = token.Handle().iTokenTypeUid;
        if ( tokenuid == aTokenUid )
            {
            certstore.List( *aArray, aFilter, iStatus );
            iOperation = EOperationList;
            iStore = aStore;
            SetActive();
            iWait.Start();
            HandleErrorL();
            status = iStatus.Int();
            break;
            }
        }

    iOperation = EOperationNone;

    CERTMANUILOGGER_LEAVEFN( "CCertManUISyncWrapper::ListL 2" );
    return status;
    }

// -----------------------------------------------------------------------------
// CCertManUISyncWrapper::ListL
// -----------------------------------------------------------------------------
//
TInt CCertManUISyncWrapper::ListL(
    CUnifiedKeyStore*& aStore,
    RMPointerArray<CCTKeyInfo>* aArray,
    const TCTKeyAttributeFilter& aFilter )
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUISyncWrapper::ListL 3" );

    if ( IsActive() )
        {
        // Wrapper is active. Don't go further
        return KErrGeneral;
        }

    aStore->List( *aArray, aFilter, iStatus );
    iOperation = EOperationKeyList;
    iKeyStore = aStore;
    SetActive();
    iWait.Start();
    HandleErrorL();

    iOperation = EOperationNone;
    CERTMANUILOGGER_LEAVEFN( "CCertManUISyncWrapper::ListL 3" );
    return iStatus.Int();
    }


// -----------------------------------------------------------------------------
// CCertManUISyncWrapper::ListL
// -----------------------------------------------------------------------------
//
TInt CCertManUISyncWrapper::ListL(
    CUnifiedKeyStore*& aStore,
    RMPointerArray<CCTKeyInfo>* aArray,
    const TCTKeyAttributeFilter& aFilter,
    const TUid aTokenUid )
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUISyncWrapper::ListL 4" );

    TInt status( KErrNone );

    if ( IsActive() )
        {
        // Wrapper is active. Don't go further
        return KErrGeneral;
        }

    TInt count = aStore->KeyStoreManagerCount();
    for ( TInt ii = 0; ii < count; ii++ )
        {
        MCTKeyStoreManager& keystoremanager = aStore->KeyStoreManager( ii );
        MCTToken& token = keystoremanager.Token();
        TUid tokenuid = token.Handle().iTokenTypeUid;
        if ( tokenuid == aTokenUid )
            {
            keystoremanager.List( *aArray, aFilter, iStatus );
            iOperation = EOperationKeyList;
            iKeyStore = aStore;
            SetActive();
            iWait.Start();
            HandleErrorL();
            status = iStatus.Int();
            break;
            }
        }

    iOperation = EOperationNone;

    CERTMANUILOGGER_LEAVEFN( "CCertManUISyncWrapper::ListL 4" );
    return status;
    }

// -----------------------------------------------------------------------------
// CCertManUISyncWrapper::GetCertificateL
// -----------------------------------------------------------------------------
//
TInt CCertManUISyncWrapper::GetCertificateL(
    CUnifiedCertStore*& aStore,
    const CCTCertInfo& aCertInfo,
    CCertificate*& aCert )
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUISyncWrapper::GetCertificateL 1" );

    aCert = NULL;
    HBufC8* buf = HBufC8::NewLC( KMaxCertificateLength );
    iCertPtr.Set( buf->Des() );
    aStore->Retrieve( aCertInfo, iCertPtr, iStatus);
    iOperation = EGetCertificate;
    iStore = aStore;
    SetActive();
    iWait.Start();

    if ( iStatus.Int() == KErrNone )
        {
        switch ( aCertInfo.CertificateFormat() )
            {
            case EX509Certificate:
                {
                aCert = CX509Certificate::NewL( iCertPtr );
                break;
                }
            case EX509CertificateUrl:
                {
#ifdef  _CERTMANUI_LOG_
                TPtrC8 dump = buf->Des();
                LOG_HEXDUMP(dump);
#endif
                break;
                }
            default:
                {
                break;
                }
            }
        }

    CleanupStack::PopAndDestroy();  // buf
    HandleErrorL();
    iOperation = EOperationNone;

    CERTMANUILOGGER_LEAVEFN( "CCertManUISyncWrapper::GetCertificateL 1" );
    return iStatus.Int();
    }

// -----------------------------------------------------------------------------
// CCertManUISyncWrapper::GetUrlCertificateL
// -----------------------------------------------------------------------------
//
TInt CCertManUISyncWrapper::GetUrlCertificateL(
    CUnifiedCertStore*& aStore,
    const CCTCertInfo& aCertInfo,
    TDes8& aUrl )
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUISyncWrapper::GetUrlCertificateL" );

    aStore->Retrieve( aCertInfo, aUrl, iStatus);
    iOperation = ERetriveURLCertificate;
    iStore = aStore;
    SetActive();
    iWait.Start();
    HandleErrorL();
    iOperation = EOperationNone;

    CERTMANUILOGGER_LEAVEFN( "CCertManUISyncWrapper::GetUrlCertificateL" );
    return iStatus.Int();
    }


// -----------------------------------------------------------------------------
// CCertManUISyncWrapper::GetCertificateL
// -----------------------------------------------------------------------------
//
TInt CCertManUISyncWrapper::GetCertificateL( CUnifiedCertStore*& aStore,
    const CCTCertInfo& aCertInfo,
    CCertificate*& aCert,
    TUid aTokenUid )
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUISyncWrapper::GetCertificateL 2 " );

    aCert = NULL;
    TInt status( KErrNone );

    HBufC8* buf = HBufC8::NewLC( KMaxCertificateLength );
    iCertPtr.Set( buf->Des() );

    TInt count = aStore->CertStoreCount();
    for (TInt i = 0; i < count; i++)
        {
        MCTCertStore& certstore = aStore->CertStore( i );
        MCTToken& token = certstore.Token();
        TUid tokenuid = token.Handle().iTokenTypeUid;
        if ( tokenuid == aTokenUid )
            {
            certstore.Retrieve( aCertInfo, iCertPtr, iStatus );
            iOperation = EGetCertificate;
            iStore = aStore;
            SetActive();
            iWait.Start();
            HandleErrorL();
            status = iStatus.Int();
            break;
            }
        }

    if ( status == KErrNone )
        {
        switch ( aCertInfo.CertificateFormat() )
            {
            case EX509Certificate:
                {
                aCert = CX509Certificate::NewL( iCertPtr );
                break;
                }
            case EX509CertificateUrl:
                {
#ifdef  _CERTMANUI_LOG_
                TPtrC8 dump = buf->Des();
                LOG_HEXDUMP( dump );
#endif
                break;
                }
            default:
                {
                break;
                }
            }
        }

    CleanupStack::PopAndDestroy();  // buf
    iOperation = EOperationNone;

    CERTMANUILOGGER_LEAVEFN( "CCertManUISyncWrapper::GetCertificateL 2" );
    return status;
    }


// -----------------------------------------------------------------------------
// CCertManUISyncWrapper::DeleteCertL
// -----------------------------------------------------------------------------
//
TInt CCertManUISyncWrapper::DeleteCertL( CUnifiedCertStore*& aStore,
    const CCTCertInfo& aCertInfo )
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUISyncWrapper::DeleteCertL 1" );

    aStore->Remove( aCertInfo, iStatus );
    iOperation = EOperationDelete;
    iStore = aStore;
    SetActive();
    iWait.Start();
    HandleErrorL();
    iOperation = EOperationNone;

    CERTMANUILOGGER_LEAVEFN( "CCertManUISyncWrapper::DeleteCertL 1" );
    return iStatus.Int();
    }

// -----------------------------------------------------------------------------
// CCertManUISyncWrapper::DeleteCertL
// -----------------------------------------------------------------------------
//
TInt CCertManUISyncWrapper::DeleteCertL( CUnifiedCertStore*& aStore,
    const CCTCertInfo& aCertInfo,
    TUid aTokenUid )
    {
    TInt status( KErrNone );
    CERTMANUILOGGER_ENTERFN( "CCertManUISyncWrapper::DeleteCertL 2" );

    TInt count = aStore->WritableCertStoreCount();
    for (TInt i = 0; i < count; i++)
        {
        MCTWritableCertStore& writablestore = aStore->WritableCertStore( i );
        MCTToken& token = writablestore.Token();
        TUid tokenuid = token.Handle().iTokenTypeUid;
        if ( tokenuid == aTokenUid )
            {
            writablestore.Remove( aCertInfo, iStatus );
            iOperation = EOperationDelete;
            iStore = aStore;
            SetActive();
            iWait.Start();
            HandleErrorL();
            status = iStatus.Int();
            break;
            }
        }

    iOperation = EOperationNone;

    CERTMANUILOGGER_LEAVEFN( "CCertManUISyncWrapper::DeleteCertL 2" );
    return status;
    }

// -----------------------------------------------------------------------------
// CCertManUISyncWrapper::GetApplicationsL
// -----------------------------------------------------------------------------
//
TInt CCertManUISyncWrapper::GetApplicationsL( CUnifiedCertStore*& aStore,
    const CCTCertInfo& aCertInfo, RArray<TUid>& aApps )
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUISyncWrapper::GetApplicationsL" );

    aStore->Applications( aCertInfo, aApps, iStatus );
    iOperation = EOperationGetApps;
    iStore = aStore;
    SetActive();
    iWait.Start();
    HandleErrorL();
    iOperation = EOperationNone;

    CERTMANUILOGGER_LEAVEFN( "CCertManUISyncWrapper::GetApplicationsL" );
    return iStatus.Int();
    }

// -----------------------------------------------------------------------------
// CCertManUISyncWrapper::IsApplicableL
// -----------------------------------------------------------------------------
//
TInt CCertManUISyncWrapper::IsApplicableL( CUnifiedCertStore*& aStore,
    const CCTCertInfo& aCertInfo, TUid aApplication,
    TBool& aIsApplicable )
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUISyncWrapper::IsApplicableL" );

    aStore->IsApplicable( aCertInfo, aApplication, aIsApplicable, iStatus );
    iOperation = EOperationIsApplicable;
    iStore = aStore;
    SetActive();
    iWait.Start();
    HandleErrorL();
    iOperation = EOperationNone;

    CERTMANUILOGGER_LEAVEFN( "CCertManUISyncWrapper::IsApplicableL" );
    return iStatus.Int();
    }

// -----------------------------------------------------------------------------
// CCertManUISyncWrapper::IsTrustedL
// -----------------------------------------------------------------------------
//
TInt CCertManUISyncWrapper::IsTrustedL( CUnifiedCertStore*& aStore,
    const CCTCertInfo& aCertInfo, TBool& aTrusted )
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUISyncWrapper::IsTrustedL" );

    aStore->Trusted( aCertInfo, aTrusted, iStatus );
    iOperation = EOperationIsTrusted;
    iStore = aStore;
    SetActive();
    iWait.Start();
    HandleErrorL();
    iOperation = EOperationNone;

    CERTMANUILOGGER_LEAVEFN( "CCertManUISyncWrapper::IsTrustedL" );
    return iStatus.Int();
    }

// -----------------------------------------------------------------------------
// CCertManUISyncWrapper::SetApplicabilityL
// -----------------------------------------------------------------------------
//
TInt CCertManUISyncWrapper::SetApplicabilityL( CUnifiedCertStore*& aStore,
    const CCTCertInfo& aCertInfo,
    RArray<TUid>& aApplications )
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUISyncWrapper::SetApplicabilityL" );

    aStore->SetApplicability( aCertInfo, aApplications, iStatus );
    iOperation = EOperationSetApplicability;
    iStore = aStore;
    SetActive();
    iWait.Start();

    if ( !(iStatus.Int()) )
        {
        aStore->SetTrust( aCertInfo, ETrue, iStatus );
        iOperation = EOperationSetToTrusted;
        SetActive();
        iWait.Start();
        }
    HandleErrorL();
    iOperation = EOperationNone;

    CERTMANUILOGGER_LEAVEFN( "CCertManUISyncWrapper::SetApplicabilityL" );
    return iStatus.Int();
    }

// -----------------------------------------------------------------------------
// CCertManUISyncWrapper::MoveKeyL
// -----------------------------------------------------------------------------
//
void CCertManUISyncWrapper::MoveKeyL(
    CUnifiedKeyStore*& aStore,
    const TCTKeyAttributeFilter& aFilter,
    const TUid aSourceTokenId,
    const TUid aTargetTokenId )
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUISyncWrapper::MoveKeyL" );

    TInt sourceIndex(-1);
    TInt targetIndex(-1);

    // Find the index of key stores
    TInt count = aStore->KeyStoreManagerCount();

    for (TInt ii = 0; ii < count; ii++)
        {
        MCTKeyStoreManager& keystoremanager = aStore->KeyStoreManager( ii );
        MCTToken& token = keystoremanager.Token();
        TUid tokenuid = token.Handle().iTokenTypeUid;

        if ( tokenuid == aSourceTokenId )
            {
            sourceIndex = ii;
            }

        if ( tokenuid == aTargetTokenId )
            {
            targetIndex = ii;
            }
        }

    if (( sourceIndex == -1 ) || ( targetIndex == -1 ))
        {
        // Key store(s) doesn't exist
        User::Leave( KErrNotFound );
        }

    RMPointerArray<CCTKeyInfo> keyEntries;

    MCTKeyStoreManager& sourcekeystore =
                            aStore->KeyStoreManager( sourceIndex );

    MCTKeyStoreManager& targetkeystore =
                            aStore->KeyStoreManager( targetIndex );


    ListL( aStore, &keyEntries, aFilter, aSourceTokenId );

    // Go through all matching keys and move them to
    // target store
    for ( TInt ii = 0; ii < keyEntries.Count(); ii++ )
        {
        HBufC8* keyData = HBufC8::NewLC( KMaxKeyLength );

        // Retrieve key from source key store
        sourcekeystore.ExportKey( (*keyEntries[ii]).Handle(), keyData, iStatus );
        iOperation = EOperationExportKey;
        SetActive();
        iWait.Start();
        User::LeaveIfError( iStatus.Int() );

        TCTTokenObjectHandle sourceKeyHandle = (*keyEntries[ii]).Handle();

        // Import key to target key store

        // If key info access type indicates that key is local, then importing is
        // not possible. The following is the workarround. Almost identical
        // copy of keyinfo is created without CCTKeyInfo::ELocal access type flag.
        // UsePolicy is also updated
        TInt accessType = (*keyEntries[ii]).AccessType();
        if ( accessType & CCTKeyInfo::ELocal )
            {
            // CCTKeyInfo::ELocal is set in key info
            HBufC* label = (*keyEntries[ii]).Label().AllocLC();

            // The following XOR operation will clear local bit if it is on.
            accessType ^= CCTKeyInfo::ELocal;

            CCTKeyInfo* keyInfo = CCTKeyInfo::NewL( (*keyEntries[ii]).ID(),
                                (*keyEntries[ii]).Usage(),
                                (*keyEntries[ii]).Size(),
                                NULL,
                                label,
                                (*keyEntries[ii]).Token(),
                                (*keyEntries[ii]).HandleID(),
                                KKeyStoreUsePolicy,
                                (*keyEntries[ii]).ManagementPolicy(),
                                (*keyEntries[ii]).Algorithm(),
                                accessType,
                                (*keyEntries[ii]).Native(),
                                (*keyEntries[ii]).StartDate(),
                                (*keyEntries[ii]).EndDate() );

            CleanupStack::Pop(label);
            targetkeystore.ImportKey( *keyData, keyInfo, iStatus );
            }
        else
            {
            targetkeystore.ImportKey( *keyData, keyEntries[ii], iStatus );
            }

        iOperation = EOperationImportKey;
        SetActive();
        iWait.Start();
        User::LeaveIfError( iStatus.Int() );

        // Delete key from source key store
        sourcekeystore.DeleteKey( sourceKeyHandle, iStatus );
        iOperation = EOperationDeleteKey;
        SetActive();
        iWait.Start();
        User::LeaveIfError( iStatus.Int() );

        CleanupStack::PopAndDestroy();  // keyData
        }

    CERTMANUILOGGER_LEAVEFN( "CCertManUISyncWrapper::MoveKeyL" );
    }


// -----------------------------------------------------------------------------
// CCertManUISyncWrapper::MoveCertL
// -----------------------------------------------------------------------------
//
TInt CCertManUISyncWrapper::MoveCertL(
    CUnifiedCertStore*& aStore,
    const CCTCertInfo& aCertInfo,
    const TUid aSourceTokenId,
    const TUid aTargetTokenId )
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUISyncWrapper::MoveCertL" );

    TInt sourceIndex(-1);
    TInt targetIndex(-1);
    TInt certCount (0);

    // Find the index of certificate stores
    TInt count = aStore->WritableCertStoreCount();
    for (TInt ii = 0; ii < count; ii++)
        {
        MCTWritableCertStore& writablestore = aStore->WritableCertStore( ii );
        MCTToken& token = writablestore.Token();
        TUid tokenuid = token.Handle().iTokenTypeUid;

        if ( tokenuid == aSourceTokenId )
            {
            sourceIndex = ii;
            }

        if ( tokenuid == aTargetTokenId )
            {
            targetIndex = ii;
            }
        }

    if (( sourceIndex == -1 ) || ( targetIndex == -1 ))
        {
        // Certificate store(s) doesn't exist
        User::Leave( KErrNotFound );
        }


    MCTWritableCertStore& sourcewritablestore =
                    aStore->WritableCertStore( sourceIndex );

    // All of the certificates that are associated with same
    // private key will be moved to target certificate store.
    CCertAttributeFilter* filter = CCertAttributeFilter::NewL();
    filter->SetOwnerType( EUserCertificate );
    filter->SetSubjectKeyId( aCertInfo.SubjectKeyId() );
    RMPointerArray<CCTCertInfo> certEntries;

    // List certificates from source certificate store
    ListL( aStore, &certEntries, *filter, aSourceTokenId );

    delete filter;

    for ( TInt ii = 0; ii < certEntries.Count(); ii++ )
        {
        // Retrieve certificate from source certificate store
        HBufC8* buf = HBufC8::NewLC( KMaxCertificateLength );
        iCertPtr.Set( buf->Des() );
        sourcewritablestore.Retrieve( *certEntries[ii], iCertPtr, iStatus );
        iOperation = EGetCertificate;
        SetActive();
        iWait.Start();
        User::LeaveIfError( iStatus.Int() );

        // Add certificate to target certificate store
        MCTWritableCertStore& targetwritablestore =
                        aStore->WritableCertStore( targetIndex );

        targetwritablestore.Add( (*certEntries[ii]).Label(), EX509Certificate,
                      EUserCertificate, &((*certEntries[ii]).SubjectKeyId()),
                      &((*certEntries[ii]).IssuerKeyId()), *buf, iStatus );

        iOperation = EAddCertificate;
        SetActive();
        iWait.Start();
        User::LeaveIfError( iStatus.Int() );

        // Delete certificate from source certificate store
        sourcewritablestore.Remove( *certEntries[ii], iStatus );
        iOperation = EOperationDelete;
        iStore = aStore;
        SetActive();
        iWait.Start();
        HandleErrorL();
        iOperation = EOperationNone;
        User::LeaveIfError( iStatus.Int() );
        certCount++;

        CleanupStack::PopAndDestroy();  // buf
        }

    CERTMANUILOGGER_LEAVEFN( "CCertManUISyncWrapper::MoveCertL" );
    return certCount;
    }

// -----------------------------------------------------------------------------
// CCertManUISyncWrapper::ValidateX509RootCertificateL
// -----------------------------------------------------------------------------
//
TInt CCertManUISyncWrapper::ValidateX509RootCertificateL(
    CPKIXValidationResult*& aValidationResult,
    const TTime& aValidationTime, CPKIXCertChain* aChain )
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUISyncWrapper::ValidateX509RootCertificateL" );

    aChain->ValidateL( *aValidationResult, aValidationTime, iStatus );
    iOperation = EOperationValidateX509Root;
    iChain = aChain;
    SetActive();
    iWait.Start();
    iOperation = EOperationNone;

    CERTMANUILOGGER_LEAVEFN( "CCertManUISyncWrapper::ValidateX509RootCertificateL" );
    return iStatus.Int();
    }

// -----------------------------------------------------------------------------
// CCertManUISyncWrapper::GetInterface
// -----------------------------------------------------------------------------
//
TInt CCertManUISyncWrapper::GetInterface(
    TUid aRequiredInterface, MCTToken& aToken,
    MCTTokenInterface*& aReturnedInterface)
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUISyncWrapper::GetInterface" );

    iToken = &aToken;
    aToken.GetInterface( aRequiredInterface, aReturnedInterface, iStatus );
    iOperation = EOperationGetInterface;
    SetActive();
    iWait.Start();
    iOperation = EOperationNone;

    CERTMANUILOGGER_LEAVEFN( "CCertManUISyncWrapper::GetInterface" );
    return iStatus.Int();
    }

// -----------------------------------------------------------------------------
// CCertManUISyncWrapper::RunL
// If no errors happened, stop. Show an error note if needed.
// -----------------------------------------------------------------------------
//

void CCertManUISyncWrapper::RunL()
    {
    CERTMANUILOGGER_ENTERFN("CCertManUISyncWrapper::RunL");

    if ( iWait.IsStarted() )
        {
        iWait.AsyncStop();
        }

    CERTMANUILOGGER_LEAVEFN( "CCertManUISyncWrapper::RunL" );
    }

// -----------------------------------------------------------------------------
// CCertManUISyncWrapper::HandleErrorL
// Shows an error note according to status of operation,
// -----------------------------------------------------------------------------
//
void CCertManUISyncWrapper::HandleErrorL()
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUISyncWrapper::HandleErrorL" );

    TInt status = iStatus.Int();
    switch ( status )
        {
        case KErrNone:
            {
            LOG_WRITE( "No error" );
            break;
            }
        case KErrCancel:
            {
            LOG_WRITE( "HandleErrorL : KErrCancel" );
            // Up to caller decide if note is needed. No error note shown.
            break;
            }
        case KErrNoMemory:
        case KErrBadHandle:
        case KErrAlreadyExists:
        case KErrNotSupported:
        case EShowErrorNote:
            {
            LOG_WRITE_FORMAT( "HandleErrorL LEAVE : error code %i", status );
            User::Leave( iStatus.Int() );
            break;
            }
        default:
            {
            LOG_WRITE_FORMAT( "HandleErrorL LEAVE (default) : error code %i", status );
            break;
            }
        }

    CERTMANUILOGGER_LEAVEFN( "CCertManUISyncWrapper::HandleErrorL" );
    }


// End of File

