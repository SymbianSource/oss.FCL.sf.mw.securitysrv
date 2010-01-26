// Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
// Define generic methods which can be used accross the ocsp component.
// 
//

#include <asymmetrickeys.h>
#include <ocsp.h>
#include "oids.h"
#include <x509certext.h>
#include "ocsprequestandresponse.h"

namespace OCSPUtils
	{
	/**
		Checks whether the supplied certificate matches the responder
		ID in the supplied response.  This implements S3.2.3 of RFC2560.
		Note this does <em>not</em> mean the response is signed by the
		certificate's subject.  It just checks that the certificate's
		name, or a hash of its key, is present in the response's plaintext.
		
		@param	aResponse		Response whose responder ID should describe
								<var>aCert</var>.
		@param	aCert			Certificate whose name or key should be
								in the response.
		@return ETrue if the response's responder ID matches <var>aCert</var>,
			EFalse otherwise.
	 */
	TBool DoesResponderIdMatchCertL( const COCSPResponse& aResponse, 
												const CX509Certificate& aResponderCert)
		{	
		return (DoesIssuerKeyMatchL(aResponse,aResponderCert) || DoesDNNameMatchL(aResponse,aResponderCert));
		}
	
	TBool DoesDNNameMatchL( 	const COCSPResponse& aResponse, const CX509Certificate& aCert)
		{
		TBool result = EFalse;
		const TPtrC8* nameData = aResponse.DataElementEncoding(COCSPResponse::EResponderIDName);
		if (nameData)
			{
			CX500DistinguishedName* name = CX500DistinguishedName::NewLC(*nameData);
			if (aCert.SubjectName().ExactMatchL(*name))
				{
				result = ETrue;
				}
			CleanupStack::PopAndDestroy(name);
			}
		return result;
		}
	
	TBool DoesIssuerKeyMatchL(const COCSPResponse& aResponse, const CX509Certificate& aCert)
		{
		TBool result = EFalse;
		// check the responder's issuer key and verify that it is the same as the issuer cert
		const TPtrC8* keyHash = aResponse.DataElementEncoding(COCSPResponse::EResponderIDKeyHash);
		const TPtrC8* keyEncoding = aCert.DataElementEncoding(CX509Certificate::ESubjectPublicKeyInfo);
		if (!keyHash || !keyEncoding)
			{
			return result;
			}
		
		TASN1DecSequence decSeq;
		TInt pos = 0;
		CArrayPtrFlat<TASN1DecGeneric>* seq = decSeq.DecodeDERLC(*keyEncoding, pos, 2, KMaxTInt);
	
		TASN1DecBitString decBitStr;
		HBufC8* toHashData = decBitStr.ExtractOctetStringL(*seq->At(1));
		CleanupStack::PushL(toHashData);
		
		CSHA1* hashAlg = CSHA1::NewL();
		CleanupStack::PushL(hashAlg);
		TPtrC8 certKeyHash = hashAlg->Hash(*toHashData);
		
		if (certKeyHash == *keyHash)
			{
			// Public key hashes match
			result = ETrue;
			}
		CleanupStack::PopAndDestroy(3, seq); // hashAlg, toHashData, seq
			
		return result;
		}
	
	TBool IsResponseSignedByCertL(
		COCSPResponse* aResponse, const CX509Certificate& aCert)
	/**
		Checks whether the supplied response is signed by the supplied
		certificate.
		
		@param	aResponse		Response which should be signed by aCert.
		@param	aCert			Candidate certificate which may have been
								used to sign <var>aResponse</var>.
		@return ETrue if <var>aCert</var> is used to sign <var>aResponse</var>,
			EFalse otherwise.  Note <code>COCSPResponse</code> derives
			from <code>CSignedObject</code>.
	 */
		{
		if (aCert.PublicKey().AlgorithmId() == EDSA)
			{
			TX509KeyFactory factory;
			CDSAParameters* theDSAParams = factory.DSAParametersL(aCert.PublicKey().EncodedParams());
			CleanupStack::PushL(theDSAParams);
			
			CSigningKeyParameters* params = CSigningKeyParameters::NewLC();
			params->SetDSAParamsL(*theDSAParams);
					
			aResponse->SetParametersL(*params);
			CleanupStack::PopAndDestroy(2, theDSAParams);
			}
		
		return aResponse->VerifySignatureL(aCert.PublicKey().KeyData());
		}
	
	TBool DoesCertHaveOCSPNoCheckExt(const CX509Certificate& aCert)
	/**
		Test whether the supplied certificate has id-pkix-ocsp-nocheck
		in an extendedKeyUsage extension.  
		
		@param	aCert		Certificate to test for id-kp-OCSPSigning.
								
		@return				ETrue if the supplied certificate has an
							extendedKeyUsage extension which contains
							id-pkix-ocsp-nocheck.
	 */
		{
		return aCert.Extension(KOCSPOidNoCheck) != NULL;
		}
	
	/**
	 * Determine the URI of the OCSP server to use for a certificate.  Checks to see
	 * if there is an authority info access extension containing an access
	 * description with method oid id-ad-ocsp. If so, it copies the access
	 * description into a new descriptor which it stores in iURI.  Otherwise it
	 * returns iDefaultURI, which may be null.
	 */
	
	TDesC8* ServerUriL(const CX509Certificate& aCert, const COCSPParameters* aParameters )
		{
		
		TDesC8* uri = NULL;
		
		// check whether AIA is present or not:
		// if present and UseAIA is enabled the get the AIA extension.
		// if AIA is absent the else if part will handle it
		// if present and UseAIA is disabled, means that iUseGlobalOCSPUri is enabled,
		// do not leave with an error in this case the next case will handle this condition.
		if( aParameters->UseAIA() && OCSPUtils::IsAIAForOCSPPresentL(aCert) )
			{
			uri = OCSPUtils::GetAIAL(aCert);
			}
		else if( aParameters->DefaultURI() != KNullDesC8())
			{
			uri = aParameters->DefaultURI().AllocL();
			}
		// the extension is absent and iUseGlobalOCSPUri is disabled, it has reached this case
		// where neither is there an AIA in the cert nor is the global ocsp responder available,
		// this should leave with EInvalidURI but this has been done to maintain backward compatibility.
		else
			{
			User::Leave(KErrArgument);
			}
		return uri;
		}
	
	// check whether AIA is present or not:
	// if present and UseAIA is enabled then return true.
	// if default uri is not null return true.
	TBool IsUriAvailableL(const CX509Certificate& aCert, const COCSPParameters* aParameters )
		{
		return  ( aParameters->UseAIA() && OCSPUtils::IsAIAForOCSPPresentL(aCert) ) || 
				 ( aParameters->DefaultURI() != KNullDesC8() ) ;
		}
	
	TBool IsAIAForOCSPPresentL(const CX509Certificate& aCert)
		{
		TBool found = EFalse;
		const CX509CertExtension* ext = aCert.Extension(KAuthorityInfoAccess);
		
		if(ext)
			{
			CX509AuthInfoAccessExt* auth = CX509AuthInfoAccessExt::NewLC(ext->Data());
			for (TInt i = 0 ; i < auth->AccessDescriptions().Count() ; ++i)
				{
				const CX509AccessDescription* desc = auth->AccessDescriptions()[i];
				if (desc->Method() == KAccessMethodOCSP && desc->Location().Tag() == EX509URI )
					{
					found = ETrue;
					break;
					}
				} // end of for
			CleanupStack::PopAndDestroy(auth);
			}
		return found;
		}
	
	TDesC8* GetAIAL(const CX509Certificate& aCert)
		{
		TDesC8*	uri = NULL;
		
		const CX509CertExtension* ext = aCert.Extension(KAuthorityInfoAccess);
		
		if (ext)
			{
			CX509AuthInfoAccessExt* auth = CX509AuthInfoAccessExt::NewLC(ext->Data());
			for (TInt i = 0 ; i < auth->AccessDescriptions().Count() ; ++i)
				{
				const CX509AccessDescription* desc = auth->AccessDescriptions()[i];
				if (desc->Method() == KAccessMethodOCSP)
					{
					const CX509GeneralName& gn = desc->Location();
					if (gn.Tag() == EX509URI)
						{
						// Decode the general name rather than using CX509IPBaseURI, 
						// as this currently rejects some URIs
						TASN1DecIA5String encStr;
						TInt pos = 0;
						HBufC* suri = encStr.DecodeDERL(gn.Data(), pos);
						CleanupStack::PushL(suri);
						HBufC8* buf = HBufC8::NewL(suri->Length());
						uri = buf;
						buf->Des().Copy(*suri);
						CleanupStack::PopAndDestroy(suri);
						break;
						}
					}
				}
			CleanupStack::PopAndDestroy(auth);
			}
		return uri;
		}
	
	/**
		Test whether the supplied certificate has id-kp-OCSPSigning
		in an extendedKeyUsage extension.  This tests whether the
		certificate is a trusted responder, as defined in RFC 2560 S4.2.2.2.
			
		@param	aCert			Certificate to test for id-kp-OCSPSigning.
								This should be the certificate which was
								immediately signed by the CA which issued
								the certificate in question.
		@return					ETrue iff the supplied certificate has an
								extendedKeyUsage extension which contains
								id-kp-OCSPSigning.
	*/
		
	TBool DoesCertHaveOCSPSigningExtL(const CX509Certificate& aCert)
		{
		TBool found = EFalse;
		// check has extended key usage
		const CX509CertExtension* ekuExt = aCert.Extension(KExtendedKeyUsage);
		if (ekuExt != NULL)
			{	
			// get set of extended key usages
			const TDesC8& extData = ekuExt->Data();
			CX509ExtendedKeyUsageExt* extUses = CX509ExtendedKeyUsageExt::NewLC(extData);
			
			// check if id-kp-OCSPSigning is one of the uses
			const CArrayPtrFlat<HBufC>& usages = extUses->KeyUsages();
			TInt usageCount = usages.Count();
			for (TInt i = 0; i < usageCount; ++i)
				{
				if (*usages[i] == KOCSPOidOCSPSigning)
					{
					found = ETrue;
					break;
					}
				}
			CleanupStack::PopAndDestroy(extUses);
			}
		return found;
		}
	
	
	CX509Certificate* GetResponderCertLC(const TDesC8& aEncodedCerts)
		{
		// This specifies the position of the certificate which is to be retrieved from
		// the encoded certificate chain. As responder certificate would always be the 
		// end entity certificate, here we are retrieving the first one from the chain.
		TInt pos = 0;
		return CX509Certificate::NewLC(aEncodedCerts, pos);
		}
	} // OCSPUtils namespace
