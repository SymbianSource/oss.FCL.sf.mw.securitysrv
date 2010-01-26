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
*              Implements a watcher for disk status change events.   
*
*
*/


#include <e32base.h>
#include <f32file.h>
#include <DevEncEngineConstants.h>
#include "DevEncDiskStatusObserver.h"
#include "DevEncLog.h"
#include "DevEncProtectedPSKey.h"


EXPORT_C CDiskStatusObserver* CDiskStatusObserver::NewL( MDiskStatusObserver* aObserver, TDriveNumber aDriveNumber )
    {
    DFLOG( "CDiskStatusObserver::NewL" );
    CDiskStatusObserver* self = new (ELeave) CDiskStatusObserver();
    CleanupStack::PushL( self );
    self->ConstructL( aObserver, aDriveNumber );
    CleanupStack::Pop( self );
    return self;
    }

void CDiskStatusObserver::ConstructL( MDiskStatusObserver* aObserver, TDriveNumber aDriveNumber )
    {
    DFLOG( "CDiskStatusObserver::ConstructL" );
    TInt err = iNfeStatus.Attach( KEncUtility, ENC_KEY( aDriveNumber, KDEStatusToUiKey ) );
    if ( err )
        {
        DFLOG2( "ERROR: Attach failed, err %d", err );
        }
    iObserver = aObserver;
    iDrive = aDriveNumber;
    CActiveScheduler::Add( this );
    iNfeStatus.Subscribe( iStatus );
    SetActive();
    }

CDiskStatusObserver::CDiskStatusObserver() : CActive( EPriorityIdle )
    {
    }

CDiskStatusObserver::~CDiskStatusObserver()
    {
    DFLOG( "CDiskStatusObserver::~CDiskStatusObserver" );
    Cancel();
    iNfeStatus.Close();
    }

void CDiskStatusObserver::DoCancel()
    {
    DFLOG( "CDiskStatusObserver::DoCancel" );
    iNfeStatus.Cancel();
    }

void CDiskStatusObserver::RunL()
    {
    DFLOG( "CDiskStatusObserver::RunL" );
    iNfeStatus.Subscribe( iStatus );
    SetActive();
    
    TInt nfeStatus;
    TInt err = iNfeStatus.Get( nfeStatus );
    if( !err )
        {
        TInt status;
        switch( nfeStatus )
        	{
        	case EDecrypting: status = EOpDecrypting;
        		              break;
        	case EEncrypting: status = EOpEncrypting;
        		              break;
        	default: status = EOpIdle;
        		     break;
            }
        //if a multidrive encryption/decryption is ongoing we cannot change the status of the ps key until
        //all the drives have concluded the operation. 
        if( ( status == EOpIdle && !IsEncryptionOperationOngoing() ) || status != EOpIdle )
            {
            RProperty::Set( KDevEncProtectedUid, KDevEncOperationKey, status );
            }
        iObserver->DiskStatusChangedL( nfeStatus );
        }
    }

TBool CDiskStatusObserver::IsEncryptionOperationOngoing()
    {
    DFLOG( "CDiskStatusObserver::IsEncryptionOperationOngoing" );
    TBool ret( EFalse );
    TBool cont( ETrue );
    for( TInt i = EDriveA; ( i < KMaxDrives ) && cont; i++ )
        {
        TInt status;
        TInt err = RProperty::Get( KEncUtility, ENC_KEY( (TDriveNumber) i, KDEStatusToUiKey ), status );
        if( !err )
            {
            if( status == EDecrypting || status == EEncrypting )
                {
                DFLOG2( "drive %d has an encryption or decryption operation ongoing", i );
            	ret = ETrue;
            	cont = EFalse;
                }
            else
                {
                DFLOG2( "drive %d is in Idle", i );
                }
            }
        else
        	{
        	DFLOG3( "ERROR: Get failed for drive %d, err %d", i, err );
            }
        }    
    return ret;
    }

// End of File
