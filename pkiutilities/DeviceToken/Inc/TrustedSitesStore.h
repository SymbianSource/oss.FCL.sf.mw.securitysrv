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
* Description:   The header file of TrustedSitesStore
*
*/



#ifndef __TRUSTEDSITESSTORE_H__
#define __TRUSTEDSITESSTORE_H__

#include <e32base.h>

class RDevTokenClientSession;

/**
 * Handle trusted sites store
 *
 * The class implementes the interface to handle the trust sites store.
 * Trusted sites are stored associated with certificates, which means for this
 * site the attached certificate is trusted.
 * 
 *  @lib DevTokenClient.dll
 *  @since S60 v3.2
 */
class CTrustSitesStore : public CBase
    {
    public:  
        IMPORT_C static CTrustSitesStore* NewL();
        IMPORT_C virtual ~CTrustSitesStore();

    public: // New functions

        /**
         * add new trust record to trust site store
         *
         * @param aCert a certificate
         * @param aSiteName trusted site name
         * @return 
         */
        IMPORT_C TInt AddL( const TDesC8& aCert, const TDesC& aTrustedSite );
        
        
        /**
         * add forgiven site 
         *
         * @param aSite the untrusted certificate from this site is allowed
         * @param aIsOutOfDateAllowed outofdate certificate is allowed 
         * @return 
         */
        IMPORT_C TInt AddForgivenSiteL( const TDesC& aSite, const TBool& aIsOutOfDateAllowed );        
        
        /**
         * Remove forgiven sites set by this client 
         *
         * @param aSite, empty then remove all of the forgiven sites set by the client
         *               otherwise, remove the given site from forgiven site list
         * @return 
         */
        IMPORT_C TInt RemoveForgivenSiteL( const TDesC& aSite = KNullDesC );
        
        
        /**
         * check whether the given cert is trusted for the given site
         *
         * @param aCert a certificate
         * @param aSiteName trusted site name
         * @return ETrue /EFalse
         */
        IMPORT_C TBool IsTrustedSiteL( const TDesC8& aCert, const TDesC& aTrustedSite );
        
        /**
         * check whether out of date certificate is allowed for this site
         *
         * @param aCertificate 
         * @param aTrustedSite 
         * @return ETrue/EFalse
         */
        IMPORT_C TBool IsOutOfDateAllowedL( const TDesC8& aCertificate, const TDesC& aTrustedSite );
        
        /**
         * list all of the sites for which the cert is trusted
         *
         * @param aCert a certificate
         * @param aListOfSites trusted sites' name
         * @param aMessage IPC message
         * @return ETrue /EFalse
         */
        IMPORT_C TInt GetTrustedSitesL( const TDesC8& aCert, RPointerArray<HBufC>& aListOfSites );

    private:
        
        CTrustSitesStore();
        
        void ConstructL();
        
        void FreeRequestBuffer();
        
        TInt AllocRequestBuffer(TInt aReqdSize);
        
        void SendSyncRequestAndHandleOverflowL(TInt aInitialBufSize,
                                               const TIpcArgs& aArgs);

    private:
        
        RDevTokenClientSession* iClientSession;
        
        HBufC8* iRequestDataBuf;    
        
        TPtr8 iRequestPtr;
    };

#endif //__TRUSTEDSITESSTORE_H__

//EOF

