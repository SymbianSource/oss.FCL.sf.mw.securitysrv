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
* Description:   Implementation of class CCertManUIKeeper
*                Creates and destroys all the views CertManUI uses.
*                Handles changing CertManUI views.
*
*/


// INCLUDE FILES
#include <unifiedcertstore.h>
#include <unifiedkeystore.h>
#include <mctwritablecertstore.h>
#include <X509CertNameParser.h>

#include <HbMessageBox>

#include <QErrorMessage>
#include <../../inc/cpsecplugins.h>
#include "cpcertdatacontainer.h"
#include "cpcertmanuisyncwrapper.h"

// CONSTANTS
_LIT( KNameSeparator, " " );

// ============================= LOCAL FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// TInt CompareCALabels( const CCTCertInfo& cert1, const CCTCertInfo& cert2 )
// Compares two labels.
// -----------------------------------------------------------------------------
//
TInt CompareCALabels(
    const CpCertManUICertData& cert1,
    const CpCertManUICertData& cert2 )
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
    const CpCertManUICertData& cert1,
    const CpCertManUICertData& cert2 )
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
    const CpCertManUICertData& cert1,
    const CpCertManUICertData& cert2 )
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
    const CpCertManUICertData& cert1,
    const CpCertManUICertData& cert2 )
    {
    return ( (cert1.iDeviceEntryLabel->Des()).
        CompareF( cert2.iDeviceEntryLabel->Des() ) );
    }


// ================= HELPER CLASS ===========================

// Constructor.
CpCertManUICertData::CpCertManUICertData()
    {
    }

// Destructor.
CpCertManUICertData::~CpCertManUICertData()
    {
    delete iCAEntryLabel;
    delete iUserEntryLabel;
    delete iPeerEntryLabel;
    delete iDeviceEntryLabel;
    }

// Releases Certificate entries.
void CpCertManUICertData::Release()
    {
    RDEBUG("0", 0);
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
CpCertDataContainer::CpCertDataContainer()
    {
    }

// ---------------------------------------------------------
// CCertManUIKeeper::~CCertManUIKeeper()
// Destructor
// ---------------------------------------------------------
//
CpCertDataContainer::~CpCertDataContainer()
    {
	iCALabelEntries.Close();
	iUserLabelEntries.Close();
	iPeerLabelEntries.Close();
	iDeviceLabelEntries.Close();
	
    if ( iWrapper )
        {
        iWrapper->Cancel();
        }
    delete iWrapper;

    delete iFilter;
    delete iKeyFilter;
    delete iStore;
    delete iKeyStore;

    iCAEntries.Close();
    iUserEntries.Close();
    iPeerEntries.Close();
    iDeviceEntries.Close();
    iKeyEntries.Close();
    
    iRfs.Close();
    }

// ---------------------------------------------------------
// CCertManUIKeeper* CCertManUIKeeper::NewL(const TRect& aRect, TUid aViewId)
// ---------------------------------------------------------
//

CpCertDataContainer* CpCertDataContainer::NewL()
    {
	CpCertDataContainer* self = new ( ELeave ) CpCertDataContainer;
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

// ---------------------------------------------------------
// CCertManUIKeeper* CCertManUIKeeper::NewLC(
//      const TRect& aRect, TUid aViewId)
// ---------------------------------------------------------
//
CpCertDataContainer* CpCertDataContainer::NewLC()
    {
	CpCertDataContainer* self = new ( ELeave ) CpCertDataContainer;
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }


// ---------------------------------------------------------
// CCertManUIKeeper::ConstructL(
 //     const TRect& aRect, TUid aViewId
//
// ---------------------------------------------------------
//
void CpCertDataContainer::ConstructL()
    {
    RDEBUG("0", 0);
    // Ensure that file server session is succesfully created
    User::LeaveIfError( iRfs.Connect() );

    TRAPD ( error, iStore = CUnifiedCertStore::NewL( iRfs, ETrue ) );
        
	if ( error == KErrNotSupported || error == KErrNotReady
	   || error == KErrArgument || error == KErrNotFound )
		{
		// Something may be wrong with the databases
		ShowErrorNoteL( error );
		}
        
	User::LeaveIfError( error );
    
    iWrapper = CpCertManUISyncWrapper::NewL();
    iWrapper->InitStoreL( iStore );

    iFilter = CCertAttributeFilter::NewL();

    TRAP( error, iKeyStore = CUnifiedKeyStore::NewL( iRfs ) );
    
	if (error == KErrNotSupported || error == KErrNotReady
		|| error == KErrArgument || error == KErrNotFound)
		{
		// Something is wrong with key stores
		ShowErrorNoteL( error );
		}
	
	User::LeaveIfError( error );
    
    iWrapper->InitStoreL( iKeyStore );
    iKeyEntries.Close();
    
    }


// ---------------------------------------------------------
// CCertManUIKeeper::CertManager()
// ---------------------------------------------------------
//
CUnifiedCertStore*& CpCertDataContainer::CertManager()
    {
    return iStore;
    }

// ---------------------------------------------------------
// CCertManUIKeeper::KeyManager()
// ---------------------------------------------------------
//
CUnifiedKeyStore*& CpCertDataContainer::KeyManager()
    {
    return iKeyStore;
    }

// ---------------------------------------------------------
// CCertManUIKeeper::RefreshCAEntriesL()
// Checks that cacerts.dat file is not corrupted and
// returns CA certificates from it
// ---------------------------------------------------------
//
void CpCertDataContainer::RefreshCAEntriesL()
    {
    RDEBUG("0", 0);
    iFilter->SetOwnerType( ECACertificate );
    iCAEntries.Close();
    CUnifiedCertStore*& store = CertManager();
    iWrapper->ListL( store, &iCAEntries, *iFilter );

    iCALabelEntries.Close();
    for ( TInt ii = 0; ii < iCAEntries.Count(); ii++ )
        {
        switch ( iCAEntries[ii]->CertificateFormat() )
            {
            case EX509Certificate:
                {
                CpCertManUICertData* data = new ( ELeave ) CpCertManUICertData();
                CleanupStack::PushL( data );
                data->iCAEntry = CCTCertInfo::NewL( *iCAEntries[ii] );
                
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

                CleanupStack::PopAndDestroy( 3, cert ); // pri, sec
                CleanupStack::Pop( data ); 
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
    TLinearOrder<CpCertManUICertData> order( CompareCALabels );
    iCALabelEntries.Sort( order );

    }

// ---------------------------------------------------------
// CCertManUIKeeper::RefreshPeerCertEntriesL()
// Checks that Trusted site certificate store is
// not corrupted and returns Peer certificates from it
// ---------------------------------------------------------
//
void CpCertDataContainer::RefreshPeerCertEntriesL()
    {
    RDEBUG("0", 0);
    iFilter->SetOwnerType( EPeerCertificate );
    iPeerEntries.Close();
    CUnifiedCertStore*& store = CertManager();

    iWrapper->ListL( store, &iPeerEntries, *iFilter, KCMTrustedServerTokenUid );

    iPeerLabelEntries.Close();
    
    for ( TInt ii = 0; ii < iPeerEntries.Count(); ii++ )
        {
        switch ( iPeerEntries[ii]->CertificateFormat() )
            {
            case EX509Certificate:
                {
                CpCertManUICertData* data = new (ELeave) CpCertManUICertData();
                CleanupStack::PushL( data );
                data->iPeerEntry = CCTCertInfo::NewL( *iPeerEntries[ii] );

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
    TLinearOrder<CpCertManUICertData> order( ComparePeerLabels );
    iPeerLabelEntries.Sort( order );

    }


// ---------------------------------------------------------
// CCertManUIKeeper::RefreshUserCertEntriesL()
// Checks that cacerts.dat file is not corrupted and
// returns User certificates from it
// ---------------------------------------------------------
//
void CpCertDataContainer::RefreshUserCertEntriesL()
    {
    RDEBUG("0", 0);
    iFilter->SetOwnerType( EUserCertificate );
    iUserEntries.Close();
    CUnifiedCertStore*& store = CertManager();
    iWrapper->ListL( store, &iUserEntries, *iFilter );

    iUserLabelEntries.Close();
    
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
                CpCertManUICertData* data = new (ELeave) CpCertManUICertData();
                CleanupStack::PushL( data );
                data->iUserEntry = CCTCertInfo::NewL( *iUserEntries[i] );
                
                HBufC* pri = NULL;
                HBufC* sec = NULL;

                CCertificate* cert = NULL;
                TRAPD(error, iWrapper->GetCertificateL( CertManager(), *iUserEntries[i], cert ));

                if( error == KErrArgument)
                    {
                    CleanupStack::PopAndDestroy( data );
                    break;
                    }
                else if ( error != KErrNone )
                    {
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

                CleanupStack::PopAndDestroy( 3, cert ); // pri, sec
                CleanupStack::Pop( data ); // data
                break;
                } // EX509Certificate

            case EX509CertificateUrl:
                {
                CpCertManUICertData* urlCertData = new (ELeave) CpCertManUICertData();
                CleanupStack::PushL( urlCertData );
                urlCertData->iUserEntry = CCTCertInfo::NewL( *iUserEntries[i] );
            
                CCertificate* cert = NULL;
                TRAPD(error, iWrapper->GetCertificateL( CertManager(), *iUserEntries[i], cert ));
                if( error == KErrArgument )
                    {
                    CleanupStack::PopAndDestroy( urlCertData ); // urlCertData
                    break;
                    }
                else if ( error != KErrNone )
                    {
                    User::Leave(error);
                    }
                else
                    {
                    CleanupStack::PushL( cert );
                    }

				_LIT(KUrlUserCert, "URL UserCert");
                HBufC* UrlLabel = HBufC::NewL( KUrlUserCert().Length() );
                UrlLabel->Des().Append( KUrlUserCert());

                urlCertData->iUserEntryLabel = UrlLabel;
                iUserLabelEntries.Append( urlCertData );

                CleanupStack::PopAndDestroy( cert ); // cert
                CleanupStack::Pop( urlCertData ); // data
                break;
                }
            }   // switch

        }   // for

    // Sort the certificates.
    TLinearOrder<CpCertManUICertData> order( CompareUserLabels );
    iUserLabelEntries.Sort( order );

    }

// ---------------------------------------------------------
// CCertManUIKeeper::RefreshDeviceCertEntriesL()
// returns device certificates
// ---------------------------------------------------------
//
void CpCertDataContainer::RefreshDeviceCertEntriesL()
    {
    RDEBUG("0", 0);
    iFilter->SetOwnerType( EUserCertificate );
    iDeviceEntries.Close();
    CUnifiedCertStore*& store = CertManager();

    iWrapper->ListL( store, &iDeviceEntries, *iFilter, KCMDeviceCertStoreTokenUid );

    iDeviceLabelEntries.Close();
    for ( TInt i = 0; i < iDeviceEntries.Count(); i++ )
        {
        switch ( iDeviceEntries[i]->CertificateFormat() )
            {
            case EX509Certificate:
                {
                CpCertManUICertData* data = new (ELeave) CpCertManUICertData();
                CleanupStack::PushL( data );
                data->iDeviceEntry = CCTCertInfo::NewL( *iDeviceEntries[i] );
                
                HBufC* pri = NULL;
                HBufC* sec = NULL;

                CCertificate* cert = NULL;
                TRAPD(error, iWrapper->GetCertificateL( CertManager(), *iDeviceEntries[i], cert, KCMDeviceCertStoreTokenUid ));

                if( error == KErrArgument)
                    {
                    CleanupStack::PopAndDestroy( data );
                    break;
                    }
                else if ( error != KErrNone )
                    {
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

                CleanupStack::PopAndDestroy( 3, cert ); // pri, sec
                CleanupStack::Pop( data ); // data
                break;
                } // EX509Certificate

            case EX509CertificateUrl:
                {
                CpCertManUICertData* urlCertData = new (ELeave) CpCertManUICertData();
                CleanupStack::PushL( urlCertData );
                urlCertData->iDeviceEntry = CCTCertInfo::NewL( *iDeviceEntries[i] );

                CCertificate* cert = NULL;
                TRAPD(error, iWrapper->GetCertificateL( CertManager(), *iDeviceEntries[i], cert, KCMDeviceCertStoreTokenUid ));
                if( error == KErrArgument)
                    {
                    CleanupStack::PopAndDestroy( urlCertData ); // urlCertData
                    break;
                    }
                else if ( error != KErrNone )
                    {
                    User::Leave(error);
                    }
                else
                    {
                    CleanupStack::PushL( cert );
                    }

                _LIT(KUrlUserCert, "URL UserCert");
                HBufC* UrlLabel = HBufC::NewL( KUrlUserCert().Length() );
                UrlLabel->Des().Append( KUrlUserCert());


                urlCertData->iDeviceEntryLabel = UrlLabel;
                iDeviceLabelEntries.Append( urlCertData );

                CleanupStack::PopAndDestroy( cert ); // cert
                CleanupStack::Pop( urlCertData ); // data
                break;
                }
            }   // switch

        }   // for

    // Sort the certificates.
    TLinearOrder<CpCertManUICertData> order( CompareDeviceLabels );
    iDeviceLabelEntries.Sort( order );

    }

// ---------------------------------------------------------
// CCertManUIKeeper::ShowErrorNoteL( TInt aError )
// Shows error note
// ---------------------------------------------------------
//
void CpCertDataContainer::ShowErrorNoteL( TInt aError )
    {
	QString sError;
    switch ( aError )
        {
        case KErrCorrupt:
            {
            sError = "Database corrupted. Some data might have been lost.";
            break;
            }
        case KErrCancel:
            {
            // Show nothing
            break;
            }
         default:
            {
            sError = "Internal error";
            break;
            }
        }

    if ( sError.length() != 0 )
        {
		HbMessageBox::information(sError);
        }
    }

