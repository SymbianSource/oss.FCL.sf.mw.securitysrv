/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:
*
*/

#ifndef DEVENCSTARTERDEBUG_H
#define DEVENCSTARTERDEBUG_H

#ifdef _DEBUG

#define DLOG( aText ) \
    { \
    _LIT( KText, aText ); \
    RDebug::Print( KText ); \
    }

#define DLOG2( aText, aParam ) \
    { \
    _LIT( KText, aText ); \
    RDebug::Print( KText, aParam ); \
    }

#else   // _DEBUG not defined, no logging code will be included at all!

#define DLOG( a )
#define DLOG2( a, p )

#endif // _DEBUG

#endif // DEVENCSTARTERDEBUG_H

// End of File
