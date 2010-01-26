/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Implementation of TrustedSitesStoreSession
*
*/



#include "TrustedSitesSession.h"
#include "TrustedSitesConduit.h"


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CTrustedSitesSession::NewL()
// ---------------------------------------------------------------------------
//
CTrustedSitesSession* CTrustedSitesSession::NewL(CTrustedSitesConduit& aConduit)
    {
    return new (ELeave) CTrustedSitesSession(aConduit);
    }


// ---------------------------------------------------------------------------
// CTrustedSitesSession::CTrustedSitesSession()
// ---------------------------------------------------------------------------
//
CTrustedSitesSession::CTrustedSitesSession(CTrustedSitesConduit& aConduit) :
    iConduit(aConduit)
    {
    }


// ---------------------------------------------------------------------------
// CTrustedSitesSession::DoServiceL()
// ---------------------------------------------------------------------------
//
void CTrustedSitesSession::DoServiceL(const RMessage2& aMessage)
    {
    iConduit.ServiceTrustedSitesRequestL(aMessage);
    }
    
//EOF
