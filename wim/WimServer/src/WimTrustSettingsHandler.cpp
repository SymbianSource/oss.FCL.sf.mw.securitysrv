/*
* Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Handler class for Trust Settings Store calls from WimClient
*
*/



// INCLUDE FILES
#include    "WimTrustSettingsHandler.h"
#include    "WimTrustSettingsStore.h"
#include    "WimClsv.h"
#include    "WimCertInfo.h"
#include    "WimTrace.h"

#ifdef _DEBUG
_LIT( KWimTrustSettPanic, "WimTrustSett" );
#endif

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWimTrustSettingsHandler::CWimTrustSettingsHandler
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CWimTrustSettingsHandler::CWimTrustSettingsHandler( 
    CWimTrustSettingsStore* aWimTrustSettingsStore )
    : CActive( EPriorityStandard ),
      iWimTrustSettingsStore( aWimTrustSettingsStore )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTrustSettingsHandler::CWimTrustSettingsHandler | Begin"));
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsHandler::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsHandler::ConstructL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTrustSettingsHandler::ConstructL | Begin"));
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsHandler::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CWimTrustSettingsHandler* CWimTrustSettingsHandler::NewL( 
    CWimTrustSettingsStore* aWimTrustSettingsStore ) 
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTrustSettingsHandler::NewL | Begin"));
    CWimTrustSettingsHandler* self = new( ELeave ) CWimTrustSettingsHandler( 
    aWimTrustSettingsStore );
 
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    return self;
    }

    
// Destructor
CWimTrustSettingsHandler::~CWimTrustSettingsHandler()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTrustSettingsHandler::~CWimTrustSettingsHandler | Begin"));
    iApplications.Close();
    delete iCertInfo;
    iWimTrustSettingsStore = NULL;  // not owned
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsHandler::GetTrustSettingsL
// Fetches trust settings for given certificate. 
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsHandler::GetTrustSettingsL( const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTrustSettingsHandler::GetTrustSettingsL | Begin"));

    TPckgBuf<TWimCertInfoPckg> pckg;

    aMessage.ReadL( 0, pckg );

    __ASSERT_DEBUG( iCertInfo == NULL, User::Panic( KWimTrustSettPanic, KErrGeneral ) );
    iCertInfo = CWimCertInfo::InternalizeL( pckg() );

    iStatus = KRequestPending;
    iMessage = aMessage;
    iPhase = EGetTrustSettings;
    SetActive();
    iWimTrustSettingsStore->GetTrustSettings( *iCertInfo,
                                              iTrusted,
                                              iApplications, 
                                              iStatus );
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsHandler::SetApplicabilityL
// Set applicability for given certificate. New certificate entry is set if
// one not found from database (trust flag is set to EFalse).
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsHandler::SetApplicabilityL( const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTrustSettingsHandler::SetApplicabilityL | Begin"));

    TPckgBuf<TWimCertInfoPckg> pckg;
    aMessage.ReadL( 0, pckg );

    __ASSERT_DEBUG( iCertInfo == NULL, User::Panic( KWimTrustSettPanic, KErrGeneral ) );
    iCertInfo = CWimCertInfo::InternalizeL( pckg() );

    RArray<TUid> applications;
    CleanupClosePushL( applications );

    TPckgBuf<TTrustSettings> trustSettingsPckg;

    aMessage.ReadL( 1, trustSettingsPckg );

    TInt applicationCount = trustSettingsPckg().iApplicationCount;

    for ( TInt i = 0; i < applicationCount; i++ )
        {
        TUid uid;
        uid.iUid = trustSettingsPckg().iUids[i];
        applications.Append( uid );
        }

    iStatus = KRequestPending;
    iMessage = aMessage;
    iPhase = ESetApplicability;
    SetActive();
    iWimTrustSettingsStore->SetApplicability( *iCertInfo,
                                              applications,
                                              iStatus );

    CleanupStack::PopAndDestroy( &applications );
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsHandler::SetTrustL
// Set trust flag for given certificate. New certificate entry is set if
// one not found from database.
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsHandler::SetTrustL( const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTrustSettingsHandler::SetTrustL | Begin"));

    TPckgBuf<TWimCertInfoPckg> pckg;
    aMessage.ReadL( 0, pckg );

    __ASSERT_DEBUG( iCertInfo == NULL, User::Panic( KWimTrustSettPanic, KErrGeneral ) );
    iCertInfo = CWimCertInfo::InternalizeL( pckg() );

    TPckgBuf<TTrustSettings> trustSettingsPckg;

    aMessage.ReadL( 1, trustSettingsPckg );

    TBool trusted = trustSettingsPckg().iTrusted;

    iStatus = KRequestPending;
    iMessage = aMessage;
    iPhase = ESetTrust;

    SetActive();

    iWimTrustSettingsStore->SetTrust( *iCertInfo,
                                      trusted,
                                      iStatus );
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsHandler::SetDefaultTrustSettingsL
// Set default trust settings for given certificate. If certificate not
// found from database new entry is inserted.
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsHandler::SetDefaultTrustSettingsL( 
    const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTrustSettingsHandler::SetDefaultTrustSettingsL | Begin"));

    TPckgBuf<TWimCertInfoPckg> pckg;
    aMessage.ReadL( 0, pckg );
   
    TBool addApps = aMessage.Int1();

    __ASSERT_DEBUG( iCertInfo == NULL, User::Panic( KWimTrustSettPanic, KErrGeneral ) );
    iCertInfo = CWimCertInfo::InternalizeL( pckg() );

    iStatus = KRequestPending;
    iMessage = aMessage;
    iPhase = ESetDefaultTrustSettings;
    SetActive();
    iWimTrustSettingsStore->SetDefaultTrustSettings( *iCertInfo,
                                                     addApps, 
                                                     iStatus );
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsHandler::RemoveTrustSettingsL
// Remove trust settings of given certificate. If certificate
// is not found, return with status.Int() = KErrNotFound
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsHandler::RemoveTrustSettingsL( const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTrustSettingsHandler::RemoveTrustSettingsL | Begin"));

    TPckgBuf<TWimCertInfoPckg> pckg;
    aMessage.ReadL( 0, pckg );

    __ASSERT_DEBUG( iCertInfo == NULL, User::Panic( KWimTrustSettPanic, KErrGeneral ) );
    iCertInfo = CWimCertInfo::InternalizeL( pckg() );

    iStatus = KRequestPending;
    iMessage = aMessage;
    iPhase = ERemoveTrustSettings;
    SetActive();
    iWimTrustSettingsStore->RemoveTrustSettings( *iCertInfo,
                                                 iStatus );
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsHandler::CancelDoing
// Cancel ongoing TrustSettingsStore operation
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsHandler::CancelDoing()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTrustSettingsHandler::CancelDoing | Begin"));

    iWimTrustSettingsStore->CancelDoing();
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsHandler::RunL
// Complete asynchronous call
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsHandler::RunL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTrustSettingsHandler::RunL | Begin"));

    delete iCertInfo;
    iCertInfo = NULL;
    
    switch ( iPhase )
        {
        case EGetTrustSettings:
            {
            TInt applicationCount = iApplications.Count();

            // Check that there is no more apps than allocated in array for
            // If there is more applications than there is space in array
            // complete message and return
            if ( applicationCount > KMaxApplicationCount )
                {
                iMessage.Complete( KErrOverflow );
                iApplications.Close();
                return;
                }

            TPckgBuf<TTrustSettings> trustSettings;
            iMessage.ReadL( 1, trustSettings );
            trustSettings().iTrusted = iTrusted;
            trustSettings().iApplicationCount = iApplications.Count();

            for ( TInt i = 0; i < iApplications.Count(); i++ )
                {
                trustSettings().iUids[i] = iApplications[i].iUid;
                }

            iApplications.Close();
            iMessage.WriteL( 1, trustSettings );
            break;
            }

        case ESetDefaultTrustSettings:  // Flow through
        case ESetApplicability:         // Flow through
        case ESetTrust:                 // Flow through
        case ERemoveTrustSettings:
            {
            break;
            }

        default:
            {
            break;
            }
        }

    iMessage.Complete( iStatus.Int() );
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsHandler::DoCancel
// 
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsHandler::DoCancel()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTrustSettingsHandler::DoCancel | Begin"));
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsHandler::RunError
// Handle trapped leave in RunL. Complete message with leave error code and
// close open resources that need be freed.
// -----------------------------------------------------------------------------
//
TInt CWimTrustSettingsHandler::RunError( TInt aError )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTrustSettingsHandler::RunError | Begin"));

    iMessage.Complete( aError );
    iApplications.Close();
    return KErrNone;
    }

//  End of File
