/*
* Copyright (c) 2003 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Definitions for client-server communication
*
*/



#ifndef CSCARDCLSV_H
#define CSCARDCLSV_H

//  INCLUDES
#include <e32base.h>

// CONSTANTS
// A version must be specifyed when creating a session with the server
const TUint KScardServMajorVersionNumber = 0;
const TUint KScardServMinorVersionNumber = 1;
const TUint KScardServBuildVersionNumber = 1;
// server name
_LIT( KScardServerName, "ScardServer" );

// DATA TYPES
// opcodes used in message passing between client and server
enum TScardServRqst
    {
    //  CScardComm opcodes
    EScardServerCloseSession,
    EScardServerNotifyChange,
    EScardServerCancelNotifyChange,
    EScardServerConnectToReader,
    EScardServerDisconnectFromReader,
    EScardServerTransmitToCard,
    EScardServerCancelTransmit,
    EScardServerGetATR,
    EScardServerGetCapabilities,
    EScardServerManageChannel
    };

// FUNCTION PROTOTYPES
IMPORT_C TInt StartThread();

#endif      // CSCARDCLSV_H

// End of File
