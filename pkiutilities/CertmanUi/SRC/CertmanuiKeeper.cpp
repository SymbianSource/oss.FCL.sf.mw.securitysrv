/*
* Copyright (c) 2003-2009 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Implementation of class CCertManUIKeeper
*                Creates and destroys all the views CertManUI uses.
*                Handles changing CertManUI views.
*
*/


// INCLUDE FILES
#include <aknnotewrappers.h>            // for warning & information notes
#include <unifiedcertstore.h>
#include <unifiedkeystore.h>
#include <mctwritablecertstore.h>
#include <X509CertNameParser.h>
#include <certmanui.rsg>
#include <data_caging_path_literals.hrh>
#include "CertmanuiSyncWrapper.h"
#include "CertmanuiCertificateHelper.h"
#include "Certmanuidialogs.h"
#include "CertmanuiKeeper.h"
#include "CertmanuiviewTrust.h"
#include "CertmanuiviewPersonal.h"
#include "CertmanuicontainerAuthority.h"
#include "CertmanuiCommon.h"
#include "CertManUILogger.h"
#include "securityuisvariant.hrh"

// CONSTANTS
_LIT( KNameSeparator, " " );
_LIT( KResourceFile, "z:CertManUi.rsc" );


// ============================= GLOBAL FUNCTIONS ==============================

//---------------------------------------------------------------
// CloseWaitDialogOnLeave( TAny* aPtr )
// Closes wait dialog when leave happens.
//---------------------------------------------------------------
void CloseWaitDialogOnLeave( TAny* aPtr )
    {
    CCertManUIKeeper* keeper = reinterpret_cast< CCertManUIKeeper* >( aPtr );
    if( keeper )
        {
        TRAP_IGNORE( keeper->CloseWaitDialogL() );
        }
    }

//---------------------------------------------------------------
// CleanupCloseWaitDialogPushL( CCertManUIKeeper& aKeeper )
// Helper function for to close wait dialog if leave happens.
//---------------------------------------------------------------
void CleanupCloseWaitDialogPushL( CCertManUIKeeper& aKeeper )
    {
    CleanupStack::PushL( TCleanupItem( CloseWaitDialogOnLeave, &aKeeper ) );
    }


// ============================= LOCAL FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// TInt CompareCALabels( const CCTCertInfo& cert1, const CCTCertInfo& cert2 )
// Compares two labels.
// -----------------------------------------------------------------------------
//
TInt CompareCALabels(
    const CCertManUICertData& cert1,
    const CCertManUICertData& cert2 )
    {
    return ( (cert1.iCAEntryLabel->Des()).
        CompareF( cert2.iCAEntryLabel->Des() ) );
    }

// -----------------------------------------------------------------------------
// TInt CompareUserLabels( const CCTCertInfo& cert1, const CCTCertInfo& cert2 )
// Compares two labels.
// -----------------------------------------------------------------------------
//
TInt CompareUserLabels(
    const CCertManUICertData& cert1,
    const CCertManUICertData& cert2 )
    {
    return ( (cert1.iUserEntryLabel->Des()).
        CompareF( cert2.iUserEntryLabel->Des() ) );
    }

// -----------------------------------------------------------------------------
// TInt ComparePeerLabels( const CCTCertInfo& cert1, const CCTCertInfo& cert2 )
// Compares two labels.
// -----------------------------------------------------------------------------
//
TInt ComparePeerLabels(
    const CCertManUICertData& cert1,
    const CCertManUICertData& cert2 )
    {
    return ( (cert1.iPeerEntryLabel->Des()).
        CompareF( cert2.iPeerEntryLabel->Des() ) );
    }

// -----------------------------------------------------------------------------
// TInt CompareDeviceLabels( const CCTCertInfo& cert1, const CCTCertInfo& cert2 )
// Compares two labels.
// -----------------------------------------------------------------------------
//
TInt CompareDeviceLabels(
    const CCertManUICertData& cert1,
    const CCertManUICertData& cert2 )
    {
    return ( (cert1.iDeviceEntryLabel->Des()).
        CompareF( cert2.iDeviceEntryLabel->Des() ) );
    }

// ================= HELPER CLASS ===========================

// Constructor.
CCertManUICertData::CCertManUICertData()
    {
    }

// Destructor.
CCertManUICertData::~CCertManUICertData()
    {
    delete iCAEntryLabel;
    delete iUserEntryLabel;
    delete iPeerEntryLabel;
    delete iDeviceEntryLabel;
    }

// Releases Certificate entries.
void CCertManUICertData::Release()
    {
    if ( iCAEntry )
        {
        iCAEntry->Release();
        }

    if ( iUserEntry )
        {
        iUserEntry->Release();
        }

    if ( iPeerEntry )
        {
        iPeerEntry->Release();
        }

    if ( iDeviceEntry )
        {
        iDeviceEntry->Release();
        }

    delete this;
    }

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CCertManUIKeeper::CCertManUIKeeper()
// Constructor with parent
// ---------------------------------------------------------
//
CCertManUIKeeper::CCertManUIKeeper(): iResourceLoader( *iCoeEnv )
    {
    CERTMANUILOGGER_CREATE; //lint !e960
    CERTMANUILOGGER_WRITE_TIMESTAMP( "CCertManUIKeeper::CCertManUIKeeper" );
    }

// ---------------------------------------------------------
// CCertManUIKeeper::~CCertManUIKeeper()
// Destructor
// ---------------------------------------------------------
//
CCertManUIKeeper::~CCertManUIKeeper()
    {
    CERTMANUILOGGER_WRITE_TIMESTAMP( "CCertManUIKeeper::~CCertManUIKeeper" );
    if ( iWrapper )
        {
        iWrapper->Cancel();
        }
    delete iFilter;
    delete iKeyFilter;
    delete iStore;
    delete iKeyStore;
    delete iWrapper;
    delete iCertificateHelper;
    delete iCertmanUIDialog;

    iCAEntries.Close();
    iCALabelEntries.Close();

    iUserEntries.Close();
    iUserLabelEntries.Close();

    iPeerEntries.Close();
    iPeerLabelEntries.Close();

    iDeviceEntries.Close();
    iDeviceLabelEntries.Close();

    iKeyEntries.Close();

    iRfs.Close();

    CERTMANUILOGGER_DELETE; //lint !e960
    }

// ---------------------------------------------------------
// CCertManUIKeeper* CCertManUIKeeper::NewL(const TRect& aRect, TUid aViewId)
// ---------------------------------------------------------
//

CCertManUIKeeper* CCertManUIKeeper::NewL(
    const TRect& aRect, TUid aViewId )
    {
    CCertManUIKeeper* self = new ( ELeave ) CCertManUIKeeper;
    CleanupStack::PushL( self );
    self->ConstructL( aRect, aViewId );
    CleanupStack::Pop();
    return self;
    }

// ---------------------------------------------------------
// CCertManUIKeeper* CCertManUIKeeper::NewLC(
//      const TRect& aRect, TUid aViewId)
// ---------------------------------------------------------
//
CCertManUIKeeper* CCertManUIKeeper::NewLC(
    const TRect& aRect, TUid aViewId )
    {
    CCertManUIKeeper* self = new ( ELeave ) CCertManUIKeeper;
    CleanupStack::PushL( self );
    self->ConstructL( aRect, aViewId );
    return self;
    }


// ---------------------------------------------------------
// CCertManUIKeeper::ConstructL(
 //     const TRect& aRect, TUid aViewId
//
// ---------------------------------------------------------
//
void CCertManUIKeeper::ConstructL( const TRect& aRect, TUid aViewId )
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUIKeeper::ConstructL" );

    AddResourceFileL();

    InitializeL( aRect, aViewId );

    CERTMANUILOGGER_LEAVEFN( "CCertManUIKeeper::ConstructL" );
    }


// ---------------------------------------------------------
// CCertManUIKeeper::InitializeL(
 //     const TRect& aRect, TUid aViewId
//
// ---------------------------------------------------------
//
void CCertManUIKeeper::InitializeL( const TRect& /*aRect*/, TUid /*aViewId*/)
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUIKeeper::InitializeL" );
    iCertmanUIDialog = new ( ELeave ) CCertManUIWaitDialog();

    // Ensure that file server session is succesfully created
    User::LeaveIfError( iRfs.Connect() );

    iCertsDatOK = EFalse;

    // Is this the place where these errors can be handled?
    // iCertsDatOK is set when cert entries are refreshed.
    // Create a R/W interface. Symbian has promised to support
    // up to two simultaneous R/W interfaces.
    TRAPD ( error, iStore = CUnifiedCertStore::NewL( iRfs, ETrue ) );
    if ( error != KErrNone )
        {
        if ( error == KErrNotSupported
           || error == KErrNotReady
           || error == KErrArgument
           || error == KErrNotFound )
            {
            // Something may be wrong with the databases
            iCertsDatOK = EFalse;
            ShowErrorNoteL( error );
            User::Leave( KErrNone );
            }
        else
            {
            User::Leave( error );
            }
        }
    else
        {
        // cacerts.dat file is ok
        iCertsDatOK = ETrue;
        }

    iCertificateHelper = new ( ELeave ) CCertManUICertificateHelper( *this );

    iWrapper = CCertManUISyncWrapper::NewL();
    error = iWrapper->InitStoreL( iStore );

    if ( error )
        {
        LOG_WRITE_FORMAT("InitStoreL returned %i", error);
        }


    iFilter = CCertAttributeFilter::NewL();

    TRAP( error, iKeyStore = CUnifiedKeyStore::NewL( iRfs ) );
    if ( error != KErrNone )
        {
        if (error == KErrNotSupported
            || error == KErrNotReady
            || error == KErrArgument
            || error == KErrNotFound)
            {
            // Something is wrong with key stores
            LOG_WRITE( "Keystore corrupt" );

            ShowErrorNoteL( error );
            User::Leave( KErrNone );
            }
        else
            {
            User::Leave( error );
            }
          }

    LOG_WRITE("Init keystore");
    iWrapper->InitStoreL( iKeyStore );
    iKeyEntries.Close();
    LOG_WRITE_FORMAT("Keys available : %i", iKeyEntries.Count());

    if ( iCertsDatOK ) // == ETrue
        {
        iStartup = ETrue;
        }


    CERTMANUILOGGER_LEAVEFN( "CCertManUIKeeper::InitializeL" );
    }

// ---------------------------------------------------------
// CCertManUIKeeper::AddResourceFileL()
//
// ---------------------------------------------------------
//
void CCertManUIKeeper::AddResourceFileL()
    {
    TFileName fileName;
    TParse parse;
    parse.Set( KResourceFile, &KDC_RESOURCE_FILES_DIR, NULL );

    fileName = parse.FullName();
    iResourceLoader.OpenL( fileName );
    }

// ---------------------------------------------------------
// CCertManUIKeeper::FocusChanged(TDrawNow aDrawNow)
// Added here because FocusChanged need to be included
// in every control derived from CCoeControl that can have listbox
// ---------------------------------------------------------
//
void CCertManUIKeeper::FocusChanged(TDrawNow aDrawNow)
    {
    if (iListBox)
        {
        iListBox->SetFocus( IsFocused(), aDrawNow );
        }
     }


// ----------------------------------------------------
// CCertManUIKeeper::ActivateTitleL(TInt aCurrentTitle)
// Activates the WimTitle and CertManUI Trust Settings title
// ----------------------------------------------------
//
void CCertManUIKeeper::ActivateTitleL( TInt aCurrentTitle )
    {
    CERTMANUILOGGER_ENTERFN("CCertManUIKeeper::ActivateTitleL" );

    CEikStatusPane* sp = STATIC_CAST(
        CAknAppUi*, iEikonEnv->EikAppUi())->StatusPane();
    // Fetch pointer to the default title pane control
    CAknTitlePane* title = STATIC_CAST(
        CAknTitlePane*, sp->ControlL( TUid::Uid( EEikStatusPaneUidTitle ) ));
    TResourceReader reader;

    if ( aCurrentTitle == KViewTitleCertManUI )
        {
        iCoeEnv->CreateResourceReaderLC( reader, R_CERTMANUI_VIEW_TITLE );
        }
    else if ( aCurrentTitle == KViewTitleCertManUIAuthority )
        {
        iCoeEnv->CreateResourceReaderLC( reader, R_CERTMANUI_VIEW_TITLE_AUTHORITY );
        }
    else if ( aCurrentTitle == KViewTitleCertManUITrustedSite )
        {
        iCoeEnv->CreateResourceReaderLC( reader, R_CERTMANUI_VIEW_TITLE_TRUSTED_SITE );
        }
    else if ( aCurrentTitle == KViewTitleCertManUIPersonal )
        {
        iCoeEnv->CreateResourceReaderLC( reader, R_CERTMANUI_VIEW_TITLE_PERSONAL );
        }
    else if ( aCurrentTitle == KViewTitleCertManUIDevice )
        {
        iCoeEnv->CreateResourceReaderLC( reader, R_CERTMANUI_VIEW_TITLE_DEVICE );
        }
    else  // for lint
        {
        }

    title->SetFromResourceL( reader );
    CleanupStack::PopAndDestroy();  // reader

    CERTMANUILOGGER_LEAVEFN( "CCertManUIKeeper::ActivateTitleL" );
    }

// ---------------------------------------------------------
// CCertManUIKeeper::HandleControlEventL(
//      CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/)
// Cannot be changed to non-leaving function.
// L-function is required by the class definition, even if empty.
// ---------------------------------------------------------
//
void CCertManUIKeeper::HandleControlEventL(
    CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/)
    {
    }

// ---------------------------------------------------------
// CCertManUIKeeper::OfferKeyEventL(
//      const TKeyEvent& /*aKeyEvent*/, TEventCode /*aType*/)
// Called when a key is pressed.
// Cannot be changed to non-leaving function.
// L-function is required by the class definition, even if empty.
// ---------------------------------------------------------
//
TKeyResponse CCertManUIKeeper::OfferKeyEventL(
    const TKeyEvent& /*aKeyEvent*/, TEventCode /*aType*/)
    {
    // Listbox takes all event even if it doesn't use them
    return EKeyWasNotConsumed;
    }

// ---------------------------------------------------------
// CCertManUIKeeper::CertManager()
// ---------------------------------------------------------
//
CUnifiedCertStore*& CCertManUIKeeper::CertManager()
    {
    return iStore;
    }

// ---------------------------------------------------------
// CCertManUIKeeper::KeyManager()
// ---------------------------------------------------------
//
CUnifiedKeyStore*& CCertManUIKeeper::KeyManager()
    {
    return iKeyStore;
    }

// ---------------------------------------------------------
// CCertManUIKeeper::RefreshCAEntriesL()
// Checks that cacerts.dat file is not corrupted and
// returns CA certificates from it
// ---------------------------------------------------------
//
void CCertManUIKeeper::RefreshCAEntriesL()
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUIKeeper::RefreshCAEntriesL" );
    iFilter->SetOwnerType( ECACertificate );
    iCAEntries.Close();
    CUnifiedCertStore*& store = CertManager();
    iWrapper->ListL( store, &iCAEntries, *iFilter );

    iCALabelEntries.Close();
    LOG_WRITE_FORMAT("CA entries found: %i", iCAEntries.Count());
    for ( TInt ii = 0; ii < iCAEntries.Count(); ii++ )
        {
        switch ( iCAEntries[ii]->CertificateFormat() )
            {
            case EX509Certificate:
                {
                CCertManUICertData* data = new ( ELeave ) CCertManUICertData();
                data->iCAEntry = CCTCertInfo::NewL( *iCAEntries[ii] );
                CleanupStack::PushL( data );
                HBufC* pri = NULL;
                HBufC* sec = NULL;

                CCertificate* cert;

                TRAPD(err, iWrapper->GetCertificateL( CertManager(), *iCAEntries[ii], cert ));
                if (( err == KErrArgument ) || ( err == KErrCorrupt ))
                    {
                    // This indicates that certificate is not according to X509
                    // specification or it is corrupted. Ignore certificate
                    if(data->iCAEntry)
                       {
                       data->iCAEntry->Release();
                       }
                    CleanupStack::PopAndDestroy( data ); // data
                    continue;
                    }
                else
                    {
                    User::LeaveIfError( err );
                    }

                CleanupStack::PushL( cert );

                X509CertNameParser::PrimaryAndSecondaryNameL
                    ( *((CX509Certificate*)cert), pri, sec, iCAEntries[ii]->Label());

                CleanupStack::PushL( pri );
                CleanupStack::PushL( sec );

                TInt lenght = 0;

                lenght += iCAEntries[ii]->Label().Length();

                if ( pri )
                    {
                    lenght += pri->Length();
                    }
                if ( sec && !(iCAEntries[ii]->Label().Length()) )
                    {
                    lenght += sec->Length();
                    }
                lenght += KNameSeparator.iTypeLength;

                HBufC* label = HBufC::NewL( lenght );
                label->Des().Append( iCAEntries[ii]->Label() );

                if ( pri )
                    {
                    label->Des().Append( KNameSeparator );
                    label->Des().Append( pri->Des() );
                    }
                if ( sec && !(iCAEntries[ii]->Label().Length()) )
                    {
                    label->Des().Append( sec->Des() );
                    }

                data->iCAEntryLabel = label;
                iCALabelEntries.Append( data );

                CleanupStack::PopAndDestroy( 3 ); // cert, pri, sec
                CleanupStack::Pop( data ); // data
                break;
                } // EX509Certificate

            case EWTLSCertificate:
                // Do nothing for wtls certificates
                break;
            default:
                {
                User::Leave( KErrNotSupported );
                }
            }   // switch

        }   // for

    // Sort the certificates.
    TLinearOrder<CCertManUICertData> order( CompareCALabels );
    iCALabelEntries.Sort( order );

    CERTMANUILOGGER_LEAVEFN("CCertManUIKeeper::RefreshCAEntriesL" );
    }

// ---------------------------------------------------------
// CCertManUIKeeper::RefreshUserCertEntriesL()
// Checks that cacerts.dat file is not corrupted and
// returns User certificates from it
// ---------------------------------------------------------
//
void CCertManUIKeeper::RefreshUserCertEntriesL()
    {
    CERTMANUILOGGER_ENTERFN(
        "CCertManUIKeeper::RefreshUserCertEntriesL" );

    iFilter->SetOwnerType( EUserCertificate );
    iUserEntries.Close();
    CUnifiedCertStore*& store = CertManager();
    iWrapper->ListL( store, &iUserEntries, *iFilter );

    iUserLabelEntries.Close();
    LOG_WRITE_FORMAT(" Userentries found %i", iUserEntries.Count());
    for ( TInt i = 0; i < iUserEntries.Count(); i++ )
        {

       if ( iUserEntries[i]->Handle().iTokenHandle.iTokenTypeUid == KCMDeviceCertStoreTokenUid )
            {
            // User certificates from DeviceCertStore are not
            // shown in Personal certificate view. Ignore this certificate.
            break;
            }

        switch ( iUserEntries[i]->CertificateFormat() )
            {
            case EX509Certificate:
                {
                LOG_WRITE_FORMAT(" User X509 certificate, index %i", i);
                CCertManUICertData* data = new (ELeave) CCertManUICertData();
                data->iUserEntry = CCTCertInfo::NewL( *iUserEntries[i] );
                CleanupStack::PushL( data );
                HBufC* pri = NULL;
                HBufC* sec = NULL;

                CCertificate* cert = NULL;
                TRAPD(error, iWrapper->GetCertificateL( CertManager(), *iUserEntries[i], cert ));

                if( error == KErrArgument)
                    {
                    LOG_WRITE("Corrupted user certificate detected");
                    CleanupStack::PopAndDestroy( data );
                    break;
                    }
                else if ( error != KErrNone )
                    {
                    LOG_WRITE_FORMAT( " GetCertificateL leaves, error code %i", error );
                    User::Leave( error );
                    }
                else
                    {
                    CleanupStack::PushL( cert );
                    }

                X509CertNameParser::PrimaryAndSecondaryNameL
                    ( *((CX509Certificate*)cert), pri, sec, iUserEntries[i]->Label());

                CleanupStack::PushL( pri );
                CleanupStack::PushL( sec );

                TInt lenght = 0;

                lenght += iUserEntries[i]->Label().Length();

                if ( pri )
                    {
                    lenght += pri->Length();
                    }
                if ( sec && !(iUserEntries[i]->Label().Length()) )
                    {
                    lenght += sec->Length();
                    }
                lenght += KNameSeparator.iTypeLength;

                HBufC* label = HBufC::NewL( lenght );
                label->Des().Append( iUserEntries[i]->Label() );

                if ( pri )
                    {
                    label->Des().Append( KNameSeparator );
                    label->Des().Append( pri->Des() );
                    }
                if ( sec && !(iUserEntries[i]->Label().Length()) )
                    {
                    label->Des().Append( sec->Des() );
                    }

                data->iUserEntryLabel = label;
                iUserLabelEntries.Append( data );

                CleanupStack::PopAndDestroy( 3 ); // cert, pri, sec
                CleanupStack::Pop( data ); // data
                break;
                } // EX509Certificate

            case EX509CertificateUrl:
                {
                LOG_WRITE_FORMAT(" User X509URL certificate, index %i", i);
                CCertManUICertData* urlCertData = new (ELeave) CCertManUICertData();
                urlCertData->iUserEntry = CCTCertInfo::NewL( *iUserEntries[i] );
                CleanupStack::PushL( urlCertData );

                CCertificate* cert = NULL;
                TRAPD(error, iWrapper->GetCertificateL( CertManager(), *iUserEntries[i], cert ));
                if( error == KErrArgument )
                    {
                    LOG_WRITE("Corrupted user certificate detected");
                    CleanupStack::PopAndDestroy( urlCertData ); // urlCertData
                    break;
                    }
                else if ( error != KErrNone )
                    {
                    LOG_WRITE_FORMAT( " GetCertificateL leaves, error code %i", error );
                    User::Leave(error);
                    }
                else
                    {
                    CleanupStack::PushL( cert );
                    }


                HBufC* UrlLabel = HBufC::NewL( 20 );
                UrlLabel->Des().Append( _L("URL UserCert"));

                urlCertData->iUserEntryLabel = UrlLabel;
                iUserLabelEntries.Append( urlCertData );

                CleanupStack::PopAndDestroy( cert ); // cert
                CleanupStack::Pop( urlCertData ); // data
                break;
                }
            default:
                {
                LOG_WRITE_FORMAT( "Unsupported certificate format 0x%x",
                                        iUserEntries[i]->CertificateFormat() );
                }
            }   // switch

        }   // for

    // Sort the certificates.
    TLinearOrder<CCertManUICertData> order( CompareUserLabels );
    iUserLabelEntries.Sort( order );

    CERTMANUILOGGER_LEAVEFN(
        "CCertManUIKeeper::RefreshUserCertEntriesL" );
    }

// ---------------------------------------------------------
// CCertManUIKeeper::RefreshPeerCertEntriesL()
// Checks that Trusted site certificate store is
// not corrupted and returns Peer certificates from it
// ---------------------------------------------------------
//
void CCertManUIKeeper::RefreshPeerCertEntriesL()
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUIKeeper::RefreshPeerEntriesL" );



    iFilter->SetOwnerType( EPeerCertificate );
    iPeerEntries.Close();
    CUnifiedCertStore*& store = CertManager();

    iWrapper->ListL( store, &iPeerEntries, *iFilter, KCMTrustedServerTokenUid );

    iPeerLabelEntries.Close();
    LOG_WRITE_FORMAT("Peer entries found: %i", iPeerEntries.Count());
    for ( TInt ii = 0; ii < iPeerEntries.Count(); ii++ )
        {
        switch ( iPeerEntries[ii]->CertificateFormat() )
            {
            case EX509Certificate:
                {
                CCertManUICertData* data = new (ELeave) CCertManUICertData();
                data->iPeerEntry = CCTCertInfo::NewL( *iPeerEntries[ii] );
                CleanupStack::PushL( data );

                CCertificate* cert;

                TRAPD( err, iWrapper->GetCertificateL( CertManager(), *iPeerEntries[ii], cert, KCMTrustedServerTokenUid ));
                if (( err == KErrArgument ) || ( err == KErrCorrupt ))
                    {
                    // This indicates that certificate is not according to X509
                    // specification or it is corrupted. Ignore certificate
                    if( data->iPeerEntry )
                       {
                       data->iPeerEntry->Release();
                       }
                    CleanupStack::PopAndDestroy( data ); // data
                    continue;
                    }
                else
                    {
                    User::LeaveIfError( err );
                    }

                CleanupStack::PushL( cert );

                TInt lenght = 0;

                lenght += iPeerEntries[ii]->Label().Length();

                HBufC* label = HBufC::NewL( lenght );
                label->Des().Append( iPeerEntries[ii]->Label() );

                data->iPeerEntryLabel = label;
                iPeerLabelEntries.Append( data );

                CleanupStack::PopAndDestroy( cert ); // cert
                CleanupStack::Pop( data ); // data
                break;
                } // EX509Certificate

            case EWTLSCertificate:
                // Do nothing for wtls certificates
                break;
            default:
                {
                User::Leave( KErrNotSupported );
                }
            }   // switch

        }   // for

    // Sort the certificates.
    TLinearOrder<CCertManUICertData> order( ComparePeerLabels );
    iPeerLabelEntries.Sort( order );

    CERTMANUILOGGER_LEAVEFN("CCertManUIKeeper::RefreshPeerEntriesL" );
    }


// ---------------------------------------------------------
// CCertManUIKeeper::RefreshDeviceCertEntriesL()
// returns device certificates
// ---------------------------------------------------------
//
void CCertManUIKeeper::RefreshDeviceCertEntriesL()
    {
    CERTMANUILOGGER_ENTERFN(
        "CCertManUIKeeper::RefreshUserCertEntriesL" );

    iFilter->SetOwnerType( EUserCertificate );
    iDeviceEntries.Close();
    CUnifiedCertStore*& store = CertManager();

    iWrapper->ListL( store, &iDeviceEntries, *iFilter, KCMDeviceCertStoreTokenUid );

    iDeviceLabelEntries.Close();
    LOG_WRITE_FORMAT(" Device entries found %i", iDeviceEntries.Count());
    for ( TInt i = 0; i < iDeviceEntries.Count(); i++ )
        {
        switch ( iDeviceEntries[i]->CertificateFormat() )
            {
            case EX509Certificate:
                {
                LOG_WRITE_FORMAT(" User X509 certificate, index %i", i);
                CCertManUICertData* data = new (ELeave) CCertManUICertData();
                data->iDeviceEntry = CCTCertInfo::NewL( *iDeviceEntries[i] );
                CleanupStack::PushL( data );
                HBufC* pri = NULL;
                HBufC* sec = NULL;

                CCertificate* cert = NULL;
                TRAPD(error, iWrapper->GetCertificateL( CertManager(), *iDeviceEntries[i], cert, KCMDeviceCertStoreTokenUid ));

                if( error == KErrArgument)
                    {
                    LOG_WRITE("Corrupted Device certificate detected");
                    CleanupStack::PopAndDestroy( data );
                    break;
                    }
                else if ( error != KErrNone )
                    {
                    LOG_WRITE_FORMAT(" GetCertificateL leaves, error code %i", error);
                    User::Leave( error );
                    }
                else
                    {
                    CleanupStack::PushL( cert );
                    }

                X509CertNameParser::PrimaryAndSecondaryNameL
                    ( *((CX509Certificate*)cert), pri, sec, iDeviceEntries[i]->Label());

                CleanupStack::PushL( pri );
                CleanupStack::PushL( sec );

                TInt lenght = 0;

                lenght += iDeviceEntries[i]->Label().Length();

                if ( pri )
                    {
                    lenght += pri->Length();
                    }
                if ( sec && !(iDeviceEntries[i]->Label().Length()) )
                    {
                    lenght += sec->Length();
                    }
                lenght += KNameSeparator.iTypeLength;

                HBufC* label = HBufC::NewL( lenght );
                label->Des().Append( iDeviceEntries[i]->Label() );

                if ( pri )
                    {
                    label->Des().Append( KNameSeparator );
                    label->Des().Append( pri->Des() );
                    }
                if ( sec && !(iDeviceEntries[i]->Label().Length()) )
                    {
                    label->Des().Append( sec->Des() );
                    }

                data->iDeviceEntryLabel = label;
                iDeviceLabelEntries.Append( data );

                CleanupStack::PopAndDestroy( 3 ); // cert, pri, sec
                CleanupStack::Pop( data ); // data
                break;
                } // EX509Certificate

            case EX509CertificateUrl:
                {
                LOG_WRITE_FORMAT(" User X509URL certificate, index %i", i);
                CCertManUICertData* urlCertData = new (ELeave) CCertManUICertData();
                urlCertData->iDeviceEntry = CCTCertInfo::NewL( *iDeviceEntries[i] );
                CleanupStack::PushL( urlCertData );

                CCertificate* cert = NULL;
                TRAPD(error, iWrapper->GetCertificateL( CertManager(), *iDeviceEntries[i], cert, KCMDeviceCertStoreTokenUid ));
                if( error == KErrArgument)
                    {
                    LOG_WRITE("Corrupted user certificate detected");
                    CleanupStack::PopAndDestroy( urlCertData ); // urlCertData
                    break;
                    }
                else if ( error != KErrNone )
                    {
                    LOG_WRITE_FORMAT(" GetCertificateL leaves, error code %i", error);
                    User::Leave(error);
                    }
                else
                    {
                    CleanupStack::PushL( cert );
                    }


                HBufC* UrlLabel = HBufC::NewL( 20 );
                UrlLabel->Des().Append( _L("URL UserCert"));

                urlCertData->iDeviceEntryLabel = UrlLabel;
                iDeviceLabelEntries.Append( urlCertData );

                CleanupStack::PopAndDestroy( cert ); // cert
                CleanupStack::Pop( urlCertData ); // data
                break;
                }
            default:
                {
                LOG_WRITE_FORMAT( "Unsupported certificate format 0x%x",
                                        iDeviceEntries[i]->CertificateFormat() );
                }
            }   // switch

        }   // for

    // Sort the certificates.
    TLinearOrder<CCertManUICertData> order( CompareDeviceLabels );
    iDeviceLabelEntries.Sort( order );

    CERTMANUILOGGER_LEAVEFN(
        "CCertManUIKeeper::RefreshDeviceCertEntriesL" );
    }



// ---------------------------------------------------------
// CCertManUIKeeper::ShowErrorNoteL( TInt aError )
// Shows error note
// ---------------------------------------------------------
//
void CCertManUIKeeper::ShowErrorNoteL( TInt aError )
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUIKeeper::ShowErrorNoteL" );
    // Make sure wait dialog isn't running
    CloseWaitDialogL();
    // Display error note
    TInt resourceid(0);

    switch ( aError )
        {
        case KErrCorrupt:
            {
            resourceid = R_ERROR_NOTE_TEXT_CACERTS_DB_CORRUPTED;
            break;
            }
        case KErrCancel:
            {
            // Show nothing
            break;
            }
         default:
            {
            resourceid = R_ERROR_NOTE_TEXT_INTERNAL_ERROR;
            break;
            }
        }

    if ( resourceid != 0 )
        {
        HBufC* string = StringLoader::LoadLC( resourceid );
        CAknErrorNote* note = new ( ELeave ) CAknErrorNote( ETrue );
        note->ExecuteLD( *string );
        CleanupStack::PopAndDestroy();  // string
        }

    CERTMANUILOGGER_LEAVEFN( "CCertManUIKeeper::ShowErrorNoteL" );
    }

// ---------------------------------------------------------
// CCertManUIKeeper::StartWaitDialogL( TInt aDialogSelector )
// Displays wait dialog
// ---------------------------------------------------------
//
void CCertManUIKeeper::StartWaitDialogL( TInt aDialogSelector )
    {
    iCertmanUIDialog->StartWaitDialogL( aDialogSelector );
    }

// ---------------------------------------------------------
// CCertManUIKeeper::CloseWaitDialogL( TInt aDialogSelector )
// Closes wait dialog
// ---------------------------------------------------------
//
void CCertManUIKeeper::CloseWaitDialogL()
    {
    iCertmanUIDialog->CloseWaitDialogL();
    }

//---------------------------------------------------------------
// CCertManUIKeeper::ReadLocalVariationL
// Reads local variation settings
//---------------------------------------------------------------
void CCertManUIKeeper::ReadLocalVariationL()
    {
    iLocalFeaturesMask |= KSecurityUIsNoNativeAppTrustSettingChange;
    }

//---------------------------------------------------------------
// CCertManUIKeeper::IsLocalFeatureSupported
// Check whether local variation setting is defined
//---------------------------------------------------------------
TBool CCertManUIKeeper::IsLocalFeatureSupported( const TInt aFeature ) const
  {
    return ( iLocalFeaturesMask & aFeature );
    }

// End of File
