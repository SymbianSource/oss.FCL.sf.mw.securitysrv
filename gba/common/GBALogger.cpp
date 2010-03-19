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
* Description:  Implementation of loggers
*
*/


#ifdef _DEBUG

#include    "GBALogger.h"
#include    <flogger.h>

const TInt KDebugBufferSize1024 = 1024;
const TInt KDebugBufferSize512  = 512;
const TInt KDebugBufferSize240  = 240;
const TInt KDebugBufferSize120  = 120;

_LIT(KTimeFormat, "%F%D/%M/%Y %J:%T:%S");

void debugline(const char *a, void *arg1, void* arg1b, void *arg2, void *arg3, const char *arg4) 
    {
    TBuf8<KDebugBufferSize1024> logbuffer;
    TPtrC8 p((const TUint8 *)a);
    TPtrC8 temp_arg4((const TUint8 *)arg4);
    
    if( temp_arg4.Length() > KDebugBufferSize1024 - KDebugBufferSize120  )
        return;
    
    logbuffer.Format(p, arg1,arg1b, arg2, arg3);
    logbuffer.Append(temp_arg4);
    
    if ( logbuffer.Length() <= KDebugBufferSize120 )
        {
        RFileLogger::WriteFormat( KGBALogDir, KGBALogFileName,
                                  EFileLoggingModeAppend, 
                                  logbuffer );
        }
    else
        {
        RFileLogger::WriteFormat( KGBALogDir, KGBALogFileName,
                                  EFileLoggingModeAppend, 
                                  logbuffer.Left( KDebugBufferSize120 ) );
        //max length is 150, print another line 
        if (logbuffer.Mid(KDebugBufferSize120).Length()<= KDebugBufferSize120 )
            { 
            RFileLogger::WriteFormat( KGBALogDir, KGBALogFileName,
                                      EFileLoggingModeAppend, 
                                      logbuffer.Mid(KDebugBufferSize120) );
            }
        else
            {
            RFileLogger::WriteFormat( KGBALogDir, KGBALogFileName,
                                      EFileLoggingModeAppend, 
                                      logbuffer.Mid(KDebugBufferSize120, KDebugBufferSize120) );
            RFileLogger::WriteFormat( KGBALogDir, KGBALogFileName,
                                      EFileLoggingModeAppend, 
                                      logbuffer.Mid(KDebugBufferSize240) );                           
            }                
          }
    }


void debugline(const char *a, void *arg1, void* arg1b, void *arg2, void *arg3, const TDesC &arg4) 
    {
    TBuf8<KDebugBufferSize1024> logbuffer;
    
    if( arg4.Length() > KDebugBufferSize1024 - KDebugBufferSize120  )
        return;
    
    TPtrC8 p((const TUint8 *)a);
    logbuffer.Format(p, arg1,arg1b, arg2, arg3);
    logbuffer.Append(arg4);
    
    if ( logbuffer.Length() <= KDebugBufferSize120 )
        {
        RFileLogger::WriteFormat( KGBALogDir, KGBALogFileName,
                                  EFileLoggingModeAppend, 
                                  logbuffer );
        }
    else
        {
        RFileLogger::WriteFormat( KGBALogDir, KGBALogFileName,
                                  EFileLoggingModeAppend, 
                                  logbuffer.Left( KDebugBufferSize120 ) );
        //max length is 150, print another line 
        if (logbuffer.Mid(KDebugBufferSize120).Length()<= KDebugBufferSize120 )
            { 
            RFileLogger::WriteFormat( KGBALogDir, KGBALogFileName,
                                      EFileLoggingModeAppend, 
                                      logbuffer.Mid(KDebugBufferSize120) );
            }
        else
            {
            RFileLogger::WriteFormat( KGBALogDir, KGBALogFileName,
                                      EFileLoggingModeAppend, 
                                      logbuffer.Mid(KDebugBufferSize120, KDebugBufferSize120) );
            RFileLogger::WriteFormat( KGBALogDir, KGBALogFileName,
                                      EFileLoggingModeAppend, 
                                      logbuffer.Mid(KDebugBufferSize240) );                          	
            }        
    	  }
    }

void debugline(const char *a, void *arg1, void *arg1b, void *arg2, void *arg3, const TDesC8 &arg4) 
    {  
    TBuf8<KDebugBufferSize1024> logbuffer;
    
    if( arg4.Length() > KDebugBufferSize1024 - KDebugBufferSize120)
        return;
    
    TPtrC8 p((const TUint8 *)a);
    logbuffer.Format(p, arg1,arg1b, arg2, arg3);
    logbuffer.Append(arg4); 
    
    if ( logbuffer.Length() <= KDebugBufferSize120 )
        {
        RFileLogger::WriteFormat( KGBALogDir, KGBALogFileName,
                                  EFileLoggingModeAppend, 
                                  logbuffer );
        }
    else
        {
        RFileLogger::WriteFormat( KGBALogDir, KGBALogFileName,
                                  EFileLoggingModeAppend, 
                                  logbuffer.Left(KDebugBufferSize120) );
        //max length is 150, print another line 
        
        if (logbuffer.Mid(KDebugBufferSize120).Length()<= KDebugBufferSize120 )
            { 
            RFileLogger::WriteFormat( KGBALogDir, KGBALogFileName,
                                      EFileLoggingModeAppend, 
                                      logbuffer.Mid(KDebugBufferSize120) );
            }
        else
            {
            RFileLogger::WriteFormat( KGBALogDir, KGBALogFileName,
                                      EFileLoggingModeAppend, 
                                      logbuffer.Mid(KDebugBufferSize120, KDebugBufferSize120) );
            RFileLogger::WriteFormat( KGBALogDir, KGBALogFileName,
                                      EFileLoggingModeAppend, 
                                      logbuffer.Mid(KDebugBufferSize240) );                          	
            }                               
    	  }     
    }

void debugline(const char *a, void *arg1, void *arg1b, void *arg2, void *arg3, const char *arg4, const TInt& aNum ) 
    {
    TBuf8<KDebugBufferSize512> logbuf;
    TPtrC8 temp_arg4((TUint8 *)arg4);
    
    if( temp_arg4.Length() > KDebugBufferSize512 - KDebugBufferSize120)
        return;
    
    logbuf.Format( temp_arg4, aNum );
    debugline(a, arg1, arg1b, arg2, arg3, logbuf);
    }

void debugline(const char *a, void *arg1, void *arg1b, void *arg2, void *arg3, const TDesC &arg4, const TInt& aNum ) 
    {
    TBuf<KDebugBufferSize512> logbuf;
    
    if( arg4.Length() > KDebugBufferSize512 - KDebugBufferSize120 )
        return;
    
    logbuf.Format( arg4, aNum );
    debugline(a, arg1, arg1b, arg2, arg3, logbuf);
    }    

void DebugBinary( const TDesC8 &buf ) 
    {
    RFileLogger::HexDump( KGBALogDir, KGBALogFileName,
        EFileLoggingModeAppend, 
       NULL, NULL , buf.Ptr(), buf.Length() );
    } 

void debugTTime( TTime& aTime )
    {
    TBuf<KDebugBufferSize120> buf;
    TRAPD(error , aTime.FormatL( buf, KTimeFormat));
    if(error != KErrNone)
        return;
    RFileLogger::WriteFormat( KGBALogDir, KGBALogFileName,
                             EFileLoggingModeAppend, 
                             buf );
    }

#endif

//EOF
