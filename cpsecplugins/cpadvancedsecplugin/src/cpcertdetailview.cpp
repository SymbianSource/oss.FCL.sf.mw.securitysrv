/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0""
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  
*
*/
#include <cctcertinfo.h>
#include <x509cert.h>
#include <unifiedcertstore.h>
#include <X509CertNameParser.h>
#include <securitydefs.h>
#include <hash.h>                   // MD5 fingerprint
#include <asymmetrickeys.h>

#include <e32math.h>                // Pow
#include <e32cmn.h>

#include <QDateTime>
#include <QStringList>
#include <QGraphicsLinearLayout>

#include <hblineedit.h>
#include <hbdataform.h>
#include <hbdataformmodel.h>
#include <hbdataformmodelitem.h>
#include <hblabel.h>
#include <hbpushbutton.h>
#include <hbmenu.h>
#include <hbaction.h>
#include <hbmessagebox.h>
#include <HbListWidget>
#include <HbListWidgetItem>
#include <HbGroupBox>
#include <hbpushbutton.h>
#include <hbdataform.h>
#include <hbdataformmodel.h>
#include <hbdataformmodelitem.h>

#include "cpcertdetailview.h"
#include "cpcertview.h"
#include "cpcertdatacontainer.h"
#include "cpcertmanuisyncwrapper.h"
#include "../../../pkiutilities/DeviceToken/Inc/TrustedSitesStore.h"

const TInt KFileCertStoreUid( 0x101F501A );
const TInt KTrustedServerCertStoreUid( 0x101FB66F );
const TInt KDeviceCertStoreUid( 0x101FB668 );
const TInt KDeviceKeyStoreUid( 0x101FB66A );
const TInt KWIMCertStoreUid ( 0x101F79D9 );

const TInt KMaxLengthTextSerialNumberFormatting = 3;
_LIT( KCertManUIDetailsViewHexaFormat, "%02X" );
_LIT( KBlockSeparator, " " );

template <class T>
class CleanupResetAndDestroy
    {
public:
    inline static void PushL( T& aRef );
private:
    static void ResetAndDestroy( TAny *aPtr );
    };

template <class T>
inline void CleanupResetAndDestroyPushL( T& aRef );

template <class T>
inline void CleanupResetAndDestroy<T>::PushL( T& aRef )
    {
    CleanupStack::PushL( TCleanupItem( &ResetAndDestroy, &aRef ) );
    }

template <class T>
void CleanupResetAndDestroy<T>::ResetAndDestroy( TAny *aPtr )
    {
    if( aPtr )
        {
        static_cast<T*>( aPtr )->ResetAndDestroy();
        }
    }

template <class T>
inline void CleanupResetAndDestroyPushL( T& aRef )
    {
    CleanupResetAndDestroy<T>::PushL( aRef );
    }

CpCertDetailView::CpCertDetailView(CpCertView::TCertificateViews aCurrentView,TInt aPos, CpCertDataContainer& aCertDataContainer, QGraphicsItem *parent /*= 0*/)
: CpBaseSettingView(0,parent), mCertDataContainer(aCertDataContainer)
	{
	setTitle(tr("Certificate Details"));
	
	try
		{
		QT_TRAP_THROWING(viewCertificateDetailsL(aPos, aCurrentView));
		}
	catch(const std::exception& exception)
		{
		HbMessageBox::information(exception.what());
		throw(exception);
		}
	}


CpCertDetailView::~CpCertDetailView()
	{}

void CpCertDetailView::viewCertificateDetailsL( TInt aIndex, CpCertView::TCertificateViews aType )
	{
	
	QGraphicsLinearLayout *layout = q_check_ptr(new QGraphicsLinearLayout(Qt::Vertical));	
	setLayout(layout);
	
	HbListWidget* mCertDetails = q_check_ptr(new HbListWidget(this));
		
    HBufC8* urlBuf = NULL;
    CCertificate* details = NULL;
    CUnifiedCertStore*& store = mCertDataContainer.CertManager();

    const CCTCertInfo* entry = certInfo(aIndex, aType);
    TCertificateFormat certificateFormat = entry->CertificateFormat();

    if( certificateFormat != EX509CertificateUrl )
        {
        mCertDataContainer.iWrapper->GetCertificateL( store, *entry, details );
        CleanupStack::PushL( details );
        validateCertificateL( aIndex, *details, *entry, aType );
        }
    else
        {
        urlBuf = HBufC8::NewLC( entry->Size() );
        TPtr8 urlPtr = urlBuf->Des();
        mCertDataContainer.iWrapper->GetUrlCertificateL( store, *entry, urlPtr );
        }

    if( aType == CpCertView::ETrustedView )
        {
		addTrustedSitesL(*details, *mCertDetails);
        }
    else
        {
		addLabel(*entry, *mCertDetails);
        }

    // certificate issuer and owner
    if( certificateFormat == EX509Certificate )
        {
		addIssuerL(*details, *mCertDetails);
		addSubjectL(*details, *mCertDetails);
        }

    // Get key usage and location. This is done only client certificates.
    const TInt KLocationUnknown = 0;
    TUid keyLocation = { KLocationUnknown };

    if(( aType == CpCertView::EPersonalView ) || ( aType == CpCertView::EDeviceView ))
        {
		TKeyUsagePKCS15 usage = keyUsageAndLocationL( *entry, &keyLocation );
        addCertUsage(usage, *mCertDetails);		
        }

    if( certificateFormat == EX509Certificate )
        {
		addValidityPeriod( *details, *mCertDetails );                
        TUid certLoc;
        certLoc.iUid = entry->Token().TokenType().Type().iUid;
        // add the location of the certificate
        setLocationInfo( ETrue, certLoc, *mCertDetails );
        }
    else if( certificateFormat == EX509CertificateUrl )
        {
        setURLLocation( *urlBuf, *mCertDetails );
        }
    else
        {
        // nothing
        }

    addCertFormat(certificateFormat, *mCertDetails);
            
    // Private key location, only for user and device certicates
    if(( aType == CpCertView::EPersonalView ) || ( aType == CpCertView::EDeviceView ))
        {
        setLocationInfo( EFalse, keyLocation, *mCertDetails );
        }

    if( certificateFormat == EX509Certificate )
        {
		addAlgoDetails( *details, *mCertDetails);
        addSerialNo( *details, *mCertDetails );
        addFingerprint( *details, *mCertDetails);
        addPublicKeyDetailsL( *details, *mCertDetails );
        }
    else
        {
        // SetUrlCertAlgorihm()
        }

    if( certificateFormat != EX509CertificateUrl )
        {
        CleanupStack::PopAndDestroy( details );
        }
    else
        {
        CleanupStack::PopAndDestroy( urlBuf );
        }

    layout->addItem(mCertDetails);
    }

const CCTCertInfo* CpCertDetailView::certInfo( TInt index, CpCertView::TCertificateViews certView )
	{
	const CCTCertInfo* certinfo = NULL;
	
	if( certView == CpCertView::EAuthorityView )
		{
		certinfo = ( mCertDataContainer.iCALabelEntries )[ index ]->iCAEntry;
		}
	else if( certView == CpCertView::ETrustedView )
		{
		certinfo = ( mCertDataContainer.iPeerLabelEntries )[ index ]->iPeerEntry;
		}
	else if( certView == CpCertView::EDeviceView )
		{
		certinfo = ( mCertDataContainer.iDeviceLabelEntries )[ index ]->iDeviceEntry;
		}
	else if( certView == CpCertView::EPersonalView )
		{
		certinfo = ( mCertDataContainer.iUserLabelEntries )[ index ]->iUserEntry;
		}
	return certinfo;
	}

void CpCertDetailView::addTrustedSitesL( const CCertificate& certDetails, HbListWidget& certDisplayDetails )
	{
	CTrustSitesStore* trustedSitesStore = CTrustSitesStore::NewL();
	CleanupStack::PushL( trustedSitesStore );

	RPointerArray<HBufC> trustedSites;
	CleanupResetAndDestroyPushL( trustedSites );
	trustedSitesStore->GetTrustedSitesL( certDetails.Encoding(), trustedSites );

	QString sTrustedSites = "Site:";
	if( trustedSites.Count() > 0 )
		{
		// display trusted sites
		sTrustedSites = "Site:";
		}
	addToListWidget(certDisplayDetails, sTrustedSites);
	
	for(int index=0;index<trustedSites.Count(); ++index)
		{
		QString trustedSiteDetail = QString((QChar*)(trustedSites[index]->Ptr()), trustedSites[index]->Length());
		addToListWidget(certDisplayDetails, trustedSiteDetail);
		}
	
	CleanupStack::PopAndDestroy( &trustedSites );
	CleanupStack::PopAndDestroy( trustedSitesStore );

	}

void CpCertDetailView::addLabel( const CCTCertInfo& aCertInfo, HbListWidget& certDisplayDetails )
	{
	QString sLabel("Label:");
	QString sLabelDetails = "(No label)"; 
	if(aCertInfo.Label().Length()  != 0)
		{
		sLabelDetails = QString((QChar*)(aCertInfo.Label().Ptr()),aCertInfo.Label().Length());
		}
	addToListWidget(certDisplayDetails, sLabel, sLabelDetails);
	}

void CpCertDetailView::addIssuerL( 	const CCertificate& aCertDetails, 
									HbListWidget& certDisplayDetails )
	{
	HBufC* issuer = NULL;
	X509CertNameParser::IssuerFullNameL( (CX509Certificate&)aCertDetails, issuer );
	CleanupStack::PushL( issuer );

	QString sIssuer = "Issuer:";
	QString sIssuerDetails = "(Not defined)"; 
	if(issuer->Des().Length() != 0)
		{
		sIssuerDetails = QString((QChar*)issuer->Des().Ptr(),issuer->Des().Length());
		}
	addToListWidget(certDisplayDetails, sIssuer, sIssuerDetails);
		
	CleanupStack::PopAndDestroy( issuer ); 
	}

void CpCertDetailView::addSubjectL( const CCertificate& aCertDetails, 
									HbListWidget& certDisplayDetails )
	{
	HBufC* subject = NULL;
	X509CertNameParser::SubjectFullNameL( (CX509Certificate&)aCertDetails, subject );
	CleanupStack::PushL( subject );

	QString sSubject = "Subject:";
	QString sSubjectDetails = "(No subject)";
	if(subject->Des().Length() != 0)
		{
		sSubjectDetails = QString((QChar*)subject->Des().Ptr(),subject->Des().Length());
		}
	
	addToListWidget(certDisplayDetails, sSubject, sSubjectDetails);
	
	CleanupStack::PopAndDestroy( subject );
	}

void CpCertDetailView::addCertUsage( TKeyUsagePKCS15 usage, HbListWidget& certDisplayDetails )
	{
	QString strUsage;
	switch( usage )
		{
		case EPKCS15UsageSignSignRecover:
		case EPKCS15UsageSignDecrypt:
		case EPKCS15UsageSign:
			strUsage = "Client authentication";
			break;
		case EPKCS15UsageNonRepudiation:
			strUsage = "Digital signing";
			break;
		default:
			strUsage = "(Not defined)";
			break;
		}
	addToListWidget(certDisplayDetails, strUsage );     
	}


void CpCertDetailView::addValidityPeriod( 	const CCertificate& aCertDetails, 
											HbListWidget& certDisplayDetails)
	{
	// Certificate validity period
	// Hometime's offset to UTC
	TLocale locale;
	TTimeIntervalSeconds offSet = locale.UniversalTimeOffset();
	
	QString sValidFrom("Valid from: ");
	const CValidityPeriod& validityPeriod = aCertDetails.ValidityPeriod();
	TTime startValue = validityPeriod.Start();
	startValue += offSet;
	TDateTime dateTime = startValue.DateTime();
	TMonth month = dateTime.Month();
	TInt day = dateTime.Day();
	TInt year = dateTime.Year();
	QDateTime qValidityPeriod(QDate(year, month, day));
	QString sValidFromDetail = qValidityPeriod.toString("MM\\dd\\yyyy");
	
	addToListWidget(certDisplayDetails, sValidFrom, sValidFromDetail);
		
	QString sValidUntil("Valid until: ");
	TTime finishValue = validityPeriod.Finish();
	finishValue += offSet;
	dateTime = finishValue.DateTime();
	month = dateTime.Month();
	day = dateTime.Day();
	year = dateTime.Year();
	qValidityPeriod.setDate(QDate(year, month, day));
	QString sValidUntilDetail = qValidityPeriod.toString("MM\\dd\\yyyy");
	
	addToListWidget(certDisplayDetails, sValidUntil, sValidUntilDetail);
		
	}

void CpCertDetailView::setURLLocation( HBufC8& urlDetails, HbListWidget& certDisplayDetails )
	{
	TPtr8 urlPtr = urlDetails.Des();
		
	QString certUrl = "Location:"; 
	QString certURLDetail = "(Not defined)";
	TInt length = urlPtr.Length();
	if(length != 0)
		{
		certURLDetail = QString((QChar*)urlPtr.Ptr(),urlPtr.Length());
		}
	addToListWidget(certDisplayDetails, certUrl, certURLDetail);
	}

void CpCertDetailView::addCertFormat( TCertificateFormat aCertFormat, HbListWidget& certDisplayDetails )
	{
	// certificate format
	QString certFormat = "Certificate format:";
	QString certFormatDetails = "(Not defined)";
	if( aCertFormat == EX509Certificate || aCertFormat == EX509CertificateUrl)
		{
		certFormatDetails = "X509";
		}
	addToListWidget(certDisplayDetails, certFormat, certFormatDetails);
	}

void CpCertDetailView::addAlgoDetails( 	const CCertificate& certificate, 
										HbListWidget& certDisplayDetails )
	{
	// certificate algorithms
	QString sDgstAlgoDetails = "Unknown";
	// digest algorithm
	TAlgorithmId algorithmId = certificate.SigningAlgorithm().DigestAlgorithm().Algorithm();
	switch( algorithmId )
		{
		case EMD2:
			{
			sDgstAlgoDetails = "MD2";
			break;
			}
		case EMD5:
			{
			sDgstAlgoDetails = "MD5";
			break;
			}
		case ESHA1:
			{
			sDgstAlgoDetails = "SHA1";
			break;
			}
		}

	QString sAsymmetricAlgoDetails = asymmetricAlgoId(certificate);
	QString sAlgorithm("Algorithm:");
	QString sAlgoDetails = "Unknown";
	// If other algorithm is unknown
	if( sDgstAlgoDetails != "Unknown" && sAsymmetricAlgoDetails != "Unknown" )
		{
		sAlgoDetails = sDgstAlgoDetails;
		sAlgoDetails.append(sAsymmetricAlgoDetails);
		}
	addToListWidget(certDisplayDetails, sAlgorithm, sAlgoDetails);
	}

void CpCertDetailView::addSerialNo( const CCertificate& certificate, 
									HbListWidget& certDisplayDetails )
	{
	// certificate serial number
	QString sCertSerialNo("Serial number:");
	QString sCertSerialNoDetails = 0;
	TPtrC8 serialNumber = certificate.SerialNumber();
	TBuf<KMaxLengthTextSerialNumberFormatting> buf2;

	for( TInt i = 0; i < serialNumber.Length(); i++ )
	   {
	   buf2.Format( KCertManUIDetailsViewHexaFormat, serialNumber[i] );
	   sCertSerialNoDetails.append( QString( (QChar*)buf2.Ptr(),buf2.Length() ) );
	   }
	addToListWidget(certDisplayDetails, sCertSerialNo, sCertSerialNoDetails);
	}

void CpCertDetailView::addFingerprint(	const CCertificate& certificate, 
										HbListWidget& certDisplayDetails)
	{
	// certificate fingerprint SHA-1
	QString sSha1("Fingerprint (SHA1):");
	TPtrC8 sha1_fingerprint = certificate.Fingerprint();
	HBufC* finalSha1 = NULL;
	QT_TRAP_THROWING(	 finalSha1 = HBufC::NewLC(100));
	TPtr finalShaPtr = finalSha1->Des();
	divideIntoBlocks( sha1_fingerprint, finalShaPtr );
	QString sSha1Details = QString( (QChar*)finalShaPtr.Ptr(), finalShaPtr.Length() );
	CleanupStack::PopAndDestroy(finalSha1);
	
	addToListWidget(certDisplayDetails, sSha1, sSha1Details);
		
	// certificate fingerprint MD5
	QString sMd5("Fingerprint (MD5):");
	CMD5* md5= NULL;
	QT_TRAP_THROWING(	 md5 = CMD5::NewL());
	QT_TRAP_THROWING(CleanupStack::PushL( md5 ));
	TBuf8<20> fingerprint = md5->Hash( certificate.Encoding() );
	CleanupStack::PopAndDestroy( md5 );
	HBufC* finalMd5 = NULL;
	QT_TRAP_THROWING(	finalMd5 = HBufC::NewLC(100));
	TPtr finalMd5Ptr = finalMd5->Des();
	divideIntoBlocks( fingerprint, finalMd5Ptr );
	QString sMd5Details = QString( (QChar*)finalMd5Ptr.Ptr(), finalMd5Ptr.Length());
	CleanupStack::PopAndDestroy(finalMd5);
	addToListWidget(certDisplayDetails, sMd5, sMd5Details);
	}

void CpCertDetailView::addPublicKeyDetailsL(	const CCertificate& certificate, 
											HbListWidget& certDisplayDetails )
	{
	// Public key
    // Should look like: "Public key (%0U %1N bit):"
        
	QString sPublicKey("Public key ("); // %S %d bits):");
	sPublicKey.append(asymmetricAlgoId(certificate));
	
	// Get public key already here to get length of it for string loader.
	const CSubjectPublicKeyInfo& publicKey = certificate.PublicKey();
	TPtrC8 keyData = publicKey.KeyData();
	TInt size = keyData.Size();  // %1N

	// To get key size decode DER encoded key got from the certificate.
	TX509KeyFactory key;

	// There doesn't seem to be definition of MAX_INT anywhere so calculate it.
	TReal maxInt;
	TReal intBits = sizeof( TInt ) * 8;
	Math::Pow( maxInt, 2, intBits );
	maxInt = ( maxInt / 2 ) - 1;

	TAlgorithmId algorithmId = certificate.SigningAlgorithm().AsymmetricAlgorithm().Algorithm();
	switch( algorithmId )
		{
		case ERSA:
			{
			const CRSAPublicKey* keyRSA = key.RSAPublicKeyL( keyData );

			const TInteger& n = keyRSA->N();

			TUint keySizeN = n.BitCount();

			// Play it safe.
			if( keySizeN < maxInt )
				{
				size = keySizeN;
				}

			delete keyRSA;

			break;
			}
		case EDSA:
			{
			TPtrC8 params = publicKey.EncodedParams();
			const CDSAPublicKey* keyDSA = key.DSAPublicKeyL( params, keyData );

			const TInteger& y = keyDSA->Y();

			TUint keySizeY = y.BitCount();

			// Play it safe.
			if( keySizeY < maxInt )
				{
				size = keySizeY;
				}

			delete keyDSA;

			break;
			}
		// There doesn't seem to be TX509KeyFactory function for DH keys.
		// If the key is DH or unknown, just multiply length of the key
		// in bytes with 8. It is not correct but at least gives an idea.
		// Without setting something to size 'unknown' text should be used
		// below for the string which is much more error prone than setting
		// at least something.
		case EDH:
		default:
			{
			size = 8 * keyData.Size();
			}
		}

	sPublicKey.append(" ").append(QString::number(size)).append(" bits):");
	
	// Finally append the public key.
	HBufC* pubKey = HBufC::NewLC(size*3);
	TPtr pubKeyPtr = pubKey->Des();
	divideIntoBlocks( keyData, pubKeyPtr );
	QString sPublicKeyDetails = QString ( (QChar*)pubKeyPtr.Ptr(), pubKeyPtr.Length());
	CleanupStack::PopAndDestroy(pubKey);
	addToListWidget(certDisplayDetails, sPublicKey, sPublicKeyDetails);
	}

QString CpCertDetailView::asymmetricAlgoId( const CCertificate& certificate )
	{
	QString sAsymmetricAlgoId = 0;
	// public-key algorithm
	TAlgorithmId algorithmId = certificate.SigningAlgorithm().AsymmetricAlgorithm().Algorithm();
	switch( algorithmId )
		{
		case ERSA:
			{
			sAsymmetricAlgoId = "RSA";
			break;
			}
		case EDSA:
			{
			sAsymmetricAlgoId = "DSA";
			break;
			}
		case EDH:
			{
			sAsymmetricAlgoId = "DH";
			break;
			}
		default:
			{
			sAsymmetricAlgoId = "Unknown";
			}
		}
	return sAsymmetricAlgoId;
	}

void CpCertDetailView::addToListWidget(HbListWidget& certDisplayDetails, const QString& data )
	{
	HbListWidgetItem* wItem = q_check_ptr(new HbListWidgetItem());
	wItem->setText(data);
	certDisplayDetails.addItem(wItem);	
	}

void CpCertDetailView::addToListWidget(	HbListWidget& certDisplayDetails, 
										const QString& displayString, 
										const QString& displayDetails )
	{
	HbListWidgetItem* wDisplayString = q_check_ptr(new HbListWidgetItem());
	wDisplayString->setText(displayString);
	certDisplayDetails.addItem(wDisplayString);
		
	HbListWidgetItem* wDisplayDetails = q_check_ptr(new HbListWidgetItem());
	wDisplayDetails->setText(displayDetails);
	certDisplayDetails.addItem(wDisplayDetails);
	}

void CpCertDetailView::validateCertificateL(
    TInt aIndex, const CCertificate& certDetails, const CCTCertInfo& aCertInfo, CpCertView::TCertificateViews aType )
    {
    
    TInt poppableItems = 0;
    // Check allways first the validity period
    // Show Expired/Not yet valid notes
    const CValidityPeriod& validityPeriod = certDetails.ValidityPeriod();
    const TTime& startValue = validityPeriod.Start();
    const TTime& finishValue = validityPeriod.Finish();
    TTime current;
    current.UniversalTime();

    if ( startValue > current )
        {
        // certificate is not valid yet
        HbMessageBox::warning("The certificate is not valid yet");
        }
    else if ( finishValue < current )
        {
        // certificate is expired
        HbMessageBox::warning("The certificate has expired");
        }
    else
        {
        // put here check for certificate status, show warning notes if needed
        TBool noValidationError = ETrue;
        CArrayFixFlat<TValidationError>* validationError = NULL;
        // set it to the most common cert format
        TCertificateFormat certificateFormat = EX509Certificate;

        if ( aType == CpCertView::EAuthorityView )
            {
            certificateFormat = mCertDataContainer.iCALabelEntries[aIndex]->
                                        iCAEntry->CertificateFormat();
            }
        else if ( aType == CpCertView::ETrustedView )
            {
            certificateFormat = mCertDataContainer.iPeerLabelEntries[aIndex]->
                                        iPeerEntry->CertificateFormat();
            }
        else if ( aType == CpCertView::EDeviceView )
            {
            certificateFormat = mCertDataContainer.iDeviceLabelEntries[aIndex]->
                                        iDeviceEntry->CertificateFormat();
            }
        else if( aType == CpCertView::EPersonalView )
            {
            certificateFormat = mCertDataContainer.iUserLabelEntries[aIndex]->
                                        iUserEntry->CertificateFormat();
            }

        switch ( certificateFormat )
            {
            case EX509Certificate:
                {
                validationError = validateX509CertificateL((CX509Certificate&)certDetails );
                break;
                }
            default:
                {
                validationError = new ( ELeave) CArrayFixFlat<TValidationError>( 1 );
                break;
                }
            }
        CleanupStack::PushL( validationError );
        poppableItems++;

        TInt errorCount = validationError->Count();
        TBool ready = EFalse;
        for ( TInt i = 0; i < errorCount && !ready; i++ )
            {
            TValidationError errorType = validationError->At(i);

            switch ( errorType )
                {
                case EValidatedOK:
                case EChainHasNoRoot:
                case EBadKeyUsage:
                // Ignore these errors
                    //LOG_WRITE( "Ignored certificate validation error" );
                    break;

                default:
                // certificate is corrupted
                noValidationError = EFalse;
                HbMessageBox::warning("The certificate is corrupted");
                ready = ETrue;
                    break;
                }
            }

        if ( noValidationError  && (aType == CpCertView::EAuthorityView ))
        // Check for last if the CA certificate has no clients,
        // ie. the trust state of every client is No
        // For user certificates we don't do the check
            {

            RArray<TUid> trusterUids;
            CleanupClosePushL( trusterUids );
            poppableItems++;

            mCertDataContainer.iWrapper->GetApplicationsL(
                mCertDataContainer.CertManager(), aCertInfo, trusterUids );

            if ( trusterUids.Count() == 0)
                {
                HbMessageBox::warning("Certificate not trusted");
                }
            }
        }
    CleanupStack::PopAndDestroy( poppableItems );
    }

CArrayFixFlat<TValidationError>* CpCertDetailView::validateX509CertificateL( const CX509Certificate& certDetails )
    {
    TInt poppableItems = 0;
    CArrayFixFlat<TValidationError>* validationError =
        new ( ELeave ) CArrayFixFlat<TValidationError>( 1 );
    CleanupStack::PushL( validationError ); //This is returned, so it isn't destroyed at the end.
        
    TTime GMTTime;
    GMTTime.UniversalTime(); // Get Universal Time
    RPointerArray<CX509Certificate> certArray;
    certArray.Append( &certDetails );

    CPKIXCertChain* chain = CPKIXCertChain::NewLC(
        mCertDataContainer.iRfs, certDetails.Encoding(), certArray );
    ++poppableItems;

    CPKIXValidationResult* result = CPKIXValidationResult::NewLC();
    ++poppableItems;
    mCertDataContainer.iWrapper->ValidateX509RootCertificateL( result, GMTTime, chain );

    TValidationStatus validationStatus = result->Error();
    //Set reserve space. One for error, other for warnings.
    validationError->SetReserveL( 1 + result->Warnings().Count() );
    validationError->AppendL(validationStatus.iReason);
    for ( TUint8 i = 0; i < result->Warnings().Count(); i++ )
        {
		validationError->AppendL( result->Warnings().At(0).iReason );
        }

    CleanupStack::PopAndDestroy( poppableItems, chain );    //  All but validationError.
    CleanupStack::Pop(validationError); 
    return validationError;
    }

TKeyUsagePKCS15 CpCertDetailView::keyUsageAndLocationL( const CCTCertInfo& certEntry,
                                                        TUid* keyLocation )
    {
    TKeyUsagePKCS15 keyUsage;

    // Check whether we have key for this certificate
    RMPointerArray<CCTKeyInfo> keyEntry;
    TCTKeyAttributeFilter filter;
    filter.iKeyId = certEntry.SubjectKeyId();

    mCertDataContainer.iWrapper->ListL( mCertDataContainer.KeyManager(), &keyEntry, filter );

    if (keyEntry.Count())
        {
        keyUsage = keyEntry[0]->Usage();
        // Get Location
        keyLocation->iUid = keyEntry[0]->Token().TokenType().Type().iUid ;
        }
    else
        {
        keyUsage = EPKCS15UsageNone;
        }

    keyEntry.Close();
    return keyUsage;
    }

// ---------------------------------------------------------
// CCertManUICertificateHelper::setLocationInfo( (HBufC& aMessage,
//                                               TBool certificate,
//                                               TUid* aLocUid)
// Adds certificate/private key's location info to certificate details
// ---------------------------------------------------------
//
void CpCertDetailView::setLocationInfo(TBool certificate, TUid locUid, HbListWidget& certDisplayDetails)
    {
    
    QString locationDetails = "(Not defined)";

    switch ( locUid.iUid )
        {
        case KFileCertStoreUid:
        case KTrustedServerCertStoreUid:
        case KDeviceCertStoreUid:
        case KDeviceKeyStoreUid:
        case KTokenTypeFileKeystore:
        	locationDetails = "Phone memory";
            break;

        case KWIMCertStoreUid:
        	locationDetails = "Smart card";
            break;

        default:
            if ( !certificate )
                {
				locationDetails = "No private key";
                }
            break;
        }

    QString location = "Location:";
    if ( !certificate )
        {
		location = "Private key location:";
        }

    addToListWidget(certDisplayDetails, location, locationDetails );
            
    }

// ---------------------------------------------------------
// CCertManUICertificateHelper::divideIntoBlocks
// ---------------------------------------------------------
//
void CpCertDetailView::divideIntoBlocks( const TDesC8& input, TPtr& output )
    {
    const TInt KBlockLength = 2;
    TInt blockIndex = 0;
    for ( TInt j = 0 ; j < input.Length() ; j++ )
        {
        if ( blockIndex == KBlockLength )
            {
			output.Append( KBlockSeparator );
            blockIndex = 0;
            }
        output.AppendNumFixedWidthUC( (TUint)(input[ j ]), EHex, 2 );
        ++blockIndex;
        }
    }


