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
* Description:   The header file of DevTokenServer
*
*/



#ifndef __DEVTOKENSERVER_H__
#define __DEVTOKENSERVER_H__


#include <e32base.h>
#include "DevTokenCliServ.h"
#include "DevTokenTypesEnum.h"

class CDevCertKeyStoreServer;
class CDevandTruSrvCertStoreServer;
class CTrustedSitesServer;

/**
 * Implements shutdown of the server.  When the last client disconnects, this
 * class is activated, and when the timer expires, causes the server to
 * close.
 *
 *  @lib
 *  @since S60 v3.2
 */
class CShutdown : public CTimer
    {  
    public:
    
        inline CShutdown();
        
        inline void ConstructL();
        
        inline void Start();
        
    private:
        
        void RunL();
    };


/** 
 * Device Tokens server class, manages sessions. 
 * 
 *  @lib
 *  @since S60 v3.2
 */
class CDevTokenServer : public CServer2
    {
    public:
        
        static CServer2* NewLC();
        
        ~CDevTokenServer();
        
        void AddSession();
        
        void DropSession();
        
        /**
         * return the server object of keystore
         *
         * @param 
         * @return server object of keystore
         */
        CDevCertKeyStoreServer& KeyStoreServerL() const;
        
        /**
         * return the server object of certstore
         *
         * @param 
         * @return server object of certstore
         */
        CDevandTruSrvCertStoreServer& CertStoreServerL() const;
        
        /**
         * return the server object of Trusted Sites
         *
         * @param 
         * @return server object of Truste sites
         */
        CTrustedSitesServer& CDevTokenServer::TrustedSitesL() const;
        
    // For CServer2
    private:

        virtual CSession2* NewSessionL(const TVersion& aVersion, const RMessage2& aMessage) const;
    
    private:
    
        CDevTokenServer();
       
        void ConstructL();
    
    private:
       
        TInt iSessionCount;
       
        CShutdown iShutdown;
       
        mutable CDevCertKeyStoreServer* iKeyStoreServer;
       
        mutable CDevandTruSrvCertStoreServer* iCertStoreServer;
       
        mutable CTrustedSitesServer* iTrustedSitesServer;
    };

/**
 * Base class for session objects.
 *
 *  @lib
 *  @since S60 v3.2
 */
class CDevTokenServerSession : public CSession2
    {
    public:
       
        CDevTokenServerSession();
       
        virtual ~CDevTokenServerSession();
       
        inline CDevTokenServer& Server();
    
    protected:
       
        virtual void DoServiceL(const RMessage2& aMessage) = 0;
    
    private:
       
        virtual void CreateL();
       
        virtual void ServiceError(const RMessage2& aMessage, TInt aError);
       
        virtual void ServiceL(const RMessage2& aMessage);
    };

#endif  //  __DEVTOKENSERVER_H__

//EOF

