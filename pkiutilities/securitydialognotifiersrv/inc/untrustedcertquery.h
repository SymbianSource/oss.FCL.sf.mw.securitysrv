/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Displays untrusted certificate dialog
*
*/

#ifndef C_UNTRUSTEDCERTQUERY_H
#define C_UNTRUSTEDCERTQUERY_H

#include <e32base.h>                            // CActive
#include <hb/hbcore/hbdevicedialogsymbian.h>    // MHbDeviceDialogObserver
#include <securitydefs.h>                       // TValidationError

class CHbDeviceDialogSymbian;
class CHbSymbianVariantMap;


/**
* Displays untrusted certificate query.
* Untrusted certificate query dialog is displayed for secure connection
* (SSL/TLS) server authentication failure errors. CCTUntrustedCertQuery
* class uses UntrustedCertificateDialog device dialog to show the query.
*/
NONSHARABLE_CLASS( CUntrustedCertQuery ) : public CActive, public MHbDeviceDialogObserver
    {
    public:     // constructors and destructor
        static CUntrustedCertQuery* NewL( TValidationError aValidationError,
                const TDesC8& aCertificate, const TDesC& aServerName,
                TBool aCanHandlePermanentAccept );
        ~CUntrustedCertQuery();

    public:     // new functions
        enum TResponse {
            EQueryRejected,
            EQueryAccepted,
            EQueryAcceptedPermanently
        };
        void ShowQueryAndWaitForResponseL( TResponse& aResponse );

    protected:  // from CActive
        void DoCancel();
        void RunL();

    private:    // from MHbDeviceDialogObserver
        void DataReceived( CHbSymbianVariantMap& aData );
        void DeviceDialogClosed( TInt aCompletionCode );

    private:    // new functions
        CUntrustedCertQuery( TValidationError aValidationError,
                const TDesC8& aCertificate, const TDesC& aServerName,
                TBool aCanHandlePermanentAccept );
        void ConstructL();

    private:    // data
        TValidationError iValidationError;
        const TDesC8& iCertificate;
        const TDesC& iServerName;
        TBool iCanHandlePermanentAccept;
        CActiveSchedulerWait *iWait;
        TInt iWaitCompletionCode;
        CHbDeviceDialogSymbian* iDeviceDialog;
        CHbSymbianVariantMap* iVariantMap;
        TResponse iResponse;
    };

#endif  // C_UNTRUSTEDCERTQUERY_H

