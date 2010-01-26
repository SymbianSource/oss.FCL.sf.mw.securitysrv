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
* Description:   The header file of DevTokenClientSession
*
*/



#ifndef __DEVTOKENCLIENTSESSION_H__
#define __DEVTOKENCLIENTSESSION_H__

#include <e32base.h>
#include "DevTokenCliServ.h"

/**
 * Makes SendReceive requests on the filetokens server
 *
 *  @lib DevTokenClient.dll
 *  @since S60 v3.2
*/
class RDevTokenClientSession : public RSessionBase
    {
    public:
    
        RDevTokenClientSession();
    
        static RDevTokenClientSession* ClientSessionL();
    
        TInt Connect(EDevTokenEnum aTokenEnum);
    
        TInt SendRequest(TDevTokenMessages aRequest, const TIpcArgs& aArgs) const;
    
        void SendAsyncRequest(TDevTokenMessages aRequest, const TIpcArgs& aArgs, TRequestStatus* aStatus) const;
    };

#endif  //  __DEVTOKENCLIENTSESSION_H__

//EOF

