/*
* ============================================================================
*  Name        : SimLockUIDocument.cpp
*  Part of     : Sim Lock UI Application
*  Description : Implementation of Sim Lock UI Application
*  Version     : 
*  
* Copyright (c) 2005-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   Build info file for Ado domain appinstall 
*
* ============================================================================
*/

// System Include Files
#include <rmmcustomapi.h>           // RMmCustomAPI, RMobilePhone
#include <mmtsy_names.h>            // KMmTsyModuleName

// User Include Files
#include "simlockdatahandlingdelegate.h"
#include "simlockisaserverdefinitions.h"
#include "simlockuiappui.h"
#include "simlockuidocument.h"

// Local Constants
const TInt KTriesToConnectServer( 2 );
const TInt KTimeBeforeRetryingServerConnection( 50000 );
static const TInt KPhoneInfoIndex( 0 );


// ---------------------------------------------------------------------------
// CSimLockUIDocument::NewL
// ---------------------------------------------------------------------------
CSimLockUIDocument* CSimLockUIDocument::NewL( CEikApplication& aApp )
    {
    CSimLockUIDocument* self = NewLC( aApp );
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// CSimLockUIDocument::NewLC
// ---------------------------------------------------------------------------
CSimLockUIDocument* CSimLockUIDocument::NewLC( CEikApplication& aApp )
    {
    CSimLockUIDocument* self = new ( ELeave ) CSimLockUIDocument( aApp );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------------------------
// CSimLockUIDocument::~CSimLockUIDocument
// ---------------------------------------------------------------------------
CSimLockUIDocument::~CSimLockUIDocument()
    {
    // Close phone
    if ( iPhone.SubSessionHandle() )
        {
        iPhone.Close();
        }

    // Close custom phone
    if ( iCustomPhone.SubSessionHandle() )
        {
        iCustomPhone.Close();
        }

    // Close ETel connection
    if ( iServer.Handle() )
        {
        iServer.UnloadPhoneModule( KMmTsyModuleName );
        iServer.Close();
        }

    // Delete simlock delegate
    delete iSimLockDelegate;
    }

// ---------------------------------------------------------------------------
// CSimLockUIDocument::CreateAppUiL
// ---------------------------------------------------------------------------
CEikAppUi* CSimLockUIDocument::CreateAppUiL()
    {
    // Create the application user interface, and return a pointer to it,
    // the framework takes ownership of this object
    CEikAppUi* appUi = new(ELeave)CSimLockUIAppUi( *iSimLockDelegate );
    return appUi;
    }

// ---------------------------------------------------------------------------
// CSimLockUIDocument::ConstructL
// ---------------------------------------------------------------------------
void CSimLockUIDocument::ConstructL()
    {
    RTelServer::TPhoneInfo phoneInfo;

    TInt error( KErrGeneral );

    // Connect to ETel server
    // All server connections are tried to be made KTriesToConnectServer times because occasional
    // fails on connections are possible, at least on some servers.
    for ( TInt thisTry=0; thisTry<KTriesToConnectServer; thisTry++ )
        {
        error = iServer.Connect();
        if ( error == KErrNone )
            {
            break;
            }
            
        // Very small delay.  Does not have negative impact on UI.  Justifiable as workaround
        // for potential failure.
        User::After( KTimeBeforeRetryingServerConnection );
        }
    User::LeaveIfError( error );

    // load TSY module
    error = iServer.LoadPhoneModule( KMmTsyModuleName );
    if ( error != KErrAlreadyExists )
        {
        // May also return KErrAlreadyExists if something else
        // has already loaded the TSY module. And that is
        // not an error.
        User::LeaveIfError( error );
        }

    // Set TSY paramaters and open RPhone handle, then RMobilePhone handle
    User::LeaveIfError( iServer.SetExtendedErrorGranularity( RTelServer::EErrorExtended ) );
    User::LeaveIfError( iServer.GetPhoneInfo( KPhoneInfoIndex, phoneInfo ) );
    User::LeaveIfError( iPhone.Open( iServer, phoneInfo.iName ) );
    User::LeaveIfError( iCustomPhone.Open( iPhone ) );

    // Create SimLock Data Handling Delegate
    iSimLockDelegate = CSimLockDataHandlingDelegate::NewL( iCustomPhone );
    }

// ---------------------------------------------------------------------------
// CSimLockUIDocument::CSimLockUIDocument
// ---------------------------------------------------------------------------
CSimLockUIDocument::CSimLockUIDocument( CEikApplication& aApp )
    : CAknDocument( aApp )
    {
    // no implementation required
    }

// end of file.

