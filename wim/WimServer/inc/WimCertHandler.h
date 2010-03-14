/*
* Copyright (c) 2002-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  WIM Certificate handler
*
*/



#ifndef CWIMCERTHANDLER_H
#define CWIMCERTHANDLER_H

//  INCLUDES
#include "WimClsv.h"

// FORWARD DECLARATIONS
class CWimMemMgmt;
class CWimUtilityFuncs;

// CLASS DECLARATION

/**
*  Class for handling certificates in WIM card
*
*  @since Series60 2.1
*/
class CWimCertHandler : public CBase
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CWimCertHandler* NewL();

        /**
        * Destructor.
        */
        virtual ~CWimCertHandler();

    public: // New functions

        /**
        * Fetches certificates from WIM card.
        * @param aMessage Encapsulates a client request.
        * @param aWimMgmt Pointer to WIM reference management class.
        */
        void GetCertificatesFromWimL( const RMessage2& aMessage,
                                      CWimMemMgmt* aWimMgmt );

        /**
        * Fetches certificate extra information from WIM card.
        * Extra information includes trusted usage and CDF referense
        * information.
        * @param aMessage Encapsulates a client request.
        * @param aWimMgmt Pointer to WIM reference management class.
        */
        void GetExtrasFromWimL( const RMessage2& aMessage,
                                CWimMemMgmt* aWimMgmt );

        /**
        * Fetches count of certicates in certain WIM card.
        * @param aMessage Encapsulates a client request.
        * @param aWimMgmt Pointer to WIM reference management class.
        */
        void GetCerticateCountL( const RMessage2& aMessage,
                                 CWimMemMgmt* aWimMgmt ) const;


        /**
        * Stores certificate to the WIM card.
        * @since Series60 2.6
        * @param aRequest Enumerator used in message
                          passing between client and server.
        * @param aMessage Encapsulates a client request.
        */
        void StoreCertificateL( TWimServRqst aRequest,
                                const RMessage2& aMessage ) const;

        /**
        * Removes certificate from a WIM card.
        * @since Series60 2.6
        * @param aMessage Encapsulates a client request.
        * @param aWimMgmt Pointer to WIM reference management class.
        */
        void RemoveCertificateL( const RMessage2& aMessage,
                                 CWimMemMgmt* aWimMgmt ) const;

        /**
        * Fetches certificate details.
        * @param aOpCode Enumerator used in message
                          passing between client and server.
        * @param aMessage Encapsulates a client request.
        */
        void GetCertificateDetailsL( TWimServRqst aOpCode,
                                      const RMessage2& aMessage ) ;

        /**
        * Exports Public Key of certificate
        * @since Series60 2.6
        * @param aMessage Encapsulates a client request.
        */
        void ExportPublicKeyL( const RMessage2& aMessage ) const;


        TBool SanityCheck( TUint32 aCertRef );

        TBool CheckReadCapsForUsage( const RMessage2& aMsg,
                                       TUint8 aUsage );


        WIMI_STAT ResolveCertUsage( const RMessage2& aMsg,
                                                TUint8& aUsage );

        WIMI_STAT GetCertificateInfo( WIMI_Ref_pt aCertRef,
                                                  TUint8& aUsage );
    private:

        CWimCertHandler();
        void ConstructL();

        /**
        * Fetches certificate from the WIM card.
        * @param aTmpWimRef Reference(id) of certain WIM card.
        * @param aUsage     Certificate usage. Client/CA.
        * @param aCertNum   Certificate number.
        * @param aCertRefLst    Pointer to certificate reference list.
        * @param aCertInfoLst   Pointer to certificate info list.
        * @param aMessage   Encapsulates a client request.
        * @return Status returned by WIMI. See in wimclsv.h.
        */
        WIMI_STAT GetCertificateFromWimRefL( WIMI_Ref_t* aTmpWimRef,
                                             TInt8 aUsage,
                                             TUint8& aCertNum,
                                             TUint32* aCertRefLst,
                                             TWimCertInfo* aCertInfoLst,
                                             const RMessage2& aMessage );

        /**
        * Copies certificate information to client's allocated memory area.
        * @param aCertInfo  Certificate information to be copied.
        * @param aCert      Certificate reference.
        * @param aMessage   Encapsulates a client request.
        * @return TInt      KErrNone or KErrArgument
        */
        TInt CopyCertificateInfo( TWimCertInfo& aCertInfo,
                                   WIMI_Ref_t* aCert,
                                   const RMessage2& aMessage );

        /**
        * Fetches certificate extra information from the WIM card.
        * @param aTmpWimRef Reference(id) of certain WIM card.
        * @param aUsage     Certificate usage. Client/CA.
        * @param aKeyHash   Certificate key hash.
        * @param aMessage   Encapsulates a client request.
        * @return Status returned by WIMI. See in WimClsv.h.
        */
        WIMI_STAT GetExtrasFromWimRefL( WIMI_Ref_t* aTmpWimRef,
                                        TInt8 aUsage,
                                        TDesC8& aKeyHash,
                                        const RMessage2& aMessage );

        /**
        * Copies certificate extra information to client's allocated
        * memory area. Extra information includes trusted usage OID's
        * and certificate location information.
        * @param aCert            Certificate reference.
        * @param aMessage         Encapsulates a client request.
        */
        void CopyCertExtrasInfoL( WIMI_Ref_t* aCert,
                                  const RMessage2& aMessage );

        /**
        * Get count of certificates in WIM
        * @param aRef       WIM card reference pointer.
        * @param aCertCount Count of certificates in WIM card pointed by aRef
        * @param aUsage     Certificate usage.
        * @return Status returned by WIMI. See in wimclsv.h.
        */
        WIMI_STAT GetCertificateCountByWIM( WIMI_Ref_t* aRef,
                                            TUint8& aCertCount,
                                            TUint8 aUsage ) const;

        /**
        * Parse Public Key from certificate.
        * @since Series60 2.6
        * @param aCertData   Certificate binary data
        * @param aPublicKey  Public Key
        * @param aCertType   Certificate type (X509/WTLS)
        */
        void ParseCertPublicKeyL( const TDesC8& aCertData,
                                  TDes8& aPublicKey,
                                  const TUint8 aCertType ) const;

        /**
         * Returns WIM reference. Returns WIM ref cached in aWimMgmt, or
         * fetches new using WIMI_GetWIMRef() and caches it in aWimMgmt.
         * @since Serier60 5.2
         * @param aWimMgmt  WIM memory manager
         */
        WIMI_Ref_t* MainWimRef( CWimMemMgmt& aWimMgmt ) const;

    private:    // Data
        CWimUtilityFuncs* iWimUtilFuncs;
        RArray<TUint32> iCertRefLst;
    };

#endif      // CWIMCERTHANDLER_H

//End of File
