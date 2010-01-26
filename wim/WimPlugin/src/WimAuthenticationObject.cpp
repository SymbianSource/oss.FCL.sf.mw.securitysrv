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
* Description:  Implementation of single authentication object
*
*/


// INCLUDE FILES

#include "WimSecModuleMgr.h"
#include "WimAuthenticationObject.h"
#include "WimPin.h"
#include "WimTrace.h"
//#include "WimDebug.h"
#include "WimTokenListener.h"
#include "WimImplementationUID.hrh"
#include <unifiedkeystore.h>

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWimAuthenticationObject::CWimAuthenticationObject()
// Defalt constructor
// -----------------------------------------------------------------------------
//
CWimAuthenticationObject::CWimAuthenticationObject(
                                               CWimToken& aToken,
                                               CWimPin& aWimPin,
                                               const TUid aType,
                                               const TInt aHandle,
                                               TUint32 aObjectStatus ) :

                                               CActive( EPriorityNormal ),
                                               MCTAuthenticationObject( aToken ),
                                               iToken( aToken ),
                                               iWimPin ( aWimPin ),
                                               iType ( aType ),
                                               iObjectId ( aHandle ),
                                               iObjectStatus( aObjectStatus ),
                                               iTokenWider( aToken )

    {
    CActiveScheduler::Add( this );
    iPhase = EIdle;
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObject::NewLC()
// Two-phased constructor. One object stays in cleanup stack.
// -----------------------------------------------------------------------------
//
CWimAuthenticationObject* CWimAuthenticationObject::NewLC(
                                                         CWimToken& aToken,
                                                         CWimPin& aWimPin,
                                                         const TDesC& aLabel,
                                                         const TUid aType,
                                                         const TInt aHandle,
                                                         TUint32 aObjectStatus )
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::NewLC()" ) );
    __ASSERT_ALWAYS( aLabel.Length() > 0, User::Leave( KErrArgument ) );

    if ( aType.iUid != WIM_PIN_G_UID && aType.iUid != WIM_PIN_NR_UID )
        {
        User::Leave( KErrArgument );
        }

    CWimAuthenticationObject* that =
        new( ELeave ) CWimAuthenticationObject( aToken,
                                                 aWimPin,
                                                 aType,
                                                 aHandle,
                                                 aObjectStatus );
    CleanupStack::PushL( that );
    that->ConstructL( aLabel );
    return that;
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObject::ConstructL()
// Second phase constructor.
// -----------------------------------------------------------------------------
//
void CWimAuthenticationObject::ConstructL( const TDesC& aLabel )
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::ConstructL()" ) );
    iLabel = aLabel.AllocL();
    iPhase = EIdle;
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObject::~CWimAuthenticationObject()
// Destructor
// -----------------------------------------------------------------------------
//
CWimAuthenticationObject::~CWimAuthenticationObject()
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::~CWimAuthenticationObject()" ) );
    Cancel();
    FreeUnifiedKeyStore();
    FreeUnifiedCertStore();
    delete iCertFilter;
    delete iLabel;
    }

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWimAuthenticationObject::Label()
// Returns a reference to this authentication object's label.
// -----------------------------------------------------------------------------
//
const TDesC& CWimAuthenticationObject::Label() const
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::Label()" ) );

    return *iLabel;
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObject::Token()
// Returns a reference to token (MCTToken) of this authentication object
// -----------------------------------------------------------------------------
//
MCTToken& CWimAuthenticationObject::Token() const
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::Token()" ) );
    return iToken;
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObject::TokenWider()
// Returns a reference to token (CWimToken) of this authentication object
// -----------------------------------------------------------------------------
//
CWimToken& CWimAuthenticationObject::TokenWider() const
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::TokenWider()" ) );
    return iTokenWider;
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObject::Type()
// Returns type (TUid) of current authentication object.
// WIM_PIN_G_UID or WIM_PIN_NR_UID
// -----------------------------------------------------------------------------
//
TUid CWimAuthenticationObject::Type() const
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::Type()" ) );
    return iType;
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObject::Release()
// Deletes this instance
// -----------------------------------------------------------------------------
//
void CWimAuthenticationObject::DoRelease()
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::DoRelease()" ) );
    delete this;
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObject::Handle()
// Returns a handle (TCTTokenObjectHandle) of this authentication object.
// iObjectId runs from 0 to N where N is (count of authentication objects) - 1
// -----------------------------------------------------------------------------
//
TCTTokenObjectHandle CWimAuthenticationObject::Handle() const
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::Handle()" ) );
    return TCTTokenObjectHandle( Token().Handle(), iObjectId );
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObject::PinModule()
// Returns a reference to pin module manager (CWimPin)
// -----------------------------------------------------------------------------
//
CWimPin& CWimAuthenticationObject::PinModule() const
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::PinModule()" ) );
    return iWimPin;
    }
// -----------------------------------------------------------------------------
// CWimAuthenticationObject::ListProtectedObjects()
// Returns a list of all the objects which this authentication object protects.
// Procedure: 1. Fetch all keys this authentication object protects
//            2. Fetch certificates those keys are used for and
//            3. Make objects from those protected keys and certificates
// -----------------------------------------------------------------------------
//
void CWimAuthenticationObject::
     ListProtectedObjects( RMPointerArray<MCTTokenObject>& aObjects,
                           TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::ListProtectedObjects()" ) );

    if ( !EnteringAllowed( aStatus ) )
        {
        return;
        }

    iObjects = &aObjects;

    iKeyInfos.Close();

    iFs = static_cast<CCTTokenType&>( Token().TokenType() ).Fs();

    TRAPD( err, iUnifiedKeyStore = CUnifiedKeyStore::NewL( iFs ) );

    if ( err != KErrNone )
        {
        User::RequestComplete( iOriginalRequestStatus, err );
        }
    else
        {
        iStatus = KRequestPending;
        iUnifiedKeyStore->Initialize( iStatus );
        iPhase = EGetKeyInfos;
        SetActive();
        }
    }

// -----------------------------------------------------------------------------
// void CWimAuthenticationObject::RunL()
// Core operations are done here.
// -----------------------------------------------------------------------------
//
void CWimAuthenticationObject::RunL()
    {
    switch ( iPhase )
        {
        case EGetKeyInfos:
            {
            if ( iStatus.Int() == KErrNone )
                {
                iKeyFilter.iKeyId = KNullDesC8;
                iKeyFilter.iUsage = EPKCS15UsageAll;
                iStatus = KRequestPending;
                iPhase = EFilterKeys;
                iStatus = KRequestPending;
                iUnifiedKeyStore->List( iKeyInfos, iKeyFilter, iStatus );
                SetActive();
                }
            else // Something went wrong (or call was cancelled) with
                 // iUnifiedKeyStore->Initialize( iStatus )
                {
                FreeUnifiedKeyStore();
                iPhase = EIdle;
                User::RequestComplete( iOriginalRequestStatus, iStatus.Int() );
                }
            break;
            }
        case EFilterKeys:
            {
            DoFilterKeys();
            break;
            }
        case EGetCerts:
            {
            // In this phase cert store must allways be NULL
            if ( !iUnifiedCertStore )
                {
                iFs = static_cast<CCTTokenType&>( Token().TokenType() ).Fs();
                iUnifiedCertStore = CUnifiedCertStore::NewL( iFs, EFalse );
                iStatus = KRequestPending;
                iPhase = EInitCertStore;
                iUnifiedCertStore->Initialize( iStatus );
                SetActive();
                }
            else
                {
                FreeUnifiedKeyStore();
                iPhase = EIdle;
                User::RequestComplete( iOriginalRequestStatus, KErrCorrupt );
                }
            break;
            }
        case EInitCertStore:
            {
            // Called when cert store init completes
            if ( iStatus.Int() == KErrNone )
                {
                if ( iCertFilter )
                    {
                    delete iCertFilter;
                    iCertFilter = NULL;
                    }
                iCertFilter = CCertAttributeFilter::NewL();
                iStatus = KRequestPending;
                iPhase = EListCertEntries;
                iUnifiedCertStore->List( iCertStoreEntries,
                                        *iCertFilter,
                                         iStatus );
                SetActive();
                }
            else // Something went wrong (or call was cancelled) with
                 // iUnifiedCertStore->Initialize( iStatus )
                {
                FreeUnifiedKeyStore();
                FreeUnifiedCertStore();
                iPhase = EIdle;
                User::RequestComplete( iOriginalRequestStatus, iStatus.Int() );
                }
            break;
            }
        case EListCertEntries:
            {
            DoListCertEntries();
            break;
            }
        case EMakeObjects:
            {
            DoMakeObjectsL();
            break;
            }
        case EEnablePinQuery:
            {
            User::RequestComplete( iOriginalRequestStatus, iStatus.Int() );
            break;
            }
        case EDisablePinQuery:
            {
            User::RequestComplete( iOriginalRequestStatus, iStatus.Int() );
            break;
            }
        case EChangePin:
            {
            User::RequestComplete( iOriginalRequestStatus, iStatus.Int() );
            break;
            }
        case EUnblockPin:
            {
            User::RequestComplete( iOriginalRequestStatus, iStatus.Int() );
            break;
            }
        case EVerifyPin:
            {
            User::RequestComplete( iOriginalRequestStatus, iStatus.Int() );
            break;
            }
        default:
            {
            User::RequestComplete( iOriginalRequestStatus, KErrCorrupt );
            break;
            }
        }
    }

// -----------------------------------------------------------------------------
// void CWimAuthenticationObject::DoFilterKeys()
// Select keys according to given filter
// -----------------------------------------------------------------------------
//
void CWimAuthenticationObject::DoFilterKeys()
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::DoFilterKeys()" ) );
    if ( iStatus.Int() == KErrNone )
        {
        // From all keys get only those that match with
        // this protector
        TInt count = iKeyInfos.Count();
        TInt i = 0;
        for ( i = 0; i < count ; i++ )
            {
            // If protector is not set or protector is not this, delete item
            if ( !iKeyInfos[i]->Protector() ||
                ( iKeyInfos[i]->Protector()->Handle().iObjectId !=
                                       this->Handle().iObjectId ) )
                {
                    // If match is not found, delete object
                    CCTKeyInfo* keyinfo = ( iKeyInfos ) [i];
                    keyinfo->Release();
                    ( iKeyInfos ) [i] = NULL;
                }
            }
        i = 0;
        while ( count )
            {
            if ( !iKeyInfos[i] )
                {
                // Remove element placeholder from CWimCertStore array
                iKeyInfos.Remove( i );
                }
            else
                {
                i++;
                }
            count--;
            }
        iKeyInfos.Compress();
        count = iKeyInfos.Count();
        if ( count ) // Continue handling
            {
            iPhase = EGetCerts;
            TRequestStatus* status = &iStatus;
            User::RequestComplete( status, KErrNone );
            SetActive();
            }
        else // No keys, no continue
            {
            FreeUnifiedKeyStore();
            iPhase = EIdle;
            User::RequestComplete( iOriginalRequestStatus, iStatus.Int() );
            }
        }
    else // Something went wrong (or call was cancelled) with
         // iUnifiedKeyStore->List(...)
        {
        FreeUnifiedKeyStore();
        iPhase = EIdle;
        User::RequestComplete( iOriginalRequestStatus, iStatus.Int() );
        }
    }

// -----------------------------------------------------------------------------
// void CWimAuthenticationObject::DoListCertEntries()
// Select certificates according to keys
// -----------------------------------------------------------------------------
//
void CWimAuthenticationObject::DoListCertEntries()
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::DoListCertEntries()" ) );
    if ( iStatus.Int() == KErrNone )
        {
         // All certs are listed
         // Match every cert to keys. If match not found, delete cert
         TInt certcount = iCertStoreEntries.Count();
         TInt i = 0;
         for ( i = 0; i < certcount; i++ )
             {
             TInt keycount = iKeyInfos.Count();
             TInt j = 0;
             for ( j = 0; j < keycount; j++ )
                 {
                 if ( iKeyInfos[j]->ID() ==
                                iCertStoreEntries[i]->SubjectKeyId() )
                    {
                    j = keycount + 1;
                    }
                 }
             if ( j == keycount )
                {
                // If match is not found, delete object and remove item
                CCTCertInfo* certinfo = ( iCertStoreEntries ) [i];
                // Delete object self
                certinfo->Release();
                ( iCertStoreEntries ) [i] = NULL;
                }
            }
        i = 0;
        while ( certcount )
            {
            if ( !iCertStoreEntries[i] )
                {
                // Remove element placeholder from array
                iCertStoreEntries.Remove( i );
                }
            else
                {
                i++;
                }
            certcount--;
            }
        iCertStoreEntries.Compress();
        iPhase = EMakeObjects;
        TRequestStatus* status = &iStatus;
        User::RequestComplete( status, KErrNone );
        SetActive();
        }
    else // Something went wrong (or call was cancelled) with
         // iUnifiedCertStore->List(...)
        {
        FreeUnifiedKeyStore();
        FreeUnifiedCertStore();
        iPhase = EIdle;
        User::RequestComplete( iOriginalRequestStatus, iStatus.Int() );
        }
    }

// -----------------------------------------------------------------------------
// void CWimAuthenticationObject::DoMakeObjectsL()
// Protected objects are validated here
// -----------------------------------------------------------------------------
//
void CWimAuthenticationObject::DoMakeObjectsL()
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::DoMakeObjectsL()" ) );
    if ( iStatus.Int() == KErrNone ) // Not cancelled
        {
        TInt count = iKeyInfos.Count();
        TInt i = 0;
        for ( ; i < count; i++ )
            {
            // Make a copy of key info
            HBufC* wlabel = HBufC::NewLC( ( iKeyInfos )[i]->Label().Length() );
            wlabel->Des().Copy( ( iKeyInfos )[i]->Label() );
            // Make a copy of protector if there is
            MCTAuthenticationObject* protector = NULL;
            if ( iKeyInfos[i]->Protector() )
                {
                TUint thishandle = iKeyInfos[i]->Protector()->
                                                    Handle().iObjectId;
                
                //All the key list here is protected by this Authobj
                if( this->Handle().iObjectId == thishandle )
	                {
	                protector = MakeAuthObjectL();
	                }
                                                    
                else
                    {
                	return;
                    }
                
                
                if ( protector )
                    {
                    CleanupStack::PushL( protector );
                    }
                }
#ifdef __SECURITY_PLATSEC_ARCH__
            CCTKeyInfo* copy =
                CCTKeyInfo::NewL( ( iKeyInfos ) [i]->ID(),
                                  ( iKeyInfos ) [i]->Usage(),
                                  ( iKeyInfos ) [i]->Size(),
                                    protector,
                                    wlabel,
                                  ( iKeyInfos ) [i]->Token(),
                                  ( iKeyInfos ) [i]->Handle().iObjectId,
                                  ( iKeyInfos ) [i]->UsePolicy(),
                                  ( iKeyInfos ) [i]->ManagementPolicy(),
                                  ( iKeyInfos ) [i]->Algorithm(),
                                  ( iKeyInfos ) [i]->AccessType(),
                                  ( iKeyInfos ) [i]->Native(),
                                  ( iKeyInfos ) [i]->StartDate(),
                                  ( iKeyInfos ) [i]->EndDate() );
#else
            CCTKeyInfo* copy =
                CCTKeyInfo::NewL( ( iKeyInfos ) [i]->ID(),
                                  ( iKeyInfos ) [i]->Usage(),
                                  ( iKeyInfos ) [i]->Size(),
                                    protector,
                                    wlabel,
                                  ( iKeyInfos ) [i]->Token(),
                                  ( iKeyInfos ) [i]->Handle().iObjectId,
                                  ( iKeyInfos ) [i]->Owner(),
                                  ( iKeyInfos ) [i]->Users(),
                                  ( iKeyInfos ) [i]->Algorithm(),
                                  ( iKeyInfos ) [i]->AccessType(),
                                  ( iKeyInfos ) [i]->Native(),
                                  ( iKeyInfos ) [i]->StartDate(),
                                  ( iKeyInfos ) [i]->EndDate() );
#endif
            // Call Release() for copy, if leave occurs
            CleanupReleasePushL( *copy );
            // Take MCTokenObject part of it and append it to client
            MCTTokenObject* obj = copy;
            User::LeaveIfError( iObjects->Append ( obj ) );
            CleanupStack::Pop( copy );
            if ( protector )
                {
                CleanupStack::Pop( protector );
                }
            CleanupStack::Pop( wlabel );
            }
        count = iCertStoreEntries.Count();
        for ( i = 0; i < count; i++ )
            {
            // Make a copy of certificate info

            CCTCertInfo* copy = CCTCertInfo::NewLC( *iCertStoreEntries [i] );
            MCTTokenObject* obj = copy;
            User::LeaveIfError( iObjects->Append ( obj ) );
            CleanupStack::Pop( copy );
            }
        }
    FreeUnifiedKeyStore();
    FreeUnifiedCertStore();
    iPhase = EIdle;
    User::RequestComplete( iOriginalRequestStatus, iStatus.Int() );
    }

// -----------------------------------------------------------------------------
// void CWimAuthenticationObject::MakeAuthObjectL()
// Leavable function for making authenticaton objects
// -----------------------------------------------------------------------------
//
MCTAuthenticationObject*
                       CWimAuthenticationObject::MakeAuthObjectL()
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::MakeAuthObjectL()" ) );
    
     CWimAuthenticationObject* me = 
          CWimAuthenticationObject::NewLC( this->TokenWider(),
                                           this->PinModule(),
                                           this->Label(),
                                           this->Type(),
                                           this->Handle().iObjectId,
                                           this->Status() );
      CleanupStack::Pop( me );
      return me;                                     
    }

// -----------------------------------------------------------------------------
// void CWimAuthenticationObject::FreeUnifiedKeyStore()
// Frees key storage resources.
// -----------------------------------------------------------------------------
//
void CWimAuthenticationObject::FreeUnifiedKeyStore()
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::FreeUnifiedKeyStore()" ) );
    if ( iUnifiedKeyStore )
        {
        iKeyInfos.Close();
        delete iUnifiedKeyStore;
        iUnifiedKeyStore = 0;
        }
    iPhase = EIdle;
    }

// -----------------------------------------------------------------------------
// void CWimAuthenticationObject::FreeUnifiedCertStore()
// Frees certificate storage resources.
// -----------------------------------------------------------------------------
//
void CWimAuthenticationObject::FreeUnifiedCertStore()
    {
    if ( iUnifiedCertStore )
        {
        iCertStoreEntries.Close();
        delete iUnifiedCertStore;
        iUnifiedCertStore = NULL;
        }
    }
// -----------------------------------------------------------------------------
// void CWimAuthenticationObject::RunError()
// Unexpected error in RunL (e.g. Leave) leads us here.
// -----------------------------------------------------------------------------
//
TInt CWimAuthenticationObject::RunError( TInt aError )
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::RunError()" ) );
    FreeUnifiedKeyStore();
    FreeUnifiedCertStore();
    iPhase = EIdle;
    User::RequestComplete( iOriginalRequestStatus, aError );
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// void CWimAuthenticationObject::DoCancel()
//
// -----------------------------------------------------------------------------
//
void CWimAuthenticationObject::DoCancel()
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::DoCancel()" ) );

    if ( iUnifiedKeyStore )
        {
        switch ( iPhase )
            {
            case EGetKeyInfos:
                {
                if ( iUnifiedKeyStore->IsActive() )
                    {
                    iUnifiedKeyStore->CancelInitialize();
                    }
                break;
                }
            case EFilterKeys:
                {
                if ( iUnifiedKeyStore->IsActive() )
                    {
                    iUnifiedKeyStore->CancelList();
                    }
                break;
                }
            default:
                {
                 break;
                }
            }
        }

    if ( iUnifiedCertStore )
        {
        switch ( iPhase )
            {
            case EInitCertStore:
                {
                if ( iUnifiedCertStore->IsActive() )
                    {
                    iUnifiedCertStore->CancelInitialize();
                    }
                break;
                }
            case EListCertEntries:
                {
                if ( iUnifiedCertStore->IsActive() )
                    {
                    iUnifiedCertStore->CancelList();
                    }
                break;
                }
            default:
                {
                // Other phases won't cause any action
                break;
                }
            }
        }
    
    switch ( iPhase )
        {
        case EEnablePinQuery:
        	{
        	PinModule().CancelEnablePinQuery();	
        	break;
        	}
        case EDisablePinQuery:
            {
            PinModule().CancelDisablePinQuery();
            break;	
            }
        case EChangePin:
            {
        	PinModule().CancelChangePin();
        	break;
            }
        case EUnblockPin:
            {
         	PinModule().CancelUnblockPin();
         	break;
            }
        case EVerifyPin:
            {
            PinModule().CancelVerifyPin();
            break;
            }
        default: break;   
        }
    
    
    FreeUnifiedKeyStore();
    FreeUnifiedCertStore();
    iPhase = EIdle;
    User::RequestComplete( iOriginalRequestStatus, KErrCancel );
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObject::CancelListProtectedObjects()
// Cancels an ongoing ListProtectedObjects operation.
// -----------------------------------------------------------------------------
//
void CWimAuthenticationObject::CancelListProtectedObjects()
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::CancelListProtectedObjects()" ) );

    if ( TokenRemoved() )
        {
        return;
        }

    Cancel();
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObject::ChangeReferenceData()
// Change the reference data (e.g. PIN value).
// -----------------------------------------------------------------------------
//
void CWimAuthenticationObject::ChangeReferenceData( TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::ChangeReferenceData()" ) );

    if ( !EnteringAllowed( aStatus ) )
        {
        return;
        }

    PinModule().ChangePin( iStatus );
    iPhase = EChangePin;
    SetActive();
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObject::CancelListProtectedObjects()
// Cancel an ongoing ChangeReferenceData operation.
// -----------------------------------------------------------------------------
//
void CWimAuthenticationObject::CancelChangeReferenceData()
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::CancelChangeReferenceData()" ) );

    if ( TokenRemoved() )
        {
        return;
        }

    Cancel();
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObject::Unblock()
// Unblock the authentication object.
// -----------------------------------------------------------------------------
//
void CWimAuthenticationObject::Unblock( TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::Unblock()" ) );

    if ( !EnteringAllowed( aStatus ) )
        {
        return;
        }

    PinModule().UnblockPin( iStatus );
    iPhase = EUnblockPin;
    SetActive();
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObject::CancelUnblock()
// Cancel an ongoing Unblock operation.
// -----------------------------------------------------------------------------
//
void CWimAuthenticationObject::CancelUnblock()
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::CancelUnblock()" ) );

    if ( TokenRemoved() )
        {
        return;
        }

    Cancel();
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObject::Status()
// Get the status of the authentication object.
// Values are as in enum TCTAuthenticationStatus. Note: Blocked information
// is not available before authentication object is verified.
// -----------------------------------------------------------------------------
//
TUint32 CWimAuthenticationObject::Status() const
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::Status()" ) );

    if ( iToken.TokenListener()->TokenStatus() != KRequestPending )
        {
        return 0;
        }

    return PinModule().PinStatus();
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObject::Disable()
// Disables the authentication object. This means that authentication data is
// is not asked for any operations. Note: Disabling requires authentication data
// to be entered.
// -----------------------------------------------------------------------------
//
void CWimAuthenticationObject::Disable( TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::Disable()" ) );


    if ( !EnteringAllowed( aStatus ) )
        {
        return;
        }

    PinModule().DisablePinQuery( iStatus );
    iPhase = EDisablePinQuery;
    SetActive();
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObject::CancelDisable()
// Cancel an ongoing Disable operation.
// -----------------------------------------------------------------------------
//
void CWimAuthenticationObject::CancelDisable()
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::CancelDisable()" ) );

    if ( TokenRemoved() )
        {
        return;
        }
    
    Cancel();	
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObject::Enable()
// Enables the authentication object. This means that authentication data is
// is asked for certain operations. Note: Enabling requires authentication data
// to be entered.
// -----------------------------------------------------------------------------
//
void CWimAuthenticationObject::Enable( TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::Enable()" ) );

    if ( !EnteringAllowed( aStatus ) )
        {
        return;
        }

    PinModule().EnablePinQuery( iStatus );
    iPhase = EEnablePinQuery;
    SetActive();
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObject::CancelEnable()
// Cancel an ongoing Enable operation.
// -----------------------------------------------------------------------------
//
void CWimAuthenticationObject::CancelEnable()
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::CancelEnable()" ) );

    if ( TokenRemoved() )
        {
        return;
        }

    Cancel();
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObject::Open()
// Opens the authentication object, meaning that the protected objects can be
// accessed without provoking the authentication mechanism for the duration of
// the timeout period. Note. It is not strictly necessary to call this function,
// as the authentication object will be opened when any operation that needs it
// to be opened is called, but it is sometimes useful to control the timing of
// authentication dialogs. Note also that this function will do nothing if the
// authentication object is open, or if it authentication object requires
// the authentication
// -----------------------------------------------------------------------------
//
void CWimAuthenticationObject::Open( TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::Open()" ) );

    if ( !EnteringAllowed( aStatus ) )
        {
        return;
        }

    if ( this->Type().iUid == WIM_PIN_G_UID ) // If this is a Pin-G
        {
        TBool wimopen = iToken.WimSecModule()->IsOpen();
        if ( !wimopen )
            {
            if( iToken.WimSecModule()->CloseAfter() != 0 )
                {
                PinModule().VerifyPin( iStatus );
                iPhase = EVerifyPin;
                SetActive();
                }
            else
                {
                User::RequestComplete( iOriginalRequestStatus, KErrNone );
                }
            
            }
        else
            {
            User::RequestComplete( iOriginalRequestStatus, KErrNone );
            }
        }
    else
        {
        User::RequestComplete( iOriginalRequestStatus, KErrNotSupported );
        }
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObject::Close()
// Closes an authentication object. After this call authentication data is
// needed again for certain operations.
// -----------------------------------------------------------------------------
//
void CWimAuthenticationObject::Close( TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::Close()" ) );
    TInt err = KErrNone;

    if ( !EnteringAllowed( aStatus ) )
        {
        return;
        }

    if ( this->Type().iUid == WIM_PIN_G_UID ) // If this is a Pin-G
        {
        TBool wimopen = iToken.WimSecModule()->IsOpen();
        if ( wimopen )
            {
            err = iToken.WimSecModule()->Close();
            }
        }
    else
        {
        err = KErrNotSupported;
        }
    User::RequestComplete( iOriginalRequestStatus, err );
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObject::TimeRemaining()
// Returns the amount of time in seconds that the authentication object
// will remain open for, or 0 if it is closed.
// -----------------------------------------------------------------------------
void CWimAuthenticationObject::TimeRemaining( TInt& aStime,
                                              TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::TimeRemaining()" ) );
    TInt ret = KErrNone;
    aStime = 0;

    if ( !EnteringAllowed( aStatus ) )
        {
        return;
        }

    if ( this->Type().iUid == WIM_PIN_G_UID ) // If this is a Pin-G
        {
        TBool wimopen = iToken.WimSecModule()->IsOpen();
        if ( wimopen )
            {
            aStime = iToken.WimSecModule()->CloseAfter();
            }
        }
    else
        {
        ret = KErrNotSupported;
        }
    User::RequestComplete( iOriginalRequestStatus, ret );
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObject::SetTimeout()
// Sets the time in seconds for this authentication object. Non-positive
// special values is 0, meaning always ask, and -1, meaning until it's
// explicitly closed.
// -----------------------------------------------------------------------------
//
void CWimAuthenticationObject::SetTimeout( TInt aTimeout,
                                           TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::SetTimeout()" ) );
    TInt ret = KErrNone;

    if ( !EnteringAllowed( aStatus ) )
        {
        return;
        }

    if ( this->Type().iUid == WIM_PIN_G_UID ) // If this is a Pin-G
        {
        if ( aTimeout >= -1 && aTimeout <= 2146 )
            {
            iToken.WimSecModule()->SetCloseAfter( aTimeout );
            }
        else
            {
            ret = KErrArgument;
            }
        }
    else
        {
        ret = KErrNotSupported;
        }
    User::RequestComplete( iOriginalRequestStatus, ret );
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObject::Timeout()
// Gets the current timeout value, in seconds. See SetTimeout for an explanation
// of the values.
// -----------------------------------------------------------------------------
//
void CWimAuthenticationObject::Timeout( TInt& aTime,
                                        TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::Timeout()" ) );
    TInt ret = KErrNone;

    if ( !EnteringAllowed( aStatus ) )
        {
        return;
        }

    aTime = 0;

    if ( this->Type().iUid == WIM_PIN_G_UID ) // If this is a Pin-G
        {
        aTime = iToken.WimSecModule()->CloseAfter();
        }
    else
        {
        ret = KErrNotSupported;
        }
    User::RequestComplete( iOriginalRequestStatus, ret );
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObject::EnteringAllowed()
//
// -----------------------------------------------------------------------------
//
TBool CWimAuthenticationObject::EnteringAllowed( TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::EnteringAllowed()" ) );
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
// CWimAuthenticationObject::TokenRemoved()
// Returns true or false indicating if token is removed
// -----------------------------------------------------------------------------
//
TBool CWimAuthenticationObject::TokenRemoved()
    {
    _WIMTRACE ( _L( "CWimAuthenticationObject::TokenRemoved()" ) );

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

//  End of File
