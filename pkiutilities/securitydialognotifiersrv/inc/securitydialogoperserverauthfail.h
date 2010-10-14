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
* Description:  Server authentication failure operation in security dialog
*
*/

#ifndef SECURITYDIALOGOPERSERVERAUTHFAIL_H
#define SECURITYDIALOGOPERSERVERAUTHFAIL_H

#include "securitydialogoperation.h"    // CSecurityDialogOperation
#include <securitydefs.h>               // TValidationError
#include <ct/rmpointerarray.h>          // RMPointerArray
#include <f32file.h>                    // RFs
#include <secdlgimpldefs.h>             // TServerAuthenticationFailureDialogResult

class MSecurityDialogOperationObserver;
class CServerAuthenticationFailureInput;
class CUnifiedCertStore;
class MCTWritableCertStore;
class CCertAttributeFilter;
class CCTCertInfo;
class CUntrustedCertQuery;


/**
 * CServerAuthFailOperation is a CSecurityDialogOperation that handles
 * the EServerAuthenticationFailure operation.
 */
NONSHARABLE_CLASS( CServerAuthFailOperation ) : public CSecurityDialogOperation
    {
    public:     // constructors and destructor
        static CServerAuthFailOperation* NewL( MSecurityDialogOperationObserver& aObserver,
                const RMessage2& aMessage, TInt aReplySlot );
        ~CServerAuthFailOperation();

    public:     // from CSecurityDialogOperation
        void StartL( const TDesC8& aBuffer );
        void CancelOperation();

    protected:  // from CActive (via CSecurityDialogOperation)
        void RunL();
        void DoCancel();

    private:    // new functions
        CServerAuthFailOperation( MSecurityDialogOperationObserver& aObserver,
                const RMessage2& aMessage, TInt aReplySlot );
        void InitializeUnifiedCertStoreL();
        void ProcessServerAuthorizationFailureL();
        void OpenTrustedSiteCertificateStoreL();
        TBool IsAlreadyTrustedSiteL();
        void StartFetchingTrustedSiteCertsL();
        void ShowUntrustedCertificateDialogL();
        void SaveServerCertToTrustedSiteCertStoreL();
        void SaveServerNameToTrustedSitesStoreL();
        void ReturnResultL( TServerAuthenticationFailureDialogResult aResult );
        void RetrieveFirstTrustedSiteCertL();
        void RetrieveNextTrustedSiteCertL();
        TBool IsRetrievedCertSameAsServerCertL();

    private:    // data
        CServerAuthenticationFailureInput* iInput;

        TValidationError iAuthFailReason;
        TPtrC8 iEncodedServerCert;
        HBufC8* iServerCertFingerprint;
        HBufC* iServerName;
        HBufC* iCertLabel;

        CUntrustedCertQuery* iUntrustedCertQuery;

        RFs iFs;
        CUnifiedCertStore* iCertStore;
        MCTWritableCertStore* iTrustedSiteCertStore;
        CCertAttributeFilter* iCertAttributeFilter;
        RMPointerArray<CCTCertInfo> iCertInfos;
        TInt iRetrieveCertIndex;
        HBufC8* iRetrievedCertBuffer;
        TPtr8 iRetrievedCertBufPtr;

        enum {
            EInitialiseCertStore,
            EListTrustedSiteCerts,
            ERetrieveTrustedSiteCert,
            ESavingServerCert
        } iMode;
    };

#endif  // SECURITYDIALOGOPERSERVERAUTHFAIL_H

