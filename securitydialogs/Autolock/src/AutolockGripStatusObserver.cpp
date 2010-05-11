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
#include "AutolockGripStatusObserver.h"
#include "AutolockApp.h"
#include <aknkeylock.h>


EXPORT_C CAutolockGripStatusObserver* CAutolockGripStatusObserver::NewL( MAutolockGripStatusObserver* aObserver, RWsSession& aSession )
    {
    CAutolockGripStatusObserver* self = new (ELeave) CAutolockGripStatusObserver( aSession );
    CleanupStack::PushL( self );
    self->ConstructL( aObserver );
    CleanupStack::Pop( self );
    return self;
    }

void CAutolockGripStatusObserver::ConstructL( MAutolockGripStatusObserver* aObserver )
    {
    #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockGripStatusObserver::ConstructL") );
    #endif
    TInt err = iGripStatus.Attach( KPSUidHWRM, KHWRMGripStatus );
    if ( err )
        {
        #if defined(_DEBUG)
        RDebug::Print(_L("(AUTOLOCK)ERROR: Attach failed, err %d"), err );
        #endif
        }
    iObserver = aObserver;
    CActiveScheduler::Add( this );
    iGripStatus.Subscribe( iStatus );
    SetActive();
    }

CAutolockGripStatusObserver::CAutolockGripStatusObserver( RWsSession& aSession ) : CActive( EPriorityIdle ), iSession( aSession )
    {
    }

CAutolockGripStatusObserver::~CAutolockGripStatusObserver()
    {
    #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockGripStatusObserver::~CAutolockGripStatusObserver") );
    #endif
    Cancel();
    iGripStatus.Close();
    }

void CAutolockGripStatusObserver::DoCancel()
    {
    #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockGripStatusObserver::DoCancel") );
    #endif
    iGripStatus.Cancel();
    }

void CAutolockGripStatusObserver::RunL()
    {
    #if defined(_DEBUG)
    RDebug::Print(_L("(AUTOLOCK)CAutolockGripStatusObserver::RunL") );
    #endif
    iGripStatus.Subscribe( iStatus );
    SetActive();
    
    TInt gripStatus;
    TInt err = iGripStatus.Get( gripStatus );
    if( !err )
    	{
        GripStatusChangedL( gripStatus );
        }
    }

void CAutolockGripStatusObserver::GripStatusChangedL( TInt aGripStatus )
    {
    #if defined(_DEBUG)
	RDebug::Print(_L("(AUTOLOCK)CAutolockGripStatusObserver::::GripStatusChangedL"));
	#endif   
    if( aGripStatus == EPSHWRMGripOpen ) 
    	{
        #if defined(_DEBUG)
    	RDebug::Print(_L("(AUTOLOCK)CAutolockGripStatusObserver::::GripStatusChangedL => Grip opened"));
    	#endif 
    	if( !iObserver->DeviceLockQueryStatus() && iObserver->DeviceLockStatus() )
    		{
            #if defined(_DEBUG)
        	RDebug::Print(_L("(AUTOLOCK)CAutolockGripStatusObserver::::GripStatusChangedL => send command"));
        	#endif
    	    //Grip opened
        	TApaTaskList tasklist( iSession );
        	/* this is old code. It was changed to a new one, following a suggestion from the Slide-handling team
        	TApaTask capserver = tasklist.FindApp( KAknCapServerUid );
        	if( capserver.Exists() )
        	    {
        	    TKeyEvent key;
        	    key.iCode = EKeyDevice0;
        	    key.iModifiers = 0;
        	    key.iRepeats = 0;
        	    key.iScanCode = EStdKeyDevice0;
        	    capserver.SendKey( key );
        	    }
					*/
					TApaTask capserver = tasklist.FindApp( KUidAutolock ); 
					if( capserver.Exists() ) 
					        { 
					        TKeyEvent key; 
					        key.iCode = EKeyBell; 
					        capserver.SendKey( key ); 
					        } 
					RAknKeylock2 keylock; 
					TInt error( keylock.Connect() ); 
					if ( !error ) 
					    { 
					    keylock.DisableWithoutNote(); 
					    keylock.Close(); 
					    } 
    		}
        }
    else
        {
        #if defined(_DEBUG)
    	RDebug::Print(_L("(AUTOLOCK)CAutolockGripStatusObserver::::GripStatusChangedL => Grip closed"));
    	#endif 
        //Grip closed
        if( iObserver->DeviceLockQueryStatus() )
        	{
            #if defined(_DEBUG)
        	RDebug::Print(_L("(AUTOLOCK)CAutolockGripStatusObserver::::GripStatusChangedL => send key event"));
        	#endif
            //the device lock query is on top
        	//generate cancel key event
        	TRawEvent rawEvent;
        	rawEvent.Set( TRawEvent::EKeyDown, KCancelKeyCode );
        	iSession.SimulateRawEvent( rawEvent );        	
            }
        }
    }

// End of File
