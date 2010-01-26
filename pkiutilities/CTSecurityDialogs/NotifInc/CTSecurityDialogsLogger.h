/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Implemented logger functionality of the module
*
*/


#ifndef WIMSECURITYDIALOGS_LOGGER_H
#define WIMSECURITYDIALOGS_LOGGER_H

#if defined ( _DEBUG )

// INCLUDES

#include <flogger.h>

// log files are stored to KDebugLogDirFull folder
_LIT(KDebugLogDependency,"C:\\Logs\\");
_LIT(KDebugLogDoubleBackSlash,"\\");

_LIT(KDebugLogDir, "WimSecurityDialogs");
_LIT(KDebugLogFile, "WimSecurityDialogsNotifierDebugLog.log");
_LIT(KDebugLogTitle, "- BrowserTelServiceNotifier Debug Log File -");
_LIT( KWimsecuritydialogsLogMessageAppBanner,   "WimSecurityDialogs: module (%d.%d.%d) started" );
_LIT( KDebugLogEnterFn,            "WimSecurityDialogs: -> %S" );
_LIT( KDebugLogLeaveFn,            "WimSecurityDialogs: <- %S" );
_LIT( KDebugLogTimeFormatString,   "%H:%T:%S:%*C3" );
_LIT( KDebugLogExit,               "WimSecurityDialogs: module exit" );


// MACROS

/**
* Use this macro in order to initialize logger :
* - create log directory,
* - write version information into the log file
*/
#define WIMSECURITYDIALOGS_CREATE { TFileName path( KDebugLogDependency ); path.Append( KDebugLogDir ); path.Append( KDebugLogDoubleBackSlash ); RFs& fs = CEikonEnv::Static()->FsSession(); fs.MkDirAll( path ); RFileLogger::WriteFormat( KDebugLogDir, KDebugLogFile, EFileLoggingModeOverwrite, KWimsecuritydialogsLogMessageAppBanner ); }

/**
* Use this macro for writing information about exiting.
*/
#define WIMSECURITYDIALOGS_DELETE { RFileLogger::Write( KDebugLogDir, KDebugLogFile, EFileLoggingModeAppend, KDebugLogExit ); }

/**
* Use this function at the entry point of any functions.
* @param a Entry information of the method.
*/
#define WIMSECURITYDIALOGS_ENTERFN( a ) { _LIT( temp, a ); RFileLogger::WriteFormat( KDebugLogDir, KDebugLogFile, EFileLoggingModeAppend, KDebugLogEnterFn, &temp ); }

/**
* Use this function right before you leave the method.
* @param a Leaving information of the method.
*/
#define WIMSECURITYDIALOGS_LEAVEFN( a ) { _LIT( temp, a ); RFileLogger::WriteFormat( KDebugLogDir, KDebugLogFile, EFileLoggingModeAppend, KDebugLogLeaveFn, &temp ); }

/**
* Use this function at any points of a function for logging the current state.
* @param a String to be written into logfile about the current state
*/
#define WIMSECURITYDIALOGS_WRITE( a ) { _LIT( temp, a ); RFileLogger::Write( KDebugLogDir, KDebugLogFile, EFileLoggingModeAppend, temp ); }

/**
* Use this function at any points of a function for logging the current state.
* You can use printf-like formats, but with only one parameter to be substituted..
* @param a Format string,
* @param b Parameter to be substituted.
*/
#define WIMSECURITYDIALOGS_WRITE_FORMAT( a, b ) { _LIT( temp, a ); RFileLogger::WriteFormat( KDebugLogDir, KDebugLogFile, EFileLoggingModeAppend, temp, b ); }

/**
* Use this function at any points of a function for logging the current state.
* The current date/time stamp is written besides the string you give.
* @param a String to be written into logfile about the current state
*/
#define WIMSECURITYDIALOGS_WRITE_TIMESTAMP( a ) { _LIT( temp, a ); TTime time; time.HomeTime(); TBuf<256> buffer; time.FormatL( buffer, KDebugLogTimeFormatString ); buffer.Insert( 0, temp ); RFileLogger::Write( KDebugLogDir, KDebugLogFile, EFileLoggingModeAppend, buffer); }


#else // _DEBUG

// Empty macros
#define WIMSECURITYDIALOGS_CREATE
#define WIMSECURITYDIALOGS_DELETE
#define WIMSECURITYDIALOGS_ENTERFN( a )
#define WIMSECURITYDIALOGS_LEAVEFN( a )
#define WIMSECURITYDIALOGS_WRITE( a )
#define WIMSECURITYDIALOGS_WRITE_FORMAT( a, b )
#define WIMSECURITYDIALOGS_WRITE_TIMESTAMP( a )

#endif // _DEBUG


#endif	// WIMSECURITYDIALOGS_LOGGER_H

// End of file
