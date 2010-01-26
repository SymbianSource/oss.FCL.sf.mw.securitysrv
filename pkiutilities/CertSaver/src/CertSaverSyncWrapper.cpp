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
* Description:   Implementation of CCertSaverSyncWrapper class
*
*/


// INCLUDE FILES
#include    "CertSaverSyncWrapper.h"
#include    <unifiedcertstore.h>
#include    <mctwritablecertstore.h>
#include    <unifiedkeystore.h>
#include    <mctkeystore.h>

_LIT( KCertSaverName, "CertSaver" );

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCertSaverSyncWrapper::CCertSaverSyncWrapper
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CCertSaverSyncWrapper::CCertSaverSyncWrapper() : CActive( EPriorityStandard )
    {
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CCertSaverSyncWrapper::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CCertSaverSyncWrapper::ConstructL()
    {
    }

// -----------------------------------------------------------------------------
// CCertSaverSyncWrapper::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CCertSaverSyncWrapper* CCertSaverSyncWrapper::NewL()
    {
    CCertSaverSyncWrapper* self = new( ELeave ) CCertSaverSyncWrapper;

    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop(self);

    return self;
    }

// Destructor
CCertSaverSyncWrapper::~CCertSaverSyncWrapper()
    {
    Cancel();
    delete iUnifiedKeyStore;
    }

// -----------------------------------------------------------------------------
// CCertSaverSyncWrapper::InitializeCertStore
//
// -----------------------------------------------------------------------------
//
TInt CCertSaverSyncWrapper::InitializeCertStore(
    CUnifiedCertStore& aStore )
    {
    aStore.Initialize( iStatus );
    return SetActiveAndWait();
    }

// -----------------------------------------------------------------------------
// CCertSaverSyncWrapper::InitializeKeyStore
//
// -----------------------------------------------------------------------------
//
TInt CCertSaverSyncWrapper::InitializeKeyStoreL( RFs& aFs )
    {
    iUnifiedKeyStore = CUnifiedKeyStore::NewL( aFs );
    iUnifiedKeyStore->Initialize( iStatus );
    return SetActiveAndWait();
    }

// -----------------------------------------------------------------------------
// CCertSaverSyncWrapper::AddCert
//
// -----------------------------------------------------------------------------
//
TInt CCertSaverSyncWrapper::AddCert(
    MCTWritableCertStore& aStore,
    const TDesC& aLabel,
    TCertificateFormat aCertFormat,
    TCertificateOwnerType aCertType,
    TKeyIdentifier* aSubjectKeyId,
    TKeyIdentifier* aIssuerKeyId,
    TPtrC8 aCert )
    {
    aStore.Add( aLabel, aCertFormat, aCertType, aSubjectKeyId, aIssuerKeyId,
        aCert, iStatus );
    return SetActiveAndWait();
    }

// -----------------------------------------------------------------------------
// CCertSaverSyncWrapper::ListCerts
//
// -----------------------------------------------------------------------------
//
TInt CCertSaverSyncWrapper::ListCerts(
    CUnifiedCertStore& aStore,
    RMPointerArray<CCTCertInfo>& aEntries,
    CCertAttributeFilter& aFilter )
    {
    aStore.List( aEntries, aFilter, iStatus );
    return SetActiveAndWait();
    }

// -----------------------------------------------------------------------------
// CCertSaverSyncWrapper::ListKeys
//
// -----------------------------------------------------------------------------
//
TInt CCertSaverSyncWrapper::ListKeys( RMPointerArray<CCTKeyInfo>& aEntries,
                       TCTKeyAttributeFilter& aFilter )
    {
    iUnifiedKeyStore->List( aEntries, aFilter, iStatus );
    return SetActiveAndWait();
    }

// -----------------------------------------------------------------------------
// CCertSaverSyncWrapper::SetApplicability
//
// -----------------------------------------------------------------------------
//
TInt CCertSaverSyncWrapper::SetApplicability(
    MCTWritableCertStore& aStore,
    const CCTCertInfo& aCertInfo,
    RArray<TUid>& aApplications )
    {
    aStore.SetApplicability( aCertInfo, aApplications, iStatus );
    return SetActiveAndWait();
    }

// -----------------------------------------------------------------------------
// CCertSaverSyncWrapper::SetTrust
//
// -----------------------------------------------------------------------------
//
TInt CCertSaverSyncWrapper::SetTrust(
    MCTWritableCertStore& aStore,
    const CCTCertInfo& aCertInfo,
    TBool aTrust )
    {
    aStore.SetTrust( aCertInfo, aTrust, iStatus );
    return SetActiveAndWait();
    }

// -----------------------------------------------------------------------------
// CCertSaverSyncWrapper::SetTrust
//
// -----------------------------------------------------------------------------
//
TInt CCertSaverSyncWrapper::Retrieve(
    CUnifiedCertStore& aStore,
    const CCTCertInfo& aCertInfo,
    CCertificate*& aCert )
    {
    aStore.Retrieve( aCertInfo, aCert, iStatus );
    return SetActiveAndWait();
    }

// -----------------------------------------------------------------------------
// CCertSaverSyncWrapper::AddKey
//
// -----------------------------------------------------------------------------
//
TInt CCertSaverSyncWrapper::AddKey(
    TInt aKeyStoreIndex,
    const TDesC8& aKeyData,
    TKeyUsagePKCS15 aUsage,
    const TDesC& aLabel,
    TInt aAccessType,
    TTime aStartDate,
    TTime aEndDate,
    CCTKeyInfo*& aKeyInfoOut )
    {
    iUnifiedKeyStore->ImportKey( aKeyStoreIndex, aKeyData, aUsage, aLabel,
        aAccessType, aStartDate, aEndDate, aKeyInfoOut, iStatus );
    return SetActiveAndWait();
    }

// -----------------------------------------------------------------------------
// CCertSaverSyncWrapper::SetManagementPolicy
//
// -----------------------------------------------------------------------------
//
TInt CCertSaverSyncWrapper::SetManagementPolicy(
    TCTTokenObjectHandle aHandle,
  const TSecurityPolicy& aPolicy )
    {
    iUnifiedKeyStore->SetManagementPolicy( aHandle, aPolicy, iStatus );
    return SetActiveAndWait();
    }

// -----------------------------------------------------------------------------
// CCertSaverSyncWrapper::SetUsePolicy
//
// -----------------------------------------------------------------------------
//
TInt CCertSaverSyncWrapper::SetUsePolicy(
    TCTTokenObjectHandle aHandle,
    const TSecurityPolicy& aPolicy )
    {
    iUnifiedKeyStore->SetUsePolicy( aHandle, aPolicy, iStatus );
    return SetActiveAndWait();
    }

// -----------------------------------------------------------------------------
// CCertSaverSyncWrapper::UnifiedKeyStore
//
// -----------------------------------------------------------------------------
//
CUnifiedKeyStore& CCertSaverSyncWrapper::UnifiedKeyStore()
    {
    __ASSERT_ALWAYS( iUnifiedKeyStore, User::Panic( KCertSaverName, KErrBadHandle ) );
    return *iUnifiedKeyStore;
    }

// -----------------------------------------------------------------------------
// CCertSaverSyncWrapper::DoCancel
//
// -----------------------------------------------------------------------------
//
void CCertSaverSyncWrapper::DoCancel()
    {

    if ( iUnifiedKeyStore )
        {
        iUnifiedKeyStore->Cancel();
        }

    if ( iWait.IsStarted() )
        {
        iWait.AsyncStop();
        }
    }

// -----------------------------------------------------------------------------
// CCertSaverSyncWrapper::RunL()
//
// -----------------------------------------------------------------------------
//
void CCertSaverSyncWrapper::RunL()
    {

    iWait.AsyncStop();
    }

// -----------------------------------------------------------------------------
// CCertSaverSyncWrapper::SetActiveAndWait
//
// -----------------------------------------------------------------------------
//
TInt CCertSaverSyncWrapper::SetActiveAndWait()
    {
    SetActive();
    iWait.Start();
    return iStatus.Int();
    }

//  End of File
