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
* Description:   The header file of TrustedSitesSession
*
*/



#ifndef __TRUSTEDSITESSESSION_H__
#define __TRUSTEDSITESSESSION_H__

#include "DevTokenServer.h"

class RMessage2;
class CTrustedSitesConduit;

/**
 * trusted sites session
 *
 * The class implementes in the server side to
 * handle the trust site session
 * 
 *  @lib 
 *  @since S60 v3.2
 */
class CTrustedSitesSession : public CDevTokenServerSession
    {
    public:
    
        static CTrustedSitesSession* NewL(CTrustedSitesConduit& aConduit);

    private:
    
        CTrustedSitesSession(CTrustedSitesConduit& aConduit);
    
        virtual void DoServiceL(const RMessage2& aMessage);

    private:
    
        CTrustedSitesConduit& iConduit;
    };

#endif //__TRUSTEDSITESSESSION_H__

//EOF

