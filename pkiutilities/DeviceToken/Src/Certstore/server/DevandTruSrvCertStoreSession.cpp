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
* Description:   Implementation of DevandTruSrvCertStoreSession
*
*/



#include "DevandTruSrvCertStoreSession.h"
#include "DevandTruSrvCertStoreConduit.h"


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreSession::NewL()
// ---------------------------------------------------------------------------
//
CDevandTruSrvCertStoreSession* CDevandTruSrvCertStoreSession::NewL(CDevandTruSrvCertStoreConduit& aConduit)
    {
    return new (ELeave) CDevandTruSrvCertStoreSession(aConduit);
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreSession::CDevandTruSrvCertStoreSession()
// ---------------------------------------------------------------------------
//	
CDevandTruSrvCertStoreSession::CDevandTruSrvCertStoreSession(CDevandTruSrvCertStoreConduit& aConduit) :
    iConduit(aConduit)
    {
    }


// ---------------------------------------------------------------------------
// CDevandTruSrvCertStoreSession::DoServiceL()
// ---------------------------------------------------------------------------
//
void CDevandTruSrvCertStoreSession::DoServiceL(const RMessage2& aMessage)
    {
    iConduit.ServiceCertStoreRequestL(aMessage);
    }

//EOF


