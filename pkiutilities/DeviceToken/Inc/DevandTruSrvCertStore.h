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
* Description:   The header file of DevandTruSrvCertStore
*
*/



#ifndef __DEVANDTRUSRVCERTSTORE_H__
#define __DEVANDTRUSRVCERTSTORE_H__

#include <mctwritablecertstore.h>
#include "DevTokenMarshaller.h"
#include "DevTokenDataTypes.h"
#include "DTClient.h"


/**
 * The device tokens Certstore client interface.
 * 
 * Implements the MCTWritableCertStore interface, with inheritance from
 * CFSClient.  Instances of this class are created by CClientInterfaceFactory.
 * Marshals client data for server requests and unpacks returned parameter data
 * across IPC boundary.
 * Note that although this class is derived from CActive (via CDTClient) is does
 * not act an active object, or use CActive functionality.
 *
 *  @lib DevTokenClient.dll
 *  @since S60 v3.2
 */
class CDevandTruSrvCertStore : public CDTClient, public MCTWritableCertStore
    {
    public:
        
        static MCTTokenInterface* NewWritableInterfaceL(MCTToken& aToken, RDevTokenClientSession& aClient);
        
        static MCTTokenInterface* NewReadableInterfaceL(MCTToken& aToken, RDevTokenClientSession& aClient);
        
    public:
        
        virtual ~CDevandTruSrvCertStore();
        
    public: 
        
        // From MCTTokenInterface
        virtual MCTToken& Token();
        
        virtual void DoRelease();
        
    public:
        
        // From MCTCertStore
        virtual void List(RMPointerArray<CCTCertInfo>& aCerts, 
                          const CCertAttributeFilter& aFilter, 
                          TRequestStatus& aStatus);
        
        virtual void CancelList();  
        
        virtual void GetCert(CCTCertInfo*& aCertInfo, const TCTTokenObjectHandle& aHandle,
                             TRequestStatus& aStatus);
        
        virtual void CancelGetCert(); 
        
        virtual void Applications(const CCTCertInfo& aCertInfo, RArray<TUid>& aApplications,
                                  TRequestStatus& aStatus); 
                                  
        virtual void CancelApplications();  
        
        virtual void IsApplicable(const CCTCertInfo& aCertInfo, TUid aApplication, 
                                  TBool& aIsApplicable, TRequestStatus& aStatus);
        
        virtual void CancelIsApplicable();  
        
        virtual void Trusted(const CCTCertInfo& aCertInfo, TBool& aTrusted, 
                             TRequestStatus& aStatus);
        
        virtual void CancelTrusted(); 
        
        virtual void Retrieve(const CCTCertInfo& aCertInfo, TDes8& aEncodedCert,
                              TRequestStatus& aStatus);
        
        virtual void CancelRetrieve();
        
        // From MCTWritableCertStore
        virtual void Add(const TDesC& aLabel, TCertificateFormat aFormat,
                         TCertificateOwnerType aCertificateOwnerType, 
                         const TKeyIdentifier* aSubjectKeyId, const TKeyIdentifier* aIssuerKeyId,
                         const TDesC8& aCert, TRequestStatus& aStatus);
        
        virtual void Add(const TDesC& aLabel, TCertificateFormat aFormat,
                         TCertificateOwnerType aCertificateOwnerType, 
                         const TKeyIdentifier* aSubjectKeyId, const TKeyIdentifier* aIssuerKeyId,
                         const TDesC8& aCert, const TBool aDeletable,
                         TRequestStatus& aStatus);
        
        virtual void CancelAdd(); 
        
        virtual void Remove(const CCTCertInfo& aCertInfo, TRequestStatus& aStatus);
        
        virtual void CancelRemove();  
        
        virtual void SetApplicability(const CCTCertInfo& aCertInfo, 
        
        const RArray<TUid>& aApplications, TRequestStatus &aStatus);
        
        virtual void CancelSetApplicability();
        
        virtual void SetTrust(const CCTCertInfo& aCertInfo, TBool aTrusted, 
                              TRequestStatus& aStatus);
        
        virtual void CancelSetTrust();  

    protected:
        
        // From CActive
        virtual void RunL();

    private:
        
        CDevandTruSrvCertStore(TInt aUID, MCTToken& aToken, RDevTokenClientSession& aClient);
        
        void ConstructL();
        
        void DoListL(RMPointerArray<CCTCertInfo>& aCerts, const CCertAttributeFilter& aFilter);
        
        void DoGetCertL(CCTCertInfo*& aCertInfo, const TCTTokenObjectHandle& aHandle);
        
        void DoApplicationsL(const CCTCertInfo& aCertInfo, RArray<TUid>& aApplications);
        
        TInt DoRetrieve(const CCTCertInfo& aCertInfo, TDes8& aEncodedCert);
        
        void CopyFilter( CCertAttributeFilter* aDesFilter, const CCertAttributeFilter& aSrcFilter);
        
    private:
        
        CCertAttributeFilter* iFilter;    
    };

#endif  //__DEVANDTRUSRVCERTSTORE_H__

//EOF
