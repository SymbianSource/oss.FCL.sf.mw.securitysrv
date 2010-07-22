/*
 * Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * This component and the accompanying materials are made available
 * under the terms of "Eclipse Public License v1.0""
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

#if defined (_DEBUG)

_LIT( KLogFile, "Devicelockplugin.log" );
_LIT( KLogDir, "Devicelockplugin" );

#include <e32std.h>
#include <f32file.h>
#include <flogger.h>
#include <e32svr.h>

inline void FWrite (TRefByValue<const TDesC> aFmt,...)
    {
    VA_LIST list;
    VA_START( list, aFmt );
    RFileLogger::WriteFormat( KLogDir,KLogFile,EFileLoggingModeAppend ,TPtrC(aFmt) ,list );
    }

    #define Dprint(arg...)    FWrite arg;
#else
    #define Dprint(arg...)
#endif  // _DEBUG


