/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description: SecUi notification plugin dialog.
*
*/

#ifndef SECUINOTIFICATIONDEBUG_H
#define SECUINOTIFICATIONDEBUG_H

#include <e32debug.h>
#ifdef _DEBUG
#define RDEBUG( x, y ) RDebug::Printf( "%s %s (%u) %s=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, x, y );
#else
#define RDEBUG( x, y )
#endif

#endif // SECUINOTIFICATIONDEBUG_H
