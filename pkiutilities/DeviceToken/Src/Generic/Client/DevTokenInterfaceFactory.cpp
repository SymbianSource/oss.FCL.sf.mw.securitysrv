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
* Description:   Implementation of DevTokenInterfaceFactory
*
*/



#ifdef SYMBIAN_ENABLE_SPLIT_HEADERS
#include <mctkeystoreuids.h>
#endif
#include <e32debug.h>
#include "DevTokenInterfaceFactory.h"
#include "DevCertKeyStore.h"
#include "DevandTruSrvCertStore.h"
#include "DevTokenImplementationUID.hrh"
#include "DevtokenLog.h"

// ======== MEMBER FUNCTIONS ========

// -----------------------------------------------------------------------------
// DevTokenInterfaceFactory::ClientInterfaceL()
// -----------------------------------------------------------------------------
//
MCTTokenInterface* DevTokenInterfaceFactory::ClientInterfaceL(
               TInt32 aUid, 
               MCTToken& aToken, 
               RDevTokenClientSession& aClient)
    {
    MCTTokenInterface* tokenInterface = NULL; 
    TRACE_PRINT("ClientInterfaceL --> 1");
    switch ( aUid )
         {
         case KInterfaceKeyStore:
             TRACE_PRINT("ClientInterfaceL --> 4");
             tokenInterface = CDevCertKeyStore::NewKeyStoreUserInterfaceL(aToken, aClient);
             TRACE_PRINT("ClientInterfaceL --> 5");
             break;

         case KInterfaceKeyStoreManager:
             TRACE_PRINT("ClientInterfaceL --> 2");
             tokenInterface = CDevCertKeyStore::NewKeyStoreManagerInterfaceL(aToken, aClient);
             TRACE_PRINT("ClientInterfaceL --> 3");
             break;

         case KInterfaceCertStore:
             tokenInterface  = CDevandTruSrvCertStore::NewReadableInterfaceL( aToken, aClient );
             break;

         case KInterfaceWritableCertStore: 
             tokenInterface = CDevandTruSrvCertStore::NewWritableInterfaceL( aToken, aClient );
             break;

         default:
             User::Leave(KErrArgument);
             break;
         }

    return (tokenInterface);
    }
//EOF

