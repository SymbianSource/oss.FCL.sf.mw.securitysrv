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
* Description:
*
*/


// Class include
#include <DevEncSession.h>

// User includes
//#include "DevEncDiskUtils.h"
#include "DevEncLog.h"
#include "DevEncProtectedPSKey.h"

// System includes
#include <DevEncEngineBase.h>
#include <DevEncEngineConstants.h>
#include <e32property.h>

#include <centralrepository.h>
#include <DevEncExternalCRKeys.h>

EXPORT_C CDevEncSessionBase* NewSessionL()
    {
    return new (ELeave) CDevEncSession;
    }

// --------------------------------------------------------------------------
// CDevEncSession::CDevEncSession()
// 
// --------------------------------------------------------------------------
EXPORT_C CDevEncSession::CDevEncSession():
iDriveNumber( EDriveC )
	{
	
	}
	
EXPORT_C CDevEncSession::CDevEncSession( TDriveNumber aNumber ):
    iConnected (EFalse), iDriveNumber( aNumber ), iDevEncEngine (NULL)
    {
    // Derived from CBase -> members zeroed.
    }

// --------------------------------------------------------------------------
// CDevEncSession::~CDevEncSession()
// 
// --------------------------------------------------------------------------
CDevEncSession::~CDevEncSession()
    {
    if (iConnected)
        UnloadDevEncEngine();
    }

// --------------------------------------------------------------------------
// CDevEncSession::Close()
// 
// --------------------------------------------------------------------------
EXPORT_C void CDevEncSession::Close()
    {
    UnloadDevEncEngine();
    iConnected = EFalse;
    }

// --------------------------------------------------------------------------
// CDevEncSession::Connect()
// 
// --------------------------------------------------------------------------
EXPORT_C TInt CDevEncSession::Connect()
    {
    TRAPD(err,LoadDevEncEngineL());
    return err;
    }

// --------------------------------------------------------------------------
// CDevEncSession::StartDiskEncrypt()
// 
// --------------------------------------------------------------------------
EXPORT_C TInt CDevEncSession::StartDiskEncrypt()
    {
    FLOG("CDevEncSession::StartDiskEncrypt >>");
     
     ASSERT( iConnected );

     TInt err = KErrNone;
     TInt diskStatus = 0;

     err = DiskStatus( diskStatus );

     if( err )
         {
         DFLOG2( "ERROR: Disk status query failed, err %d", err);
         }
     else if( EDecrypted == diskStatus )
         {
         DFLOG( "Starting encryption" );
         err = iDevEncEngine->StartDiskOperation( EEncryptDisk );
         }
     else
         {
         DFLOG2( "Encryption not possible, disk status %d", diskStatus);
         err = KErrNotReady;
         }
     FLOG2("CDevEncSession::StartDiskEncrypt, err = %d <<", err);
     return err;
    }

// --------------------------------------------------------------------------
// CDevEncSession::StartDiskDecrypt()
// 
// --------------------------------------------------------------------------
EXPORT_C TInt CDevEncSession::StartDiskDecrypt()
    {
    ASSERT( iConnected );

    TInt err = KErrNone;
    TInt diskStatus = 0;

    err = DiskStatus( diskStatus );

    if( err )
        {
        DFLOG2( "ERROR: Disk status query failed, err %d", err);
        }
    else if( EEncrypted == diskStatus )
        {
        DFLOG( "Starting decryption" );
        err = iDevEncEngine->StartDiskOperation( EDecryptDisk );
        }
    else
        {
        DFLOG2( "Encryption not possible, disk status %d", diskStatus);
        err = KErrNotReady;
        }

    return err;
    }

// --------------------------------------------------------------------------
// CDevEncSession::StartDiskWipe()
// 
// --------------------------------------------------------------------------
EXPORT_C TInt CDevEncSession::StartDiskWipe()
    {
    ASSERT( iConnected );

    DFLOG( "Starting wiping" );
    TInt err = iDevEncEngine->StartDiskOperation( EWipeDisk );

    return err;
    }

// --------------------------------------------------------------------------
// CDevEncSession::DiskStatus()
// 
// --------------------------------------------------------------------------
EXPORT_C TInt CDevEncSession::DiskStatus( TInt& aStatus ) const
    {
    ASSERT( iConnected );
    TInt err = iDevEncEngine->DiskStatus(aStatus);
    return err;
    }

// --------------------------------------------------------------------------
// CDevEncSession::Progress()
// 
// --------------------------------------------------------------------------
EXPORT_C TInt CDevEncSession::Progress( TInt& aProgress ) const
    {
    ASSERT( iConnected );
    // Progress status must be polled, because after the extension has stopped
    // updating the status there will be no new progress updates --> will wait
    // indefinitely.
    TInt err = iDevEncEngine->Progress(aProgress);

    return err;
    }

// --------------------------------------------------------------------------
// CDevEncSession::Connected()
// 
// --------------------------------------------------------------------------
EXPORT_C TBool CDevEncSession::Connected() const
    {
    return iConnected;
    }

// --------------------------------------------------------------------------
// CDevEncSession::DriveNumber()
// 
// --------------------------------------------------------------------------
EXPORT_C TDriveNumber CDevEncSession::DriveNumber() const
    {
    return iDriveNumber;
    }
	
// --------------------------------------------------------------------------
// CDevEncSession::SetDrive()
// 
// --------------------------------------------------------------------------
EXPORT_C void CDevEncSession::SetDrive( TDriveNumber aNumber )
    {
    iDriveNumber = aNumber;
    }

void CDevEncSession::LoadDevEncEngineL()
    {
    FLOG(" CDevEncSession::LoadDevEncEngineL >> ");
    
    if (!iDevEncEngine)
        {
         TInt err = iLibrary.Load(KEncryptionDll);
         if (err != KErrNone)
             {
             FLOG2("Error in finding the library... %d", err);
             if (err == KErrNotFound)
                 err = KErrNotSupported;
             User::Leave(err);
             }
         TLibraryFunction entry = iLibrary.Lookup(1);
         
         if (!entry)
             {
             FLOG("Error in loading the library...");
             User::Leave(KErrBadLibraryEntryPoint);
             }
         iDevEncEngine = (CDevEncEngineBase *) entry();
         TInt errconnection = iDevEncEngine->Connect(iDriveNumber);
         if( errconnection != KErrNone )
        	 {
             User::Leave(errconnection);
        	 }
         iConnected = ETrue;
        }
    FLOG(" CDevEncSession::LoadDevEncEngineL << ");
    }

void CDevEncSession::UnloadDevEncEngine()
    {
    FLOG(" CDevEncSession::UnloadDevEncEngineL >> ");
    
    if (iDevEncEngine)
        {
        iDevEncEngine->Close();
        delete iDevEncEngine;
        iDevEncEngine = NULL;
        iLibrary.Close();
        }
    iConnected = EFalse;
    
    FLOG(" CDevEncSession::UnloadDevEncEngineL << ");
    }

EXPORT_C TInt CDevEncSession::SetDevEncControlL(TInt aValue)
    {
    DFLOG2("CDevEncSession SetDevEncControl, value = %d >>", aValue);
    
    CRepository* rep = CRepository::NewLC(TUid::Uid(KCRDevEncUiSettings));
    TInt err = rep->Set(KDevEncUiDmControl, aValue);
    CleanupStack::PopAndDestroy(rep);
    
    DFLOG2("CDevEncSession SetDevEncControl, err = %d <<", err);
    return err;
    }

EXPORT_C TInt CDevEncSession::GetDevEncControlL(TInt& aValue)
    {
    DFLOG("CDevEncSession GetDevEncControl >>");
    
    CRepository* rep = CRepository::NewLC(TUid::Uid(KCRDevEncUiSettings));
    TInt err = rep->Get(KDevEncUiDmControl, aValue);
    CleanupStack::PopAndDestroy(rep);
    
    DFLOG2("CDevEncSession GetDevEncControl, err = %d <<", err);
    return err;
    }
// End of file
