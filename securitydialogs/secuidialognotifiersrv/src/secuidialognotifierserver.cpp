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
* Description:  Secui dialog notifier server
*
*/

#include "secuidialognotifierserver.h"   // CSecuiDialogNotifierServer
#include "secuidialognotifiersession.h"  // CSecuiDialogNotifierSession
#include "secuidialognotifierservername.h" // KSecuiDialogNotifierServerName
#include "secuidialogstrace.h"           // TRACE macro


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

const CPolicyServer::TPolicy KSecuiDialogNotifierServerPolicy =
    {
    CPolicyServer::EAlwaysPass,
    KRangeCount,
    KRanges,
    KElementsIndex,
    KPolicyElements
    };


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CSecuiDialogNotifierServer::NewLC()
// ---------------------------------------------------------------------------
//
CSecuiDialogNotifierServer* CSecuiDialogNotifierServer::NewLC()
    {
    CSecuiDialogNotifierServer* self = new( ELeave ) CSecuiDialogNotifierServer;
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifierServer::~CSecuiDialogNotifierServer()
// ---------------------------------------------------------------------------
//
CSecuiDialogNotifierServer::~CSecuiDialogNotifierServer()
    {
    RDEBUG("0", 0);
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifierServer::AddSession()
// ---------------------------------------------------------------------------
//
void CSecuiDialogNotifierServer::AddSession()
    {
    RDEBUG("iSessionCount", iSessionCount);
    ++iSessionCount;
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifierServer::RemoveSession()
// ---------------------------------------------------------------------------
//
void CSecuiDialogNotifierServer::RemoveSession()
    {
    RDEBUG("iSessionCount", iSessionCount);
    --iSessionCount;
    if( iSessionCount == 0 )
        {
        RDEBUG("CActiveScheduler::Stop 0", 0);
        CActiveScheduler::Stop();
        }
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifierServer::NewSessionL()
// ---------------------------------------------------------------------------
//
CSession2* CSecuiDialogNotifierServer::NewSessionL( const TVersion &aVersion,
        const RMessage2& /*aMessage*/ ) const
    {
    RDEBUG("0", 0);
    TVersion version( KSecuiDialogNotifierServerMajorVersionNumber,
            KSecuiDialogNotifierServerMinorVersionNumber,
            KSecuiDialogNotifierServerBuildVersionNumber );
    if( !User::QueryVersionSupported( version, aVersion ) )
        {
        RDEBUG("KErrNotSupported", KErrNotSupported);
        User::Leave( KErrNotSupported );
        }

    return CSecuiDialogNotifierSession::NewL();
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifierServer::CSecuiDialogNotifierServer()
// ---------------------------------------------------------------------------
//
CSecuiDialogNotifierServer::CSecuiDialogNotifierServer() :
        CPolicyServer( CActive::EPriorityStandard, KSecuiDialogNotifierServerPolicy )
    {
    RDEBUG("0", 0);
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifierServer::ConstructL()
// ---------------------------------------------------------------------------
//
void CSecuiDialogNotifierServer::ConstructL()
    {
    StartL( KSecuiDialogNotifierServerName );
    }

