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
* Description:   Recogniser of application/x-pkcs12 mime type
*
*/



// INCLUDE FILES
#include "PKCS12Recognizer.h"
#include <apmrec.h>
#include <apmstd.h>
#include <e32svr.h>
#include <ecom/implementationproxy.h>
#include <mpkcs12.h>

_LIT8(KPKCS12MimeType, "application/x-pkcs12");
const TInt KMimeTypesSupported(1);
const TUid KUidMimePKCS12Recognizer = { 0x101F8714 };

// ---------------------------------------------------------
// CPKCS12Recognizer::CPKCS12Recognizer
// ---------------------------------------------------------
//
CPKCS12Recognizer::CPKCS12Recognizer()
    :CApaDataRecognizerType(
    KUidMimePKCS12Recognizer, 
        CApaDataRecognizerType::ELow )
    {
    iCountDataTypes = KMimeTypesSupported;
    #ifdef _DEBUG
    RDebug::Print(_L("CPKCS12Recognizer constructed"));
    #endif
    }

// ---------------------------------------------------------
// CPKCS12Recognizer::~CPKCS12Recognizer()
// ---------------------------------------------------------
//
CPKCS12Recognizer::~CPKCS12Recognizer()
    {
    if (iPKCS12)
        {
        iPKCS12->Release();
        }
    }

// ---------------------------------------------------------
// CPKCS12Recognizer::PreferredBufSize
// ---------------------------------------------------------
//
TUint CPKCS12Recognizer::PreferredBufSize()
    {
    return KPKCS12DataMinLength; // we don't need bigger
    }

// ---------------------------------------------------------
// CPKCS12Recognizer::SupportedDataTypeL
// ---------------------------------------------------------
//
TDataType CPKCS12Recognizer::SupportedDataTypeL(TInt /*aIndex*/) const
    {
    return TDataType(KPKCS12MimeType);
    }

// ---------------------------------------------------------
// CPKCS12Recognizer::DoRecognizeL
// Checks if aBuffer contains a PKCS#12.
// ---------------------------------------------------------
//
void CPKCS12Recognizer::DoRecognizeL(
    const TDesC& /*aName*/, 
    const TDesC8& aBuffer)
    {   
      
    if (aBuffer.Size() >= KPKCS12DataMinLength)
		{	
		if (iPKCS12 == NULL)
		    {
		    iPKCS12 = PKCS12Factory::CreateL();
		    }
		if (iPKCS12->IsPKCS12Data(aBuffer))
			{
			iDataType = TDataType(KPKCS12MimeType);
			iConfidence = EProbable;			
			}		
		}
   }
    
// ---------------------------------------------------------
// CPKCS12Recognizer::CreateRecognizerL
// ---------------------------------------------------------
//
CApaDataRecognizerType* CPKCS12Recognizer::CreateRecognizerL()
	{
	return new (ELeave) CPKCS12Recognizer();
	}

// ---------------------------------------------------------
// ImplementationTable
// ---------------------------------------------------------
//
const TImplementationProxy ImplementationTable[] = 
    {    
	IMPLEMENTATION_PROXY_ENTRY(0x101F8803,CPKCS12Recognizer::CreateRecognizerL)
	};
	
// ---------------------------------------------------------
// ImplementationGroupProxy
// ---------------------------------------------------------
//
EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt& aTableCount)
    {
    #ifdef _DEBUG    	
    RDebug::Print(_L("CPKCS12Recognizer ImplementationGroupProxy"));
    #endif        
    aTableCount = sizeof(ImplementationTable) / sizeof(TImplementationProxy);
    return ImplementationTable;
    }

// End of File

