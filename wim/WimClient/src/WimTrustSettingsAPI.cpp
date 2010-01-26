/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Interface for WIM Trust Settings Store
*
*/



// INCLUDE FILES
#include "WimTrustSettingsAPI.h"
#include "WimCertInfo.h"
#include "WimTrustSettingsMgmt.h"
#include "WimTrace.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWimTrustSettingsAPI::CWimTrustSettingsAPI()
// Default constructor.
// -----------------------------------------------------------------------------
//
CWimTrustSettingsAPI::CWimTrustSettingsAPI() : CActive( EPriorityNormal )
    {
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsAPI::ConstructL()
// Second phase constructor.
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsAPI::ConstructL()
    {
    _WIMTRACE ( _L( "CWimTrustSettingsAPI::ConstructL()" ) );
    iConnectionHandle = RWimTrustSettingsMgmt::ClientSessionL();  
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsAPI::NewL()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CWimTrustSettingsAPI* CWimTrustSettingsAPI::NewL()
    {
    _WIMTRACE ( _L( "CWimTrustSettingsAPI::NewL()" ) );
    CWimTrustSettingsAPI* self = new( ELeave ) CWimTrustSettingsAPI;
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsAPI::~CWimTrustSettingsAPI()
// Destructor. If updates has been done, compact databse.
// -----------------------------------------------------------------------------

EXPORT_C CWimTrustSettingsAPI::~CWimTrustSettingsAPI()
    {
    _WIMTRACE ( _L( "CWimTrustSettingsAPI::~CWimTrustSettingsAPI()" ) );
    iConnectionHandle->Close();
    delete iConnectionHandle;
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsAPI::Close()
// Closes all open resources and deletes this instance of the TrustSettingsStore
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimTrustSettingsAPI::Close()
    {
    _WIMTRACE ( _L( "CWimTrustSettingsAPI::Close()" ) );
    delete ( this );
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsAPI::GetTrustSettings()
// Return trust settings for given certificate.
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimTrustSettingsAPI::GetTrustSettings( 
    const CWimCertInfo& aCert,
    TBool& aTrusted,
    RArray<TUid>& aApplications, 
    TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimTrustSettingsAPI::GetTrustSettings()" ) );
    
    iApplications = &aApplications;
    iTrusted = &aTrusted;

    iClientStatus = &aStatus;
    iStatus = KRequestPending;
    iPhase = EGetTrustSettings;

    TRAPD( error, iConnectionHandle->GetTrustSettingsL( aCert,
                                                        iStatus ) );
    SetActive();

    HandleLeaveError( error );
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsAPI::SetApplicability()
// Set applicability for certificate.
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimTrustSettingsAPI::SetApplicability(
    const CWimCertInfo& aCert,
    const RArray<TUid>& aApplications,
    TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimTrustSettingsAPI::SetApplicability()" ) );
    iClientStatus = &aStatus;
    iStatus = KRequestPending;
    iPhase = ESetApplicability;

    SetActive();
    
    TRAPD( error,
        iConnectionHandle->SetApplicabilityL( aCert, aApplications, iStatus ) );

    HandleLeaveError( error );
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsAPI::SetTrust()
// Set trust flag for certificate.
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimTrustSettingsAPI::SetTrust(
    const CWimCertInfo& aCert,
    TBool aTrusted,
    TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimTrustSettingsAPI::SetTrust()" ) );
    iClientStatus = &aStatus;
    iStatus = KRequestPending;
    iPhase = ESetTrust;

    TRAPD( error, iConnectionHandle->SetTrustL( aCert, aTrusted, iStatus ) );
    SetActive();
    HandleLeaveError( error );
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsAPI::SetDefaultTrustSettings()
// Set default trust settings for certificate.
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimTrustSettingsAPI::SetDefaultTrustSettings(
    const CWimCertInfo& aCert,
    TBool aAddApps,
    TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimTrustSettingsAPI::SetDefaultTrustSettings()" ) );

    iClientStatus = &aStatus;
    iStatus = KRequestPending;
    iPhase = ESetDefaultTrustSettings;

    TRAPD( error, iConnectionHandle->SetDefaultTrustSettingsL( aCert,
                                                               aAddApps,
                                                               iStatus ) );
    SetActive();   
    HandleLeaveError( error );
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsAPI::RemoveTrustSettings()
// Remove trust settings of given certificate
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimTrustSettingsAPI::RemoveTrustSettings(
    const CWimCertInfo& aCert,
    TRequestStatus& aStatus )
    {
    _WIMTRACE( _L( "CWimTrustSettingsAPI::RemoveTrustSettings()" ) );
    
    iClientStatus = &aStatus;
    iStatus = KRequestPending;
    iPhase = ERemoveTrustSettings;

    TRAPD( error,
        iConnectionHandle->RemoveTrustSettingsL( aCert, iStatus ) );
    SetActive();
    HandleLeaveError( error );
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsAPI::CancelDoing()
// Cancel ongoing operation
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimTrustSettingsAPI::CancelDoing()
    {
    _WIMTRACE( _L( "CWimTrustSettingsAPI::CancelDoing()" ) );
    iConnectionHandle->CancelDoing();
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsAPI::RunL()
// Handle result of asynchronous Trust Settings Store operation.
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsAPI::RunL()
    {
    _WIMTRACE2( _L( "CWimTrustSettingsAPI::RunL(), iStatus=%d" ),
               iStatus.Int() );

    switch ( iPhase )
        {
        case EGetTrustSettings:
            {
            if ( iStatus.Int() == KErrNone )
                {
                TPckgBuf<TTrustSettings> trustSettingsPckgBuf;
    
                trustSettingsPckgBuf = 
                    ( *iConnectionHandle->TrustSettingsPckg()->PckgBuf() );

                TUid uid;

                for ( TInt i = 0;
                    i < trustSettingsPckgBuf().iApplicationCount; i++ )
                    {
                    uid.iUid = trustSettingsPckgBuf().iUids[i];

                    iApplications->Append( uid );
                    }
                *iTrusted = trustSettingsPckgBuf().iTrusted;
                }

            break;
            }

        default:
            {
            break;
            }
        }
    User::RequestComplete( iClientStatus, iStatus.Int() );
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsAPI::DoCancel()
// 
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsAPI::DoCancel()
    {

    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsAPI::HandleLeaveError()
// Handle trapped leave. Complete client request with error code.
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsAPI::HandleLeaveError( TInt aError )
    {
    _WIMTRACE2( _L( "CWimTrustSettingsAPI::HandleLeaveError(), aError=%d" ),
                aError );
    if ( aError != KErrNone )
        {
        TRequestStatus* status = &iStatus;
        User::RequestComplete( status, aError );
        }
    }

//  End of File  
