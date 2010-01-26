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
* Description:   The header file of DevandTruSrvCertStoreConduit
*
*/



#ifndef __DEVANDTRUSRVCERTSTORECONDUIT_H__
#define __DEVANDTRUSRVCERTSTORECONDUIT_H__

#include "DevTokenCliServ.h"
#include "DevTokenMarshaller.h"
#include "DevTokenDataTypes.h"
#include <ccertattributefilter.h>

class CDevandTruSrvCertStoreServer;

/**
 *  class CDevandTruSrvCertStoreConduit
 *
 * Unmarshalls incoming client request data and uses it to execute certstore
 * operations.  Marshalls the return data into a suitable format to pass back to
 * to the client
 *
 *  @lib 
 *  @since S60 v3.2
 */
class CDevandTruSrvCertStoreConduit : public CBase
    {
    public:
        
        static CDevandTruSrvCertStoreConduit* NewL(CDevandTruSrvCertStoreServer& aServer);
        
        virtual ~CDevandTruSrvCertStoreConduit();
        
        void ServiceCertStoreRequestL(const RMessage2& aMessage);

    private:
        
        // For MCTCertStore
        void ListCertsL(const RMessage2& aMessage);
        
        void GetCertL(const RMessage2& aMessage);
        
        void ApplicationsL(const RMessage2& aMessage);
        
        TInt IsApplicableL(const RMessage2& aMessage);
        
        TInt TrustedL(const RMessage2& aMessage);
        
        void RetrieveL(const RMessage2& aMessage);

        // For MCTWritableCertStore
        void AddCertL(const RMessage2& aMessage);
        
        void RemoveCertL(const RMessage2& aMessage);
        
        void SetApplicabilityL(const RMessage2& aMessage);
        
        void SetTrustL(const RMessage2& aMessage);

    private:
        
        CDevandTruSrvCertStoreConduit(CDevandTruSrvCertStoreServer& aServer);
        
        HBufC8* AllocResponseBufferLC(TInt aSize, const RMessage2& aMessage);
        
        CDevandTruSrvCertStoreConduit(const CDevandTruSrvCertStoreConduit&);      //  No copying
        
        CDevandTruSrvCertStoreConduit& operator=(const CDevandTruSrvCertStoreConduit&); //  No copying

    private:
        
        // The cert store server used to fulfill client requests
        CDevandTruSrvCertStoreServer& iServer;
    };

#endif  //  __DEVANDTRUSRVCERTSTORECONDUIT_H__

//EOF

