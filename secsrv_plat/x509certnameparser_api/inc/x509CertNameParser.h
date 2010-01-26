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
* Description:  Extracts relevant certificate information to be shown to user.
*
*/

																				

#ifndef X509CERTNAMEPARSER_H
#define X509CERTNAMEPARSER_H

//  INCLUDES
#include <e32std.h> 
#include <x509gn.h> // TGNType
//------------------------------------------------------------------------------
// CONSTANTS
_LIT(KX509CNPComma,", ");

// FORWARD DECLARATIONS
class CX509Certificate;  
class CX500DistinguishedName;
class CX509CertExtension;
class CX509AltNameExt;


//------------------------------------------------------------------------------
//
// CLASS DECLARATION

/**
*  Static class CX509CertNameParser provides functions that extract relevant
*  information for user from x.509 certificate. CX509CertNameParser 
*  extracts the most important name info and tries to guarantee  uniqueness 
*  in a list of certificates. 
*  @lib X509CertNameParser.dll
*  @since Series60_2.0
*/
class X509CertNameParser  
	{
    public: // Constructors and destructor
        

    public: // New functions
																				        
	    /**
        * PrimaryAndSecondaryNameL_description. PrimaryAndSecondaryNameL 
        * member function is used to collect Primary and Secondsry Name data of 
        * certificate information. The first found non-empty naming element and 
        * not equal with label  is Primary Name.
        * The second available information element not equal with label is 
        * Secondary Name.
        * @since Series60_2.0
        * @param aCertificate Certificate where the information is extracted.
		* @param aPrimaryName IN null pointer, OUT first found name element 
        * according to name element order or KNulDesC if Primary Name isn't 
        * found.
        * @param aSecondaryName IN null pointer, OUT second found name element 
        * according to name element order or KNullDesC if Secondary Name isn't found.
        * @param aLabel Label of the certificate. Found elements are compared to
        * label to make sure that aPrimaryName and aSecondaryName differ from 
        * label. If label doesn't exist, this can be left away.
		* @return status, at the moment always KErrNone
        */                                                                              
        IMPORT_C static TInt PrimaryAndSecondaryNameL( 
                                const CX509Certificate& aCertificate, 
                                HBufC*& aPrimaryName, 
                                HBufC*& aSecondaryName,
                                const TDesC& aLabel =KNullDesC );
		
		/**
        * SubjectFullNameL_description. SubjectFullNameL member function is 
        * used to collect all relevant subject information from the subject 
        * and from the subject alternative name extension of the certificate.
        * @since Series60_2.0
        * @param aCertificate Certificate where the information is extracted.
        * @param aSubjectName IN null pointer, OUT Full subject name of the 
        * certificate or KNullDesC if nothing is found.
		* @param aLabel Label of the certificate. Found elements are compared to
        * label to make sure that aPrimaryName and aSecondaryName differ from 
        * label. If label doesn't exist, this can be left away.
		* @return status, at the moment always KErrNone
        */
        IMPORT_C static TInt SubjectFullNameL( 
                                const CX509Certificate& aCertificate,
                                HBufC*& aSubjectName,
			                    const TDesC& aLabel = KNullDesC );

		/**
        * IssuerFullNameL_description. IssuerFullNameL member function is 
        * used to collect all the relevant Issuer information from the Issuer 
        * and from the Issuer alternative name extension of the certificate.
        * @since Series60_2.0
        * @param aCertificate Certificate where the information is extracted.
        * @param aIssuerName IN null pointer, OUT Full issuer name of the 
        * certificate or KNullDesC if nothing is found..
		* @return status, at the moment always KErrNone
        */
		IMPORT_C static TInt IssuerFullNameL( 
                                const CX509Certificate& aCertificate,
                                HBufC*& aIssuerName );
	
    public: // Functions from base classes
       
    protected:  // New functions
        
    protected:  // Functions from base classes

    private: //Private data types

	/**
    * enum TX509CNPNameType declaration.
	*
    */
    enum TX509CNPNameType
		{	
        EX509CNPPrimAndSecond,
        EX509CNPFullName
		};

    /**
    * enum TX509CNPNameElement declaration.
	* This enum defines the naming element order that can be changed by 
    * chancing the order of enum's members.
    */
    enum TX509CNPNameElement
        {                           // DN = Distinguish name, Alt = Alternative name 
        EDNCommonName = 1,          // common name of the subject 
        ECritAltRFC822Name,         // rfc822Name of the critical subject alternative name
        EDNOrganizationName,        // OrganizationName of the subject
        EDNOrganizationalUnitName,  // OrganizationalUnitName of the subject
        ECritAltDNSName,            // dNSName of the critical subject alternative name
        ECritAltURI,                // URI of the critical subject alternative name
        ECritAltDirectoryName,      // directoryName of the critical subject alternative name
        ECritAltIPAddress,          // iPAddress of the critical subject alternative name                       
        EAltRFC822Name,             // rfc822Name of the subject alternative name
        EAltDNSName,                // dNSName of the subject alternative name
        EAltURI,                    // URI of the subject alternative name
        EDNCountryName,             // countryName of the subject
        EDNStateOrProvinceName,     // stateOrProvinceName of the subject
        EDNLocalityName,            // localityName of the subject
        EAltDirectoryName,          // directoryName of the subject alternative name
        EAltIPAddress               // iPAddress of the subject alternative name                       
        };  
       
    private:

        /**
        * C++ default constructor.
        */
        X509CertNameParser();

	
    private: //New functions
        
        /**
        * GetFullNameL_description.
		* Common function for FullName functions.
        * @since Series60_2.0
        * @param aDN Distinguished name
        * @param aExtension Pointer to Extension
        * @param aSubjectName IN null pointer, OUT Full subject name of the 
        * certificate or KNullDesC if nothing is found.
        * @return TInt type value.
        */
        static TInt GetFullNameL( 
                        const CX500DistinguishedName& aDN, 
                        const CX509CertExtension* aExtension,
                        HBufC*& aName,
                        const TDesC& aLabel = KNullDesC );


        /**
        * GetNamePartsL_description.
		* Collects all the name parts of the certificate to array. One name element
        * is placed to one element of array.
        * @since Series60_2.0
        * @param aDN Distinguished name
        * @param aExtension Pointer to Extension
        * @param aNameArray Found name elements are appended to this array.
        * @param aNameType EX509CNPPrimAndSecond or EX509CNPFullName
        * @return TInt type value.
        */
        static TInt GetNamePartsL( 
                        const CX500DistinguishedName& aDN, 
                        const CX509CertExtension* aExtension, 
                        CDesCArray& aNameArray, 
                        TX509CNPNameType aNameType );
                                                                                                                                           
        /**
        * GetDNPartL_description.
		* Gets one name element of the DN identified by OID.
        * @since Series60_2.0
        * @param aDN Distinguished name
        * @param aSubPartOID OID of the distinguished name subpart 
        * @param aNameArray Found name element is appended to this array.
        * @return void
        */
        static void GetDNPartL( 
                        const CX500DistinguishedName& aDN, 
                        const TDesC& aSubPartOID, 
                        CDesCArray& aNameArray );

        /**
        * GetDNPartAllL_description.
		* Gets all name elements of the DN identified by one OID.
        * @since Series60_2.0
        * @param aDN Distinguished name
        * @param aSubPartOID OID of the distinguished name subpart 
        * @param aNameArray Found name elements are appended to this array.
        * @return void
        */
        static void GetDNPartAllL( 
                        const CX500DistinguishedName& aDN, 
                        const TDesC& aSubPartOID, 
                        CDesCArray& aNameArray );

        /**
        * GetAltNamePartL_description.
		* Gets all elements of alternative name identified by one OID.
        * @since Series60_2.0
        * @param aAltName Alternative name extension
        * @param aSubPartType Type of the alternative name subpart
        * @param aNameArray Found name elements are appended to this array.
        * @return void
        */
        static void GetAltNamePartL( 
                        const CX509AltNameExt& aAltName, 
                        const TGNType& aSubPartType, 
                        CDesCArray& aNameArray );

        /**
        * HandleAltNameL_description.
        * Handles error. If error is KErrNone, KErrArgument or KErrNotSupported, it just
        * returns aError, otherwise it leaves with aError.
        * @since Series60_2.0
        * @param aError Error
        * @return TInt aError
        */    
        static TInt AddAltNameToArrayL( 
                        TInt aError, 
                        CX509DomainName* aDomainName,
                        CDesCArray& aNameArray );

        /**
        * CompareAndAddNameL_description.
		* Adds proposal to array if none of the elements of array isn't equal 
        * with proposal.
        * @since Series60_2.0
        * @param aProposal 
        * @param aNameArray 
        * @return void.
        */
        static void CompareAndAddNameL( 
                        const TDesC& aProposal, 
                        CDesCArray& aNameArray );

        /**
        * AddToBufLC_description.
		* Appends aName to aBuf. If aBuf isn't NULL, first aBuf is deleted and
        * memory is allocated for existing data and for aName.
        * @since Series60_2.0
        * @param aName Desciptor that is appended to aBuf
        * @param aBuf  IN Null pointer or buffer
        * @return void.
        */
        static void AddToBufLD( const TDesC& aName, HBufC*& aBuf );

        /**
        * HandleErrorL_description.
        * Handles error. If error is KErrNone, KErrArgument or KErrNotSupported, it just
        * returns aError, otherwise it leaves with aError.
        * @since Series60_2.0
        * @param aError Error
        * @return TInt aError
        */    
        static TInt HandleErrorL( TInt aError );

    };

#endif      // X509CERTNAMEPARSER_H
            
// End of File
