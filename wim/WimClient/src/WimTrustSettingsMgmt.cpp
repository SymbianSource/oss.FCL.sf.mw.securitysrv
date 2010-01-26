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
* Description:  Trust Settings management class
*
*/


//INCLUDES
#include "WimTrustSettingsMgmt.h"
#include "WimCertInfo.h"
#include "WimTrace.h"


// -----------------------------------------------------------------------------
// RWimTrustSettingsMgmt::RWimTrustSettingsMgmt()
// Default constructor
// -----------------------------------------------------------------------------
//
RWimTrustSettingsMgmt::RWimTrustSettingsMgmt() : iPckgBufTrustSettings( NULL ),
																								 iCertPkcg( NULL )
    {
    }

// -----------------------------------------------------------------------------
// RWimTrustSettingsMgmt::ClientSessionL()
// Return new RWimTrustSettingsMgmt object 
// -----------------------------------------------------------------------------
//
RWimTrustSettingsMgmt* RWimTrustSettingsMgmt::ClientSessionL()
    {
    RWimTrustSettingsMgmt* self = new( ELeave ) RWimTrustSettingsMgmt;
    CleanupStack::PushL( self );
    User::LeaveIfError( self->Connect() );
    CleanupStack::Pop( self );
    _WIMTRACE ( _L( "RWimTrustSettingsMgmt::ClientSessionL()" ) );
    return self;
    }

// -----------------------------------------------------------------------------
// RWimTrustSettingsMgmt::~RWimTrustSettingsMgmt() 
// Destructor, all allocated memory is released.
// -----------------------------------------------------------------------------
//
RWimTrustSettingsMgmt::~RWimTrustSettingsMgmt()
    {
    _WIMTRACE ( _L( "RWimTrustSettingsMgmt::~RWimTrustSettingsMgmt()" ) );
    DeleteBuffers();
    }

// -----------------------------------------------------------------------------
// RWimTrustSettingsMgmt::GetTrustSettings()
// Return trust settings for given certificate.
// -----------------------------------------------------------------------------
//
void RWimTrustSettingsMgmt::GetTrustSettingsL( 
    const CWimCertInfo& aCert,
    TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "RWimTrustSettingsMgmt::GetTrustSettings()" ) );

    TTrustSettings trustSettings;
    trustSettings.iTrusted = EFalse; // Initialised to remove warning

    if ( iPckgBufTrustSettings || iCertPkcg )
        {
        DeleteBuffers();
        }
    iPckgBufTrustSettings = new( ELeave ) CWimCertPckgBuf<TTrustSettings>( 
                                                                trustSettings );
    TWimCertInfoPckg* certInfo = aCert.ExternalizeL();

    iCertPkcg = new( ELeave ) CWimCertPckgBuf<TWimCertInfoPckg>( *certInfo );

    TIpcArgs args;
    args.Set( 0, iCertPkcg->PckgBuf() );
    args.Set( 1, iPckgBufTrustSettings->PckgBuf() );
    
    SendReceiveData( EGetTrustSettings, args, aStatus );
    
    delete certInfo;
    }

// -----------------------------------------------------------------------------
// RWimTrustSettingsMgmt::SetApplicabilityL()
// Set applicability for certificate.
// -----------------------------------------------------------------------------
//
void RWimTrustSettingsMgmt::SetApplicabilityL(
    const CWimCertInfo& aCert,
    const RArray<TUid>& aApplications,
    TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "RWimTrustSettingsMgmt::SetApplicabilityL()" ) );
    
    TInt applicationCount = aApplications.Count();

    // Check that there is no more applications than allocated in array for
    if ( applicationCount > KMaxApplicationCount )
        {
        User::Leave( KErrOverflow );
        }

    TTrustSettings trustSettings;
    for ( TInt i = 0; i < applicationCount; i++ )
        {
        trustSettings.iUids[i] = aApplications[i].iUid;
        }
    trustSettings.iApplicationCount = aApplications.Count();

    if ( iPckgBufTrustSettings || iCertPkcg )
        {
        DeleteBuffers();
        }
    iPckgBufTrustSettings = new( ELeave ) CWimCertPckgBuf<TTrustSettings>(
                                                                trustSettings );

    TWimCertInfoPckg* certInfo = aCert.ExternalizeL();

    iCertPkcg = new( ELeave ) CWimCertPckgBuf<TWimCertInfoPckg>( *certInfo );

    TIpcArgs args;
    args.Set( 0, iCertPkcg->PckgBuf() );
    args.Set( 1, iPckgBufTrustSettings->PckgBuf() );

    SendReceiveData( ESetApplicability, args, aStatus );
    
    delete certInfo;
    }

// -----------------------------------------------------------------------------
// RWimTrustSettingsMgmt::SetTrustL()
// Set trust flag for certificate.
// -----------------------------------------------------------------------------
//
void RWimTrustSettingsMgmt::SetTrustL(
    const CWimCertInfo& aCert,
    TBool aTrusted,
    TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "RWimTrustSettingsMgmt::SetTrustL()" ) );

    TTrustSettings trustSettings;
    trustSettings.iTrusted = aTrusted;

    if ( iPckgBufTrustSettings || iCertPkcg )
        {
        DeleteBuffers();
        }
    iPckgBufTrustSettings = new( ELeave ) CWimCertPckgBuf<TTrustSettings>(
                                                                trustSettings );

    TWimCertInfoPckg* certInfo = aCert.ExternalizeL();

    iCertPkcg = new( ELeave ) CWimCertPckgBuf<TWimCertInfoPckg>( *certInfo );

    TIpcArgs args;
    args.Set( 0, iCertPkcg->PckgBuf() );
    args.Set( 1, iPckgBufTrustSettings->PckgBuf() );

    SendReceiveData( ESetTrust, args, aStatus );
    
    delete certInfo;
    }

// -----------------------------------------------------------------------------
// RWimTrustSettingsMgmt::SetDefaultTrustSettingsL()
// Set default trust settings for certificate.
// -----------------------------------------------------------------------------
//
void RWimTrustSettingsMgmt::SetDefaultTrustSettingsL(
    const CWimCertInfo& aCert,
    TBool aAddApps,
    TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "RWimTrustSettingsMgmt::SetDefaultTrustSettingsL()" ) );

    if ( iPckgBufTrustSettings || iCertPkcg )
        {
        DeleteBuffers();
        }
    TWimCertInfoPckg* certInfo = aCert.ExternalizeL();

    iCertPkcg = new( ELeave ) CWimCertPckgBuf<TWimCertInfoPckg>( *certInfo );

    TIpcArgs args;
    args.Set( 0, iCertPkcg->PckgBuf() );
    args.Set( 1, aAddApps );

    SendReceiveData( ESetDefaultTrustSettings, args, aStatus );

    delete certInfo;
    }

// -----------------------------------------------------------------------------
// RWimTrustSettingsMgmt::RemoveTrustSettingsL()
// Remove trust settings of given certificate
// -----------------------------------------------------------------------------
//
void RWimTrustSettingsMgmt::RemoveTrustSettingsL(
    const CWimCertInfo& aCert,
    TRequestStatus& aStatus )
    {
    _WIMTRACE( _L( "RWimTrustSettingsMgmt::RemoveTrustSettingsL()" ) );

    if ( iPckgBufTrustSettings || iCertPkcg )
        {
        DeleteBuffers();
        }
    TWimCertInfoPckg* certInfo = aCert.ExternalizeL();

    iCertPkcg = new( ELeave ) CWimCertPckgBuf<TWimCertInfoPckg>( *certInfo );

    TIpcArgs args;
    args.Set( 0, iCertPkcg->PckgBuf() );

    SendReceiveData( ERemoveTrustSettings, args, aStatus );
    
    delete certInfo;
    }

// -----------------------------------------------------------------------------
// RWimTrustSettingsMgmt::CancelDoing
// Cancel any asynchronous operation ongoing
// -----------------------------------------------------------------------------
//
void RWimTrustSettingsMgmt::CancelDoing()
    {
    _WIMTRACE( _L( "RWimTrustSettingsMgmt::CancelDoing()" ) );

    TIpcArgs args;

    SendReceiveData( ECancelTrustSettings, args );
    }

// -----------------------------------------------------------------------------
// RWimTrustSettingsMgmt::TrustSettingsPckg
// Return pointer to trust settings package.
// -----------------------------------------------------------------------------
//
CWimCertPckgBuf<TTrustSettings>* RWimTrustSettingsMgmt::TrustSettingsPckg()
    {
    return iPckgBufTrustSettings;
    }

// -----------------------------------------------------------------------------
// RWimTrustSettingsMgmt::DeleteBuffers
// Deletes packet buffers
// -----------------------------------------------------------------------------
//
void RWimTrustSettingsMgmt::DeleteBuffers()
    {
    _WIMTRACE( _L( "RWimTrustSettingsMgmt::DeleteBuffers()" ) );

    delete iCertPkcg;
    delete iPckgBufTrustSettings;
    iCertPkcg = NULL;
    iPckgBufTrustSettings = NULL;
    }

// End of File
