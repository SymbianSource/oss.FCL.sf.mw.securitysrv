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
* Description:   The header file of DevTokenInterfaceFactory
*
*/



#ifndef __DEVTOKENINTERFACEFACTORIES_H__
#define __DEVTOKENINTERFACEFACTORIES_H__

#include "DevTokenClientSession.h"

class MCTTokenInterface;
class MCTToken;

/**
 * Factory class to create the appropriate token interface object according
 * to the UID required
 * 
 *  @lib DevTokenClient.dll
 *  @since S60 v3.2
*/
class DevTokenInterfaceFactory
    {
    public:
    
        static MCTTokenInterface* ClientInterfaceL(TInt32 aUid, MCTToken& aToken, RDevTokenClientSession& aClient);
    };

#endif // __DEVTOKENINTERFACEFACTORIES_H__

//EOF

