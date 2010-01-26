/*
* Copyright (c) 2003 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Certificate trust settings information
*
*/



// INCLUDE FILES

#include "WimCertInfo.h"
#include "WimDummyToken.h"
#include <securitydefs.h>
#include "WimTrace.h"


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWimCertInfo::CWimCertInfo( CCTCertInfo* aCCTCertInfo, 
//                            const TBuf8<KSHA1HashLengthBytes> aHash, 
//                            RArray<HBufC*> aTrustedUsages, 
//                            TUint8 aCDFRefs ) 
//                          : iCCTCertInfo( aCCTCertInfo ),
//                            iCertHash( aHash ),
//                            iTrustedUsage( aTrustedUsages ),
//                            iCDFRefs( aCDFRefs )
// Default constructor
// -----------------------------------------------------------------------------
//
CWimCertInfo::CWimCertInfo( CCTCertInfo* aCCTCertInfo, 
                            const TBuf8<KSHA1HashLengthBytes> aHash, 
                            RArray<HBufC*> aTrustedUsages, 
                            TUint8 aCDFRefs ) 
                          : iCCTCertInfo( aCCTCertInfo ),
                            iCertHash( aHash ),
                            iTrustedUsage( aTrustedUsages ),
                            iCDFRefs( aCDFRefs )
    {
    _WIMTRACE(_L("WIM | WIMUtil | CWimCertInfo::CWimCertInfo | Begin"));
    }

// -----------------------------------------------------------------------------
// CWimCertInfo::ConstructL()
// Second phase constructor
// -----------------------------------------------------------------------------
//
void CWimCertInfo::ConstructL()
    {
    _WIMTRACE(_L("WIM | WIMUtil | CWimCertInfo::ConstructL | Begin"));
    }

// -----------------------------------------------------------------------------
// CWimCertInfo::NewLNewL( CCTCertInfo* aCCTCertInfo,
//                         const TBuf8<KSHA1HashLengthBytes> aHash,
//                         RArray<HBufC*> aTrustedUsages,
//                         TUint8 aCDFRefs )
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CWimCertInfo* CWimCertInfo::NewL( CCTCertInfo* aCCTCertInfo,
                                     const TBuf8<KSHA1HashLengthBytes> aHash,
                                     RArray<HBufC*> aTrustedUsages,
                                     TUint8 aCDFRefs )
    {
    _WIMTRACE(_L("WIM | WIMUtil | CWimCertInfo::NewL | Begin"));
    CWimCertInfo* self = new( ELeave ) CWimCertInfo( aCCTCertInfo, 
                                                     aHash,
                                                     aTrustedUsages,
                                                     aCDFRefs );
    CleanupStack::PushL( self );
    self->ConstructL( );
    CleanupStack::Pop( self );
    return self;
    }
    
// -----------------------------------------------------------------------------
// CWimCertInfo::~CWimCertInfo()
// Destructor
// -----------------------------------------------------------------------------
//
CWimCertInfo::~CWimCertInfo()
    {
    _WIMTRACE(_L("WIM | WIMUtil | CWimCertInfo::~CWimCertInfo | Begin"));
    if ( iCCTCertInfo )
        {
        iCCTCertInfo->Release();
        iCCTCertInfo = NULL;
        }

    //Delete arrays of trusted usaga and extendedkeyusage
    for ( TInt i = 0; i < iTrustedUsage.Count(); i++ )
        {
        delete iTrustedUsage[i]; //Delete buffers
        }

    iTrustedUsage.Close();
    
    for ( TInt j = 0; j < iExtendedKeyUsage.Count(); j++ )
        {
        delete iExtendedKeyUsage[j]; //Delete buffers
        }

    iExtendedKeyUsage.Close();
    }

// -----------------------------------------------------------------------------
// CWimCertInfo::AddTrustedUsage( HBufC* aTrustedUsage )
// Add TrustedUsage OID
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CWimCertInfo::AddTrustedUsage( HBufC* aTrustedUsage )
    {
    _WIMTRACE(_L("WIM | WIMUtil | CWimCertInfo::AddTrustedUsage | Begin"));
    return ( iTrustedUsage.Append( aTrustedUsage ) );
    }

// -----------------------------------------------------------------------------
// CWimCertInfo::AddExtendedKeyUsage( HBufC* aExtendedKeyUsage )
// Add ExtendedKeyUsage OID
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CWimCertInfo::AddExtendedKeyUsage( HBufC* aExtendedKeyUsage )
    {
    _WIMTRACE(_L("WIM | WIMUtil | CWimCertInfo::AddExtendedKeyUsage | Begin"));
    return ( iExtendedKeyUsage.Append( aExtendedKeyUsage ) );
    }

// -----------------------------------------------------------------------------
// CWimCertInfo::GetCertHash( TBuf8<KSHA1HashLengthBytes>& aHash ) const
// Get hash of certificate
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimCertInfo::GetCertHash( 
                            TBuf8<KSHA1HashLengthBytes>& aHash ) const
    {
    _WIMTRACE(_L("WIM | WIMUtil | CWimCertInfo::GetCertHash | Begin"));
    aHash = iCertHash;
    }

// -----------------------------------------------------------------------------
// CWimCertInfo::CctCert() const
// Return pointer to CctCert (Symbian's certificate info )
// -----------------------------------------------------------------------------
//
EXPORT_C const CCTCertInfo* CWimCertInfo::CctCert() const
    {
    _WIMTRACE(_L("WIM | WIMUtil | CWimCertInfo::CctCert | Begin"));
    return iCCTCertInfo;
    }

// -----------------------------------------------------------------------------
// CWimCertInfo::TrustedUsage() const
// Get pointer array of TrustedUsage OID's
// -----------------------------------------------------------------------------
//
EXPORT_C RArray<HBufC*> CWimCertInfo::TrustedUsage() const
    {
    _WIMTRACE(_L("WIM | WIMUtil | CWimCertInfo::TrustedUsage | Begin"));
    return iTrustedUsage;
    }
// -----------------------------------------------------------------------------
// CWimCertInfo::ExtendedKeyUsage
// Get pointer array of ExtendedKeyUsage OID's
// -----------------------------------------------------------------------------
//
EXPORT_C RArray<HBufC*> CWimCertInfo::ExtendedKeyUsage() const
    {
    _WIMTRACE(_L("WIM | WIMUtil | CWimCertInfo::ExtendedKeyUsage | Begin"));
    return iExtendedKeyUsage;
    }
// -----------------------------------------------------------------------------
// CWimCertInfo::CDFRefs
// Get location of certificate
// -----------------------------------------------------------------------------
//
EXPORT_C TUint8 CWimCertInfo::CDFRefs() const
    {
    _WIMTRACE(_L("WIM | WIMUtil | CWimCertInfo::CDFRefs | Begin"));
    return iCDFRefs;
    }
// -----------------------------------------------------------------------------
// CWimCertInfo::SetCDFRefs
// Set location of certificate
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimCertInfo::SetCDFRefs( TUint8 aCDFRefs )
    {
    _WIMTRACE(_L("WIM | WIMUtil | CWimCertInfo::SetCDFRefs | Begin"));
    iCDFRefs = aCDFRefs;
    }

// -----------------------------------------------------------------------------
// CWimCertInfo::Externalize
// Externalize CWimCertInfo data to flat buffer
// -----------------------------------------------------------------------------
//
EXPORT_C TWimCertInfoPckg* CWimCertInfo::ExternalizeL() const
    {
    _WIMTRACE(_L("WIM | WIMUtil | CWimCertInfo::ExternalizeL | Begin"));
    TWimCertInfoPckg* certPckg = new( ELeave ) TWimCertInfoPckg();
    
    GetCertHash( certPckg->iHash );
    certPckg->iCertLabel.Copy( iCCTCertInfo->Label() );
    certPckg->iCertificateOwnerType = iCCTCertInfo->CertificateOwnerType();
    certPckg->iSize = iCCTCertInfo->Size();
    certPckg->iFormat = iCCTCertInfo->CertificateFormat();
    certPckg->iSubjectKeyId = iCCTCertInfo->SubjectKeyId();
    certPckg->iIssuerKeyId = iCCTCertInfo->IssuerKeyId();
    certPckg->iCDFRefs = iCDFRefs;
    certPckg->iTokenSerialNbr = iCCTCertInfo->Token().Label();
    certPckg->iIndex = iCCTCertInfo->Handle().iObjectId;
    return certPckg;
    }

// -----------------------------------------------------------------------------
// CWimCertInfo::InternalizeL
// Instantiates CWimCertInfo of externalized data
// -----------------------------------------------------------------------------
//
EXPORT_C CWimCertInfo* CWimCertInfo::InternalizeL( 
    TWimCertInfoPckg& aExternalizedInfo )
    {
    _WIMTRACE(_L("WIM | WIMUtil | CWimCertInfo::InternalizeL | Begin"));
    RArray<HBufC*> trustedUsages;
      
    CWimDummyTokenType* tokenType = CWimDummyTokenType::NewL();
    CleanupReleasePushL( *tokenType );

    CWimDummyToken* token = CWimDummyToken::NewL( *tokenType );
	CleanupStack::Pop( tokenType );
    CleanupReleasePushL( *token );

    token->SetSerialNumber( aExternalizedInfo.iTokenSerialNbr );
  
    TInt certificateId = aExternalizedInfo.iIndex;
    TBool deletable = EFalse;


    CCTCertInfo* cctCertInfo = CCTCertInfo::NewL( 
                                        aExternalizedInfo.iCertLabel,
                                        aExternalizedInfo.iFormat,
                                        aExternalizedInfo.iCertificateOwnerType,
                                        aExternalizedInfo.iSize,
                                        &(aExternalizedInfo.iSubjectKeyId),
                                        &(aExternalizedInfo.iIssuerKeyId),
                                        *token,
                                        certificateId,
                                        deletable );
	CleanupStack::Pop( token );
	CleanupReleasePushL( *cctCertInfo );    

    CWimCertInfo* certInfo = CWimCertInfo::NewL( cctCertInfo,
                                                 aExternalizedInfo.iHash,
                                                 trustedUsages,
                                                 0 );

    CleanupStack::Pop( cctCertInfo );

    certInfo->SetCDFRefs( aExternalizedInfo.iCDFRefs );

    return certInfo;
    }

//  End of File  
