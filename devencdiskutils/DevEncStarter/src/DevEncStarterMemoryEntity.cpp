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
* Description:  Implementation of CDevEncUiMemoryEntity
*
*/


#include "DevEncLog.h"
#include "DevEncStarterMemoryEntity.h"
#include "DevEncUiMemInfoObserver.h"

#include <DevEncDiskUtils.h>
#include <DevEncSession.h>
#include <f32file.h> // for TDriveNumer
#include <DevEncEngineConstants.h>

// Local definitions
const TInt KPercent100( 100 );
const TTimeIntervalMicroSeconds32 KProgressInterval( 500000 ); // ms, 0.5 seconds
const TTimeIntervalMicroSeconds32 KPollInterval( 2000000 ); // ms, 2 seconds

// --------------------------------------------------------------------------
// CDevEncStarterMemoryEntity::CDevEncStarterMemoryEntity()
// 
// --------------------------------------------------------------------------
CDevEncStarterMemoryEntity::CDevEncStarterMemoryEntity(
                        TDevEncUiMemoryType aType )
    : iType( aType )
    {
    // TBI: Get the real states from the underlying SW levels
	iState = EDecrypted;
    }


// --------------------------------------------------------------------------
// CDevEncStarterMemoryEntity::~CDevEncStarterMemoryEntity()
// 
// --------------------------------------------------------------------------
CDevEncStarterMemoryEntity::~CDevEncStarterMemoryEntity()
    {
    if ( iPeriodic )
        {
        iPeriodic->Cancel();
        delete iPeriodic;
        }

    iObservers.Close();
    delete iSession;
    delete iDiskStatusObserver;
    }

// --------------------------------------------------------------------------
// CDevEncStarterMemoryEntity::NewL()
// 
// --------------------------------------------------------------------------
CDevEncStarterMemoryEntity* CDevEncStarterMemoryEntity::NewL(
                                TDevEncUiMemoryType aType )
	{
	CDevEncStarterMemoryEntity* self =
	    CDevEncStarterMemoryEntity::NewLC( aType );
	CleanupStack::Pop( self );
	return self;
	}

// --------------------------------------------------------------------------
// CDevEncStarterMemoryEntity::NewLC()
// 
// --------------------------------------------------------------------------
CDevEncStarterMemoryEntity* CDevEncStarterMemoryEntity::NewLC(
                                TDevEncUiMemoryType aType )
	{
	CDevEncStarterMemoryEntity* self =
	    new ( ELeave ) CDevEncStarterMemoryEntity( aType );
	CleanupStack::PushL( self );
	self->ConstructL();
	return self;
	}

// --------------------------------------------------------------------------
// CDevEncStarterMemoryEntity::ConstructL()
// 
// --------------------------------------------------------------------------
void CDevEncStarterMemoryEntity::ConstructL()
	{
    DFLOG( ">>CDevEncStarterMemoryEntity::ConstructL" );
    switch( iType )
        	{
        	case EPhoneMemory: iSession = new ( ELeave ) CDevEncSession( /*EDriveC*/EDriveE );
                               iDiskStatusObserver = CDiskStatusObserver::NewL( this, /*EDriveC*/EDriveE ); 
        		               break;
        		               
        	case EPrimaryPhoneMemory: iSession = new ( ELeave ) CDevEncSession( EDriveC );
                              iDiskStatusObserver = CDiskStatusObserver::NewL( this, EDriveC );
        	    		      break;
        	    		      
        	case EMemoryCard: iSession = new ( ELeave ) CDevEncSession( /*EDriveE*/EDriveF );
                              iDiskStatusObserver = CDiskStatusObserver::NewL( this, /*EDriveE*/EDriveF );
        	    		      break;    	    		               
            }
   
    User::LeaveIfError( iSession->Connect() );

    // Get initial memory state
    TInt nfeDiskStatus( EUnmounted );
#ifndef __WINS__
    TInt err = iSession->DiskStatus( nfeDiskStatus );
    if ( err )
        {
        DFLOG2( "DevEncStarter: Could not get disk status, error %d", err );
        nfeDiskStatus = EDecrypted;
        //User::Leave( err );
        }
#else
    nfeDiskStatus = EDecrypted;
#endif
    SetState( nfeDiskStatus );

    if ( ( iState == EEncrypting ) ||
         ( iState == EDecrypting ) )
        {
        // If the app was started in the middle of an ongoing operation,
        // start polling the progress
        StartPolling( KProgressInterval );
        }
    else
        {
        // Otherwise poll every once in a while to see if the status changes
        //StartPolling( KPollInterval );
        }

    DFLOG( "<<CDevEncStarterMemoryEntity::ConstructL" );
	}

// --------------------------------------------------------------------------
// CDevEncStarterMemoryEntity::AddObserverL()
// 
// --------------------------------------------------------------------------
void CDevEncStarterMemoryEntity::AddObserverL( MDevEncUiMemInfoObserver* aInfoObserver )
    {
    DFLOG( "CDevEncStarterMemoryEntity::AddObserverL" );
    if ( !aInfoObserver )
        {
        User::Leave( KErrArgument );
        }
    iObservers.AppendL( aInfoObserver );

    // The new observer probably wants to know the current states
    UpdateMemoryInfo();
    }

// --------------------------------------------------------------------------
// CDevEncStarterMemoryEntity::RemoveObserver()
// 
// --------------------------------------------------------------------------
void CDevEncStarterMemoryEntity::RemoveObserver( MDevEncUiMemInfoObserver* aInfoObserver )
    {
    if ( !aInfoObserver )
        {
        return;
        }
    TInt index( iObservers.Find( aInfoObserver ) );
    if ( index != KErrNotFound )
        {
        iObservers.Remove( index );
        }
    }

// --------------------------------------------------------------------------
// CDevEncStarterMemoryEntity::StartPolling()
// Starts a timer to periodically update the memory state in the UI
// --------------------------------------------------------------------------
void CDevEncStarterMemoryEntity::StartPolling(
                                      TTimeIntervalMicroSeconds32 aInterval )
    {
    DFLOG( "CDevEncStarterMemoryEntity::StartPolling" );
    TInt error( KErrNone );
    TRAP( error, DoStartPollingL( aInterval ) );
    DFLOG2( "CDevEncUiMemoryEntity::StartPolling result %d", error );
    }

// --------------------------------------------------------------------------
// CDevEncStarterMemoryEntity::DoStartPollingL()
// Starts a timer to periodically update the memory state in the UI
// --------------------------------------------------------------------------
void CDevEncStarterMemoryEntity::DoStartPollingL(
                                      TTimeIntervalMicroSeconds32 aInterval )
    {
    DFLOG( "CDevEncStarterMemoryEntity::DoStartPollingL" );
    TCallBack pollCallBack( PollTick, static_cast<TAny*>( this ) );
    TCallBack progressCallBack( ProgressTick, static_cast<TAny*>( this ) );

    if ( !iPeriodic )
        {
        iPeriodic = CPeriodic::NewL( EPriorityNormal );
        }
    iPeriodic->Cancel();

    if ( aInterval == KProgressInterval )
        {
        iPeriodic->Start( 0, aInterval, progressCallBack );
        }
    else
        {
        iPeriodic->Start( 0, aInterval, pollCallBack );
        }
    }

// --------------------------------------------------------------------------
// CDevEncStarterMemoryEntity::ProgressTick()
// 
// --------------------------------------------------------------------------
TInt CDevEncStarterMemoryEntity::ProgressTick( TAny* aPtr )
    {
    CDevEncStarterMemoryEntity* self = static_cast<CDevEncStarterMemoryEntity*>( aPtr );
    self->DoProgressTick();
    return 0;
    }

// --------------------------------------------------------------------------
// CDevEncStarterMemoryEntity::DoProgressTick()
// 
// --------------------------------------------------------------------------
void CDevEncStarterMemoryEntity::DoProgressTick()
    {
    CheckProgress();
    }

// --------------------------------------------------------------------------
// CDevEncStarterMemoryEntity::PollTick()
// 
// --------------------------------------------------------------------------
TInt CDevEncStarterMemoryEntity::PollTick( TAny* aPtr )
    {
    CDevEncStarterMemoryEntity* self = static_cast<CDevEncStarterMemoryEntity*>( aPtr );
    self->DoPollTick();
    return 0;
    }

// --------------------------------------------------------------------------
// CDevEncStarterMemoryEntity::DoPollTick()
// 
// --------------------------------------------------------------------------
void CDevEncStarterMemoryEntity::DoPollTick()
    {
    TInt nfeDiskStatus( EUnmounted );
    TInt err = iSession->DiskStatus( nfeDiskStatus );
    if ( err )
        {
        DFLOG2( "DevEncStarter: Could not get disk status, error %d", err );
        // Ignore error
        return;
        }
    if ( ( nfeDiskStatus == EEncrypting ) ||
         ( nfeDiskStatus == EDecrypting ) )
        {
        // Some other component has started an encryption operation.
        // Indicate this to the UI and start polling the progress.
        SetState( nfeDiskStatus );
        StartPolling( KProgressInterval );
        }
    else
        {
        if ( iState != nfeDiskStatus )
            {
            // The Mmc status has changed, but we are not in the middle of
            // any operation. Just set the new state.
            SetState( nfeDiskStatus );
            }
        }
    }

// --------------------------------------------------------------------------
// CDevEncStarterMemoryEntity::SetState()
// 
// --------------------------------------------------------------------------
void CDevEncStarterMemoryEntity::SetState( TUint aState )
    {
    DFLOG3( "CDevEncStarterMemoryEntity::SetState, prev %d, new %d",
            iState, aState );
    iState = aState;
    UpdateMemoryInfo();
    }

// --------------------------------------------------------------------------
// CDevEncStarterMemoryEntity::CheckProgress()
// 
// --------------------------------------------------------------------------
void CDevEncStarterMemoryEntity::CheckProgress()
    {
    ASSERT( iSession );
    ASSERT( iSession->Connected() );

    TInt err = iSession->Progress( iPercentDone );
    
    DFLOG( "CDevEncStarterMemoryEntity::CheckProgress" );
    
    if ( err )
        {
        DFLOG( "DevEncStarter: Could not get progress" );
        iPeriodic->Cancel();
        }
    else if ( iPercentDone >= KPercent100 )
        {
        DFLOG( "DevEncStarter: Operation complete" );
        iPeriodic->Cancel();
        iPercentDone = 0;
      
        if( iSession->DriveNumber() == EDriveC )
        	{
            // Finalize the phone memory
        	DFLOG( "DevEncStarter: DiskFinalize" );
            err = CDevEncDiskUtils::DiskFinalize( iSession->DriveNumber() );
            if ( err )
                {
                DFLOG2( "DevEncStarter: Finalization failed, error %d", err );
                }
            }

        // Restart the polling at a slower pace
        // StartPolling( KPollInterval );
        }
    else // Not yet finished...
        {
        UpdateMemoryInfo();
        }
    }

// --------------------------------------------------------------------------
// CDevEncStarterMemoryEntity::UpdateMemoryInfo()
// 
// --------------------------------------------------------------------------
void CDevEncStarterMemoryEntity::UpdateMemoryInfo()
    {
    DFLOG( "CDevEncStarterMemoryEntity::UpdateMemoryInfo()" );
    for ( TInt i = 0; i < iObservers.Count(); i++ )
        {
        iObservers[i]->UpdateInfo( iType, iState, iPercentDone );
        }
    }

// --------------------------------------------------------------------------
// CDevEncStarterMemoryEntity::State()
// 
// --------------------------------------------------------------------------
TUint CDevEncStarterMemoryEntity::State() const
    {
    return iState;
    }

// --------------------------------------------------------------------------
// CDevEncStarterMemoryEntity::DiskStatusChangedL()
// From MDiskStatusObserver
// --------------------------------------------------------------------------
void CDevEncStarterMemoryEntity::DiskStatusChangedL( TInt aNfeStatus )
    {
    if ( ( aNfeStatus == EEncrypting ) ||
         ( aNfeStatus == EDecrypting ) )
        {
        // Some other component has started an encryption operation.
        // Indicate this to the UI and start polling the progress.
        DFLOG2( "CDevEncStarterMemoryEntity::DiskStatusChangedL => status = %d",
        		aNfeStatus );
        SetState( aNfeStatus );
        StartPolling( KProgressInterval );
        }
    else
        {        
        DFLOG2( "CDevEncStarterMemoryEntity::DiskStatusChangedL => status = %d",
            	 aNfeStatus );
        
        if( iState != aNfeStatus )
        	{
        	DFLOG2( "CDevEncStarterMemoryEntity::DiskStatusChangedL => status changed, iState was %d", iState );
            SetState( aNfeStatus );
        	}
        }
    }

// End of File



