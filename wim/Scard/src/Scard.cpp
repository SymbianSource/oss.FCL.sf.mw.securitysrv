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
* Description:  Main smard card object
*
*/


// INCLUDE FILES
#include    "ScardClsv.h"
#include    "Scard.h"
#include    "WimTrace.h"

// CONSTANTS
const TUint KDefaultMessageSlots = 4;

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// RSCard::RScard
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
RScard::RScard()
    {
    }

// -----------------------------------------------------------------------------
// RScard::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void RScard::ConstructL()
    {
    _WIMTRACE(_L("WIM|Scard|CScard::ConstructL|Begin"));
    TInt error = StartThread();
    if ( error == KErrNone )    
        {
        error = CreateSession( KScardServerName,
                               Version(),
                               KDefaultMessageSlots );
        }

    if ( error )
        {
        User::Leave( error );
        }
    }

// -----------------------------------------------------------------------------
// RScard::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C RScard* RScard::NewL()
    {
    _WIMTRACE(_L("WIM|Scard|RScard::NewL|Begin"));
    RScard* self = new( ELeave ) RScard;
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// Destructor
EXPORT_C RScard::~RScard()
    {
    _WIMTRACE(_L("WIM|Scard|RScard::~CScard|Begin"));
    TIpcArgs args( TIpcArgs::ENothing );
    SendReceive( EScardServerCloseSession, args );
    RHandleBase::Close();
    }

// -----------------------------------------------------------------------------
// RScard::Version
// Returns version of the Scard Server
// -----------------------------------------------------------------------------
//
TVersion RScard::Version() const
    {
    _WIMTRACE(_L("WIM|Scard|RScard::Version|Begin"));
    return ( TVersion( KScardServMajorVersionNumber, 
        KScardServMinorVersionNumber, KScardServBuildVersionNumber ) );
    }

//  End of File  
