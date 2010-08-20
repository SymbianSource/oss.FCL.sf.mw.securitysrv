/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Security dialog notifier server
*
*/

#include "securitydialognotifierserver.h"   // CSecurityDialogNotifierServer
#include "securitydialognotifiersession.h"  // CSecurityDialogNotifierSession
#include "securitydialognotifierservername.h" // KSecurityDialogNotifierServerName
#include "securitydialogstrace.h"           // TRACE macro


const TUint KRangeCount = 1;

const TInt KRanges[ KRangeCount ] =
    {
    0
    };

const TUint8 KElementsIndex[ KRangeCount ] =
    {
    0
    };

const TInt KUikonServerId = 0x10003a4a;
const CPolicyServer::TPolicyElement KPolicyElements[] =
    {
    { _INIT_SECURITY_POLICY_S0( KUikonServerId ), CPolicyServer::EFailClient }
    };

const CPolicyServer::TPolicy KSecurityDialogNotifierServerPolicy =
    {
    CPolicyServer::EAlwaysPass,
    KRangeCount,
    KRanges,
    KElementsIndex,
    KPolicyElements
    };


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CSecurityDialogNotifierServer::NewLC()
// ---------------------------------------------------------------------------
//
CSecurityDialogNotifierServer* CSecurityDialogNotifierServer::NewLC()
    {
    CSecurityDialogNotifierServer* self = new( ELeave ) CSecurityDialogNotifierServer;
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------------------------
// CSecurityDialogNotifierServer::~CSecurityDialogNotifierServer()
// ---------------------------------------------------------------------------
//
CSecurityDialogNotifierServer::~CSecurityDialogNotifierServer()
    {
    TRACE( "CSecurityDialogNotifierServer::~CSecurityDialogNotifierServer" );
    }

// ---------------------------------------------------------------------------
// CSecurityDialogNotifierServer::AddSession()
// ---------------------------------------------------------------------------
//
void CSecurityDialogNotifierServer::AddSession()
    {
    TRACE( "CSecurityDialogNotifierServer::AddSession" );
    ++iSessionCount;
    }

// ---------------------------------------------------------------------------
// CSecurityDialogNotifierServer::RemoveSession()
// ---------------------------------------------------------------------------
//
void CSecurityDialogNotifierServer::RemoveSession()
    {
    TRACE( "CSecurityDialogNotifierServer::RemoveSession" );
    --iSessionCount;
    if( iSessionCount == 0 )
        {
        TRACE( "CSecurityDialogNotifierServer::RemoveSession, shutting down" );
        CActiveScheduler::Stop();
        }
    }

// ---------------------------------------------------------------------------
// CSecurityDialogNotifierServer::NewSessionL()
// ---------------------------------------------------------------------------
//
CSession2* CSecurityDialogNotifierServer::NewSessionL( const TVersion &aVersion,
        const RMessage2& /*aMessage*/ ) const
    {
    TRACE( "CSecurityDialogNotifierServer::NewSessionL" );
    TVersion version( KSecurityDialogNotifierServerMajorVersionNumber,
            KSecurityDialogNotifierServerMinorVersionNumber,
            KSecurityDialogNotifierServerBuildVersionNumber );
    if( !User::QueryVersionSupported( version, aVersion ) )
        {
        TRACE( "CSecurityDialogNotifierServer::NewSessionL, version not supported" );
        User::Leave( KErrNotSupported );
        }

    return CSecurityDialogNotifierSession::NewL();
    }

// ---------------------------------------------------------------------------
// CSecurityDialogNotifierServer::CSecurityDialogNotifierServer()
// ---------------------------------------------------------------------------
//
CSecurityDialogNotifierServer::CSecurityDialogNotifierServer() :
        CPolicyServer( CActive::EPriorityStandard, KSecurityDialogNotifierServerPolicy )
    {
    TRACE( "CSecurityDialogNotifierServer::CSecurityDialogNotifierServer" );
    }

// ---------------------------------------------------------------------------
// CSecurityDialogNotifierServer::ConstructL()
// ---------------------------------------------------------------------------
//
void CSecurityDialogNotifierServer::ConstructL()
    {
    StartL( KSecurityDialogNotifierServerName );
    }

