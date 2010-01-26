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
// The panic method and TOCSP panic code declarations
// 
//

/**
 @file 
 @internalTechnology
*/

#ifndef __TOCSP_PANIC_H__
#define __TOCSP_PANIC_H__

#include <e32def.h>

enum TTOCSPPanics
	{
	KErrTOCSPInternalError,
	KErrTOCSPScriptParameterError
	};


GLDEF_C void Panic(TInt aPanic);

#endif
