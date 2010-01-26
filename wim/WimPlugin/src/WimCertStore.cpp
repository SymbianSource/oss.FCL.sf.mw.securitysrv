/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Implementation of single certificate store interface
*
*/


// INCLUDE FILES

#include "WimCertStore.h"
#include "WimCertConverter.h"
#include "WimCertInfo.h"
#include "WimTrustSettingsAPI.h"
#include "WimTrace.h"
#include "WimToken.h"
#include "WimTokenListener.h"
#include <unifiedkeystore.h>
#include <ct.h>
#include <x509cert.h>
#include <x509certext.h>
#include <wtlscert.h>
#include <certificateapps.h>

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWimCertStore::CWimCertStore()
// Default constructor
// -----------------------------------------------------------------------------
//
CWimCertStore::CWimCertStore( CWimToken& aToken )
                            : CActive( EPriorityNormal ),
                              iToken( aToken )
    {
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CWimCertStore::NewL()
// Two-phased constructor
// -----------------------------------------------------------------------------
//
CWimCertStore* CWimCertStore::NewL( CWimToken& aToken )
    {
    _WIMTRACE ( _L( "CWimCertStore::NewL()" ) );
    CWimCertStore* self = new( ELeave ) CWimCertStore( aToken );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CWimCertStore::ConstructL()
// Instantiates converter, completes message
// -----------------------------------------------------------------------------
//
void CWimCertStore::ConstructL()
    {
    _WIMTRACE ( _L( "CWimCertStore::ConstructL()" ) );
    iCWimCertConverter = CWimCertConverter::NewL( Token() );
    // Open trust settings database
    iCWimTrustSettingsStore = CWimTrustSettingsAPI::NewL();
    iPhase = EIdle;
    iPhaseOriginal = EIdle;
    }

// -----------------------------------------------------------------------------
// CWimCertStore::~CWimCertStore()
// Destructor
// -----------------------------------------------------------------------------
//
CWimCertStore::~CWimCertStore()
    {
    _WIMTRACE ( _L( "CWimCertStore::~CWimCertStore()" ) );
    Cancel();
    iKeyInfos.Close();
    if ( iCerts )
        {
        delete iCerts;
        }
    iCertInfos.ResetAndDestroy();
    iCertsList = NULL;
    delete iUnifiedKeyStore;
    if ( iCWimTrustSettingsStore )
        {
        iCWimTrustSettingsStore->Close();
        }
    iOriginalRequestStatus = NULL;
    iFilter = NULL;
    iEncodedCert = NULL;
    if ( iCWimCertConverter )
        {
        delete iCWimCertConverter;
        }

    if ( iOldTrusters )
        {
        iOldTrusters->Close();
        delete iOldTrusters;
        }
    }

// -----------------------------------------------------------------------------
// CWimCertStore::Token()
// Returns a reference to current token (MCTToken) of this certificate store
// interface. Reference is created during construction.
// -----------------------------------------------------------------------------
//
MCTToken& CWimCertStore::Token()
    {
    _WIMTRACE ( _L( "CWimCertStore::Token()" ) );
    return iToken;
    }

// -----------------------------------------------------------------------------
// CWimCertStore::DoRelease()
// Deletes this interface on demand.
// -----------------------------------------------------------------------------
//
void CWimCertStore::DoRelease()
    {
    _WIMTRACE ( _L( "CWimCertStore::DoRelease()" ) );
    delete this;
    }

// -----------------------------------------------------------------------------
// CWimCertStore::List()
// Lists certificates according to filter parameter.
// -----------------------------------------------------------------------------
//
void CWimCertStore::List( RMPointerArray<CCTCertInfo>& aCertInfos,
                          const CCertAttributeFilter& aFilter,
                          TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimCertStore::List()" ) );

    if ( !EnteringAllowed( aStatus ) )
        {
        return;
        }

    iCertsList = &aCertInfos;
    iFilter = &aFilter;

    if ( iFilter->iKeyUsage != EX509UsageAll )
        {
        // We must ensure that in this case only user certs are allowed
        if ( iFilter->iOwnerTypeIsSet &&
             iFilter->iOwnerType == EUserCertificate )
            {
            // We have to initialize the unified key store because
            // in this phase we don't know if user certificates are found or not.
            // User certificate private key must be checked for usage reason
            if ( iUnifiedKeyStore )
                {
                User::RequestComplete( iOriginalRequestStatus, KErrCorrupt );
                }
            else
                {
                iFs = static_cast<CCTTokenType&>( Token().TokenType() ).Fs();
                TRAPD( err, iUnifiedKeyStore = CUnifiedKeyStore::NewL( iFs ) );

                if ( err != KErrNone )
                    {
                    User::RequestComplete( iOriginalRequestStatus, err );
                    }
                else
                    {
                    iPhase = EGetKeyInfos;
                    iUnifiedKeyStore->Initialize( iStatus );
                    SetActive();
                    }
                }
            }
        else
            {
            User::RequestComplete( iOriginalRequestStatus, KErrArgument );
            }
        }
    else
        {
        iPhase = EList;
        iPhaseOriginal = EList;
        TRequestStatus* status = &iStatus;
        User::RequestComplete( status, KErrNone );
        SetActive();
        }
    }

// -----------------------------------------------------------------------------
// CWimCertStore::CancelList()
// Cancels issued List operation. Main functionality in DoCancel().
// -----------------------------------------------------------------------------
//
void CWimCertStore::CancelList()
    {
    _WIMTRACE ( _L( "CWimCertStore::CancelList()" ) );
    if ( TokenRemoved() )
        {
        return;
        }
    Cancel();
    }

// -----------------------------------------------------------------------------
// CWimCertStore::GetCert()
// Fetches one certificate info according to given handle.
// -----------------------------------------------------------------------------
//
void CWimCertStore::GetCert( CCTCertInfo*& aCertInfo,
                             const TCTTokenObjectHandle& aHandle,
                             TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimCertStore::GetCert()" ) );

    if ( !EnteringAllowed( aStatus ) )
        {
        return;
        }

    if ( aHandle.iTokenHandle != Token().Handle() )
        {
        User::RequestComplete( iOriginalRequestStatus, KErrBadHandle );
        }
    else
        {
        iCertInfo = &aCertInfo;
        iHandle = &aHandle;
        iPhase = EGetCert;
        TRequestStatus* status = &iStatus;
        User::RequestComplete( status, KErrNone );
        SetActive();
        }
    }

// -----------------------------------------------------------------------------
// CWimCertStore::DoGetCert()
// Fetches one certificate
// -----------------------------------------------------------------------------
//
void CWimCertStore::DoGetCert()
    {
    _WIMTRACE ( _L( "CWimCertStore::DoGetCert()" ) );

    TInt err = KErrNotFound;
    TRAP( err, *iCertInfo =
            CCTCertInfo::NewL( iCerts->EntryByHandleL ( iHandle->iObjectId ) ) );
    User::RequestComplete( iOriginalRequestStatus, err );
    }

// -----------------------------------------------------------------------------
// CWimCertStore::CancelGetCert()
// Cancels issued GetCert operation. Main functionality in DoCancel().
// -----------------------------------------------------------------------------
//
void CWimCertStore::CancelGetCert()
    {
    _WIMTRACE ( _L( "CWimCertStore::CancelGetCert()" ) );
    Cancel();
    }

// -----------------------------------------------------------------------------
// CWimCertStore::Applications()
// Lists the applications of a certificate. Applications are represented by UIDs
// -----------------------------------------------------------------------------
//
void CWimCertStore::Applications( const CCTCertInfo& aCertInfo,
                                  RArray<TUid>& aApplications,
                                  TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimCertStore::Applications()" ) );

    if ( !EnteringAllowed( aStatus ) )
        {
        return;
        }

    iCertInfoReadOnly = &aCertInfo;
    iApplications = &aApplications;
    iPhase = EApplications;
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, KErrNone );
    SetActive();
    }

// -----------------------------------------------------------------------------
// CWimCertStore::DoApplications()
// Fetch all certificate's applications
// -----------------------------------------------------------------------------
//
void CWimCertStore::DoApplications()
    {
    _WIMTRACE ( _L( "CWimCertStore::DoApplications()" ) );

    TInt err = KErrNone;
    TInt index = iCerts->Index( *iCertInfoReadOnly );

    if ( index != KErrNotFound )
        {
        const RArray<TUid>& apps = iCerts->Mapping( index )->CertificateApps();
        TInt end = apps.Count();
        for ( TInt i = 0; ( i < end ) && ( err == KErrNone ); i++ )
            {
            err = iApplications->Append( apps[i] );
            }
        }
    else
        {
        err = index;
        }

    if ( err != KErrNone )
        {
        iApplications->Reset();
        }
    User::RequestComplete( iOriginalRequestStatus, err );
    }
// -----------------------------------------------------------------------------
// CWimCertStore::CancelApplications()
// Cancels issued Applications operation. Main functionality in DoCancel().
// -----------------------------------------------------------------------------
//
void CWimCertStore::CancelApplications()
    {
    _WIMTRACE ( _L( "CWimCertStore::CancelApplications()" ) );
    Cancel();
    }

// -----------------------------------------------------------------------------
// CWimCertStore::IsApplicable()
// Checks if a particular certificate is applicable to a particular application.
// -----------------------------------------------------------------------------
//
void CWimCertStore::IsApplicable( const CCTCertInfo& aCertInfo,
                                  TUid aApplication,
                                  TBool& aIsApplicable,
                                  TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimCertStore::IsApplicable()" ) );

    if ( !EnteringAllowed( aStatus ) )
        {
        return;
        }

    iCertInfoReadOnly = &aCertInfo;
    iApplication = aApplication;
    iIsApplicable = &aIsApplicable;
    iPhase = EIsApplicable;
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, KErrNone );
    SetActive();
    }

// -----------------------------------------------------------------------------
// CWimCertStore::DoIsApplicable()
// Match given application to certificate's application
// -----------------------------------------------------------------------------
//
void CWimCertStore::DoIsApplicable()
    {
    _WIMTRACE ( _L( "CWimCertStore::DoIsApplicable()" ) );

    TInt index = iCerts->Index( *iCertInfoReadOnly );
    if ( index != KErrNotFound )
        {
        const RArray<TUid>& apps = iCerts->Mapping( index )->CertificateApps();
        TInt end = apps.Count();
        TInt i = 0;
        for ( ; i < end; i++ )
            {
            if ( apps[i] == iApplication )
                {
                i = end + 1; // This completes loop but differentiates from
                             // normal end condition
                }
            }
        if ( i == end )
            {
            *iIsApplicable = EFalse;
            }
        else
            {
            *iIsApplicable = ETrue;
            }
        index = KErrNone;
        }
    User::RequestComplete( iOriginalRequestStatus, index );
    }

// -----------------------------------------------------------------------------
// CWimCertStore::CancelIsApplicable()
// Cancels issued IsApplicable operation. Main functionality in DoCancel().
// -----------------------------------------------------------------------------
//
void CWimCertStore::CancelIsApplicable()
    {
    _WIMTRACE ( _L( "CWimCertStore::CancelIsApplicable()" ) );
    Cancel();
    }

// -----------------------------------------------------------------------------
// CWimCertStore::Trusted()
// Returns a parameter with true or false, if a certificate is trusted.
// Trust is only meaningful for CA certificates where it means that the
// certificate can be used as a trust root for the purposes
// of certificate validation.
// -----------------------------------------------------------------------------
//
void CWimCertStore::Trusted( const CCTCertInfo& aCertInfo,
                             TBool& aTrusted,
                             TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimCertStore::Trusted()" ) );

    if ( !EnteringAllowed( aStatus ) )
        {
        return;
        }

    iCertInfoReadOnly = &aCertInfo;
    iTrustedCert = &aTrusted;
    iPhase = ETrusted;
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, KErrNone );
    SetActive();
    }

// -----------------------------------------------------------------------------
// CWimCertStore::DoTrusted()
// 
// -----------------------------------------------------------------------------
//
void CWimCertStore::DoTrusted()
    {
    _WIMTRACE ( _L( "CWimCertStore::DoTrusted()" ) );

    TInt index = iCerts->Index( *iCertInfoReadOnly );
    if ( index != KErrNotFound )
        {
        *iTrustedCert = iCerts->Mapping( index )->Trusted();
        index = KErrNone;
        }

    User::RequestComplete( iOriginalRequestStatus, index );
    }

// -----------------------------------------------------------------------------
// CWimCertStore::CancelTrusted()
// Cancels issued Trusted operation. Main functionality in DoCancel().
// -----------------------------------------------------------------------------
//
void CWimCertStore::CancelTrusted()
    {
    _WIMTRACE ( _L( "CWimCertStore::CancelTrusted()" ) );
    Cancel();
    }

// -----------------------------------------------------------------------------
// CWimCertStore::Retrieve()
// Retrieves all the data of the certificate.
// -----------------------------------------------------------------------------
//
void CWimCertStore::Retrieve( const CCTCertInfo& aCertInfo,
                              TDes8& aEncodedCert,
                              TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimCertStore::Retrieve()" ) );

    if ( !EnteringAllowed( aStatus ) )
        {
        return;
        }

    iCertInfoReadOnly = &aCertInfo;
    iEncodedCert = &aEncodedCert;
    iPhase = ERetrieve;
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, KErrNone );
    SetActive();
    }

// -----------------------------------------------------------------------------
// CWimCertStore::DoRetrieve()
// Retrieves all the data of the certificate.
// -----------------------------------------------------------------------------
//
void CWimCertStore::DoRetrieve()
    {
    _WIMTRACE ( _L( "CWimCertStore::DoRetrieve()" ) );

    // Let's get the index of certificate info
    // Index used as handle in WimClient in order to locate this certificate
    iCertIndex = iCerts->Index( *iCertInfoReadOnly );

    if ( iCertIndex == KErrNotFound )
        {
        User::RequestComplete( iOriginalRequestStatus, KErrNotFound );
        }
    else
        {
        iPhase = ERetrieveFromWim;
        TRequestStatus* status = &iStatus;
        User::RequestComplete( status, KErrNone );
        SetActive();
        }
    }

// -----------------------------------------------------------------------------
// CWimCertStore::CancelRetrieve()
// Cancels issued Retrieve operation and informs converter to stop.
// -----------------------------------------------------------------------------
//
void CWimCertStore::CancelRetrieve()
    {
    _WIMTRACE ( _L( "CWimCertStore::CancelRetrieve()" ) );
    Cancel();
    }

// -----------------------------------------------------------------------------
// void CWimCertStore::Add()
// Adds a certificate to the WIM store.
// -----------------------------------------------------------------------------
//
void CWimCertStore::Add( const TDesC& aLabel,
                         TCertificateFormat aFormat,
                         TCertificateOwnerType aCertificateOwnerType,
                         const TKeyIdentifier* aSubjectKeyId,
                         const TKeyIdentifier* aIssuerKeyId,
                         const TDesC8& aCert,
                         TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimCertStore::Add()" ) );

    if ( !EnteringAllowed( aStatus ) )
        {
        return;
        }

    // Label must be
    if ( aLabel.Length() == 0 )
        {
        User::RequestComplete( iOriginalRequestStatus, KErrArgument );
        return;
        }

    switch ( aCertificateOwnerType )
        {
        case ECACertificate:
            {
            break;
            }
        case EUserCertificate:
            {
            break;
            }
        case EPeerCertificate:
            {
            break;
            }
        default:
            {
            User::RequestComplete( iOriginalRequestStatus, KErrArgument );
            return;
            }
        }

    iKeyFilter.iKeyId = KNullDesC8;
    iKeyFilter.iUsage = ( TKeyUsagePKCS15 )0;
    iLabel = &aLabel;
    iFormat = aFormat;
    iCertificateOwnerType = aCertificateOwnerType;

    if ( aSubjectKeyId && ( *aSubjectKeyId != KNullDesC8 ) )
        {
        iSubjectKeyId = aSubjectKeyId;
        }
    else
        {
        iSubjectKeyId = 0;
        }

    iIssuerKeyId = aIssuerKeyId;
    iCert = &aCert;

    TRAPD( err, ComputeAndCheckSubjectKeyIdL() );

    if ( err != KErrNone )
        {
        User::RequestComplete( iOriginalRequestStatus, err );
        return;
        }

    if ( aCertificateOwnerType == EUserCertificate )
        {
        if ( iUnifiedKeyStore ) // Should never happen
            {
            User::RequestComplete( iOriginalRequestStatus, KErrCorrupt );
            }
        else
            {
            iFs = static_cast<CCTTokenType&>( Token().TokenType() ).Fs();

            TRAPD( err2, iUnifiedKeyStore = CUnifiedKeyStore::NewL( iFs ) );
            if ( err2 != KErrNone )
                {
                User::RequestComplete( iOriginalRequestStatus, err2 );
                }
            else
                {
                iPhase = EGetCorrespondingPrivateKey;
                iUnifiedKeyStore->Initialize( iStatus );
                SetActive();
                }
            }
        }
    else
        {
        iPhase = EAdd;
        TRequestStatus* status = &iStatus;
        User::RequestComplete( status, KErrNone );
        SetActive();
        }
    }

// -----------------------------------------------------------------------------
// void CWimCertStore::DoAdd()
// Adds a certificate to the WIM store.
// -----------------------------------------------------------------------------
//
void CWimCertStore::DoAdd()
    {
    _WIMTRACE ( _L( "CWimCertStore::DoAdd()" ) );

    // Check that certificate label don't already exist
    TInt iend = iCerts->Count();
    for ( TInt i = 0; i < iend; i++ )
        {
        if ( iCerts->Entry( i ).Label() == *iLabel )
            {
            User::RequestComplete( iOriginalRequestStatus, KErrBadName );
            return;
            }
        }
    iPhase = EAddToWim;
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, KErrNone );
    SetActive();
    }

// -----------------------------------------------------------------------------
// CWimCertStore::ComputeAndCheckSubjectKeyIdL()
// Computes subject key id
// -----------------------------------------------------------------------------
//
void CWimCertStore::ComputeAndCheckSubjectKeyIdL()
    {
    _WIMTRACE ( _L( "CWimCertStore::ComputeAndCheckSubjectKeyIdL()" ) );
    switch ( iFormat )
        {
        case EX509Certificate:
            {
            CX509Certificate* cert = CX509Certificate::NewLC( *iCert );
            const CX509CertExtension* ext = cert->Extension( KKeyUsage );
            if ( ext )
                {
                CX509KeyUsageExt* keyUsageExt =
                                  CX509KeyUsageExt::NewLC( ext->Data() );
                if ( keyUsageExt->IsSet( EX509DigitalSignature ) )
                    {
                    iKeyFilter.iUsage =
                        ( TKeyUsagePKCS15 )
                        ( iKeyFilter.iUsage | EX509UsageDigitalSignature );
                    }
                if ( keyUsageExt->IsSet( EX509NonRepudiation ) )
                    {
                    iKeyFilter.iUsage =
                        ( TKeyUsagePKCS15 )
                        ( iKeyFilter.iUsage | EX509UsageNonRepudiation );
                    }
                if ( keyUsageExt->IsSet( EX509KeyEncipherment ) )
                    {
                    iKeyFilter.iUsage =
                        ( TKeyUsagePKCS15 )
                        ( iKeyFilter.iUsage | EX509UsageKeyEncipherment );
                    }
                if ( keyUsageExt->IsSet( EX509DataEncipherment ) )
                    {
                    iKeyFilter.iUsage =
                        ( TKeyUsagePKCS15 )
                        ( iKeyFilter.iUsage | EX509UsageDataEncipherment );
                    }
                if ( keyUsageExt->IsSet( EX509KeyAgreement ) )
                    {
                    iKeyFilter.iUsage =
                        ( TKeyUsagePKCS15 )
                        ( iKeyFilter.iUsage | EX509UsageKeyAgreement );
                    }
                if ( keyUsageExt->IsSet( EX509KeyCertSign ) )
                    {
                    iKeyFilter.iUsage =
                        ( TKeyUsagePKCS15 )
                        ( iKeyFilter.iUsage | EX509UsageKeyCertSign );
                    }
                if ( keyUsageExt->IsSet( EX509CRLSign ) )
                    {
                    iKeyFilter.iUsage =
                        ( TKeyUsagePKCS15 )
                        ( iKeyFilter.iUsage | EX509UsageCRLSign );
                    }
                if ( keyUsageExt->IsSet( EX509EncipherOnly ) )
                    {
                    iKeyFilter.iUsage =
                        ( TKeyUsagePKCS15 )
                        ( iKeyFilter.iUsage | EX509UsageEncipherOnly );
                    }
                if ( keyUsageExt->IsSet( EX509DecipherOnly ) )
                    {
                    iKeyFilter.iUsage =
                        ( TKeyUsagePKCS15 )
                        ( iKeyFilter.iUsage | EX509UsageDecipherOnly );
                    }

                CleanupStack::PopAndDestroy( keyUsageExt );
                }
            iComputedSubjectKeyId = cert->KeyIdentifierL();
            if ( !iSubjectKeyId )
                {
                iSubjectKeyId = &iComputedSubjectKeyId;
                }
            else if ( iComputedSubjectKeyId.Compare( *iSubjectKeyId ) )
                {
                User::Leave( KErrArgument );
                }
            CleanupStack::PopAndDestroy( cert );
            break;
            }
        case EWTLSCertificate:
            {
            CCertificate* cert = CWTLSCertificate::NewLC( *iCert );
            iComputedSubjectKeyId = cert->KeyIdentifierL();
            if ( !iSubjectKeyId )
                {
                iSubjectKeyId = &iComputedSubjectKeyId;
                }
            else if ( iComputedSubjectKeyId.Compare( *iSubjectKeyId ) )
                {
                User::Leave( KErrArgument );
                }
            CleanupStack::PopAndDestroy( cert );
            break;
            }
        case EX509CertificateUrl:
            {
            iKeyFilter.iUsage = EPKCS15UsageAll;
            if ( !iSubjectKeyId )
                {
                User::Leave( KErrArgument );
                }
            break;
            }
        default:
            {
            User::Leave( KErrNotSupported );
            break;
            }
        }
    }

// -----------------------------------------------------------------------------
// CWimCertStore::CancelAdd()
// Cancels issued Add operation and informs converter to stop.
// -----------------------------------------------------------------------------
//
void CWimCertStore::CancelAdd()
    {
    _WIMTRACE ( _L( "CWimCertStore::CancelAdd()" ) );
    Cancel();
    }

// -----------------------------------------------------------------------------
// CWimCertStore::Remove()
// Removes a certificate from WIM store
// -----------------------------------------------------------------------------
//
void CWimCertStore::Remove( const CCTCertInfo& aCertInfo,
                                     TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimCertStore::Remove()" ) );

    if ( !EnteringAllowed( aStatus ) )
        {
        return;
        }

    iCertInfoReadOnly = &aCertInfo;
    iPhase = ERemove;
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, KErrNone );
    SetActive();
    }

// -----------------------------------------------------------------------------
// CWimCertStore::DoRemove()
// Removes a certificate from WIM store
// -----------------------------------------------------------------------------
//
void CWimCertStore::DoRemove()
    {
    _WIMTRACE ( _L( "CWimCertStore::DoRemove()" ) );

    TInt index = iCerts->Index( *iCertInfoReadOnly );
    // Check that given certificate info exists
    if ( index == KErrNotFound )
        {
        User::RequestComplete( iOriginalRequestStatus, KErrNotFound );
        return;
        }

    CWimCertStoreMapping* mapping = iCerts->Mapping( index );

    if ( !mapping )
        {
        User::RequestComplete( iOriginalRequestStatus, KErrNotFound );
        return;
        }

    // Is certificate deletable?

    const CCTCertInfo& certInfo = iCerts->Entry( index );

    if ( !certInfo.IsDeletable() )
        {
        User::RequestComplete( iOriginalRequestStatus, KErrAccessDenied );
        return;
        }

    // This index is used in the next phase when deleting certificate from Wim
    iCertIndex = index;
    iPhase = EDeleteFromWim;
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, KErrNone );
    SetActive();
    }

// -----------------------------------------------------------------------------
// CWimCertStore::CancelRemove()
// Cancels ongoing certificate removal.
// -----------------------------------------------------------------------------
//
void CWimCertStore::CancelRemove()
    {
    _WIMTRACE ( _L( "CWimCertStore::CancelRemove()" ) );
    Cancel();
    }

// -----------------------------------------------------------------------------
// void CWimCertStore::SetApplicability()
// Replaces the current applicability settings with the settings
// in the supplied array.
// -----------------------------------------------------------------------------
//
void CWimCertStore::SetApplicability( const CCTCertInfo& aCertInfo,
#ifdef __SECURITY_PLATSEC_ARCH__
                                      const RArray<TUid>& aTrusters,
#else
                                      RArray<TUid>* aTrusters,
#endif
                                      TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimCertStore::SetApplicability()" ) );

    if ( !EnteringAllowed( aStatus ) )
        {
        return;
        }

    TRAPD( err, CheckApplicabilityL( aTrusters ) );

    if ( err == KErrNone )
        {
        iCertInfoReadOnly = &aCertInfo;

#ifdef __SECURITY_PLATSEC_ARCH__
        iApplications = new RArray<TUid>;
        if ( iApplications )
            {
            for ( TInt i = 0; i < aTrusters.Count(); i++ )
                {
                iApplications->Append( aTrusters[i] );
                }
            }
        else
            {
            User::RequestComplete( iOriginalRequestStatus, KErrNoMemory );
            }
#else
        iApplications = aTrusters;
#endif

        iPhase = ESetApplicability;
        TRequestStatus* status = &iStatus;
        User::RequestComplete( status, KErrNone );
        SetActive();
        }
    else
        {
        User::RequestComplete( iOriginalRequestStatus, err );
        }
    }

// -----------------------------------------------------------------------------
// void CWimCertStore::DoSetApplicability()
// Replaces the current applicability settings with the settings
// in the supplied array.
// -----------------------------------------------------------------------------
//
void CWimCertStore::DoSetApplicability()
    {
    _WIMTRACE ( _L( "CWimCertStore::DoSetApplicability()" ) );

    TRAPD( err, DoSetApplicabilityL() );
    if ( err != KErrNone )
        {
#ifdef __SECURITY_PLATSEC_ARCH__
        iApplications->Close();
        delete iApplications;
        iApplications = NULL;
#endif
        User::RequestComplete( iOriginalRequestStatus, err );
        }
    }

// -----------------------------------------------------------------------------
// void CWimCertStore::CheckApplicabilityL()
// Check that given applications exist
// -----------------------------------------------------------------------------
//
void CWimCertStore::CheckApplicabilityL(
#ifdef __SECURITY_PLATSEC_ARCH__
    const RArray<TUid>& aTrusters
#else
    RArray<TUid>* aTrusters
#endif
    )
    {
    _WIMTRACE ( _L( "CWimCertStore::CheckApplicabilityL()" ) );

#ifndef __SECURITY_PLATSEC_ARCH__
    TCleanupItem cleanupTrusters( CWimCertStore::CleanTrustersArray, aTrusters );
    CleanupStack::PushL( cleanupTrusters );
#endif
    // Let's fetch application infos from file this device supports
    iFs = static_cast<CCTTokenType&>( Token().TokenType() ).Fs();
    CCertificateAppInfoManager* appInfoManager =
                                CCertificateAppInfoManager::NewLC( iFs, EFalse );
    // Take references to those application infos
    const RArray<TCertificateAppInfo>& applications =
                                       appInfoManager->Applications();
    // Chcek that given new applications exists in supported applications
#ifdef __SECURITY_PLATSEC_ARCH__
    TInt count1 = aTrusters.Count();
#else
    TInt count1 = aTrusters->Count();
#endif

    TInt count2 = applications.Count();
    TInt i = 0;
    for ( ; i < count1; i++ )
        {
        TInt j = 0;
        for ( ; j < count2; j++ )
            {
#ifdef __SECURITY_PLATSEC_ARCH__
            if ( aTrusters[i] == applications[j].Id() ) // Match found
#else
            if ( ( *aTrusters)[i] == applications[j].Id() ) // Match found
#endif

                {
                j = count2 + 1; // This stops loop but differentiates from
                                // normal end ( j == count2 )
                }
            }
        if ( j == count2 )      // Some of the given application does not exist
            {
            User::Leave( KErrArgument );
            }
        }

    // Application info not needed anymore
    CleanupStack::PopAndDestroy( appInfoManager );
#ifndef __SECURITY_PLATSEC_ARCH__
    CleanupStack::Pop();
#endif
    }

// -----------------------------------------------------------------------------
// void CWimCertStore::CleanTrustersArray()
// Deletes array of trusters.
// -----------------------------------------------------------------------------
//
void CWimCertStore::CleanTrustersArray( TAny* aTrusters )
    {
    RArray<TUid>* array = reinterpret_cast< RArray<TUid>* >( aTrusters );
    array->Close();
    delete array;
    }

// -----------------------------------------------------------------------------
// void CWimCertStore::DoSetApplicabilityL()
// Replaces the current applicability settings with the settings
// in the supplied array.
// -----------------------------------------------------------------------------
//
void CWimCertStore::DoSetApplicabilityL()
    {
    _WIMTRACE ( _L( "CWimCertStore::DoSetApplicabilityL()" ) );

    TCleanupItem cleanupTrusters( CWimCertStore::CleanTrustersArray, iApplications );
    CleanupStack::PushL( cleanupTrusters );

    // Check that given certificate info exists
    TInt index = iCerts->Index( *iCertInfoReadOnly );
    if ( index == KErrNotFound )
        {
        User::Leave( index );
        }

    // The idea behind next operation is to keep old applications in safe
    // until they are succesfully replaced with given new applications

    // Take a pointer to mapping
    CWimCertStoreMapping* mapping = iCerts->Mapping( index );
    // Check that mapping is found
    if ( !mapping )
        {
        User::Leave( KErrNotFound );
        }
    // Take a reference to old applications
    const RArray<TUid>& trusters = mapping->CertificateApps();
        
    // Copy old applications to recently created new pointer array
    if ( iOldTrusters )
        {
        // There can be old trusters if next leaving function has leaved
        // last time
        iOldTrusters->Close();
        delete iOldTrusters;
        iOldTrusters = NULL;
        }

    iOldTrusters = new( ELeave ) RArray<TUid>;
    TInt end = trusters.Count();
    for ( TInt i = 0; i < end; i++ )
        {
        User::LeaveIfError( iOldTrusters->Append( trusters[i] ) );
        }
    // Set new applications to mapping. This replaces old ones
    mapping->SetCertificateAppsL( iApplications );

    // In this phase old applications are in oldTrusted
    // and new applications are in mapping
    // Now we must update TrustSettingStore to make applications permanent
    iStatus = KRequestPending;
    iPhase = EWaitSetApplicability;
    iCWimTrustSettingsStore->SetApplicability( *iCertInfos[index],
                                               *iApplications,
                                                iStatus );
    SetActive();
 
    CleanupStack::Pop();
    }

// -----------------------------------------------------------------------------
// void CWimCertStore::CancelSetApplicability()
// Cancels an ongoing operation. The operation will be
// completed with KErrCancel if it was cancelled
// -----------------------------------------------------------------------------
//
void CWimCertStore::CancelSetApplicability()
    {
    _WIMTRACE ( _L( "CWimCertStore::CancelSetApplicability()" ) );
    Cancel();
    }

// -----------------------------------------------------------------------------
// void CWimCertStore::SetTrust()
// Changes the trust settings. A CA certificate is trusted if the
// user is willing to use it for authenticating servers. It has no
// meaning with other types of certificates.
// -----------------------------------------------------------------------------
//
void CWimCertStore::SetTrust( const CCTCertInfo& aCertInfo,
                              TBool aTrusted,
                              TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimCertStore::SetTrust()" ) );

    if ( !EnteringAllowed( aStatus ) )
        {
        return;
        }

    switch ( aTrusted )
        {
        case EFalse:
            {
            break;
            }
        case ETrue:
            {
            break;
            }
        default:
            {
            User::RequestComplete( iOriginalRequestStatus, KErrArgument );
            return;
            }
        }

    iCertInfoReadOnly = &aCertInfo;
    iTrustedValue = aTrusted;
    iPhase = ESetTrust;
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, KErrNone );
    SetActive();
    }

// -----------------------------------------------------------------------------
// void CWimCertStore::DoSetTrust()
// Changes the trust settings.
// -----------------------------------------------------------------------------
//
void CWimCertStore::DoSetTrust()
    {
    _WIMTRACE ( _L( "CWimCertStore::DoSetTrust()" ) );
    TRAPD( err, DoSetTrustL() );
    
    if ( err != KErrNone )
        {
        User::RequestComplete( iOriginalRequestStatus, err );
        }
    }

// -----------------------------------------------------------------------------
// void CWimCertStore::DoSetTrustL()
// Changes the trust settings.
// -----------------------------------------------------------------------------
//
void CWimCertStore::DoSetTrustL()
    {
    _WIMTRACE ( _L( "CWimCertStore::DoSetTrustL()" ) );
    // Check that given certificate info exists
    TInt index = iCerts->Index( *iCertInfoReadOnly );
    if ( index == KErrNotFound )
        {
        User::Leave( index );
        }

    CWimCertStoreMapping* mapping = iCerts->Mapping( index );
    // Check that mapping is found
    if ( !mapping )
        {
        User::Leave( KErrNotFound );
        }
    // Save old trust value for back up reason
    iOldTrusted = mapping->Trusted();
    // Set new trust value
    const_cast<CWimCertStoreMapping*>( mapping )->SetTrusted( iTrusted );
    // In this phase old trust value is in iOldTrusted
    // and new value is in mapping
    // Now we must update TrustSettingStore to make trust value permanent
    iPhase = EWaitSetTrust;
    iStatus = KRequestPending;
    iCWimTrustSettingsStore->SetTrust( *iCertInfos[index],
                                        iTrusted,
                                        iStatus );
    SetActive();
    }

// -----------------------------------------------------------------------------
// void CWimCertStore::CancelSetTrust()
// Cancels an ongoing SetTrust operation.
// -----------------------------------------------------------------------------
//
void CWimCertStore::CancelSetTrust()
    {
    _WIMTRACE ( _L( "CWimCertStore::CancelSetTrust()" ) );
    Cancel();
    }

// -----------------------------------------------------------------------------
// void CWimCertStore::RunL()
// The first thing is to ensure that certificates are read from WIM
// -----------------------------------------------------------------------------
//
void CWimCertStore::RunL()
    {
    _WIMTRACE3( _L( "CWimCertStore::RunL()| iStatus=%d, iPhase=%d" ), iStatus.Int(), iPhase );
    if ( !iCerts &&
         iPhase != EList &&
         iPhase != EInitializeGetCertList &&
         iPhase != EInitializeLoadMappings &&
         iPhase != EGetKeyInfos )
        {
        iPhaseOriginal = iPhase;
        iPhase = EList;
        TRequestStatus* status = &iStatus;
        User::RequestComplete( status, KErrNone );
        SetActive();
        }
    else
        {
        switch ( iPhase )
        {
            case EInitializeGetCertList:
                {
                DoInitializeGetCertListL();
                break;
                }
            case EInitializeLoadMappings:
                {
                DoInitializeLoadMappingsL();
                break;
                }
            case EInitializeLoadTrustSettingsStart:
                {
                DoInitializeLoadTrustSettingsStartL();
                break;
                }
            case EInitializeLoadTrustSettingsWait:
                {
                DoInitializeLoadTrustSettingsWaitL();
                break;
                }
            case EGetKeyInfos:
                {
                DoGetKeyInfos();
                break;
                }
            case EList:
                {
                DoList();
                break;
                }
            case EListGo:
                {
                DoListGoL();
                break;
                }
            case EGetCert:
                {
                DoGetCert();
                break;
                }
            case EApplications:
                {
                DoApplications();
                break;
                }
            case EIsApplicable:
                {
                DoIsApplicable();
                break;
                }
            case ETrusted:
                {
                DoTrusted();
                break;
                }
            case ESetApplicability:
                {
                DoSetApplicability();
                break;
                }
            case EWaitSetApplicability:
                {
                // If there is an error undo the change
                if ( iStatus.Int() != KErrNone ) 
                    {
                    TInt index = iCerts->Index( *iCertInfoReadOnly );
                    // Take a pointer to mapping
                    CWimCertStoreMapping* mapping = iCerts->Mapping( index );
                    // Set backed up trusters to mapping
                    mapping->SetCertificateAppsL( iOldTrusters );
                    iOldTrusters = NULL; // Ownership moved to mapping
                    }
                else // Delete old trusters array, it is not needed anymore
                    {
                    iOldTrusters->Close();
                    delete iOldTrusters;
                    iOldTrusters = NULL;
                    }
                User::RequestComplete( iOriginalRequestStatus, iStatus.Int() );
                break;
                }
            case ESetTrust:
                {
                DoSetTrust();
                break;
                }
            case EWaitSetTrust:
                {
                if ( iStatus.Int() != KErrNone )
                    {
                    // Couldn't add changes to file, so restore old settings
                    TInt index = iCerts->Index( *iCertInfoReadOnly );
                    CWimCertStoreMapping* mapping = iCerts->Mapping( index );
                    mapping->SetTrusted( iOldTrusted );
                    }
                User::RequestComplete( iOriginalRequestStatus, iStatus.Int() );
                break;
                }
            case ERetrieve:
                {
                DoRetrieve();
                break;
                }
            case ERetrieveFromWim:
                {
                iPhase = ERetrieveWait;
                iCWimCertConverter->RetrieveCertByIndexL( iCertIndex,
                                                        *iEncodedCert,
                                                         iStatus );
                SetActive();
                break;
                }
            case ERetrieveWait:
                {
                iPhase = EIdle;
                User::RequestComplete( iOriginalRequestStatus, iStatus.Int() );
                break;
                }
            case EGetCorrespondingPrivateKey:
                {
                if ( iStatus.Int() == KErrNone )
                    {
                    iKeyFilter.iKeyId = *iSubjectKeyId;
                    iUnifiedKeyStore->List( iKeyInfos, iKeyFilter, iStatus );
                    iPhase = ECheckCorrespondingPrivateKey;
                    SetActive();
                    }
                else
                    {
                    iPhase = EIdle;
                    User::RequestComplete( iOriginalRequestStatus, iStatus.Int() );
                    }
                break;
                }
            case ECheckCorrespondingPrivateKey:
                {
                DoCheckCorrespondingPrivateKey();
                break;
                }
            case EAdd:
                {
                DoAdd();
                break;
                }
            case EAddToWim:
                {
                iPhase = ECheckAddToWim;
                // Update last Wim cache
                iCWimCertConverter->AddCertificate ( *iLabel,
                                                  iFormat,
                                                  iCertificateOwnerType,
                                                 *iSubjectKeyId,
                                                 *iIssuerKeyId,
                                                 *iCert,
                                                  iStatus );
                SetActive();
                break;
                }
            case ECheckAddToWim:
                {
                if ( iStatus.Int() == KErrNone )
                    {
                    iPhase = EList;
                    iPhaseOriginal = ECompleteMessage;
                    TRequestStatus* status = &iStatus;
                    User::RequestComplete( status, KErrNone );
                    SetActive();
                    }
                else
                    {
                    iPhase = EIdle;
                    User::RequestComplete( iOriginalRequestStatus, iStatus.Int() );
                    }
                break;
                }
            case ERemove:
                {
                DoRemove();
                break;
                }
            case EDeleteFromWim:
                {
                iPhase = ECheckDeleteFromWim;
                // Update Wim cache
                iCWimCertConverter->RemoveL( iCertIndex, iStatus );
                SetActive();
                break;
                }
            case ECheckDeleteFromWim:
                {
                DoCheckDeleteFromWim();
                break;
                }
            case EWaitRemoveTrustSettings:
                {
                if ( iStatus.Int() == KErrNone )
                    {
                    iPhase = EList;
                    iPhaseOriginal = ECompleteMessage;
                    TRequestStatus* status = &iStatus;
                    User::RequestComplete( status, KErrNone );
                    SetActive();
                    }
                else
                    {
                    iPhase = EIdle;
                    User::RequestComplete( iOriginalRequestStatus,
                                           iStatus.Int() );
                    }
                break;
                }
            case ECompleteMessage:
                {
                iPhase = EIdle;
                User::RequestComplete( iOriginalRequestStatus, iStatus.Int() );
                break;
                }
            default:
                {
                // Here we should not be
                User::RequestComplete( iOriginalRequestStatus, KErrCorrupt );
                break;
                }
            } // switch
        } // if
    }

// -----------------------------------------------------------------------------
// void CWimCertStore::DoInitializeGetCertListL()
// Certificates are fetched from Wim
// -----------------------------------------------------------------------------
//
void CWimCertStore::DoInitializeGetCertListL()
    {
    _WIMTRACE ( _L( "CWimCertStore::DoInitializeGetCertListL()" ) );
    iCertInfos.ResetAndDestroy();
    if ( iCWimCertConverter )
        {
        delete iCWimCertConverter;
        iCWimCertConverter = NULL;
        }
    iCWimCertConverter = CWimCertConverter::NewL( Token() );
    iStatus = KRequestPending;
    // Call converter to fetch certificate infos from Wim
    iPhase = EInitializeLoadMappings;
    iCWimCertConverter->Restore( iCertInfos, iStatus );
    SetActive();
    }

// -----------------------------------------------------------------------------
// void CWimCertStore::DoInitializeLoadMappingsL()
// Load certificates into mappings
// -----------------------------------------------------------------------------
//
void CWimCertStore::DoInitializeLoadMappingsL()
    {
    _WIMTRACE ( _L( "CWimCertStore::DoInitializeLoadMappingsL()" ) );
     if ( iStatus.Int() == KErrNone || iStatus.Int() == KErrNotFound )
        {
        // Load certificate infos into mappings
        LoadMappingsL();
        iCertIndex = 0;
        if ( iCertInfos.Count() > 0 )
            {
            iPhase = EInitializeLoadTrustSettingsStart;
            }
        else
            {
            iPhase = EListGo;
            }
        TRequestStatus* status = &iStatus;
        User::RequestComplete( status, KErrNone );
        SetActive();
        }
     else // Something went wrong with Restore or Cancel call was issued
        {
        FreeUnifiedKeyStore();
        iPhase = EIdle;
        User::RequestComplete( iOriginalRequestStatus, iStatus.Int() );
        }
    }
// -----------------------------------------------------------------------------
// void CWimCertStore::DoInitializeLoadTrustSettingsStartL()
// Fetch trust settings for a certificate
// -----------------------------------------------------------------------------
//
void CWimCertStore::DoInitializeLoadTrustSettingsStartL()
    {
    _WIMTRACE ( _L( "CWimCertStore::DoInitializeLoadTrustSettingsStartL()" ) );
    switch ( iStatus.Int() )
        {
        case KErrNone:
            {
            // Application array is created here
            // Ownership is transferred to CWimCertStoreMapping class
            iCertificateApps = new( ELeave ) RArray<TUid>();
            iStatus = KRequestPending;
            iCWimTrustSettingsStore->
               GetTrustSettings( *iCertInfos[iCertIndex],
                                  iTrusted,
                                 *iCertificateApps,
                                  iStatus );
            iPhase = EInitializeLoadTrustSettingsWait;
            SetActive();
            break;
            }
        case KErrArgument:
            {
            // Certificate data was corrupted. Skip default trustsettings.
            if ( iCertIndex < iCerts->Count() - 1 )
                {
                iCertIndex++;
                iPhase = EInitializeLoadTrustSettingsStart;
                }
            else
                {
                iCertIndex = 0;
                iPhase = EListGo;
                }
            TRequestStatus* status = &iStatus;
            User::RequestComplete( status, KErrNone );
            SetActive();
            break;
            }
        default:
            {
            FreeUnifiedKeyStore();
            iPhase = EIdle;
            User::RequestComplete( iOriginalRequestStatus, iStatus.Int() );
            }
        }
    }

// -----------------------------------------------------------------------------
// void CWimCertStore::DoInitializeLoadTrustSettingsWaitL()
// Check if trust settings were found, if not, do them and assign to mappings
// -----------------------------------------------------------------------------
//
void CWimCertStore::DoInitializeLoadTrustSettingsWaitL()
    {
    _WIMTRACE ( _L( "CWimCertStore::DoInitializeLoadTrustSettingsWaitL()" ) );
    switch ( iStatus.Int() )
        {
        case KErrNone:
            {
            // Parameters are fetched from trust store
            // and assigned into mapping
            SetTrustSettingsOnMappingL( iTrusted,
                                        iCertificateApps );
            if ( iCertIndex < iCerts->Count() - 1 )
                {
                iCertIndex++;
                iPhase = EInitializeLoadTrustSettingsStart;
                }
            else
                {
                iCertIndex = 0;
                iPhase = EListGo;
                }
            TRequestStatus* status = &iStatus;
            User::RequestComplete( status, KErrNone );
            SetActive();
            break;
            }
        case KErrNotFound: // Trust settings not found; let's do
            {
            iStatus = KRequestPending;
            iCWimTrustSettingsStore->SetDefaultTrustSettings( 
                                                        *iCertInfos[iCertIndex],
                                                        ETrue,
                                                        iStatus );
            iPhase = EInitializeLoadTrustSettingsStart;

            // Nobody is taking ownership of these so delete them
            if ( iCertificateApps )
                {
                iCertificateApps->Close();
                delete iCertificateApps;
                iCertificateApps = NULL;
                }

            SetActive();
            break;
            }
        default: // Something went wrong with GetTrustSettings
                // or Cancel call was issued
            {
            // Nobody is taking ownership of these so delete them
            if ( iCertificateApps )
                {
                iCertificateApps->Close();
                delete iCertificateApps;
                iCertificateApps = NULL;
                }

            FreeUnifiedKeyStore();
            iPhase = EIdle;
            User::RequestComplete( iOriginalRequestStatus,
                                   iStatus.Int() );
            break;
            }
        } // switch
    }

// -----------------------------------------------------------------------------
// void CWimCertStore::DoGetKeyInfos()
// Fetch keys from keystores
// -----------------------------------------------------------------------------
//
void CWimCertStore::DoGetKeyInfos()
    {
    _WIMTRACE ( _L( "CWimCertStore::DoGetKeyInfos()" ) );
    if ( iStatus.Int() == KErrNone )
        {
        // In this phase key info count must allways be zero
        if ( iKeyInfos.Count() == 0 )
            {
            iCertIndex = 0;
            iKeyFilter.iKeyId = KNullDesC8;
            iKeyFilter.iUsage =
                KeyUsageX509ToPKCS15Private( iFilter->iKeyUsage );
            iPhase = EList;
            iPhaseOriginal = EList;
            iUnifiedKeyStore->List( iKeyInfos, iKeyFilter, iStatus );
            SetActive();
            }
        else
            {
            FreeUnifiedKeyStore();
            iPhase = EIdle;
            User::RequestComplete( iOriginalRequestStatus, KErrCorrupt );
            }
        }
    else // Something went wrong (or call was cancelled) with
         // iUnifiedKeyStore->Initialize( iStatus )
        {
        FreeUnifiedKeyStore();
        iPhase = EIdle;
        User::RequestComplete( iOriginalRequestStatus, iStatus.Int() );
        }
    }


// -----------------------------------------------------------------------------
// void CWimCertStore::DoList()
// Start listing certificates
// -----------------------------------------------------------------------------
//
void CWimCertStore::DoList()
    {
    _WIMTRACE ( _L( "CWimCertStore::DoList()" ) );
    if ( iStatus.Int() == KErrNone )
        {
        iCertIndex = 0;
        iPhase = EInitializeGetCertList;
        TRequestStatus* status = &iStatus;
        User::RequestComplete( status, KErrNone );
        SetActive();
        }
    else // iUnifiedKeyStore->List call has been cancelled or failed
        {
        FreeUnifiedKeyStore();
        iPhase = EIdle;
        User::RequestComplete( iOriginalRequestStatus, iStatus.Int() );
        }
    }

// -----------------------------------------------------------------------------
// void CWimCertStore::DoListGoL()
// In this phase all certificates are fetched from Wim.
// Serve the original request.
// -----------------------------------------------------------------------------
//
void CWimCertStore::DoListGoL()
    {
    _WIMTRACE ( _L( "CWimCertStore::DoListGoL()" ) );
    if ( iPhaseOriginal == EList )
        {
        if ( iCertIndex < ( iCerts->Count() ) )
            {
            const CCTCertInfo& certInfo = iCerts->Entry( iCertIndex );
            TBool accept = ETrue;
            if ( iFilter->iUidIsSet )
                {
                accept = iCerts->Mapping( iCertIndex )->
                                 IsApplicable( iFilter->iUid );
                }
            if ( iFilter->iFormatIsSet && accept )
                {
                accept = ( iFilter->iFormat == certInfo.CertificateFormat() );
                }
            if ( iFilter->iOwnerTypeIsSet && accept )
                {
                accept = ( iFilter->iOwnerType == certInfo.CertificateOwnerType() );
                }
            if ( ( iFilter->iSubjectKeyId != KNullDesC8 ) && accept )
                {
                accept = ( iFilter->iSubjectKeyId == certInfo.SubjectKeyId() );
                }
            if ( ( iFilter->iIssuerKeyId != KNullDesC8 ) && accept )
                {
                accept = ( iFilter->iIssuerKeyId == certInfo.IssuerKeyId() );
                }
            if ( ( iFilter->iLabelIsSet ) && accept )
                {
                accept = ( iFilter->iLabel == certInfo.Label() );
                }
            if ( ( iFilter->iKeyUsage != EX509UsageAll ) && accept &&
                 ( certInfo.CertificateOwnerType() == EUserCertificate) )
                {
                // This test must be done after we checked that
                // the certificate owner is a user cert
                // We must get the private key info associated with the
                // certificate so that we know the usages
                TInt end = iKeyInfos.Count();
                TInt i = 0;
                for ( ; i < end; i++ )
                    {
                    if ( iKeyInfos[i]->ID() == certInfo.SubjectKeyId() )
                        {
                        i = end + 1; // This completes loop and
                                     // differentiates from normal
                                    // ending (i == end)
                        }
                    }
                if ( i == end )
                    {
                    accept = EFalse;
                    }
                }

            if ( accept )
                {
                // Here is done another copy of certificate for
                // application needs
                CCTCertInfo* copy = CCTCertInfo::NewLC( certInfo );
                User::LeaveIfError( iCertsList->Append( copy ) );
                CleanupStack::Pop( copy );
                }

            // iCertIndex is initialized in EList/KErrNone
            iCertIndex++;

            // Poll status in order to give time for other aos
            TRequestStatus* status = &iStatus;
            User::RequestComplete( status, KErrNone );
            SetActive();
            } // if ( iCertIndex...
        else
            {
            // All certificates are listed or there are not any
            iKeyInfos.Close();
            delete iUnifiedKeyStore;
            iUnifiedKeyStore = NULL;
            iPhase = EIdle;
            User::RequestComplete( iOriginalRequestStatus, KErrNone );
            }
        }
    else
        {
        iPhase = iPhaseOriginal;
        iPhaseOriginal = EIdle;
        TRequestStatus* status = &iStatus;
        User::RequestComplete( status, KErrNone );
        SetActive();
        }
    }

// -----------------------------------------------------------------------------
// void CWimCertStore::DoCheckCorrespondingPrivateKey()
// Check if private key is found for user certificate
// -----------------------------------------------------------------------------
//
void CWimCertStore::DoCheckCorrespondingPrivateKey()
    {
    _WIMTRACE ( _L( "CWimCertStore::DoCheckCorrespondingPrivateKey()" ) );
    if ( iStatus.Int() == KErrNone )
        {
        if ( ( iKeyInfos.Count() == 0 ) ||
           ( ( iFormat != EX509CertificateUrl ) &&
             ( iKeyInfos[0]->Usage() != iKeyFilter.iUsage ) ) )
            {
            // The private key can't be found in any key store,
            // so we must return an error
            iPhase = EIdle;
            User::RequestComplete( iOriginalRequestStatus, KErrArgument );
            }
        else
            {
            //
            iPhase = EAdd;
            TRequestStatus* status = &iStatus;
            User::RequestComplete( status, KErrNone );
            SetActive();
            }
        }
    else
        {
        iPhase = EIdle;
        User::RequestComplete( iOriginalRequestStatus, iStatus.Int() );
        }
    iKeyInfos.Close();
    delete iUnifiedKeyStore;
    iUnifiedKeyStore = NULL;
    }

// -----------------------------------------------------------------------------
// void CWimCertStore::DoCheckDeleteFromWim()
// If delete from Wim succeeded, continue deleting from trust setting store
// -----------------------------------------------------------------------------
//
void CWimCertStore::DoCheckDeleteFromWim()
    {
    _WIMTRACE ( _L( "CWimCertStore::DoCheckDeleteFromWim()" ) );
    if ( iStatus.Int() == KErrNone )
        {
        iStatus = KRequestPending;
        iPhase = EWaitRemoveTrustSettings;
        iCWimTrustSettingsStore->RemoveTrustSettings(
                                            *iCertInfos[iCertIndex], iStatus );
        SetActive();
        }
    else
        {
        iPhase = EIdle;
        User::RequestComplete( iOriginalRequestStatus, iStatus.Int() );
        }
    }

// -----------------------------------------------------------------------------
// void CWimCertStore::FreeUnifiedKeyStore()
// Frees key storage resources.
// -----------------------------------------------------------------------------
//
void CWimCertStore::FreeUnifiedKeyStore()
    {
    _WIMTRACE ( _L( "CWimCertStore::FreeUnifiedKeyStore()" ) );
    if ( iUnifiedKeyStore )
        {
        iKeyInfos.Close();
        delete iUnifiedKeyStore;
        iUnifiedKeyStore = NULL;
        iCertsList->Reset();
        }
    }

// -----------------------------------------------------------------------------
// void CWimCertStore::RunError()
// Unexpected error in RunL (e.g. Leave) leads us here.
// -----------------------------------------------------------------------------
//
TInt CWimCertStore::RunError( TInt aError )
    {
    _WIMTRACE ( _L( "CWimCertStore::RunError()" ) );

    FreeUnifiedKeyStore();
    iPhase = EIdle;
    User::RequestComplete( iOriginalRequestStatus, aError );
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// void CWimCertStore::DoCancel()
// Cancels current operation.
// -----------------------------------------------------------------------------
//
void CWimCertStore::DoCancel()
    {
    _WIMTRACE ( _L( "CWimCertStore::DoCancel()" ) );

    if ( iUnifiedKeyStore )
        {
        switch ( iPhase )
            {
            case EGetKeyInfos:
                {
                if ( iUnifiedKeyStore->IsActive() )
                    {
                    iUnifiedKeyStore->CancelInitialize();
                    }
                break;
                }
            case EList:
                {
                if ( iUnifiedKeyStore->IsActive() )
                    {
                    iUnifiedKeyStore->CancelList();
                    }
                break;
                }
            default:
                {
                // Other phases won't cause any action
                break;
                }
            }
        }

    if ( iCWimCertConverter )
        {
        switch ( iPhase )
            {
            case EInitializeLoadMappings:
                {
                if ( iCWimCertConverter->IsActive() )
                    {
                    iCWimCertConverter->CancelRestore();
                    delete iCWimCertConverter;
                    iCWimCertConverter = NULL;
                    }
                break;
                }
            case ERetrieveWait:
                {
                if ( iCWimCertConverter->IsActive() )
                    {
                    iCWimCertConverter->CancelRetrieve();
                    delete iCWimCertConverter;
                    iCWimCertConverter = NULL;
                    }
                break;
                }
            case ECheckAddToWim:
                {
                if ( iCWimCertConverter->IsActive() )
                    {
                    iCWimCertConverter->CancelAddCertificate();
                    delete iCWimCertConverter;
                    iCWimCertConverter = NULL;
                    }
                break;
                }
            case ECheckDeleteFromWim:
                {
                if ( iCWimCertConverter->IsActive() )
                    {
                    iCWimCertConverter->CancelRemove();
                    delete iCWimCertConverter;
                    iCWimCertConverter = NULL;
                    }
                break;
                }
            default:
                {
                // Other phases won't cause any action
                break;
                }
            }
        }

    if ( iCWimTrustSettingsStore )
        {
        switch ( iPhase )
            {
            case EInitializeLoadTrustSettingsStart:
                {
                if ( iCWimTrustSettingsStore->IsActive() )
                    {
                    iCWimTrustSettingsStore->CancelDoing();
                    }
                break;
                }
            default:
                {
                // Other phases won't cause any action
                break;
                }
            }
        }

    if ( iCerts )
        {
        delete iCerts;
        iCerts = NULL;
        }

    if ( iPhase == EInitializeLoadTrustSettingsWait )
        {
        // Nobody is taking ownership of these so delete them
        iCertificateApps->Close();
        delete iCertificateApps;
        }

    FreeUnifiedKeyStore();
    iPhase = EIdle;
    User::RequestComplete( iOriginalRequestStatus, KErrCancel );
    }

// -----------------------------------------------------------------------------
// void CWimCertStore::LoadMappingsL()
// In this phase we have retrieved certificate infos from WimClient and they
// are in the iCertInfos array. This methdod creates iCerts array
// and loads those certificates into it. Trust settings are updated in the next
// phase.
// -----------------------------------------------------------------------------
//
void CWimCertStore::LoadMappingsL()
    {
    _WIMTRACE ( _L( "CWimCertStore::LoadMappingsL()" ) );
    if ( iCerts )
        {
        delete iCerts;
        iCerts = NULL;
        }
    // Create a manager class for mapping entries
    iCerts = CWimCertStoreMappings::NewL();
    TInt count = iCertInfos.Count();
    // Go through all certificates and insert them into a mapping array
    for ( TInt i = 0; i < count; i++ )
        {
        CWimCertStoreMapping* certMapping = CWimCertStoreMapping::NewL();
        CleanupStack::PushL( certMapping );
        // Ownership moves to CWimCertStoreMapping
        CCTCertInfo* certInfo = ( CCTCertInfo* )( iCertInfos )[i]->CctCert();
        certMapping->SetEntryL( certInfo );
        // Set default applications. This object must not push to cleanupstack
        // because on leave at AddL this object is deleted in association with
        // certMapping
        RArray<TUid>* certificateApps = new( ELeave ) RArray<TUid>();
        certMapping->SetCertificateAppsL( certificateApps );
        // Set default
        certMapping->SetTrusted( ETrue );
        // Append mapping pointer to pointer array
        iCerts->AddL( certMapping );
        CleanupStack::Pop( certMapping );
        }
    }

// -----------------------------------------------------------------------------
// void CWimCertStore::SetTrustSettingsOnMappingL()
// If certificate has no trust settings and there should be,
// here they are updated. This method sets applications and trusted
// information into mapping entry.
// -----------------------------------------------------------------------------
//
void CWimCertStore::SetTrustSettingsOnMappingL( TBool aTrusted,
                                                RArray<TUid>* aApplications )
    {
    _WIMTRACE ( _L( "CWimCertStore::SetTrustSettingsOnMappingL()" ) );
    // Take a pointer to mapping
    CWimCertStoreMapping* mapping = iCerts->Mapping( iCertIndex );
    // Check that mapping is found
    if ( !mapping )
        {
        User::Leave( KErrNotFound );
        }
    // Ownership is changed here:
    // iCertificateApps is not any more responsible for this array.
    mapping->SetCertificateAppsL( aApplications );
    mapping->SetTrusted( aTrusted );
    }

// -----------------------------------------------------------------------------
// CWimCertStore::EnteringAllowed()
// Check if token is removed and if this ao is active.
// -----------------------------------------------------------------------------
//
TBool CWimCertStore::EnteringAllowed( TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimCertStore::EnteringAllowed()" ) );
    if ( TokenRemoved() )
        {
        TRequestStatus* status = &aStatus;
        User::RequestComplete( status, KErrHardwareNotAvailable );
        return EFalse;
        }

    // If this active object is in running, don't accept entering
    if ( IsActive() )
        {
        // If the caller status is the same as the status, that activated
        // this object, just return
        if ( &aStatus == iOriginalRequestStatus )
            {
            return EFalse;
            }
        else
            {
            // Otherwise complete it with error
            TRequestStatus* status = &aStatus;
            User::RequestComplete( status, KErrInUse );
            return EFalse;
            }
        }
    else
        {
        iOriginalRequestStatus = &aStatus;
        aStatus = KRequestPending;
        return ETrue;
        }
    }

// -----------------------------------------------------------------------------
// CWimCertStore::TokenRemoved()
// Returns true or false indicating if token is removed
// -----------------------------------------------------------------------------
//
TBool CWimCertStore::TokenRemoved()
    {
    _WIMTRACE ( _L( "CWimCertStore::TokenRemoved()" ) );
    // If token listener is not alive, then token is removed
    if ( iToken.TokenListener()->TokenStatus() != KRequestPending )
        {
        return ETrue;
        }
    else
        {
        return EFalse;
        }
    }
