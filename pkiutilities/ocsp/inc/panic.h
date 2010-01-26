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
// The panic method and OCSP panic code declarations
// 
//

/**
 @file 
 @internalComponent 
*/

#ifndef __OCSP_PANIC_H__
#define __OCSP_PANIC_H__

#include <e32def.h>

enum TOCSPPanics
	{
	KErrNoCertificates,
	KErrNoAuthorisationSchemes
	};


GLDEF_C void Panic(TInt aPanic);

#endif
