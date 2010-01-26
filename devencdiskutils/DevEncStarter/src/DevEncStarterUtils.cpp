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
* Description:  DevEncStarter utilities
*
*/


#include "DevEncStarterUtils.h"
#include "DevEncLog.h"

// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CDevEncStarterUtils::CDevEncStarterUtils()
// ----------------------------------------------------------
//
CDevEncStarterUtils::~CDevEncStarterUtils()
    {
    if (iAppStarter)
        {
        delete iAppStarter;
        iAppStarter = NULL;
        }
    }

// ----------------------------------------------------------
// CDevEncStarterUtils::NewL
// ----------------------------------------------------------
//
CDevEncStarterUtils* CDevEncStarterUtils::NewL()
    {
    CDevEncStarterUtils* self = new ( ELeave ) CDevEncStarterUtils();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// ----------------------------------------------------------
// CDevEncStarterUtils::CDevEncStarterUtils()
// ----------------------------------------------------------
//
CDevEncStarterUtils::CDevEncStarterUtils()
    {
    
    }

// ----------------------------------------------------------
// CDevEncStarterUtils::ConstructL()
// ----------------------------------------------------------
//
void CDevEncStarterUtils::ConstructL()
    {
    
    }

// ----------------------------------------------------------
// CDevEncStarterUtils::StartAppL()
//
// ----------------------------------------------------------
//
void CDevEncStarterUtils::StartAppL( const TUint32& aUid,
                                     MUtilsCallback* aCallback )
    {
    DFLOG( "CDevEncStarterUtils::StartAppL" );
    if ( aCallback )
        {
        // The calling class wants to know when the application is started
        if ( iCallback )
            {
            User::Leave( KErrInUse );
            }
        else
            {
            // Used in CDevEncStarterUtils::AppStarted()
            DFLOG( "Callback stored" );
            iCallback = aCallback;
            }
        }

    if ( !iAppStarter )
        {
        iAppStarter = CDevEncStarterStarter::NewLC( this );
        CleanupStack::Pop( iAppStarter ); // member variable, no cleanupstack
        DFLOG( "Starter created" );
        }

    if ( !iAppStarter->IsActive() )
        {
        iAppStarter->StartAppL( aUid );
        DFLOG( "StartAppL called" );
        }
    else
        {
        DFLOG( "CDevEncStarterUtils::StartAppL: App starter already active" );
        }
    }


// ----------------------------------------------------------
// CDevEncStarterUtils::AppStarted()
//
// ----------------------------------------------------------
//
void CDevEncStarterUtils::AppStarted( const TUint32& aUid )
    {
    DFLOG( "CDevEncStarterUtils::AppStarted" );
    if ( iCallback )
        {
        DFLOG2( "CDevEncStarterUtils::AppStarted %x", aUid );
        iCallback->AppStarted( aUid );
        iCallback = NULL;
        }
    }

// End of file

