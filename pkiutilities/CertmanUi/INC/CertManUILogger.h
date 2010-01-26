/*
* Copyright (c) 2003-2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Implements logger functionality of the module
*
*/


#ifndef CERTMANUI_LOGGER_H
#define CERTMANUI_LOGGER_H


#if defined ( _DEBUG ) || defined ( _CERTMANUI_LOG_ )

// INCLUDES
#include <flogger.h>

enum TLoggingSelectors
  {
  ECertManUILogEnter = 1,
    ECertManUILogLeave,
    ECertManUILogTimeStamp,
    ECertManUILogWrite
  };

// CONSTANTS

// Macros to be substituted
_LIT( KCertManUILogMessageAppBanner,   "CertManUI: module (%d.%d.%d) started" );
_LIT( KCertManUILogExit,               "CertManUI: module exit" );
_LIT( KCertManUILogTimeFormatString,   "%H:%T:%S:%*C3" );
_LIT8( KCertManUILogEnterFn,           "ENTRY " );
_LIT8( KCertManUILogLeaveFn,           "EXIT " );

// ATTENTION!!! We're depending on FLogger DLL, because it presumes the
// existance of C:\LOGS directory.
_LIT( KCertManUIFLoggerDependency,    "c:\\logs\\" );
_LIT( KCertManUIDoubleBackSlash,    "\\" );
_LIT( KCertManUILogDir,         "certmanui" );
_LIT( KCertManUILogFile,        "certmanui.log" );

/**
* Use this macro in order to initialize logger :
* - create log directory,
* - write version information into the log file
*/
#define CERTMANUILOGGER_CREATE { \
                              TFileName path( KCertManUIFLoggerDependency ); \
                              path.Append( KCertManUILogDir ); \
                              path.Append( KCertManUIDoubleBackSlash ); \
                              RFs& fs = CEikonEnv::Static()->FsSession(); \
                              fs.MkDirAll( path ); \
                              RFileLogger::WriteFormat( \
                                                      KCertManUILogDir, \
                                                      KCertManUILogFile, \
                                                      EFileLoggingModeOverwrite, \
                                                      KCertManUILogMessageAppBanner \
                                                      ); \
                              }

/**
* Use this macro for writing information about exiting.
*/
#define CERTMANUILOGGER_DELETE { \
                                RFileLogger::Write( \
                                                  KCertManUILogDir, \
                                                  KCertManUILogFile, \
                                                  EFileLoggingModeAppend, \
                                                  KCertManUILogExit \
                                                  ); \
                                }

/**
* Use this function at the entry point of any functions.
* @param a Entry information of the method.
*/
#define CERTMANUILOGGER_ENTERFN( a ) write_log (ECertManUILogEnter, a)
/**
* Use this function right before you leave the method.
* @param a Leaving information of the method.
*/
#define CERTMANUILOGGER_LEAVEFN( a ) write_log (ECertManUILogLeave, a)

/**
* Use this function at any points of a function for logging the current state.
* @param a String to be written into logfile about the current state
*/
#define LOG_WRITE( a ) write_log (ECertManUILogWrite, a)
/**
* Use this function at any points of a function for logging the current state.
* You can use printf-like formats, but with only one parameter to be substituted..
* @param a Format string,
* @param b Parameter to be substituted.
*/
#define LOG_WRITE_FORMAT( a, b ) write_log (ECertManUILogWrite, a, b)

/**
* Use this function at any points of a function for logging the current state.
* The current date/time stamp is written besides the string you give.
* @param a String to be written into logfile about the current state
*/

#define CERTMANUILOGGER_WRITE_TIMESTAMP( a ) write_log (ECertManUILogTimeStamp, a)

inline void write_log (TInt aSelector, const char* aFmt,...) //lint !e960
    {
    TBuf8<256> buf;
        {
      VA_LIST ap;//lint !e960
        TPtrC8 fmt((unsigned char *)aFmt);
      VA_START(ap, aFmt);  //lint !e960
      buf.FormatList(fmt, ap);
      VA_END(ap);  //lint !e960
        }
    switch(aSelector)
        {
        case ECertManUILogEnter:
            buf.Insert(0, KCertManUILogEnterFn);
            break;
        case ECertManUILogLeave:
            buf.Insert(0, KCertManUILogLeaveFn);
            break;
        default:
            break;
        }

    RFileLogger::Write( KCertManUILogDir, KCertManUILogFile, EFileLoggingModeAppend, buf);

    TBuf<256> tempBuf;
    tempBuf.Copy( buf );
    RDebug::Print( tempBuf );
    }

#define LOG_HEXDUMP( aBuf ) { \
                            RFileLogger::HexDump( \
                                                  KCertManUILogDir, \
                                                  KCertManUILogFile, \
                                                  EFileLoggingModeAppend, \
                                                  ((const TText *)" "), \
                                                  ((const TText *)" "), \
                                                  aBuf.Ptr(), aBuf.Size()); \
                            }


#else // _DEBUG || _CERTMANUI_LOG

// Empty macros
#define CERTMANUILOGGER_CREATE
#define CERTMANUILOGGER_DELETE
#define CERTMANUILOGGER_ENTERFN( a )
#define CERTMANUILOGGER_LEAVEFN( a )
#define LOG_WRITE( a )
#define LOG_WRITE_FORMAT( a, b )
#define CERTMANUILOGGER_WRITE_TIMESTAMP( a ) /**/

#endif // _DEBUG || _CERTMANUI_LOG

#endif  // CERTMANUI_LOGGER_H

// End of file
