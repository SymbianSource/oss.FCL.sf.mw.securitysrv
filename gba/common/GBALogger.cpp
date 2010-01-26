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
const TInt KDebugBufferSize240 = 240;
const TInt KDebugBufferSize120 = 120;

_LIT(KTimeFormat, "%F%D/%M/%Y %J:%T:%S");

void debuglineL(const char *a, void *arg1, void* arg1b, void *arg2, void *arg3, const char *arg4) 
    {
    
    HBufC8* logbuf = HBufC8::NewLC( KDebugBufferSize1024 );
    TPtr8 ptrlogbuf = logbuf->Des();
    
    TPtrC8 p((const unsigned char *)a);
    TPtrC8 temp_arg4((TUint8 *)arg4);
    ptrlogbuf.Format(p, arg1,arg1b, arg2, arg3);
    ptrlogbuf.Append(temp_arg4);
    
    if ( logbuf->Length() <= KDebugBufferSize120 )
        {
        RFileLogger::WriteFormat( KGBALogDir, KGBALogFileName,
                                  EFileLoggingModeAppend, 
                                  *logbuf );
        }
    else
        {
        RFileLogger::WriteFormat( KGBALogDir, KGBALogFileName,
                                  EFileLoggingModeAppend, 
                                  logbuf->Left( KDebugBufferSize120 ) );
        //max length is 150, print another line 
        if (logbuf->Mid(KDebugBufferSize120).Length()<= KDebugBufferSize120 )
            { 
            RFileLogger::WriteFormat( KGBALogDir, KGBALogFileName,
                                      EFileLoggingModeAppend, 
                                      logbuf->Mid(KDebugBufferSize120) );
            }
        else
            {
            RFileLogger::WriteFormat( KGBALogDir, KGBALogFileName,
                                      EFileLoggingModeAppend, 
                                      logbuf->Mid(KDebugBufferSize120, KDebugBufferSize120) );
            RFileLogger::WriteFormat( KGBALogDir, KGBALogFileName,
                                      EFileLoggingModeAppend, 
                                      logbuf->Mid(KDebugBufferSize240) );                           
            }  
                                  
          }     
    CleanupStack::PopAndDestroy( logbuf );
    }


void debuglineL(const char *a, void *arg1, void* arg1b, void *arg2, void *arg3, const TDesC &arg4) 
    {
    
    HBufC8* logbuf = HBufC8::NewLC( KDebugBufferSize1024 );
    TPtr8 ptrlogbuf = logbuf->Des();
    
    TPtrC8 p((const unsigned char *)a);
    ptrlogbuf.Format(p, arg1,arg1b, arg2, arg3);
    ptrlogbuf.Append(arg4);
    
    if ( logbuf->Length() <= KDebugBufferSize120 )
        {
        RFileLogger::WriteFormat( KGBALogDir, KGBALogFileName,
                                  EFileLoggingModeAppend, 
                                  *logbuf );
        }
    else
        {
        RFileLogger::WriteFormat( KGBALogDir, KGBALogFileName,
                                  EFileLoggingModeAppend, 
                                  logbuf->Left( KDebugBufferSize120 ) );
        //max length is 150, print another line 
        if (logbuf->Mid(KDebugBufferSize120).Length()<= KDebugBufferSize120 )
            { 
            RFileLogger::WriteFormat( KGBALogDir, KGBALogFileName,
                                      EFileLoggingModeAppend, 
                                      logbuf->Mid(KDebugBufferSize120) );
            }
        else
            {
            RFileLogger::WriteFormat( KGBALogDir, KGBALogFileName,
                                      EFileLoggingModeAppend, 
                                      logbuf->Mid(KDebugBufferSize120, KDebugBufferSize120) );
            RFileLogger::WriteFormat( KGBALogDir, KGBALogFileName,
                                      EFileLoggingModeAppend, 
                                      logbuf->Mid(KDebugBufferSize240) );                          	
            }  
                                  
    	  }     
    CleanupStack::PopAndDestroy( logbuf );
    }

void debuglineL(const char *a, void *arg1, void *arg1b, void *arg2, void *arg3, const TDesC8 &arg4) 
    {
    HBufC8* logbuf = HBufC8::NewLC( KDebugBufferSize1024 );
    TPtr8 ptrlogbuf = logbuf->Des();
    TPtrC8 p((const unsigned char *)a);
    ptrlogbuf.Format(p, arg1,arg1b, arg2, arg3);
    ptrlogbuf.Append(arg4);
    if ( logbuf->Length() <= KDebugBufferSize120 )
        {
        RFileLogger::WriteFormat( KGBALogDir, KGBALogFileName,
                                  EFileLoggingModeAppend, 
                                  *logbuf );
        }
    else
        {
        RFileLogger::WriteFormat( KGBALogDir, KGBALogFileName,
                                  EFileLoggingModeAppend, 
                                  logbuf->Left(KDebugBufferSize120) );
        //max length is 150, print another line 
        
        if (logbuf->Mid(KDebugBufferSize120).Length()<= KDebugBufferSize120 )
            { 
            RFileLogger::WriteFormat( KGBALogDir, KGBALogFileName,
                                      EFileLoggingModeAppend, 
                                      logbuf->Mid(KDebugBufferSize120) );
            }
        else
            {
            RFileLogger::WriteFormat( KGBALogDir, KGBALogFileName,
                                      EFileLoggingModeAppend, 
                                      logbuf->Mid(KDebugBufferSize120, KDebugBufferSize120) );
            RFileLogger::WriteFormat( KGBALogDir, KGBALogFileName,
                                      EFileLoggingModeAppend, 
                                      logbuf->Mid(KDebugBufferSize240) );                          	
            }                               
    	  }     
    CleanupStack::PopAndDestroy( logbuf );     
    }

void debuglineL(const char *a, void *arg1, void *arg1b, void *arg2, void *arg3, const char *arg4, const TInt& aNum ) 
    {
    HBufC8* logbuf = HBufC8::NewLC( KDebugBufferSize120 );
    TPtr8 ptrlogbuf = logbuf->Des();
    TPtrC8 temp_arg4((TUint8 *)arg4);
    ptrlogbuf.Format( temp_arg4, aNum );
     
    debuglineL(a, arg1, arg1b, arg2, arg3, *logbuf);
    CleanupStack::PopAndDestroy( logbuf );
    }

void debuglineL(const char *a, void *arg1, void *arg1b, void *arg2, void *arg3, const TDesC &arg4, const TInt& aNum ) 
    {
    HBufC* logbuf = HBufC::NewLC(KDebugBufferSize120);
    TPtr ptrlogbuf = logbuf->Des();
    ptrlogbuf.Format( arg4, aNum );
    
    debuglineL(a, arg1, arg1b, arg2, arg3, *logbuf);
    CleanupStack::PopAndDestroy( logbuf );
    }    

void DebugBinary( const TDesC8 &buf ) 
    {
    RFileLogger::HexDump( KGBALogDir, KGBALogFileName,
        EFileLoggingModeAppend, 
       NULL, NULL , buf.Ptr(), buf.Length() );
    } 

void debugTTimeL( TTime& aTime )
    {
    TBuf<KDebugBufferSize120> buf;
    aTime.FormatL( buf, KTimeFormat);
    RFileLogger::WriteFormat( KGBALogDir, KGBALogFileName,
                             EFileLoggingModeAppend, 
                             buf );
    }

#endif

//EOF
