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
* Description:   The header file of TrustedSiteConduit
*
*/



#ifndef __TRUSTEDSITESCONDUIT_H__
#define __TRUSTEDSITESCONDUIT_H__

#include "DevTokenCliServ.h"
#include "DevTokenMarshaller.h"
#include "DevTokenDataTypes.h"

class CTrustedSitesServer;

/**
* Trust sites store implementation in server side
* 
 *  @lib 
 *  @since S60 v3.2
*/
class CTrustedSitesConduit : public CBase
    {
    public:
    
        static CTrustedSitesConduit* NewL(CTrustedSitesServer& aServer);
    
        virtual ~CTrustedSitesConduit();
    
        void ServiceTrustedSitesRequestL(const RMessage2& aMessage);

    private:
    
        void AddL( const RMessage2& aMessage );
    
        TInt IsTrustedSiteL( const RMessage2& aMessage );   
    
        void GetTrustedSitesL( const RMessage2& aMessage ); 

        void AddForgivenSiteL(const RMessage2& aMessage );
        
        void RemoveForgivenSiteL( const RMessage2& aMessage );
        
        TInt IsOutOfDateAllowedL(const RMessage2& aMessage); 
    private:
     
        CTrustedSitesConduit(CTrustedSitesServer& aServer);
     
        HBufC8* AllocResponseBufferLC(TInt aSize, const RMessage2& aMessage);
     
        //  No copying
        CTrustedSitesConduit(const CTrustedSitesConduit&);      
     
        //  No copying
        CTrustedSitesConduit& operator=(const CTrustedSitesConduit&); 

    private:
      
        CTrustedSitesServer& iServer;
    };

#endif  //__TRUSTEDSITESCONDUIT_H__

//EOF

