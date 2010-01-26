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
* Description:  Implementation of CDevEncStarterMmcObserver
*
*  Version    : %version: 9 %, %date_modified: Fri Nov 13 11:16:40 2009 % by %derived_by: e0326969 %
*/


#include <centralrepository.h>
#include <DevEncExternalCRKeys.h>
#include <DevEncEngineConstants.h>
#include "DevEncLog.h"
#include "DevEncMmcObserver.h"
#include "DevEncStarterMmcObserver.h"
#include "DevEncStarterMemoryEntity.h"
#include "DevEncStarterUtils.h"
#include "DevEncUids.hrh"
#include "MmcInfo.h"

const TInt mmcDrive( /*EDriveE*/EDriveF );
//const TInt KPowerKeyCode( EKeyPowerOff );
//const TInt KPowerKeyScanCode( EStdKeyPowerOff );

// -----------------------------------------------------------------------------
// CDevEncStarterMmcObserver::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
CDevEncStarterMmcObserver* CDevEncStarterMmcObserver::NewL(
                                               CDevEncStarterUtils*& aUtils )
    {
    CDevEncStarterMmcObserver* self =
        new ( ELeave ) CDevEncStarterMmcObserver( aUtils );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CDevEncStarterMmcObserver::CDevEncStarterMmcObserver
// 
// -----------------------------------------------------------------------------
CDevEncStarterMmcObserver::CDevEncStarterMmcObserver(
                                               CDevEncStarterUtils*& aUtils )
    : iUtils( aUtils )
    {
    }

// -----------------------------------------------------------------------------
// CDevEncStarterMmcObserver::ConstructL
// 
// -----------------------------------------------------------------------------
void CDevEncStarterMmcObserver::ConstructL()
    {
    DFLOG( "CDevEncStarterMmcObserver::ConstructL" );
    // Central repository settings
    iCrSettings = CRepository::NewL( TUid::Uid( KCRDevEncUiSettings ) );

    DFLOG( "ObsCon 1" );
    
    // Create objects representing the different types of memory
    // The order is important
    CDevEncStarterMemoryEntity* phoneMemory =
        CDevEncStarterMemoryEntity::NewLC( EPhoneMemory );
    iMemEntities.AppendL( phoneMemory );
    CleanupStack::Pop( phoneMemory );

        DFLOG( "ObsCon 2" );

    CDevEncStarterMemoryEntity* memoryCard =
        CDevEncStarterMemoryEntity::NewLC( EMemoryCard );
    iMemEntities.AppendL( memoryCard );
    CleanupStack::Pop( memoryCard );

        DFLOG( "ObsCon 3" );
    CDevEncStarterMemoryEntity* massMemory =
                CDevEncStarterMemoryEntity::NewLC( EPrimaryPhoneMemory );
    
    iMemEntities.AppendL( massMemory );
    CleanupStack::Pop( massMemory );

    // Create an MMC file system observer
    User::LeaveIfError( iFs.Connect() );
    iObserver = CMmcObserver::NewL( this, &iFs );
    iObserver->StartObserver();
    
        DFLOG( "ObsCon 4" );

    // Call the MMC callback function once to get the initial card status
    MMCStatusChangedL();

        DFLOG( "ObsCon 5" );

    // This class needs to observe memory card status changes.
    // (This call results in a call to UpdateInfo, hence we need to be fully
    // constructed before this.)
    iMemEntities[ EMemoryCard ]->AddObserverL( this );

        DFLOG( "ObsCon 6" );

    }

// -----------------------------------------------------------------------------
// CDevEncStarterMmcObserver::~CDevEncStarterMmcObserver
// 
// -----------------------------------------------------------------------------
CDevEncStarterMmcObserver::~CDevEncStarterMmcObserver()
    {
    delete iObserver;
    iFs.Close();

    if ( iMemEntities.Count() > EMemoryCard ) 
	    {
        iMemEntities[ EMemoryCard ]->RemoveObserver( this );
        for ( TInt i = 0; i < iMemEntities.Count(); i++ )
            {
            delete iMemEntities[i];
            }
        iMemEntities.Close();
		}
    delete iCrSettings;
    }

// -----------------------------------------------------------------------------
// CDevEncStarterMmcObserver::MMCStatusChangedL
// From MMemoryCardObserver
// -----------------------------------------------------------------------------
void CDevEncStarterMmcObserver::MMCStatusChangedL()
    {
    DFLOG( "CDevEncStarterMmcObserver::MMCStatusChangedL" );
    TVolumeInfo volInfo;

    TInt err = iFs.Volume( volInfo, /*EDriveE*/EDriveF );
    switch ( err )
        {
        case KErrNone:
            {
            // Readable MMC inserted
            DFLOG( "MMC inserted" );
            iMmcStatus = EMmcOk;
            
            break;
            }

        case KErrNotReady:
            {
            // MMC ejected
            DFLOG( "MMC ejected" );
            iMmcStatus = EMmcNotPresent;
            //This is because for some reason when the memory card is ejected the Disk status observer is not notified
            //thus the Ui is started even if the MMC is removed.
            iMemEntities[ EMemoryCard ]->SetState( EUnmounted );
            break;
            }

        case KErrCorrupt:
            {
            // Corrupt or unformatted MMC, or wrong key
            DFLOG( "MMC corrupt, unformatted or encrypted with other key" );
            iMmcStatus = EMmcNotReadable;

                break;
            }

        default:
            {
            DFLOG2( "RFs::Volume returned error %d", err );
            break;
            }
        } // switch
    }

// -----------------------------------------------------------------------------
// CDevEncStarterMmcObserver::GetInfo
// From MMemoryCardObserver
// -----------------------------------------------------------------------------
TInt CDevEncStarterMmcObserver::GetInfo( TMmcInfo& aInfo )
    {
    TVolumeInfo volInfo;
    TDriveInfo driveInfo;

    // Check if MMC mounted
    TInt err = CheckMMC();

    DFLOG2( "CDevEncStarterMmcObserver::GetInfo - CheckMMC returned %d", err );
    
    if( !err )
        {
        // MMC is mounted, get further information
        err = iFs.Drive( driveInfo, mmcDrive );
        
        DFLOG2( "CDevEncStarterMmcObserver::GetInfo - iFs.Drive returned %d", err );
        DFLOG3( "CDevEncStarterMmcObserver::GetInfo - driveInfo.iDriveAtt %d, driveInfo.iMediaAtt %d",
                driveInfo.iDriveAtt,
                driveInfo.iMediaAtt );

        if ( !err )
            {
            aInfo.iIsFormattable = driveInfo.iMediaAtt & KMediaAttFormattable;
            aInfo.iIsLockable = driveInfo.iMediaAtt & KMediaAttLockable;
            aInfo.iIsLocked = driveInfo.iMediaAtt & KMediaAttLocked;
            aInfo.iIsReadOnly = driveInfo.iMediaAtt & KMediaAttWriteProtected;
            aInfo.iIsPasswordSet = driveInfo.iMediaAtt & KMediaAttHasPassword;

            if ( !aInfo.iIsLocked )
                {
                // Then get volume info for name etc.
                err = iFs.Volume( volInfo, mmcDrive );
//#if defined _DEBUG
#if 0
                TBuf<50> tempDes1; // helpers for showing valid memory amount, which do not fit in 32 bits
                TBuf<50> tempDes2;

                tempDes1.Num( volInfo.iSize );
                tempDes2.Num( volInfo.iFree );
                DFLOG2( "CDevEncStarterMmcObserver::GetInfo - iFs.Volume returned %d", err );
                DFLOG2( "CDevEncStarterMmcObserver::GetInfo - volInfo iUniqueID %d, iSize, iFree, iName:", 
                        volInfo.iUniqueID );
                DFLOGBUF( tempDes1 );
                DFLOGBUF( tempDes2 );
                DFLOGBUF( volInfo.iName );
#endif // _DEBUG
                }
            else
                {
                err = KErrLocked;
                }
            }
        else
            {
            err = KErrNotReady;
            }
        }

    DFLOG2( "CDevEncStarterMmcObserver::GetInfo - err %d", err );
    
    if( !err )
        {
        // MMC ok, get rest of the information
        aInfo.iIsInserted = ETrue;
        aInfo.iIsInUse = EFalse;
        aInfo.iName = volInfo.iName;
        aInfo.iUid = volInfo.iUniqueID;
        // Use all 64 bits
        aInfo.iCapacity = volInfo.iSize;
        aInfo.iSpaceFree = volInfo.iFree;

        // Check if the volume is formatted
        aInfo.iIsFormatted = volInfo.iSize > 0;

        // Test if a previous backup exists
        aInfo.iBackupExists = EFalse; //BaflUtils::FileExists( iFs, aBackupFile );
        }
    else
        {
        // By default set values as 
        aInfo.iIsInserted = EFalse;
        aInfo.iIsInUse = EFalse;
        aInfo.iIsFormatted = EFalse;
        aInfo.iBackupExists = EFalse;

        switch( err )
            {
            case KErrInUse:
                {
                aInfo.iIsInserted = ETrue;
                aInfo.iIsInUse = ETrue;
                break;
                }
            case KErrNotReady:
                {
                // If card is not ready, but media attributes set, it is reserved.
                if ( driveInfo.iMediaAtt )
                    {
                    aInfo.iIsInserted = ETrue;
                    aInfo.iIsInUse = ETrue;
                    }
                break;
                }
            case KErrCorrupt:
                {
                aInfo.iIsInserted = ETrue;
                break;
                }
            case KErrLocked:
                {
                // Could not mount due card locked or volume info query returned card locked.
                // Hence info-flags possibly set accoring to iMediaAtt are overridden.
                aInfo.iIsInserted = ETrue;
                aInfo.iIsFormatted = ETrue;
                aInfo.iIsLocked = ETrue;
                aInfo.iIsPasswordSet = ETrue;
                aInfo.iIsLockable = ETrue;
                break;
                }
            default:
                {
                break;
                }
            }
        }

    return err;
    }

// -----------------------------------------------------------------------------
// CDevEncStarterMmcObserver::CheckMMC
// 
// -----------------------------------------------------------------------------
TInt CDevEncStarterMmcObserver::CheckMMC()
    {
    // Check if MMC mounted, mount if required
    TFullName fsname;

    // If can't use the drive, returns KErrNotFound
    TInt err = iFs.FileSystemName( fsname, mmcDrive );

    if ( !err )
        {
        // MMC found. Checking if it is mounted
        if ( fsname.Length( ) == 0 )
            {
            // MMC not mounted ( file system name is empty ) 
            DFLOG( "Mmc not mounted" );
            err = KErrDisMounted;
            }
        }
    return err;
    }

// -----------------------------------------------------------------------------
// CDevEncStarterMmcObserver::CheckMMC
// 
// -----------------------------------------------------------------------------
void CDevEncStarterMmcObserver::LogInfo( const TMmcInfo& aInfo ) const
    {
    DFLOG2( "aInfo.iIsFormattable %d", aInfo.iIsFormattable ? 1 : 0 );
    DFLOG2( "aInfo.iIsFormatted %d", aInfo.iIsFormatted ? 1 : 0 );
    DFLOG2( "aInfo.iIsLockable %d", aInfo.iIsLockable ? 1 : 0 );
    DFLOG2( "aInfo.iIsLocked %d", aInfo.iIsLocked ? 1 : 0 );
    DFLOG2( "aInfo.iIsReadOnly %d", aInfo.iIsReadOnly ? 1 : 0 );
    DFLOG2( "aInfo.iIsPasswordSet %d", aInfo.iIsPasswordSet ? 1 : 0 );
    DFLOG2( "aInfo.iIsInserted %d", aInfo.iIsInserted ? 1 : 0 );
    DFLOG2( "aInfo.iBackupExists %d", aInfo.iBackupExists ? 1 : 0 );
    DFLOG2( "aInfo.iIsInUse %d", aInfo.iIsInUse ? 1 : 0 );
    }

// --------------------------------------------------------------------------
// Called by memory entity when memory status changes.
// 
// --------------------------------------------------------------------------
void CDevEncStarterMmcObserver::UpdateInfo( TDevEncUiMemoryType aType,
                                            TUint aState,
                                            TUint /*aProgress*/ )
    {
    DFLOG3( "CDevEncStarterMmcObserver::UpdateInfo, type %d, state %d",
            aType, aState );
    DFLOG2( "Phone state %d", iPhoneEncState );
    DFLOG2( "Mmc state %d", iMmcStatus );

    TBool startUi = NeedToStartUi( aType, aState );

    if ( aType == EPhoneMemory )
        {
        iPhoneEncState = aState;
        }
    else // EMemoryCard
        {
        iMmcEncState = aState;    
        }

    if ( startUi )
        {
        TInt error( KErrNone );
        TRAP( error, iUtils->StartAppL( KDevEncUiUid,
                                        this ) );
        DFLOG2( "App start requested, result %d", error );
        }
    }

// -----------------------------------------------------------------------------
// CDevEncStarterMmcObserver::NeedToStartUi
// 
// -----------------------------------------------------------------------------
TBool CDevEncStarterMmcObserver::NeedToStartUi( TDevEncUiMemoryType aType,
                                                TUint aState )
    {
    TBool startUi( EFalse );
    
    DFLOG2( "CDevEncStarterMmcObserver::NeedToStartUi => aState = %d",
    		aState );
    DFLOG2( "CDevEncStarterMmcObserver::NeedToStartUi => iMmcStatus = %d",
    		iMmcStatus );
    DFLOG2( "CDevEncStarterMmcObserver::NeedToStartUi => iMmcEncState = %d",
    		iMmcEncState );

    // Check encryption key status
    TInt mmcKeyInDriver( 0 );
    TInt error = iCrSettings->Get( KDevEncUserSettingMemoryCard,
                                   mmcKeyInDriver );
    if ( error )
        {
        DFLOG2( "Cenrep get error %d", error );
        return startUi;
        }
    
    if ( ( iMmcStatus == EMmcOk ) && 
         ( iMmcEncState == EUnmounted ) &&
         ( aState == EDecrypted ) )
        {
        // A readable memory card has just been inserted
    
        if ( mmcKeyInDriver )
            {
            // We have a valid encryption key, but the newly inserted
            // card is decrypted. Ask if the user wants to encrypt.
            // (see UI spec 2.13)
            startUi = ETrue;
            }
        }

    if ( ( iMmcStatus == EMmcNotReadable ) && 
         ( iMmcEncState == EUnmounted ) &&
         ( aState == ECorrupted ) )
        {
        // An unreadable memory card has just been inserted
        
        if ( mmcKeyInDriver )
            {
            // We seem to have the wrong encryption key in the driver.
            // Inform the user that the card cannot be used.
            // (see UI spec 2.16)
            startUi = ETrue;
            }
        else
            {
            // We have no encryption key in the driver.
            // Ask if the user wants to import a key and decrypt.
            // (see UI spec 2.17)
            startUi = ETrue;
            }
        }

    if ( ( iMmcStatus == EMmcNotReadable ) && 
         ( iMmcEncState == ECorrupted ) &&
         ( aState == ECorrupted ) )
        {
        // We have an unreadable memory card, and tried to load a new key
        // but the card is still unreadable. Inform the user and let the user
        // select some other key. (see UI spec 2.10)
        startUi = ETrue;
        }

    if ( ( iMmcStatus == EMmcOk ) && 
         ( iMmcEncState == ECorrupted ) &&
         ( aState == EEncrypted ) )
        {
        // We have just imported the correct key and the card is now
        // readable. Show a note to the user.
        // (see UI spec 2.7)
        startUi = ETrue;
        }
    return startUi;
    }


// -----------------------------------------------------------------------------
// CDevEncStarterMmcObserver::AppStarted
// 
// -----------------------------------------------------------------------------
void CDevEncStarterMmcObserver::AppStarted( const TUint32& aUid )
    {
    if ( !iUtils )
        {
        DFLOG( "CDevEncStarterMmcObserver::AppStarted error: no utils" );
        return;
        }
    }

// End of file
