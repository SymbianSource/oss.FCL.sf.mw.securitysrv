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
* Description:   Extracts relevant certificate information to be shown to user.
*
*/


// INCLUDE FILES
#include "X509CertNameParser.h"
#include <x509cert.h>       //X509Certificate
#include <x500dn.h>         // CX500DistinguishedName
#include <pkixcertchain.h>  // OIDS for the extensions
#include <escapeutils.h>    // for escape decoding

// CONSTANTS
_LIT (KX509CertParserSpace, " ");

// Max length of array in case of PrimaryAndSecondaryNameL.
const TInt KX509CNPPrimAndSecondNameLengh( 3 ); 

// ============================ MEMBER FUNCTIONS =============================== 
                                                                                                                                                               
// -----------------------------------------------------------------------------
// X509CertNameParser::PrimaryAndSecondaryNameL
//
// -----------------------------------------------------------------------------
EXPORT_C TInt X509CertNameParser::PrimaryAndSecondaryNameL( 
                                      const CX509Certificate& aCertificate,
                                      HBufC*& aPrimaryName, 
                                      HBufC*& aSecondaryName,
                                      const TDesC& aLabel)
    {
    // Get subjectname
    const CX500DistinguishedName& dName = aCertificate.SubjectName();
    // Get subject alternative name
    const CX509CertExtension* extension = 
        aCertificate.Extension( KSubjectAltName );

    CDesCArrayFlat* nameArray = new( ELeave ) CDesCArrayFlat(3);
    CleanupStack::PushL( nameArray );
    // Append label to array
    nameArray->AppendL( aLabel );
    
    // Get PrimaryName and SecondaryName to nameArray if they exists.
    GetNamePartsL( dName, extension, *nameArray, EX509CNPPrimAndSecond );
    
    // Make sure that these are NULL
    aPrimaryName = NULL;
    aSecondaryName = NULL;

    switch ( nameArray->MdcaCount() )
        {
        case 1: //Neither PrimaryName nor SecondaryName found
            {
            aPrimaryName = HBufC::NewLC( KNullDesC.iTypeLength );
            *aPrimaryName = KNullDesC;
            aSecondaryName = HBufC::NewLC( KNullDesC.iTypeLength );
            *aSecondaryName = KNullDesC;
            break;
            }
        case 2: // Only PrimaryName found
            {
            aPrimaryName = HBufC::NewLC(( *nameArray)[1].Length() );
            *aPrimaryName = ( *nameArray )[1];
            aSecondaryName = HBufC::NewLC( KNullDesC.iTypeLength );
            *aSecondaryName = KNullDesC;
            break;
            }
        case 3: // Both found
            {
            aPrimaryName = HBufC::NewLC(( *nameArray)[1].Length() );
            *aPrimaryName = ( *nameArray )[1];
            aSecondaryName = HBufC::NewLC( ( *nameArray )[2].Length() );
            *aSecondaryName = ( *nameArray )[2];
            break;
            }
        default: // Something went wrong.
            {
            aPrimaryName = HBufC::NewLC( KNullDesC.iTypeLength );
            *aPrimaryName = KNullDesC;
            aSecondaryName = HBufC::NewLC( KNullDesC.iTypeLength );
            *aSecondaryName = KNullDesC;
            break;
            }
        }

    if ( aSecondaryName )
        {
        CleanupStack::Pop( aSecondaryName ); // aSecondaryName
        }
    if ( aPrimaryName )
        {
        CleanupStack::Pop( aPrimaryName ); // aPrimaryName
        }
    CleanupStack::PopAndDestroy( nameArray ); //  nameArray
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// X509CertNameParser::SubjectFullNameL
// 
// -----------------------------------------------------------------------------

EXPORT_C TInt X509CertNameParser::SubjectFullNameL(
                                      const CX509Certificate& aCertificate,
                                      HBufC*& aSubjectName,
									  const TDesC& aLabel )
    {
    // Get subjectname
    const CX500DistinguishedName& dName = aCertificate.SubjectName();
    // Get subject alternative name
    const CX509CertExtension* extension = 
        aCertificate.Extension( KSubjectAltName );
    
    return GetFullNameL( dName, extension, aSubjectName, aLabel );
    }

// -----------------------------------------------------------------------------
// X509CertNameParser::IssuerFullNameL
//  
// -----------------------------------------------------------------------------
                                                                                
EXPORT_C TInt X509CertNameParser::IssuerFullNameL( 
                                      const CX509Certificate& aCertificate,
                                      HBufC*& aIssuerName )
    {
    // Get issuerName
    const CX500DistinguishedName& dName = aCertificate.IssuerName();
    // Get issuer alternative name
    const CX509CertExtension* extension = 
        aCertificate.Extension( KIssuerAltName );

    return GetFullNameL( dName, extension, aIssuerName );
    }

// -----------------------------------------------------------------------------
// X509CertNameParser::GetFullNameL
// 
// -----------------------------------------------------------------------------
TInt X509CertNameParser::GetFullNameL( 
                              const CX500DistinguishedName& aDN, 
                              const CX509CertExtension* aExtension,
                              HBufC*& aName,
                              const TDesC& aLabel )
    {
    CDesCArrayFlat* nameArray = new( ELeave ) CDesCArrayFlat(3);
    CleanupStack::PushL( nameArray );
    // Add label to array. It is used for comparing.
    nameArray->AppendL( aLabel );

    // Collect name parts to array
    GetNamePartsL( aDN, aExtension, *nameArray, EX509CNPFullName);
    
    // Count length of the aName.
    TInt length = 0;
    TInt i = 1; // Pass label
    for ( ; i < nameArray->MdcaCount(); i++ )
        { 
        length += (*nameArray)[i].Length();
        length += KX509CNPComma.iTypeLength;
        }
    
    // Add elements of the array to aName.
    // First element is label, so it is passed.
    aName = HBufC::NewLC( length );
    for ( i = 1; i < nameArray->MdcaCount(); i++ )
        {
        if ( i != 1 )
            {
            aName->Des().Append( KX509CNPComma );
            }
        aName->Des().Append( (*nameArray)[i] );   
        }

    CleanupStack::Pop( aName ); 
    CleanupStack::PopAndDestroy( nameArray ); 

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// X509CertNameParser::GetNamePartsL
// 
// -----------------------------------------------------------------------------

TInt X509CertNameParser::GetNamePartsL( 
                             const CX500DistinguishedName& aDN, 
                             const CX509CertExtension* aExtension, 
                             CDesCArray& aNameArray, 
                             TX509CNPNameType aNameType)
    {
    const CX509AltNameExt* altNameExt = NULL;
    
    if (aExtension)
        {
        altNameExt = CX509AltNameExt::NewLC( aExtension->Data() ); 
        }

    TBool notFound = ETrue; 
    
    // In case of PrimaryAndSecondaryName loop goes through name elements
    // until two of them is found.
    // In case of FullNames loop goes through all supported name element and
    // adds founded names to array.
    for( TInt round = 1; notFound; round++ )
        {
        switch( round )
            {
            case EDNCommonName:
                {
                GetDNPartL( aDN, KX520CommonName, aNameArray );
                break;
                }
            case ECritAltRFC822Name:
                {
                if (aExtension && aExtension->Critical())
                    {
                    GetAltNamePartL( *altNameExt, EX509RFC822Name, aNameArray );
                    }
                break;
                }
            case EDNOrganizationName:
                {
                GetDNPartL( aDN, KX520OrganizationName, aNameArray);    
                break;  
                }
            case EDNOrganizationalUnitName:
                {
                if ( aNameType == EX509CNPPrimAndSecond )
                    {
                    GetDNPartL( aDN, KX520OrganizationalUnitName, aNameArray);    
                    }
                else
                    {
                    GetDNPartAllL( aDN, KX520OrganizationalUnitName, aNameArray);    
                    }
                break;
                }
            case ECritAltURI:
                {
                if (aExtension && aExtension->Critical())
                    {
                    GetAltNamePartL( *altNameExt, EX509URI, aNameArray );
                    }
                break;
                }
            case ECritAltDNSName:
                {
                if (aExtension && aExtension->Critical())
                    {
                    GetAltNamePartL( *altNameExt, EX509DNSName, aNameArray);
                    }
                break;
                }
            case ECritAltDirectoryName:
                {
                if (aExtension && aExtension->Critical())
                    {
                    GetAltNamePartL( *altNameExt, EX509DirectoryName, 
                                     aNameArray );                                   
                    }
                break;
                }
            case ECritAltIPAddress:
                {
                if (aExtension && aExtension->Critical())
                    {
                    GetAltNamePartL( *altNameExt, EX509IPAddress, 
                                     aNameArray );                                   
                    }
                break;
                } 
            case EAltRFC822Name:
                {
                if (aExtension)
                    {
                    GetAltNamePartL( *altNameExt, EX509RFC822Name, 
                                      aNameArray );
                    }
                break;
                }
            case EAltURI:
                {
                if ( aExtension )
                    {
                    GetAltNamePartL( *altNameExt, EX509URI, aNameArray );
                    }
                break;
                }
            case EAltDNSName:
                {
                if ( aExtension )
                    {
                    GetAltNamePartL( *altNameExt, EX509DNSName, aNameArray );
                    }  
                break;
                }
            case EDNCountryName:
                {
                GetDNPartL( aDN, KX520CountryName, aNameArray );
                break;
                }
            case EDNStateOrProvinceName:
                {
                GetDNPartL( aDN, KX520StateOrProvinceName, aNameArray );
                break;
                }
            case EDNLocalityName:
                {
                GetDNPartL( aDN, KX520LocalityName, aNameArray );
                break;
                }
            case EAltDirectoryName:
                {
                if ( aExtension )
                    {
                    GetAltNamePartL( *altNameExt, EX509DirectoryName, 
                                     aNameArray );
                    }
                break;
                }
            case EAltIPAddress:
                {
                if ( aExtension )
                    {
                    GetAltNamePartL( *altNameExt, EX509IPAddress, 
                                     aNameArray );                                   
                    }
                break;
                }    
            default:
                {
                // All the names are gone through.
                notFound = EFalse;
                break;
                }
            } // switch

        if ( notFound ) 
            {
            // If caller is PrimaryAndSecondaryNameL and PrimaryName and
            // SecondaryName have found then end loop.
            if ( aNameType == EX509CNPPrimAndSecond &&  
               ( aNameArray.MdcaCount() >= KX509CNPPrimAndSecondNameLengh ) )
                {
                notFound = EFalse;    
                } // if
            } // if 
        } // for
    
    if ( aExtension )
        {
        CleanupStack::PopAndDestroy(); // altNameExt
        }
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// X509CertNameParser::GetDNPartAllL
// 
// -----------------------------------------------------------------------------

void X509CertNameParser::GetDNPartAllL(
                             const CX500DistinguishedName& aDN, 
                             const TDesC& aSubPartOID, 
                             CDesCArray& aNameArray )
    {
    HBufC* buf = NULL;
    // Count of the name parts of the DN
    TInt count = aDN.Count();

    // This goes through all the nameparts of the DN and appends all the
    // parts, that OID is aSubPartOID, to the array.
	for ( TInt i = 0; i < count; i++ )
		{
		const CX520AttributeTypeAndValue& ava = aDN.Element( i );
		if ( ava.Type() == aSubPartOID )
			{
			TRAPD( error, buf = ava.ValueL() );
            HandleErrorL( error );
            
            if ( buf )
                {
                CleanupStack::PushL( buf );
                CompareAndAddNameL( *buf, aNameArray );
                CleanupStack::PopAndDestroy( buf ); // buf
                buf = NULL; 
                } // if 
            } // if
        } // for
    }

// -----------------------------------------------------------------------------
// X509CertNameParser::GetDNPartL
// 
// -----------------------------------------------------------------------------

void X509CertNameParser::GetDNPartL( 
                             const CX500DistinguishedName& aDN, 
                             const TDesC& aSubPartOID, 
                             CDesCArray& aNameArray )
    {
    HBufC* buf = NULL;
    // Extract one DN part
    TRAPD( error, buf = aDN.ExtractFieldL( aSubPartOID ) );
    HandleErrorL( error );

    // Add to array
    if ( buf )
        {
        CleanupStack::PushL( buf );
        CompareAndAddNameL( *buf, aNameArray );
        CleanupStack::PopAndDestroy( buf );  // buf
        }
    }

// -----------------------------------------------------------------------------
// X509CertNameParser::GetAltNamePartL
// 
// -----------------------------------------------------------------------------

void X509CertNameParser::GetAltNamePartL( 
                             const CX509AltNameExt& aAltName, 
                             const TGNType& aSubPartType, 
                             CDesCArray& aNameArray )
    {
    // Get all parts of the alternative name to the array
    const CArrayPtrFlat<CX509GeneralName>& subParts = aAltName.AltName();
    TInt count = subParts.Count();
    CX509DomainName* domainName = NULL;
    TInt error = KErrNone;  

    // This goes through all the nameparts of the DN and appends all the
    // parts, that OID is aSubPartOID, to the array.
    for (TInt i = 0; count > i; i++)
        {
        if ( subParts[i]->Tag() == aSubPartType )
            {
            TPtrC8 data = subParts[i]->Data(); 
            switch ( aSubPartType )
                {
                case EX509RFC822Name:
                    {
                    TRAP( error, domainName = CX509RFC822Name::NewL( data ) );
                    CleanupStack::PushL( domainName );
                    AddAltNameToArrayL( error, domainName, aNameArray );
                    CleanupStack::PopAndDestroy( domainName );     
                    break;
                    }
                case EX509DNSName:
                case EX509DirectoryName:
                    {
                    TRAP( error, domainName = CX509DNSName::NewL( data ) );
                    CleanupStack::PushL( domainName );
                    AddAltNameToArrayL( error, domainName, aNameArray );
                    CleanupStack::PopAndDestroy( domainName );
                    break;
                    }
                case EX509URI:
                    {
                    CX509IPBasedURI* uri = NULL;
                    HBufC* decodedUri = NULL;
                    TRAP( error, uri = CX509IPBasedURI::NewL( data ) );
                    
                    if ( error == KErrNone )
                        {
                        CleanupStack::PushL( uri );
                        TRAP( error, decodedUri = EscapeUtils::
                            EscapeDecodeL( uri->Name() ) );
                        CleanupStack::PopAndDestroy(); //uri
                        }
                    
                    HandleErrorL( error );
                    if ( error == KErrNone )
                        {
                        CleanupStack::PushL( decodedUri );
                        CompareAndAddNameL( *decodedUri, aNameArray );
                        CleanupStack::PopAndDestroy(); // decodedUri
                        }
                    break;
                    }
                case EX509IPAddress:
                    {
                    // Not supported
                    break;
                    }
                default:
                    {
                    break;
                    }
                } // switch
            } // if 
        } // for
    }

// -----------------------------------------------------------------------------
// X509CertNameParser::AddAltNameToArrayL
// 
// -----------------------------------------------------------------------------

TInt X509CertNameParser::AddAltNameToArrayL( 
                            TInt aError, 
                            CX509DomainName* aDomainName, 
                            CDesCArray& aNameArray )
    {
    TInt error = HandleErrorL( aError );
    
    if ( error == KErrNone )
        {
        const TPtrC name = aDomainName->Name();
        CompareAndAddNameL( name, aNameArray );
        }

    return error;
    }

// -----------------------------------------------------------------------------
// X509CertNameParser::AddToBufLD
// 
// -----------------------------------------------------------------------------

void X509CertNameParser::AddToBufLD( const TDesC& aName, HBufC*& aBuf )
    {
    if (!aBuf)
        {
        aBuf = HBufC::NewL( aName.Length() );
        aBuf->Des().Append( aName );
        }
    else
        {
        HBufC* helpBuf = HBufC::NewL( aName.Length() + aBuf->Length() + 1);
        helpBuf->Des().Append( *aBuf );
        helpBuf->Des().Append( KX509CertParserSpace );
        helpBuf->Des().Append( aName );
        CleanupStack::PopAndDestroy( aBuf ); // aBuf
        aBuf = helpBuf;
        }
    }

// -----------------------------------------------------------------------------
// X509CertNameParser::CompareAndAddNameL
// 
// -----------------------------------------------------------------------------

void X509CertNameParser::CompareAndAddNameL( const TDesC& aProposal, 
                                             CDesCArray& aNameArray )
    {
    TInt pos = 0; // This is needed only for calling Find().
    // Append to array, if aProposal don't already exist in the array.
    if ( aNameArray.Find( aProposal, pos, ECmpNormal ) )
        {
        aNameArray.AppendL( aProposal );
        }
    }



// -----------------------------------------------------------------------------
// X509CertNameParser::HandleErrorL
// 
// -----------------------------------------------------------------------------

TInt X509CertNameParser::HandleErrorL( TInt aError )
    {    
    if ( !( aError == KErrNone || aError == KErrArgument || aError == KErrNotSupported ) )
        {
        User::Leave( aError );
        }       
    return aError;
    }
    
//  End of File  
