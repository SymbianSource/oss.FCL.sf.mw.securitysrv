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
#include <../../inc/cpsecplugins.h>
#include "cpcertmanuisyncwrapper.h"

// CONSTANTS
_LIT_SECURITY_POLICY_C1( KKeyStoreUsePolicy, ECapabilityReadUserData );
// Maximum length of a certificate
const TInt KMaxCertificateLength = 5000;
const TInt KMaxKeyLength = 10000;

// -----------------------------------------------------------------------------
// CpCertManUISyncWrapper::ListL
// -----------------------------------------------------------------------------
//
void CpCertManUISyncWrapper::ListL(
    CUnifiedCertStore*& aStore,
    RMPointerArray<CCTCertInfo>* aArray,
    const CCertAttributeFilter& aFilter,
    const TUid aTokenUid )
    {
    RDEBUG("0", 0);
    if ( IsActive() )
        {
        // Wrapper is active. Don't go further
        User::Leave(KErrGeneral);
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
            User::LeaveIfError( iStatus.Int() );
            break;
            }
        }
    iOperation = EOperationNone;
    }

// -----------------------------------------------------------------------------
// CpCertManUISyncWrapper::ListL
// -----------------------------------------------------------------------------
//
void CpCertManUISyncWrapper::ListL(
    CUnifiedKeyStore*& aStore,
    RMPointerArray<CCTKeyInfo>* aArray,
    const TCTKeyAttributeFilter& aFilter )
    {
    RDEBUG("0", 0);
    if ( IsActive() )
        {
        // Wrapper is active. Don't go further
		User::Leave(KErrGeneral);
        }

    aStore->List( *aArray, aFilter, iStatus );
    iOperation = EOperationKeyList;
    iKeyStore = aStore;
    SetActive();
    iWait.Start();
    User::LeaveIfError( iStatus.Int() );
    iOperation = EOperationNone;
    }

// -----------------------------------------------------------------------------
// CpCertManUISyncWrapper::GetCertificateL
// -----------------------------------------------------------------------------
//
void CpCertManUISyncWrapper::GetCertificateL( CUnifiedCertStore*& aStore,
    const CCTCertInfo& aCertInfo,
    CCertificate*& aCert,
    TUid aTokenUid )
    {
    aCert = NULL;
    RDEBUG("0", 0);
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
            User::LeaveIfError( iStatus.Int() );
            break;
            }
        }

    
	switch ( aCertInfo.CertificateFormat() )
		{
		case EX509Certificate:
			{
			aCert = CX509Certificate::NewL( iCertPtr );
			break;
			}
		case EX509CertificateUrl:
			{
			break;
			}
		default:
			{
			break;
			}
		}
        
    CleanupStack::PopAndDestroy(buf);  
    iOperation = EOperationNone;

    }

// -----------------------------------------------------------------------------
// CpCertManUISyncWrapper::DeleteCertL
// -----------------------------------------------------------------------------
//
void CpCertManUISyncWrapper::DeleteCertL( CUnifiedCertStore*& aStore,
    const CCTCertInfo& aCertInfo )
    {
    RDEBUG("0", 0);
    aStore->Remove( aCertInfo, iStatus );
    iOperation = EOperationDelete;
    iStore = aStore;
    SetActive();
    iWait.Start();
    User::LeaveIfError( iStatus.Int() );
    iOperation = EOperationNone;
    }

// -----------------------------------------------------------------------------
// CpCertManUISyncWrapper::DeleteCertL
// -----------------------------------------------------------------------------
//
void CpCertManUISyncWrapper::DeleteCertL( CUnifiedCertStore*& aStore,
    const CCTCertInfo& aCertInfo,
    TUid aTokenUid )
    {
    RDEBUG("0", 0);
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
            User::LeaveIfError( iStatus.Int() );
            break;
            }
        }
    iOperation = EOperationNone;
    }

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CpCertManUISyncWrapper::CpCertManUISyncWrapper()
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CpCertManUISyncWrapper::CpCertManUISyncWrapper() : CActive( EPriorityStandard ), iCertPtr(0,0)
    {
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CpCertManUISyncWrapper::ConstructL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CpCertManUISyncWrapper::ConstructL()
    {
    }


// -----------------------------------------------------------------------------
// CpCertManUISyncWrapper::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CpCertManUISyncWrapper* CpCertManUISyncWrapper::NewLC()
    {
    CpCertManUISyncWrapper* wrap = new ( ELeave ) CpCertManUISyncWrapper();
    CleanupStack::PushL( wrap );
    wrap->ConstructL();
    return wrap;
    }

// -----------------------------------------------------------------------------
// CpCertManUISyncWrapper::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CpCertManUISyncWrapper* CpCertManUISyncWrapper::NewL()
    {
    CpCertManUISyncWrapper* wrap = CpCertManUISyncWrapper::NewLC();
    CleanupStack::Pop(wrap);
    return wrap;
    }

// Destructor
CpCertManUISyncWrapper::~CpCertManUISyncWrapper()
    {
    }

// -----------------------------------------------------------------------------
// CpCertManUISyncWrapper::RunL
// If no errors happened, stop. Show an error note if needed.
// -----------------------------------------------------------------------------
//

void CpCertManUISyncWrapper::RunL()
    {
    RDEBUG("0", 0);
    if ( iWait.IsStarted() )
        {
        iWait.AsyncStop();
        }

    }

// -----------------------------------------------------------------------------
// CpCertManUISyncWrapper::DoCancel
// Cancels the ongoing operation if possible.
// -----------------------------------------------------------------------------
//
void CpCertManUISyncWrapper::DoCancel()
    {
    RDEBUG("iOperation", iOperation);
    switch ( iOperation )
        {
        case EOperationInit:
            {
            iStore->CancelInitialize();
            break;
            }
        case EOperationList:
            {
            iStore->CancelList();
            break;
            }
        case EGetCertificate:
            {
            iStore->CancelRetrieve();
            break;
            }
        case EAddCertificate:
            {
            iStore->Cancel();
            break;
            }
        case ERetriveURLCertificate:
            {
            iStore->CancelRetrieve();
            break;
            }
        case EOperationDelete:
            {
            iStore->CancelRemove();
            break;
            }
        case EOperationGetApps:
            {
            iStore->CancelApplications();
            break;
            }
        case EOperationValidateX509Root:
            {
            iChain->CancelValidate();
            break;
            }
        case EShowErrorNote:
            {
            break;
            }
        case EOperationInitKeyStore:
            {
            iKeyStore->CancelInitialize();
            break;
            }
        case EOperationKeyList:
            {
            iKeyStore->CancelList();
            break;
            }
        case EOperationExportKey:
            {
            iKeyStore->CancelExportKey();
            break;
            }
        case EOperationImportKey:
            {
            iKeyStore->CancelImportKey();
            break;
            }
        case EOperationDeleteKey:
            {
            iKeyStore->CancelDeleteKey();
            break;
            }
        default:
            {
            break;
            }
        }
    }

// -----------------------------------------------------------------------------
// CpCertManUISyncWrapper::InitStoreL
// -----------------------------------------------------------------------------
//
void CpCertManUISyncWrapper::InitStoreL( CUnifiedCertStore*& aStore )
    {
    aStore->Initialize( iStatus );
    iOperation = EOperationInit;
    iStore = aStore;
    SetActive();
    iWait.Start();
    User::LeaveIfError( iStatus.Int() );
    iOperation = EOperationNone;
    }

// -----------------------------------------------------------------------------
// CpCertManUISyncWrapper::InitStoreL
// -----------------------------------------------------------------------------
//
void CpCertManUISyncWrapper::InitStoreL( CUnifiedKeyStore*& aStore  )
    {
    aStore->Initialize( iStatus );
    iOperation = EOperationInitKeyStore;
    iKeyStore = aStore;
    SetActive();
    iWait.Start();
    User::LeaveIfError( iStatus.Int() );
    iOperation = EOperationNone;
    }

// -----------------------------------------------------------------------------
// CpCertManUISyncWrapper::ListL
// -----------------------------------------------------------------------------
//
void CpCertManUISyncWrapper::ListL(
    CUnifiedCertStore*& aStore,
    RMPointerArray<CCTCertInfo>* aArray,
    const CCertAttributeFilter& aFilter )
    {
    
    if ( IsActive() )
        {
        // Wrapper is active. Don't go further
		User::Leave(KErrGeneral);
        }

    aStore->List( *aArray, aFilter, iStatus );
    iOperation = EOperationList;
    iStore = aStore;
    SetActive();
    iWait.Start();
    User::LeaveIfError( iStatus.Int() );
    iOperation = EOperationNone;
    }

// -----------------------------------------------------------------------------
// CpCertManUISyncWrapper::ListL
// -----------------------------------------------------------------------------
//
void CpCertManUISyncWrapper::ListL(
    CUnifiedKeyStore*& aStore,
    RMPointerArray<CCTKeyInfo>* aArray,
    const TCTKeyAttributeFilter& aFilter,
    const TUid aTokenUid )
    {
    
    if ( IsActive() )
        {
        // Wrapper is active. Don't go further
		User::Leave(KErrGeneral);
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
            User::LeaveIfError( iStatus.Int() );
            break;
            }
        }
    iOperation = EOperationNone;
    }

// -----------------------------------------------------------------------------
// CpCertManUISyncWrapper::GetCertificateL
// -----------------------------------------------------------------------------
//
void CpCertManUISyncWrapper::GetCertificateL(
    CUnifiedCertStore*& aStore,
    const CCTCertInfo& aCertInfo,
    CCertificate*& aCert )
    {    
    aCert = NULL;
    HBufC8* buf = HBufC8::NewLC( KMaxCertificateLength );
    iCertPtr.Set( buf->Des() );
    aStore->Retrieve( aCertInfo, iCertPtr, iStatus);
    iOperation = EGetCertificate;
    iStore = aStore;
    SetActive();
    iWait.Start();
    User::LeaveIfError( iStatus.Int() );
    
	switch ( aCertInfo.CertificateFormat() )
		{
		case EX509Certificate:
			{
			aCert = CX509Certificate::NewL( iCertPtr );
			break;
			}
		case EX509CertificateUrl:
			{
			break;
			}
		default:
			{
			break;
			}
		}
    CleanupStack::PopAndDestroy(buf); 
    iOperation = EOperationNone;
    }

// -----------------------------------------------------------------------------
// CpCertManUISyncWrapper::ValidateX509RootCertificateL
// -----------------------------------------------------------------------------
//
void CpCertManUISyncWrapper::ValidateX509RootCertificateL(
    CPKIXValidationResult*& aValidationResult,
    const TTime& aValidationTime, CPKIXCertChain*& aChain )
    {
    
    aChain->ValidateL( *aValidationResult, aValidationTime, iStatus );
    iOperation = EOperationValidateX509Root;
    iChain = aChain;
    SetActive();
    iWait.Start();
    iOperation = EOperationNone;
    User::LeaveIfError(iStatus.Int());
    }

// -----------------------------------------------------------------------------
// CpCertManUISyncWrapper::GetUrlCertificateL
// -----------------------------------------------------------------------------
//
void CpCertManUISyncWrapper::GetUrlCertificateL(
    CUnifiedCertStore*& aStore,
    const CCTCertInfo& aCertInfo,
    TDes8& aUrl )
    {
    
    aStore->Retrieve( aCertInfo, aUrl, iStatus);
    iOperation = ERetriveURLCertificate;
    iStore = aStore;
    SetActive();
    iWait.Start();
    User::LeaveIfError( iStatus.Int() );
    iOperation = EOperationNone;
    }

// -----------------------------------------------------------------------------
// CpCertManUISyncWrapper::GetApplicationsL
// -----------------------------------------------------------------------------
//
void CpCertManUISyncWrapper::GetApplicationsL( CUnifiedCertStore*& aStore,
    const CCTCertInfo& aCertInfo, RArray<TUid>& aApps )
    {
    
    aStore->Applications( aCertInfo, aApps, iStatus );
    iOperation = EOperationGetApps;
    iStore = aStore;
    SetActive();
    iWait.Start();
    User::LeaveIfError( iStatus.Int() );
    iOperation = EOperationNone;
    }

// -----------------------------------------------------------------------------
// CpCertManUISyncWrapper::SetApplicabilityL
// -----------------------------------------------------------------------------
//
void CpCertManUISyncWrapper::SetApplicabilityL( CUnifiedCertStore*& aStore,
    const CCTCertInfo& aCertInfo,
    RArray<TUid>& aApplications )
    {
    
    aStore->SetApplicability( aCertInfo, aApplications, iStatus );
    iOperation = EOperationSetApplicability;
    iStore = aStore;
    SetActive();
    iWait.Start();
    User::LeaveIfError( iStatus.Int() );    
	aStore->SetTrust( aCertInfo, ETrue, iStatus );
	iOperation = EOperationSetToTrusted;
	SetActive();
	iWait.Start();
	User::LeaveIfError( iStatus.Int() );
    iOperation = EOperationNone;
    }

// -----------------------------------------------------------------------------
// CpCertManUISyncWrapper::MoveKeyL
// -----------------------------------------------------------------------------
//
void CpCertManUISyncWrapper::MoveKeyL(
    CUnifiedKeyStore*& aStore,
    const TCTKeyAttributeFilter& aFilter,
    const TUid aSourceTokenId,
    const TUid aTargetTokenId )
    {
    
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

        CleanupStack::PopAndDestroy(keyData);  
        }

    }

// -----------------------------------------------------------------------------
// CpCertManUISyncWrapper::MoveCertL
// -----------------------------------------------------------------------------
//
TInt CpCertManUISyncWrapper::MoveCertL(
    CUnifiedCertStore*& aStore,
    const CCTCertInfo& aCertInfo,
    const TUid aSourceTokenId,
    const TUid aTargetTokenId )
    {
    
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
        User::LeaveIfError( iStatus.Int() );
        iOperation = EOperationNone;
        certCount++;
        CleanupStack::PopAndDestroy(buf);  
        }

    return certCount;
    }


