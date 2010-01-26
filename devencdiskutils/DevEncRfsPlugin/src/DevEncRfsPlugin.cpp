/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: 
*     
*
*/


// INCLUDE FILES
#include "DevEncRfsPlugin.h"

#ifdef _DEBUG
#define DEBUG( args... ) RDebug::Print( args )
#define DEBUGRAW( rawdata ) RDebug::RawPrint( rawdata )
#else
#define DEBUG( args... )
#define DEBUGRAW( rawdata )
#endif

// LOCAL CONSTANTS
_LIT(KScriptPath, "");

// ================= MEMBER FUNCTIONS =======================

// C++ default constructor can NOT contain any code, that
// might leave.
//
CDevEncRfsPlugin::CDevEncRfsPlugin()
    {
    }

CDevEncRfsPlugin::CDevEncRfsPlugin( TAny* /*aInitParams*/ )
    { 
    }

// Destructor
CDevEncRfsPlugin::~CDevEncRfsPlugin()
    {
    delete iKeyUtils;
    }

// ---------------------------------------------------------
// NewL
// ---------------------------------------------------------
//
CDevEncRfsPlugin* CDevEncRfsPlugin::NewL( TAny* aInitParams )
    {
    DEBUG( _L("CDevEncRfsPlugin::NewL()") );
    CDevEncRfsPlugin* self = new (ELeave) CDevEncRfsPlugin( aInitParams );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }


// ---------------------------------------------------------
// ConstructL
// ---------------------------------------------------------
//
void CDevEncRfsPlugin::ConstructL()
    {
    DEBUG( _L("CDevEncRfsPlugin::ConstructL()") );
    iKeyUtils  = new (ELeave) CDevEncKeyUtils();
    }

void CDevEncRfsPlugin::RestoreFactorySettingsL( const TRfsReason aType )
    {
    DEBUG(_L("CDevEncRfsPlugin::RestoreFactorySettingsL(%d)"), aType);
    
    if( aType != EInitRfs )
    	  {
        TRequestStatus status;
        iKeyUtils->ResetKey( status );
        User::WaitForRequest( status );
        }
    }

void CDevEncRfsPlugin::GetScriptL( const TRfsReason /*aType*/, TDes& aPath )
    {
    aPath.Copy( KScriptPath );
    }

void CDevEncRfsPlugin::ExecuteCustomCommandL( const TRfsReason /*aType*/,
                                              TDesC& /*aCommand*/ )
    {
    }

// End of file
