/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Implementation of Wim key store interface
*
*/


// INCLUDE FILES

#include "WimKeyStore.h"
#include "WimKeyDetails.h"
#include "WimToken.h"
#include "WimPin.h"
#include "WimAuthenticationObject.h"
#include "WimTrace.h"
#include "WimTokenListener.h"
#include "WimImplementationUID.hrh"
#include "WimRSASigner.h"
#include "WimConsts.h"
#include <mctauthobject.h>
#include <ct.h>
#include <asymmetric.h>


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWimKeyStore::CWimKeyStore()
// Default constructor
// -----------------------------------------------------------------------------
//
CWimKeyStore::CWimKeyStore( CWimToken& aToken ) :
                            CActive( EPriorityNormal ),
                            iToken( aToken )

    {
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CWimKeyStore::NewL()
// Two-phased constructor
// -----------------------------------------------------------------------------
//
CWimKeyStore* CWimKeyStore::NewL( CWimToken& aToken )
    {
    _WIMTRACE ( _L( "CWimKeyStore::NewL()" ) );
    CWimKeyStore* self = new( ELeave ) CWimKeyStore( aToken );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CWimKeyStore::ConstructL()
// Second phase constructor
// -----------------------------------------------------------------------------
//
void CWimKeyStore::ConstructL()
    {
    _WIMTRACE ( _L( "CWimKeyStore::ConstructL()" ) );
    iCWimKeyDetails = CWimKeyDetails::NewL( Token() );
    iKeyNumbers = new( ELeave )CArrayFixFlat<TUint8>(1);
    iPhase = EIdle;
    iPhaseOriginal = EIdle;
    }

// -----------------------------------------------------------------------------
// CWimKeyStore::~CWimKeyStore()
// Destructor
// -----------------------------------------------------------------------------
//
CWimKeyStore::~CWimKeyStore()
    {
    _WIMTRACE ( _L( "CWimKeyStore::~CWimKeyStore()" ) );
    Cancel();
    TInt count = iKeyInfos.Count();
    for ( TInt i = 0; i < count; i++ )
        {
        iKeyInfos[i]->Release();
        }
    iKeyInfos.Close();

    if ( iKeyNumbers )
        {
        iKeyNumbers->Reset();
        delete iKeyNumbers;
        }

    delete iSignature;

    if ( iCWimKeyDetails )
        {
        delete iCWimKeyDetails;
        iCWimKeyDetails = NULL;
        }

    delete iKeyId;
    }
// -----------------------------------------------------------------------------
// CWimKeyStore::DoRelease()
// Deletes this interface when Release() for this instance is called.
// -----------------------------------------------------------------------------
//
void CWimKeyStore::DoRelease()
    {
    _WIMTRACE ( _L( "CWimKeyStore::DoRelease()" ) );
    delete this;
    }

// -----------------------------------------------------------------------------
// CWimKeyStore::Token()
// Returns a reference to current token of this key store interface.
// -----------------------------------------------------------------------------
//
MCTToken& CWimKeyStore::Token()
    {
    _WIMTRACE ( _L( "CWimKeyStore::Token()" ) );
    return iToken;
    }

// -----------------------------------------------------------------------------
// CWimKeyStore::List()
// Lists all the keys in the store that match the filter.
// -----------------------------------------------------------------------------
//
void CWimKeyStore::List( RMPointerArray<CCTKeyInfo>& aKeys,
                         const TCTKeyAttributeFilter& aFilter,
                         TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimKeyStore::List()" ) );

    if ( !EnteringAllowed( aStatus ) )
        {
        return;
        }

    iKeyList = &aKeys;
    iKeyFilter = aFilter;
    iPhase = EGetKeyList;
    iPhaseOriginal = EGetKeyList;
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, KErrNone );
    SetActive();
    }


// -----------------------------------------------------------------------------
// CWimKeyStore::Open()
// Open an RSA key for signing
// -----------------------------------------------------------------------------
//
void CWimKeyStore::Open( const TCTTokenObjectHandle& aHandle,
                        MRSASigner*& aSigner,
                        TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimKeyStore::Open()" ) );

    if ( !EnteringAllowed( aStatus ) )
        {
        return;
        }

    //Check whether handle matches.
    if ( aHandle.iTokenHandle == Token().Handle() )
        {
        TRAPD( err, iSigner = CWimRSASigner::NewL( *this ) );
        if ( err ) //OOM
            {
            iPhase = EIdle;
            User::RequestComplete( iOriginalRequestStatus, err );
            }
        else
           {
            aSigner = iSigner;
            iOpenSigninKeyHandle = aHandle;
            iPhase = ECreateRSASignerObject;
            TRequestStatus* status = &iStatus;
            User::RequestComplete( status, KErrNone );
            SetActive();
           }
        }

    else
        {
        User::RequestComplete( iOriginalRequestStatus, KErrArgument );
        }
    }

// -----------------------------------------------------------------------------
// CWimKeyStore::CancelOpen()
// Cancels an ongoing Open operation
// -----------------------------------------------------------------------------
//
void CWimKeyStore::CancelOpen()
    {
    _WIMTRACE ( _L( "CWimKeyStore::CancelOpen()" ) );
    Cancel();
    }

// -----------------------------------------------------------------------------
// CWimKeyStore::ExportPublic()
// Returns the public key in DER-encoded ASN-1.
// -----------------------------------------------------------------------------
//
void CWimKeyStore::ExportPublic( const TCTTokenObjectHandle& aHandle,
                                 HBufC8*& aPublicKey,
                                 TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimKeyStore::ExportPublic()" ) );

    if ( !EnteringAllowed( aStatus ) )
        {
        return;
        }

    if ( aHandle.iTokenHandle == Token().Handle() )
        {
    
            TRAPD( err, iPublicKey = HBufC8::NewL( KPublicKeyLength ) );

            if ( err )
                {
                User::RequestComplete( iOriginalRequestStatus, err );
                }
            else
                {
                iExportKeyIdIndex = aHandle.iObjectId;
                aPublicKey = iPublicKey;
                iPhase = EStartExportPublicKey;
                TRequestStatus* status = &iStatus;
                User::RequestComplete( status, KErrNone );
                SetActive();
                }
         
           
        }
    else
        {
        User::RequestComplete( iOriginalRequestStatus, KErrBadHandle );
        }
    }

// -----------------------------------------------------------------------------
// CWimKeyStore::CancelExportPublic()
// Cancels an ongoing Export operation.
// -----------------------------------------------------------------------------
//
void CWimKeyStore::CancelExportPublic()
    {
    _WIMTRACE ( _L( "CWimKeyStore::CancelExportPublic()" ) );
    Cancel();
    }

// -----------------------------------------------------------------------------
// CWimKeyStore::Open()
//Open an DSA key for signing. Not Supported.
// -----------------------------------------------------------------------------
//
void CWimKeyStore::Open( const TCTTokenObjectHandle& /*aHandle*/,
                         MDSASigner*& /*aSigner*/,
                         TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimKeyStore::Open()" ) );
    TRequestStatus* stat = &aStatus;
    User::RequestComplete(stat, KErrNotSupported );
    }

// -----------------------------------------------------------------------------
// CWimKeyStore::Open()
// Open a RSA key for private decryption. Not Supported.
// -----------------------------------------------------------------------------
//
void CWimKeyStore::Open( const TCTTokenObjectHandle& /*aHandle*/,
                         MCTDecryptor*& /*aDecryptor*/,
                         TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimKeyStore::Open()" ) );
    TRequestStatus* stat = &aStatus;
    User::RequestComplete( stat, KErrNotSupported );
    }

// -----------------------------------------------------------------------------
// CWimKeyStore::Open()
// Open a DH key for key agreement. Not Supported.
// -----------------------------------------------------------------------------
//
void CWimKeyStore::Open( const TCTTokenObjectHandle& /*aHandle*/,
                         MCTDH*& /*aDH*/,
                         TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimKeyStore::Open()" ) );
    TRequestStatus* stat = &aStatus;
    User::RequestComplete( stat, KErrNotSupported );
    }

// -----------------------------------------------------------------------------
// void CWimKeyStore::RunL()
// Core operations are done here.
// -----------------------------------------------------------------------------
//
void CWimKeyStore::RunL()
    {
    _WIMTRACE ( _L( "CWimKeyStore::RunL()" ) );

    if ( !iListDone &&
         iPhase != EGetKeyList &&
         iPhase != ECheckGetKeyList )
        {
        iPhaseOriginal = iPhase;
        iPhase = EGetKeyList;
        TRequestStatus* status = &iStatus;
        User::RequestComplete( status, KErrNone );
        SetActive();
        }
    else
        {
        switch ( iPhase )
            {
        case EGetKeyList:
            {
            TInt count = iKeyInfos.Count();

            for ( TInt i = 0; i < count; i++ )
                {
                iKeyInfos[i]->Release();
                }

            iKeyInfos.Reset();
            delete iCWimKeyDetails;
            iCWimKeyDetails = NULL;
            iKeyNumbers->Reset();
            iCWimKeyDetails = CWimKeyDetails::NewL( Token() );
            iStatus = KRequestPending;
            iPhase = ECheckGetKeyList;
            iCWimKeyDetails->GetKeyList( iKeyInfos, *iKeyNumbers, iStatus );
            SetActive();
            break;
            }
        case ECheckGetKeyList:
            {
            if ( iStatus.Int() == KErrNone )
                {
                iListDone = ETrue;
                iKeyIndex = 0;
                iPhase = EFilterKeys;
                TRequestStatus* status = &iStatus;
                User::RequestComplete( status, KErrNone );
                SetActive();
                }
            else // No keys found or iWimKeyDetails->Restore call has
                 // been cancelled or failed
                {
                iPhase = EIdle;
                iPhaseOriginal = EIdle;
                User::RequestComplete( iOriginalRequestStatus, iStatus.Int() );
                }
            break;
            }
        case EFilterKeys:
            {
            DoFilterKeysL();
            break;
            }
        case EGetKey:
            {
            TInt count = iKeyInfos.Count();
            TInt i;

            for ( i = 0; i < count; i++ )
                {
                if ( iKeyInfos[i]->Handle().iObjectId == iHandle )
                    {
                    // Make a copy of key info
                    *iInfo = CreateKeyInfoL( i );
                    i = count + 1;
                    }
                }
            if ( i == count )
                {
                User::RequestComplete( iOriginalRequestStatus, KErrNotFound );
                }
            else
                {
                User::RequestComplete( iOriginalRequestStatus, KErrNone );
                }
            iPhase = EIdle;
            iPhaseOriginal = EIdle;
            break;  
            }
            
        case ECreateRSASignerObject:
            {
            TBool found = EFalse;
            TInt i;

            for ( i = 0; i<iKeyInfos.Count() && !found; i++ )
                {
                //Check if the handle matches.
                if ( iKeyInfos[i]->Handle().iObjectId ==
                    iOpenSigninKeyHandle.iObjectId )
                    {
                    found = ETrue;
                    }
                if ( found )
                    {
                    //Check whether found key can do RSA sign.
                    if ( iKeyInfos[i]->Algorithm() != CKeyInfoBase::ERSA )
                        {
                        //Set flag to false, the key does not do RSA sign.
                        found = EFalse;
                        }
                    else
                        {
                        //Set label for new signing key
                        iSigner->SetLabelL( iKeyInfos[i]->Label() );
                        //Set objectId
                        iSigner->SetObjectId( iKeyInfos[i]->Handle().iObjectId );
                        }
                    }
                }
            if ( found )
                {
                User::RequestComplete( iOriginalRequestStatus, KErrNone );
                }
            else
                {
                delete iSigner;
                iSigner = NULL;
                User::RequestComplete( iOriginalRequestStatus, KErrBadHandle );
                }
            iPhase = EIdle;
            iPhaseOriginal = EIdle;
            break;
            }
        case EStartNonRepudiableRSASign:
            {
            TBool found = EFalse;
            TInt i = 0;

            for ( ; i < iKeyInfos.Count() && !found; i++ )
                {
                 if ( iKeyInfos.operator[]( i )->Handle().iObjectId ==
                 iRSAKeyIdIndex )
                     {
                     found = ETrue;
                     TRAPD( err, iKeyId = iKeyInfos[i]->ID().AllocL() )

                     if ( err )
                         {
                         User::RequestComplete( iOriginalRequestStatus, err );
                         }
                     else
                         {
                         iCWimKeyDetails->Sign( *iDigest,
                                        *iKeyId,
                                        iSignature,
                                        iStatus );
                         iPhase = ESignCompleted;
                         SetActive();
                         }
                     }
                }
            //Key was not found, no need for signature
           if ( !found )
              {
              iPhase = EIdle;
              iPhaseOriginal = EIdle;
              delete iSignature;
              iSignature = NULL;
              User::RequestComplete( iOriginalRequestStatus, KErrBadHandle );
              }
        	break;
            }
        case ESignCompleted:
            {
            //No need for KeyId anymore
            delete iKeyId;
            iKeyId = NULL;

            //Check whether signing failed
            if ( iStatus.Int() != KErrNone )
                {
                delete iSignature;
                iSignature = NULL;
                User::RequestComplete( iOriginalRequestStatus, iStatus.Int() );
                }
            else //Sign was succesful.
                {
                 //Create signature and complete operation
                RInteger signature = RInteger::NewL( *iSignature );
                CleanupStack::PushL( signature );
                *iRSASignature = CRSASignature::NewL( signature );
                CleanupStack::Pop();
                delete iSignature;
                iSignature = NULL;
                User::RequestComplete( iOriginalRequestStatus, iStatus.Int() );
                }
            iPhase = EIdle;
            iPhaseOriginal = EIdle;
            break;
            }
        case EStartExportPublicKey:
            {
            TInt i = 0;
            TBool found = EFalse;
            
            for( i = 0; i < iKeyInfos.Count() && !found; i++ )
                {
            	//Check if the objectIds matches
            	if( iKeyInfos[i]->Handle().iObjectId == iExportKeyIdIndex )
            	    {
            		found = ETrue;
            	    }	
                }
               
            if( !found )
                {
            	iPhase = EIdle;
            	iPhaseOriginal = EIdle;
            	delete iPublicKey;
            	iPublicKey = NULL;
            	User::RequestComplete( iOriginalRequestStatus, KErrNotFound );
                }
            else
                {
            	iKeyId = iKeyInfos[iExportKeyIdIndex]->ID().AllocL();
                iCWimKeyDetails->ExportPublicKeyL( *iKeyId,
                                               iPublicKey,
                                               iStatus );

                iPhase = EExportPublicKeyDone;
                SetActive();
                }
            
            break;
            }
        case EExportPublicKeyDone:
            {
            delete iKeyId;
            iKeyId = NULL;
            iPhase = EIdle;
            iPhaseOriginal = EIdle;
            User::RequestComplete( iOriginalRequestStatus, iStatus.Int() );
            break;
            }
        default:
            {
            // Here we should not be
            iPhase = EIdle;
            iPhaseOriginal = EIdle;
            User::RequestComplete( iOriginalRequestStatus, KErrCorrupt );
            break;
            }
        }
        }
    }

// -----------------------------------------------------------------------------
// CWimKeyStore::NonRepudiableRSASign()
// Perform signing operation
// -----------------------------------------------------------------------------
//
void CWimKeyStore::NonRepudiableRSASign( const TCTTokenObjectHandle& aHandle,
                                         const TDesC8& aDigest,
                                         CRSASignature*& aSignature,
                                         TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimKeyStore::NonRepudiableRSASign()" ) );

    if ( !EnteringAllowed( aStatus ) )
        {
        return;
        }

    TRAPD( err, iSignature = HBufC8::NewL( KMaxSignatureLength ) );

    if ( err ) //OOM
        {
        User::RequestComplete( iOriginalRequestStatus, err );
        return;
        }

    iRSASignature = &aSignature;
    iRSAKeyIdIndex = aHandle.iObjectId;
    iDigest = &aDigest;

    iPhase = EStartNonRepudiableRSASign;
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, KErrNone );
    SetActive();
    }

// -----------------------------------------------------------------------------
// void CWimKeyStore::DoFilterKeysL()
// Core operations are done here.
// -----------------------------------------------------------------------------
//
void CWimKeyStore::DoFilterKeysL()
    {
    _WIMTRACE ( _L( "CWimKeyStore::DoFilterKeysL()" ) );
    if ( iPhaseOriginal == EGetKeyList )
        {
        if ( iKeyIndex < iKeyInfos.Count() )
            {
            // If usage is given in filter and handled key (in iKeyIndex) has
            // also same usage, take with
            //
            // OR, (if filter usage is 0 or EAllKeyUsages) AND handled key
            // usage is also 0 AND (key id length in filter is 0 OR handled
            // key id is equal to key id in filter), take with
            //
            if ( ( ( iKeyFilter.iUsage & iKeyInfos[iKeyIndex]->Usage() ) ||
               ( ( ( iKeyFilter.iUsage == 0 ) || ( iKeyFilter.iUsage ==
               EPKCS15UsageAll ) ) &&
               ( iKeyInfos[iKeyIndex]->Usage() == 0 ) ) ) &&
               ( iKeyFilter.iKeyId.Length() == 0 ||
                !iKeyInfos[iKeyIndex]->ID().Compare( iKeyFilter.iKeyId ) ) )
                {
                // Make a copy of key info
                CCTKeyInfo* copy = CreateKeyInfoL( iKeyIndex );
                // Call Release() for copy, if leave occurs
                CleanupReleasePushL( *copy );
                User::LeaveIfError( iKeyList->Append ( copy ) );
                CleanupStack::Pop( copy );
                }
            iKeyIndex++;
            TRequestStatus* status = &iStatus;
            User::RequestComplete( status, KErrNone );
            SetActive();
            }
        else
            {
            // All keys are listed or there are not any
            iPhase = EIdle;
            iPhaseOriginal = EIdle;
            User::RequestComplete( iOriginalRequestStatus, KErrNone );
            }
        }
    else
        {
        iPhase = iPhaseOriginal;
        iPhaseOriginal = EIdle;
        TRequestStatus* status = &iStatus;
        User::RequestComplete( status, KErrNone );
        SetActive();
        }
    }

// -----------------------------------------------------------------------------
// void CWimKeyStore::CreateKeyInfoL()
// Creates a new key info from internal array.
// -----------------------------------------------------------------------------
//
CCTKeyInfo* CWimKeyStore::CreateKeyInfoL( TInt aIndex )
    {
    _WIMTRACE ( _L( "CWimKeyStore::CreateKeyInfoL()" ) );
    // Create protector
    MCTAuthenticationObject* protector = NULL;
    TUint thishandle = iKeyNumbers->At ( aIndex );
    protector = MakeAuthObjectL( thishandle );
    
    if ( protector )
        {
        CleanupStack::PushL( protector );
        }

    HBufC* wlabel = HBufC::NewLC( ( iKeyInfos )[aIndex]->Label().Length() );
    wlabel->Des().Copy( ( iKeyInfos )[aIndex]->Label() );

#ifdef __SECURITY_PLATSEC_ARCH__
    CCTKeyInfo* copy = CCTKeyInfo::NewL( ( iKeyInfos ) [aIndex]->ID(),
                                         ( iKeyInfos ) [aIndex]->Usage(),
                                         ( iKeyInfos ) [aIndex]->Size(),
                                         protector,
                                         wlabel,
                                         ( iKeyInfos ) [aIndex]->Token(),
                                         ( iKeyInfos ) [aIndex]->Handle().iObjectId,
                                         ( iKeyInfos ) [aIndex]->UsePolicy(),
                                         ( iKeyInfos ) [aIndex]->ManagementPolicy(),
                                         ( iKeyInfos ) [aIndex]->Algorithm(),
                                         ( iKeyInfos ) [aIndex]->AccessType(),
                                         ( iKeyInfos ) [aIndex]->Native(),
                                         ( iKeyInfos ) [aIndex]->StartDate(),
                                         ( iKeyInfos ) [aIndex]->EndDate() );
#else
   CCTKeyInfo* copy = CCTKeyInfo::NewL( ( iKeyInfos ) [aIndex]->ID(),
                                         ( iKeyInfos ) [aIndex]->Usage(),
                                         ( iKeyInfos ) [aIndex]->Size(),
                                         protector,
                                         wlabel,
                                         ( iKeyInfos ) [aIndex]->Token(),
                                         ( iKeyInfos ) [aIndex]->Handle().iObjectId,
                                         ( iKeyInfos ) [aIndex]->Owner(),
                                         ( iKeyInfos ) [aIndex]->Users(),
                                         ( iKeyInfos ) [aIndex]->Algorithm(),
                                         ( iKeyInfos ) [aIndex]->AccessType(),
                                         ( iKeyInfos ) [aIndex]->Native(),
                                         ( iKeyInfos ) [aIndex]->StartDate(),
                                         ( iKeyInfos ) [aIndex]->EndDate() );
#endif

    CleanupStack::Pop( wlabel );
    if ( protector )
        {
        CleanupStack::Pop( protector );
        }
    return copy;
    }

// -----------------------------------------------------------------------------
// void CWimKeyStore::MakeAuthObjectL()
// Lists authentication objects and matches each on given handle.
// When match is found, one instance is created and returned.
// -----------------------------------------------------------------------------
//
MCTAuthenticationObject* CWimKeyStore::MakeAuthObjectL( TInt aHandle )
    {
    
    if( !iPinNRs )
	    {
	    TInt ret = iToken.WimSecModule()->PinNrEntriesL( iPinNRs );
	   
	    if ( ret != KErrNone )
	        {
	        User::Leave( ret );
	        }	
	    }
    
        
    if ( iPinNRs ) // If PINs were found
        {
        TUid pintype = {WIM_PIN_G_UID}; // The first one is always PIN-G
        TInt count = iPinNRs->Count();
        TUint32 status = 0;

        for ( TInt i = 0; i < count; i++ )
            {
            if ( aHandle == (*iPinNRs)[i]->PinNumber() )
                {
                CWimAuthenticationObject* me =
                        CWimAuthenticationObject::NewLC(
                                            iToken,
                                         *( *iPinNRs )[i],
                                          ( *iPinNRs )[i]->Label(),
                                             pintype,
                                          ( *iPinNRs )[i]->PinNumber(),
                                             status );
                 CleanupStack::Pop( me );
                 return me;
                }
            pintype.iUid = WIM_PIN_NR_UID; // The others are always PIN-NRs
            }
        }
    return NULL;    
    }
// -----------------------------------------------------------------------------
// void CWimKeyStore::RunError()
// Unexpected error in RunL (e.g. Leave) leads us here.
// -----------------------------------------------------------------------------
//
TInt CWimKeyStore::RunError( TInt aError )
    {
    _WIMTRACE ( _L( "CWimKeyStore::RunError()" ) );

    iPhase = EIdle;
    iPhaseOriginal = EIdle;
    User::RequestComplete( iOriginalRequestStatus, aError );
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// void CWimKeyStore::DoCancel()
// Cancels outgoing async. calls.
// -----------------------------------------------------------------------------
//
void CWimKeyStore::DoCancel()
    {
    _WIMTRACE ( _L( "CWimKeyStore::DoCancel()" ) );

    if ( iCWimKeyDetails && iPhase == ECheckGetKeyList )
        {
        if ( iCWimKeyDetails->IsActive() )
            {
            iCWimKeyDetails->CancelList();
            }
        }
    else if ( iCWimKeyDetails && iPhase == ESignCompleted )
        {
        if ( iCWimKeyDetails->IsActive() )
            {
            iCWimKeyDetails->CancelSign();
            delete iSignature;
            iSignature = NULL;
            }
        }
    else if ( iCWimKeyDetails && iPhase == EExportPublicKeyDone )
        {
        if ( iCWimKeyDetails->IsActive() )
            {
            iCWimKeyDetails->CancelExport();
            }
        }
    iPhase = EIdle;
    iPhaseOriginal = EIdle;
    delete iKeyId;
    iKeyId = NULL;
    User::RequestComplete( iOriginalRequestStatus, KErrCancel );
    }

// -----------------------------------------------------------------------------
// CWimKeyStore::CancelList()
// Cancels an ongoing List operation. Sets an internal flag to true.
// -----------------------------------------------------------------------------
//
void CWimKeyStore::CancelList()
    {
    _WIMTRACE ( _L( "CWimKeyStore::CancelList()" ) );

    if ( TokenRemoved() )
        {
        return;
        }

    Cancel();
    }

// -----------------------------------------------------------------------------
// CWimKeyStore::GetKeyInfo()
// Retrieves a key according to given handle.
// -----------------------------------------------------------------------------
//
void CWimKeyStore::GetKeyInfo( TCTTokenObjectHandle aHandle,
                               CCTKeyInfo*& aInfo,
                               TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimKeyStore::GetKeyInfo()" ) );

    if ( !EnteringAllowed( aStatus ) )
        {
        return;
        }

    if ( aHandle.iTokenHandle == Token().Handle() )
        {
        iInfo = &aInfo;
        iHandle = aHandle.iObjectId;
        iPhase = EGetKey;
        TRequestStatus* status = &iStatus;
        User::RequestComplete( status, KErrNone );
        SetActive();
        }
    else
        {
        User::RequestComplete( iOriginalRequestStatus, KErrArgument );
        }
    }

// -----------------------------------------------------------------------------
// CWimKeyStore::CancelGetKeyInfo()
// Cancels an ongoing GetKeyInfo operation.
// -----------------------------------------------------------------------------
//
void CWimKeyStore::CancelGetKeyInfo()
    {
    _WIMTRACE ( _L( "CWimKeyStore::CancelGetKeyInfo()" ) );

    if ( TokenRemoved() )
        {
        return;
        }

    Cancel();
    }

// -----------------------------------------------------------------------------
// CWimKeyStore::EnteringAllowed()
//
// -----------------------------------------------------------------------------
//
TBool CWimKeyStore::EnteringAllowed( TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimKeyStore::EnteringAllowed()" ) );
    if ( TokenRemoved() )
        {
        TRequestStatus* status = &aStatus;
        User::RequestComplete( status, KErrHardwareNotAvailable );
        return EFalse;
        }

    // If this active object is in running, don't accept entering
    if ( IsActive() )
        {
        // If the caller status is the same as the status, that activated
        // this object, just return
        if ( &aStatus == iOriginalRequestStatus )
            {
            return EFalse;
            }
        else
            {
            // Otherwise complete it with error
            TRequestStatus* status = &aStatus;
            User::RequestComplete( status, KErrInUse );
            return EFalse;
            }
        }
    else
        {
        iOriginalRequestStatus = &aStatus;
        aStatus = KRequestPending;
        return ETrue;
        }
    }

// -----------------------------------------------------------------------------
// CWimKeyStore::TokenRemoved()
// Returns true or false indicating if token is removed
// -----------------------------------------------------------------------------
//
TBool CWimKeyStore::TokenRemoved()
    {
    _WIMTRACE ( _L( "CWimKeyStore::TokenRemoved()" ) );
    // If token listener is not alive, then token is removed
    if ( iToken.TokenListener()->TokenStatus() != KRequestPending )
        {
        return ETrue;
        }
    else
        {
        return EFalse;
        }
    }

