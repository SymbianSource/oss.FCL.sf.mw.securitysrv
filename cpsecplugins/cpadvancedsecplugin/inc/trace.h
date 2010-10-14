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
* Description:  TRACE macros
*
*/

#ifndef TRACE_H
#define TRACE_H

#include <e32debug.h>                   // RDebug


// TODO: replace with OST tracing
#ifdef _DEBUG
#define TRACE( a, ARGS... ) DebugPrint( _L( a ), ##ARGS )
#define TRACE_FUNC( )       RDebug::Printf( "%s %s (%u)", __FILE__, __PRETTY_FUNCTION__, __LINE__ )
#else
#define TRACE( a, ARGS... )
#define TRACE_FUNC( )
#endif


// ---------------------------------------------------------------------------
// DebugPrint()
// ---------------------------------------------------------------------------
//
#ifdef _DEBUG
const TInt KMaxPrintSize = 256;
inline void DebugPrint( TRefByValue<const TDesC> aFmt, ... )
    {
    VA_LIST list;
    VA_START( list, aFmt );
    TBuf<KMaxPrintSize> buf;
    buf.AppendFormatList( aFmt, list );
    RDebug::Print( buf );
    }
#endif


#endif  // CPADVSECSETTINGSTRACE_H

