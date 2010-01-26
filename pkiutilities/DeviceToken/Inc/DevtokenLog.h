/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Print debug log 
*
*/



#ifndef	DEVTOKENLOG_H
#define	DEVTOKENLOG_H

#include <e32base.h>


#ifdef _DEBUG
#include <e32svr.h>
#define TRACE_PRINT( aText ) { RDebug::Print( _L(aText) ); }
#define TRACE_PRINT_NUM( aText, aNum ) \
		{ RDebug::Print( _L(aText), aNum ); }
#define TRACE_PRINT_RAW( aBuf ) { RDebug::RawPrint( aBuf ); }

#else

#define TRACE_PRINT( aText )
#define TRACE_PRINT_NUM( aText, aNum ) 
#define TRACE_PRINT_RAW( aBuf )
#endif

#endif	//DEVTOKENLOG_H
//EOF