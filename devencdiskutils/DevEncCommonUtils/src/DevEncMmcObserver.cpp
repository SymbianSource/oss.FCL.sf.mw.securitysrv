/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
*              Implements a watcher for MMC disk change events.   
*
*
*/


#include <e32base.h>
#include <f32file.h>
#include "DevEncMmcObserver.h"


EXPORT_C CMmcObserver* CMmcObserver::NewL( MMemoryCardObserver* aObserver,
                                  RFs* aFileServerSession )
    {
    CMmcObserver* self = new(ELeave) CMmcObserver();
    CleanupStack::PushL( self );
    self->ConstructL( aObserver, aFileServerSession );
    CleanupStack::Pop( self );
    return self;
    }

void CMmcObserver::ConstructL( MMemoryCardObserver* aObserver,
                               RFs* aFileServerSession )
    {
    iObserver = aObserver;
    iFsSession = aFileServerSession;
    CActiveScheduler::Add( this );
    }

CMmcObserver::CMmcObserver() : CActive( EPriorityIdle )
    {
    }

CMmcObserver::~CMmcObserver()
    {
    Cancel();
    }

void CMmcObserver::DoCancel()
    {
    iFsSession->NotifyChangeCancel();
    }

void CMmcObserver::RunL()
    {
    //TRACES(RDebug::Print( _L("CMmcObserver::RunL status: %d"), iStatus.Int() ) );
    iObserver->MMCStatusChangedL();
    StartObserver();
    }

EXPORT_C void CMmcObserver::StartObserver()
    {
    //TRACES(RDebug::Print(_L("CMmcObserver::StartObserver")));
    if ( IsActive() ) 
        {
        Cancel();
        }
    iFsSession->NotifyChange( ENotifyDisk, iStatus );
    SetActive();
    }

// End of File
