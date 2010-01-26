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
* Description:  Definitions and structures for smart card server.
*
*/



#ifndef CSCARDSERVERBASE_H
#define CSCARDSERVERBASE_H

// INCLUDE FILES
#include <e32base.h>

// ENUMERATIONS
enum TSpecialSessionIDs 
    {
    ENoSession      = 0, //-1,
    EPollSessionID  = -2,
    EAccessMasterID = -3
    };

// CONSTANTS

//needed for creating server thread.
const TUint KDefaultMinHeapSize = 0x1000;
const TUint KDefaultMaxHeapSize = 0x100000;

const TInt8 KChannel0       = 0;
const TInt8 KAllChannels    = -1;

const TInt8 KMaxChannels    = 0x04;

const TUint8 KManageChannel = 0x70;
const TUint8 KOpenChannel   = 0x00;
const TUint8 KCloseChannel  = 0x80;

const TUint8 KReservation   = 0x01;
const TUint8 KClosing       = 0x02;
const TUint8 KConnection    = 0x80;


// MACROS
#define __ASSERT_MEMORY( err ) if ( ( err ) )\
    User::Panic( _L( "Not enough free memory" ), KScPanicNoMemory )

#endif      // CSCARDSERVERBASE_H

// End of File
