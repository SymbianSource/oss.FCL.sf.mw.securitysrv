/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Implementation of application entry point functions.
*
*/


// INCLUDE FILES
#include <apmrec.h>
#include <apmstd.h>
#include <e32svr.h>
#include <implementationproxy.h>

#include "Pk5Recognizer.h"
#include <barsread.h>
#include <eikenv.h>
#include <f32file.h>

// CONSTANTS
const TUid KUidPk5Recognizer={0x2000FD89};
_LIT8(KPk5MimeType, "application/vnd.nokia.pk5");
const TInt KMaxBufferSize(512);
_LIT(KPk5Extension, ".pk5");

// ============================ MEMBER FUNCTIONS ===============================

// ---------------------------------------------------------
// CPk5Recognizer::CPk5Recognizer()
// ---------------------------------------------------------
//
CPk5Recognizer::CPk5Recognizer()
    :CApaDataRecognizerType(
    	KUidPk5Recognizer,
        CApaDataRecognizerType::ENormal )
    {
    iCountDataTypes = 1;
    }

// ---------------------------------------------------------
// CPk5Recognizer::~CPk5Recognizer()
// ---------------------------------------------------------
//
CPk5Recognizer::~CPk5Recognizer()
    {
    
    }

// ---------------------------------------------------------
// CPk5Recognizer::NewL()
// ---------------------------------------------------------
//
CPk5Recognizer* CPk5Recognizer::NewL()
    {
    CPk5Recognizer* self = new(ELeave) CPk5Recognizer();
	CleanupStack::PushL( self  );
	self->ConstructL();
	CleanupStack::Pop();
	return self;
    }

// ---------------------------------------------------------
// CPk5Recognizer::ConstructL()
// ---------------------------------------------------------
//
void CPk5Recognizer::ConstructL()
	{
	
	}

// ---------------------------------------------------------
// CPk5Recognizer::PreferredBufSize()
// ---------------------------------------------------------
//
TUint CPk5Recognizer::PreferredBufSize()
	{
    return KMaxBufferSize;
    }

// ---------------------------------------------------------
// CPk5Recognizer::SupportedDataTypeL()
// ---------------------------------------------------------
//
TDataType CPk5Recognizer::SupportedDataTypeL(TInt /*aIndex*/) const
	{
    return TDataType(KPk5MimeType);
    }
    		
// ---------------------------------------------------------
// CPk5Recognizer::DoRecognizeL()
// ---------------------------------------------------------
//
void CPk5Recognizer::DoRecognizeL(const TDesC& aName, const TDesC8& /*aBuffer*/)
    {
    iConfidence = ENotRecognized;
    
    if ( aName.Length() > KPk5Extension().Length() )
        {
        TPtrC extension = aName.Right( KPk5Extension().Length() );
        
        if( extension.Compare( KPk5Extension ) == 0 )
        	{
            iDataType = TDataType(KPk5MimeType);
            iConfidence = ECertain;
            }
        }
    }

// ========================== OTHER EXPORTED FUNCTIONS =========================

// -----------------------------------------------------------------------------
// CApaDataRecognizerType* CreateRecognizer
// The gate function - ordinal 1
// -----------------------------------------------------------------------------
//
#ifdef EKA2

// ---------------------------------------------------------
// CPk5Recognizer::CreateRecognizerL()
// ---------------------------------------------------------
//
CApaDataRecognizerType* CPk5Recognizer::CreateRecognizerL()
    {
    return CPk5Recognizer::NewL();
    }

const TImplementationProxy ImplementationTable[] =
    {
    IMPLEMENTATION_PROXY_ENTRY(KPk5RecognizerImplementationUid, 
    	CPk5Recognizer::CreateRecognizerL)
    };

EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt& aTableCount)
    {
    aTableCount = sizeof(ImplementationTable) / sizeof(TImplementationProxy);
    return ImplementationTable;
    }
    
#else
EXPORT_C CApaDataRecognizerType* CreateRecognizer()
    {
    CApaDataRecognizerType* thing=new CPk5Recognizer();
    return thing; // NULL if new failed
    }

#endif
// -----------------------------------------------------------------------------
// E32Dll
// DLL entry point
// -----------------------------------------------------------------------------
//

#ifndef EKA2
GLDEF_C TInt E32Dll(TDllReason /*aReason*/)
    {
    return KErrNone;
    }
#endif

// End of File
