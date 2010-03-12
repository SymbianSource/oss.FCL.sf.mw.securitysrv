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
* Description:  logger declarations
*
*/

#ifndef __GBALOGGER_H_
#define __GBALOGGER_H_

#include <e32base.h>

_LIT( KGBALogDir, "GBA" );
_LIT( KGBALogFileName, "GBALog.txt" );
_LIT( KEnter, "Enter ---->");
_LIT( KEnd, "Exit <----");

#ifdef _DEBUG

    void debugline(const char *a, void *arg1, void* f, void *arg2, void *arg3, const TDesC &arg4);
    void debugline(const char *a, void *arg1, void* f, void *arg2, void *arg3, const TDesC8 &arg4);
    void debugline(const char *a, void *arg1, void* f, void *arg2, void *arg3, const char *arg4, const TInt &aNum);
    void debugline(const char *a, void *arg1, void* f, void *arg2, void *arg3, const TDesC &arg4, const TInt &aNum);
    void DebugBinary(const TDesC8 &buf);
    void debugTTime( TTime& aTime );
    void debugline(const char *a, void *arg1, void* f, void *arg2, void *arg3, const char *arg4);

    #define GBA_DEBUG
    #define GBA_TRACE_DEBUG_DESC(a) debugline("%s:%s %d: %d",(void *) __FILE__, (void *) __func__, (void *)__LINE__, (void *)User::TickCount(),(a))
    #define GBA_TRACE_DEBUG_NUM( a,b ) debugline("%s:%s %d: %d",(void *) __FILE__, (void *) __func__, (void *)__LINE__, (void *)User::TickCount(),(a),b)
    
#ifdef LOG_GBA_KEYS
    #define GBA_TRACE_DEBUG_BINARY(a) DebugBinary((a));
#else
    #define GBA_TRACE_DEBUG_BINARY(a)
#endif 
    
    #define GBA_TRACE_BEGIN() debugline("%s:%s %d: %d",(void *) __FILE__, (void *) __func__, (void *)__LINE__, (void *)User::TickCount(),(KEnter))
    #define GBA_TRACE_END() debugline("%s:%s %d: %d",(void *) __FILE__, (void *) __func__, (void *)__LINE__, (void *)User::TickCount(),(KEnd))
    #define GBA_TRACE_TIME( a ) debugTTime( (a) );
    #define GBA_TRACE_DEBUG(a) debugline("%s:%s %d: %d",(void *) __FILE__, (void *) __func__, (void *)__LINE__, (void *)User::TickCount(),(a))

#else

    #define GBA_TRACE_DEBUG_DESC(a)
    #define GBA_TRACE_DEBUG(a) 
    #define GBA_TRACE_DEBUG_NUM(a,b)
    #define GBA_TRACE_DEBUG_BINARY(a)
    #define GBA_TRACE_BEGIN()
    #define GBA_TRACE_END()
    #define GBA_TRACE_TIME( a )

#endif


#endif //__GBALOGGER_H_
                                                                        
//EOF
    
    
