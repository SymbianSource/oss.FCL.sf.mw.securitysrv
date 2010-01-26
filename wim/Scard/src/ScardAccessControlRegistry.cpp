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
* Description:  Handles the creation and destruction of individual access 
*                controllers
*
*/



// INCLUDE FILES
#include    "ScardAccessControlRegistry.h"
#include    "ScardAccessControl.h"
#include    "WimTrace.h"

#ifdef _DEBUG // for logging
#include    "ScardLogs.h"
#include    <flogger.h> 
#endif
// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CScardAccessControlRegistry::CScardAccessControlRegistry
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CScardAccessControlRegistry::CScardAccessControlRegistry()
    {
    }

// -----------------------------------------------------------------------------
// CScardAccessControlRegistry::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CScardAccessControlRegistry::ConstructL( CScardServer* aServer )
    {
    _WIMTRACE(_L("WIM|Scard|CScardAccessControlRegistry::ConstructL|Begin"));
    iAccessControllers = new( ELeave ) CArrayFixFlat<CScardAccessControl*>( 1 );
    iServer = aServer;
    }

// -----------------------------------------------------------------------------
// CScardAccessControlRegistry::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CScardAccessControlRegistry* CScardAccessControlRegistry::NewL(
    CScardServer* aServer )
    {
    _WIMTRACE(_L("WIM|Scard|CScardAccessControlRegistry::NewL|Begin"));
    CScardAccessControlRegistry* self = 
        new( ELeave ) CScardAccessControlRegistry;
    
    CleanupStack::PushL( self );
    self->ConstructL( aServer );
    CleanupStack::Pop( self );

    return self;
    }

    
// Destructor
CScardAccessControlRegistry::~CScardAccessControlRegistry()
    {
    _WIMTRACE(_L("WIM|Scard|CScardAccessControlRegistry::~CScardAccessControlRegistry|Begin"));
    while ( iAccessControllers->Count() )
        {
        RemoveAccessController( 0 );
        }
    delete iAccessControllers;
    }


// -----------------------------------------------------------------------------
// CScardAccessControlRegistry::ControllerRetired
// An access controller has been disconnected
// -----------------------------------------------------------------------------
//
void CScardAccessControlRegistry::ControllerRetired(
    CScardAccessControl* aControl )
    {
    _WIMTRACE(_L("WIM|Scard|CScardAccessControlRegistry::ControllerRetired|Begin"));
    for ( TInt i( 0 ); i < iAccessControllers->Count(); i++ )
        {
        if ( iAccessControllers->At( i ) == aControl )
            {
            iAccessControllers->Delete( i );
            return;
            }
        }
    }

// -----------------------------------------------------------------------------
// CScardAccessControlRegistry::RemoveAccessController
// Remove Access controller from registry
// -----------------------------------------------------------------------------
//
void CScardAccessControlRegistry::RemoveAccessController(
    const TInt aIndex )
    {
    _WIMTRACE(_L("WIM|Scard|CScardAccessControlRegistry::RemoveAccessController|Begin"));
    __ASSERT_ALWAYS( aIndex < iAccessControllers->Count(), 
        User::Panic( _L( "Registry corruption" ), 
            KScServerPanicInternalError ) );

    //  we allocated the controller, so we must also delete it..
    delete ( (*iAccessControllers)[aIndex] );
    
    //  ...and remove it from the registy as well
    //Check first it is not removed yet...
    if ( aIndex < iAccessControllers->Count() )
        {
        iAccessControllers->Delete( aIndex );
        }
    return;
    }

// -----------------------------------------------------------------------------
// CScardAccessControlRegistry::Reader
// Get reader object by Reader ID
// -----------------------------------------------------------------------------
//
MScardReader* CScardAccessControlRegistry::Reader(
    const TReaderID aReaderID) const
    {
    _WIMTRACE(_L("WIM|Scard|CScardAccessControlRegistry::Reader|Begin"));
    for ( TInt i( 0 ); i < iAccessControllers->Count(); i++ )
        {
        if ( (*iAccessControllers)[i]->ReaderID() == aReaderID )
            {
            return (*iAccessControllers)[i]->Reader();
            }
        }
    return NULL;
    }

// -----------------------------------------------------------------------------
// CScardAccessControlRegistry::FindAccessController
// Returns the access controller for given reader. 
// Returns NULL if access controlled is not created
// -----------------------------------------------------------------------------
//
CScardAccessControl* CScardAccessControlRegistry::FindAccessController(
    const TReaderID aReaderID )
    {
    _WIMTRACE(_L("WIM|Scard|CScardAccessControlRegistry::FindAccessController|Begin"));
    for ( TInt i( 0 ); i < iAccessControllers->Count(); i++ )
        {
        if ( (*iAccessControllers)[i]->ReaderID() == aReaderID )
            {
            return (*iAccessControllers)[i];
            }
        }
    return NULL;
    }

// -----------------------------------------------------------------------------
// CScardAccessControlRegistry::ReaderHandlerLoaded
// Check if this reader has a controller loaded
// -----------------------------------------------------------------------------
//
TBool CScardAccessControlRegistry::ReaderHandlerLoaded(
    const TReaderID aReaderID ) const 
    {
    _WIMTRACE(_L("WIM|Scard|CScardAccessControlRegistry::ReaderHandlerLoaded|Begin"));
    for ( TInt i( 0 ); i < iAccessControllers->Count(); i++ )
        {
        if ( (*iAccessControllers)[i]->ReaderID() == aReaderID )
            {
            return ETrue;
            }
        }
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CScardAccessControlRegistry::Server
// Returns the server
// -----------------------------------------------------------------------------
//
CScardServer* CScardAccessControlRegistry::Server()
    {
    _WIMTRACE(_L("WIM|Scard|CScardAccessControlRegistry::Server|Begin"));
    return iServer;
    }

// -----------------------------------------------------------------------------
// CScardAccessControlRegistry::AccessController
// Creates access controller if necessary
// -----------------------------------------------------------------------------
//
CScardAccessControl* CScardAccessControlRegistry::AccessController(
    const TReaderID aReaderID )
    {
    _WIMTRACE(_L("WIM|Scard|CScardAccessControlRegistry::AccessController|Begin"));
#ifdef _DEBUG
    RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
        EFileLoggingModeAppend, 
        _L( "CScardAccessControlRegistry::GetAccessController entered.\n" ) );
#endif
    //  If a controller is already made for the reader, just return it
    CScardAccessControl* ctrl = FindAccessController( aReaderID );
    if ( ctrl )
        {
#ifdef _DEBUG
        RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
            EFileLoggingModeAppend, 
            _L( "CScardAccessControlRegistry::GetAccessController access\
            controller found.\n" ) );
#endif
        return ctrl;
        }
        
    //  If we're here, the controller has not yet been made, so create it
#ifdef _DEBUG
    RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
        EFileLoggingModeAppend, 
        _L( "CScardAccessControlRegistry::GetAccessController create new access\
        controller.\n" ) );
#endif
    TRAPD( err,  ctrl = CScardAccessControl::NewL( aReaderID, this ) );
    if ( !ctrl || err )
        {
#ifdef _DEBUG
        RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
            EFileLoggingModeAppend, 
            _L( "CScardAccessControlRegistry::GetAccessController access\
            controller creation failed: %d.\n" ), err );
#endif
        return NULL;
        }

    TRAP( err, iAccessControllers->AppendL( ctrl ) );
    if ( err )
        {
        return NULL;
        }

    return ctrl;
    }

//  End of File  
