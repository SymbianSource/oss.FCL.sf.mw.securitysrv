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
* Description:   Implementation of DevandTruSrvCertStore
*
*/



#include "DevandTruSrvCertStore.h"
#include "DevTokenType.h"
#include "DevToken.h"
#include "DevTokenUtils.h"
#include "DevTokenImplementationUID.hrh"

// This is sized for roughly five cert infos, to ensure that the overflow
// mechanism gets tested.
const TInt KDefaultBufferSize = 256;

// ======== MEMBER FUNCTIONS ========

// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::NewWritableInterfaceL()
// -----------------------------------------------------------------------------
//
MCTTokenInterface* CDevandTruSrvCertStore::NewWritableInterfaceL(MCTToken& aToken, 
                         RDevTokenClientSession& aClient)
    {
    // Destroyed by MCTTokenInterface::DoRelease() (no refcounting)
    CDevandTruSrvCertStore* me = new (ELeave) CDevandTruSrvCertStore(KInterfaceWritableCertStore, 
                                                                     aToken, 
                                                                     aClient);
    CleanupStack::PushL(me);
    me->ConstructL();
    CleanupStack::Pop(me);
    return (me);
    }


// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::NewReadableInterfaceL()
// -----------------------------------------------------------------------------
//
MCTTokenInterface* CDevandTruSrvCertStore::NewReadableInterfaceL(MCTToken& aToken, 
                         RDevTokenClientSession& aClient)
    {
    // Destroyed by MCTTokenInterface::DoRelease() (no refcounting)
    CDevandTruSrvCertStore* me = new (ELeave) CDevandTruSrvCertStore(KInterfaceCertStore,
                                                                     aToken, 
                                                                     aClient);
    CleanupStack::PushL(me);
    me->ConstructL();
    CleanupStack::Pop(me);
    return (me);
    }


// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::CDevandTruSrvCertStore()
// -----------------------------------------------------------------------------
//
CDevandTruSrvCertStore::CDevandTruSrvCertStore(TInt aUID,
                                               MCTToken& aToken, 
                                               RDevTokenClientSession& aClient) :
    CDTClient(aUID, aToken, aClient)  
    {
    iFilter = NULL;
    }


// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::~CDevandTruSrvCertStore()
// -----------------------------------------------------------------------------
//
CDevandTruSrvCertStore::~CDevandTruSrvCertStore()
    {
    if( iFilter )
        {
    	delete iFilter;
        }
    }


// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::ConstructL()
// -----------------------------------------------------------------------------
//
void CDevandTruSrvCertStore::ConstructL()
    {
    CActiveScheduler::Add(this);
    }


// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::DoRelease()
// -----------------------------------------------------------------------------
//
void CDevandTruSrvCertStore::DoRelease()
    {
    MCTTokenInterface::DoRelease();
    }


// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::Token()
// -----------------------------------------------------------------------------
//
MCTToken& CDevandTruSrvCertStore::Token()
    {
    return iToken;
    }


// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::RunL()
// -----------------------------------------------------------------------------
//  
void CDevandTruSrvCertStore::RunL()
    {
    // Never active, so should never get called
    DevTokenPanic(ENoRequestOutstanding);
    }


//MCTCertStore 

// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::List()
// -----------------------------------------------------------------------------
//
void CDevandTruSrvCertStore::List(RMPointerArray<CCTCertInfo>& aCerts, 
                                  const CCertAttributeFilter& aFilter,
                                  TRequestStatus& aStatus)
    {
    CDevToken* token = static_cast<CDevToken*>( &iToken ); 
    
    CDevTokenType* tokenType = static_cast<CDevTokenType*>( &(token->TokenType()) );
    
    if ( tokenType->iUid.iUid == DEVCERTSTORE_IMPLEMENTATION_UID )
        {
        if (aFilter.iOwnerType != EUserCertificate)
            {
            if( aFilter.iOwnerTypeIsSet )
                {
                TRequestStatus* status = &aStatus;
                User::RequestComplete( status, KErrNone );
                return;	
                }
            //If the owner type is not set, we create a filter and 
            // set the type to EUserCertificate    
            else
                {
                //alway use new filter.
                if( iFilter )
                    {
                	delete iFilter;
                	iFilter = NULL;
                    }
                TRAPD( err, iFilter = CCertAttributeFilter::NewL() );	
                if ( err != KErrNone )    
                    {
                    TRequestStatus* status = &aStatus;
                    User::RequestComplete( status, err ); 
                    return;	
                	  }
                
                CopyFilter( iFilter, aFilter );
                //Set owner type to user certificate
                iFilter->SetOwnerType( EUserCertificate );
                
                TRAPD(r, DoListL(aCerts, *iFilter));
                FreeRequestBuffer();
                TRequestStatus* status = &aStatus;
                User::RequestComplete(status, r); 
                return;
                }
            }
        }
    else if ( tokenType->iUid.iUid == TRUSRVCERTSTORE_IMPLEMENTATION_UID )
        {
        if (aFilter.iOwnerType != EPeerCertificate)
            {
            if( aFilter.iOwnerTypeIsSet )
                {
                TRequestStatus* status = &aStatus;
                User::RequestComplete( status, KErrNone );
                return;	
                }
            //If the owner type is not set, we create a filter and 
            // set the type to EPeerCertificate      
            else
                {
                if( iFilter )
                    {
                	delete iFilter;
                	iFilter = NULL;
                    }
                    
                TRAPD( er, iFilter = CCertAttributeFilter::NewL() );	
                if ( er != KErrNone )    
                    {
                    TRequestStatus* status = &aStatus;
                    User::RequestComplete( status, er ); 
                    return;	
                	  }    

                CopyFilter( iFilter, aFilter );
                
                iFilter->SetOwnerType( EPeerCertificate );
                TRAPD(r, DoListL(aCerts, *iFilter));
                FreeRequestBuffer();
                TRequestStatus* status = &aStatus;
                User::RequestComplete(status, r); 
                return;	
                }
            }
        }
    else
        {
        TRequestStatus* status = &aStatus;
        User::RequestComplete( status, KErrNone );  
        return; 
        }    

    TRAPD(r, DoListL(aCerts, aFilter));
    FreeRequestBuffer();
    TRequestStatus* status = &aStatus;
    User::RequestComplete(status, r); 
    }


// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::CopyFilterL()
// -----------------------------------------------------------------------------
//
void CDevandTruSrvCertStore::CopyFilter( CCertAttributeFilter* aDesFilter, const CCertAttributeFilter& aSrcFilter )
    {
    if ( aSrcFilter.iLabelIsSet )
        {
    	aDesFilter->SetLabel( aSrcFilter.iLabel );
        }

	if ( aSrcFilter.iUidIsSet )
	    {
		aDesFilter->SetUid( aSrcFilter.iUid );
	    }

	if ( aSrcFilter.iFormatIsSet )
	    {
		aDesFilter->SetFormat( aSrcFilter.iFormat );
	    }

	if ( aSrcFilter.iOwnerTypeIsSet )
	    {
		aDesFilter->SetOwnerType( aSrcFilter.iOwnerType );
	    }
	

	if ( aSrcFilter.iSubjectKeyIdIsSet )
	   {
	   aDesFilter->SetSubjectKeyId( aSrcFilter.iSubjectKeyId );	
	   }

	if ( aSrcFilter.iIssuerKeyIdIsSet )
	    {
		aDesFilter->SetIssuerKeyId( aSrcFilter.iIssuerKeyId );
	    }
	    
    aDesFilter->SetKeyUsage( aSrcFilter.iKeyUsage );    
    }


// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::DoListL()
// -----------------------------------------------------------------------------
//
void CDevandTruSrvCertStore::DoListL(RMPointerArray<CCTCertInfo>& aCerts, 
                 const CCertAttributeFilter& aFilter)
    {
    HBufC8* filterBuf = HBufC8::NewLC(DevTokenDataMarshaller::Size(aFilter));
    TPtr8 ptr(filterBuf->Des());
    DevTokenDataMarshaller::WriteL(aFilter, ptr);
    TIpcArgs args(0, filterBuf, 0, &iRequestPtr);
    SendSyncRequestAndHandleOverflowL(EListCerts, KDefaultBufferSize, args);
    CleanupStack::PopAndDestroy(filterBuf);
    DevTokenDataMarshaller::ReadL(iRequestPtr, iToken, aCerts);
    }


// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::CancelList()
// -----------------------------------------------------------------------------
//
void CDevandTruSrvCertStore::CancelList()
    {
    // synchronous, nothing to do
    }


// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::GetCert()
// -----------------------------------------------------------------------------
//
void CDevandTruSrvCertStore::GetCert(CCTCertInfo*& aCertInfo, 
                                     const TCTTokenObjectHandle& aHandle,
                                     TRequestStatus& aStatus)
    {
    TRAPD(r, DoGetCertL(aCertInfo, aHandle));
    FreeRequestBuffer();
    TRequestStatus* stat = &aStatus;
    User::RequestComplete(stat, r); 
    }


// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::DoGetCertL()
// -----------------------------------------------------------------------------
//
void CDevandTruSrvCertStore::DoGetCertL(CCTCertInfo*& aCertInfo, 
                                        const TCTTokenObjectHandle& aHandle)
    {
    User::LeaveIfError(AllocRequestBuffer(sizeof(CCTCertInfo))); // assumes serialized size <= sizeof class
    TIpcArgs args(aHandle.iObjectId, 0, 0, &iRequestPtr);
    User::LeaveIfError(iClientSession.SendRequest(EGetCert, args));
    DevTokenDataMarshaller::ReadL(iRequestPtr, iToken, aCertInfo);
    }


// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::CancelGetCert()
// -----------------------------------------------------------------------------
//
void CDevandTruSrvCertStore::CancelGetCert()
    {
    // synchronous, nothing to do
    }


// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::Applications()
// -----------------------------------------------------------------------------
//
void CDevandTruSrvCertStore::Applications(const CCTCertInfo& aCertInfo, 
                                          RArray<TUid>& aApplications,
                                          TRequestStatus& aStatus)
    {
    TRAPD(r, DoApplicationsL(aCertInfo, aApplications));
    FreeRequestBuffer();
    TRequestStatus* status = &aStatus;
    User::RequestComplete(status, r); 
    }


// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::DoApplicationsL()
// -----------------------------------------------------------------------------
//
void CDevandTruSrvCertStore::DoApplicationsL(const CCTCertInfo& aCertInfo, 
                                             RArray<TUid>& aApplications)
    {
    TIpcArgs args(aCertInfo.Handle().iObjectId, 0, 0, &iRequestPtr);
    SendSyncRequestAndHandleOverflowL(EApplications, KDefaultBufferSize, args);
    DevTokenDataMarshaller::ReadL(iRequestPtr, aApplications);        
    }


// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::CancelApplications()
// -----------------------------------------------------------------------------
//
void CDevandTruSrvCertStore::CancelApplications()
    {
    // synchronous, nothing to do
    }


// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::IsApplicable()
// -----------------------------------------------------------------------------
//
void CDevandTruSrvCertStore::IsApplicable(const CCTCertInfo& aCertInfo, TUid aApplication, 
                    TBool& aIsApplicable, TRequestStatus& aStatus)
    {
    TIpcArgs args(aCertInfo.Handle().iObjectId, 0, aApplication.iUid);
    TInt r = iClientSession.SendRequest(EIsApplicable, args); 
    if (r >= 0)
        {
        aIsApplicable = !!r;  // Convert TInt to TBool
        r = KErrNone;
        }

    TRequestStatus* status = &aStatus;
    User::RequestComplete(status, r); 
    }


// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::CancelIsApplicable()
// -----------------------------------------------------------------------------
//
void CDevandTruSrvCertStore::CancelIsApplicable()
    {
    // synchronous, nothing to do
    }


// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::Trusted()
// -----------------------------------------------------------------------------
//
void CDevandTruSrvCertStore::Trusted(const CCTCertInfo& aCertInfo, TBool& aTrusted, 
                 TRequestStatus& aStatus)
    {
    TIpcArgs args(aCertInfo.Handle().iObjectId);
    TInt r = iClientSession.SendRequest(ETrusted, args);  
    if (r >= 0)
        {
        aTrusted = !!r;  // Convert TInt to TBool
        r = KErrNone;
        }

    TRequestStatus* status = &aStatus;
    User::RequestComplete(status, r); 
    }


// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::CancelTrusted()
// -----------------------------------------------------------------------------
//
void CDevandTruSrvCertStore::CancelTrusted()
    {
    // synchronous, nothing to do
    }


// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::Retrieve()
// -----------------------------------------------------------------------------
//
void CDevandTruSrvCertStore::Retrieve(const CCTCertInfo& aCertInfo, TDes8& aEncodedCert,
                  TRequestStatus& aStatus)
    {
    TInt err = DoRetrieve(aCertInfo, aEncodedCert);
    FreeRequestBuffer();
    TRequestStatus* stat = &aStatus;
    User::RequestComplete(stat, err);
    }


// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::DoRetrieve()
// -----------------------------------------------------------------------------
//
TInt CDevandTruSrvCertStore::DoRetrieve(const CCTCertInfo& aCertInfo, TDes8& aEncodedCert)
    {
    if (aCertInfo.Size() <= 0)
        {
        return KErrArgument;
        }

    if (aEncodedCert.MaxSize() < aCertInfo.Size())
        {
        return KErrOverflow;
        }

    TInt r = AllocRequestBuffer(aEncodedCert.MaxSize());  
    if (KErrNone!=r)
        {
        return r;
        }

    TIpcArgs args(aCertInfo.Handle().iObjectId, 0, 0, &iRequestPtr);
    r = iClientSession.SendRequest(ERetrieve, args);
    if (r == KErrNone)
        {
        aEncodedCert.Copy(iRequestPtr);
        }

    return r;
    }


// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::CancelRetrieve()
// -----------------------------------------------------------------------------
//
void CDevandTruSrvCertStore::CancelRetrieve()
    {
    // synchronous, nothing to do
    }


//  MCTWritableCertStore interface

// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::Add()
// -----------------------------------------------------------------------------
//
void CDevandTruSrvCertStore::Add(const TDesC& aLabel, TCertificateFormat aFormat,
               TCertificateOwnerType aCertificateOwnerType, 
               const TKeyIdentifier* aSubjectKeyId, const TKeyIdentifier* aIssuerKeyId,
               const TDesC8& aCert, TRequestStatus& aStatus)
    {
    Add(aLabel, aFormat, aCertificateOwnerType, aSubjectKeyId, aIssuerKeyId, aCert, 
    ETrue, aStatus);
    } 


// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::Add()
// -----------------------------------------------------------------------------
//
void CDevandTruSrvCertStore::Add(const TDesC& aLabel, TCertificateFormat aFormat,
               TCertificateOwnerType aCertificateOwnerType, 
               const TKeyIdentifier* aSubjectKeyId, const TKeyIdentifier* aIssuerKeyId,
               const TDesC8& aCert, const TBool aDeletable,
               TRequestStatus& aStatus)
    {
    CDevToken* token = static_cast<CDevToken*>( &iToken ); 

    CDevTokenType* tokenType = static_cast<CDevTokenType*>( &(token->TokenType()) );

    if ( tokenType->iUid.iUid == DEVCERTSTORE_IMPLEMENTATION_UID )
        {
        if (aCertificateOwnerType != EUserCertificate)
            {
            TRequestStatus* status = &aStatus;
            User::RequestComplete( status, KErrArgument );  
            return; 
            }
        }
    else if ( tokenType->iUid.iUid == TRUSRVCERTSTORE_IMPLEMENTATION_UID )
        {
        if (aCertificateOwnerType != EPeerCertificate)
            {
            TRequestStatus* status = &aStatus;
            User::RequestComplete( status, KErrArgument );  
            return; 
            }
        }
    else
        {
        TRequestStatus* status = &aStatus;
        User::RequestComplete( status, KErrArgument );  
        return; 
        } 


    TRequestStatus* stat = &aStatus;
    if (aLabel.Length() > KMaxCertLabelLength)
        {
        //  Too long to be a label
        User::RequestComplete(stat, KErrOverflow);
        return;
        }

    TPckgBuf<TDevTokenAddCertDataStruct> data;
    data().iLabel.Zero();
    data().iSubjectKeyId.Zero();
    data().iIssuerKeyId.Zero();   
    data().iLabel.Copy(aLabel);   
    data().iFormat = aFormat;
    data().iCertificateOwnerType = aCertificateOwnerType;
    if (aSubjectKeyId)
        {
        data().iSubjectKeyId.Copy(*aSubjectKeyId);
        }
    if (aIssuerKeyId)
        {
        data().iIssuerKeyId.Copy(*aIssuerKeyId);
        }
    data().iDeletable = aDeletable;

    TInt r = AllocRequestBuffer(aCert.Size());

    if (r!=KErrNone)
        {
        User::RequestComplete(stat, r);
        return;
        }

    iRequestPtr.FillZ();
    iRequestPtr.Copy(aCert);

    TIpcArgs args(0, &data, 0, &iRequestPtr);
    r = iClientSession.SendRequest(EAddCert, args);

    FreeRequestBuffer();
    User::RequestComplete(stat, r);
    } 


// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::CancelAdd()
// -----------------------------------------------------------------------------
//
void CDevandTruSrvCertStore::CancelAdd()
    {
    // synchronous, nothing to do
    }


// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::Remove()
// -----------------------------------------------------------------------------
//
void CDevandTruSrvCertStore::Remove(const CCTCertInfo& aCertInfo, TRequestStatus& aStatus)
    {
    TIpcArgs args(aCertInfo.Handle().iObjectId);
    TInt r = iClientSession.SendRequest(ERemoveCert, args); 
    TRequestStatus* stat = &aStatus;
    User::RequestComplete(stat, r);
    }


// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::CancelRemove()
// -----------------------------------------------------------------------------
//
void CDevandTruSrvCertStore::CancelRemove()
    {
    // synchronous, nothing to do
    }


// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::SetApplicability()
// -----------------------------------------------------------------------------
//
void CDevandTruSrvCertStore::SetApplicability(const CCTCertInfo& aCertInfo, 
                      const RArray<TUid>& aApplications,
                      TRequestStatus &aStatus)
    {
    TInt r = AllocRequestBuffer(DevTokenDataMarshaller::Size(aApplications));
    if (r != KErrNone)
        {
        TRequestStatus* stat = &aStatus;
        User::RequestComplete(stat, r);
        return;
        }
    DevTokenDataMarshaller::Write(aApplications, iRequestPtr);

    TIpcArgs args(aCertInfo.Handle().iObjectId, 0, &iRequestPtr);
    TInt err = iClientSession.SendRequest(ESetApplicability, args);
    TRequestStatus* status = &aStatus;
    User::RequestComplete(status, err);
    }


// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::CancelSetApplicability()
// -----------------------------------------------------------------------------
//
void CDevandTruSrvCertStore::CancelSetApplicability()
    {
    // synchronous, nothing to do
    }


// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::SetTrust()
// -----------------------------------------------------------------------------
//
void CDevandTruSrvCertStore::SetTrust(const CCTCertInfo& aCertInfo,
                  TBool aTrusted, 
                  TRequestStatus& aStatus)
    {
    TIpcArgs args(aCertInfo.Handle().iObjectId, 0, aTrusted);
    TInt err = iClientSession.SendRequest(ESetTrust, args);
    TRequestStatus* status = &aStatus;
    User::RequestComplete(status, err);
    }


// -----------------------------------------------------------------------------
// CDevandTruSrvCertStore::CancelSetTrust()
// -----------------------------------------------------------------------------
//
void CDevandTruSrvCertStore::CancelSetTrust()
    {
    // synchronous, nothing to do
    }
    
//EOF

