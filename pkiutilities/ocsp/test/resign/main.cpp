// Copyright (c) 2001-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// resign.cpp
// App for resigning OCSP responses.
// 
//

#include <e32cons.h>
#include <f32file.h>
#include <asn1dec.h>
#include <asn1enc.h>
#include <x509cert.h>
#include <signed.h>
#include <bigint.h>
#include <asymmetric.h>
#include <hash.h>
#include <bacline.h>

#include "resign.h"

static CConsoleBase* console;

static void RunConsoleL()
	{
	__UHEAP_MARK;
	// Create console
	console = Console::NewL(_L("OCSP Response Resign Utility"), TSize(KDefaultConsWidth,KDefaultConsHeight));
	CleanupStack::PushL(console);

	CResign* resign = CResign::NewLC();
	// Next stage
	resign->ProcessCommandLineL();
	
	CleanupStack::PopAndDestroy(resign);
	CleanupStack::PopAndDestroy(console);
	__UHEAP_MARKEND;
	}


// Entry point
GLDEF_C TInt E32Main()
    {
	__UHEAP_MARK;

	CTrapCleanup* cleanup=CTrapCleanup::New();

	TRAPD(error, RunConsoleL()); 

	__ASSERT_ALWAYS(!error, User::Panic(_L("TOCSP\\RESIGN ERROR"), error));

	delete cleanup;

	__UHEAP_MARKEND;

	return KErrNone;
    }
