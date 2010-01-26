/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Trust Settings management class.
*
*/


#ifndef WIMTRUSTSETTINGSMGMT_H
#define WIMTRUSTSETTINGSMGMT_H

//  INCLUDES
#include "WimClient.h"
#include "WimCertInfo.h"
#include "WimCertTClassWrappers.h"

// FORWARD DECLARATIONS
class CWimCertInfo;

// CLASS DECLARATION

/**
*  RWimTrustSettingsMgmt 
*  This class handles Trust Settings Store
*  related traffic between client and server.
*  @lib WimClient.lib
*  @since Series 60 3.0
*/
class RWimTrustSettingsMgmt : public RWimClient
    {
    public:

        /**
        * A Static method to get the client Session
        * @return Pointer to newly created instance of RWimTrustSettingsMgmt
        */  
        static RWimTrustSettingsMgmt* ClientSessionL();

        /**
        * Destructor.
        * Allocated memory is released.
        */  
        virtual ~RWimTrustSettingsMgmt();

        /**
        * Fetches trust settings for given certificate. 
        * @param  aCert (IN) Certificate
        * @param  aStatus (IN/OUT) Request Status of asynchronous call
        * @return void
        */
        void GetTrustSettingsL( const CWimCertInfo& aCert,
                                TRequestStatus& aStatus );

        /**
        * Set applicability for given certificate. 
        * @param aCert (IN) Certificate
        * @param aApplications (IN) Array of applications supported by 
        *        certificate
        * @param aStatus Request (IN/OUT) Status of asynchronous call.
        * @return void
        */
        void SetApplicabilityL( const CWimCertInfo& aCert,
                                const RArray<TUid>& aApplications,
                                TRequestStatus& aStatus );

        /**
        * Set trust flag for given certificate.
        * @param aCert (IN) Certificate
        * @param aTrusted (IN) Is certificate trusted
        * @param aStatus (IN/OUT) Request Status of asynchronous call.
        * @return void
        */
        void SetTrustL( const CWimCertInfo& aCert,
                        TBool aTrusted,
                        TRequestStatus& aStatus );

        /**
        * Set default trust settings for given certificate.
        * @param aCert (IN) Certificate
        * @param aAddApps (IN) Are default applications inserted 
        *        (ETrue means yes)
        * @param aStatus (IN/OUT) Request Status of asynchronous call
        * @return void
        */
        void SetDefaultTrustSettingsL( const CWimCertInfo& aCert,
                                       TBool aAddApps,
                                       TRequestStatus& aStatus );

        /**
        * Remove trust settings of given certificate. If certificate
        * is not found, return with status.Int() = KErrNotFound
        * @param aCert (IN) Certificate
        * @param aStatus (IN/OUT) Request Status of asynchronous call
        * @return void
        */
        void RemoveTrustSettingsL( const CWimCertInfo& aCert,
                                   TRequestStatus& aStatus );

        /**
        * Cancel any issued asynchronous call
        * @return void
        */
        void CancelDoing();

        /**
        * Returns pointer to package of trust settings.
        * @return Pointer to package of trust settings
        */
        CWimCertPckgBuf<TTrustSettings>* TrustSettingsPckg();

        /**
        * Delete buffer pointers
        * @return void
        */
        void DeleteBuffers();
     
    private:
        
        /**
        * Constructor 
        */
        RWimTrustSettingsMgmt();

        /** 
        * Copy constructor
        * @param aSource -Reference to class object.
        */
        RWimTrustSettingsMgmt( const RWimTrustSettingsMgmt& aSource );

        /** 
        * Assignment operator
        * @param aParam -Reference to class object.
        */
        RWimTrustSettingsMgmt& operator=( const RWimTrustSettingsMgmt& aParam );

    private:

        // Package for trust settings. Owned.
        CWimCertPckgBuf<TTrustSettings>* iPckgBufTrustSettings;

        // Package for cert info. Owned.
        CWimCertPckgBuf<TWimCertInfoPckg>* iCertPkcg;
    };

#endif // WIMTRUSTSETTINGSMGMT_H

// End of File









































