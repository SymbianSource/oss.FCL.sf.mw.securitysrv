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
* Description:   The header file of TrustedSitesServer
*
*/



#ifndef __TRUSTEDSITESSERVER_H__
#define __TRUSTEDSITESSERVER_H__

#include <s32file.h>
#include <e32std.h>
#include <securitydefs.h>
#include <d32dbms.h>

class CTrustedSitesSession;
class CTrustedSitesConduit;
class CDevTokenServer;
/**
* Trust sites store implementation in server side
* 
 *  @lib 
 *  @since S60 3.2
*/
class CTrustedSitesServer : public CBase
    {
    public:
        
        static CTrustedSitesServer* NewL( CDevTokenServer* aServer );
        
        virtual ~CTrustedSitesServer();
        
        CTrustedSitesSession* CreateSessionL();

    public:

        /**
         * add new trust record to trust site store
         *
         * @param aCert a certificate
         * @param aSiteName trusted site name
         * @param aMessage IPC message
         * @return 
         */
        void AddL(const TDesC8& aCert, const TDesC& aSiteName, const RMessage2& aMessage );
        
        /**
         * add new trust record to trust site store, this is called internally in DeviceToken server
         *
         * @param aCert a certificate
         * @param aSiteName trusted site name
         * @return 
         */
        void AddL(const TDesC8& aCertHash, const TDesC& aSiteName );

        /**
         * check whether the given cert is trusted for the given site
         *
         * @param aCert a certificate
         * @param aSiteName trusted site name
         * @param aCertBuf
         * @param aMessage IPC message
         * @return ETrue /EFalse
         */
        TBool IsTrustedSiteL( const TDesC8& aCert, const TDesC& aSiteName, const TDesC8& aCertBuf, const RMessage2& aMessage );
        
        
        /**
         * check whether the out of date certificate is allowed for the forgiven sites
         *
         * @param aCert a certificate
         * @param aSiteName trusted site name
         * @param aCertBuf
         * @param aMessage IPC message
         * @return ETrue /EFalse
         */
        TBool IsOutOfDateAllowedL( const TDesC8& aCertHash, const TDesC& aSiteName, const RMessage2& aMessage );

        /**
         * list all of the sites for which the cert is trusted
         *
         * @param aCert a certificate
         * @param aListOfSites trusted sites' name
         * @param aMessage IPC message
         * @return ETrue /EFalse
         */
        void GetTrustedSitesL( const TDesC8& aCert, RPointerArray<HBufC>& aListOfSites, const RMessage2& aMessage);

        /**
         * Remove trust record from trust site store
         *
         * @param aCert a certificate
         * @return 
         */
        void RemoveL( const TDesC8& aCert );
        
        /**
         * Add forgiven site. The site will be forgiven for untrusted certificate
         *
         * @param aSite
         * @param aOutOfDateAllowed
         * @param aMessage IPC message
         */
        void AddForgivenSiteL(const TDesC& aSite,const TBool& aOutOfDateAllowed, const RMessage2& aMessage );
        
        /**
         * Remove all forgiven sites set by this client
         *
         * @param aMessage IPC message
         */
        void RemoveAllForgivenSitesL( const RMessage2& aMessage );
        
        
        /**
         * Remove the given site 
         * 
         * @param aMessage IPC message
         */
        void RemoveThisForgivenSiteL( const TDesC& aSite, const RMessage2& aMessage );

    private:
        
        CTrustedSitesServer( CDevTokenServer* aServer );
        
        void ConstructL();
        
        void CreateDBL();
        
        TBool IsSiteAlreadyExistL(const TInt& aCertID, const TDesC& aSiteName);
        
        TBool IsOutOfDateAllowedForTheSiteL(const TInt& aCertID, const TDesC& aSiteName);
        
        void DoAddSiteNameL(const TInt& aCertID, const TDesC& aSiteName );
        
        void DoAddCertL( const TDesC8& aCertHash );
        
        void GetCertIDL( const TDesC8& aCertHash, TInt& aCertID );
        
        void DoRemoveL(const TInt& aCertID );
        
        void DoGetTrustedSitesL( const TInt& aCertID, RPointerArray<HBufC>& aListOfSites);   
        
        const TSecurityPolicy& AddRemovePolicy() const;
        
        const TSecurityPolicy& ReadPolicy() const;
        
        void DoOverWriteOutOfDateL( const TDesC& aSite, const TBool& aOutOfDateAllowed );
        
        void DoOverWriteCertL( const TDesC& aSiteName, const TDesC8& aCertHash );

        void DoAddForgivenSiteL(const TDesC& aSite, const TBool& aOutOfDateAllowed, const TUint32& aUID );
        
        TBool IsForgivenSiteAlreadyL(const TDesC& aSite );

    private:
        
        // Conduit for marshalling/unmarshalling client communications
        CTrustedSitesConduit* iConduit;
        
        RFs iFs;
        
        RDbNamedDatabase iDatabase;
        
        CDevTokenServer* iServer; //not owned
    };

#endif //_TRUSTEDSITESSERVER_H__

//EOF

