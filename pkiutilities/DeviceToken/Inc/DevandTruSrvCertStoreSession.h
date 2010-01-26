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
* Description:   The header file of DevandTruSrvCertStoreSession
*
*/



#ifndef __DEVANDTRUSRVCERSTORESESSION_H__
#define __DEVANDTRUSRVCERSTORESESSION_H__

#include "DevTokenServer.h"

class RMessage2;
class CDevandTruSrvCertStoreConduit;

/**
 * A cert store session
 *
 * Handles client requests by passing them to the cert store conduit.
 *
 *  @lib 
 *  @since S60 v3.2
 */
class CDevandTruSrvCertStoreSession : public CDevTokenServerSession
    {
    public:
        
        /**
         * Create a new session object.
         *
         * @param aConduit The cert store conduit used to service user requests
         * @return 
         */
        static CDevandTruSrvCertStoreSession* NewL(CDevandTruSrvCertStoreConduit& aConduit);

    private:
        
        CDevandTruSrvCertStoreSession(CDevandTruSrvCertStoreConduit& aConduit);
        
        virtual void DoServiceL(const RMessage2& aMessage);
        
    private:
        
        CDevandTruSrvCertStoreConduit& iConduit;
    };

#endif //__DEVANDTRUSRVCERSTORESESSION_H__

//EOF

