/*
* Copyright (c) 2003-2009 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  WIM Response functions. Complete messages from client.
*
*/


// INCLUDE FILES
#include    "WimResponse.h"
#include    "WimUtilityFuncs.h"
#include    "WimTrace.h"


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CWimResponse::CWimResponse( RMessage2 aMessage )
    : iMessage( aMessage ),
      iStatus( WIMI_ERR_Internal ),
      iError( 0 )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimResponse::CWimResponse | Begin"));
    }

// Destructor
CWimResponse::~CWimResponse()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimResponse::~CWimResponse | Begin"));
    if( !iMessage.IsNull() )
        {
        if( iError ) // Check if any intenal error
            {
            iMessage.Complete( iError );
            }
        else
            {
            // Set the status of the WIM Call
            iMessage.Complete( CWimUtilityFuncs::MapWIMError( iStatus ) );
            }
        }
    }

// -----------------------------------------------------------------------------
// CWimResponse::CompleteMsgAndDelete
// Delete message. Message is completed in destructor so just delete response.
// -----------------------------------------------------------------------------
//
void CWimResponse::CompleteMsgAndDelete()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimResponse::CompleteMsgAndDelete | Begin"));
    delete this;
    }

// End of File
