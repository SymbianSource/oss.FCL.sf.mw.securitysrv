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
* Description:  Implementation of bootstrap functions
*
*/


// This component has the following responsibilities 
// 1. Allow configuration of the BSF URL   
// 2. Open http connection to bsf    
// 3. On digest auth req get nonce  
// 4. Use this to find RAND AUTN 
// 5. Execute SIM auth, get back SRES, IK,CK 
// 6. return SRES 
// 7. Store keymaterial  to a token   

#include <s32stor.h>
#include <s32file.h>
#include <tconvbase64.h>                   //for base64 en/decoding

#include <ecom.h>
#include "dataretriever.h"
#include "bootstrap.h" 
#include "GbaCommon.h"
#include "GBALogger.h"
#include "GbaSession.h"

const TInt KKsNAFMaxLength = 128;
const TInt KStringLength = 80;
const TInt KMaxLengthIMPI = 255;
const TInt KMaxLengthNAFID = 255; 
const TInt KMaxLengthBTID = 255;
const TInt KMaxLengthKs = 32;
const TInt KMaxLengthRES = 16;          //16 bytes for RES
const TInt KMaxLengthAUTS = 16;         //16 bytes for AUTS
const TInt KMaxLengthEnAUTS = 20;
const TInt KMaxLengthRAND = 16;         //16 bytes for RAND
const TInt KMaxLengthTimeString = 30 ;
const TInt KHTTPTagLength = 7;
const TInt KAlgorithmTybeBufferLength = 32;
const TInt KBufferSize255 = 255;
const TInt KVariableIndex0 = 0;
const TInt KVariableIndex1 = 1;
const TInt KVariableIndex2 = 2;
const TInt KVariableIndex3 = 3;
const TInt KVariableIndex4 = 4;
const TInt KVariableIndex5 = 5;
const TInt KVariableIndex6 = 6;

#define GBA_UICC_INTERFACE_IMPLE        0x20029F0F
#define GBA_SOFTISIM_INTERFACE_IMPLE    0x20029F0E

// XML tags
_LIT8(KBTIDFlag,"<btid>");
_LIT8(KBTIDEndFlag,"</btid>");
_LIT8(KLifetimeFlag,"<lifetime>");
_LIT8(KLifetimeEndFlag,"</lifetime>");
_LIT(KGbaCredentialsFileName, "GbaCredentials.dat");
_LIT(KGBAStoreStandardDrive, "C:");
_LIT(KTimeFormat, "time %d,%d,%d,%d,%d,%d");
_LIT(KErrorMessage, "FAILED error = %d");
_LIT(Kakav1, "akav1");

// -----------------------------------------------------------------------------
// C3GPPBootstrap::NewL()
// -----------------------------------------------------------------------------
//
C3GPPBootstrap* C3GPPBootstrap::NewL( CGbaServerSession* aSession )
    {
    C3GPPBootstrap* self = NewLC( aSession );
    CleanupStack::Pop(self);
    return(self) ;
    }

// -----------------------------------------------------------------------------
// C3GPPBootstrap::NewLC()
// -----------------------------------------------------------------------------
//
C3GPPBootstrap* C3GPPBootstrap::NewLC( CGbaServerSession* aSession )
    {
    C3GPPBootstrap* self = new (ELeave) C3GPPBootstrap( aSession );
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }


// -----------------------------------------------------------------------------
// C3GPPBootstrap::~C3GPPBootstrap()
// -----------------------------------------------------------------------------
//
C3GPPBootstrap::~C3GPPBootstrap()
    {
    REComSession::DestroyedImplementation(iDtorIDKey);  
    if(IsActive())
        Cancel();
    if( iSmartCardInterface )
        {
        iSmartCardInterface->Release();
        REComSession::FinalClose();
        }
    delete iKsNAF;
    delete iNAFID;
    delete iRand;
    delete iBTID;
    delete iIdentity;
    delete iMasterKey;
    delete iDataRetriever;
    }


// -----------------------------------------------------------------------------
// C3GPPBootstrap::ConstructL()
// -----------------------------------------------------------------------------
//
void C3GPPBootstrap::ConstructL()
    {
    GBA_TRACE_BEGIN();

    GBA_TRACE_DEBUG(("initializing buffers"));
    iKsNAF = HBufC8::NewL(KKsNAFMaxLength);
    iBTID = HBufC8::NewL(KMaxLengthBTID);
    iNAFID = HBufC8::NewL(KMaxLengthNAFID);
    iIdentity = HBufC8::NewL(KMaxLengthIMPI);
    iMasterKey = HBufC8::NewL(KMaxLengthKs);
    iRand = HBufC8::NewL(KMaxLengthRAND);

    GBA_TRACE_DEBUG(("initializing http handler"));
    iDataRetriever  = CDataRetriever::NewL( this );
    
    iImplementationUID.iUid = GBA_UICC_INTERFACE_IMPLE;
    //if want softisim for testing
    //iImplementationUID.iUid = GBA_SOFTISIM_INTERFACE_IMPLE;
    
    //Interface is initialized based on the plug in
    if ( !IsPluginExistL() )
        {
        GBA_TRACE_DEBUG(("There aren't plug-in existing"));
        User::LeaveIfError( KErrGeneral );
        }
     
    GBA_TRACE_DEBUG(("requesting uicc handler"));
    iSmartCardInterface = RequestUICCInterfaceL();
    GBA_TRACE_DEBUG(("request made"));
    
    if ( !iSmartCardInterface )
        {
        GBA_TRACE_DEBUG(("Get interface failed"));
        User::LeaveIfError( KErrGeneral );
        }
    
    CActiveScheduler::Add( this );

    GBA_TRACE_END();
    }


// -----------------------------------------------------------------------------
// C3GPPBootstrap::C3GPPBootstrap()
// -----------------------------------------------------------------------------
//
C3GPPBootstrap::C3GPPBootstrap( CGbaServerSession* aSession ): CActive( EPriorityStandard ),
        iSmartCardInterface( NULL ), iLifetime ( 0 ),iGBARunType( ENoType ), iBSState(EIdle), iSession ( aSession )
    {
    }


// ---------------------------------------------------------------------------
// CBootstrapStateMachine::InitializeL()
// ---------------------------------------------------------------------------
//
void C3GPPBootstrap::InitializeL()
    {
    if ( IsActive() )
        {
        User::LeaveIfError( KErrInUse );
        }
    
    iBSState = EInitialize;
    GBA_TRACE_DEBUG(("Bootstrapping State machine startL()"));
    iStatus = KRequestPending;
    SetActive();
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, KErrNone );
    }


// ---------------------------------------------------------------------------
// C3GPPBootstrap::RunL()
// ---------------------------------------------------------------------------
//
void C3GPPBootstrap::RunL()
    {
    GBA_TRACE_DEBUG(("C3GPPBootstrap::RunL() -enter"));
    
    GBA_TRACE_DEBUG_NUM(("C3GPPBootstrap::RunL iStatus=%d"), iStatus.Int() );
    GBA_TRACE_DEBUG_NUM(("C3GPPBootstrap::RunL iBSState=%d"), iBSState );
    
    if((iBSState == ECancel) & iStatus.Int() == KErrCancel)
        {
        iBSState = EIdle;
        return;
        }
    
    User::LeaveIfError( iStatus.Int() );
    
    switch(iBSState)
        {
        case EInitialize:
            {
            GBA_TRACE_DEBUG(("C3GPPBootstrap::RunL() - EInitialize"));
            DoBootstrapL();
            GBA_TRACE_DEBUG(("C3GPPBootstrap::RunL() - EInitialize end"));
            break;  
            }
           
        case EBusy:
            {
            GBA_TRACE_DEBUG(("C3GPPBootstrap::RunL() - EBusy"));
            iSession->StateMachineCallBack( iStatus.Int() );
            Cleanup();
            GBA_TRACE_DEBUG(("C3GPPBootstrap::RunL() - EBusy end"));
            break;
            }
  
       default:
           GBA_TRACE_DEBUG(("C3GPPBootstrap::RunL() - default"));
           break;
       }
    GBA_TRACE_DEBUG(("C3GPPBootstrap::RunL() -exit"));
    }


TInt C3GPPBootstrap::RunError(TInt aError)
    {
    GBA_TRACE_DEBUG_NUM(("Failed with error = %d"), aError );
    iSession->StateMachineCallBack( aError );
    //Back to EBootstrapIdle
    iBSState = EIdle;
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// C3GPPBootstrap::DoCancel()
// ---------------------------------------------------------------------------
//
void C3GPPBootstrap::DoCancel()
    {
    GBA_TRACE_DEBUG(("C3GPPBootstrap::DoCancel()"));
    switch(iBSState)
        {
         case EInitialize:
         case EBusy:
             {
             GBA_TRACE_DEBUG(("C3GPPBootstrap::DoCancel() EBootstrapDone"));
             CancelBootstrap();
             break;
             }
        default:
            break;
        };
    Cleanup();
    }


// -----------------------------------------------------------------------------
// C3GPPBootstrap::CancelBootstrap()
// -----------------------------------------------------------------------------
//
void C3GPPBootstrap::CancelBootstrap()
    {
    GBA_TRACE_BEGIN();
    iBSState = ECancel;
    iDataRetriever->CancelRequest();
    GBA_TRACE_END();
    }


// -----------------------------------------------------------------------------
// C3GPPBootstrap::DoBootstrapL()
// -----------------------------------------------------------------------------
//
void C3GPPBootstrap::DoBootstrapL( )
    {

    HBufC8 *BsfUrl = HBufC8::NewLC(KMaxURLLength);
    TPtr8 PtrBSFUrl( BsfUrl->Des() );
    
    // fetch bsf address override if available
    if ( !iSession->Server()->ReadOptionL( KGbaBSFConfiguration, PtrBSFUrl ) )
        {
        //If no preset bsf, empty the buffer
        //We will calculate the address ourselves
        PtrBSFUrl.Zero();
        }
    
    GBA_TRACE_DEBUG((" BSF server address:"));
    GBA_TRACE_DEBUG( PtrBSFUrl );
    
    GetBootstrappingMaterialL(
                                   PtrBSFUrl,
                                   iSession->iGbaInputParams.iNAFName,
                                   iSession->iGbaInputParams.iUICCLabel,
                                   iSession->iGbaInputParams.iFlags,
                                   iSession->iGbaInputParams.iProtocolIdentifier,
                                   iSession->iGbaOutputParams.iKNAF, 
                                   iSession->iGbaOutputParams.iBTID, 
                                   iSession->iGbaOutputParams.iLifetime,
                                   iSession->iGbaOutputParams.iGbaRunType,
                                   iSession->iGbaInputParams.iAPID );
    
    GBA_TRACE_DEBUG(("Bootstrap Request is sent out, let us wait for callback "));
    
    CleanupStack::PopAndDestroy( BsfUrl );
    }

// -----------------------------------------------------------------------------
// C3GPPBootstrap::GetBootstrappingMaterialL()
// -----------------------------------------------------------------------------
//
void C3GPPBootstrap::GetBootstrappingMaterialL(
                                    const TDesC8& aBSFAddress, 
                                    const TDesC8& aNAFURL,
                                    const TDesC8& aUICC_LABEL,
                                    const TUint8& aFlags,
                                    const TDesC8& aKeyUsage,
                                    TDes8 &aKsNAF, 
                                    TDes8 &aBTID,
                                    TTime &aLifetime,
                                    EGBARunType& aGBARunType, 
                                    const TInt& aIAPID )
    { 
    GBA_TRACE_BEGIN();
    //save the return buffer's pointer. Copy the databack when bootstrapping is done
    iCallerKsNAFBuf = &aKsNAF;
    iCallerLifetime = &aLifetime;
    iCallerBTIDBuf  = &aBTID;
    iCallerGBARunType = &aGBARunType;

    TUriParser8 uriparser;
    

    GBA_TRACE_DEBUG(("the given naf url is:"));    
    GBA_TRACE_DEBUG(aNAFURL);
    
    //this implementation is used to solve symbian API's problem
    // when parsing pure ip address
    if ( (aNAFURL.FindF(KHTTPTag) != KErrNotFound) || (aNAFURL.FindF(KHTTPSTag) != KErrNotFound) )
        {
        //the naf url has http tag
        User::LeaveIfError( uriparser.Parse(aNAFURL) );
        } 
    else
        {
        GBA_TRACE_DEBUG(("the naf url has no http tag, add one for parser"));
        HBufC8* temp = HBufC8::NewL( aNAFURL.Length() +  KHTTPTagLength );
        CleanupStack::PushL(temp);
        TPtr8 ptrtemp = temp->Des();
        ptrtemp.Copy( KHTTPTag );
        ptrtemp.Append(aNAFURL);
        GBA_TRACE_DEBUG( *temp );
        User::LeaveIfError( uriparser.Parse( *temp ) );
        CleanupStack::PopAndDestroy(temp);
        }
    
  
    //Calculate the NAFID, NAFID= NAF_FQDN + keyusage
    
    TPtr8 ptrNAFID = iNAFID->Des();
    //parse the FQDN out from NAFaddress
    ptrNAFID.Copy( uriparser.Extract(EUriHost) );
    ptrNAFID.Append( aKeyUsage );
    
    GBA_TRACE_DEBUG(("NAF ID ="));
    GBA_TRACE_DEBUG(*iNAFID);
    
    
    GBA_TRACE_DEBUG(("UICC label is:"));
    GBA_TRACE_DEBUG(aUICC_LABEL);
  
    // Get IMPI   
    TPtr8 ptrIdentity = iIdentity->Des();
    //Clean the buffer and Set Length to 0
    ptrIdentity.Zero();
  
    //get the IMPI from smart card
    iSmartCardInterface->QueryIdentityL(ptrIdentity);
  
    GBA_TRACE_DEBUG(("IMPI is:"));
    GBA_TRACE_DEBUG( *iIdentity );
  
    // Get GBA_U availability 
    // EFalse means the call is not from gba client directly.
    iGBAUAvailable = CheckGBAUAvailabilityL( EFalse );

  
    if ( iGBAUAvailable )
        {
        GBA_TRACE_DEBUG(("GBA-U is available"));    
        }
    else
        {
        GBA_TRACE_DEBUG(("GBA-U is not available"));
        }   
  
    // Load Credentails from store.
    // First compare the IMPI and key lifetime
    // If IMPI is different or key expires, then return EFalse to 
    // start a new bootstrapping.
  
    // If IMPI & key lifetime checking both ok,
    // GBA_U will load B-TID and key lifetime and ask smart card to 
    // calculate the Ks_NAF
    // GBA_ME will load Ks, Rand, B-TId and key lifetime and calculate it
    // by ourselves.
    
    // The GBA run type is also taken from cached store
    
    if( !(aFlags & EGBAForce) && LoadCredentialsL() ) 
        {
        GBA_TRACE_DEBUG(("Still ok ???"));
        iStatus = KRequestPending;
        SetActive();
        iBSState = EBusy;
        TRequestStatus* callerStatus = &iStatus;
        User::RequestComplete(callerStatus, KErrNone);
        return;
        }
  
    // check if the BSF URL was overriden
    TBuf8<KMaxURLLength> BsfUrl;
    
    if ( aBSFAddress.Length() == 0 )
        {
        GBA_TRACE_DEBUG(("No preset BSF URL, calcuate from IMPI"));
        iSmartCardInterface->QueryHomeNetworkDnL(BsfUrl);
        }
    else
        {
        if ( iSmartCardInterface->InterfaceIs2G() )
            { 
            BsfUrl.Copy( KHTTPSTag );
            }
        else
            {
            BsfUrl.Copy( KHTTPTag ); 
            }    
       BsfUrl.Append( aBSFAddress );
       }
  
    GBA_TRACE_DEBUG(("The bsf url is:"));
    GBA_TRACE_DEBUG(BsfUrl);
    
    // dataretriever makes the actual http connection to the bsf
    // when it completes values can be retrieved 
    TUriParser8 uriparser1;
    User::LeaveIfError( uriparser1.Parse(BsfUrl) );
    
    iStatus = KRequestPending;
    SetActive(); // event should come from bootstrapper 
    iBSState = EBusy;

    
    //set reaml as FQDN of bsf
    iDataRetriever->MakeRequestL( &iStatus, *iIdentity, uriparser1.Extract( EUriHost ), BsfUrl, aNAFURL, aIAPID );
    GBA_TRACE_END();
    }

// -----------------------------------------------------------------------------
// C3GPPBootstrap::GBAUAvailabilityL()
// -----------------------------------------------------------------------------
//
TBool C3GPPBootstrap::GBAUAvailabilityL()
    {
    //the call is from gba client directly
    return CheckGBAUAvailabilityL( ETrue );
    }

// -----------------------------------------------------------------------------
// C3GPPBootstrap::GetState()
// -----------------------------------------------------------------------------
//
TInt C3GPPBootstrap::GetState() const 
    {   
    return iBSState;  
    };    

// -----------------------------------------------------------------------------
// C3GPPBootstrap::CompleteBootStrappingL()
// This is the callback function called by datareceiver
// when B-TID and key lifetime is received from BSF
// -----------------------------------------------------------------------------
//
void C3GPPBootstrap::CompleteBootStrappingL( TInt aError )
    {
    // check that we have handler to receive data  
    GBA_TRACE_DEBUG(("C3GPPBootstrapImpl::CompleteBootStrappingL"));

    // Check if triplet auth failed.
    if ( aError != KErrNone ) 
        {
        GBA_TRACE_DEBUG(("Bootstrap failed"));
        return;
        }
  
    HBufC8* rspbody = iDataRetriever->QueryResponseValueL();
    if (!rspbody)
        {
        GBA_TRACE_DEBUG(("Error no response body"));
        User::LeaveIfError( KErrGeneral );
        }
    CleanupStack::PushL( rspbody );
    GBA_TRACE_DEBUG(("response body is received. Loaded response:"));
    GBA_TRACE_DEBUG(*rspbody);

    // make sure it contains a B-TID
    TInt p = rspbody->FindC(KBTIDFlag);
    TInt e = rspbody->FindC(KBTIDEndFlag);
    if ( p != KErrNotFound && e != KErrNotFound)
        {
        p += TPtrC8(KBTIDFlag).Length();
        TInt length = e-p;
        
        //Copy the B-TID value to caller's buffer
        //Continue getting Ks_(ext)_NAF
        iCallerBTIDBuf->Copy(rspbody->Mid(p,length)); 
        
        //The B-TID needs to be stored in smart card.
        //Must save here for later use.
        TPtr8 ptrBTID = iBTID->Des();
        ptrBTID.Copy(*iCallerBTIDBuf);
        } 
    else 
        {  
        GBA_TRACE_DEBUG(("Error didn't contain BTID"));
        CleanupStack::PopAndDestroy(rspbody);
        User::LeaveIfError( KErrGeneral );
        }
  
    GBA_TRACE_DEBUG(("Extracted B-TID:"));
    GBA_TRACE_DEBUG(*iBTID);

    TBuf8<KMaxLengthTimeString> LifetimeBuf;
    p = rspbody->FindC(KLifetimeFlag);
    e = rspbody->FindC(KLifetimeEndFlag);
    if ( p != KErrNotFound && e != KErrNotFound )
        {
        p += TPtrC8(KLifetimeFlag).Length();
        TInt length = e-p;
        LifetimeBuf.Copy( rspbody->Mid(p,length) );
        } 

    CleanupStack::PopAndDestroy(rspbody);

    GBA_TRACE_DEBUG(("Lifetime in string:"));
    GBA_TRACE_DEBUG(LifetimeBuf);
    
    //Convert the string to TTime object
    //
    if ( !ConvertStringToTTime( LifetimeBuf, &iLifetime ) )
        {
        GBA_TRACE_DEBUG(("Couldn't convert lifetime"));
        User::LeaveIfError( KErrGeneral );
        }

    // Set the lifetime to caller's buffer
    *iCallerLifetime = iLifetime;
    
    //After bootstrap is succesful, the B-TID and keylifetime
    //should be sent back to smart card to store in GBA_U.
    if ( iGBAUAvailable )
        {
        HBufC8* lifetime = HBufC8::NewLC( LifetimeBuf.Length() );
        TPtr8 lifetimePtr = lifetime->Des();
        lifetimePtr.Copy( LifetimeBuf );
        TInt err = iSmartCardInterface->UpdateGBADataL( *iBTID, *lifetime );
        CleanupStack::PopAndDestroy( lifetime );
        User::LeaveIfError( err );
        iGBARunType = EGBAU;
        }
    else
        {
        if ( iSmartCardInterface->InterfaceIs2G() )
            {
            iGBARunType = E2GGBA;
            }
        else
            {
            iGBARunType = E3GGBAME;
            }    
        }

    // Set the GBA run type back to caller   
    *iCallerGBARunType = iGBARunType;  
    // cache the new fangled credentials
    StoreCredentialsL();
  
    // Generate Key material and copy it to parameter being returned 
    if ( GenerateKeyMaterialL() )
        {
    
        if ( iGBAUAvailable )
            {
            GBA_TRACE_DEBUG(("It is GBA_U mode,The ks_ext_naf is calculated by smart card"));
            GBA_TRACE_DEBUG_BINARY( *iKsNAF); 
            }
        else
            {
            GBA_TRACE_DEBUG(("It is GBA_ME mode,The ks_naf is calculated by gbamodule"));
            GBA_TRACE_DEBUG_BINARY( *iKsNAF );  
            GBA_TRACE_DEBUG(("It is GBA_ME mode,The ks_naf is calculated by gbamodule 1"));
            }
        //Set KsNAF back to caller's buffer
        iCallerKsNAFBuf->Copy(*iKsNAF);     
        }
    else
        {
        GBA_TRACE_DEBUG(("Error no keys"));
        User::LeaveIfError( KErrGeneral );
        }
    GBA_TRACE_END();
    }


// -----------------------------------------------------------------------------
// C3GPPBootstrap::GetCredentialsL()
// function GetCredentialsL
// discussion:  Gets URI, Realm, Nonce and AuthenticationType, and gives back Username and Password.
// param: :aURI : the address of the bsf.
// param: :aRealm : the realm of the bsf.
// param: :aNonce : the nonce sent by the bsf.
// param: :aAuthenticationType : the authentication type sent by the bsf.
// param: :aUsername : Generated Username by underlying algorithm (AKA or SIM).
// param: :aPassword : Generated Password by underlying algorithm (AKA or SIM).
// result:  The result of the operation: true for success and false for failure!
// ----------------------------------------------------------------------------- 
TBool C3GPPBootstrap::GetCredentialsL(const TUriC8& /*aURI*/, RString aRealm, RString aNonce, 
                      RString aAlgorithm,
                      RString& aUsername, 
                      RString& aPassword,
                      RString& aResync,
                      TBool& aResyncRequired)
    {
    GBA_TRACE_BEGIN();    
    
    // smartcard interface returns true if credentials are received
    TBool GotKeys = EFalse;
    TInt pushCount = 0;
  
    HBufC *algType=HBufC::NewLC( KAlgorithmTybeBufferLength );
    pushCount++;
    
    TPtr pAlgType(algType->Des());
    pAlgType.Copy(aAlgorithm.DesC());
    pAlgType.LowerCase();
  
    TInt SecurityAlgorithm;
    
    //AuthenticationType MUST be AKAV1-MD5
    //Find() function returns the offset of the data sequence from the beginning of 
    //this descriptor's data. KErrNotFound, if the data sequence cannot be found.
    if ( pAlgType.Find(Kakav1) != KErrNotFound ) 
        {
        GBA_TRACE_DEBUG((" algorithm is akav1")); 
        SecurityAlgorithm = EAKAv1;
        }
    else 
        {
        GBA_TRACE_DEBUG((" algorithm is unknown")); 
        SecurityAlgorithm = ESecAlgUnknown;
        }   
  
    HBufC8 *nonce = HBufC8::NewLC(KStringLength);
    pushCount++;

    TPtr8 ptrNonce = nonce->Des();
    ptrNonce.Copy(aNonce.DesC());
    
    GBA_TRACE_DEBUG((" The base64 encoded nonce value from BSF server (RAND + AUTN)"));
    GBA_TRACE_DEBUG(aNonce.DesC());
    GBA_TRACE_DEBUG((" The base64 encoded nonce value after copying into buffer:"));
    GBA_TRACE_DEBUG(*nonce);
  
    //buffer used to store decoded nonce value
    HBufC8 *decodedNonce = HBufC8::NewLC(KStringLength);
    pushCount++;

    TPtr8 ptrDecNonce = decodedNonce->Des();
    
    //fill the buffer with zeros.
    ptrDecNonce.FillZ();
    
    // decodes the base64 nonce
    TBase64 b64coder;
    b64coder.Decode( *nonce, ptrDecNonce );
    
    GBA_TRACE_DEBUG((" The decoded nonce value in binary"));
    GBA_TRACE_DEBUG_BINARY( *decodedNonce );
  
    //Allocate buffer RES, password for http digest
    HBufC8* RES = HBufC8::NewLC( KMaxLengthRES );
    pushCount++;
    TPtr8 ptrRES = RES->Des();
    
    //Allocate buffer AUTS, if the out of sync happens        
    HBufC8* AUTS = HBufC8::NewLC( KMaxLengthAUTS );
    pushCount++;
    TPtr8 ptrAUTS = AUTS->Des();
    
    GBA_TRACE_DEBUG((" check algorithm value"));
    switch (SecurityAlgorithm) 
        {
    case EAKAv1: 
        {  
        GBA_TRACE_DEBUG((" EAKAv1"));  
        aResyncRequired = EFalse;
        //Set the RAND AUTN to smart card
        GotKeys = iSmartCardInterface->QueryAuthenticationL( *decodedNonce, ptrRES, ptrAUTS );
        
        TBool AUTSBufEmpty = EFalse;

        // No out of sync detected
        if ( GotKeys || AUTS->Length() == 0 )
            {
            GBA_TRACE_DEBUG(("AUTS is not returned. No out of sync detected"));
            AUTSBufEmpty = ETrue;
            }
          
        if ( !GotKeys && !AUTSBufEmpty && AUTS->Length()>0 )
            {
            aResyncRequired = ETrue;
            GBA_TRACE_DEBUG(("AUTS is returned. Resync is required"));
            GBA_TRACE_DEBUG_BINARY(*AUTS);
            }
            
      // keep rand
      // as the first 16 bytes in Nonce as RAND
      TPtr8 ptrRand = iRand->Des();
      ptrRand.Copy( ptrDecNonce.Left(KMaxLengthRAND) );
      
      GBA_TRACE_DEBUG(("RAND is:"));
      GBA_TRACE_DEBUG_BINARY(*iRand);
      
      //if GBA_U is available, we donot need to save the 
      //Ks. It is stored on smart card
      
      if( GotKeys && !iGBAUAvailable ) 
          {
          // keep master key
          TPtr8 ptrMasterKey = iMasterKey->Des();
          if ( !iSmartCardInterface->QueryKs( ptrMasterKey ) )
              {
              User::LeaveIfError( KErrGeneral );
              }   
          }
      }
        break;
    
      case ESecAlgUnknown:
          {
          User::LeaveIfError( KErrArgument );
          }
          break;
        
      default:
           {
           User::LeaveIfError( KErrArgument );    
           } 
        break;
        }
  

    GBA_TRACE_DEBUG(("AKA password RES for BSF"));
    GBA_TRACE_DEBUG_BINARY( *RES );
  
    // if received keys or auts then we can attempt to authenticate towards the BSF 
    // is this code setting the password? Password should be the RES or empty
        
    if ( aResyncRequired )
    //AUTS returned
        {
        _LIT8(KEmptyPwd,"");
        aPassword = aRealm.Pool().OpenStringL(KEmptyPwd);
        }

    if(GotKeys)
        {
        //Set RES as password
        TRAPD( err,aPassword = aRealm.Pool().OpenStringL(*RES) );
        err = err;
        GBA_TRACE_DEBUG_NUM((" Set RES as password error %d"), err );
        }
    
    //Set IMPI as username
    aUsername = aRealm.Pool().OpenStringL(iIdentity->Des());
    
    // convert received auts into a string
    if ( aResyncRequired )
        {
        GBA_TRACE_DEBUG(("adding auts to the headers"));      
        HBufC8 *encodedAUTS = HBufC8::NewLC( KMaxLengthEnAUTS );
        pushCount++;
        TPtr8 ptrEncAUTS = encodedAUTS->Des();
        ptrEncAUTS.FillZ();

        b64coder.Encode( *AUTS, ptrEncAUTS );
                
        GBA_TRACE_DEBUG(*encodedAUTS);
      
        aResync = aRealm.Pool().OpenStringL(*encodedAUTS);
        }
      
    CleanupStack::PopAndDestroy(pushCount);
    GBA_TRACE_END();
    return GotKeys;
    }

// -----------------------------------------------------------------------------
// C3GPPBootstrap::GenerateKeyMaterialL()
// ----------------------------------------------------------------------------- 
TBool C3GPPBootstrap::GenerateKeyMaterialL() 
    {
    TPtr8 derivedKey(iKsNAF->Des());
    
    if ( iGBAUAvailable )
        {
        iSmartCardInterface->QueryKeyMaterialL(
              KNullDesC8,
              KNullDesC8,
              iIdentity->Des(),
              *iNAFID,
              derivedKey
              );
        return ETrue; 
        }
    else
        {
        iSmartCardInterface->QueryKeyMaterialL(
                                  iMasterKey->Des(),
                                  iRand->Des(),
                                  iIdentity->Des(),
                                  *iNAFID,
                                  derivedKey
                                  );
        return ETrue; 
        }
    }


// -----------------------------------------------------------------------------
// C3GPPBootstrap::Cleanup()
// ----------------------------------------------------------------------------- 
void C3GPPBootstrap::Cleanup()
    {
    GBA_TRACE_DEBUG(("Enter "));
    iBSState = EIdle;
    iGBAUAvailable = EFalse;
    iGBARunType = ENoType;
    
    iKsNAF->Des().Zero(); 
    iBTID->Des().Zero();   
    iLifetime = 0; 

    iIdentity->Des().Zero(); //buffer for impi
    iNAFID->Des().Zero();    //buffer for nafid
    iRand->Des().Zero();     //buffer for rand
    iMasterKey->Des().Zero(); //buffer for Ks
    GBA_TRACE_DEBUG(("Exit "));
    }


// -----------------------------------------------------------------------------
// C3GPPBootstrap::LoadCredentials()
// GBA_ME, load the cached credentails to calculate ks_naf 
// ----------------------------------------------------------------------------- 
TBool C3GPPBootstrap::LoadCredentialsL()
    {
    GBA_TRACE_DEBUG(("Enter "));
    TBool result = EFalse;
  
    // check if the values are in memory
    // IsStillValid needs to be implemented
    //if( iMasterKey && IsStillValid( &iLifetime ))
      //{
      //GBA_TRACE_DEBUG(("using the Ks in memory"));
      //return ETrue;
      //}
    
    GBA_TRACE_DEBUG(("Checking if the credtials are stored"));
    
    RFs fs;
    User::LeaveIfError( fs.Connect() ); 
    CleanupClosePushL(fs);
    TFindFile folder( fs );
    
    TFileName fullPath;
    
    MakePrivateFilenameL(fs, KGbaCredentialsFileName, fullPath);
    GBA_TRACE_DEBUG(fullPath);
    
    TInt err = folder.FindByDir( fullPath, KNullDesC);
  
    if (  err != KErrNone )
        {
        GBA_TRACE_DEBUG(("No credentials store available"));
        CleanupStack::PopAndDestroy(&fs);
        return result;
        }
    else
        {
        GBA_TRACE_DEBUG(("Found credentials store "));
        CDictionaryFileStore* pStore = CDictionaryFileStore::OpenLC(fs,fullPath,KUidGBACredRoot);    
        
        TInt length = 0;
        HBufC8* tmpIdentity = NULL;

        //Read IMPI
        if ( pStore->IsPresentL( KUidIMPI ) )
            {
            GBA_TRACE_DEBUG(("IMPI is present "));
            RDictionaryReadStream reader;
            CleanupClosePushL(reader);
            reader.OpenL(*pStore,KUidIMPI); 
            length = reader.ReadInt32L();
            tmpIdentity = HBufC8::NewL(length);
            TPtr8 ptrtmpIdentity = tmpIdentity->Des();
      
            reader.ReadL(ptrtmpIdentity,length);
            GBA_TRACE_DEBUG(("IMPI = ")); 
            GBA_TRACE_DEBUG(ptrtmpIdentity);
            CleanupStack::PopAndDestroy( &reader );
            }
        
        //Read ks
        if ( pStore->IsPresentL( KUidKs ) )
            {
            GBA_TRACE_DEBUG(("Ks is present, the cached credentials are from GBA_ME "));
            GBA_TRACE_DEBUG(("Ks is present "));
            RDictionaryReadStream reader1;
            CleanupClosePushL(reader1);
            reader1.OpenL(*pStore,KUidKs); 
            length = reader1.ReadInt32L();
            
            TPtr8 ptrmk = iMasterKey->Des();
            ptrmk.Zero();
            reader1.ReadL(ptrmk,length);
            ptrmk.SetLength(length);
            GBA_TRACE_DEBUG(("Ks = ")); 
            GBA_TRACE_DEBUG_BINARY(ptrmk);
            CleanupStack::PopAndDestroy( &reader1 );
            }
      
      
      //Read rand
        if ( pStore->IsPresentL( KUidRand ) )
            {
            GBA_TRACE_DEBUG(("Rand is present "));
            RDictionaryReadStream reader2;
            CleanupClosePushL(reader2);
            reader2.OpenL(*pStore,KUidRand); 
            length = reader2.ReadInt32L();

            TPtr8 ptrRand = iRand->Des();
            ptrRand.Zero();
            reader2.ReadL(ptrRand,length);
            ptrRand.SetLength(length);
            GBA_TRACE_DEBUG(("Rand = ")); 
            GBA_TRACE_DEBUG_BINARY(ptrRand);  
            CleanupStack::PopAndDestroy( &reader2 );
            }  
        
        //Read BTID
        if ( pStore->IsPresentL( KUidBTID ) )
            {
            GBA_TRACE_DEBUG(("B-TID is present "));
            RDictionaryReadStream reader3;
            CleanupClosePushL(reader3);
            reader3.OpenL(*pStore,KUidBTID); 
            length = reader3.ReadInt32L();

            TPtr8 ptrBTID = iBTID->Des();
            ptrBTID.Zero();
            reader3.ReadL(ptrBTID,length);
            ptrBTID.SetLength(length);
            GBA_TRACE_DEBUG(("BTID = ")); 
            GBA_TRACE_DEBUG(ptrBTID); 
            CleanupStack::PopAndDestroy( &reader3 );
            }
      
        //Read key lifetime
        if ( pStore->IsPresentL( KUidkeylifetime ) )
            {
            GBA_TRACE_DEBUG(("key lifetime is present "));
            RDictionaryReadStream reader4;
            CleanupClosePushL(reader4);
            reader4.OpenL(*pStore,KUidkeylifetime); 
            iLifetime = NULL;
            TPtr8 ptrkeylifetime((TUint8*)&iLifetime,sizeof(TTime));
          
            reader4.ReadL(ptrkeylifetime,sizeof(TTime));
            GBA_TRACE_DEBUG(("keylifetime = "));
            GBA_TRACE_TIME( iLifetime );
            CleanupStack::PopAndDestroy( &reader4 );
            }  
      
        //Read GBA run type
        if ( pStore->IsPresentL( KUidGBARunType ) )
            {
            GBA_TRACE_DEBUG(("GBARunType is present "));
            RDictionaryReadStream reader5;
            CleanupClosePushL(reader5);
            reader5.OpenL(*pStore,KUidGBARunType); 
            iGBARunType = (EGBARunType)reader5.ReadInt8L();  
            CleanupStack::PopAndDestroy( &reader5 );
            }
      
      
        //done with store
        CleanupStack::PopAndDestroy( pStore );
	    CleanupStack::PopAndDestroy(&fs);
                                           
        result = IsStillValid( &iLifetime ) && *tmpIdentity == *iIdentity;
        
        //smart card has been changed
        if ( *tmpIdentity != *iIdentity )
            {
        	iSmartCardInterface->NotifyCardChangeL();
            }
        
        GBA_TRACE_DEBUG(("Cached IMPI = ")); 
        if(tmpIdentity != NULL)
        	GBA_TRACE_DEBUG(*tmpIdentity);
        GBA_TRACE_DEBUG(("IMPI from smart card = ")); 
        GBA_TRACE_DEBUG(*iIdentity);
        
        // this has served its purpose
        delete tmpIdentity;

        // if the credentials aren't valid delete the old key
        if ( !result )
            {
            GBA_TRACE_DEBUG(("clean buffers since the key wasn't valid anymore or the card had been changed"));
            // clean up the created buffers;
            iMasterKey->Des().Zero();
            iRand->Des().Zero();
            iBTID->Des().Zero();
            iLifetime = NULL;
            }
        else
            {
            GBA_TRACE_DEBUG(("cached keys were still valid"));
            GBA_TRACE_DEBUG(("Cached B-TID is:"));
            GBA_TRACE_DEBUG(*iBTID );
            
            iCallerBTIDBuf->Copy( *iBTID );
            
            // Set the run type
            *iCallerGBARunType = iGBARunType;
            
            //Set key life time
            *iCallerLifetime = iLifetime;
            
            // generate material based on the stored credentials
            if ( GenerateKeyMaterialL() )
                {
                if ( iGBAUAvailable )
                    {
                    GBA_TRACE_DEBUG(("Calu from Cached Ks: It is GBA_U mode,The ks_ext_naf is calculated by smart card"));
                    GBA_TRACE_DEBUG_BINARY( *iKsNAF ); 
                    GBA_TRACE_DEBUG(("Calu test11"));

                    }
                else
                    {
                    GBA_TRACE_DEBUG(("Calu from Cached Ks. It is GBA_ME mode,The ks_naf is calculated by gbamodule"));
                    GBA_TRACE_DEBUG_BINARY( *iKsNAF ); 
                    }
                    //Set KsNAF back to caller's buffer
                iCallerKsNAFBuf->Copy(*iKsNAF);       
                }
            else
                {
                GBA_TRACE_DEBUG(("Error no keys"));
                result = EFalse;
                }
            }
         GBA_TRACE_DEBUG(("Calu test11"));   
        }
    return result;
    }



// -----------------------------------------------------------------------------
// C3GPPBootstrap::StoreCredentialsL()
// Store bootstrapping credentails into the store
// GBA_ME: IMPI, Ks, Rand, B-TID, key lifetime
// GBA_U:  IMPI, B-TID, key lifetime
// ----------------------------------------------------------------------------- 
void C3GPPBootstrap::StoreCredentialsL()
    {
    GBA_TRACE_DEBUG(("Save credentials into dictionary store"));
  
    RFs fs;    
    User::LeaveIfError(fs.Connect());
    CleanupClosePushL(fs);
    TFindFile folder( fs );
    
    TFileName fullPath;
    
    MakePrivateFilenameL(fs, KGbaCredentialsFileName, fullPath);
    EnsurePathL(fs, fullPath );

    GBA_TRACE_DEBUG(fullPath);
    TInt err = folder.FindByDir( fullPath, KNullDesC );
  
    if (  err == KErrNotFound || err == KErrNone || err == KErrPathNotFound )  
        {
    
        CDictionaryFileStore* pStore = CDictionaryFileStore::OpenLC(fs,fullPath,KUidGBACredRoot);   
        
        //Save IMPI
        RDictionaryWriteStream writer;
        CleanupClosePushL(writer);
        writer.AssignL(*pStore, KUidIMPI);
        writer.WriteInt32L(iIdentity->Size());
        writer.WriteL(*iIdentity);
        writer.CommitL();
        CleanupStack::PopAndDestroy( &writer );
       
        //GBA_U doesn't need to save ks and rand.
        if ( !iGBAUAvailable )
            {
            GBA_TRACE_DEBUG(("It is GBA_ME, save Ks and Rand"));
            //Save ks
            RDictionaryWriteStream writer1;
            CleanupClosePushL(writer1);
            writer1.AssignL( *pStore, KUidKs );
            writer1.WriteInt32L(iMasterKey->Size());
            writer1.WriteL(*iMasterKey);
            writer1.CommitL();
            CleanupStack::PopAndDestroy( &writer1 );
          
            //Save Rand
            RDictionaryWriteStream writer2;
            CleanupClosePushL(writer2);
            writer2.AssignL(*pStore,KUidRand );
            writer2.WriteInt32L(iRand->Size());
            writer2.WriteL(*iRand);
            writer2.CommitL();
            CleanupStack::PopAndDestroy( &writer2 );
            }
        else
            {
            GBA_TRACE_DEBUG(("It is GBA_U, remove Ks and Rand if there are"));    
            pStore->RemoveL(KUidKs);
            pStore->RemoveL(KUidRand);
            }    
        //Save B-TID
        RDictionaryWriteStream writer3;
        CleanupClosePushL(writer3);
        writer3.AssignL(*pStore,KUidBTID );
        writer3.WriteInt32L(iBTID->Size());
        writer3.WriteL(*iBTID);
        writer3.CommitL();
        CleanupStack::PopAndDestroy( &writer3 );
      
        //Save key lifetime
        RDictionaryWriteStream writer4;
        CleanupClosePushL(writer4);
        writer4.AssignL(*pStore,KUidkeylifetime );
        TPtrC8 ptrkeylifetime((TUint8*)&iLifetime,sizeof(TTime));
        writer4.WriteL(ptrkeylifetime); 
        writer4.CommitL();
        CleanupStack::PopAndDestroy( &writer4 );
      
        //Save gba run type
        RDictionaryWriteStream writer5;
        CleanupClosePushL(writer5);
        writer5.AssignL(*pStore,KUidGBARunType);
        writer5.WriteInt8L(iGBARunType);
        writer5.CommitL();
        CleanupStack::PopAndDestroy( &writer5 );
      
        pStore->CommitL();
        CleanupStack::PopAndDestroy( pStore );
        CleanupStack::PopAndDestroy( &fs );     
        }
    else
        {
        CleanupStack::PopAndDestroy( &fs );
        User::LeaveIfError( err );    
        } 
    }

// -----------------------------------------------------------------------------
// C3GPPBootstrap::CheckGBAUAvailability()
// Check the GBA availability from store, if no record there, check from 
// smart card directly.
// -----------------------------------------------------------------------------
TBool C3GPPBootstrap::CheckGBAUAvailabilityL( TBool aIsfromGBAClient )
    {       
    TBool GBAUAvailability = EFalse;
    //Read the record from store
    TInt result = LoadGBAUAvailabililtyFromStoreL( aIsfromGBAClient, GBAUAvailability );
    
    //The store GBA type is GBA_U or GBA_ME
    if ( result != KErrNotFound && result != KErrGeneral )
        {
        //return the cached GBAU availabilty, since smart card is not changed
        return GBAUAvailability;
        }
    else if ( result == KErrGeneral )
        {
        //impi is different
        //delete the whole store
        RFs fs;
        User::LeaveIfError( fs.Connect() );
        CleanupClosePushL(fs);
        TFileName fullPath;
        MakePrivateFilenameL(fs, KGbaCredentialsFileName, fullPath);
        fs.Delete( fullPath );
        CleanupStack::PopAndDestroy( &fs );
        }
    else
        {
        //The first time query    
        } 
    // There is no record on GBA_U availability or impis are different   
    // It only happens on the first time when GBA server starts
    // or after smart card is removed.
    
    //retrieve the GBA availability from smart card
    iSmartCardInterface->QueryGBAUAvailabilityL( GBAUAvailability );
    
    SaveGBAUAvailabilityToStoreL( GBAUAvailability );
    return GBAUAvailability;   
    }


// -----------------------------------------------------------------------------
// C3GPPBootstrap::LoadGBAUAvailabililtyFromStoreL()
// -----------------------------------------------------------------------------
TInt C3GPPBootstrap::LoadGBAUAvailabililtyFromStoreL( TBool& aIsfromGBAClient, TBool& aGBAUAvail )
    { 
    GBA_TRACE_DEBUG(("Loading the GBA-U avail from store"));
    TInt cleanupstack = 0;
    RFs fs;
    User::LeaveIfError( fs.Connect() );
    CleanupClosePushL(fs);
    cleanupstack++;
    TFindFile folder( fs );
    
    
    TFileName fullPath;
    MakePrivateFilenameL(fs, KGbaCredentialsFileName, fullPath);
    GBA_TRACE_DEBUG(fullPath);
    TInt err = folder.FindByDir( fullPath, KNullDesC );
  
    // the store file is not available  
    if (  err != KErrNone )
        {
        GBA_TRACE_DEBUG(("No store available"));
        CleanupStack::PopAndDestroy(cleanupstack); 
        
        //get impi here, save to store later
        if ( aIsfromGBAClient )
            {
            //retrieve the impi from smart card
            TPtr8 idPtr = iIdentity->Des();
            idPtr.Zero();
            TRAPD(error,iSmartCardInterface->QueryIdentityL(idPtr)); 
    
            if( error != KErrNone )
                {
                GBA_TRACE_DEBUG((" Can't get IMPI from smart card"));
                User::LeaveIfError( error );
                }
        
            GBA_TRACE_DEBUG((" got IMPI"));    
            }
        return KErrNotFound;
      }
    else
       {
       GBA_TRACE_DEBUG(("Found store "));
       CDictionaryFileStore* pStore = CDictionaryFileStore::OpenLC(fs,KGbaCredentialsFileName,KUidGBACredRoot);    
       cleanupstack++;
       TInt impiLength = 0;
       HBufC8* cachedimpi = NULL;
       TInt8 GBAUAvail = 0;
       
       //Read GBA-U avail
       if ( pStore->IsPresentL( KUidGBAUAvail ) )
           {
           GBA_TRACE_DEBUG(("GBAU Avail is present "));
           RDictionaryReadStream reader;
           CleanupClosePushL(reader);
           reader.OpenL(*pStore,KUidGBAUAvail); 
           GBAUAvail = reader.ReadInt8L();
           CleanupStack::PopAndDestroy( &reader );
           }
            
       //Read IMPI
       if ( pStore->IsPresentL( KUidIMPI ) )
          {
          GBA_TRACE_DEBUG(("IMPI is present "));
          RDictionaryReadStream reader1;
          CleanupClosePushL(reader1);
          cleanupstack++; 
          reader1.OpenL(*pStore,KUidIMPI);
          impiLength = reader1.ReadInt32L();
        
          cachedimpi = HBufC8::NewLC(impiLength);
          cleanupstack++;
          TPtr8 ptrcachedimpi = cachedimpi->Des();
      
          reader1.ReadL(ptrcachedimpi,impiLength);
          ptrcachedimpi.SetLength(impiLength);
          GBA_TRACE_DEBUG(("IMPI = ")); 
          GBA_TRACE_DEBUG(ptrcachedimpi);

          if ( aIsfromGBAClient )
              {
              //retrieve the impi from smart card
              TPtr8 idPtr = iIdentity->Des();
              idPtr.Zero();
              TRAPD(error,iSmartCardInterface->QueryIdentityL(idPtr)); 
    
              if( error != KErrNone )
                  {
                  GBA_TRACE_DEBUG((" Can't get IMPI from smart card"));
                  User::LeaveIfError( error );
                  }
        
              GBA_TRACE_DEBUG((" got IMPI"));    
              }

        
          if( *iIdentity == *cachedimpi )
              {
              GBA_TRACE_DEBUG(("IMPI are the same"));    
              //the impi are the same, return the cached gbau availability
              CleanupStack::PopAndDestroy(cleanupstack);
              aGBAUAvail = (TBool)GBAUAvail;
              return KErrNone;
              }
          else
              {
              GBA_TRACE_DEBUG(("IMPIs are different"));    
              //the impi are different, return KErrGeneral
              iSmartCardInterface->NotifyCardChangeL();
              CleanupStack::PopAndDestroy(cleanupstack); 
              return KErrGeneral;    
              }
           }
        else
           {
           GBA_TRACE_DEBUG(("no impi present"));    
           return KErrGeneral;   
           }  
        }
    }

// -----------------------------------------------------------------------------
// C3GPPBootstrap::SaveGBAUAvailabilityToStoreL()
// -----------------------------------------------------------------------------
//
void C3GPPBootstrap::SaveGBAUAvailabilityToStoreL( TBool& aGBAUAvail )
    {
    GBA_TRACE_DEBUG(("Save GBA-U avail into dictionary store"));
  
    RFs fs;
    User::LeaveIfError(fs.Connect());
    CleanupClosePushL(fs);
    TFindFile folder( fs );
    
    TFileName fullPath;
    MakePrivateFilenameL(fs, KGbaCredentialsFileName, fullPath);
    EnsurePathL(fs, fullPath );
    GBA_TRACE_DEBUG(fullPath);
    TInt err = folder.FindByDir( fullPath, KNullDesC );
  
    if (  err == KErrNotFound || err == KErrNone || err == KErrPathNotFound )  
        {
    
        CDictionaryFileStore* pStore = CDictionaryFileStore::OpenLC(fs,fullPath,KUidGBACredRoot);   
      
        //Save IMPI
        RDictionaryWriteStream writer;
        CleanupClosePushL(writer);
        writer.AssignL(*pStore, KUidIMPI);
        writer.WriteInt32L(iIdentity->Size());
        writer.WriteL(*iIdentity);
        writer.CommitL();
        CleanupStack::PopAndDestroy( &writer );    
        //Save GBA-U avail
        RDictionaryWriteStream writer1;
        CleanupClosePushL(writer1);
        writer1.AssignL(*pStore,KUidGBAUAvail );
        writer1.WriteInt8L((TInt8)aGBAUAvail);
        writer1.CommitL();
        CleanupStack::PopAndDestroy( &writer1 );
      
        pStore->CommitL();
        CleanupStack::PopAndDestroy(pStore);
        CleanupStack::PopAndDestroy(&fs);
        GBA_TRACE_DEBUG(("Save GBA-U avail successfully"));    
        }
    else
        {
        GBA_TRACE_DEBUG(("Save GBA-U avail failed"));
        CleanupStack::PopAndDestroy(&fs);
        User::LeaveIfError( err );    
        }
    }

// -----------------------------------------------------------------------------
// C3GPPBootstrap::IsStillValid()
// -----------------------------------------------------------------------------
//
TBool C3GPPBootstrap::IsStillValid( TTime* aLifetime )
    {
    TTime nowTime;
    nowTime.UniversalTime();
    GBA_TRACE_DEBUG(("Current Time:"));
    GBA_TRACE_TIME( nowTime );
    
    GBA_TRACE_DEBUG(("Expiry Time:"));
    GBA_TRACE_TIME( *aLifetime );
    
    if( *aLifetime > nowTime )
        {
        GBA_TRACE_DEBUG(("key still valid"));
        return ETrue;
        }
    else
        {
        GBA_TRACE_DEBUG(("key expired"));
        return EFalse;
        }
    }

// -----------------------------------------------------------------------------
// C3GPPBootstrap::ConvertStringToTTime()
// -----------------------------------------------------------------------------
//
TBool C3GPPBootstrap::ConvertStringToTTime(TDesC8& aLifetimeBuf, TTime* aLifetime)
{
  GBA_TRACE_BEGIN();
  TInt aTimeVariable[KVariableIndex6] = 
  {
          0,0,0,0,0,0
  }; // { year,month,day,hour,minute,second }
  
  // assuming the lifetime is given in ISO 8601 format
  // YYYY-MM-DD or YYYYMMDD (we don't allow YYYYMM or YYYY format, although this isn't specified in the standard)
  // (year could have a + before it if key is valid beyond year 9999, but we hope that someone will rewrite this within next 8000 years)
  // there might be a T between date and time we just ignore it
  // hours 
  // hh:mm:ss or hhmmss
  // hh:mm or hhmm
  // hh

  TInt pos = 0;
  TInt len = KVariableIndex4;
  for(TInt variableIndex = 0; variableIndex < KVariableIndex6; variableIndex++)
  {
    // if next value isn't a digit we're done
    if(!TChar(aLifetimeBuf[pos]).IsDigit())
      break;
        
    if(pos + len < aLifetimeBuf.Length())
    {
    
      TLex8 lex(aLifetimeBuf.Mid(pos,len));
      if(KErrNone  != lex.Val(aTimeVariable[variableIndex]))
      {
        GBA_TRACE_DEBUG(("failed"));
        GBA_TRACE_DEBUG(aLifetimeBuf.Mid(pos,len));
        return EFalse;
      }
      pos += len;
      

      // remove possible separators
      if(aLifetimeBuf[pos] == '-' || aLifetimeBuf[pos] == ':')
        pos += 1;
      // there can be spaces after month
      if( variableIndex == KVariableIndex2 )
        while(pos < aLifetimeBuf.Length() && 
        (TChar(aLifetimeBuf[pos]).IsSpace() || aLifetimeBuf[pos] == 'T'))
            pos++;
      
    }
    else
    { 
      // if we don't get second it doesn't matter
      if(variableIndex >= KVariableIndex5)
        break; 
      return EFalse;
    }
    len = KVariableIndex2; // year is the exception 
  }
  // Symbian months and days start from 0, UTC months from 1
  aTimeVariable[1] = aTimeVariable[1] - 1;
  aTimeVariable[2] = aTimeVariable[2] - 1;

  TBuf<KBufferSize255> aDBBuf;
  aDBBuf.Format(KTimeFormat, aTimeVariable[KVariableIndex0],aTimeVariable[KVariableIndex1],aTimeVariable[KVariableIndex2],
                          aTimeVariable[KVariableIndex3],aTimeVariable[KVariableIndex4],aTimeVariable[KVariableIndex5]);
  GBA_TRACE_DEBUG(aDBBuf);

  // time should always be in UTC so no need to check the timezone
  TDateTime dt;

  TRAPD(error,dt.Set(
    aTimeVariable[KVariableIndex0],
    (TMonth)(aTimeVariable[KVariableIndex1]), 
    aTimeVariable[KVariableIndex2],
    aTimeVariable[KVariableIndex3],
    aTimeVariable[KVariableIndex4],
    aTimeVariable[KVariableIndex5],
    0
    ));

  if(error != KErrNone)
  {
    aDBBuf.Format(KErrorMessage,error);
    GBA_TRACE_DEBUG(aDBBuf);
    return EFalse;
  }

  *aLifetime = dt;
  
  GBA_TRACE_END();
  return ETrue;
}


// ---------------------------------------------------------------------------
// C3GPPBootstrap::CreateImplementationL()
// ---------------------------------------------------------------------------
//
MUICCInterface* C3GPPBootstrap::RequestUICCInterfaceL()
    {
    GBA_TRACE_BEGIN();
    TAny* ptr = REComSession::CreateImplementationL(iImplementationUID, iDtorIDKey);
    MUICCInterface* ptyped = static_cast<MUICCInterface*>(ptr);
    GBA_TRACE_END();
    return ptyped;
    }


// ---------------------------------------------------------------------------
// C3GPPBootstrap::IsPluginExistL()
// ---------------------------------------------------------------------------
//
TBool C3GPPBootstrap::IsPluginExistL()
    {
    GBA_TRACE_BEGIN();
    RImplInfoPtrArray array; 
    // Note that a special cleanup function is required to reset and destroy
    // all items in the array, and then close it.
    TCleanupItem cleanup(ResetAndDestroyArray, &array);
    CleanupStack::PushL(cleanup);  

    REComSession::ListImplementationsL( KGBAUICCInterfaceUID, array );
    
    if( array.Count())
        {
        CleanupStack::PopAndDestroy(&array); //array, results in a call to ResetAndDestroyArray
        GBA_TRACE_END();
        return ETrue;
        }
    else
        {
        GBA_TRACE_END();
        CleanupStack::PopAndDestroy(&array); //array, results in a call to ResetAndDestroyArray
        return EFalse;
        }
    }


// CleanupEComArray function is used for cleanup support of locally declared arrays
void ResetAndDestroyArray(TAny* aArray)
    {
    GBA_TRACE_BEGIN();
    (static_cast<RImplInfoPtrArray*> (aArray))->ResetAndDestroy();
    (static_cast<RImplInfoPtrArray*> (aArray))->Close();
    GBA_TRACE_END();
    }


// ---------------------------------------------------------------------------
// C3GPPBootstrap::InterfaceIs2G()
// ---------------------------------------------------------------------------
//
TBool C3GPPBootstrap::InterfaceIs2G() 
    {
    return iSmartCardInterface->InterfaceIs2G();
    }
    
    
// ---------------------------------------------------------------------------
// C3GPPBootstrap::MakePrivateFilenameL()
// ---------------------------------------------------------------------------
//
void C3GPPBootstrap::MakePrivateFilenameL(RFs& aFs, const TDesC& aLeafName, 
                                     TDes& aNameOut)
    {
    aNameOut.Copy(KGBAStoreStandardDrive);
    // Get private path
    TBuf<KStringLength> privatePath;
    aFs.PrivatePath(privatePath);
    aNameOut.Append(privatePath);
    aNameOut.Append(aLeafName);
    }
    
      
// ---------------------------------------------------------------------------
// C3GPPBootstrap::EnsurePathL()
// ---------------------------------------------------------------------------
//      
void C3GPPBootstrap::EnsurePathL( RFs& aFs, const TDesC& aFile )
    {
    TInt err = aFs.MkDirAll(aFile);
    if (err != KErrNone && err != KErrAlreadyExists)
        {
        User::Leave(err);
        }
    }   
   
//EOF


