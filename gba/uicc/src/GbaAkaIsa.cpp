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
* Description:  Implementation of class CAkaIsaInterface
*
*/


// INCLUDE FILES

#include <e32math.h>
#include <etelmmerr.h>
#include <mmtsy_names.h>            //TSY and Phone name
#include <hash.h>
#include <implementationproxy.h>    //for ecom
#include "GBAAkaIsa.h"
#include "GBALogger.h"
#include "GbaCommon.h"

//Constants
const TInt KMaxBufferSize = 256;
const TInt KNetworkIdLength2 = 2;
const TInt KNetworkIdLength3 = 3;
const TInt KIntegerConstant4 = 4;
const TInt KIntegerConstant16 = 16;
const TInt KIntegerConstant32 = 32;

_LIT8( KAT, "@" );
_LIT8( KIMSMNC, "ims.mnc" );
_LIT8( KIMSMNC0, "ims.mnc0" );
_LIT8( KBSFMNC, "bsf.mnc" ); 
_LIT8( KBSFMNC0, "bsf.mnc0" );     
_LIT8( KMCC,".mcc");        
_LIT8( KPUB3GPPORG, ".pub.3gppnetwork.org");
_LIT8( K3GPPORG, ".3gppnetwork.org");
_LIT8( K3GPPGBARES, "3gpp-gba-res");
_LIT8( K3GPPGBAKS, "3gpp-gba-ks");
_LIT8( KGBAME, "gba-me");

#ifdef __WINS__
_LIT8( KWINTESTID, "test@pub.3gpp.org");
#endif

static MUICCInterface* NewFunctionL();


// ======== LOCAL FUNCTIONS ========
// ---------------------------------------------------------------------------
// MUICCInterface* NewFunctionL()
// ---------------------------------------------------------------------------
//
MUICCInterface* NewFunctionL()
    {
    GBA_TRACE_DEBUG(("MUICCInterface* NewFunctionL"));
    MUICCInterface* UICCInterface = NULL;
    UICCInterface = CAkaIsaInterface::NewL();
    return UICCInterface;
    }


// -----------------------------------------------------------------------------
// TImplementationProxy ImplementationTable[] 
// -----------------------------------------------------------------------------
//
const TImplementationProxy ImplementationTable[] =
    {
    IMPLEMENTATION_PROXY_ENTRY(GBA_UICC_INTERFACE_IMPLE, NewFunctionL)
    };


// -----------------------------------------------------------------------------
// TImplementationProxy* ImplementationGroupProxy()
// This function is needed by ECom and is the only one exported function
// -----------------------------------------------------------------------------
//
EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt& aTableCount)
    {
    aTableCount = sizeof(ImplementationTable) / sizeof(TImplementationProxy);
    return (ImplementationTable);
    }


// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CAkaIsaInterface::CAkaIsaInterface()
// -----------------------------------------------------------------------------
//
CAkaIsaInterface::CAkaIsaInterface():iCardInterface( ENoInterface )   
    {
    }


// -----------------------------------------------------------------------------
// CAkaIsaInterface::NewL()
// -----------------------------------------------------------------------------
//
MUICCInterface* CAkaIsaInterface::NewL()
    {
    GBA_TRACE_DEBUG(("Constructing: CAkaIsaInterface"));
    CAkaIsaInterface* self = new(ELeave) CAkaIsaInterface();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    GBA_TRACE_DEBUG(("Construction complete: CAkaIsaInterface"));
    return self;
    }


// -----------------------------------------------------------------------------
// CAkaIsaInterface::NewL()
// -----------------------------------------------------------------------------
//
void CAkaIsaInterface::ConstructL()
    {
    GBA_TRACE_BEGIN();
    //Connect to ETel server
    User::LeaveIfError( iEtelServer.Connect() );
    //Load phone module
    User::LeaveIfError( iEtelServer.LoadPhoneModule( KMmTsyModuleName ) );
    //open phone
    User::LeaveIfError( iPhone.Open( iEtelServer, KMmTsyPhoneName ) );
    //open custom api
    User::LeaveIfError( iCustomAPI.Open(iPhone) );
    // check card interface, initialize iCardInterface
    QueryCardInterfaceL();
    GBA_TRACE_END();
    }


// -----------------------------------------------------------------------------
// CAkaIsaInterface::~CAkaIsaInterface()
// -----------------------------------------------------------------------------
//
CAkaIsaInterface::~CAkaIsaInterface()
    {
    GBA_TRACE_BEGIN();
    //Close all created resources 
    iCustomAPI.Close(); 
    iPhone.Close();
    iEtelServer.Close();
    GBA_TRACE_END();
    }


void CAkaIsaInterface::QueryIdentityL( TDes8& aIdentity ) 
    {
    #ifdef __WINS__
    aIdentity.Copy(KWINTESTID);
    return;
    #endif

    GBA_TRACE_BEGIN();
    GBA_TRACE_DEBUG(("QueryIdentity is starting..."));
    
    TUint32 caps = 0;
    GBA_TRACE_DEBUG(("Phone is open now doing the actual query..."));

    // Get capability
    User::LeaveIfError( iPhone.GetIdentityCaps( caps ) );

    if ( caps & (RMobilePhone::KCapsGetSubscriberId))
        {
        RMobilePhone::TMobilePhoneSubscriberId id;
        
        TRequestStatus status;
        
        status =  KRequestPending;
        RMobilePhone::TMobilePhoneSubscriberId iccID;
        iPhone.GetSubscriberId( status, iccID );
        User::WaitForRequest( status );
        
        GBA_TRACE_DEBUG_NUM(("Can't get IMSI, error = %d"), status.Int() );
        User::LeaveIfError( status.Int() );
        
        GBA_TRACE_DEBUG(("id="));
        GBA_TRACE_DEBUG_DESC(iccID);

        aIdentity.Zero();
        aIdentity.Copy(iccID);
        
        // derive the ending part
        // get the mobile network name MNC and country code MCC
        
        //Make IMPI 
        RMobilePhone::TMobilePhoneNetworkInfoV1 info;
        TPckg<RMobilePhone::TMobilePhoneNetworkInfoV1> netInfoPckg(info);
        status =  KRequestPending;
        iPhone.GetHomeNetwork(status, netInfoPckg);
        
        User::WaitForRequest( status );
        GBA_TRACE_DEBUG_NUM(("Can't get network name, error = %d"), status.Int() );
        User::LeaveIfError( status.Int() );
        
        // append the "<IMSI>@ims.mnc<MNC>.mcc<MCC>.3gppnetwork.org"
        
        aIdentity.Append( KAT );

        if ( info.iNetworkId.Length() == KNetworkIdLength2 )
            {
            //add one more zero if only 2 digits
            aIdentity.Append( KIMSMNC0 );
            }
    
        if ( info.iNetworkId.Length() == KNetworkIdLength3 )
            {
            aIdentity.Append( KIMSMNC ); 
            }

        aIdentity.Append( info.iNetworkId );

        aIdentity.Append( KMCC );        
        aIdentity.Append( info.iCountryCode );
        aIdentity.Append( K3GPPORG );
      

        GBA_TRACE_DEBUG(("aIdentity="));
        GBA_TRACE_DEBUG_DESC(aIdentity);
        }
    else
        {
        aIdentity.Copy(_L8(""));
        }
    
    GBA_TRACE_END();
    }

// -----------------------------------------------------------------------------
// CAkaIsaInterface::QueryAuthenticationGBAUL
// If the GBA_U available, pass RAND and AUTN to smart card via custom API
// otherwise use RMobilePhone instead
// -----------------------------------------------------------------------------
TBool CAkaIsaInterface::QueryAuthenticationGBAUL(   const TDesC8& aNonce,
                                                    TDes8& aResponse,
                                                    TDes8& aResync ) 
    {
    GBA_TRACE_BEGIN();
    TBool keys_available = ETrue;
    //creat buffer to carry the input and output value
    RMmCustomAPI::TSimAuthenticationGbaBootstrap gbadata;
    
    RMmCustomAPI::TGbaBootstrapDataPckg gbabuf( gbadata ); 
    
    //Copy input data, RAND and AUTN
    gbadata.iRandomParameters.Copy(aNonce.Ptr(),KAKA_RAND_LENGTH);
    gbadata.iAUTN.Copy(aNonce.Ptr()+KAKA_RAND_LENGTH,KAKA_RAND_LENGTH);
    
    //call the API to get the RES 
    TRequestStatus status;
    
    status =  KRequestPending;
    
    GBA_TRACE_DEBUG(("Send Authentication request to card"));
    iCustomAPI.GetWlanSimAuthenticationData( status, gbabuf );
    GBA_TRACE_DEBUG(("Wait for result..."));
    User::WaitForRequest( status );
    
    GBA_TRACE_DEBUG_NUM(("Authentication is Done, err = %d"),status.Int() );

    if(status == KErrNone)
        {
        aResponse.Copy(gbadata.iRES);
        GBA_TRACE_DEBUG_BINARY(gbadata.iRES);
        }
    else
        {
        keys_available = EFalse;
        
        if (status != KErrMMEtelSqnVerificationFailed )
            {
            User::LeaveIfError( status.Int() );
            }
        }

    if ( status == KErrMMEtelSqnVerificationFailed )
        {
        GBA_TRACE_DEBUG_NUM(("AUTS len is Done !!!, len = %d"), gbadata.iAUTS.Length() );
        GBA_TRACE_DEBUG_BINARY(gbadata.iAUTS);
      
        if(gbadata.iAUTS.Length() == 0)
            {
            GBA_TRACE_DEBUG(("Auts is Still zero !!!!!"));
            }
       else
            {
            GBA_TRACE_DEBUG((" before copy"));
            aResync.Copy(gbadata.iAUTS); 
            GBA_TRACE_DEBUG(("after copy auts"));
            }
        }
    GBA_TRACE_DEBUG((" Done!"));
    GBA_TRACE_END(); 
    return keys_available;
    }


// -----------------------------------------------------------------------------
// CAkaIsaInterface::QueryAuthentication2GL
// If the GBA_U available, pass RAND and AUTN to smart card via custom API
// otherwise use RMobilePhone instead
// run simulated aka
// -----------------------------------------------------------------------------

void CAkaIsaInterface::QueryAuthentication2GL(  const TDesC8& aNonce, TDes8& aResponse )
    {
    GBA_TRACE_BEGIN();
    RMmCustomAPI::TSimAuthenticationEapSim authSIMdata;
    RMmCustomAPI::TSimDataPckg authSIMBuf(authSIMdata);
    
    TRequestStatus status;
    status =  KRequestPending;
    
    authSIMdata.iRandomParameters.Copy(aNonce.Ptr(),KAKA_RAND_LENGTH);
    GBA_TRACE_DEBUG(("SIM authentication"));
    iCustomAPI.GetWlanSimAuthenticationData(status,authSIMBuf);
    
    User::WaitForRequest( status );
    GBA_TRACE_DEBUG_NUM(("GetWlanSimAuthenticationData return, status = %d"), status.Int() );
    User::LeaveIfError( status.Int() );
    
    GBA_TRACE_DEBUG(("Creating 2G authentication vector KDF(key,\"3gpp-gba-res\",sres)"));
    GBA_TRACE_DEBUG_BINARY(authSIMdata.iSRES);
    GBA_TRACE_DEBUG_BINARY(authSIMdata.iKC);
    
    TBuf8<KMaxBufferSize> lastSRESKC;
    TBuf8<2*RMmCustomAPI::KMaxKCLength+KAKA_RAND_LENGTH> kc2;
    
    // COPYING Kc twice
    GBA_TRACE_DEBUG(("Copying iKC"));
    for( TInt  i=0; i<(RMmCustomAPI::KMaxKCLength) ; i++ )
        {
        kc2.Append(  (TUint8)(authSIMdata.iKC[i]) );
        }
    GBA_TRACE_DEBUG(("Copying iKC"));
    for( TInt  i=0;i<(RMmCustomAPI::KMaxKCLength);i++ )
        {
        kc2.Append(  (TUint8)(authSIMdata.iKC[i]) );
        }
    GBA_TRACE_DEBUG(("appending RAND"));
    kc2.Append(aNonce.Ptr(),KAKA_RAND_LENGTH);
    
    GBA_TRACE_DEBUG(("K part"));
    GBA_TRACE_DEBUG_BINARY(kc2);
    
    // appending RAND
    lastSRESKC.Append( 0x01);
    lastSRESKC.Append(K3GPPGBARES);
    lastSRESKC.Append( 0x00);
    lastSRESKC.Append( 0x0C);
    GBA_TRACE_DEBUG(("Copying SRES"));
    
    for( TInt i=0; i<KIntegerConstant4; i++ )
        {
        lastSRESKC.Append( (TUint8)(authSIMdata.iSRES[i]) );
        }
    lastSRESKC.Append( 0x00);
    lastSRESKC.Append( 0x04);
    GBA_TRACE_DEBUG(("S part"));
    GBA_TRACE_DEBUG_BINARY(lastSRESKC);
    
    // derive response
    CMessageDigest* digest = CMessageDigestFactory::NewDigestL( CMessageDigest::ESHA256);
    CleanupStack::PushL( digest );
    CHMAC* sha256 = CHMAC::NewL(kc2, digest);
    CleanupStack::Pop( digest );
    CleanupStack::PushL( sha256 );
    TPtrC8 hash(sha256->Hash(lastSRESKC));
    GBA_TRACE_DEBUG(("resulting hash: "));
    GBA_TRACE_DEBUG_BINARY(hash);
    
    aResponse.Append(hash.Ptr(),KRESPONSE_2G_LENGTH);
    
    // KEY MATERIAL NEEDS TO BE STORED
    // CHECK 3GPP TS 3.3220 V7.5.0 (2006-09) page 54 item 7
    // The BSF shall generate key material Ks by computing Ks = KDF (key, Ks-input, "3gpp-gba-ks", SRES).
    // The B-TID value shall be also generated in format of NAI by taking the base64 encoded [12] RAND value
    // from step 3, and the BSF server name, i.e. base64encoded(RAND)@BSF_servers_domain_name.
    GBA_TRACE_DEBUG(("DERIVING THE MASTER KEY Ks"));
    
    TBuf8<KMaxBufferSize> hashMaterial;
    hashMaterial.Append(0x01);
    hashMaterial.Append(aNonce.Mid(KIntegerConstant32,KIntegerConstant16));
    hashMaterial.Append(0x00);
    hashMaterial.Append(0x10);
    hashMaterial.Append(K3GPPGBAKS);
    hashMaterial.Append(0x00);
    hashMaterial.Append(0x0b);
    hashMaterial.Append(authSIMdata.iSRES.Left(RMmCustomAPI::KMaxRESLength));
    hashMaterial.Append(0x00);
    hashMaterial.Append(0x04);
    
    GBA_TRACE_DEBUG(("key for creating the key"));
    GBA_TRACE_DEBUG_BINARY(kc2);
    
    GBA_TRACE_DEBUG(("hashing material for creating the key"));
    GBA_TRACE_DEBUG_BINARY(hashMaterial);
    
    TPtrC8 keymaterial( sha256->Hash( hashMaterial) );
    
    GBA_TRACE_DEBUG(("Master key Ks, !!! Remove from LOG"));
    GBA_TRACE_DEBUG_BINARY(keymaterial);
    
    iCKBuf.Copy(keymaterial.Left(KIntegerConstant16));
    iIKBuf.Copy(keymaterial.Mid(KIntegerConstant16,KIntegerConstant16));
    
    CleanupStack::PopAndDestroy( sha256 );
    
    GBA_TRACE_DEBUG(("Done!"));
    GBA_TRACE_END();
    }

// -----------------------------------------------------------------------------
// CAkaIsaInterface::QueryAuthentication3GL
// If the GBA_U available, pass RAND and AUTN to smart card via custom API
// otherwise use RMobilePhone instead
// run aka
// -----------------------------------------------------------------------------
TBool CAkaIsaInterface::QueryAuthentication3GL(   const TDesC8& aNonce,
                                                    TDes8& aResponse,
                                                    TDes8& aResync )
    {
    GBA_TRACE_BEGIN();
    TBool keys_available = ETrue;
    
    RMmCustomAPI::TSimAuthenticationEapAka authAkadata;
    RMmCustomAPI::TAkaDataPckg authAkaBuf(authAkadata);
    TRequestStatus status;
    status =  KRequestPending;
    authAkadata.iRandomParameters.Copy(aNonce.Ptr(),KAKA_RAND_LENGTH);
    authAkadata.iAUTN.Copy(aNonce.Ptr()+KAKA_RAND_LENGTH,KAKA_RAND_LENGTH);
    GBA_TRACE_DEBUG(("AKA authentication"));
    
    iCustomAPI.GetWlanSimAuthenticationData(status,authAkaBuf);
    User::WaitForRequest( status );
    
    GBA_TRACE_DEBUG_NUM(("GBA_ME:QueryAuthentication3GL: Authentication error = %d"),status.Int()  );
    
    if( status.Int() == KErrNone )
        {
        GBA_TRACE_DEBUG(("AKA, appending result"));
        aResponse.Copy(authAkadata.iRES);
        GBA_TRACE_DEBUG_BINARY(authAkadata.iRES);
        
        GBA_TRACE_DEBUG(("storing keys"));
        
        // store results
        iCKBuf.Copy(authAkadata.iCK);
        iIKBuf.Copy(authAkadata.iIK);
        }
    else
        {
        keys_available = EFalse;
        
        if (status != KErrMMEtelSqnVerificationFailed )
            {
            User::LeaveIfError( status.Int() );
            }
        }
    
    if ( status == KErrMMEtelSqnVerificationFailed )
        {
        GBA_TRACE_DEBUG_NUM(("GBA_ME:QueryAuthentication3GL: AUTS len is Done !!!, len = %d"), authAkadata.iAUTS.Length() );
        GBA_TRACE_DEBUG_BINARY( authAkadata.iAUTS );
        
        if(authAkadata.iAUTS.Length() == 0)
            {
            GBA_TRACE_DEBUG(("GBA_U:QueryAuthentication3GL: Auts is Still zero !!!!!"));
            }
        else
            {
            GBA_TRACE_DEBUG(("QueryAuthentication3GL before copy"));
            aResync.Copy(authAkadata.iAUTS);
            GBA_TRACE_DEBUG(("QueryAuthentication3GL after copy auts"));
            }
        }
    GBA_TRACE_END();
    return keys_available;
    }


// -----------------------------------------------------------------------------
// CAkaIsaInterface::QueryAuthenticationL
// If the GBA_U available, pass RAND and AUTN to smart card via custom API
// otherwise use RMobilePhone instead
// -----------------------------------------------------------------------------
TBool CAkaIsaInterface::QueryAuthenticationL(
     const TDesC8& aNonce, 
     TDes8& aResponse, 
     TDes8& aResync ) 
    {
    GBA_TRACE_BEGIN();
    GBA_TRACE_DEBUG(("aka nonce="));
    GBA_TRACE_DEBUG_BINARY(aNonce);
    // GBA_U not availiable
    if ( iCardInterface != EGBAUInterface )
        {
        GBA_TRACE_DEBUG(("QueryAuthenticationL is starting..."));
        
        //run aka
        if ( iCardInterface == E3GInterface )
            {
            return QueryAuthentication3GL( aNonce, aResponse, aResync );
            }
        
        // run simulated aka
        if ( iCardInterface == E2GInterface )
            {
            QueryAuthentication2GL( aNonce, aResponse );
            return ETrue;
            }
        return EFalse;
        }
    else
        {
        GBA_TRACE_DEBUG(("GBA_U authentication"));
        return QueryAuthenticationGBAUL( aNonce, aResponse, aResync );   
        }
    }


// -----------------------------------------------------------------------------
// CAkaIsaInterface::QueryKeyMaterialL()
// If GBA_U, send the request to smart card.
// -----------------------------------------------------------------------------
//
void CAkaIsaInterface::QueryKeyMaterialL(const TDesC8& aKey, const TDesC8& aRand, const TDesC8& aIMPI, const TDesC8& aUTF8_NAF_ID, TDes8& aDerivedKey ) 
    {
    if ( iCardInterface == EGBAUInterface )
        { 
        GBA_TRACE_DEBUG_BINARY(aUTF8_NAF_ID);

        GBA_TRACE_DEBUG(("GBA_U:QueryKeyMaterialL: Enter the function"));
        
        TRequestStatus status;
        status =  KRequestPending;

        RMmCustomAPI::TSimAuthenticationGbaNafDerivation nafdata;
    
        RMmCustomAPI::TGbaNafDerivationDataPckg nafbuf( nafdata );
  
        nafdata.iNafId.Copy( aUTF8_NAF_ID );
        
        
        GBA_TRACE_DEBUG(("GBA_U:QueryKeyMaterialL: copy nafid, no truncate"));
        GBA_TRACE_DEBUG_BINARY(aUTF8_NAF_ID);
        
        nafdata.iImpi.Copy( aIMPI );
        
         GBA_TRACE_DEBUG(("GBA_U:QueryKeyMaterialL: copy impi, no truncate"));
        iCustomAPI.GetWlanSimAuthenticationData( status, nafbuf ); 
       
        User::WaitForRequest( status ); 
        
        GBA_TRACE_DEBUG_NUM(("GBA_U:QueryKeyMaterialL: QueryKeyMaterial is Done, err = %d"), status.Int());
          
        User::LeaveIfError( status.Int() );

        aDerivedKey.Copy( nafdata.iKsExtNaf ); //return the ks_ext_naf
        }
    else //GBA_ME
        {
        GBA_TRACE_DEBUG(("QueryKeyMaterial"));
        GBA_TRACE_DEBUG(("derive key as specified in 33.22 V7.5.0 page 35"));
        // derive key as specified in 33.22 V7.5.0 page 35
        // S = FC || P0 || L0 || P1 || L1 || P2 || L2 || P3 || L3 ||... || Pn || Ln
        // derived key = HMAC-SHA-256 ( Key , S )
        // -  FC = 0x01,
        // -  P1 = RAND,
        // -  L1 = length of RAND is 16 octets (i.e. 0x00 0x10),
        // -  P2 = IMPI encoded to an octet string using UTF-8 encoding (see clause B.2.1),
        // -  L2 = length of IMPI is variable (not greater that 65535),
        // -  P3 = NAF_ID with the FQDN part of the NAF_ID encoded to an octet string using UTF-8 encoding (see clause B.2.1), and
        // -  L3 = length of NAF_ID is variable (not greater that 65535).
        // In the key derivation of Ks_NAF as specified in clause 4 and Ks_ext_NAF as specified in clause 5,
        // -  P0 = "gba-me" (i.e. 0x67 0x62 0x61 0x2d 0x6d 0x65), and
        // -  L0 = length of P0 is 6 octets (i.e., 0x00 0x06).
        // In the key derivation of Ks_int_NAF as specified in clause 5,
        // -  P0 = "gba-u" (i.e. 0x67 0x62 0x61 0x2d 0x75), and
        // -  L0 = length of P0 is 5 octets (i.e., 0x00 0x05).
        TBuf8<KMaxBufferSize> ks_NAF_material;
        // FC
        ks_NAF_material.Append(0x01);
        // P0
        ks_NAF_material.Append(KGBAME);
        // L0
        ks_NAF_material.Append(0x00);
        ks_NAF_material.Append(0x06);
        // P1
        ks_NAF_material.Append(aRand);
        // L1 - length of rand in two bytes
        ks_NAF_material.Append(0x00);
        ks_NAF_material.Append((TUint8)aRand.Length());
        // P2
        ks_NAF_material.Append(aIMPI);
        // L2 - length of impi in two bytes
        ks_NAF_material.Append(0x00);
        ks_NAF_material.Append((TUint8)aIMPI.Length());
        // P2
        ks_NAF_material.Append(aUTF8_NAF_ID);
        // L2 - length of FQDN (NAF_ID) in two bytes
        ks_NAF_material.Append(0x00);
        ks_NAF_material.Append((TUint8)aUTF8_NAF_ID.Length());
        GBA_TRACE_DEBUG(("NAF_ID"));
        GBA_TRACE_DEBUG_DESC(aUTF8_NAF_ID);    
        GBA_TRACE_DEBUG(("Value of K"));
        GBA_TRACE_DEBUG_BINARY(aKey);
        GBA_TRACE_DEBUG(("Value of S"));
        GBA_TRACE_DEBUG_BINARY(ks_NAF_material);
        
        CMessageDigest* digest = CMessageDigestFactory::NewDigestL( CMessageDigest::ESHA256);
        CleanupStack::PushL( digest );
        CHMAC* sha256 = CHMAC::NewL(aKey, digest); //sha256 will take the ownership of digest pointer
        CleanupStack::Pop( digest );
        CleanupStack::PushL( sha256 );
        TPtrC8 hash(sha256->Hash(ks_NAF_material));
        aDerivedKey.Copy(hash);
        CleanupStack::PopAndDestroy( sha256 ); //sha256
        }
    }


// -----------------------------------------------------------------------------
// CAkaIsaInterface::QueryKs()
// -----------------------------------------------------------------------------
//
TBool CAkaIsaInterface::QueryKs( TDes8& aKS )
    {
    GBA_TRACE_DEBUG(("CAkaIsaInterface::QueryKs"));
    if( iIKBuf.Length()> 0 && iCKBuf.Length() > 0 )
        {
        GBA_TRACE_DEBUG(("got Ks to copy"));
        aKS.Append(iCKBuf);
        aKS.Append(iIKBuf);
        GBA_TRACE_DEBUG(("Copy done!"));
        return ETrue;
        }
    GBA_TRACE_DEBUG(("No Ks to copy, something wrong in Authentication"));
    return EFalse;
    }
    

// -----------------------------------------------------------------------------
// CAkaIsaInterface::QueryHomeNetworkDnL()
// -----------------------------------------------------------------------------
//
void CAkaIsaInterface::QueryHomeNetworkDnL( TDes8& aHNDN )
    {
    RMobilePhone::TMobilePhoneNetworkInfoV1 info;
    TPckg<RMobilePhone::TMobilePhoneNetworkInfoV1> netInfoPckg(info);
    TRequestStatus status;

    status =  KRequestPending;
    iPhone.GetHomeNetwork(status, netInfoPckg);
    User::WaitForRequest( status );
    
    GBA_TRACE_DEBUG_NUM(("phone status = %d"), status.Int() );
    User::LeaveIfError( status.Int() );
    
    // append the bsf.mnc0<MNC>.mcc<MCC>.pub.3gppnetwork.org or bsf.mnc.<MNC>.mcc<MCC>.pub.3gppnetwork.org
  
    if ( InterfaceIs2G() )
        {
        aHNDN.Copy( KHTTPSTag );  
        }
    else
        {
        aHNDN.Copy( KHTTPTag ); 
        } 

    if ( info.iNetworkId.Length() == KNetworkIdLength2 )
        {
        //Append KBSFMNC0 to aHNDN
        aHNDN.Append( KBSFMNC0 );
        }
    
    if ( info.iNetworkId.Length() == KNetworkIdLength3 )
        {
		//Append KBSFMNC to aHNDN
        aHNDN.Append( KBSFMNC ); 
        }
       
    aHNDN.Append( info.iNetworkId );
    aHNDN.Append( KMCC );        
    aHNDN.Append( info.iCountryCode );
    aHNDN.Append( KPUB3GPPORG );
    }


// -----------------------------------------------------------------------------
// CAkaIsaInterface::InterfaceIs2G()
// -----------------------------------------------------------------------------
//
TBool CAkaIsaInterface::InterfaceIs2G() 
    {
    GBA_TRACE_DEBUG(("CAkaIsaInterface: Enter the function "));
    GBA_TRACE_DEBUG_NUM(("CAkaIsaInterface: iCardInterface is %d "), iCardInterface );
    return iCardInterface == E2GInterface;
    };


// -----------------------------------------------------------------------------
// CAkaIsaInterface::QueryGBAUAvailability()
// Get the GBA_U availability from smart card via custom API
// -----------------------------------------------------------------------------
void CAkaIsaInterface::QueryGBAUAvailabilityL( TBool& aGBAAvail )
    {
    GBA_TRACE_DEBUG(("QueryGBAUAvailability: Enter the function "));
    GBA_TRACE_DEBUG_NUM(("QueryGBAUAvailability: iCardInterface is %d "), iCardInterface );
    aGBAAvail = ( iCardInterface == EGBAUInterface );
    }

// -----------------------------------------------------------------------------
// CAkaIsaInterface::UpdateGBADataL()
// Save the B-TID and keylifetime to smart card
// -----------------------------------------------------------------------------

TInt CAkaIsaInterface::UpdateGBADataL( const TDesC8& aBTID, const TDesC8& aLifetime )
    {
    
    GBA_TRACE_DEBUG(("GBA_U:UpdateGBADataL: Enter the function"));
    
    TRequestStatus status;
    status =  KRequestPending;

    RMmCustomAPI::TSimAuthenticationGbaBootstrapUpdate updatedata;

    updatedata.iBTid.Copy( aBTID );
    updatedata.iKeyLifeTime.Copy( aLifetime );

    RMmCustomAPI::TGbaBootstrapUpdateDataPckg updatebuf( updatedata );
        
    iCustomAPI.GetWlanSimAuthenticationData( status, updatebuf ); 
     
    User::WaitForRequest( status );
    
    GBA_TRACE_DEBUG_NUM(("GBA_U:UpdateGBADataL: Update GBA is Done, err = %d"), status.Int());
    
    return status.Int();
    }


// -----------------------------------------------------------------------------
// CAkaIsaInterface::Release()
// -----------------------------------------------------------------------------
void CAkaIsaInterface::Release()
     {
     delete this; 
     }


// -----------------------------------------------------------------------------
// CAkaIsaInterface::NotifyCardChangeL()
// -----------------------------------------------------------------------------
void CAkaIsaInterface::NotifyCardChangeL()
    {
    // the card is changed, we need to re-initialize the interface 
	QueryCardInterfaceL();
    }


// -----------------------------------------------------------------------------
// CAkaIsaInterface::QueryCardInterfaceL()
// ----------------------------------------------------------------------------- 
//   
void CAkaIsaInterface::QueryCardInterfaceL()
    {
    GBA_TRACE_BEGIN();

    GBA_TRACE_DEBUG(("QueryCardInterfaceL is starting..."));

    TUint32 caps = 0;
      
    GBA_TRACE_DEBUG(("Found! Phone open! Checking what kind of ICC access we have...known types : {SIM,RUSIM,USIM}"));    
    User::LeaveIfError( iPhone.GetIccAccessCaps( caps ) );

    if ( caps & RMobilePhone::KCapsUSimAccessSupported )
        {
        GBA_TRACE_DEBUG(("This device  offers USIM access")); 
             
        //check if the card support GBA-U interface
        GBA_TRACE_DEBUG(("Check is GBA-U supported")); 
        TRequestStatus status;
        status =  KRequestPending;

        RMmCustomAPI::TAppSupport appSupport;
    
        //GBA-U 0, MGV-U 1
        appSupport.iAppNum = 0; 
     
        iCustomAPI.GetUSIMServiceSupport( status, appSupport);

        User::WaitForRequest( status );

        GBA_TRACE_DEBUG_NUM(("QueryCardInterfaceL: GBA_U avail checking is Done, err = %d"), status.Int());
    
        if ( status.Int() == KErrNotFound )
            {
            //Not gba-u service available
            GBA_TRACE_DEBUG(("QueryCardInterfaceL: it returns KErrNotFound, No GBA-U "));
            // set interface as 3g then
            iCardInterface = E3GInterface;
            }
        else if ( status.Int() == KErrNone )
            {
            //pass the value back
            GBA_TRACE_DEBUG(("QueryCardInterfaceL: it returns KErrNone"));
            if ( appSupport.iSupported )
                {
            	GBA_TRACE_DEBUG(("QueryCardInterfaceL: GBA-U support "));
                iCardInterface = EGBAUInterface;
                }
            else
                {
                GBA_TRACE_DEBUG(("QueryCardInterfaceL: GBA-U NOT support "));
                iCardInterface = E3GInterface;	
                }    
            }
        else
            {
            //leave , unexpected situation
            User::LeaveIfError(status.Int());    
            }    
        }
    else if( caps & RMobilePhone::KCapsSimAccessSupported )
        {
    	GBA_TRACE_DEBUG(("This device  offers SIM access only"));        
        iCardInterface = E2GInterface;
        //The card only support 2G interface
        }
    else
        {
        //nothing	
        }
    }

//EOF
