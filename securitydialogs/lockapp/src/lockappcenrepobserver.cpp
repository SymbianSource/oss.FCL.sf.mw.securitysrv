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
* Description:  Central Repository key observer
 *
*/


#include <centralrepository.h>
#include "lockappcenrepobserver.h"
#include "lockapputils.h"

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
CLockAppCenRepObserver* CLockAppCenRepObserver::NewL( MLockAppObserverInterface* aObserver,
        TUid aCenRepUid, TUint32 aKeyId )
    {
    CLockAppCenRepObserver* self = new (ELeave) CLockAppCenRepObserver(aObserver, aCenRepUid, aKeyId);
    CleanupStack::PushL( self );
    self->ConstructL( );
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
CLockAppCenRepObserver::~CLockAppCenRepObserver( )
    {
    if ( iNotifyHandler )
        {
        iNotifyHandler->StopListening( );
        delete iNotifyHandler;
        iNotifyHandler = NULL;
        }
    if ( iRepository )
        {
        delete iRepository;
        iRepository = NULL;
        }
    }

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
CLockAppCenRepObserver::CLockAppCenRepObserver( MLockAppObserverInterface* aObserver,
        TUid aCenRepUid, TUint32 aKeyId ) :
    iObserver(aObserver),
    iCenRepUid(aCenRepUid),
    iKeyId(aKeyId)
    {
    }

// ---------------------------------------------------------------------------
// Symbian OS default constructor
// ---------------------------------------------------------------------------
void CLockAppCenRepObserver::ConstructL( )
    {
    // init cenrep connection
RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    iRepository = CRepository::NewL( iCenRepUid );
    iNotifyHandler = CCenRepNotifyHandler::NewL( *this, *iRepository,
            CCenRepNotifyHandler::EIntKey, iKeyId );
    iNotifyHandler->StartListeningL( );
    }

// ---------------------------------------------------------------------------
// Gets value of the default key from CenRep.
// ---------------------------------------------------------------------------
TInt CLockAppCenRepObserver::GetValue(TInt& aValue )
    {
    return iRepository->Get( iKeyId, aValue );
    }

// ---------------------------------------------------------------------------
// Gets value of the key from CenRep.
// ---------------------------------------------------------------------------
TInt CLockAppCenRepObserver::GetKeyValue(TUint32 aKey, TInt& aValue )
    {
    	    RDebug::Printf( "%s %s (%u) aKey=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aKey );

    return iRepository->Get( aKey, aValue );
    }

// ---------------------------------------------------------------------------
// Sets a value for the default key in CenRep.
// ---------------------------------------------------------------------------
TInt CLockAppCenRepObserver::SetValue(TInt aValue )
    {
    return iRepository->Set( iKeyId, aValue );
    }

// ---------------------------------------------------------------------------
// Sets a value for the key in CenRep.
// ---------------------------------------------------------------------------
TInt CLockAppCenRepObserver::SetKeyValue(TUint32 aKey, TInt aValue )
    {
    return iRepository->Set( aKey, aValue );
    }

// ---------------------------------------------------------------------------
// Handles changes. Called by CenRep.
// ---------------------------------------------------------------------------
void CLockAppCenRepObserver::HandleNotifyInt(TUint32 aKeyId, TInt aValue )
    {
RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    if ( aKeyId == iKeyId )
        {
        if ( iObserver )
            {
            iObserver->HandleCenRepNotify( iCenRepUid, iKeyId, aValue );
            }
        }
    }

// ---------------------------------------------------------------------------
// Handles errors. Called by CenRep.
// ---------------------------------------------------------------------------
void CLockAppCenRepObserver::HandleNotifyError(TUint32 aId, TInt error, CCenRepNotifyHandler* /*aHandler*/)
    {
    ERROR_1(error, "CLockAppCenRepObserver::HandleNotifyError - %d", aId);
    }

// ---------------------------------------------------------------------------
// Handles Repository wide reset caused generic notifications. Called by CenRep.
// ---------------------------------------------------------------------------
void CLockAppCenRepObserver::HandleNotifyGeneric(TUint32 aId )
    {
    if ( aId == NCentralRepositoryConstants::KInvalidNotificationId )
        {
        // TODO implement what to do in this case
        }
    }

// END OF FILE
