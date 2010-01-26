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
* Description:  SWIM launcher. Creates new instance of SwimReader.
*
*/



// INCLUDE FILES 
#include    "SwimLauncher.h"
#include    "SwimReaderIF.h"
#include    "ScardBase.h"
#include    "WimTrace.h"        // for trace logging


// ============================= LOCAL FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// NewServiceL
// New CSwimReaderLauncher
// -----------------------------------------------------------------------------
//
EXPORT_C CSwimReaderLauncher* NewServiceL()
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReaderLauncher::NewServiceL|Begin"));
    return new( ELeave ) CSwimReaderLauncher;
    }


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CSwimReaderLauncher::CSwimReaderLauncher
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CSwimReaderLauncher::CSwimReaderLauncher()
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReaderLauncher::CSwimReaderLauncher|Begin"));
    }

// -----------------------------------------------------------------------------
// CSwimReaderLauncher::ConstructL
// Symbian 2nd phase constructor can leave. 
// -----------------------------------------------------------------------------
//
void CSwimReaderLauncher::ConstructL( MScardReaderService* aService )
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReaderLauncher::ConstructL|Begin"));
    iService = aService;
    }

// Detructor
CSwimReaderLauncher::~CSwimReaderLauncher()
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReaderLauncher::~CSwimReaderLauncher|Begin"));
    }

// -----------------------------------------------------------------------------
// CSwimReaderLauncher::CreateReaderL
// Create reader
// -----------------------------------------------------------------------------
//
MScardReader* CSwimReaderLauncher::CreateReaderL( TReaderID aReaderID )
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReaderLauncher::CreateReaderL|Begin"));
    iReader = CSwimReaderIF::NewL( iService->NotifyObserver(), 
                                   aReaderID, this );  
    return iReader;
    }

// -----------------------------------------------------------------------------
// CSwimReaderLauncher::DeleteReader
// Delete reader
// -----------------------------------------------------------------------------
//
void CSwimReaderLauncher::DeleteReader( TReaderID /*aReaderID*/ )
    {
    _WIMTRACE(_L("WIM|SwimReader|CSwimReaderLauncher::DeleteReaderL|Begin"));
    delete iReader;
    iReader = NULL;
    }

// End of File
