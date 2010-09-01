/*
* Copyright (c) 2003 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Registry of Smart Card readers
*
*/



// INCLUDE FILES

#include    "ScardReaderRegistry.h"
#include    "ScardReaderLauncher.h"
#include    "ScardServer.h"
#include    "ScardAccessControlRegistry.h"
#include    "WimTrace.h"

#ifdef _DEBUG // for logging
#include    "ScardLogs.h"
#include    <flogger.h> 
#endif

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// TLauncherStruct::TLauncherStruct
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
TLauncherStruct::TLauncherStruct()
    : iGroupID( 0 ),
      iReaderID( 0 ),
      iLauncher( NULL ) 
    {
    }

// -----------------------------------------------------------------------------
// CScardReaderRegistry::CScardReaderRegistry
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CScardReaderRegistry::CScardReaderRegistry( CScardServer* aServer )
    : iServer( aServer ),
      iLaunchersInUse( NULL )
    {
    _WIMTRACE(_L("WIM|Scard|CScardReaderRegistry::CScardReaderRegistry|Begin"));
    }

// -----------------------------------------------------------------------------
// CScardReaderRegistry::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CScardReaderRegistry::ConstructL()
    {
    _WIMTRACE(_L("WIM|Scard|CScardReaderRegistry::ConstructL|Begin"));
    iLaunchersInUse = new( ELeave ) CArrayFixFlat<TLauncherStruct>( 1 );
    }

// -----------------------------------------------------------------------------
// CScardReaderRegistry::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CScardReaderRegistry* CScardReaderRegistry::NewL( 
    CScardServer *aServer )
    {
    _WIMTRACE(_L("WIM|Scard|CScardReaderRegistry::NewL|Begin"));
    CScardReaderRegistry* self = new( ELeave ) CScardReaderRegistry( aServer );
    
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    return self;
    }

    
// Destructor
CScardReaderRegistry::~CScardReaderRegistry()
    {
    _WIMTRACE(_L("WIM|Scard|CScardReaderRegistry::~CScardReaderRegistry|Begin"));
    //  remove all currently used readers
    while ( iLaunchersInUse->Count() )
        {
        delete (*iLaunchersInUse)[0].iLauncher;
        (*iLaunchersInUse)[0].iLibrary.Close();
        (*iLaunchersInUse)[0].iSession.Close();
        iLaunchersInUse->Delete( 0 );
        }

    delete iLaunchersInUse;
    }


// -----------------------------------------------------------------------------
// CScardReaderRegistry::ReaderID
// Get ID for name
// -----------------------------------------------------------------------------
//
TReaderID CScardReaderRegistry::ReaderID(
    const TScardReaderName& aReaderName )
    {
    _WIMTRACE(_L("WIM|Scard|CScardReaderRegistry::ReaderID|Begin"));
    TReaderID readerID( 0 );

    TScardReaderName readerName( KReaderName ); //Only supported is "SwimReader"

    if ( aReaderName.Compare( readerName ) ) //Compare names
        {
        readerID = 0; //Not equal
        }
    else
        {
        readerID = KReaderID;
        }

    return readerID;
    }

// -----------------------------------------------------------------------------
// CScardReaderRegistry::CloseReaderL
// Unload the reader module with given ID. Also unloads the launcher 
// module if it's no longer needed
// -----------------------------------------------------------------------------
//
void CScardReaderRegistry::CloseReader( const TReaderID aReaderID )
    {
    _WIMTRACE(_L("WIM|Scard|CScardReaderRegistry::CloseReaderL|Begin"));
    //  first retrieve the launcher structure
    TLauncherStruct launcher;
    TInt index( 0 );
    for ( ; index < iLaunchersInUse->Count(); index++ )
        {
        if ( iLaunchersInUse->At( index ).iReaderID == aReaderID )
            {
            launcher = iLaunchersInUse->At( index );
            break;
            }
        }

    //  Couldn't find it!! (So assume it's closed ok)
    if ( index >= iLaunchersInUse->Count() )
        {
        return;
        }

    //  Instuct the launcher to cleanup the reader..
    launcher.iLauncher->DeleteReader( aReaderID );

    //  ..and never mind any error codes, just close the library...
    delete launcher.iLauncher;
    launcher.iLibrary.Close();
    launcher.iSession.Close();
    
    //  Finally remove the launcher from our records
    iLaunchersInUse->Delete( index );
    }

// -----------------------------------------------------------------------------
// CScardReaderRegistry::IsSupported
// Is the given reader supported
// -----------------------------------------------------------------------------
//
TBool CScardReaderRegistry::IsSupported(
    const TScardReaderName& aReaderName )
    {
    _WIMTRACE(_L("WIM|Scard|CScardReaderRegistry::IsSupported|Begin"));
    TScardReaderName readerName( KReaderName ); //Only supported is "SwimReader"

    if ( aReaderName.Compare( readerName ) ) //Compare names
        {
        return EFalse;
        }
    else
        {
        return ETrue;
        }
    }

// -----------------------------------------------------------------------------
// CScardReaderRegistry::ListOpenReaders
// List all open readers
// -----------------------------------------------------------------------------
//
void CScardReaderRegistry::ListOpenReadersL(
    CArrayFix<CScardAccessControl*>* aACList )
    {
    _WIMTRACE(_L("WIM|Scard|CScardReaderRegistry::ListOpenReadersL|Begin"));
    CScardAccessControl* ac =
        iServer->AccessRegistry()->FindAccessController( KReaderID );
    if ( ac )
        {
        aACList->AppendL( ac );
        }
    }

// -----------------------------------------------------------------------------
// CScardReaderRegistry::ListClosedReaders
// List closed readers
// -----------------------------------------------------------------------------
//
void CScardReaderRegistry::ListClosedReadersL( CArrayFix<TReaderID>* aIDList )
    {
    _WIMTRACE(_L("WIM|Scard|CScardReaderRegistry::ListClosedReadersL|Begin"));
    // See if it has already an access controller (aka it's been opened already)
    CScardAccessControl* ac = 
        iServer->AccessRegistry()->FindAccessController( KReaderID );
    if ( !ac )
        {
        aIDList->AppendL( KReaderID );
        }
    }

// -----------------------------------------------------------------------------
// CScardReaderRegistry::FriendlyName
// Get friendly name of reader
// -----------------------------------------------------------------------------
//
const TScardReaderName CScardReaderRegistry::FriendlyName(
    const TReaderID aReaderID )
    {
    _WIMTRACE(_L("WIM|Scard|CScardReaderRegistry::FriendlyName|Begin"));
    TScardReaderName name;
    if ( aReaderID == KReaderID ) //Only supported is 1
        {
        name.Copy( KReaderName );
        }
    return name;

    }

// -----------------------------------------------------------------------------
// CScardReaderRegistry::LoadReaderL
// Load reader
// -----------------------------------------------------------------------------
//
MScardReader* CScardReaderRegistry::LoadReaderL( const TReaderID aReaderID )
    {
    _WIMTRACE(_L("WIM|Scard|CScardReaderRegistry::LoadReaderL|Begin"));
    TGroupID groupID( 0 );
    if ( aReaderID == KReaderID )
        {
        groupID = KGroupID;
        }

    //  load the group launcher's module
#ifdef _DEBUG    
    RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
        EFileLoggingModeAppend, 
        _L("CScardReaderRegistry::LoadReaderL loading launcher module...\n") );
#endif
    MScardReaderLauncher* launcher = NULL;
    
    launcher = LoadFactoryModuleL( groupID, aReaderID );

    MScardReader* reader = NULL;

    //  create the reader
#ifdef _DEBUG    
    RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
        EFileLoggingModeAppend, 
        _L( "CScardReaderRegistry::LoadReaderL creating reader...\n" ) );
#endif
    reader = (/*(CSwimReaderLauncher*)*/launcher)->CreateReaderL( aReaderID );

    return reader;
    }

// -----------------------------------------------------------------------------
// CScardReaderRegistry::LoadFactoryModuleL
// Load factory module (SwimReader.dll)
// -----------------------------------------------------------------------------
//
MScardReaderLauncher* CScardReaderRegistry::LoadFactoryModuleL(
    const TGroupID aGroupID, 
    const TReaderID aReaderID )
    {
    _WIMTRACE(_L("WIM|Scard|CScardReaderRegistry::LoadFactoryModuleL|Begin"));
    //Check ID's, only supported ID's in Series 60 are
    // KGroupID and KReaderID
    if ( aGroupID != KGroupID || aReaderID != KReaderID )
        {
        User::Leave( KErrArgument );
        }
    TLauncherStruct launcher;
    launcher.iGroupID = KGroupID; //This is always 1
    launcher.iReaderID = KReaderID; //This is always 1
    launcher.iSession.Connect();

    User::LeaveIfError( launcher.iLibrary.Load( KSwimReaderDLL ) );

    //  Create the actual CScardReaderLauncher object
#ifdef _DEBUG    
    RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
        EFileLoggingModeAppend, 
        _L( "CScardReaderRegistry::LoadFactoryModuleL creating\
        CScardReaderLauncher object...\n" ) );
#endif
    TLibraryFunction entry = launcher.iLibrary.Lookup( 1 );

    if ( entry ) // entry found
        {
        launcher.iLauncher = reinterpret_cast< MScardReaderLauncher* >
                                ( entry() );
        }
        else
        {
        //delete launcher.iLauncher; // (?) why delete
        launcher.iLibrary.Close();
        launcher.iSession.Close();
        User::Leave( KErrNotFound );
        }

    //  Initialise the launcher and append it to the list
#ifdef _DEBUG    
    RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
        EFileLoggingModeAppend, 
        _L( "CScardReaderRegistry::LoadFactoryModuleL initialising\
        launcher...\n" ) );
#endif
    TRAPD( initialisationError, launcher.iLauncher->ConstructL( 
        iServer->ReaderService() ) );
    if ( initialisationError )
        {
        delete launcher.iLauncher;
        launcher.iLibrary.Close();
        launcher.iSession.Close();
        User::Leave( initialisationError );
        }

    iLaunchersInUse->AppendL( launcher );
    return launcher.iLauncher;
    }

//  End of File  
