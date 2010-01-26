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
* Description:  Starts applications
*
*/


#include <apgcli.h> // for RApaLsSession
#include <apacmdln.h> // for CApaCommandLine
#include "DevEncStarterAppStarter.h"
#include "DevEncLog.h"

// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CDevEncStarterStarter::NewL
// Instancies CDevEncStarterStarter object
// ----------------------------------------------------------
//
CDevEncStarterStarter* CDevEncStarterStarter::NewL( MAppStarterCallback* aCallback )
    {
    CDevEncStarterStarter* self = CDevEncStarterStarter::NewLC(aCallback);
    CleanupStack::Pop();
    return self;
    }


// ----------------------------------------------------------
// CDevEncStarterStarter::NewL
// Instancies CDevEncStarterStarter object
// ----------------------------------------------------------
//
CDevEncStarterStarter* CDevEncStarterStarter::NewLC( MAppStarterCallback* aCallback )
    {
    CDevEncStarterStarter* self = new ( ELeave ) CDevEncStarterStarter( aCallback );
    CleanupStack::PushL ( self );
    self->ConstructL();
    //CleanupStack::Pop();
    return self;
    }

// ----------------------------------------------------------
// CDevEncStarterStarter::ConstructL()
// Initializes data objects
// ----------------------------------------------------------
//
void CDevEncStarterStarter::ConstructL()
    {
    CActiveScheduler::Add( this );
    }

// ----------------------------------------------------------
// CDevEncStarterStarter::CDevEncStarterStarter()
// Constructor
// ----------------------------------------------------------
//
CDevEncStarterStarter::CDevEncStarterStarter( MAppStarterCallback* aCallback )
    : CActive( EPriorityStandard ),
      iCallback( aCallback )
    {
    }

// ----------------------------------------------------------
// CDevEncStarterStarter::CDevEncStarterStarter()
// Destructor
// ----------------------------------------------------------
//
CDevEncStarterStarter::~CDevEncStarterStarter()
    {
    Cancel();
    }

// ----------------------------------------------------------
// CDevEncStarterStarter::CanStartAppL()
//
// ----------------------------------------------------------
//
TBool CDevEncStarterStarter::CanStartAppL( const TUint32& aUid )
    {
    DFLOG2( ">>CDevEncStarterStarter::CanStartAppL %x", aUid );
    TBool result( EFalse );
    RApaLsSession ls;
    CleanupClosePushL( ls );
    User::LeaveIfError( ls.Connect() );

    TApaAppInfo appInfo;
    TInt err = ls.GetAppInfo( appInfo, TUid::Uid( aUid ) );

    if ( err == KErrNone )
        {
        result = ETrue;
        }
    CleanupStack::PopAndDestroy(); // ls
    return result;
    }

// ----------------------------------------------------------
// CDevEncStarterStarter::StartAppL()
//
// ----------------------------------------------------------
//
void CDevEncStarterStarter::StartAppL( const TUint32& aUid )
    {
    DFLOG2( ">>CDevEncStarterStarter::StartAppL %x", aUid );

    if ( this->IsActive() )
        {
        DFLOG( "Already active" );
        return;
        }

    RApaLsSession ls;
    CleanupClosePushL( ls );
    User::LeaveIfError( ls.Connect() );

    TApaAppInfo appInfo;
    TInt err = ls.GetAppInfo( appInfo, TUid::Uid( aUid ) );

    if ( err == KErrNone )
        {
        CApaCommandLine* apaCommandLine = CApaCommandLine::NewLC();
        apaCommandLine->SetExecutableNameL( appInfo.iFullName );
        apaCommandLine->SetCommandL( EApaCommandRun );
        TThreadId threadId;
        DFLOG2( "iStatus = %d", iStatus.Int() );
        TRequestStatus* status = &iStatus;
        TInt startErr = ls.StartApp( *apaCommandLine, threadId, status );
        DFLOG2( "iStatus after StartApp = %d", iStatus.Int() );
        CleanupStack::PopAndDestroy( apaCommandLine );
        if ( startErr == KErrNone )
            {
            iAppUidStarting = aUid;
            SetActive();
            }
        else
            {
            DFLOG2( "Start error %d", startErr );
            }
        }
    else
        {
        FLOG2( "GetAppInfo error %d", err );
        }
    CleanupStack::PopAndDestroy(); // ls
    DFLOG( "<<CDevEncStarterStarter::StartAppL" );
    }

// ----------------------------------------------------------
// CDevEncStarterStarter::RunError()
// CActive Object method
// ----------------------------------------------------------
//
TInt CDevEncStarterStarter::RunError( TInt aError )
    {
    DFLOG2( "CDevEncStarterStarter::RunError aError = %d", aError );
    return aError;
    }

// ----------------------------------------------------------
// CDevEncStarterStarter::RunL()
// CActive Object method
// ----------------------------------------------------------
//
void CDevEncStarterStarter::RunL()
    {
    DFLOG2( "CDevEncStarterStarter::RunL, status %d", iStatus.Int()  );
    if ( iStatus.Int() == KErrNone )
        {
        if ( iCallback )
            {
            DFLOG( "Informing callback object" );
            iCallback->AppStarted( iAppUidStarting );
            }
        else
            {
            DFLOG( "No callback object registered" );
            }
        }
    else
        {
        DFLOG( "No callback informing because of error" );
        }
    iAppUidStarting = 0;
    }

// ----------------------------------------------------------
// CDevEncStarterStarter::DoCancel()
// CActive Object method
// ----------------------------------------------------------
//
void CDevEncStarterStarter::DoCancel()
    {
    }

// End of file

