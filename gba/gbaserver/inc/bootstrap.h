/*
 * Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies). 
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
 * Description:  Bootstrap classes definitions
 *
 */

#ifndef BOOTSTRAPIMPL_H
#define BOOTSTRAPIMPL_H

#include <e32base.h>
#include <stringpool.h>
#include <http.h>
#include <GbaUtility.h>
#include "M3GPPAuthenticationCallback.h"
#include "gbauicc.h"

void ResetAndDestroyArray(TAny* aArray);

class CGbaServerSession;
class CDataRetriever;
// format for output of data/time values
_LIT(KDateFormat,"%D%M%Y%/0%1%/1%2%/2%3%/3 %:0%H%:1%T%:2%S.%C%:3");

class MBootstrapCallBack : public M3GPPAuthenticationCallback
    {
public:
    
    virtual void CompleteBootStrappingL(TInt aError) = 0;
    
    virtual TBool InterfaceIs2G() = 0;
    
    };

class C3GPPBootstrap : public CActive, public MBootstrapCallBack
    {
public:
    
    static C3GPPBootstrap* NewL(CGbaServerSession* aSession);
    
    static C3GPPBootstrap* NewLC(CGbaServerSession* aSession);
    
    void InitializeL();
    
    void GetBootstrappingMaterialL(const TDesC8& aBSFAddress,
            const TDesC8& aNAFURL, const TDesC8& aUICC_LABEL,
            const TUint8& aFlags, const TDesC8& aKeyUsage, TDes8 &aKsNAF,
            TDes8 &aBTID, TTime &aLifetime, EGBARunType& aGBARunType,
            const TInt& aIAPID);
    
    void CancelBootstrap();
    
    TInt GetState() const;
    
    void Cleanup();
    
    TBool GBAUAvailabilityL();

    // from M3GPPAuthenticationCallback  
    TBool GetCredentialsL(const TUriC8& aURI, RString aRealm, RString aNonce,
            RString aAlgorithm, RString& aUsername, RString& aPassword,
            RString& aResync, TBool& aResyncRequired);

    virtual ~C3GPPBootstrap();

    //from MBootstrapCallBack
    void CompleteBootStrappingL(TInt aError);
    
    TBool InterfaceIs2G();

private:

    void DoBootstrapL();
    
    TBool GenerateKeyMaterialL();
    
    void ConstructL();
    
    C3GPPBootstrap(CGbaServerSession* aSession);
    
    TBool LoadCredentialsL();
    
    void StoreCredentialsL();
    
    TBool ConvertStringToTTime(TDesC8& aLifetimeBuf, TTime* aLifetime);
    
    TBool IsStillValid(TTime* aLifetime);
    
    TInt LoadGBAUAvailabililtyFromStoreL(TBool& aIsfromGBAClient, TBool& aGBAUAvail);
    
    void SaveGBAUAvailabilityToStoreL(TBool& aGBAUAvail);
    
    TBool CheckGBAUAvailabilityL(TBool aIsfromGBAClient);
    
    MUICCInterface* RequestUICCInterfaceL();
    
    TBool IsPluginExistL();
    
    void MakePrivateFilenameL(RFs& aFs, const TDesC& aLeafName, TDes& aNameOut);
    
    void EnsurePathL(RFs& aFs, const TDesC& aFile);

    //From CActive
    void RunL();
    
    TInt RunError(TInt aError);
    
    void DoCancel();

public:

    enum TBootstrapperState
        {
        EIdle, EInitialize, EBusy, ECancel
        };
private:
    enum TSecurityAlgorithm
        {
        ESecAlgUnknown, EAKAv1, EAKAv2, ESIMv2
        };

private:

    MUICCInterface* iSmartCardInterface;
    TDes8*          iCallerBTIDBuf;     //not owned,  pointer to return buffer
    TDes8*          iCallerKsNAFBuf;    //not owned,  pointer to return buffer
    TTime*          iCallerLifetime;    //not owned, pointer to return object
    EGBARunType*    iCallerGBARunType;  //not owned, pointer to return object
    HBufC8*         iKsNAF;             //owned
    HBufC8*         iBTID;              //owned
    TTime           iLifetime;          //onwed
    HBufC8*         iIdentity;          //buffer for impi
    HBufC8*         iNAFID;             //buffer for nafid
    HBufC8*         iRand;              //buffer for rand
    HBufC8*         iMasterKey;         //buffer for Ks
    EGBARunType     iGBARunType;
    CDataRetriever* iDataRetriever;     //owned
    TInt            iAuthFailed;
    TBootstrapperState iBSState;
    TBool           iGBAUAvailable;
    TUid            iImplementationUID;
    TUid            iDtorIDKey;
    CGbaServerSession* iSession;        //Not owned
    };

#endif
//EOF
