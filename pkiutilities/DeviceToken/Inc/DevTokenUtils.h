/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   The header file of DevTokenUtils
*
*/



#ifndef __DEVTOKENUTILS_H__
#define __DEVTOKENUTILS_H__

#include <e32std.h>
#include "DevTokenTypesEnum.h"

_LIT(KDevTokenTypeClientPanic, "DEVTOKENTYPECLIENT");

void DevTokenPanic(EDevTokenTypeClientPanic aPanicCode);


#endif //__DEVTOKENUTILS_H__

