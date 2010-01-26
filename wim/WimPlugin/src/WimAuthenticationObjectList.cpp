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
* Description:  Implementation of authentication object list
*
*/


// INCLUDE FILES

#include "WimAuthenticationObjectList.h"
#include "WimAuthenticationObject.h"
#include "WimPin.h"
#include "WimClsv.h"
#include "WimTrace.h"
#include "WimTokenListener.h"
#include "WimImplementationUID.hrh"
#include <mctauthobject.h>

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWimAuthenticationObjectList::CWimAuthenticationObjectList()
// Default constructor
// -----------------------------------------------------------------------------
//
CWimAuthenticationObjectList::CWimAuthenticationObjectList( CWimToken& aToken ) : 
                                                            CActive( EPriorityHigh ),
                                                            iToken( aToken ) 
    {
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObjectList::NewL()
// Two phased constructor
// -----------------------------------------------------------------------------
//
CWimAuthenticationObjectList* 
                      CWimAuthenticationObjectList::NewL( CWimToken& aToken ) 
    {
    _WIMTRACE ( _L( "CWimAuthenticationObjectList::NewL()" ) );
    CWimAuthenticationObjectList* that = 
                        new( ELeave ) CWimAuthenticationObjectList( aToken );
    CleanupStack::PushL( that );
    that->ConstructL();
    CleanupStack::Pop( that );
    return that;
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObjectList::ConstructL()
// Second phase constructor.
// -----------------------------------------------------------------------------
//
void CWimAuthenticationObjectList::ConstructL()
    {
    _WIMTRACE ( _L( "CWimAuthenticationObjectList::ConstructL()" ) );
    CActiveScheduler::Add( this ); 
    }

// -----------------------------------------------------------------------------
// CWimKeyStore::DoRelease()
// Deletes this interface on demand.
// -----------------------------------------------------------------------------
//
void CWimAuthenticationObjectList::DoRelease()
    {
    _WIMTRACE ( _L( "CWimAuthenticationObjectList::DoRelease()" ) );
    delete this;
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObjectList::~CWimAuthenticationObjectList()
// Destructor
// -----------------------------------------------------------------------------
//
CWimAuthenticationObjectList::~CWimAuthenticationObjectList()
    {
    _WIMTRACE ( 
        _L( "CWimAuthenticationObjectList::~CWimAuthenticationObjectList()" ) );
    Cancel();
    TInt count = iAuthObjects.Count();

    while ( count )
        {
        CWimAuthenticationObject* entry = iAuthObjects[count - 1];
        entry->Release();
        iAuthObjects.Remove( count - 1 );
        count--;
        }

    iAuthObjects.Close();
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObjectList::Token()
// Returns a reference to current token (MCTToken) of this authentication 
// object list interface.
// -----------------------------------------------------------------------------
//
MCTToken& CWimAuthenticationObjectList::Token()
    {
    _WIMTRACE ( _L( "CWimAuthenticationObjectList::Token()" ) );
    return iToken;
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObjectList::List()
// Lists all authentication objects.
// -----------------------------------------------------------------------------
//
void CWimAuthenticationObjectList::List( 
                        RMPointerArray<MCTAuthenticationObject>& aAuthObjects, 
                        TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimAuthenticationObjectList::List()" ) );

    if ( TokenRemoved() )
        {
        return;
        }
    
    _WIMTRACE ( _L( "CWimAuthenticationObjectList::List() after TokenRemoved checking" ) );
    aStatus = KRequestPending;
    iPhase = ECreateList;
        
    iClientStatus = &aStatus;
    iClientAuthObjects = &aAuthObjects;
       
    iStatus = KRequestPending;
    SetActive();
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, KErrNone );
    
    _WIMTRACE ( _L( "CWimAuthenticationObjectList::List() End" ) );

    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObjectList::DoListL()
// Fetches first all authentication objects from Wim. After that makes a copy
// each of them for the client.
// -----------------------------------------------------------------------------
//
void CWimAuthenticationObjectList::DoListL( 
                        RMPointerArray<MCTAuthenticationObject>& aAuthObjects )
    {
    _WIMTRACE ( _L( "CWimAuthenticationObjectList::DoListL()" ) );
    
    TInt count = iAuthObjects.Count();

    for ( TInt i = 0; i < count; i++ )
        {
        
        //SecModUI will call Status() to get the PIN status later.
        //We donot need to send PIN status in the phase
        
        TUint32 status = 0;
        CWimAuthenticationObject* me = 
                          MakeAuthObjectL( iAuthObjects[i]->TokenWider(),
                                           iAuthObjects[i]->PinModule(),
                                           iAuthObjects[i]->Label(),
                                           iAuthObjects[i]->Type(),
                                           iAuthObjects[i]->Handle().iObjectId,
                                           status );
        CleanupStack::PushL( me );
        User::LeaveIfError( aAuthObjects.Append( me ) );
        CleanupStack::Pop( me );
        }
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObjectList::CreateListL()
// This function creates an internal array for authentication objects.
// -----------------------------------------------------------------------------
//
void CWimAuthenticationObjectList::CreateListL( TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimAuthenticationObjectList::CreateListL()" ) );

    // Delete previous ones
    TInt count = iAuthObjects.Count();

    while ( count )
        {
        CWimAuthenticationObject* entry = iAuthObjects[count - 1];
        entry->Release();
        iAuthObjects.Remove( count - 1 );
        count--;
        }
    
   TInt ret = iToken.WimSecModule()->PinNrEntriesL( iPinNRs, aStatus );
    
    if ( ret != KErrNone )
        {
        User::Leave( ret );
        }
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObjectList::MakeAuthObjectL()
// This function is called from two different places
// -----------------------------------------------------------------------------
//
CWimAuthenticationObject* 
     CWimAuthenticationObjectList::MakeAuthObjectL( CWimToken& aToken,
                                                    CWimPin& aPinModule,
                                                    const TDesC& aLabel,
                                                    TUid aType,
                                                    TUint32 aHandle,
                                                    TUint32 aStatus )
    {
    _WIMTRACE ( _L( "CWimAuthenticationObjectList::MakeAuthObjectL()" ) );
    CWimAuthenticationObject* me = 
                   CWimAuthenticationObject::NewLC( aToken,
                                                    aPinModule,
                                                    aLabel,
                                                    aType,
                                                    aHandle,
                                                    aStatus );
    CleanupStack::Pop( me );
    return me;
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObjectList::CancelList()
// Because List is synchronous, there is nothing to do here.
// -----------------------------------------------------------------------------
//
void CWimAuthenticationObjectList::CancelList()
    {
    _WIMTRACE ( _L( "CWimAuthenticationObjectList::CancelList()" ) );
    Cancel();
    }

// -----------------------------------------------------------------------------
// CWimAuthenticationObjectList::TokenRemoved()
// Returns true or false indicating if token is removed
// -----------------------------------------------------------------------------
//
TBool CWimAuthenticationObjectList::TokenRemoved()
    {
    _WIMTRACE ( _L( "CWimAuthenticationObjectList::TokenRemoved()" ) );
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

// -----------------------------------------------------------------------------
// CWimAuthenticationObjectList::RunL()
// -----------------------------------------------------------------------------
//
void CWimAuthenticationObjectList::RunL()
    {
    _WIMTRACE ( _L( "CWimAuthenticationObjectList::RunL()" ) );
    switch ( iPhase )
        {
        case ECreateList:
            {
            iPhase = ECreateListDone;
            iStatus = KRequestPending;
            SetActive();
            //iPinNRs is initialized here
            CreateListL( iStatus ); 
            break;	
            }
            
        case ECreateListDone:
            {
            //Pin type is set here
            
            if ( iPinNRs ) // If PINs were found
	           {
	           TUid pintype = {WIM_PIN_G_UID}; // The first one is always PIN-G
	           TInt count = iPinNRs->Count();
	           TUint32 status = 0;

	           for ( TInt i = 0; i < count; i++ )
		           {
		           CWimAuthenticationObject* me = 
		                                MakeAuthObjectL( iToken, 
		                                             *( *iPinNRs )[i],
		                                              ( *iPinNRs )[i]->Label(),
		                                                 pintype,
		                                              ( *iPinNRs )[i]->PinNumber(),
		                                                 status );
		           CleanupStack::PushL( me );
		           User::LeaveIfError ( iAuthObjects.Append( me ) );
		           CleanupStack::Pop( me );
		           pintype.iUid = WIM_PIN_NR_UID; // The others are always PIN-NR
		           }
	           }    
            iPhase  = EDoList;  
            iStatus = KRequestPending;
            SetActive();
            TRequestStatus* status = &iStatus;
            User::RequestComplete( status, KErrNone );
            break;         	
            } 
    	
    	case EDoList:
    	    {
    	    _WIMTRACE ( _L( "CWimAuthenticationObjectList::RunL(): EDoList" ) );
    	    DoListL( *iClientAuthObjects );
			    User::RequestComplete( iClientStatus, KErrNone );  	
			    break;
    	    }
    	    
       default:
            break;	
         }
    
    }
        
// -----------------------------------------------------------------------------
// CWimAuthenticationObjectList::DoCancel()
// -----------------------------------------------------------------------------
//       
void CWimAuthenticationObjectList::DoCancel()
   {
   _WIMTRACE ( _L( "CWimAuthenticationObjectList::DoCancel()" ) );	
   User::RequestComplete( iClientStatus, KErrCancel );
   }
  
// -----------------------------------------------------------------------------
// CWimAuthenticationObjectList::RunError()
// -----------------------------------------------------------------------------
//  
TInt CWimAuthenticationObjectList::RunError( TInt aError )    
   {
   _WIMTRACE ( _L( "CWimAuthenticationObjectList::RunError()" ) );	
   User::RequestComplete( iClientStatus, aError );
   return KErrNone;
   }
   

//  End of File
