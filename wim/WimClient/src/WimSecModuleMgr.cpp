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
* Description:  API for list all WIM Security Modules
*
*/



//INCLUDES
#include "WimSecModuleMgr.h"
#include "WimSecModule.h"
#include "WimMgmt.h"
#include "WimTrace.h"
 


// ================= MEMBER FUNCTIONS  for CWimSecModuleMgr=======================
//
//  C++ default constructor. 
//
//
CWimSecModuleMgr::CWimSecModuleMgr():CActive( EPriorityHigh )
    {
    }

// -----------------------------------------------------------------------------
// CWimSecModuleMgr::NewL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
EXPORT_C CWimSecModuleMgr* CWimSecModuleMgr::NewL()
    {
    _WIMTRACE ( _L( "CWimSecModuleMgr::NewL()" ) );
    CWimSecModuleMgr* self = new( ELeave ) CWimSecModuleMgr();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CWimSecModuleMgr::ConstructL()
// Second phase
// -----------------------------------------------------------------------------
//
void CWimSecModuleMgr::ConstructL()
    {
    _WIMTRACE ( _L( "CWimSecModuleMgr::ConstructL()" ) );
    CActiveScheduler::Add( this );  
    }

// -----------------------------------------------------------------------------
// CWimSecModuleMgr::~CWimSecModuleMgr()
// Frees memory & preferences that have been allocated.
// -----------------------------------------------------------------------------
//     
EXPORT_C CWimSecModuleMgr::~CWimSecModuleMgr()
    {
    _WIMTRACE ( _L( "CWimSecModuleMgr::~CWimSecModuleMgr()" ) );
    Cancel();
    DeAllocateWimReferences();
    }

// -----------------------------------------------------------------------------
// CWimSecModuleMgr::GetEntries()
// Returns all WIM Security Modules in array.
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimSecModuleMgr::GetEntries( RCPointerArray<HBufC>& aTokens, 
                                           TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimSecModuleMgr::GetEntries()" ) );
    aStatus = KRequestPending;
    iTokenLabels = &aTokens;  
    iPhase = ECreateNewSession;
    iClientStatus = &aStatus;   
    SignalOwnStatusAndComplete();
    }

// -----------------------------------------------------------------------------
// CWimSecModuleMgr::WimCount()
// Returns the number of WIM-cards
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CWimSecModuleMgr::WimCount()
    {
    _WIMTRACE ( _L( "CWimSecModuleMgr::WimCount()" ) );
    if( !iWims )
        {
        return KErrNotFound;
        }
    return iWims->Count(); 
    }

// -----------------------------------------------------------------------------
// CWimSecModuleMgr::GetWimSecModuleByIndexL()
// Returns CWimSecModule according to received index.
// -----------------------------------------------------------------------------
//
EXPORT_C CWimSecModule* CWimSecModuleMgr::GetWimSecModuleByIndexL(
                                                   const TInt aIndex )
    {
    _WIMTRACE ( _L( "CWimSecModuleMgr::GetWimSecModuleByIndexL()" ) );
    if ( !iWims || aIndex > iWims->Count() || aIndex < 0 )
        {
        User::Leave( KErrNotFound );
        }
    return iWims->At( aIndex );        
    }

// -----------------------------------------------------------------------------
// CWimSecModuleMgr::WimLabelByIndexL()
// Returns WIM label according to received index
// -----------------------------------------------------------------------------
//
EXPORT_C TPtrC CWimSecModuleMgr::WimLabelByIndexL( const TInt aIndex )
    {
    _WIMTRACE ( _L( "CWimSecModuleMgr::WimLabelByIndexL()" ) );
    if ( !iWims || aIndex > iWims->Count() || aIndex < 0 )
        {
        User::Leave( KErrNotFound );
        }
    return iWims->At( aIndex )->Label();
    }

// -----------------------------------------------------------------------------
// CWimSecModuleMgr::WimTokenNumberByIndexL()
// Returns the number of "slot" -where current token is attached.
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CWimSecModuleMgr::WimTokenNumberByIndexL( const TInt aIndex )
    {
    _WIMTRACE ( _L( "CWimSecModuleMgr::WimTokenNumberByIndexL()" ) );
    if ( !iWims || aIndex > iWims->Count() || aIndex < 0 )
        {
        User::Leave( KErrNotFound );
        }
    return iWims->At( aIndex )->TokenNumber();
    }

// -----------------------------------------------------------------------------
// CWimSecModuleMgr::CancelList()
// Cancels listing operation.
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimSecModuleMgr::CancelList()
    {
    _WIMTRACE ( _L( "CWimSecModuleMgr::CancelList()" ) );
    Cancel();                          
    }

// -----------------------------------------------------------------------------
// CWimSecModuleMgr::SignalOwnStatusAndComplete()
// Sets own iStatus to KRequestPending, and signals it 
// with User::RequestComplete() -request. This gives chance 
// to active scheduler to run other active objects. After a quick
// visit in active scheduler, signal returns to RunL() and starts next
// phase of listing operation. This makes possible the cancelling 
// -function to get through.
// -----------------------------------------------------------------------------
//
void CWimSecModuleMgr::SignalOwnStatusAndComplete()
    {
    _WIMTRACE ( _L( "CWimSecModuleMgr::SignalOwnStatusAndComplete()" ) );
    iStatus = KRequestPending;
    SetActive();
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, KErrNone );
    }


// -----------------------------------------------------------------------------
//  CWimSecModuleMgr::WIMModulesL() 
//  Allocate pointer to WIM and create CWimSecModules.
//  This function calls server one time. It gets WIM references.
// -----------------------------------------------------------------------------
//
TInt CWimSecModuleMgr::WIMModulesL() 
    {
    _WIMTRACE ( _L( "CWimSecModuleMgr::WIMModulesL()" ) );
    ReadWimCount();
    TInt status = KErrNone;
    if ( iWimCount > 0 )
        {
        iWimAddrLst = new ( ELeave ) TWimAddress[iWimCount];
        status = iClientSession->WIMRefs( iWimAddrLst, ( TText8 )iWimCount );
        
        if ( status == KErrNone )
            {
            if( iWims )
                {
                iWims->ResetAndDestroy();
                delete iWims;
                iWims = NULL;
                }
            
            iWims = new( ELeave ) CArrayPtrFlat<CWimSecModule> ( iWimCount );
            /* Construct WIM security modules */ 
            TUint8 index;
            for( index = 0; index < iWimCount; index++ )
                {
                CWimSecModule* wimSecModule = CWimSecModule::NewL( 
                                                    iWimAddrLst[index] );
                CleanupStack::PushL( wimSecModule );
                wimSecModule->SetClientSession( iClientSession );
                iWims->AppendL( wimSecModule );
                CleanupStack::Pop( wimSecModule );
                }
            AppendTokenLabelsToArrayL();
            }
        }
    else
        {
        status = KErrHardwareNotAvailable;
        }
    return status;
    }

// -----------------------------------------------------------------------------
// CWimSecModuleMgr::AppendTokenLabelsToArrayL()
// Appends labels of available Tokens (wims) to an array.
// -----------------------------------------------------------------------------
//
void CWimSecModuleMgr::AppendTokenLabelsToArrayL()
    {
    _WIMTRACE ( _L( "CWimSecModuleMgr::AppendTokenLabelsToArrayL()" ) );
    TInt count = iWims->Count();
    TInt i;
    for ( i = 0; i < count; i++ )
        {
        HBufC* Tokenlabel = iWims->At( i )->Label().AllocLC();
        User::LeaveIfError( iTokenLabels->Append( Tokenlabel ) );
        CleanupStack::Pop( Tokenlabel ); //Tokenlabel
        }
    }

// -----------------------------------------------------------------------------
// CWimSecModuleMgr::ReadWimCount()
// Reads the count of the WIM.
// -----------------------------------------------------------------------------
//
void CWimSecModuleMgr::ReadWimCount()
    {
    _WIMTRACE ( _L( "CWimSecModuleMgr::ReadWimCount()" ) );
    iWimCount = iClientSession->WIMCount();
    }

// -----------------------------------------------------------------------------
// CWimSecModuleMgr::DoCancel()
// Deallocates member variables and completes client status with
// KErrCancel error code..
// -----------------------------------------------------------------------------
//
void CWimSecModuleMgr::DoCancel()
    {
    if( iCurrentPhase == EInitializeWim )
	    {
	    iClientSession->CancelInitialize();	
	    }
    User::RequestComplete( iClientStatus, KErrCancel );
    }


// -----------------------------------------------------------------------------
// CWimSecModuleMgr::RunL()
// Different phases are handled here. This might be considered as state machine. 
// iPhase = ECreateNewSession: Gets ClientSession handle. Tries to connect
//          to existing session. If there is no existing session, a new session 
//          is created.
// iPhase = EConnectClientSession: If new session was needed to be created, 
//          new connecting request is needed.
// iPhase = EInitializeWim: Sends initialize request to server and waits
//          asyncronously.
// iPhase = EGetWimRefs: Reads information about WIM-cards and
//          constructs CWimSecmodules. (one WIM-card  == one CWimSecModule )
// -----------------------------------------------------------------------------
//
void CWimSecModuleMgr::RunL()
    {
    
    if ( iStatus.Int() != KErrNone )
        {
        //This is possible only when initializing WIM.
        _WIMTRACE2( _L("CWimSecModuleMgr::RunL unable to initialize =%d"),iStatus.Int() );
        DeAllocateWimReferences();
        User::RequestComplete( iClientStatus, KErrHardwareNotAvailable );
        return;
        }

    switch ( iPhase )
        {
        case ECreateNewSession:
            { 
            iCurrentPhase = ECreateNewSession;
            
            if ( !iClientSession )
                {
                 _WIMTRACE ( _L( "CWimSecModuleMgr::RunL() brand new session" ) );
                //get ClientSession handle
                iClientSession = RWimMgmt::ClientSessionL();
                }
            
            iPhase = EConnectClientSession;
            TInt retval = 0;
            iWimStartErr = iClientSession->Connect();
            if ( iWimStartErr == KErrNotFound ) //is there existing session?
                {
                retval = iClientSession->StartWim();
                if ( retval != KErrNone )
                    {
                    User::RequestComplete( iClientStatus, 
                                           KErrHardwareNotAvailable ); 
                    }
                else
                    {
                    SignalOwnStatusAndComplete();
                    }
                } 
            else
                {
                 // Yes there is, continue
                 SignalOwnStatusAndComplete(); 
                }
            break;
            }
        case EConnectClientSession:
            {
            //Raise iPhase
            iCurrentPhase  = EConnectClientSession;
            iPhase = EInitializeWim; 
            if ( iWimStartErr )
                {
                //New session was needed to be created. Try to reconnect.
                iWimStartErr = iClientSession->Connect();
                if ( iWimStartErr != KErrNone )
                    {
                    //Failed to connect. No reason to continue
                    User::RequestComplete( iClientStatus, 
                                           KErrHardwareNotAvailable );
                    }
                else
                    {
                    SignalOwnStatusAndComplete();
                    }
                }
            else
                {
                SignalOwnStatusAndComplete();
                }
            break;
            }
        case EInitializeWim:
            {
            iCurrentPhase = EInitializeWim;
            //Initialize WIM
            SetActive();
            iClientSession->Initialize( iStatus );
            iPhase = EGetWimRefs;
            break;   
            }
        case EGetWimRefs:
            {
            iCurrentPhase = EGetWimRefs;
            TInt retval = TInt( WIMModulesL() );
            _WIMTRACE2 ( _L( "CWimSecModuleMgr::RunL(),\
                case:EGetWimRefs error =%d" ), retval );
      
            User::RequestComplete( iClientStatus, retval );
            break;
            }

        default:
            {
            break;
            }
        } 
    }


// -----------------------------------------------------------------------------
// CWimSecModuleMgr::RunError() 
//                                       
// The active scheduler calls this function if this active object's RunL() 
// function leaves. This gives this active object the opportunity to perform 
// any necessary cleanup.
// After array's cleanup, complete request with received error code.
// -----------------------------------------------------------------------------
TInt CWimSecModuleMgr::RunError( TInt aError )
    {
    _WIMTRACE ( _L( "CWimSecModuleMgr::RunError()" ) );
    DeAllocateWimReferences();
    User::RequestComplete( iClientStatus, aError );
    return KErrNone;
    }


// -----------------------------------------------------------------------------
// CWimSecModuleMgr::DeAllocateWimReferences()
// Frees memory & preferences that have been allocated.
// -----------------------------------------------------------------------------
//     

void CWimSecModuleMgr::DeAllocateWimReferences()
    {
    _WIMTRACE ( _L( "CWimSecModuleMgr::DeAllocateWimReferences()" ) );
    if ( iTokenLabels )
        {
        iTokenLabels = NULL;
        }
    if ( iWims )
        {
        iWims->ResetAndDestroy();
        delete iWims;
        iWims = NULL;
        }
    if ( iClientSession )
        {
        if (( iWimCount > 0 ) && iWimAddrLst )
            {
            iClientSession->FreeWIMAddrLst( iWimAddrLst, iWimCount );
            delete[] iWimAddrLst;
            }
        }
    if ( iClientSession )
        {
        iClientSession->Close();
        delete iClientSession;
        iClientSession = NULL;
        }
    }





