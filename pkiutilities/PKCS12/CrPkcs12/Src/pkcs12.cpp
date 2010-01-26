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
* Description:   Implementation of the PKCS#12 parser API
*
*/


// INCLUDE FILES
#include <securityerr.h>
#include    "pkcs12.h"
#include    "crpkcs12.h"
//#include    <?include_file>

// EXTERNAL DATA STRUCTURES
//extern  ?external_data;

// EXTERNAL FUNCTION PROTOTYPES  
//extern ?external_function( ?arg_type,?arg_type );

// CONSTANTS

// ASN.1 tag values
const TInt KASN1SequenceTagValue = 0x30;
const TInt KASN1ImplicitConstructedTagValue = 0xA0;   // Implicit constructed tag
const TInt KASN1OctetStringTagValue   = 0x04;     // Octet string tag
const TInt KASN1BerConstructedBit = 0x20;

_LIT8(KPKCS12Version3, "\x02\x01\x03");
_LIT8(KPKCS7DataOID, "\x06\x09\x2A\x86\x48\x86\xF7\x0D\x01\x07\x01");
_LIT8(KPKC7EncryptedDataOID, "\x06\x09\x2A\x86\x48\x86\xF7\x0D\x01\x07\x06");
_LIT8(KSequenceTagInOctet,"\x04\x01\x30\x04\x01\x80");
_LIT8(KMozillaPKCS7EncryptedDataOID, 
    "\x04\x01\x06\x04\x01\x09\x04\x09\x2a\x86\x48\x86\xf7\x0d\x01\x07\x06");
_LIT8(KMozillaPKCS7DataOID, 
    "\x04\x01\x06\x04\x01\x09\x04\x09\x2a\x86\x48\x86\xf7\x0d\x01\x07\x01");

// MACROS
//#define ?macro ?macro_def

// LOCAL CONSTANTS AND MACROS
//const ?type ?constant_var = ?constant;
//#define ?macro_name ?macro_def

// MODULE DATA STRUCTURES
//enum ?declaration
//typedef ?declaration

// LOCAL FUNCTION PROTOTYPES
//?type ?function_name( ?arg_type, ?arg_type );

// FORWARD DECLARATIONS
//class ?FORWARD_CLASSNAME;

// ============================ MEMBER FUNCTIONS ===============================

// ---------------------------------------------------------
// PKCS12Factory::CreateL
// ---------------------------------------------------------
//
EXPORT_C MPKCS12* PKCS12Factory::CreateL()
    {
    return CPKCS12::NewL();
    }

// -----------------------------------------------------------------------------
// CPKCS12::CPKCS12
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CPKCS12::CPKCS12()
    {
    }

// -----------------------------------------------------------------------------
// CPKCS12::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CPKCS12::ConstructL()
    {        
    
    }

// -----------------------------------------------------------------------------
// CPKCS12::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CPKCS12* CPKCS12::NewL()
    {
    CPKCS12* self = new( ELeave ) CPKCS12();
    
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();

    return self;
    }

void CPKCS12::Release()
    {
    delete this;
    }

// ---------------------------------------------------------
// CPKCS12::IsPKCS12Data
// ---------------------------------------------------------
//    
TBool CPKCS12::IsPKCS12Data(const TDesC8& aBinaryData)
     {     
    // We don't decode the data because we may only have the first few bytes -
	// instead we check the ASN1 by hand.
	ASSERT(aBinaryData.Length() >= KPKCS12DataMinLength);
	TInt pos = 0;
	
	if (!IsASN1Tag(KASN1SequenceTagValue,aBinaryData, pos))
	    {
	    return EFalse;
	    }
	if (!IsExpectedData(aBinaryData, pos, KPKCS12Version3))
	    {
	    return EFalse;
	    }
    if (!IsASN1Tag(KASN1SequenceTagValue, aBinaryData, pos))
	    {
	    return EFalse;
	    } 
	if (!IsExpectedData(aBinaryData, pos, KPKCS7DataOID))
	    {
	    return EFalse;
	    } 
    if (!IsASN1Tag(KASN1ImplicitConstructedTagValue, aBinaryData, pos))
	    {
	    return EFalse;
	    } 
	// OpenSSL, IE and Opera type of PKCS#12 file    
    if (IsASN1Tag(KASN1OctetStringTagValue, aBinaryData, pos))
        {
        if (!IsASN1Tag(KASN1SequenceTagValue, aBinaryData, pos))
            {
            return EFalse;
            }
        if (!IsASN1Tag(KASN1SequenceTagValue, aBinaryData, pos))
            {
            return EFalse;
            }    
    	if  (IsExpectedData(aBinaryData, pos, KPKC7EncryptedDataOID) || 
    	     IsExpectedData(aBinaryData, pos, KPKCS7DataOID))    		
    	    {    	    
    	    return ETrue;
    	    }
    	else
    	    {
    	    return EFalse;
    	    }
        }
    // Netscape and Mozilla type of PKCS#12 file    
	else if (IsASN1Tag(KASN1OctetStringTagValue + KASN1BerConstructedBit, aBinaryData, pos))
	    {
	    if (IsExpectedData(aBinaryData, pos, KSequenceTagInOctet))
            {
            if (!IsExpectedData(aBinaryData, pos, KSequenceTagInOctet))
                {
                return EFalse;
                }    
        	if  (IsExpectedData(aBinaryData, pos, KMozillaPKCS7DataOID) ||
        		IsExpectedData(aBinaryData, pos, KMozillaPKCS7EncryptedDataOID))
        	    {    	    
        	    return ETrue;
        	    }
            }
        // Firefox version 1.5 type of PKCS#12 data    
        else if (IsASN1Tag(KASN1OctetStringTagValue, aBinaryData, pos))
            {
            if (!IsASN1Tag(KASN1SequenceTagValue, aBinaryData, pos))
                {
                return EFalse;
                }
            if (!IsASN1Tag(KASN1SequenceTagValue, aBinaryData, pos))
                {
                return EFalse;
                }    
    	    if (IsExpectedData(aBinaryData, pos, KPKC7EncryptedDataOID) || 
    	     IsExpectedData(aBinaryData, pos, KPKCS7DataOID))    		
    	        {    	    
    	        return ETrue;
    	        }
    	    else
    	        {
    	        return EFalse;
    	        }
            }
        else
            {
            return EFalse;
            }            
	    }
    else
        {            
        return EFalse;
        }
    return EFalse;    
    }
    
// ---------------------------------------------------------
// CPKCS12Recognizer::IsASN1Tag
// ---------------------------------------------------------
//  
TBool CPKCS12::IsASN1Tag(
    TInt aTag, 
    const TDesC8& aBinaryData, 
    TInt& aPos)
	{	
	// Check we have enough data
	if ((aPos + 2) >= aBinaryData.Length())
		{
		return EFalse;
		}
	// Check the outermost sequence is valid
	if (aBinaryData[aPos] != aTag)
		{
		return EFalse;
		}
	// Skip sequence length
	aPos++;
	TInt length0 = aBinaryData[aPos++];
	if (length0 & 0x80)
		{
		aPos += length0 & 0x7f;
		}
	return ETrue;
	}

// ---------------------------------------------------------
// CPKCS12Recognizer::IsExpectedData
// ---------------------------------------------------------
//  
TBool CPKCS12::IsExpectedData(
    const TDesC8& aBinaryData, 
    TInt& aPos, 
    const TDesC8& aExpectedData)
	{	
	TInt length = aExpectedData.Length();
	// Check we have enough data
	if (aPos + length >= aBinaryData.Length())
		{
		return EFalse;
		}
	// Check data matches	
	if (aBinaryData.Mid(aPos, length) != aExpectedData)
		{
		return EFalse;
		}
	aPos += length;
	return ETrue;
	}    

// -----------------------------------------------------------------------------
// CPKCS12::ParseL
// 
// -----------------------------------------------------------------------------
//
void CPKCS12::ParseL(const TDesC8& aPKCS12, const TDesC16& aPassword)
    {
    Reset();
    iCrPkcs12 = CCrPKCS12::NewL();
    iCrData = CCrData::NewL(EFalse);
    User::LeaveIfError(iCrData->Write(aPKCS12));
    TInt start = 0;
    iCrData->Seek( ESeekStart, start );
    HandleErrorL( iCrPkcs12->OpenL( *iCrData, aPassword ));
    }

// -----------------------------------------------------------------------------
// CPKCS12::HandleErrorL
// 
// -----------------------------------------------------------------------------
//
void CPKCS12::HandleErrorL( TUint16 aError ) const
    {

    switch ( aError & 0x00ff )
        {
        case KCrOK:
            return;
        case KCrNotValidPkcs12Object:
            User::Leave(KErrArgument);
        case KCrBerLibraryError:
            User::Leave(KErrGeneral);
        case KCrNotSupportedHMACalgorithm:
            User::Leave(KErrNotSupported);
        case KCrCancelled:
            User::Leave(KErrCancel);
        case KCrGeneralError:
            User::Leave(KErrGeneral);
        case KCrWrongPassWordOrCorruptedFile:
            User::Leave(KErrBadPassphrase);
        case KCrNotPasswordBasedEncryption:
            User::Leave(KErrNotSupported);
        case KCrNoMemory:
            User::Leave(KErrNoMemory);
        default:
            User::Leave(KErrGeneral);
        }
    }
    
// Destructor
CPKCS12::~CPKCS12()
    {
    Reset();
    }

// -----------------------------------------------------------------------------
// CPKCS12::Reset
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CPKCS12::Reset()
    {
    if (iCrData)
        {        
        iCrData->Close();
        delete iCrData;
        iCrData = NULL;
        }    
    delete iCrPkcs12;
    iCrPkcs12 = NULL;
    }

// -----------------------------------------------------------------------------
// CPKCS12::CACertificates
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
const CArrayPtr<CX509Certificate>& CPKCS12::CACertificates() const
    {
    return iCrPkcs12->CACertificates();
    }

// -----------------------------------------------------------------------------
// CPKCS12::UserCertificates
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
const CArrayPtr<CX509Certificate>& CPKCS12::UserCertificates() const
    {    
    return iCrPkcs12->UserCertificates();
    }

// -----------------------------------------------------------------------------
// CPKCS12::PrivateKeys
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
const CArrayPtr<HBufC8>& CPKCS12::PrivateKeys() const
    {    
    return iCrPkcs12->PrivateKeys();
    }

// -----------------------------------------------------------------------------
// CPKCS12::SafeBagsCount
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TUint CPKCS12::SafeBagsCount() const
    {
    return iCrPkcs12->SafeBagsCount();
    }

// -----------------------------------------------------------------------------
// CPKCS12::IterCount
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//     
TUint CPKCS12::IterCount() const
    {
    return iCrPkcs12->Iter();
    }

// ========================== OTHER EXPORTED FUNCTIONS =========================
                                                                                
// ---------------------------------------------------------
// E32Dll
// DLL entry point
// ---------------------------------------------------------
//
#ifndef EKA2
GLDEF_C TInt E32Dll(TDllReason /*aReason*/)
    {
    return KErrNone;
    }
#endif

//  End of File  
