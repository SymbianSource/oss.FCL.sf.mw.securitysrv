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
* Description:
*
*/

#include <e32base.h>
#include <e32debug.h>
#include <AknCapServerDefs.h>
#include <apgtask.h>
#include "AutolockFpsStatusObserver.h"


EXPORT_C CAutolockFpsStatusObserver* CAutolockFpsStatusObserver::NewL( MAutolockFpsStatusObserver* aObserver, RWsSession& aSession )
    {
    CAutolockFpsStatusObserver* self = new (ELeave) CAutolockFpsStatusObserver( aSession );
    CleanupStack::PushL( self );
    self->ConstructL( aObserver );
    CleanupStack::Pop( self );
    return self;
    }

void CAutolockFpsStatusObserver::ConstructL( MAutolockFpsStatusObserver* aObserver )
    {
    #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockFpsStatusObserver::ConstructL") );
    #endif
const TUid KUidFpsCategory = {0x1020507E };

// PnS key
const TUint32 KFpsAuthenticationKey = 0x00000001;

    TInt err = iFpsStatus.Attach( KUidFpsCategory, KFpsAuthenticationKey );
    if ( err )
        {
        #if defined(_DEBUG)
        RDebug::Print(_L("(AUTOLOCK)ERROR: Attach failed, err %d"), err );
        #endif
        }
    iObserver = aObserver;
    CActiveScheduler::Add( this );
    iFpsStatus.Subscribe( iStatus );
    SetActive();
    }

CAutolockFpsStatusObserver::CAutolockFpsStatusObserver( RWsSession& aSession ) : CActive( EPriorityIdle ), iSession( aSession )
    {
    }

CAutolockFpsStatusObserver::~CAutolockFpsStatusObserver()
    {
    #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockFpsStatusObserver::~CAutolockFpsStatusObserver") );
    #endif
    Cancel();
    iFpsStatus.Close();
    }

void CAutolockFpsStatusObserver::DoCancel()
    {
    #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockFpsStatusObserver::DoCancel") );
    #endif
    iFpsStatus.Cancel();
    }

void CAutolockFpsStatusObserver::RunL()
    {
    #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockFpsStatusObserver::RunL") );
    #endif
    iFpsStatus.Subscribe( iStatus );
    SetActive();
    
    TInt FpsStatus;
    TInt err = iFpsStatus.Get( FpsStatus );
    if( !err )
    	{
        FpsStatusChangedL( FpsStatus );
        }
    }

void CAutolockFpsStatusObserver::FpsStatusChangedL( TInt aFpsStatus )
    {
const TUint32 ESwipeValid = 0x00000001;
    if( aFpsStatus == ESwipeValid )
    	{
        #if defined(_DEBUG)
    	RDebug::Print(_L("(AUTOLOCK)CAutolockFpsStatusObserver::FpsStatusChangedL => Fps opened"));
    	#endif 
    	if( iObserver->DeviceLockStatus() )
    		{
            #if defined(_DEBUG)
        	RDebug::Print(_L("(AUTOLOCK)CAutolockFpsStatusObserver::FpsStatusChangedL => unlocking"));
        	#endif
		iObserver->DeviceFpsLock(1);
    		}
    	else
    		{
            #if defined(_DEBUG)
        	RDebug::Print(_L("(AUTOLOCK)CAutolockFpsStatusObserver::FpsStatusChangedL => locking"));
        	#endif
		iObserver->DeviceFpsLock(0);
    		}
        }
    }

// End of File
